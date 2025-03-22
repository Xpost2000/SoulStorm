/*
  As per usual my game.cpp mega-translation unit is just
  where I glue all the game code together, so this isn't exactly
  the most stellar of code.
*/

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "discord_rich_presence_integration.h"
#include "fade_transition.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"
#include "achievements.h"
#include "game_uid_generator.h"

#include "file_buffer.h"
#include "game_ui.h"

#include "action_mapper.h"
#include "virtual_file_system.h"

#undef GAME_UI_SCREEN
#undef GAME_SCREEN
#define GAME_UI_SCREEN(name) void Game::name(struct render_commands* commands, f32 dt)
#define GAME_SCREEN(name) void Game::name(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt)

local int projectile_sprites_requiring_rotation_count = 0;
local int projectile_sprites_requiring_rotation[PROJECTILE_SPRITE_TYPES] = {
};

static void apply_vector_quantization_deadzone_adjustment(V2& vector) {
  // NOTE(jerry):
  //
  // There's a lot of precision loss when quantizing to a 8 bit number from a floating
  // point.
  //
  const float DEADZONE_QUANTIZATION_THRESHOLD = 0.075f;
  if (fabs(vector.x) <= DEADZONE_QUANTIZATION_THRESHOLD) {
    vector.x = 0;
  }

  if (fabs(vector.y) <= DEADZONE_QUANTIZATION_THRESHOLD) {
    vector.y = 0;
  }
}


local float replay_timescale_choices[] = {
    0.25f,
    0.5f,
    0.75f,
    1.0f,
    1.5f,
    1.75f,
    2.0f
};

// I kinda wanna draw a cute icon for each of the levels.
local Stage stage_list[] = {
    #include "stage_list.h"
};

// mainly for difficulty information
local Gameplay_Data_Pet_Information pets_data[] = {
    #include "pets_data.h"
};

Gameplay_Data_Pet_Information* game_get_pet_data(s32 id) {
    switch (id) {
        case GAME_PET_ID_NONE:
            return &pets_data[0];
        default:
            return &pets_data[id+1];
    }

    return nullptr;
}

// Should not change in the future.
enum Achievement_ID_List {
    ACHIEVEMENT_ID_STAGE1,
    ACHIEVEMENT_ID_STAGE2,
    ACHIEVEMENT_ID_STAGE3,


    // TODO: add achievement triggers.
    ACHIEVEMENT_ID_STAGE1_FLAWLESS,
    ACHIEVEMENT_ID_STAGE2_FLAWLESS,
    ACHIEVEMENT_ID_STAGE3_FLAWLESS,
    ACHIEVEMENT_ID_UNTOUCHABLE,

    // play as every character type(?)
    // ACHIEVEMENT_ID_JACK_OF_ALL_TRADES,
    // ACHIEVEMENT_ID_MERCIFUL,
    ACHIEVEMENT_ID_KILLER,
    ACHIEVEMENT_ID_MURDERER,
    ACHIEVEMENT_ID_SLAYER,

    ACHIEVEMENT_ID_PLATINUM,

    ACHIEVEMENT_ID_COUNT,
};

local Achievement achievement_list[] = {
    #include "achievement_list.h"
};


// NOTE: this is a freestanding function since the stage completion data and
//       achievement data is global, which I think is definitely acceptable.
//
//       I would prefer to allocate these else where if I ever had hot-reloading (so
//       they could be persistent.)
//       but this game doesn't really require that.

//       id and level_id are zero indexed.
enum Game_Complete_Stage_Type {
    GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY,
    GAME_COMPLETE_STAGE_UNLOCK_NEXT_LEVEL,
    GAME_COMPLETE_STAGE_CONTINUE_TO_NEXT_LEVEL,
    GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE,
};

bool g_prefpath_portable = false;

string prefqpath(string path)
{
  string full_pref_path;
  if (g_prefpath_portable) {
    full_pref_path = string_literal("./");
  } else {
    full_pref_path = get_preference_directory(string_literal("xpostgames"), string_literal("solstorm"));
  }
  full_pref_path = string_concatenate(&Global_Engine()->scratch_arena, full_pref_path, path);
  return full_pref_path;
}

local bool game_will_be_new_high_score(s32 stage_index, s32 level_index, s32 new_score) {
    auto& level = stage_list[stage_index].levels[level_index];
    return (new_score > level.best_score);
}

local void game_update_stage_score(s32 stage_index, s32 level_index, s32 new_score) {
    _debugprintf("Updating level (%d-%d) with score value: %d", stage_index, level_index, new_score);
    auto& level = stage_list[stage_index].levels[level_index];
    level.last_score = new_score;
    level.best_score = max(new_score, level.best_score);
}

// NOTE: attempts are recorded when you start a stage at all.
//       As I'm intending to use them for one-time messages like tutorial messages
//       or attempt events.
local void game_register_stage_completion(s32 stage_index, s32 level_index) {
    auto& level = stage_list[stage_index].levels[level_index];
    level.completions += 1;
}

local void game_register_stage_attempt(s32 stage_index, s32 level_index) {
    auto& level = stage_list[stage_index].levels[level_index];
    level.attempts += 1;
}

local int game_complete_stage_level(s32 stage_id, s32 level_id, bool practicing_stage) {
    auto& stage  = stage_list[stage_id];
    s32   result = 0;
    {
        /*
          The way the game structure is setup is that I assume levels
          must be unlocked in linear order, but there can definitely be
          some clean up as to some of this logic.

          Whether I do this or not is a good question, but it's not too much code to
          fix up.
        */
        if (practicing_stage) {
            _debugprintf("Practice stage complete");
            result = GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY; 
        } else {
            if ((level_id+1) >= stage.unlocked_levels) {
                /*
                 * NOTE:
                 *
                 *  This will increment on the last level to unlock a "stage 4" which doesn't
                 *  exist, but will just be used as a marker for allowing the next stage to be unlocked.
                 */
                _debugprintf("Completed stage (%d-%d) and unlocked the next stage!", stage_id+1, level_id+1);
                stage.unlocked_levels += 1;

                result = GAME_COMPLETE_STAGE_UNLOCK_NEXT_LEVEL;
            } else {
                _debugprintf("Completed stage (%d-%d) another time!", stage_id+1, level_id+1);
                result = GAME_COMPLETE_STAGE_CONTINUE_TO_NEXT_LEVEL; 
                // result = GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY; 
            }
        }
    }

    // Check for all stages completed.
    if (stage.unlocked_levels > MAX_LEVELS_PER_STAGE) {
        assertion(ACHIEVEMENT_ID_STAGE1 + stage_id <= ACHIEVEMENT_ID_STAGE3 && "Invalid achievement for stage id.");
        auto achievement = Achievements::get(ACHIEVEMENT_ID_STAGE1 + stage_id);
        achievement->report();

        result = GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE;
    }

    return result;
}

// NOTE: hard coded particle systems
Particle_Emitter& spawn_game_entity_hit_particle_emitter(Fixed_Array<Particle_Emitter>& particle_emitters, V2 where, Game_Resources* resources) {
    auto& emitter = *(particle_emitters.alloc());
    emitter.reset();
    emitter.sprite                  = sprite_instance(resources->circle_sprite);
    emitter.sprite.scale            = V2(0.125/4, 0.125/4);
    emitter.shape                   = particle_emit_shape_circle(where, 2.8f);
    emitter.modulation              = color32f32(1, 0.15, 0.1, 1);
    emitter.lifetime                = 0.45f;
    emitter.scale_variance          = V2(-0.005, 0.005);
    emitter.velocity_x_variance     = V2(-145, 145);
    emitter.velocity_y_variance     = V2(-145, 145);
    emitter.acceleration_x_variance = V2(-100, 100);
    emitter.acceleration_y_variance = V2(-100, 100);
    emitter.lifetime_variance       = V2(-0.25f, 1.0f);
    emitter.emission_max_timer      = 0.035f;
    emitter.max_emissions           = 1;
    emitter.emit_per_emission       = 8;
    emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE;
    emitter.scale                   = 1;

    return emitter;
}

void config_game_entity_player_propel_particle_emitter(Particle_Emitter& emitter, V2 where, Game_Resources* resources) {
    emitter.reset();
    emitter.sprite                  = sprite_instance(resources->circle_sprite16);
    emitter.sprite.scale            = V2(0.25, 0.25);
    emitter.shape                   = particle_emit_shape_point(where);
    // emitter.modulation              = color32f32(222.0f / 255.0f, 180.0f / 255.0f, 45.0f / 255.0f, 1.0f);
    emitter.modulation       = color32f32(0.7f, 0.78f, 0.95f, 1.0f);
    emitter.target_modulation       = color32f32(59/255.0f, 59/255.0f, 56/255.0f, 127/255.0f);
    emitter.lifetime                = 1.25f;
    emitter.scale_variance          = V2(-0.15, 0.45f);
    emitter.scale                   = 1.0f;
    emitter.angle_range             = V2(0, 180);
    emitter.velocity                = V2(150.0f, 150.0f);
    emitter.acceleration            = V2(0.0f, 120.0f);
    emitter.velocity_x_variance     = V2(-20, 150);
    emitter.velocity_y_variance     = V2(-20, 150);
    emitter.acceleration_x_variance = V2(-25, 25);
    emitter.acceleration_y_variance = V2(-10, 75);
    emitter.lifetime_variance       = V2(-0.25f, 0.2f);
    emitter.emission_max_timer      = 0.035f;
    emitter.max_emissions           = 1;
    emitter.emit_per_emission       = 32;
    emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
        PARTICLE_EMITTER_FLAGS_USE_ANGULAR |
        PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE |
        PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
    emitter.scale                   = 1;
    emitter.blend_mode              = BLEND_MODE_ALPHA;
}

Particle_Emitter& spawn_game_entity_death_particle_emitter(Fixed_Array<Particle_Emitter>& particle_emitters, V2 where, Game_Resources* resources, int type) {
    auto& emitter = *(particle_emitters.alloc());
    emitter.reset();
    emitter.sprite = sprite_instance(resources->circle_sprite16);
    emitter.sprite.scale = V2(0.175, 0.175);
    emitter.shape = particle_emit_shape_circle(where, 2.8f);
    if (type == 1) {
      emitter.modulation = RGBA32f32(255, 252, 127, 255);
    } else {
      emitter.modulation = RGBA32f32(167, 196, 255, 255);
    }
    emitter.lifetime = 0.45f;
    emitter.scale_variance = V2(-0.005, 0.005);
    emitter.velocity_x_variance = V2(-205, 205);
    emitter.acceleration_x_variance = V2(-150, 150);
    emitter.angle_range = V2(-360, 360);
    emitter.acceleration_y_variance = V2(-100, 100);
    emitter.lifetime_variance = V2(-0.25f, 1.0f);
    emitter.emission_max_timer = 0.035f;
    emitter.max_emissions = 1;
    emitter.emit_per_emission = 125;
    emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE | PARTICLE_EMITTER_FLAGS_USE_ANGULAR | PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
    emitter.scale = 1;

    return emitter;
}

// Method implementations

Game::Game() {
    
}

Game::~Game() {
    
}

void Game::load_projectile_sprites(Graphics_Driver* driver, lua_State* L) {
    /*
     * Well... The raw lua api isn't very pretty but it just needs to work.
     */
    _debugprintf("Loading projectile sprites.");
    lua_getglobal(L, "projectiles");
    // projectiles table loaded
    s32 sprite_count = lua_rawlen(L, -1);
    assertion(sprite_count <= PROJECTILE_SPRITE_TYPES && "Too many sprites!");
    _debugprintf("LUA Sprites to load: %d", sprite_count);

    for (int projectile_sprite_id = 0; projectile_sprite_id < sprite_count; ++projectile_sprite_id) {
        lua_pushinteger(L, projectile_sprite_id+1);
        lua_gettable(L, -2);

        // NOTE: table format is
        // { angled/directed_projectile, framelist }
        {
            lua_rawgeti(L, -1, 1);
            bool requires_rotation = lua_toboolean(L, -1);

            if (requires_rotation) {
                projectile_sprites_requiring_rotation[projectile_sprites_requiring_rotation_count++] = projectile_sprite_id;
            }

            lua_pop(L, 1);

            // frame table in
            lua_rawgeti(L, -1, 2);
        }

        s32 frames_to_alloc = lua_rawlen(L, -1);
        _debugprintf("Frames to load %d", frames_to_alloc);
        auto& sprite = resources->projectile_sprites[projectile_sprite_id];

        if (sprite.index == 0) {
            sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, frames_to_alloc);

            for (int frame_index = 0; frame_index < frames_to_alloc; ++frame_index) {
                lua_pushinteger(L, frame_index+1);
                lua_gettable(L, -2);

                // { filename, src rect (nil for now), timing (ignored for now) }
                lua_rawgeti(L, -1, 1);
                char* file_name = (char*)lua_tostring(L, -1);
                _debugprintf("sprite load FILENAME: %s", file_name);
                // lua_rawgeti(L, -1, 2); rectangle.
                // lua_rawgeti(L, -1, 3); timing.
                string frame_img_location = string_from_cstring(file_name);
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index);
                frame->img = graphics_assets_load_image(&resources->graphics_assets, frame_img_location);
                frame->source_rect = RECTANGLE_F32_NULL;

                // pop filename, table
                lua_pop(L, 2);
            }
        }
        _debugprintf("Finished loading a sprite");
        lua_pop(L, 2);
    }
}

void Game::load_entity_sprites(Graphics_Driver* driver, lua_State* L) {
    lua_getglobal(L, "entities");
    s32 sprite_count = lua_rawlen(L, -1);
    assertion(sprite_count <= ENTITY_SPRITE_TYPES && "Too many sprites!");
    _debugprintf("LUA Sprites to load: %d", sprite_count);

    for (int entity_sprite_id = 0; entity_sprite_id < sprite_count; ++entity_sprite_id) {
        lua_pushinteger(L, entity_sprite_id+1);
        lua_gettable(L, -2);

        // NOTE: table format is
        {
            // frame table in
            lua_rawgeti(L, -1, 1);
        }

        s32 frames_to_alloc = lua_rawlen(L, -1);
        _debugprintf("Frames to load %d", frames_to_alloc);
        auto& sprite = resources->entity_sprites[entity_sprite_id];

        if (sprite.index == 0) {
            sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, frames_to_alloc);

            for (int frame_index = 0; frame_index < frames_to_alloc; ++frame_index) {
                lua_pushinteger(L, frame_index+1);
                lua_gettable(L, -2);

                // { filename, src rect (nil for now), timing (ignored for now) }
                lua_rawgeti(L, -1, 1);
                char* file_name = (char*)lua_tostring(L, -1);
                _debugprintf("FILENAME: %s", file_name);
                // lua_rawgeti(L, -1, 2); rectangle.
                // lua_rawgeti(L, -1, 3); timing.
                string frame_img_location = string_from_cstring(file_name);
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index);
                frame->img = graphics_assets_load_image(&resources->graphics_assets, frame_img_location);
                frame->source_rect = RECTANGLE_F32_NULL;

                // pop filename, table
                lua_pop(L, 2);
            }
        }
        _debugprintf("Finished loading a sprite");
        lua_pop(L, 2);
    }
}
/*
 * Basically all these assets are hardcoded, and while I don't totally like this fact,
 * this is a relatively small game, and I'm the only programmer (and everything else too I guess).
 *
 * Some stuff is probably going to be pushed out to be data driven, but a lot of core stuff is hard coded.
 */
void Game::init_graphics_resources(Graphics_Driver* driver) {
    if (!initialized) {
        return;
    }
    graphics_assets_update_graphics_driver(&resources->graphics_assets, driver);

    for (unsigned index = 0; index < array_count(menu_font_variation_string_names); ++index) {
        string current = menu_font_variation_string_names[index];
        // HACKME FIX
        // didn't have too much foresight for changing graphics devices.
        // not a big deal but yeah.
        if (resources->menu_fonts[index].index == 0)
            resources->menu_fonts[index] = graphics_assets_load_bitmap_font(&resources->graphics_assets, current, 5, 12, 5, 20);
    }

    resources->circle = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/circle256.png"));
    resources->ui_marquee_bkrnd = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/bkgmarquee1.png"));
    resources->ui_marquee_bkrnd_neo[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/bkgmarquee-left.png"));
    resources->ui_marquee_bkrnd_neo[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/bkgmarquee-right.png"));

    resources->ui_vignette_borders[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/border_vignette_left.png"));
    resources->ui_vignette_borders[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/border_vignette_bottom.png"));
    resources->ui_rays_gradient = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/uieffect0.png"));

    resources->explosion_image[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/explosion_00.png"));
    resources->explosion_image[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/explosion_01.png"));
    resources->explosion_image[2] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/explosion_02.png"));

    resources->ui_border_vignette = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/border_vignette.png"));


    {
        _debugprintf("Attempting to load manifest lua file.");
        lua_State* L = luaL_newstate();
        {bind_vfs_lualib(L);}
        s32 error = vfs_lua_dofile(L, "./res/manifest.lua");
        if (error != LUA_OK) {
            _debugprintf("LUA_ERROR (%s) error?", lua_tostring(L,-1));
        }
        assertion(error == LUA_OK && "Failed to load manifest lua file?");
        load_projectile_sprites(driver, L);
        load_entity_sprites(driver, L);
        lua_close(L);
    }

    // load sprite images for main menu assets
    {
        local string hero_image_locations[] = {
            string_literal("res/img/hero/hero_idle0.png"),
            string_literal("res/img/hero/hero_idle_float_f.png"),
            string_literal("res/img/hero/hero_idle_float_l.png"),
            string_literal("res/img/hero/hero_idle_float_r.png"),
            string_literal("res/img/hero/hero_idle_float_b.png"),
            string_literal("res/img/hero/hero_idle_float_b_lean_l.png"),
            string_literal("res/img/hero/hero_idle_float_b_lean_r.png"),
        };

        local string portal_image_locations[] = {
            string_literal("res/img/mainmenu_portals/portal0.png"),
            string_literal("res/img/mainmenu_portals/portal1.png"),
            string_literal("res/img/mainmenu_portals/portal2.png"),
            string_literal("res/img/mainmenu_portals/portal3.png"),
            string_literal("res/img/mainmenu_portals/portal4.png"),
        };

        local string pet_image_locations[GAME_PET_ID_COUNT][4] = {
            {
                string_literal("res/img/pet_cat/back.png"),
                string_literal("res/img/pet_cat/front.png"),
                string_literal("res/img/pet_cat/left.png"),
                string_literal("res/img/pet_cat/right.png"),
            },
            {
                string_literal("res/img/pet_dog/back.png"),
                string_literal("res/img/pet_dog/front.png"),
                string_literal("res/img/pet_dog/left.png"),
                string_literal("res/img/pet_dog/right.png"),
            },
            {
                string_literal("res/img/pet_fish/back.png"),
                string_literal("res/img/pet_fish/front.png"),
                string_literal("res/img/pet_fish/left.png"),
                string_literal("res/img/pet_fish/right.png"),
            }
        };

        for (unsigned image_index = 0; image_index < array_count(hero_image_locations); ++image_index) {
            resources->hero_images[image_index] = graphics_assets_load_image(
                &resources->graphics_assets,
                hero_image_locations[image_index]
            );
        }

        for (unsigned pet_index = 0; pet_index < array_count(pet_image_locations); ++pet_index) {
            for (unsigned image_index = 0; image_index < 4; ++image_index) {
                resources->pet_images[pet_index][image_index] =
                    graphics_assets_load_image(
                        &resources->graphics_assets,
                        pet_image_locations[pet_index][image_index]
                    );
            }

            if (resources->pet_sprites[pet_index].index == 0) {
                resources->pet_sprites[pet_index] = graphics_assets_alloc_sprite(&resources->graphics_assets, 1);
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->pet_sprites[pet_index]), 0);
                frame->img = resources->pet_images[pet_index][PET_IMAGE_SPRITE_FACING_DIRECTION_BACK];
                frame->source_rect = RECTANGLE_F32_NULL;
            }
        }

        for (unsigned image_index = 0; image_index < array_count(portal_image_locations); ++image_index) {
            resources->main_menu_portal_images[image_index] = graphics_assets_load_image(
                &resources->graphics_assets,
                portal_image_locations[image_index]
            );
        }
    }
    {
        resources->main_menu_clutter_poop = graphics_assets_load_image(
            &resources->graphics_assets,
            string_literal("res/img/ui/ui_clutter_poop.png") 
        );

        resources->ui_hp_icons[UI_HP_ICON_TYPE_DEAD]   =
            graphics_assets_load_image(
                &resources->graphics_assets,
                string_literal("res/img/ui/ui_hp_dead.png")
            );
        resources->ui_hp_icons[UI_HP_ICON_TYPE_LIVING] =
            graphics_assets_load_image(
                &resources->graphics_assets,
                string_literal("res/img/ui/ui_hp_living.png")
            );
    }

    {
        if (resources->player_sprite.index == 0) {
            resources->player_sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, 3);
            {
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->player_sprite), 0);
                frame->img = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_BACK];
                frame->source_rect = RECTANGLE_F32_NULL;
            }
            {
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->player_sprite), 1);
                frame->img = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_BACK_LEAN_LEFT];
                frame->source_rect = RECTANGLE_F32_NULL;
            }
            {
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->player_sprite), 2);
                frame->img = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_BACK_LEAN_RIGHT];
                frame->source_rect = RECTANGLE_F32_NULL;
            }
        }
    }

    {
        if (resources->circle_sprite.index == 0) {
            resources->circle_sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, 1);
            auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->circle_sprite), 0);
            frame->img = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/circle64.png"));
            frame->source_rect = RECTANGLE_F32_NULL;
        }

        if (resources->circle_sprite16.index == 0) {
            resources->circle_sprite16 = graphics_assets_alloc_sprite(&resources->graphics_assets, 1);
            auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->circle_sprite16), 0);
            frame->img = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/circle16.png"));
            frame->source_rect = RECTANGLE_F32_NULL;
        }
    }
    {
        if (resources->point_pickup_sprite.index == 0) {
            resources->point_pickup_sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, 7);
            local string point_pickup_frames[] = {
                string_literal("res/img/pickup/item_pickup_generic1.png"),
                string_literal("res/img/pickup/item_pickup_generic2.png"),
                string_literal("res/img/pickup/item_pickup_generic3.png"),
                string_literal("res/img/pickup/item_pickup_generic4.png"),
                string_literal("res/img/pickup/item_pickup_generic3.png"),
                string_literal("res/img/pickup/item_pickup_generic2.png"),
                string_literal("res/img/pickup/item_pickup_generic1.png"),
            };

            for (unsigned frame_index = 0; frame_index < array_count(point_pickup_frames); ++frame_index) {
                auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, resources->point_pickup_sprite), frame_index);
                frame->img = graphics_assets_load_image(&resources->graphics_assets, point_pickup_frames[frame_index]);
                frame->source_rect = RECTANGLE_F32_NULL;
            }
        }
    }

#if COMPILE_IN_TRAILER_CLIPPING_CODE == 1
    {
        for (unsigned index = 0; index < array_count(resources->trailer_platform_logos); ++index) {
            resources->trailer_platform_logos[index] =
                graphics_assets_load_image(&resources->graphics_assets,
                                           g_platform_logo_paths[index]);
        }
        for (unsigned index = 0; index < array_count(resources->trailer_storefront_logos); ++index) {
            resources->trailer_storefront_logos[index] =
                graphics_assets_load_image(&resources->graphics_assets,
                                           g_storefront_logo_paths[index]);
        }
    }
#endif

#if BUILD_DEMO==0 || COMPILE_IN_TRAILER_CLIPPING_CODE==1
    {
        {
            resources->title_screen_puppet_eyes[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_eye0.png"));
            resources->title_screen_puppet_eyes[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_eye1.png"));
            resources->title_screen_puppet_eyes[2] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_eye2.png"));
            resources->title_screen_puppet_eyes[3] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_eye3.png"));
        }
        resources->title_screen_puppet_eye_brow = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_eyebrow0.png"));
        resources->title_screen_puppet_head     = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_head0.png"));
        resources->title_screen_puppet_arm      = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_arm.png"));
        resources->title_screen_puppet_torso    = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_torso0.png"));
    }
#endif

    {
        resources->title_screen_logo_fills[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_0_fill.png"));
        resources->title_screen_logo_masks[0] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_0_mask.png"));

        resources->title_screen_logo_fills[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_1_fill.png"));
        resources->title_screen_logo_masks[1] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_1_mask.png"));

        resources->title_screen_logo_fills[2] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_2_fill.png"));
        resources->title_screen_logo_masks[2] = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/title/title_part_2_mask.png"));
    }

    #if 1
    {
        {
            local string border_effect_paths[] = {
                string_literal("res/img/border/border_effect_solid.png"),
                string_literal("res/img/border/border_effect_deadly.png"),
                string_literal("res/img/border/border_effect_wraparound.png"),
                string_literal("res/img/border/border_effect_passthrough.png"),
                string_literal("res/img/border/border_alert.png"),
            };
            
            for (unsigned border_flash_id_effect = BORDER_FLASH_ID_TYPE_BLOCKING;
                 border_flash_id_effect < BORDER_FLASH_ID_TYPE_COUNT;
                 ++border_flash_id_effect) {
                resources->ui_border_effect[border_flash_id_effect-1] =
                    graphics_assets_load_image(&resources->graphics_assets, border_effect_paths[border_flash_id_effect-1]);
            }
        }

        // build UI atlas
        {
            auto& ui_chunky        = resources->ui_chunky;
            ui_chunky.top_left     = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/top-left.png"));
            ui_chunky.top_right    = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/top-right.png"));
            ui_chunky.bottom_left  = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/bottom-left.png"));
            ui_chunky.bottom_right = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/bottom-right.png"));
            ui_chunky.left         = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/left.png"));
            ui_chunky.right        = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/right.png"));
            ui_chunky.bottom       = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/bottom.png"));
            ui_chunky.top          = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/top.png"));
            ui_chunky.center       = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky/center.png"));
            ui_chunky.tile_width = ui_chunky.tile_height = 16;
        }
        {
            auto& ui_chunky        = resources->ui_chunky_outline;
            ui_chunky.top_left     = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/top-left.png"));
            ui_chunky.top_right    = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/top-right.png"));
            ui_chunky.bottom_left  = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/bottom-left.png"));
            ui_chunky.bottom_right = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/bottom-right.png"));
            ui_chunky.left         = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/left.png"));
            ui_chunky.right        = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/right.png"));
            ui_chunky.bottom       = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/bottom.png"));
            ui_chunky.top          = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/top.png"));
            ui_chunky.center       = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/ui/chunky_outline/center.png"));
            ui_chunky.tile_width = ui_chunky.tile_height = 16;
        }

        string locked_trophy_paths[] = {
            string_literal("res/img/ui/icons/trophy_locked.png"),
            string_literal("res/img/ui/icons/trophy_locked_shine1.png"),
            string_literal("res/img/ui/icons/trophy_locked_shine2.png"),
            string_literal("res/img/ui/icons/trophy_locked_shine3.png"),
            string_literal("res/img/ui/icons/trophy_locked_shine4.png"),
            string_literal("res/img/ui/icons/trophy_locked_shine5.png")
        };
        
        string unlocked_trophy_paths[] = {
            string_literal("res/img/ui/icons/trophy_blank.png"),
            string_literal("res/img/ui/icons/trophy_shine1.png"),
            string_literal("res/img/ui/icons/trophy_shine2.png"),
            string_literal("res/img/ui/icons/trophy_shine3.png"),
            string_literal("res/img/ui/icons/trophy_shine4.png"),
            string_literal("res/img/ui/icons/trophy_shine5.png")
        };

        for (int i = 0; i < TROPHY_ICON_COUNT; ++i) {
            resources->trophies_locked[i]   = graphics_assets_load_image(&resources->graphics_assets, locked_trophy_paths[i]);
            resources->trophies_unlocked[i] = graphics_assets_load_image(&resources->graphics_assets, unlocked_trophy_paths[i]);
        }

        // all main assets loaded, now try to build atlas.
        {
            int i = 0;
            image_id images[500];
            images[i++] = resources->ui_rays_gradient;
            images[i++] = resources->ui_chunky.top_left;
            images[i++] = resources->ui_chunky.top_right;
            images[i++] = resources->ui_chunky.bottom_left;
            images[i++] = resources->ui_chunky.bottom_right;
            images[i++] = resources->ui_chunky.left;
            images[i++] = resources->ui_chunky.right;
            images[i++] = resources->ui_chunky.bottom;
            images[i++] = resources->ui_chunky.top;
            images[i++] = resources->ui_chunky.center;

            images[i++] = resources->ui_chunky_outline.top_left;
            images[i++] = resources->ui_chunky_outline.top_right;
            images[i++] = resources->ui_chunky_outline.bottom_left;
            images[i++] = resources->ui_chunky_outline.bottom_right;
            images[i++] = resources->ui_chunky_outline.left;
            images[i++] = resources->ui_chunky_outline.right;
            images[i++] = resources->ui_chunky_outline.bottom;
            images[i++] = resources->ui_chunky_outline.top;
            images[i++] = resources->ui_chunky_outline.center;
            for (int j = 0; j < TROPHY_ICON_COUNT; ++j) {
                images[i++] = resources->trophies_locked[j];
            }
            for (int j = 0; j < TROPHY_ICON_COUNT; ++j) {
                images[i++] = resources->trophies_unlocked[j];
            }

            resources->ui_texture_atlas = graphics_assets_construct_texture_atlas_image(&resources->graphics_assets, images, i);
            graphics_assets_texture_atlas_unload_original_subimages(&resources->graphics_assets, resources->ui_texture_atlas);

            {
                {
                    auto& sprite = resources->locked_trophy_sprite;
                    sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, 8);
                    for (int frame_index = 0; frame_index < 5; ++frame_index) {
                        auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index);
                        frame->img = resources->ui_texture_atlas.atlas_image_id;
                        frame->source_rect = resources->ui_texture_atlas.get_subrect(resources->trophies_locked[frame_index+1]);
                        frame->frame_length = 0.15;
                    }
                    for (int frame_index = 0; frame_index < 3; ++frame_index) {
                        auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index+5);
                        frame->img = resources->ui_texture_atlas.atlas_image_id;
                        frame->source_rect = resources->ui_texture_atlas.get_subrect(resources->trophies_locked[0]);
                        frame->frame_length = 0.15;
                    }
                }
                {
                    auto& sprite = resources->unlocked_trophy_sprite;
                    sprite = graphics_assets_alloc_sprite(&resources->graphics_assets, 8);
                    for (int frame_index = 0; frame_index < 5; ++frame_index) {
                        auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index);
                        frame->img = resources->ui_texture_atlas.atlas_image_id;
                        frame->source_rect = resources->ui_texture_atlas.get_subrect(resources->trophies_unlocked[frame_index+1]);
                        frame->frame_length = 0.15;
                    }
                    for (int frame_index = 0; frame_index < 3; ++frame_index) {
                        auto frame = sprite_get_frame(graphics_get_sprite_by_id(&resources->graphics_assets, sprite), frame_index+5);
                        frame->img = resources->ui_texture_atlas.atlas_image_id;
                        frame->source_rect = resources->ui_texture_atlas.get_subrect(resources->trophies_unlocked[0]);
                        frame->frame_length = 0.15;
                    }
                }
                resources->locked_trophy_sprite_instance = sprite_instance(resources->locked_trophy_sprite);
                resources->unlocked_trophy_sprite_instance = sprite_instance(resources->unlocked_trophy_sprite);
            }
        }
    }

    // Try to build maingame atlas.
    {
        // NOTE: only for the packing algorithm
        s32 atlas_sprite_count =
            PET_IMAGE_SPRITE_FACING_DIRECTION_COUNT * GAME_PET_ID_COUNT +
            HERO_IMAGE_FRAME_COUNT + 1;
        _debugprintf("ATLAS IMAGES TO PACK %d", atlas_sprite_count);

        // count sprites...
        {
            for (s32 projectile_sprite_type = 0; projectile_sprite_type < PROJECTILE_SPRITE_TYPES; ++projectile_sprite_type) {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->projectile_sprites[projectile_sprite_type]
                );

                if (sprite_object) {
                    atlas_sprite_count += sprite_get_unique_image_count(sprite_object);
                }
            }

            for (s32 entity_sprite_type = 0; entity_sprite_type < ENTITY_SPRITE_TYPES; ++entity_sprite_type) {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->entity_sprites[entity_sprite_type]
                );

                if (sprite_object) {
                    atlas_sprite_count += sprite_get_unique_image_count(sprite_object);
                }
            }

            {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->circle_sprite
                );

                if (sprite_object) {
                    atlas_sprite_count += sprite_get_unique_image_count(sprite_object);
                }
            }

            {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->point_pickup_sprite
                );

                if (sprite_object) {
                    atlas_sprite_count += sprite_get_unique_image_count(sprite_object);
                }
            }
        }

        _debugprintf("ATLAS IMAGES TO PACK %d", atlas_sprite_count);
        image_id* images =  
            (image_id*)Global_Engine()->scratch_arena.push_unaligned(sizeof(*images) * atlas_sprite_count);

        {
            s32 cursor = 0;
            {
                images[cursor++] = resources->circle; // lol
                
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->circle_sprite
                );

                if (sprite_object) {
                    cursor += sprite_copy_all_images_into_image_array(
                        sprite_object, images + cursor, atlas_sprite_count - cursor);
                }
            }

            {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->circle_sprite16
                );

                if (sprite_object) {
                    cursor += sprite_copy_all_images_into_image_array(
                        sprite_object, images + cursor, atlas_sprite_count - cursor);
                }
            }

            for (s32 projectile_sprite_type = 0; projectile_sprite_type < PROJECTILE_SPRITE_TYPES; ++projectile_sprite_type) {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->projectile_sprites[projectile_sprite_type]
                );

                if (sprite_object) {
                    cursor += sprite_copy_all_images_into_image_array(
                        sprite_object, images + cursor, atlas_sprite_count - cursor);
                }
            }

            for (s32 entity_sprite_type = 0; entity_sprite_type < ENTITY_SPRITE_TYPES; ++entity_sprite_type) {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->entity_sprites[entity_sprite_type]
                );

                if (sprite_object) {
                    cursor += sprite_copy_all_images_into_image_array(
                        sprite_object, images + cursor, atlas_sprite_count - cursor);
                }
            }

            {
                auto sprite_object = graphics_get_sprite_by_id(
                    &resources->graphics_assets,
                    resources->point_pickup_sprite
                );

                if (sprite_object) {
                    cursor += sprite_copy_all_images_into_image_array(
                        sprite_object, images + cursor, atlas_sprite_count - cursor);
                }
            }

            for (s32 pet_image_direction = 0; pet_image_direction < 4; ++pet_image_direction) {
                for (s32 pet_image_type = 0; pet_image_type < GAME_PET_ID_COUNT; ++pet_image_type) {
                    images[cursor++] = resources->pet_images[pet_image_type][pet_image_direction];
                }
            }

            for (s32 hero_image_index = 0; hero_image_index < HERO_IMAGE_FRAME_COUNT; ++hero_image_index) {
                images[cursor++] = resources->hero_images[hero_image_index];
            }

            resources->gameplay_texture_atlas =
                graphics_assets_construct_texture_atlas_image(&resources->graphics_assets,
                                                              images,
                                                              cursor);
        }

        // REMOVE when this is confirmed to actually work!
        // graphics_assets_texture_atlas_unload_original_subimages(&resources->graphics_assets, resources->ui_texture_atlas);
    }
#endif
}

void Game::init_audio_resources() {
    if (!initialized) {
        return;
    }

    auto resources = state->resources;
    resources->attack_sounds[0] = Audio::load(("res/snds/fire1.wav"));
    resources->attack_sounds[1] = Audio::load(("res/snds/fire2.wav"));
    resources->attack_sounds[2] = Audio::load(("res/snds/fire3.wav"));
    resources->attack_sounds[3] = Audio::load(("res/snds/fire4.wav"));

    resources->explosion_sounds[0] = Audio::load("res/snds/lightning1.wav");
    resources->explosion_sounds[1] = Audio::load("res/snds/lightning2.wav");
    resources->explosion_sounds[2] = Audio::load("res/snds/lightning3.wav");
    resources->explosion_sounds[3] = Audio::load("res/snds/lightning4.wav");

    resources->opening_beep_type = Audio::load("res/snds/beep_type.wav");

    resources->hit_sounds[0]    = Audio::load(("res/snds/hit1.wav"));
    resources->hit_sounds[1]    = Audio::load(("res/snds/hit2.wav"));

    resources->death_sound      = Audio::load("res/snds/die1.wav");

    resources->title_music = Audio::load("res/snds/titlemenu1draft.ogg", true);
    resources->title_reprisal_music = Audio::load("res/snds/titlemenu1reprise.ogg", true);
    resources->score_pickup_sound = Audio::load("res/snds/scorepickup.wav", false);
}


// NOTE: as part of the replay system, I want to be able to scroll arbitrarily
// and I would rather resimulate everything vs. storing snapshots as with lua as my VM
// it's impossible to access a program counter or a similar construct. So I can't even use a snapshot
// even if I wanted to :)
// So this is here to "refresh" the simulation state
// by reloading and resetting all the appropriate state.
void Game::reset_stage_simulation_state() {
    auto state = &this->state->gameplay_data;
    s32 stage_id = this->state->mainmenu_data.stage_id_level_select;
    s32 level_id = this->state->mainmenu_data.stage_id_level_in_stage_select;
    // s32 pet_id   = this->state->

    state->main_camera.trauma = 0;
    this->state->deathscreen_data.reset();
    {
        auto& deathanimation_data       = this->state->deathanimation_data;
        deathanimation_data.phase       = DEATH_ANIMATION_PHASE_INACTIVE;
        deathanimation_data.t           = 0;
        deathanimation_data.flash_t     = 0;
        deathanimation_data.flash_count = 0;
        deathanimation_data.flashing    = 0;
        auto& deathanimation_emitter = deathanimation_data.player_explosion_emitter;

        deathanimation_emitter.reset();
        deathanimation_emitter.sprite             = 
            sprite_instance(resources->projectile_sprites[PROJECTILE_SPRITE_BLUE_STROBING]);
        deathanimation_emitter.sprite.scale       = V2(1.0f, 1.0f);
        deathanimation_emitter.modulation         = color32f32(1.0f, 1.0f, 1.0f, 1.0f);
        deathanimation_emitter.lifetime           = 1.25f;
        deathanimation_emitter.max_emissions      = 5;
        deathanimation_emitter.emission_max_timer = (DEATH_ANIMATION_MAX_T_PER_FLASH)*2;
        deathanimation_emitter.scale              = 1;
        deathanimation_emitter.emit_per_emission  = 16;
        deathanimation_emitter.flags = PARTICLE_EMITTER_FLAGS_USE_ANGULAR;
        deathanimation_emitter.angle_range        = V2(-360, 360);
        deathanimation_emitter.velocity           = V2(250, 0.0f);
        deathanimation_emitter.acceleration       = V2(0, 0.0f);
        deathanimation_emitter.scale_variance     = V2(-0.005, 0.005);
    }

    this->state->dialogue_state.in_conversation = false;
    // NOTE: need to save this to the savefile data.
    s32 pet_id   = state->selected_pet;

    {
        assertion(stage_id >= 0 && level_id >= 0 && "Something bad happened");
        _debugprintf("%d, %d", stage_id, level_id);
        // NOTE: check the ids later.
        if (!state->recording.in_playback) {
            game_register_stage_attempt(stage_id, level_id);
        }
        state->unload_all_script_loaded_resources(this->state, this->state->resources);
        
        state->stage_state = stage_load_from_lua(this->state, format_temp("stages/%d_%d.lua", stage_id+1, level_id+1));
        state->current_stage_timer = 0.0f;
        this->state->coroutine_tasks.add_task(this->state, state->stage_state.tick_task);
    }

    state->queue_bomb_use = false;

    // reset UID to 0.
    UID::reset();

    {
        state->disable_bullet_to_points = false;
        state->disable_grazing          = false;
        state->disable_enemy_to_points  = false;
        
        // Reset the player
        state->player.position                         = state->player.last_position = V2(state->play_area.width / 2, 300);
        state->pet.position                            = state->pet.last_position = state->player.position;
        state->player.visible                          = true;
        state->player.hp                               = state->player.max_hp = 1;
        state->player.die                              = false;
        state->player_damage_level_taken               = 1;
        state->paused_from_death                       = false;
        state->player.t_since_spawn                    = 0;
        state->player.trail_ghost_count                = 0;
        state->player.scale                            = V2(2, 2);
        state->player.end_invincibility();
        state->player.grazing_award_timer              = 0.0f;
        state->player.grazing_award_score_pickup_timer = 0.0f;
        state->player.reset_burst_charge_status();
        state->player.time_spent_grazing               = 0.0f;
        state->player.under_focus                      = false;
        state->show_damage_player_will_take            = false;
        state->game_alert.enabled = false;
        state->focus_tint_fade_t                       = 0.0f;
        state->focus_hitbox_fade_t                     = 0.0f;
        {
            state->player.sprite   = sprite_instance(resources->player_sprite);
        }
    }

    state->play_area.set_all_edge_behaviors_to(PLAY_AREA_EDGE_BLOCKING);
    state->play_area.edge_behavior_top    = PLAY_AREA_EDGE_WRAPPING;
    state->play_area.edge_behavior_bottom = PLAY_AREA_EDGE_WRAPPING;
    state->allow_border_switch_flashing = true;
    state->border_stop_all_notifications();
    
    state->boss_health_displays.displays.zero();

    state->pickups.clear();
    state->enemies.clear();
    state->bullets.clear();
    state->explosion_hazards.clear();
    state->laser_hazards.clear();
    state->score_notifications.clear();
    state->scriptable_render_objects.clear();
    state->simple_scrollable_backgrounds.clear();
    state->hit_score_notifications.clear();
    state->particle_pool.clear();
    state->death_particle_pool.clear();
    state->stage_exit_particle_pool.clear();
    state->ui_particle_pool.clear();
    state->particle_emitters.clear();
    state->ui_particle_emitters.clear();

    state->to_create_enemy_bullets.zero();
    state->to_create_player_bullets.zero();
    state->to_create_enemies.zero();
    state->to_create_pickups.zero();

    {
        auto pet_data = game_get_pet_data(pet_id);
        state->tries = pet_data->maximum_lives;
        state->max_tries = pet_data->maximum_lives;
    }

    state->death_count = 0;
    state->burst_usage_count = 0;
    state->total_score = 0;
    state->current_score = 0;
    state->score_awarded_points = 0;
    state->extra_life_awarded_count = 0;
    state->paused_from_death = false;
    state->current_stage_timer = 0.0f;
    state->started_system_time = system_get_current_time();
    state->triggered_stage_completion_cutscene = false;
    state->queue_bomb_use = false;

    // setup recording file for recording or playback.
    {
        if (!state->recording.in_playback) {
            gameplay_recording_file_start_recording(
                &state->recording,
                state->prng,
                &Global_Engine()->main_arena
            );
            state->recording.stage_id = stage_id;
            state->recording.level_id = level_id;
            state->recording.selected_pet = pet_id;
        } else {
            state->recording.old_prng = state->prng;
            state->prng               = state->recording.prng;
        }
    }
    state->fixed_tickrate_timer = 0;
    state->fixed_tickrate_remainder = 0;
}

void Game::cleanup_game_simulation() {
    state->gameplay_data.unload_all_script_loaded_resources(state, this->state->resources);

    if (state->gameplay_data.recording.in_playback) {
        _debugprintf("Finished playback.");
        _debugprintf(
            "Set OldPRNG to (%d, %d, %d, %d, %d)",
            state->gameplay_data.recording.old_prng.constant,
            state->gameplay_data.recording.old_prng.multiplier,
            state->gameplay_data.recording.old_prng.state,
            state->gameplay_data.recording.old_prng.seed,
            state->gameplay_data.recording.old_prng.modulus
        );
        state->gameplay_data.recording.in_playback = false;
        state->gameplay_data.prng                  = state->gameplay_data.recording.old_prng;
        _debugprintf(
            "Set State->PRNG to (%d, %d, %d, %d, %d)",
            state->gameplay_data.prng.constant,
            state->gameplay_data.prng.multiplier,
            state->gameplay_data.prng.state,
            state->gameplay_data.prng.seed,
            state->gameplay_data.prng.modulus
        );
        gameplay_recording_file_finish(&state->gameplay_data.recording);
    }
}

void Game::setup_stage_start() {
    reset_stage_simulation_state();

    auto state = &this->state->gameplay_data;
    state->triggered_stage_completion_cutscene = false;
    state->intro.begin_sequence();
    state->complete_stage.reset();
}

void Game::handle_preferences(void) {
    if (load_preferences_from_disk(&preferences, string_literal("preferences.lua"))) {
    }
    else {
        // the main code will provide us with a default
        // preferences struct.
        save_preferences_to_disk(&preferences, string_literal("preferences.lua"));
    }
    update_preferences(&temp_preferences, &preferences);
    confirm_preferences(&preferences, resources);
}

void Game::on_resolution_change(int new_width, int new_height) {
    _debugprintf("on_resolution_change");
}

void Game::init(Graphics_Driver* driver) {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state = (Game_State*)arena->push_unaligned(sizeof(*this->state)); (new (this->state) Game_State);
    this->state->resources = resources;

    VFS_mount_archive(string_literal("./data.bigfile"));
    // setup input
    {
        Action::register_action_keys(ACTION_MOVE_UP, KEY_UP, KEY_W, -1.0f);
        Action::register_action_button(ACTION_MOVE_UP, DPAD_UP, -1.0f);
        Action::register_action_joystick_axis(ACTION_MOVE_UP, CONTROLLER_JOYSTICK_LEFT, GAMEPAD_AXIS_POSITIVE_Y);

        Action::register_action_keys(ACTION_MOVE_DOWN, KEY_DOWN, KEY_S, 1.0f);
        Action::register_action_button(ACTION_MOVE_DOWN, DPAD_DOWN, 1.0f);
        Action::register_action_joystick_axis(ACTION_MOVE_DOWN, CONTROLLER_JOYSTICK_LEFT, GAMEPAD_AXIS_NEGATIVE_Y);

        Action::register_action_keys(ACTION_MOVE_LEFT, KEY_LEFT, KEY_A, -1.0f);
        Action::register_action_button(ACTION_MOVE_LEFT, DPAD_LEFT, -1.0f);
        Action::register_action_joystick_axis(ACTION_MOVE_LEFT, CONTROLLER_JOYSTICK_LEFT, GAMEPAD_AXIS_NEGATIVE_X);

        Action::register_action_keys(ACTION_MOVE_RIGHT, KEY_RIGHT, KEY_D, 1.0f);
        Action::register_action_button(ACTION_MOVE_RIGHT, DPAD_RIGHT, 1.0f);
        Action::register_action_joystick_axis(ACTION_MOVE_RIGHT, CONTROLLER_JOYSTICK_LEFT, GAMEPAD_AXIS_POSITIVE_X);

        Action::register_action_keys(ACTION_ACTION, KEY_SPACE, KEY_RETURN, 1.0f);
        Action::register_action_button(ACTION_ACTION, BUTTON_A, 1.0f);

        Action::register_action_keys(ACTION_FOCUS, KEY_SHIFT, KEY_CTRL, 1.0f);
        Action::register_action_button(ACTION_FOCUS, BUTTON_X, 1.0f);

        Action::register_action_keys(ACTION_CANCEL, KEY_BACKSPACE, KEY_UNKNOWN, 1.0f);
        Action::register_action_button(ACTION_CANCEL, BUTTON_B, 1.0f);

        Action::register_action_keys(ACTION_MENU, KEY_ESCAPE, KEY_UNKNOWN, 1.0f);
        Action::register_action_button(ACTION_MENU, BUTTON_START, 1.0f);

        Action::register_action_keys(ACTION_USE_BOMB, KEY_Z, KEY_UNKNOWN, 1.0f);
        Action::register_action_button(ACTION_USE_BOMB, BUTTON_Y, 1.0f);

        Action::register_action_keys(ACTION_SCREENSHOT, KEY_F9, KEY_UNKNOWN, 1.0f);
    }

    if (!Action::load(string_literal("controls.lua"))) {
        Action::save(string_literal("controls.lua"));
    } else {
        _debugprintf("Loaded control from file");
    }

    resources->graphics_assets   = graphics_assets_create(arena, 16, 512, 512);
    
    // initialize achievement notifier
    {
        achievement_state.notifications = Fixed_Array<Achievement_Notification>(arena, ACHIEVEMENT_ID_COUNT);
    }

    // Initialize coroutine scheduler
    {
        s32 task_max_count = MAX_BULLETS + MAX_ENEMIES + 512;
        state->coroutine_tasks = Game_Task_Scheduler(arena, task_max_count);
    }

    initialized = true;

    init_graphics_resources(driver);
    init_audio_resources();

    // gameplay_data initialize
    {
        auto state                     = &this->state->gameplay_data;
        state->bullets                 = Fixed_Array<Bullet>(arena, MAX_BULLETS);
        state->explosion_hazards       = Fixed_Array<Explosion_Hazard>(arena, MAX_EXPLOSION_HAZARDS);
        state->laser_hazards           = Fixed_Array<Laser_Hazard>(arena, MAX_LASER_HAZARDS);
        state->enemies                 = Fixed_Array<Enemy_Entity>(arena, MAX_ENEMIES);
        state->death_explosions        = Fixed_Array<DeathExplosion>(arena, MAX_ENEMIES);
        state->pickups                 = Fixed_Array<Pickup_Entity>(arena, MAX_PICKUP_ENTITIES);
        state->score_notifications     = Fixed_Array<Gameplay_UI_Score_Notification>(arena, MAX_SCORE_NOTIFICATIONS);
        state->hit_score_notifications = Fixed_Array<Gameplay_UI_Hitmark_Score_Notification>(arena, MAX_SCORE_NOTIFICATIONS);
        state->particle_emitters       = Fixed_Array<Particle_Emitter>(arena, MAX_BULLETS + MAX_ENEMIES + MAX_LASER_HAZARDS + MAX_EXPLOSION_HAZARDS + 128);
        state->ui_particle_emitters    = Fixed_Array<Particle_Emitter>(arena, 128);
        state->prng                    = random_state();
        state->prng_unessential        = random_state();
        state->main_camera             = camera(V2(0, 0), 1.0);
        state->main_camera.rng         = &state->prng;
        state->particle_pool.init(arena, 16384);
        state->death_particle_pool.init(arena, 128);
        state->stage_exit_particle_pool.init(arena, 256);
        state->ui_particle_pool.init(arena, 3500);

        // creation queues
        {
            // It's easier to just use more memory, and then remerge at the end...
            state->to_create_player_bullets = Fixed_Array<Bullet>(arena, MAX_BULLETS);
            state->to_create_enemy_bullets  = Fixed_Array<Bullet>(arena, MAX_BULLETS);
            state->to_create_enemies        = Fixed_Array<Enemy_Entity>(arena, MAX_ENEMIES);
            state->to_create_pickups        = Fixed_Array<Pickup_Entity>(arena, MAX_PICKUP_ENTITIES);
        }

        state->boss_health_displays.displays = Fixed_Array<Boss_Healthbar_Display>(arena, 16);
        state->scriptable_render_objects     = Fixed_Array<Scriptable_Render_Object>(arena, MAX_SCRIPTABLE_RENDER_OBJECTS);
        state->simple_scrollable_backgrounds = Simple_Scrollable_Background_Entities(arena, MAX_BACKGROUND_ENTITIES);
        state->script_loaded_images          = Fixed_Array<image_id>(arena, MAX_TRACKED_SCRIPT_LOADABLE_IMAGES);
        state->script_loaded_sounds          = Fixed_Array<Audio::Sound_ID>(arena, MAX_TRACKED_SCRIPT_LOADABLE_SOUNDS);
    }


    /*
     * NOTE: all state initialization data is here for now
     * since it's easier for me to just add to this,
     *
     * but most certainly moving these to their own file might make more sense.
     */
    // mainmenu_data initialize
    mainmenu_data_initialize(driver);

    // opening_data initialize
    opening_data_initialize(driver);

    // ending_data initialize
    ending_data_initialize(driver);

    // title screen initialize
    title_data_initialize(driver);

    // trailer cutaway initialize
    trailer_cutaway_data_initialize(driver);

    Achievements::init_achievements(arena, make_slice<Achievement>(achievement_list, array_count(achievement_list)));
    GameUI::initialize(arena);

    // NOTE: loading save game here so that way the achievement menu
    // is correctly synched with the savefile (since achievements are stored with the save file)
    //
    // The game will actually load properly in the title screen "PLAY button".
    {
        load_game();
    }
}

void Game::deinit() {
    graphics_assets_finish(&resources->graphics_assets);
    VFS_finish();
}

// Scriptable_Render_Object 
void Scriptable_Render_Object::render(Game_Resources* resources, struct render_commands* render_commands) {
    auto destination_rect = rectangle_f32(position.x, position.y, scale.x, scale.y);

    if (image_id.index == 0) {
        render_commands_push_quad_ext2(
            render_commands,
            destination_rect,
            modulation,
            rotation_center,
            z_angle,
            y_angle,
            // unused angle_x
            // blend mode param not used
            BLEND_MODE_ALPHA
        );
    } else {
        render_commands_push_image_ext2(
            render_commands,
            graphics_assets_get_image_by_id(&resources->graphics_assets, image_id),
            destination_rect,
            src_rect,
            color32u8_to_color32f32(modulation),
            rotation_center,
            z_angle,
            y_angle,
            // unused angle_x
            0, // flags param not used
            // blend mode param not used
            BLEND_MODE_ALPHA
        );
    }
}

// Gameplay_Data
void Gameplay_Data::border_notify(s32 id, s32 type, bool override) {
    if (id < 0 || id >= PLAY_AREA_EDGE_ID_COUNT) {
        return;
    }

    auto& border_flash = border_flashes[id];

    if (!override && (border_flash.flash_id_type != BORDER_FLASH_ID_TYPE_NONE)) {
        return;
    }

    border_flash.flash_id_type       = type;
    border_flash.delay_between_flash = false;
    border_flash.flash_count         = BORDER_NOTIFY_FLASH_COUNT;
    border_flash.per_flash_length    = BORDER_NOTIFY_PER_FLASH_LENGTH; // seconds
}

void Gameplay_Data::border_stop_notify(s32 id) {
    if (id < 0 || id >= PLAY_AREA_EDGE_ID_COUNT) {
        return;
    }

    auto& border_flash = border_flashes[id];
    border_flash.flash_id_type = BORDER_FLASH_ID_TYPE_NONE;
}

void Gameplay_Data::border_stop_all_notifications(void) {
    for (s32 border_index = 0; border_index < PLAY_AREA_EDGE_ID_COUNT; ++border_index) {
        border_stop_notify(border_index);
    }
}

// NOTE(jerry):
// never called automatically.
void Gameplay_Data::border_notify_current_status(void) {
    for (s32 border_index = 0; border_index < PLAY_AREA_EDGE_ID_COUNT; ++border_index) {
        border_notify(border_index, play_area.edge_behaviors[border_index]+1, true);
    }
}

void Gameplay_Data::set_pet_id(s8 id, Game_Resources* resources) {
    selected_pet = id;
    pet.set_id(id, resources);
}

void Gameplay_Data::remove_life(void) {
    stage_perfect_clear    = false;
    campaign_perfect_clear = false;

    if (tries > 0) {
        auto& ui_req = ui_particle_spawn_queue[ui_particle_spawn_queue_count++];
        assertion(ui_particle_spawn_queue_count < array_count(ui_particle_spawn_queue) && "[crash, out of fixed space]");
        ui_req.type = GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_LOST_LIFE;
        ui_req.data = tries;
        tries -= 1;
    }
}

void Gameplay_Data::add_life(void) {
    if (tries < MAX_TRIES_ALLOWED) {
        tries += 1;
        auto& ui_req = ui_particle_spawn_queue[ui_particle_spawn_queue_count++];
        assertion(ui_particle_spawn_queue_count < array_count(ui_particle_spawn_queue) && "[crash, out of fixed space]");
        ui_req.type = GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_GAINED_LIFE;
        ui_req.data = tries;

        if (tries > max_tries) {
          max_tries = tries;
        }
    }
}

void Gameplay_Data::unload_all_dialogue_loaded_resources(Game_State* state, Game_Resources* resources) {
    _debugprintf("Unloading all dialogue resources");
    {
        auto& dialogue_state = state->dialogue_state;
        for (s32 tracked_image_index = 0;
             tracked_image_index < dialogue_state.tracked_image_count;
             ++tracked_image_index) {
            graphics_assets_unload_image(&state->resources->graphics_assets, dialogue_state.tracked_images[tracked_image_index]);
            dialogue_state.tracked_images[tracked_image_index].index = 0;
        }
    }
}

void Gameplay_Data::unload_all_script_loaded_resources(Game_State* game_state, Game_Resources* resources) {
    _debugprintf("Unloading level-specific resources");
    unload_all_dialogue_loaded_resources(game_state, resources);
    for (s32 image_index = 0; image_index < script_loaded_images.size; ++image_index) {
        auto img_id = script_loaded_images[image_index];
        graphics_assets_unload_image(&resources->graphics_assets, img_id);
    }

    Audio::stop_sounds();
    Audio::stop_music(); // should stop all audio
    for (s32 sound_index = 0; sound_index < script_loaded_sounds.size; ++sound_index) {
        auto snd_id = script_loaded_sounds[sound_index];
        Audio::unload(snd_id);
    }

    script_loaded_images.zero();
    script_loaded_sounds.zero();

    // also a script loaded resource... is also the
    // script...
    {
        auto state = this;
        if (state->stage_state.L) {
            // levels allocate from the top of the "stack"
            Global_Engine()->main_arena.clear_top();
            game_state->coroutine_tasks.abort_all_lua_tasks();
            game_state->coroutine_tasks.tasks.zero();
            game_state->coroutine_tasks.active_task_ids.zero();
            assertion(state->stage_state.L && "This state should not be null?");
            lua_close(state->stage_state.L);
            game_state->coroutine_tasks.L = nullptr;
            game_state->gameplay_data.scriptable_render_objects.zero();
            state->stage_state.L = nullptr;
        }
    }
}

void Gameplay_Data::add_bullet(Bullet b) {
    switch (b.source_type) {
        case BULLET_SOURCE_PLAYER: {
            to_create_player_bullets.push(b);
        } break;
        default: {
            to_create_enemy_bullets.push(b);
        } break;
    }
}

void Gameplay_Data::add_laser_hazard(Laser_Hazard h) {
    laser_hazards.push(h); 
}

void Gameplay_Data::add_explosion_hazard(Explosion_Hazard h) {
    explosion_hazards.push(h);
}

void Gameplay_Data::add_enemy_entity(Enemy_Entity e) {
    to_create_enemies.push(e);
}

void Gameplay_Data::add_pickup_entity(Pickup_Entity s) {
    to_create_pickups.push(s);
}

bool Gameplay_Data::entity_spawned(u64 uid) {
    for (s32 index = 0; index < enemies.size; ++index) {
        if (enemies[index].uid == uid) return true;
    }
    return false;
}

bool Gameplay_Data::bullet_spawned(u64 uid) {
    for (s32 index = 0; index < bullets.size; ++index) {
        if (bullets[index].uid == uid) return true;
    }
    return false;
}

Enemy_Entity* Gameplay_Data::lookup_enemy(u64 uid) {
    for (s32 index = 0; index < to_create_enemies.size; ++index) {
        if (to_create_enemies[index].uid == uid) return &to_create_enemies[index];
    }
    for (s32 index = 0; index < enemies.size; ++index) {
        if (enemies[index].uid == uid) return &enemies[index];
    }
    return nullptr;
}

Bullet* Gameplay_Data::lookup_bullet(u64 uid) {
    for (s32 index = 0; index < to_create_enemy_bullets.size; ++index) {
        if (to_create_enemy_bullets[index].uid == uid) return &to_create_enemy_bullets[index];
    }
    for (s32 index = 0; index < to_create_player_bullets.size; ++index) {
        if (to_create_player_bullets[index].uid == uid) return &to_create_player_bullets[index];
    }
    for (s32 index = 0; index < bullets.size; ++index) {
        if (bullets[index].uid == uid) return &bullets[index];
    }
    return nullptr;
}

void Gameplay_Data::reify_all_creation_queues() {
#if 1
    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*)ctx;

            for (int i = 0; i < (int)state->to_create_enemy_bullets.size; ++i) {
                auto& b = state->to_create_enemy_bullets[i];
                state->bullets.push(b);
            }

            for (int i = 0; i < (int)state->to_create_player_bullets.size; ++i) {
                auto& b = state->to_create_player_bullets[i];
                state->bullets.push(b);
            }

            state->to_create_player_bullets.zero();
            state->to_create_enemy_bullets.zero();
            return 0;
        },
        this
    );

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*)ctx;

            for (int i = 0; i < (int)state->to_create_pickups.size; ++i) {
                auto& e = state->to_create_pickups[i];
                state->pickups.push(e);
            }

            state->to_create_pickups.zero();
            return 0;
        },
        this
    );

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*)ctx;

            for (int i = 0; i < (int)state->to_create_enemies.size; ++i) {
                auto& e = state->to_create_enemies[i];
                assert(e.hp > 0 && "A dead enemy being created? This must be a bug!");
                assert(!e.die && "An enemy shouldn't be dead on spawn?");
                state->enemies.push(e);
            }

            state->to_create_enemies.zero();
            return 0;
        },
        this
    );

    Thread_Pool::synchronize_tasks();
#else
    auto state = this;
    for (int i = 0; i < (int)state->to_create_enemy_bullets.size; ++i) {
        auto& b = state->to_create_enemy_bullets[i];
        state->bullets.push(b);
    }

    for (int i = 0; i < (int)state->to_create_player_bullets.size; ++i) {
        auto& b = state->to_create_player_bullets[i];
        state->bullets.push(b);
    }

    for (int i = 0; i < (int)state->to_create_pickups.size; ++i) {
        auto& e = state->to_create_pickups[i];
        state->pickups.push(e);
    }

    state->to_create_pickups.zero();


    state->to_create_player_bullets.zero();
    state->to_create_enemy_bullets.zero();

    for (int i = 0; i < (int)state->to_create_enemies.size; ++i) {
        auto& e = state->to_create_enemies[i];
        assert(e.hp > 0 && "A dead enemy being created? This must be a bug!");
        assert(!e.die && "An enemy shouldn't be dead on spawn?");
        state->enemies.push(e);
    }

    state->to_create_enemies.zero();
#endif
}

bool Gameplay_Data::any_hazards() const {
    return (laser_hazards.size > 0) || (explosion_hazards.size > 0);
}

bool Gameplay_Data::any_enemies() const {
    // NOTE: queued entities will also count.
    //_debugprintf("eeee %d, %d\n", enemies.size, to_create_enemies.size);
    return (enemies.size > 0) || (to_create_enemies.size > 0);
}

bool Gameplay_Data::any_bullets() const {
    // NOTE: queued entities will also count.
    return (bullets.size > 0) || (to_create_enemy_bullets.size > 0);
}

bool Gameplay_Data::any_living_danger() const {
    return any_enemies() || any_hazards();
}

void Gameplay_Data::notify_score(s32 amount, bool interesting) {
    auto pet_data = game_get_pet_data(selected_pet);
    current_score += amount * pet_data->score_modifier;

    s32 score_delta = current_score - score_awarded_points;
    
    // First 3 extra lives are generally pretty easy to obtain,
    // everything else is a bit harder.
    local s32 extra_life_score_table[] = {
      POINTS_TO_AWARD_EXTRA_LIFE,
      POINTS_TO_AWARD_EXTRA_LIFE * 1.25,
      POINTS_TO_AWARD_EXTRA_LIFE * 1.50,
      POINTS_TO_AWARD_EXTRA_LIFE * 1.50,
      POINTS_TO_AWARD_EXTRA_LIFE * 1.75,
      POINTS_TO_AWARD_EXTRA_LIFE * 2.0,
    };

    if (score_delta >= extra_life_score_table[extra_life_awarded_count++]) {
        add_life();
        score_awarded_points = current_score;
    }

    if (extra_life_awarded_count >= array_count(extra_life_score_table)) {
      extra_life_awarded_count = array_count(extra_life_score_table) - 1;
    }

    if (!interesting)
        return;

    Gameplay_UI_Score_Notification notification;
    notification.additional_score = amount;
    notification.lifetime.start();
    score_notifications.push(notification);
}

void Gameplay_Data::notify_score_with_hitmarker(s32 amount, V2 where) {
    notify_score(amount, true);
    Gameplay_UI_Hitmark_Score_Notification notification;
    notification.where = where;
    notification.additional_score = amount;
    notification.lifetime.start();
    hit_score_notifications.push(notification);
}

void Gameplay_Data::add_scriptable_render_object(Scriptable_Render_Object ro) {
    scriptable_render_objects.push(ro);
}

//////////// TODO(jerry):
/*
 * I really, really, really hate this set up here, I should've been using a proper layering
 * system so that way I didn't need to functions to do this stupid thing!
 */
void Gameplay_Data::update_and_render_all_background_scriptable_render_objects(Game_Resources* resources, struct render_commands* render_commands, f32 dt) {
    for (s32 index = 0; index < scriptable_render_objects.size; ++index) {
        auto& render_object = scriptable_render_objects[index];
        if (render_object.layer != SCRIPTABLE_RENDER_OBJECT_LAYER_BACKGROUND) continue;
        render_object.render(resources, render_commands);
    }
}

void Gameplay_Data::update_and_render_all_foreground_scriptable_render_objects(Game_Resources* resources, struct render_commands* render_commands, f32 dt) {
    for (s32 index = 0; index < scriptable_render_objects.size; ++index) {
        auto& render_object = scriptable_render_objects[index];
        if (render_object.layer != SCRIPTABLE_RENDER_OBJECT_LAYER_FOREGROUND) continue;
        render_object.render(resources, render_commands);
    }
}

local void draw_outlined_box(struct render_commands* render_commands, rectangle_f32 where, union color32u8 color, f32 thickness) {
    // Left
    {
        auto r = rectangle_f32(
            where.x-thickness/2, where.y,
            thickness, where.h
        );
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color,
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );
    }
    // Right
    {
        auto r = rectangle_f32(
            where.x+where.w-thickness/2, where.y,
            thickness, where.h
        );
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color,
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );
    }
    // Top
    {
        auto r = rectangle_f32(
            where.x, where.y-thickness/2,
            where.w, thickness
        );
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color,
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );
    }
    // Bottom
    {
        auto r = rectangle_f32(
            where.x, where.y+where.h-thickness/2,
            where.w, thickness
        );
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color,
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );
    }
}

void Gameplay_Data::update_and_render_focus_mode_hitboxes(Game_State* state, struct render_commands* render_commands, Game_Resources* resources, f32 dt) {
    /*
      NOTE(jerry): 
      It *would* be pretty nice to do a fancier animation than a fade in, but
      it would be in a bad taste (and also more complicated since there's already
      a lot of elements that are animated in code for the game...)
    */

    bool should_try_to_draw = !f32_close_enough(focus_hitbox_fade_t, 0.0f);
    s32  burst_rank         = player.get_burst_rank();

    {
        const f32 HITBOX_FADE_T_CHARGE_MOD = 8;
        const f32 HITBOX_FADE_T_DISCHARGE_MOD = 0.55; // very prolonged decay, since burst/focus doesn't tend to last long.

        if (player.under_focus) {
          if (burst_rank >= 2) {
            focus_hitbox_fade_t += dt * HITBOX_FADE_T_CHARGE_MOD;
          } else {
            focus_hitbox_fade_t += dt * HITBOX_FADE_T_CHARGE_MOD/2; // on low ranks, expire quickly, but don't make it impossible to see stuff.

            if (focus_hitbox_fade_t >= 0.5f) {
              focus_hitbox_fade_t = 0.5f;
            }
          }
        } else {
            focus_hitbox_fade_t -= dt*HITBOX_FADE_T_DISCHARGE_MOD;
        }

        focus_hitbox_fade_t =
            clamp<f32>(
                focus_hitbox_fade_t,
                0.0f, 1.0f
            );
    }

    if (should_try_to_draw) {
        /*
         * TODO(jerry): perf is probably not
         * going to be very acceptable like this,
         * since I'm already pushing stuff due to how *relatively*
         * complex and per-frame some of the lua code is (and it's being forced to run at 60fps!)
         */
        // NearbyEnemyHitboxes
        const f32 CLOSE_ENOUGH_DISTANCE_THRESHOLD = 350; // pixels
        const f32 CLOSE_ENOUGH_DISTANCE_ALPHA_DIV = 180; // NOTE(jerry): lower so that we have a buffer
                                                         // zone of hard solid pixels.
        const f32 BOX_THICKNESS = 2.0f;
        const auto color = color32u8(255, 0, 0, focus_hitbox_fade_t*255); // is there a better color? I'd like to know.
        /*
         * NOTE(jerry):
         * Squared distance doesn't work, I need a linear fade-in, and I don't know
         * if linearizing it will look correct.
         */

        // NearbyEnemyHitboxes
        {
            for (s32 index = 0; index < (s32)enemies.size; ++index) {
                auto& e        = enemies[index];
                f32   distance = V2_distance(e.get_real_position(), player.get_real_position());
                f32   alpha    = clamp<f32>(1.0f - (distance / CLOSE_ENOUGH_DISTANCE_ALPHA_DIV), 0.0f, 1.0f) * focus_hitbox_fade_t;
                auto r = e.get_rect();
                draw_outlined_box(render_commands, r, color32u8(color.r,color.g,color.b, 255*alpha), BOX_THICKNESS); 
            }
        }
        // NearbyBulletHitboxes
        {
            for (s32 index = 0; index < (s32)bullets.size; ++index) {
                auto& b = bullets[index];
                if (b.source_type == BULLET_SOURCE_PLAYER) {
                    continue;
                }

                f32   distance = V2_distance(b.get_real_position(), player.get_real_position());
                f32   alpha    = clamp<f32>(1.0f - (distance / CLOSE_ENOUGH_DISTANCE_ALPHA_DIV), 0.0f, 1.0f) * focus_hitbox_fade_t;
                auto r = b.get_rect();
                draw_outlined_box(render_commands, r, color32u8(color.r,color.g,color.b, 255*alpha), BOX_THICKNESS); 
            }
        }
        // PlayerHitbox
        {
            auto player_rectangle = player.get_rect();
            auto r = player_rectangle;
            draw_outlined_box(render_commands, r, color, BOX_THICKNESS); 
        }
    }
}
// End of Gameplay_Data

bool Game::can_access_stage(s32 id) {
    auto state = &this->state->mainmenu_data;
    auto& stage_portal = state->portals[id];

    for (int i = 0; i < array_count(stage_portal.prerequisites); ++i) {
        if (stage_portal.prerequisites[i] == -1) {
            // _debugprintf("Null prerequisite.");
        } else {
            s32 prerequisite_index = stage_portal.prerequisites[i];
            auto& stage = stage_list[prerequisite_index];

            // Refer to 'game_complete_stage_level'
            if (stage.unlocked_levels <= MAX_LEVELS_PER_STAGE) {
                _debugprintf("Failed to reach stage (%d)", id);
                return false;
            }
        }
    }

    return true;
}

GAME_UI_SCREEN(update_and_render_renderer_change_disclaimer) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));
    GameUI::set_ui_id((char*)"ui_options_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("GRAPHICS DEVICE CHANGE"), color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        GameUI::label(V2(50, y), string_literal("The graphics renderer will be changed on restart."), color32f32(1, 1, 1, 1), 2);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        if (GameUI::button(V2(100, y), string_literal("OK"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            update_preferences(&preferences, &temp_preferences);
            confirm_preferences(&preferences, resources);
            state->viewed_renderer_change_disclaimer = true;
            state->ui_state = UI_STATE_OPTIONS; // skip affecting the UI "Stack."
        }
    }
    GameUI::end_frame();
}

GAME_UI_SCREEN(update_and_render_options_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_options_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {

        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        // GameUI::label(V2(50, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
        game_ui_draw_title_logo(commands, V2(50, y-50), 1.25, GameUI::get_visual_alpha());
        y += 45;
        GameUI::label(V2(100, y), string_literal("OPTIONS"), color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
#if 1
        {
            auto display_modes = Graphics_Driver::get_display_modes();

            Fixed_Array<string> options_list = Fixed_Array<string>(&Global_Engine()->scratch_arena, display_modes.length);
            for (s32 index = 0; index < display_modes.length; ++index) {
                auto&  dm = display_modes[index];
                string s = string_clone(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("%d x %d", dm.width, dm.height)));
                //_debugprintf("Str: %s\n", s.data);
                options_list.push(s);
            }
            //_debugprintf("displaymode sz: %d", display_modes.length);
            GameUI::option_selector(V2(100, y), string_literal("Resolution: "), color32f32(1, 1, 1, 1), 2, options_list.data, options_list.size, &temp_preferences.resolution_option_index, !temp_preferences.fullscreen);
            y += 30;
        }
#endif
        {
            Fixed_Array<string> options_list = Fixed_Array<string>(&Global_Engine()->scratch_arena, 5);
            options_list.push(string_literal("Software Renderer"));
            options_list.push(string_literal("OpenGL 3.3 Core"));
#ifdef _WIN32
            options_list.push(string_literal("DirectX11"));
#endif
            GameUI::option_selector(V2(100, y), string_literal("Renderer*: "), color32f32(1, 1, 1, 1), 2, options_list.data, options_list.size, &temp_preferences.renderer_type);
            y += 30;
        }
        if (GameUI::checkbox(V2(100, y), string_literal("Fullscreen "), color32f32(1, 1, 1, 1), 2, &temp_preferences.fullscreen)) {}
        y += 30;
        {
          extern string target_frame_rate_tick_strings[]; // in main.cpp
          GameUI::option_selector(V2(100, y), string_literal("Framerate Limit: "), color32f32(1, 1, 1, 1), 2, target_frame_rate_tick_strings, 6, &temp_preferences.frame_limiter);
        }
        y += 30;
        if (GameUI::checkbox(V2(100, y), string_literal("Controller Vibration"), color32f32(1, 1, 1, 1), 2, &temp_preferences.controller_vibration)) {}
        y += 30;
        GameUI::f32_slider(V2(100, y), string_literal("Music Volume: "), color32f32(1, 1, 1, 1), 2, &temp_preferences.music_volume, 0, 1.0, 100);
        y += 30;
        GameUI::f32_slider(V2(100, y), string_literal("Sound Volume: "), color32f32(1, 1, 1, 1), 2, &temp_preferences.sound_volume, 0, 1.0, 100);
        y += 30;
        // also NOTE:
        /*
         * This is always going to be a weird problem because the game doesn't letter
         * box, and the world select doesn't have a following camera so it would never autocorrect
         * itself.
         *
         * I don't like letterboxing (except for the ingame section because that's just how the genre works),
         * so I have to rehack to adjust the camera.
         *
         * I'm aware this can break some of the transitions I'm doing.
         */
        if (GameUI::button(V2(100, y), string_literal("Apply"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            if (temp_preferences.renderer_type != preferences.renderer_type) {
                if (!state->viewed_renderer_change_disclaimer) {
                    state->ui_state = UI_STATE_SHOW_RENDERER_DISCLAIMER;
                }
            }
            update_preferences(&preferences, &temp_preferences);
            confirm_preferences(&preferences, resources);
        }
        y += 30;
        if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            if (temp_preferences.renderer_type != preferences.renderer_type) {
                if (!state->viewed_renderer_change_disclaimer) {
                    state->ui_state = UI_STATE_SHOW_RENDERER_DISCLAIMER;
                } else {
                    switch_ui(state->last_ui_state);
                }
            } else {
                switch_ui(state->last_ui_state);
            }

            update_preferences(&preferences, &temp_preferences);
            confirm_preferences(&preferences, resources);
            save_preferences_to_disk(&preferences, string_literal("preferences.lua"));
        }
        y += 30;
        if (GameUI::button(V2(100, y), string_literal("Back"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            temp_preferences = preferences;
            state->viewed_renderer_change_disclaimer = false;
            switch_ui(state->last_ui_state);
        }
        y += 30;

        if (Action::is_pressed(ACTION_CANCEL)) {
            state->viewed_renderer_change_disclaimer = false;
            switch_ui(state->last_ui_state);
        }
    }
    GameUI::end_frame();
}

/*
 * NOTE:
 * due to how the controls are setup in this game and how it links with the UI system,
 * the controls have to be sort of "carefully setup" because some bindings might make the game
 * effectively unplayable.
 *
 * You could edit controls.lua for a definitive way to change all behaviors but this controls menu
 * will only allow you to make "safe" edits that don't make it impossible to use the engine.
 *
 *
 * NOTE: does not support joystick/axis rebinding but afaik literally no one would willingly do this.
 * There might be "inverted axis" support at best.
 */
GAME_UI_SCREEN(update_and_render_controls_menu) {
    // Some local data lists here to control binding behavior.
    local bool action_binding_allow_binding[] = {
        /* ACTION_MOVE_UP,  */true,
        /* ACTION_MOVE_DOWN */true,
        /* ACTION_MOVE_LEFT */true,
        /* ACTION_MOVE_RIGHT*/true,
        /* ACTION_ACTION,   */true,
        /* ACTION_FOCUS,    */true,
        /* ACTION_CANCEL,   */false,
        /* ACTION_MENU,     */false,
        /* ACTION_SCREENSHOT*/false,
        /* ACTION_USE_BOMB, */true 
    };
    local bool action_binding_allow_second_key_rebinding[] = {
        /* ACTION_MOVE_UP,  */true,
        /* ACTION_MOVE_DOWN */true,
        /* ACTION_MOVE_LEFT */true,
        /* ACTION_MOVE_RIGHT*/true,
        /* ACTION_ACTION,   */false,
        /* ACTION_FOCUS,    */true,
        /* ACTION_CANCEL,   */false,
        /* ACTION_MENU,     */false,
        /* ACTION_SCREENSHOT*/false,
        /* ACTION_USE_BOMB, */true 
    };
    local s32 action_bindings_order[] = {
        ACTION_MOVE_UP,
        ACTION_MOVE_DOWN,
        ACTION_MOVE_LEFT,
        ACTION_MOVE_RIGHT,
        ACTION_ACTION,
        ACTION_USE_BOMB,
        ACTION_FOCUS,
        ACTION_SCREENSHOT,
        ACTION_CANCEL,
        ACTION_MENU,
    };

    // NOTE: These keys/buttons are reserved, for screenshot and the UI stuff and cannot be bound without
    // some inconvenient UI usage.
    local s32 blacklisted_input_keys[] = {
        KEY_F12, KEY_RETURN, KEY_BACKSPACE, KEY_ESCAPE, 
    };
    local s32 blacklisted_input_buttons[] = {
        BUTTON_START, BUTTON_BACK, BUTTON_B
    };
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    bool trying_to_bind_something = control_menu_temp_data.trying_to_bind_controls != 0;

    GameUI::set_ui_id((char*)"ui_controls_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 30;

        if (trying_to_bind_something) {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_BLUE));
            GameUI::label(V2(50, y),
                          string_from_cstring(format_temp("BINDING - %s",
                                                          action_id_string_readable_name(control_menu_temp_data.action_id_to_bind))),
                          color32f32(1, 1, 1, 1), 4);
        } else {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("CONTROLS"), color32f32(1, 1, 1, 1), 4);
        }

        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;

        {
            string* readable_name_strings =
                (string*)Global_Engine()->scratch_arena.push_unaligned((array_count(action_bindings_order)+1) * sizeof(*readable_name_strings));
            {
                for (s32 index = 0; index < array_count(action_bindings_order); ++index) {
                    s32   action_id              = action_bindings_order[index];
                    readable_name_strings[index] = string_from_cstring((cstring)action_id_string_readable_name(action_id));
                }
            }
            string longest_string = longest_string_in_list(readable_name_strings, array_count(action_bindings_order));
            f32    longest_width = font_cache_text_width(resources->get_font(MENU_FONT_COLOR_WHITE), longest_string, 2);

            for (s32 index = 0; index < array_count(action_bindings_order); ++index) {
                s32    action_id       = action_bindings_order[index];
                string name            = readable_name_strings[index];
                bool   bindable        = action_binding_allow_binding[action_id] && !trying_to_bind_something;
                bool   second_bindable = action_binding_allow_second_key_rebinding[action_id] && bindable;
                auto   action_data     = &control_menu_temp_data.temp_action_map[action_id];

                f32 x = 40;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_BLUE)); // I never use blue :P
                GameUI::label(V2(x, y), name, color32f32_WHITE, 2, !Transitions::fading());
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
                x += longest_width * 1;

                // NOTE: fixed width because I like to use designated initializers in C and when porting
                // to C++, they don't seem to have this so I just converted them to jump table functions as
                // it was the next best thing...
                x += 25;
                // Key1
                {
                    string binding_string_name = string_from_cstring((cstring)keyboard_key_strings_readable(action_data->key_id[0]));
                    if (GameUI::button(V2(x, y), binding_string_name, color32f32_WHITE, 2, !Transitions::fading() && bindable) == WIDGET_ACTION_ACTIVATE) {
                        if (control_menu_temp_data.trying_to_bind_controls == 0) {
                            control_menu_temp_data.action_id_to_bind       = action_id;
                            control_menu_temp_data.action_id_to_bind_slot  = CONTROLS_MENU_DATA_BINDING_SLOT_KEY0;
                            control_menu_temp_data.trying_to_bind_controls = 2;
                        }
                    }
                }
                x += 150;
                // Key2
                {
                    string binding_string_name = string_from_cstring((cstring)keyboard_key_strings_readable(action_data->key_id[1]));
                    if (GameUI::button(V2(x, y), binding_string_name, color32f32_WHITE, 2, !Transitions::fading() && second_bindable) == WIDGET_ACTION_ACTIVATE) {
                        if (control_menu_temp_data.trying_to_bind_controls == 0) {
                            control_menu_temp_data.action_id_to_bind       = action_id;
                            control_menu_temp_data.action_id_to_bind_slot  = CONTROLS_MENU_DATA_BINDING_SLOT_KEY1;
                            control_menu_temp_data.trying_to_bind_controls = 2;
                        }
                    }
                }
                // Gamepad
                x += 150;
                {
                    string binding_string_name = string_from_cstring((cstring)controller_button_strings_readable(action_data->button_id));
                    if (GameUI::button(V2(x, y), binding_string_name, color32f32_WHITE, 2, !Transitions::fading() && bindable) == WIDGET_ACTION_ACTIVATE) {
                        if (control_menu_temp_data.trying_to_bind_controls == 0) {
                            control_menu_temp_data.action_id_to_bind       = action_id;
                            control_menu_temp_data.action_id_to_bind_slot  = CONTROLS_MENU_DATA_BINDING_SLOT_GAMEPAD;
                            control_menu_temp_data.trying_to_bind_controls = 2;
                        }
                    }
                }

                y += 30;
            }

            if (GameUI::button(V2(50, y), string_literal("Apply"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Action::copy_action_map(control_menu_temp_data.temp_action_map, Action::get_action_map());
                Action::save(string_literal("controls.lua"));
                control_menu_temp_data.trying_to_bind_controls = 0;
            }
            y += 30;
            if (GameUI::button(V2(50, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Action::copy_action_map(control_menu_temp_data.temp_action_map, Action::get_action_map());
                control_menu_temp_data.trying_to_bind_controls = 0;
                Action::save(string_literal("controls.lua"));
                switch_ui(state->last_ui_state);
            }
            y += 30;
            if (GameUI::button(V2(50, y), string_literal("Return"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(state->last_ui_state);
            }
        }

        if (Action::is_pressed(ACTION_CANCEL)) {
            if (control_menu_temp_data.trying_to_bind_controls != 0) {
                control_menu_temp_data.trying_to_bind_controls = 0;
            } else {
                switch_ui(state->last_ui_state);
            }
        }
    }
    GameUI::end_frame();

    // Handle the actual control binding
    {
        switch (control_menu_temp_data.trying_to_bind_controls) {
            case 0: { } break;
            case 1: {
                s32 action_id = control_menu_temp_data.action_id_to_bind;
                s32 keyid     = Input::any_key_pressed();
                s32 buttonid  = Input::controller_any_button_pressed(Input::get_gamepad(0));

                if (keyid != KEY_UNKNOWN || buttonid != BUTTON_UNKNOWN) {
                    Input::eat_key(keyid);
                    Input::eat_controller_button(0, buttonid);

                    // blacklisted keys and buttons
                    bool blacklisted = false;
                    {
                        for (s32 index = 0; index < array_count(blacklisted_input_keys); ++index) {
                            if (keyid == blacklisted_input_keys[index]) {
                                blacklisted = true;
                                break;
                            }
                        }

                        for (s32 index = 0; index < array_count(blacklisted_input_buttons); ++index) {
                            if (buttonid == blacklisted_input_buttons[index]) {
                                blacklisted = true;
                                break;
                            }
                        }
                    }

                    if (blacklisted) {
                        // skip
                        _debugprintf("Illegal binding key/button. Do not do anything.");
                    } else {
                        control_menu_temp_data.trying_to_bind_controls = 0; // binding found.
                        switch (control_menu_temp_data.action_id_to_bind_slot) {
                            case CONTROLS_MENU_DATA_BINDING_SLOT_KEY0:
                            case CONTROLS_MENU_DATA_BINDING_SLOT_KEY1:  {
                                s32 keyid_slot = 0;

                                if (control_menu_temp_data.action_id_to_bind_slot == CONTROLS_MENU_DATA_BINDING_SLOT_KEY1) {
                                    keyid_slot = 1;
                                }

                                auto existing_binding =
                                    Action::action_map_get_action_data_with_key_binding(
                                        control_menu_temp_data.temp_action_map,
                                        keyid
                                    );

                                if (existing_binding.binding) {
                                    Swap(existing_binding.binding->key_id[existing_binding.keyinput_slot_id],
                                         control_menu_temp_data.temp_action_map[action_id].key_id[keyid_slot],
                                         s32);
                                } else {
                                    control_menu_temp_data.temp_action_map[action_id].key_id[keyid_slot] = keyid;
                                }
                            } break;
                            case CONTROLS_MENU_DATA_BINDING_SLOT_GAMEPAD: {
                                auto existing_binding =
                                    Action::action_map_get_action_data_with_gamepad_binding(
                                        control_menu_temp_data.temp_action_map,
                                        buttonid
                                    );

                                if (existing_binding) {
                                    Swap(existing_binding->button_id, control_menu_temp_data.temp_action_map[action_id].button_id, s32);
                                } else {
                                    control_menu_temp_data.temp_action_map[action_id].button_id = buttonid;
                                }
                            } break;
                                bad_case;
                        }
                    }
                }
            } break;
            case 2: {
                // skip the frame.
                control_menu_temp_data.trying_to_bind_controls = 1;
            } break;
        }
    }
}

GAME_UI_SCREEN(update_and_render_review_script_error_menu) {
    /*
      NOTE:
      Transitions are "finished" pre-emptivel and there might be weird issues with this.

      This is technically a debug feature that also happens to be useful for release reasons like
      modding if I want that.
    */
    auto& task_scheduler = state->coroutine_tasks;

    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_review_script_error");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("STAGE SCRIPT ERROR"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        if (task_scheduler.absolute_failure()) {
            GameUI::label(V2(50, y), string_literal("Too many script errors."), color32f32(1, 1, 1, 1), 2);
            y += 30;
            GameUI::label(V2(50, y), string_literal("Please fix the level script."), color32f32(1, 1, 1, 1), 2);
            y += 30;
        }
        GameUI::label(V2(50, y), string_literal("Lua script error:"), color32f32(1, 1, 1, 1), 2);
        y += 30;
        {
            auto error = task_scheduler.most_recent_error();
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_LIME));
            // NOTE: not copied yet. So this is always an invalid string. Whoops!
            // some really bad "wrapping" since I don't have proper string wrapping drawing procedures
            // and this is literally the only time I need them.
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_STEEL));
            {
                char textbuffer[1024];
                s32  w = 0;
                s32 index = 0;
                auto errorasstr = error->as_str();
                s32 current_line_length = 1;
                while (index < errorasstr.length && w < 1024) {
                    switch (errorasstr.data[index]) {
                        case ' ': {
                            textbuffer[w++] = errorasstr.data[index++];
                            current_line_length++;
                        } break;
                        default: {
                            s32 word_start = index;
                            while (errorasstr.data[index] != ' ' && index < errorasstr.length && w < 1024) {
                                textbuffer[w++] = errorasstr.data[index++];
                                current_line_length++;
                            }
                        } break;
                    }

                    if (current_line_length >= 80) {
                        textbuffer[w++] = '\n';
                        current_line_length = 0;
                    }
                }
                GameUI::label(V2(50, y), string_from_cstring(textbuffer), color32f32(1, 1, 1, 1), 1);
            }
            y += 150;
        }

		GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        if (task_scheduler.absolute_failure()) {
            if (GameUI::button(V2(100, y), string_literal("Resume/Continue"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_INACTIVE);
                task_scheduler.address_error();
            }
            y += 30;
        }

        if (GameUI::button(V2(100, y), string_literal("Restart"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_INACTIVE);
            task_scheduler.address_error();
            task_scheduler.errors.zero();

            gameplay_recording_file_finish(&state->gameplay_data.recording);
            reset_stage_simulation_state();
        }
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
			task_scheduler.address_error();
            switch_ui(UI_STATE_INACTIVE);
            switch_screen(GAME_SCREEN_MAIN_MENU);
        }
    }
    GameUI::end_frame();
}

GAME_UI_SCREEN(update_and_render_confirm_back_to_main_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_confirm_back_to_main_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("CONFIRMATION?"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        GameUI::label(V2(50, y), string_literal("Are you sure? You will lose your current stage progress."), color32f32(1, 1, 1, 1), 2);
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_REPLAY_ASK_TO_SAVE);
        }
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(state->last_ui_state);
        }

        if (Action::is_pressed(ACTION_CANCEL)) {
            switch_ui(state->last_ui_state);
        }
    }
    GameUI::end_frame();
}

GAME_UI_SCREEN(update_and_render_confirm_exit_to_windows) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_confirm_exit_to_windows");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("CONFIRMATION?"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        if (state->screen_mode == GAME_SCREEN_INGAME) {
            GameUI::label(V2(50, y), string_literal("Are you sure? You will lose your current stage progress."), color32f32(1, 1, 1, 1), 2);
        } else {
            GameUI::label(V2(50, y), string_literal("Are you sure? I thought we were having so much fun!"), color32f32(1, 1, 1, 1), 2);
        }
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            Transitions::do_color_transition_in(
                color32f32(0, 0, 0, 1),
                0.15f,
                0.3f
            );
                

            Transitions::register_on_finish(
                [&](void*) {
                    save_game();
                    Global_Engine()->die();
                }
            );
        }
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(state->last_ui_state);
        }

        if (Action::is_pressed(ACTION_CANCEL)) {
            switch_ui(state->last_ui_state);
        }
    }
    GameUI::end_frame();
}

// NOTE:
// this function reroutes to handle all the "transition back to menu" stages
// so it's sort of monolithic.
GAME_UI_SCREEN(update_and_render_replay_save_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_ui_id((char*)"ui_replay_save_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    enum {
        REPLAY_SAVE_MENU_ACTION_PENDING               = -1,
        REPLAY_SAVE_MENU_ACTION_SAVE_RECORDING        = 0,
        REPLAY_SAVE_MENU_ACTION_DO_NOT_SAVE_RECORDING = 1,
    };
    int action = REPLAY_SAVE_MENU_ACTION_PENDING;

    if (state->gameplay_data.recording.in_playback) {
        if (!Transitions::fading()) {
            action = REPLAY_SAVE_MENU_ACTION_DO_NOT_SAVE_RECORDING; // just skip the prompt and don't do anything.
        }
    } else {
        {
            f32 y = 50;
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("SAVE RECORDING?"), color32f32(1, 1, 1, 1), 4);
            y += 45;
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
            if (GameUI::button(V2(100, y), string_literal("Yes"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                action = REPLAY_SAVE_MENU_ACTION_SAVE_RECORDING;
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("No"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                action = REPLAY_SAVE_MENU_ACTION_DO_NOT_SAVE_RECORDING;
            }
        }
    }

    if (action != REPLAY_SAVE_MENU_ACTION_PENDING) {
        Transitions::do_shuteye_in(
            color32f32(0, 0, 0, 1),
            0.15f,
            0.3f
        );

        string full_replay_path = PREF_PATH;
        full_replay_path = string_concatenate(&Global_Engine()->scratch_arena, full_replay_path, DEFAULT_REPLAY_LOCATION);
        if (action == REPLAY_SAVE_MENU_ACTION_SAVE_RECORDING) {
            // save recording
            if (!state->gameplay_data.recording.in_playback) {
                if (state->gameplay_data.recording.memory_arena) {
                    _debugprintf("Writing recording... (%d recorded score)", state->gameplay_data.current_score);

                    if (OS_create_directory(full_replay_path)) {
                        auto calendar_date   = current_calendar_time();
                        string recordingpath = string_from_cstring(
                            format_temp(
                                "bh-%d-%d_%d-%d-%d-%d-%d-%d.recording",
                                state->gameplay_data.recording.stage_id+1,
                                state->gameplay_data.recording.level_id+1,
                                calendar_date.day,
                                calendar_date.month,
                                calendar_date.year,
                                calendar_date.hours,
                                calendar_date.minutes,
                                calendar_date.seconds
                            )
                        );

                        auto serializer = open_write_file_serializer(string_concatenate(&Global_Engine()->scratch_arena, full_replay_path, recordingpath));

                        serializer.expected_endianess = ENDIANESS_LITTLE;
                        gameplay_recording_file_serialize(
                            &state->gameplay_data.recording,
                            nullptr,
                            &serializer
                        );
                        serializer_finish(&serializer);
                    }

                    gameplay_recording_file_finish(&state->gameplay_data.recording);
                }
            }
        } else {
            // no save
            _debugprintf("Do not save a replay.");
            gameplay_recording_file_finish(&state->gameplay_data.recording);
        }

        Transitions::do_color_transition_in(
            color32f32(0, 0, 0, 1.0),
            0.25f,
            0.5f
        );

        save_game(); // redundantly save the game, so that it does not rely on the transition to actually save.
        switch (state->last_completion_state) {
            case GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE: {
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_INACTIVE);
                        _debugprintf("Hi main menu. We can do some more stuff like demonstrate if we unlocked a new stage!");

                        // Check for postgame cutscene playing
                        {
                          auto& main_menu_state = state->mainmenu_data;
#ifndef BUILD_DEMO
                            if (!main_menu_state.cutscene1.triggered && can_access_stage(3)) {
                                _debugprintf("Switched to ending");
                                switch_screen(GAME_SCREEN_ENDING);
                                main_menu_state.start_completed_maingame_cutscene(state);
                            } else {
                                _debugprintf("Switched to main menu");
                                switch_screen(GAME_SCREEN_MAIN_MENU);
                            }
#else 
                          if (!main_menu_state.cutscene4.triggered && can_access_stage(1)) {
                            _debugprintf("demo ending!"); // no special ending state
                            switch_screen(GAME_SCREEN_MAIN_MENU);
                            main_menu_state.start_completed_demo_cutscene(state);
                          }
#endif
                        }

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            } break;
            case GAME_COMPLETE_STAGE_CONTINUE_TO_NEXT_LEVEL:
            case GAME_COMPLETE_STAGE_UNLOCK_NEXT_LEVEL: {
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_INACTIVE);
                        _debugprintf("Hi, booting you to the next level.");

                        state->mainmenu_data.stage_id_level_in_stage_select += 1;
                        setup_stage_start();
                        Transitions::do_color_transition_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            } break;
            case GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY: {
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        if (state->gameplay_data.recording.in_playback) {
                          switch_ui(UI_STATE_REPLAY_COLLECTION);
                        } else {
                          switch_ui(UI_STATE_STAGE_SELECT);
                        }
                        switch_screen(GAME_SCREEN_MAIN_MENU);

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            } break;
            default: {
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_MAIN_MENU);
                        _debugprintf("Switched to main menu");

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            } break;
        }

        state->last_completion_state = -1;
    }

    GameUI::end_frame();
    GameUI::update(dt);
}

GAME_UI_SCREEN(update_and_render_replay_not_supported_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_replay_not_supported_menu");

    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y),
            string_literal("REPLAY NOT SUPPORTED / CANNOT PLAY"),
            color32f32(1, 1, 1, 1), 4);

        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        GameUI::label(
            V2(75, y),
            string_literal("Unfortunately this replay is either:\noutdated or invalid/corrupt!"),
            color32f32(1, 1, 1, 1), 2
        );

        y += 80;
        
        if (GameUI::button(V2(100, y), string_literal("Return"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_REPLAY_COLLECTION);
        }
    }
    GameUI::end_frame();
}

GAME_UI_SCREEN(update_and_render_replay_collection_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_replay_collection_menu");

    string full_replay_path = PREF_PATH;
    full_replay_path = string_concatenate(&Global_Engine()->scratch_arena, full_replay_path, DEFAULT_REPLAY_LOCATION);

    auto replay_files = directory_listing_list_all_files_in(&Global_Engine()->scratch_arena, full_replay_path);
    int page_count = (replay_files.count+(MAX_REPLAYS_PER_PAGE-1)) / MAX_REPLAYS_PER_PAGE;
    int current_page_display_amount = replay_files.count % MAX_REPLAYS_PER_PAGE;

    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y),
                      string_from_cstring(format_temp("REPLAY COLLECTION [%d / %d]", state->gameplay_data.demo_collection_ui.current_page+1, page_count)),
                      color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        if (GameUI::button(V2(100, y), string_literal("Return"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_PAUSED);
        }

        if (GameUI::button(V2(180, y), string_literal("Next"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            state->gameplay_data.demo_collection_ui.current_page += 1;
        }
        if (GameUI::button(V2(240, y), string_literal("Previous"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            state->gameplay_data.demo_collection_ui.current_page -= 1;
        }
        y += 30;

        if (state->gameplay_data.demo_collection_ui.current_page < 0) {
            state->gameplay_data.demo_collection_ui.current_page = page_count-1;
        } else if (state->gameplay_data.demo_collection_ui.current_page >= page_count) {
            state->gameplay_data.demo_collection_ui.current_page = 0;
        }

        if (replay_files.count <= 2) {
            GameUI::label(V2(130, y), string_literal("No recordings. Go make some memories!"), color32f32(1, 1, 1, 1), 2);
        } else {
            s32 start_index = MAX_REPLAYS_PER_PAGE * state->gameplay_data.demo_collection_ui.current_page + 2; // skip ./ and ../
            s32 end_index   = min<s32>(MAX_REPLAYS_PER_PAGE * (state->gameplay_data.demo_collection_ui.current_page+1) + 2, replay_files.count);

            _debugprintf("%d, %d (%d)", start_index, end_index, replay_files.count);

            for (s32 index = start_index; index < end_index; ++index) {
                auto& file = replay_files.files[index];
                if (file.is_directory) {
                    continue;
                }

                string fullname =
                    unixify_pathname(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("%s%s", replay_files.basename, file.name)));
                    
                if (GameUI::button(V2(130, y), fullname, color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                    {
                        auto serializer = open_read_file_serializer(fullname);
                        serializer.expected_endianess = ENDIANESS_LITTLE;

                        bool recording_load_result = gameplay_recording_file_serialize(
                            &state->gameplay_data.recording,
                            &Global_Engine()->main_arena,
                            &serializer
                        );

                        serializer_finish(&serializer);

                        if (recording_load_result) {
                            Transitions::do_shuteye_in(
                                color32f32(0, 0, 0, 1),
                                0.15f,
                                0.3f
                            );

                            Transitions::register_on_finish(
                                [&](void*) mutable {
                                    // NOTE: register gameplay recording data
                                    this->state->mainmenu_data.stage_id_level_select          = state->gameplay_data.recording.stage_id;
                                    this->state->mainmenu_data.stage_id_level_in_stage_select = state->gameplay_data.recording.level_id;
                                    this->state->gameplay_data.set_pet_id(state->gameplay_data.recording.selected_pet, state->resources);

                                    switch_ui(UI_STATE_INACTIVE);
                                    switch_screen(GAME_SCREEN_INGAME);

                                    // Reset Demo Viewer
                                    {
                                        state->gameplay_data.demo_viewer.paused = false;
                                        state->gameplay_data.demo_viewer.timescale_index = 3;
                                    }
                                    gameplay_recording_file_start_playback(
                                        &state->gameplay_data.recording
                                    );
                                    setup_stage_start();
                                    Transitions::do_shuteye_out(
                                        color32f32(0, 0, 0, 1),
                                        0.15f,
                                        0.3f
                                    );
                                }
                            );
                        }
                        else {
                            switch_ui(UI_STATE_REPLAY_NOT_SUPPORTED);
                        }
                    }
                }
                y += 30;
            }

            // NOTE:
            // all the UI is the same with both interfaces, and fortunately because
            // the game has extremely basic UI layout and design, I don't think there's
            // anything crazy I need.
            if (Action::is_pressed(ACTION_CANCEL)) {
                switch_ui(UI_STATE_PAUSED);
            }
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

GAME_UI_SCREEN(update_and_render_pause_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_pause_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        // GameUI::label(V2(50, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
        game_ui_draw_title_logo(commands, V2(50, y-50), 1.25, GameUI::get_visual_alpha());
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        if (GameUI::button(V2(100, y), string_literal("Resume"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_INACTIVE);
        }
        y += 30;

#ifndef RELEASE
        if (GameUI::button(V2(100, y), string_literal("[DEVEL]Reload Level"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            gameplay_recording_file_finish(&state->gameplay_data.recording);
            reset_stage_simulation_state();
            switch_ui(UI_STATE_INACTIVE);
        }
        y += 30;
#endif

        if (state->screen_mode != GAME_SCREEN_MAIN_MENU) {
            if (GameUI::button(V2(100, y), string_literal("Return To Menu"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                if (state->screen_mode != GAME_SCREEN_INGAME) {
                    Transitions::do_shuteye_in(
                        color32f32(0, 0, 0, 1),
                        0.15f,
                        0.3f
                    );
                
                    Transitions::register_on_finish(
                        [&](void*) mutable {
                            switch_ui(UI_STATE_INACTIVE);
                            switch_screen(GAME_SCREEN_MAIN_MENU);
                            _debugprintf("Hi menu.");

                            Transitions::do_shuteye_out(
                                color32f32(0, 0, 0, 1),
                                0.15f,
                                0.3f
                            );
                        }
                    );
                } else {
                    switch_ui(UI_STATE_CONFIRM_BACK_TO_MAIN_MENU);
                }
            }
            y += 30;
        }

        if (GameUI::button(V2(100, y), string_literal("Controls"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_CONTROLS);
        }
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Options"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            _debugprintf("Open the options menu I guess");
            // I'd personally like to animate these, but it requires some more dirty code if
            // I'm doing it from scratch like this.
            switch_ui(UI_STATE_OPTIONS);
        }
        y += 30;

#ifndef BUILD_DEMO
        if (GameUI::button(V2(100, y), string_literal("Achievements"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_ACHIEVEMENTS);
        }
        y += 30;
#endif

        if (GameUI::button(V2(100, y), string_literal("Replays"), color32f32(1, 1, 1, 1), 2, !Transitions::fading() && state->screen_mode != GAME_SCREEN_INGAME) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_REPLAY_COLLECTION);
        }
        y += 30;

#ifndef BUILD_DEMO
#ifndef RELEASE
        if (state->screen_mode != GAME_SCREEN_TITLE_SCREEN) {
            if (GameUI::button(V2(100, y), string_literal("Custom Levels"), color32f32(1, 1, 1, 1), 2, !Transitions::fading() && state->screen_mode != GAME_SCREEN_INGAME) == WIDGET_ACTION_ACTIVATE) {
                _debugprintf("Not supported yet!");
                // switch_ui(UI_STATE_REPLAY_COLLECTION);
            }
            y += 30;
        }
#endif
#endif

        if (state->screen_mode != GAME_SCREEN_CREDITS) {
            if (GameUI::button(V2(100, y), string_literal("Credits"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                _debugprintf("Open the credits screen I guess");

                Transitions::do_shuteye_in(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );
                

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        _debugprintf("I am credits.");
                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_CREDITS);
                        _debugprintf("Hi credits.");

                        Transitions::do_color_transition_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            }
            y += 30;
        }

        if (GameUI::button(V2(100, y),
#ifdef _WIN32
                           string_literal("Exit To Windows")
#else
                           string_literal("Exit To Desktop")
#endif
                           ,color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_CONFIRM_EXIT_TO_WINDOWS);
        }

        // NOTE:
        // all the UI is the same with both interfaces, and fortunately because
        // the game has extremely basic UI layout and design, I don't think there's
        // anything crazy I need.
        if (Action::is_pressed(ACTION_CANCEL)) {
            switch_ui(UI_STATE_INACTIVE);
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

GAME_UI_SCREEN(update_and_render_stage_select_menu) {
    bool cancel = false;
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 64), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_stage_select_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {
        s32   stage_id = state->mainmenu_data.stage_id_level_select;
        auto& stage    = stage_list[stage_id];
        f32   y        = 50;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("STAGE SELECT"), color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::label(V2(70, y), stage.name, color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::label(V2(70, y), stage.subtitle, color32f32(1, 1, 1, 1), 2);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 55;

        // the level selection UI main parts
        {
            s32   enter_level        = -1;
            s32   display_level_icon = -1;

            auto met_all_prerequisites = can_access_stage(stage_id);

            // Oh. I see!
            for (int i = 0; i < MAX_LEVELS_PER_STAGE; ++i) {
                auto& level = stage.levels[i];
                string name = {};

                bool is_unlocked = met_all_prerequisites && i < stage.unlocked_levels;
                if (is_unlocked) {
                    name = level.name;
                } else {
                    name = string_literal("???");
                }

                auto s = format_temp("[PRACTICE] %d - %d: %.*s", (stage_id+1), (i+1), name.length, name.data);

#ifndef RELEASE
                s = format_temp("[DEBUG] %d - %d", (stage_id+1), (i+1));
                is_unlocked = true;
#endif

                s32 button_status = (GameUI::button(V2(100, y), string_from_cstring(s), color32f32(1, 1, 1, 1), 2, is_unlocked && !Transitions::fading()));
                y += 30;

                if (button_status == WIDGET_ACTION_ACTIVATE) {
                    enter_level = i;
                    state->gameplay_data.playing_practice_mode = true;
                } else if (button_status == WIDGET_ACTION_HOT) {
                    display_level_icon = i;
                }
            }

            if (GameUI::button(V2(100, y), string_literal("Play Stage"), color32f32(1, 1, 1, 1), 2, !Transitions::fading() && met_all_prerequisites) == WIDGET_ACTION_ACTIVATE) {
                enter_level = 0;
                state->gameplay_data.playing_practice_mode  = false;
                state->gameplay_data.campaign_perfect_clear = true;
                state->gameplay_data.stage_perfect_clear    = true;
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                cancel = true;
            }
            y += 30;

            if (display_level_icon != -1) {
                // I don't have icon images yet, so the best I have is just printing text.
                auto& level = stage.levels[display_level_icon];
                color32f32 color = color32f32(1, 1, 1, 1);
                if (level.boss_stage) color = color32f32(1, 0, 0, 1);
                // Since the GameUI wasn't super designed with keyboard input in mind, it was sort of patched in
                // and is somewhat buggy sometimes. So these are rendered as regular render objects.

                // render_commands_push_text(commands, subtitle_font, 4, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, percentage_t), BLEND_MODE_ALPHA);
                render_commands_push_text(commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2.0f, V2(commands->screen_width - 250, commands->screen_height/2 - 200), string_from_cstring(format_temp("Best Score: %d", level.best_score)), color, BLEND_MODE_ALPHA);
                render_commands_push_text(commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2.0f, V2(commands->screen_width - 250, commands->screen_height/2 - 170), string_from_cstring(format_temp("Last Score: %d", level.last_score)), color, BLEND_MODE_ALPHA);
                render_commands_push_text(commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2.0f, V2(commands->screen_width - 250, commands->screen_height/2 - 140), string_from_cstring(format_temp("Attempts: %d", level.attempts)), color, BLEND_MODE_ALPHA);
                render_commands_push_text(commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2.0f, V2(commands->screen_width - 250, commands->screen_height/2 - 110), string_from_cstring(format_temp("Completions: %d", level.completions)), color, BLEND_MODE_ALPHA);
#if 0
                // To be honest, getting these to show up is probably more trouble than it's worth
                // regarding UI layout.
                render_commands_push_text(commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2.0f, V2(commands->screen_width - (font_cache_text_width(resources->get_font(MENU_FONT_COLOR_GOLD), level.subtitle, 2.0f)*1.2), commands->screen_height/2), level.subtitle, color, BLEND_MODE_ALPHA);
#endif
            }

            if (enter_level != -1) {
                state->mainmenu_data.stage_id_level_in_stage_select = enter_level;
                switch_ui(UI_STATE_PET_SELECT);
            }
        }

        if (Action::is_pressed(ACTION_CANCEL)) {
            cancel = true;
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);

    if (cancel) {
        switch_ui(UI_STATE_INACTIVE);

        if (!camera_already_interpolating_for(&state->mainmenu_data.main_camera, V2(commands->screen_width/2, commands->screen_height/2), 1.0)) {
            camera_set_point_to_interpolate(
                &state->mainmenu_data.main_camera,
                V2(commands->screen_width/2, commands->screen_height/2),
                1.0
            );
        }
    }
}

void Game::update_and_render_stage_pet_select_menu(struct render_commands* commands, f32 dt) {
    auto& gameplay_data = state->gameplay_data;
    bool load_game = false;

    // no options.
    // just load game immediately.
    if (gameplay_data.unlocked_pets == 0 && !Transitions::fading()) {
        load_game = true;
    }

    local s32 pet_id_list[] = {
        GAME_PET_ID_NONE,
        GAME_PET_ID_CAT,
        GAME_PET_ID_DOG,
        GAME_PET_ID_FISH,
    };

    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    bool cancel = false;

    GameUI::set_ui_id((char*)"ui_stage_pet_select_menu");
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);

    f32 y = 50;
    if (gameplay_data.unlocked_pets > 0) {
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("PET SELECT"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        {
            auto pet_data = game_get_pet_data(pet_id_list[state->mainmenu_data.stage_pet_selection]);

            GameUI::label(V2(50, y), pet_data->name, color32f32(1, 1, 1, 1), 2);
            y += 45;
            GameUI::label(V2(50, y), pet_data->description, color32f32(1, 1, 1, 1), 2);
            y += 45;


            y += 150;

            {
                f32 y_data_show = y;
                GameUI::label(V2(commands->screen_width - 300, y_data_show),
                              string_from_cstring(format_temp("Max Lives: %d", pet_data->maximum_lives)), color32f32(1, 1, 1, 1), 2);
                y_data_show += 22;
                GameUI::label(V2(commands->screen_width - 300, y_data_show),
                              string_from_cstring(format_temp("Score Modifier: %.2f", pet_data->score_modifier)), color32f32(1, 1, 1, 1), 2);
                y_data_show += 22;
                GameUI::label(V2(commands->screen_width - 300, y_data_show),
                              string_from_cstring(format_temp("Speed Modifier: %.2f", pet_data->speed_modifier)), color32f32(1, 1, 1, 1), 2);
            }

            if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                load_game = true;
                gameplay_data.set_pet_id(pet_id_list[state->mainmenu_data.stage_pet_selection], state->resources);
                _debugprintf("%.*s\n", pet_data->name.length, pet_data->name.data);
            }

            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Next"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                state->mainmenu_data.stage_pet_selection += 1;
                if (state->mainmenu_data.stage_pet_selection >= gameplay_data.unlocked_pets+1) {
                    state->mainmenu_data.stage_pet_selection = 0;
                }
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Previous"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                state->mainmenu_data.stage_pet_selection -= 1;

                if (state->mainmenu_data.stage_pet_selection < 0) {
                    state->mainmenu_data.stage_pet_selection = gameplay_data.unlocked_pets;
                }
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                cancel = true;
            }

            if (Action::is_pressed(ACTION_CANCEL)) {
                cancel = true;
            }
        }
    } else {
        gameplay_data.set_pet_id(pet_id_list[0], state->resources);
    }

    GameUI::end_frame();
    GameUI::update(dt);

    if (load_game) {
        Audio::stop_music_fadeout(1000); // stages are responsible for their own music.

        Transitions::do_shuteye_in(
            color32f32(0, 0, 0, 1),
            0.15f,
            0.3f
        );

        Transitions::register_on_finish(
            [&](void*) mutable {
                switch_ui(UI_STATE_INACTIVE);
                switch_screen(GAME_SCREEN_INGAME);
                setup_stage_start();
                Transitions::do_shuteye_out(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );
            }
        );
    } else {
        if (cancel) {
            switch_ui(state->last_ui_state);
        }
    }
}

bool Game::can_resurrect() {
    bool out_of_tries = state->gameplay_data.tries <= 0;
    return !out_of_tries;
}

bool Game::safely_resurrect_player() {
    bool worked = can_resurrect();
    const float RESURRECT_NEARBY_RADIUS = 150;

    if (worked) {
        state->gameplay_data.paused_from_death  = false;

        for (int index = 0; index < state->gameplay_data.player_damage_level_taken; ++index) {
          state->gameplay_data.remove_life();
        }

        state->gameplay_data.player.halt_burst_abilities();
        state->gameplay_data.player.halt_burst_charge_regeneration(
            calculate_amount_of_burst_depletion_flashes_for(1.485f)
        );
        state->gameplay_data.player.begin_invincibility();
        state->gameplay_data.player.heal(1);

        {
            // check nearby bullets and clear them.
            convert_bullets_to_score_pickups(RESURRECT_NEARBY_RADIUS, 125);
        }
    }

    return worked;
}

GAME_UI_SCREEN(update_and_render_game_death_maybe_retry_menu) {
    auto& deathscreen_data = state->deathscreen_data;
    auto  font             = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    string text            = string_literal("GAME OVER");
    f32 text_scale         = 8;

    float text_width = font_cache_text_width(font, text, text_scale);
    float text_height = font_cache_text_height(font) * text_scale;
    V2 text_position = V2(commands->screen_width / 2 - text_width / 2, commands->screen_height / 2 - text_height / 2);

    switch (deathscreen_data.phase) {
        case DEATH_SCREEN_PHASE_SLIDE_IN: {
            if (deathscreen_data.black_fade_t < MAX_DEATH_BLACK_FADE_T) {
                deathscreen_data.black_fade_t += dt;
            } else {
                deathscreen_data.phase = DEATH_SCREEN_PHASE_FADE_IN_TEXT;
            }
        } break;
        case DEATH_SCREEN_PHASE_FADE_IN_TEXT: {
            f32 effective_t = clamp<f32>(deathscreen_data.text_fade_t / MAX_DEATH_TEXT_FADE_T, 0.0f, 1.0f);

            if (deathscreen_data.text_fade_t < MAX_DEATH_TEXT_PHASE_LENGTH_T) {
                deathscreen_data.text_fade_t += dt;
            } else {
                deathscreen_data.text_fade_t = 0;
                deathscreen_data.phase = DEATH_SCREEN_PHASE_FADE_OUT_TEXT;
            }

            render_commands_push_text(
                commands,
                font,
                text_scale,
                text_position,
                text,
                color32f32(1, 1, 1, effective_t),
                BLEND_MODE_ALPHA
            );
        } break;
        case DEATH_SCREEN_PHASE_FADE_OUT_TEXT: {
            f32 effective_t = clamp<f32>(deathscreen_data.text_fade_t / MAX_DEATH_TEXT_FADE_T, 0.0f, 1.0f);
            
            if (deathscreen_data.text_fade_t < MAX_DEATH_TEXT_PHASE_LENGTH_T) {
                deathscreen_data.text_fade_t += dt;
            } else {
                deathscreen_data.phase = DEATH_SCREEN_PHASE_BYE;
            }

            render_commands_push_text(
                commands,
                font,
                text_scale,
                text_position,
                text,
                color32f32(1, 1, 1, 1 - effective_t),
                BLEND_MODE_ALPHA
            );
        } break;
        case DEATH_SCREEN_PHASE_BYE: {
            switch_ui(UI_STATE_REPLAY_ASK_TO_SAVE);
        } break;
    }
}

void Game::update_and_render_achievement_notifications(struct render_commands* commands, f32 dt) {
    // NOTE: notifications have to be deleted in order
    //       so I have to loop in reverse to delete them in the same loop.
    auto& notifications = achievement_state.notifications;

    f32 y_cursor_from_bottom = 0;
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    u32 notification_tile_h = 1;
    u32 notification_tile_w = 17;
    for (s32 index = notifications.size-1; index >= 0; index--) {
        auto& notification = notifications[index];
        auto  achievement  = Achievements::get(notification.id);

        // achievements will be from bottom left.
        // NOTE: adjust visual box size
        f32 final_y = commands->screen_height - (y_cursor_from_bottom + 50);
        rectangle_f32 rectangle = rectangle_f32(0, final_y, 150, 60);

        auto modulation_color = color32f32_DEFAULT_UI_COLOR;
        auto border_color = color32f32_WHITE;
        // insert description somehow;
        switch (notification.phase) {
            case ACHIEVEMENT_NOTIFICATION_PHASE_APPEAR: {
                const f32 MAX_PHASE_TIME = 0.35f;
                f32 percentage_t = clamp<f32>(notification.timer / MAX_PHASE_TIME, 0.0f, 1.0f);
                f32 rect_y = lerp_f32(commands->screen_height - (y_cursor_from_bottom), final_y, percentage_t);

                rectangle.y = rect_y;
#ifndef BUILD_DEMO
                // render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255 * percentage_t), BLEND_MODE_ALPHA);
                modulation_color.a = border_color.a = (percentage_t);
                game_ui_draw_bordered_box(V2(rectangle.x, rectangle.y), notification_tile_w, notification_tile_h, modulation_color, border_color);
                game_ui_draw_achievement_icon(*achievement, commands, V2(rectangle.x, rectangle.y-7), 1, percentage_t);
                {
                    string text = achievement->name;
                    render_commands_push_text(commands, subtitle_font, 2, V2(rectangle.x + 65, rectangle.y+13), text, color32f32(1, 1, 1, percentage_t), BLEND_MODE_ALPHA);
                }
#endif

                if (notification.timer >= MAX_PHASE_TIME) {
                    notification.phase = ACHIEVEMENT_NOTIFICATION_PHASE_LINGER;
                    notification.timer = 0;
                }
                y_cursor_from_bottom += 60;
            } break;
            case ACHIEVEMENT_NOTIFICATION_PHASE_LINGER: {
                const f32 MAX_PHASE_TIME = 1.25f;
                f32 percentage_t = clamp<f32>(notification.timer / MAX_PHASE_TIME, 0.0f, 1.0f);
#ifndef BUILD_DEMO
                // render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);
                game_ui_draw_bordered_box(V2(rectangle.x, rectangle.y), notification_tile_w, notification_tile_h, modulation_color, border_color);
                game_ui_draw_achievement_icon(*achievement, commands, V2(rectangle.x, rectangle.y-7), 1);
                {
                    string text = achievement->name;
                    render_commands_push_text(commands, subtitle_font, 2, V2(rectangle.x+65, rectangle.y+13), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
                }

#endif

                if (notification.timer >= MAX_PHASE_TIME) {
                    notification.phase = ACHIEVEMENT_NOTIFICATION_PHASE_BYE;
                    notification.timer = 0;
                }
                y_cursor_from_bottom += 60;
            } break;
            case ACHIEVEMENT_NOTIFICATION_PHASE_BYE: {
                const f32 MAX_PHASE_TIME = 0.35f;
                if (notification.timer >= MAX_PHASE_TIME) {
                    notifications.erase(index);
                } else {
                    f32 percentage_t = clamp<f32>(notification.timer / MAX_PHASE_TIME, 0.0f, 1.0f);
#ifndef BUILD_DEMO
                    // render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255 * (1 - percentage_t)), BLEND_MODE_ALPHA);
                    modulation_color.a = border_color.a = (1 - percentage_t);
                    game_ui_draw_bordered_box(V2(rectangle.x, rectangle.y), notification_tile_w, notification_tile_h, modulation_color, border_color);
                    game_ui_draw_achievement_icon(*achievement, commands, V2(rectangle.x, rectangle.y-7), 1, 1-percentage_t);
                    {
                        string text = achievement->name;
                        render_commands_push_text(commands, subtitle_font, 2, V2(rectangle.x+65, rectangle.y+13), text, color32f32(1, 1, 1, 1 - percentage_t), BLEND_MODE_ALPHA);
                    }
#endif
                    y_cursor_from_bottom += 60;
                }
            } break;
        }

        notification.timer += dt;
    }
}

void Game::game_ui_draw_title_part(struct render_commands* commands, V2 where, s8 partid, color32f32 fill_color, f32 scale, f32 alpha) {
    if (partid < 0) partid = 0;
    if (partid > 2) partid = 2;

    image_id mask_image = resources->title_screen_logo_masks[partid];
    image_id fill_image = resources->title_screen_logo_fills[partid];

    image_buffer* mask_image_buffer = graphics_assets_get_image_by_id(&resources->graphics_assets, mask_image);
    image_buffer* fill_image_buffer = graphics_assets_get_image_by_id(&resources->graphics_assets, fill_image);

    assertion((mask_image_buffer->width == fill_image_buffer->width) &&
              (mask_image_buffer->height == fill_image_buffer->height));

    rectangle_f32 dest;

    {
        dest.x = where.x;
        dest.y = where.y;
        dest.w = mask_image_buffer->width * scale;
        dest.h = mask_image_buffer->height * scale;
    }

    fill_color.a = alpha;

    render_commands_push_image(
        commands,
        mask_image_buffer,
        dest,
        RECTANGLE_F32_NULL,
        color32f32(1, 1, 1, alpha),
        NO_FLAGS,
        BLEND_MODE_ALPHA
    );
    render_commands_push_image(
        commands,
        fill_image_buffer,
        dest,
        RECTANGLE_F32_NULL,
        fill_color,
        NO_FLAGS,
        BLEND_MODE_ALPHA
    );
}

void Game::game_ui_draw_title_logo(struct render_commands* commands, V2 where, f32 scale, f32 alpha) {
    enum {
        HERO_LOGO_PART = 0, WORD1_LOGO_PART, WORD2_LOGO_PART,
    };

    V2 logo_cursor = where;
#ifndef BUILD_DEMO
    // BLUE LOGO
    color32f32 logo_color = RGBA32f32(3, 138, 255, alpha*255.0f);
    color32f32 logo_color2 = RGBA32f32(0, 181, 204, alpha*255.0f);
#else
    // GOLD LOGO
    color32f32 logo_color = RGBA32f32(255, 223, 5, alpha*255.0f);
    color32f32 logo_color2 = RGBA32f32(247, 202, 24, alpha*255.0f);
#endif

    game_ui_draw_title_part(commands, logo_cursor + V2(0, GameUI::get_wobbly_factor(0, 1251) * 5.0f*scale), WORD1_LOGO_PART, logo_color, scale, alpha);
    logo_cursor.x += 55 * scale;
    game_ui_draw_title_part(commands, logo_cursor + V2(0, GameUI::get_wobbly_factor(1, 121) * 5.0f*scale), HERO_LOGO_PART, logo_color2, scale, alpha);
    logo_cursor.x += 65 * scale;
    game_ui_draw_title_part(commands, logo_cursor + V2(0, GameUI::get_wobbly_factor(2, 11) * 5.0f*scale), WORD2_LOGO_PART, logo_color, scale, alpha);
}

void Game::game_ui_draw_bordered_box(V2 where, s32 width, s32 height, color32f32 main_color, color32f32 border_color) {
    GameUI::ninepatch(&resources->ui_texture_atlas, resources->ui_chunky, where, width, height, main_color, 1);
    GameUI::ninepatch(&resources->ui_texture_atlas, resources->ui_chunky_outline, where, width, height, border_color, 1);
}

local color32f32 _get_color_based_on_achievement_rank(s8 rank) {
    switch (rank) {
        case ACHIEVEMENT_RANK_BRONZE:
            return color32f32(205/255.0f, 127/255.0f, 50/255.0f, 1.0f);
            break;
        case ACHIEVEMENT_RANK_SILVER:
            return color32f32_WHITE;
            break;
        case ACHIEVEMENT_RANK_GOLD:
            return color32f32(255.0f / 255.0f, 215.0f / 255.0f, 0, 1.0f);
            break;
        case ACHIEVEMENT_RANK_PLATINUM:
            return color32f32(70.0f / 255.0f, 130.0f / 255.0f, 180/255.0f, 1.0f);
            break;
    }
    return color32f32_BLACK;
}

void Game::game_ui_draw_achievement_icon(const Achievement& achievement, struct render_commands* commands, V2 where, f32 scale, f32 alpha) {
    Sprite* sprite = nullptr;
    Sprite_Instance* sprite_instance = nullptr;
    auto modulation_color = _get_color_based_on_achievement_rank(achievement.rank); 
    auto ui_color = color32f32_DEFAULT_UI_COLOR;
    if (achievement.achieved) {
        sprite = graphics_get_sprite_by_id(&resources->graphics_assets, resources->unlocked_trophy_sprite);
        sprite_instance = &resources->unlocked_trophy_sprite_instance;
    } else {
        sprite = graphics_get_sprite_by_id(&resources->graphics_assets, resources->locked_trophy_sprite);
        sprite_instance = &resources->locked_trophy_sprite_instance;
        f32 darkness_factor = 2.0f;
        modulation_color.r /= darkness_factor;
        modulation_color.g /= darkness_factor;
        modulation_color.b /= darkness_factor;
        ui_color.r /= darkness_factor;
        ui_color.g /= darkness_factor;
        ui_color.b /= darkness_factor;
    }
    ui_color.a = alpha;
    modulation_color.a = alpha;

    auto  sprite_frame      = sprite_get_frame(sprite, sprite_instance->frame);
    auto  sprite_img        = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
    V2    sprite_image_size = V2(16, 16) * scale;

    {
        u32 outline_box_size = sprite_image_size.x * 2 + 5 * scale;
        render_commands_push_quad(commands,  rectangle_f32(where.x + 13, where.y + 13, outline_box_size, outline_box_size), color32u8(255, 255, 255, 255 * alpha), BLEND_MODE_ALPHA);
        render_commands_push_quad(commands,  rectangle_f32(where.x + 14, where.y + 14, outline_box_size-2, outline_box_size-2), color32u8(0, 0, 0, 255 * alpha), BLEND_MODE_ALPHA);
    }
    render_commands_push_image(commands, sprite_img, rectangle_f32(where.x + 16, where.y + 16, sprite_image_size.x*2, sprite_image_size.y*2), sprite_frame->source_rect, modulation_color, NO_FLAGS, BLEND_MODE_ALPHA);
}

GAME_UI_SCREEN(update_and_render_achievements_menu) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    auto achievements = Achievements::get_all();
    // I want a very simple layout so it'll just be a basic "list", not a table.
    const s32 MAX_ACHIEVEMENTS_PER_COLUMN = 4;
    const s32 page_count = achievements.length / MAX_ACHIEVEMENTS_PER_COLUMN + ((achievements.length%MAX_ACHIEVEMENTS_PER_COLUMN) > 0);

    auto& achievement_menu = state->achievement_menu;
    GameUI::set_ui_id((char*)"ui_achievements_menu");


    // render achievement boxes.
    // Will need to make some nice UI for all of this but at least that's art stuff.
    GameUI::begin_frame(commands, &resources->graphics_assets);
    GameUI::set_wobbly_contribution(1.0f);
    {

        auto locked_title_font       = resources->get_font(MENU_FONT_COLOR_STEEL);
        auto unlocked_title_font       = resources->get_font(MENU_FONT_COLOR_GOLD);
        auto description_font = resources->get_font(MENU_FONT_COLOR_WHITE);

        for (s32 i = 0; i < MAX_ACHIEVEMENTS_PER_COLUMN; ++i) {
            s32 actual_i = i + MAX_ACHIEVEMENTS_PER_COLUMN * achievement_menu.page;
            if (actual_i >= achievements.length) break;

            auto& achievement = achievements[actual_i];
            auto ui_color = color32f32_DEFAULT_UI_COLOR;
            if (!achievement.achieved) {
                f32 darkness_factor = 2.0f;
                ui_color.r /= darkness_factor;
                ui_color.g /= darkness_factor;
                ui_color.b /= darkness_factor;
            }

            rectangle_f32 rectangle = rectangle_f32(30, i * 80 + 80, 500, 60);
            {
                auto units_width  = (570 - 32) / resources->ui_chunky.tile_width;
                auto units_height = 2;
                game_ui_draw_bordered_box(V2(rectangle.x, rectangle.y), units_width, units_height, ui_color);
                game_ui_draw_achievement_icon(achievement, commands, V2(rectangle.x, rectangle.y), 1);
            }
            rectangle.x += 50;
            {
                string text = achievement.name;
                if (achievement.hidden && !achievement.achieved) {
                    text = string_literal("???");
                }
                render_commands_push_text(commands,
                                          (achievement.achieved) ? unlocked_title_font : locked_title_font,
                                          2, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string text = achievement.description;
                if (achievement.hidden && !achievement.achieved) {
                    text = string_literal("???????");
                }
                render_commands_push_text(commands, description_font, 2, V2(rectangle.x+10, rectangle.y+35), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
        }
    }

    {
        GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(15, 15), string_literal("ACHIEVEMENTS"), color32f32(1, 1, 1, 1), 4);

        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));

        f32 y = commands->screen_height - font_cache_text_height(resources->get_font(MENU_FONT_COLOR_GOLD)) * 4.5;
        if (GameUI::button(V2(50, y), string_literal("Back"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(state->last_ui_state);
        }

        y -= font_cache_text_height(resources->get_font(MENU_FONT_COLOR_GOLD)) * 2;

        f32 x = 50;
        if (GameUI::button(V2(x, y), string_literal("Next"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            achievement_menu.page += 1;
            if (achievement_menu.page >= page_count) {
                achievement_menu.page = 0;
            }
        }

        x += font_cache_text_width(resources->get_font(MENU_FONT_COLOR_GOLD), string_literal("Next"), 2) * 1.5;
        if (GameUI::button(V2(x, y), string_literal("Previous"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            achievement_menu.page -= 1;
            if (achievement_menu.page < 0) {
                achievement_menu.page = page_count-1;
            }
        }

        {
            if (Action::is_pressed(ACTION_CANCEL)) {
                switch_ui(state->last_ui_state);
            }
        }
    }
    GameUI::end_frame();
}

void Game::handle_ui_update_and_render(struct render_commands* commands, f32 dt) {

    // Special Case for ingame timed transitions
    // since the fixed update loop seems to have peculiar issues
    // whenever it works with the regular transitions which are not fixed
    // framerate timed...
    {
        if (state->screen_mode == GAME_SCREEN_INGAME) {
            // Death Screen Horizontal Fade
            {
                f32 effective_t = clamp<f32>(
                    state->deathscreen_data.black_fade_t / MAX_DEATH_BLACK_FADE_T,
                    0.0f,
                    1.0f
                );

                union color32f32 render_color = color32f32(0.0f, 0.0f, 0.0f, 1.0f);
                f32 position_to_draw          = lerp_f32(-(s32)commands->screen_width, 0, effective_t);
                render_commands_push_quad(commands,
                                          rectangle_f32(
                                              position_to_draw,
                                              0,
                                              commands->screen_width,
                                              commands->screen_height),
                                          color32f32_to_color32u8(render_color),
                                          BLEND_MODE_ALPHA
                );

                // I want to have a neat slice effect to make it less
                // boring than a standard screen slide.
                {
                    u32 slices_to_draw = commands->screen_height * 0.9;
                    u32 slice_height   = commands->screen_height / slices_to_draw;
                    for (unsigned slice = 0; slice < commands->screen_height; ++slice) {
                        f32 slice_width_variance = 40 % (slice+1);
                        render_commands_push_quad(commands,
                                                  rectangle_f32(
                                                      position_to_draw,
                                                      0,
                                                      commands->screen_width + slice_width_variance,
                                                      slice_height),
                                                  color32f32_to_color32u8(render_color),
                                                  BLEND_MODE_ALPHA
                        );
                    }
                }
            }

            // death animation flashing
            {
                auto& deathanimation_data = this->state->deathanimation_data;
                if (deathanimation_data.flashing) {
                    union color32f32 render_color = color32f32(1.0f, 1.0f, 1.0f, 1.0f);
                    render_commands_push_quad(commands,
                                              rectangle_f32(
                                                  0,
                                                  0,
                                                  commands->screen_width,
                                                  commands->screen_height),
                                              color32f32_to_color32u8(render_color),
                                              BLEND_MODE_ALPHA
                    );
                }
            }
        }
    }

    switch (state->ui_state) {
        case UI_STATE_INACTIVE: {
            bool should_empty_ui_id = state->screen_mode != GAME_SCREEN_CREDITS &&
                state->screen_mode != GAME_SCREEN_TITLE_SCREEN &&
                !state->dialogue_state.in_conversation;

            if (should_empty_ui_id) {
                GameUI::set_ui_id(0);
            }
        } break;
        case UI_STATE_REPLAY_ASK_TO_SAVE: {
            update_and_render_replay_save_menu(commands, dt);
        } break;
        case UI_STATE_REPLAY_COLLECTION: {
            update_and_render_replay_collection_menu(commands, dt);
        } break;
        case UI_STATE_PAUSED: {
            update_and_render_pause_menu(commands, dt);
        } break;
        case UI_STATE_CONFIRM_BACK_TO_MAIN_MENU: {
            update_and_render_confirm_back_to_main_menu(commands, dt);
        } break;
        case UI_STATE_CONFIRM_EXIT_TO_WINDOWS: {
            update_and_render_confirm_exit_to_windows(commands, dt);
        } break;
        case UI_STATE_SHOW_RENDERER_DISCLAIMER: {
            update_and_render_renderer_change_disclaimer(commands, dt);
        } break;
        case UI_STATE_OPTIONS: {
            update_and_render_options_menu(commands, dt); 
        } break;
        case UI_STATE_CONTROLS: {
            update_and_render_controls_menu(commands, dt);
        } break;
        case UI_STATE_REVIEW_SCRIPT_ERROR: {
            update_and_render_review_script_error_menu(commands, dt);
        } break;
        case UI_STATE_STAGE_SELECT: {
            update_and_render_stage_select_menu(commands, dt); 
        } break;
        case UI_STATE_PET_SELECT: {
            update_and_render_stage_pet_select_menu(commands, dt);
        } break;
        case UI_STATE_DEAD_MAYBE_RETRY: {
            update_and_render_game_death_maybe_retry_menu(commands, dt);
        } break;
        case UI_STATE_ACHIEVEMENTS: {
            update_and_render_achievements_menu(commands, dt);
        } break;
        case UI_STATE_REPLAY_NOT_SUPPORTED: {
            update_and_render_replay_not_supported_menu(commands, dt);
        } break;
        default: {
            unimplemented("Unknown ui state type");
        } break;
    }

    DebugUI::render(commands, resources->get_font(MENU_FONT_COLOR_WHITE));
}

void Game::ingame_update_introduction_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt) {
    auto& intro_state = state->gameplay_data.intro;
    auto& timer       = intro_state.stage_timer;

    timer.start();
    
    // NOTE: this is okay because I retain the state of the main menu intentionally.
    s32   stage_id = state->mainmenu_data.stage_id_level_select;
    s32   level_id = state->mainmenu_data.stage_id_level_in_stage_select;
    auto& stage    = stage_list[stage_id];

    auto timer_percentage = clamp<f32>(intro_state.stage_timer.percentage(), 0.0f, 1.0f);

    auto title_font    = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    /*
     * I understand this code pattern for writing UI tends to duplicate a lot,
     * but personally I like doing this for animated UI since it's simple and pretty easy to write
     * yet also allowing me to do somewhat interesting UI things.
     *
     * TODO: make this a bit different looking, but the general stuff here is still fine
     */
    switch (intro_state.stage) {
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_IN: {
            render_commands_push_quad(commands, rectangle_f32(0, commands->screen_height/2-32, commands->screen_width, 64), color32u8(0, 0, 0, 128 * timer_percentage), BLEND_MODE_ALPHA);
            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                intro_state.stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_NAME;
            }
        } break;
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_NAME: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, rect_y, commands->screen_width, 64), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                string title = string_concatenate(&Global_Engine()->scratch_arena, stage.name, string_concatenate(&Global_Engine()->scratch_arena, string_literal(" - "), stage.levels[level_id].name));
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }
            // TODO: not typing in yet.
            if (timer.triggered()) {
                timer = Timer(0.25f);
                timer.reset();
                intro_state.stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_SUBTITLE;
            }
        } break;
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_TYPE_IN_SUBTITLE: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, rect_y, commands->screen_width, 64), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                string title = string_concatenate(&Global_Engine()->scratch_arena, stage.name, string_concatenate(&Global_Engine()->scratch_arena, string_literal(" - "), stage.levels[level_id].name));
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 40), title, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }
            // TODO: not typing in yet.
            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                intro_state.stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_LINGER;
            }
        } break;
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_LINGER: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, rect_y, commands->screen_width, 64), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                string title = string_concatenate(&Global_Engine()->scratch_arena, stage.name, string_concatenate(&Global_Engine()->scratch_arena, string_literal(" - "), stage.levels[level_id].name));
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 40), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }

            if (timer.triggered()) {
                timer = Timer(0.45f);
                timer.reset();
                intro_state.stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_OUT_EVERYTHING;
            }
        } break;
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_OUT_EVERYTHING: {
            f32 rect_y = commands->screen_height/2-32;
            f32 effective_t = (1 - timer_percentage);
            render_commands_push_quad(commands, rectangle_f32(0, rect_y, commands->screen_width, 64), color32u8(0, 0, 0, 128 * effective_t), BLEND_MODE_ALPHA);
            {
                string title = string_concatenate(&Global_Engine()->scratch_arena, stage.name, string_concatenate(&Global_Engine()->scratch_arena, string_literal(" - "), stage.levels[level_id].name));
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, effective_t), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 40), title, color32f32(1, 1, 1, effective_t), BLEND_MODE_ALPHA);
            }

            if (timer.triggered())  {
                // allow gameplay to happen.
                intro_state.stage = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE;
            }
        } break;
        case GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE: {} break;
    }

    timer.update(dt);
}

void Game::ingame_update_complete_stage_sequence_player_animate_exit(f32 dt) {
    auto&       complete_stage_state = state->gameplay_data.complete_stage;
    auto&       player_entity        = state->gameplay_data.player;
    const auto& play_area            = state->gameplay_data.play_area;
    V2          play_area_center     = V2(play_area.width/2, play_area.height/2);

    switch (complete_stage_state.player_exit_animation_stage) {
        case GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_PIVOT_TO_CENTER: {
            auto direction_to_center = V2_direction(player_entity.position, play_area_center);
            f32 distance_to_center = V2_distance(player_entity.position, play_area_center);

            if (distance_to_center <= 5.0f) {
                player_entity.set_position(play_area_center);
                complete_stage_state.player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BACK_UP;
                complete_stage_state.exit_animation_stage_timer = Timer(1.0);
                complete_stage_state.exit_animation_stage_timer.start();
                player_entity.acceleration = V2(0, 0);
                player_entity.velocity = V2(0, 0);
            } else {
                player_entity.acceleration = direction_to_center * 200;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BACK_UP: {
            if (complete_stage_state.exit_animation_stage_timer.triggered()) {
                {
                    config_game_entity_player_propel_particle_emitter(complete_stage_state.player_propel_particles, player_entity.position, resources);
                    complete_stage_state.player_propel_particles.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE;
                    complete_stage_state.player_propel_particles.reset();
                }
                complete_stage_state.exit_animation_stage_timer = Timer(2.5f);
                complete_stage_state.exit_animation_stage_timer.start();
                complete_stage_state.player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BLAST_OFF;
            } else {
                player_entity.acceleration = V2(0, 180);
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_BLAST_OFF: {
            // poof explosion!
            if (complete_stage_state.exit_animation_stage_timer.triggered()) {
                complete_stage_state.exit_animation_stage_timer = Timer(1.0f);
                complete_stage_state.exit_animation_stage_timer.start();
                complete_stage_state.player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_LINGER;
            } else {
                player_entity.velocity = V2(0, -800);
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_LINGER: {
            if (complete_stage_state.exit_animation_stage_timer.triggered()) {
                complete_stage_state.player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_DONE;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_DONE: {
            complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN;
            complete_stage_state.player_exit_animation_stage = GAMEPLAY_STAGE_COMPLETE_STAGE_PLAYER_EXIT_ANIMATION_STAGE_PIVOT_TO_CENTER;
        } break;
    }

    if (complete_stage_state.player_propel_particles.flags & PARTICLE_EMITTER_FLAGS_ACTIVE) {
        complete_stage_state.player_propel_particles.update(&state->gameplay_data.stage_exit_particle_pool, &state->gameplay_data.prng_unessential, dt);
    }

    state->gameplay_data.stage_exit_particle_pool.update(state, dt);

    complete_stage_state.exit_animation_stage_timer.update(dt);
}

void Gameplay_Stage_Introduction_Sequence::begin_sequence() {
    stage       = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_IN;
    stage_timer = Timer(0.25f);
}

void Gameplay_Stage_Complete_Stage_Sequence::reset(void) {
    stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;
}

void Gameplay_Stage_Complete_Stage_Sequence::begin_sequence(bool replay_mode) {
    stage       = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_ANIMATE_PLAYER_EXIT;

    stage_timer = Timer(0.35f);

    if (replay_mode) {
        stage       = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN;
    }
}

struct Stage_Completion_Score_Category {
  string name;
  int    value;
  int    text_font_color;
  float  value2;
  bool   floatv = false;
  bool   enabled=true;
};

static void 
draw_completion_score_list(
  Stage_Completion_Score_Category* categories, int count, 
  f32 where_y,
  struct render_commands* commands, 
  Game_Resources* resources,
  f32 alpha)
{
  f32 scale = 2.0f;
  f32 longest_string_width;
  {
    font_cache* _font = resources->get_font(MENU_FONT_COLOR_BLUE);
    string* string_list = (string*) Global_Engine()->scratch_arena.push_unaligned(count * sizeof(*string_list));
    for (int index = 0; index < count; ++index) {
      string_list[index] = categories[index].name;
    }

    longest_string_width = font_cache_text_width(
      _font, longest_string_in_list(string_list, count), scale);
  }

  f32 x_offset = 150;
  for (int index = 0; index < count; ++index) {
    auto& category = categories[index];
    
    if (!category.enabled) {
      continue;
    }

    font_cache* font = resources->get_font(category.text_font_color);
    font_cache* font1 = resources->get_font(MENU_FONT_COLOR_WHITE);
    f32 text_height = font_cache_text_height(font) * scale;
    
    render_commands_push_text(
      commands, font, 
      scale, V2(commands->screen_width / 2 - x_offset, where_y), category.name, 
      color32f32(1, 1, 1, alpha), BLEND_MODE_ALPHA);

    string text_string;
    if (category.floatv) {
      text_string = memory_arena_push_string(
        &Global_Engine()->scratch_arena,
        string_from_cstring(format_temp("%f", category.value2)));
    }
    else {
      text_string = memory_arena_push_string(
        &Global_Engine()->scratch_arena,
        string_from_cstring(format_temp("%d", category.value)));
    }
    
    render_commands_push_text(
      commands, font1,
      scale, V2(commands->screen_width / 2 - x_offset + longest_string_width*1.25, where_y), text_string,
      color32f32(1, 1, 1, alpha), BLEND_MODE_ALPHA);
    where_y += text_height*1.1;
  }
}

void Game::ingame_update_complete_stage_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt) {
    auto& complete_stage_state = state->gameplay_data.complete_stage;
    auto& timer                = complete_stage_state.stage_timer;

    auto title_font    = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    f32 timer_percentage = clamp<f32>(timer.percentage(), 0.0f, 1.0f);

    s32 stage_id = state->mainmenu_data.stage_id_level_select;
    s32 level_id = state->mainmenu_data.stage_id_level_in_stage_select;

    timer.start();

    // TODO(jerry): make this look nicer
    string level_complete_text = string_literal("LEVEL COMPLETE");
    if (state->gameplay_data.recording.in_playback) {
        level_complete_text = string_literal("RECORDING COMPLETE");
    }

    bool new_highscore_obtained = game_will_be_new_high_score(stage_id, level_id, state->gameplay_data.total_score);

    string label_strings[] = {
      (new_highscore_obtained) ? string_literal("NEW HIGH SCORE") : string_literal("HIGH SCORE"),
      string_literal("PET SCORE MODIFIER"),
      string_literal("DEATH PENALTY"),
      string_literal("BASE SCORE"),
      string_literal("PERFECT CLEAR BONUS"),
      string_literal("PERFECT CAMPAIGN CLEAR BONUS"),
      string_literal("LIVES REMAINING BONUS"),
      string_literal("BURST USAGE BONUS"),
      string_literal("TOTAL SCORE"),
    };

    s32 high_score = stage_list[stage_id].levels[level_id].best_score;
    if (new_highscore_obtained) {
      high_score = state->gameplay_data.total_score;
    }

    f32 score_modifier = pets_data[clamp<s32>(state->gameplay_data.selected_pet, 0, array_count(pets_data)-1)].score_modifier;
    s32 death_count_deducation_score = state->gameplay_data.death_count * 10000;
    s32 perfect_clear_score_addition = state->gameplay_data.stage_perfect_clear * 100000 * score_modifier;
    s32 campaign_clear_perfect_score_addition = (level_id==2)*state->gameplay_data.campaign_perfect_clear * 500000 * score_modifier;
    s32 burst_usage_count_score_addition = state->gameplay_data.burst_usage_count * 2500 * score_modifier;
    s32 remaining_lives_score_addition = (state->gameplay_data.tries) * 1000 * score_modifier;

    Stage_Completion_Score_Category score_categories[] = {
      {label_strings[0], high_score, MENU_FONT_COLOR_STEEL},
      {label_strings[1], 0, MENU_FONT_COLOR_STEEL, score_modifier, true, state->gameplay_data.unlocked_pets>0 || state->gameplay_data.selected_pet>0}, // no spoilers until you have a pet.
      {label_strings[2], -death_count_deducation_score, MENU_FONT_COLOR_BLOODRED},
      {label_strings[3], state->gameplay_data.current_score, MENU_FONT_COLOR_STEEL},
      {label_strings[4], perfect_clear_score_addition, MENU_FONT_COLOR_STEEL},
      {label_strings[5], campaign_clear_perfect_score_addition, MENU_FONT_COLOR_SKYBLUE},
      {label_strings[6], remaining_lives_score_addition, MENU_FONT_COLOR_SKYBLUE},
      {label_strings[7], burst_usage_count_score_addition, MENU_FONT_COLOR_SKYBLUE},
      {label_strings[8], state->gameplay_data.total_score, MENU_FONT_COLOR_GOLD},
    };

    f32 rect_y = 100;
    switch (complete_stage_state.stage) {
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_ANIMATE_PLAYER_EXIT: {
            ingame_update_complete_stage_sequence_player_animate_exit(dt);
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN: {
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128 * timer_percentage), BLEND_MODE_ALPHA);
            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE: {
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            f32 alpha = saturate(timer_percentage * 2);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, alpha), BLEND_MODE_ALPHA);
            }
            rect_y += 64;
            draw_completion_score_list(score_categories, array_count(score_categories), rect_y, commands, resources, alpha);

            if (timer.triggered()) {
                timer = Timer(0.1); // TODO: animation.
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_TALLY_TOTAL_SCORE;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_TALLY_TOTAL_SCORE: {
          render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
          {
            auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
            render_commands_push_text(commands, title_font, 4, V2(commands->screen_width / 2 - text_width / 2, rect_y), level_complete_text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
          }
          rect_y += 64;
          draw_completion_score_list(score_categories, array_count(score_categories), rect_y, commands, resources, 1);

          if (Input::any_input_activity()) {
            timer.trigger_immediate();
          }

          if (timer.triggered()) {
            timer = Timer(5.5f);
            timer.reset();
            state->gameplay_data.total_score =
              state->gameplay_data.current_score - death_count_deducation_score +
              perfect_clear_score_addition + campaign_clear_perfect_score_addition +
              burst_usage_count_score_addition + remaining_lives_score_addition;
            complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT;
          }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT: {
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            rect_y += 64;
            draw_completion_score_list(score_categories, array_count(score_categories), rect_y, commands, resources, 1);

            if (Input::any_input_activity()) {
              timer.trigger_immediate();
            }

            if (timer.triggered()) {
                timer = Timer(0.5f); // allow skip
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT;
                Audio::stop_music();
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT: {
            f32 alpha = saturate((1.0f - timer_percentage));
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128 * alpha), BLEND_MODE_ALPHA);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, 1 - timer_percentage), BLEND_MODE_ALPHA);
            }
            rect_y += 64;
            draw_completion_score_list(score_categories, array_count(score_categories), rect_y, commands, resources, alpha);

            if (timer.triggered())  {
                _debugprintf("Okay. Transition bye?");
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;

                s32 completion_type =  GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY;

                if (!state->gameplay_data.recording.in_playback) {
                    game_update_stage_score(stage_id, level_id, state->gameplay_data.total_score);
                    game_register_stage_completion(stage_id, level_id);
                    completion_type = game_complete_stage_level(stage_id, level_id, state->gameplay_data.playing_practice_mode);

                    switch (completion_type) {
                        case GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE: {
                            if (state->gameplay_data.unlocked_pets < 3) {
                                state->mainmenu_data.start_unlock_pet_cutscene(state);

                                if (state->gameplay_data.stage_perfect_clear) {
                                    // ? add to score probably.
                                    // nothing.
                                }

                                if (state->gameplay_data.campaign_perfect_clear) {
                                    auto achievement = Achievements::get(ACHIEVEMENT_ID_STAGE1_FLAWLESS + stage_id);
                                    achievement->report();
                                }
                            }
                        } break;
                        default: {
                        } break;
                    }

                    // assume the next stage is perfect cleared by default.
                    state->gameplay_data.stage_perfect_clear = true;
                }

                Transitions::do_color_transition_in(
                    color32f32(0, 0, 0, 0.5),
                    0.25f,
                    0.5f
                );

                state->last_completion_state = completion_type;
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_REPLAY_ASK_TO_SAVE);
                        Transitions::clear_effect();
                    }
                );
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE: {} break;
    }

    timer.update(dt);
}

local void update_particle_emitters(
    Fixed_Array<Particle_Emitter>& emitters,
    Particle_Pool* pool,
    random_state* prng,
    f32 dt) {
    for (s32 particle_emitter_index = 0; particle_emitter_index < emitters.size; ++particle_emitter_index) {
        auto& particle_emitter = emitters[particle_emitter_index];
        particle_emitter.update(pool, prng, dt);

        if (!(particle_emitter.flags & PARTICLE_EMITTER_FLAGS_ACTIVE)) {
            emitters.pop_and_swap(particle_emitter_index);
        }
    }
}

void Game::simulate_game_frame(Entity_Loop_Update_Packet* update_packet_data) {
    auto state = &this->state->gameplay_data;
    f32 dt = update_packet_data->dt;
    bool in_conversation = this->state->dialogue_state.in_conversation;

    // conversations are not recorded into playback. This is fine. I hope.
    if (!in_conversation) {
        if (state->recording.in_playback) {
            if (gameplay_recording_file_has_more_frames(&state->recording)) {
                // NOTE: 
                state->current_input_packet = gameplay_recording_file_next_frame(&state->recording);
            } else {
                state->triggered_stage_completion_cutscene = true;
                state->complete_stage.stage       = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN;
                state->complete_stage.stage_timer = Timer(0.35f);
                zero_memory(&state->current_input_packet, sizeof(state->current_input_packet));
                _debugprintf("Out of frames.");
                return;
            }
        } else {
            if (!state->triggered_stage_completion_cutscene) {
                state->build_current_input_packet();
                gameplay_recording_file_record_frame(&state->recording, state->current_input_packet);
            }
        }
    }

    this->state->coroutine_tasks.schedule_by_type(this->state, dt, GAME_TASK_SOURCE_GAME_FIXED);
    this->state->gameplay_data.simple_scrollable_backgrounds.update(dt);

    // shouldn't happen during replays.
    if (this->state->ui_state != UI_STATE_REVIEW_SCRIPT_ERROR && this->state->coroutine_tasks.need_to_address_error()) {
        Transitions::clear_effect();
        switch_ui(UI_STATE_REVIEW_SCRIPT_ERROR);
        return;
    }

    if (!in_conversation) {
#if 1
        Thread_Pool::add_job(
            [](void* ctx) {
                auto packet = (Entity_Loop_Update_Packet*) ctx;
                Game_State* game_state = packet->game_state;
                Gameplay_Data* state = &packet->game_state->gameplay_data;
                f32 dt = packet->dt;

                for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
                    auto& h = state->explosion_hazards[i];
                    h.update(game_state, dt);
                }

                return 0;
            },
            (void*)update_packet_data
        );

        Thread_Pool::add_job(
            [](void* ctx) {
                auto packet = (Entity_Loop_Update_Packet*) ctx;
                Game_State* game_state = packet->game_state;
                Gameplay_Data* state = &packet->game_state->gameplay_data;
                f32 dt = packet->dt;

                for (int i = 0; i < (int)state->bullets.size; ++i) {
                    auto& b = state->bullets[i];
                    b.update(packet->game_state, dt);
                }

                return 0;
            },
            (void*)update_packet_data
        );

        Thread_Pool::add_job(
            [](void* ctx) {
                auto packet = (Entity_Loop_Update_Packet*) ctx;
                Game_State* game_state = packet->game_state;
                Gameplay_Data* state = &packet->game_state->gameplay_data;
                f32 dt = packet->dt;

                for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
                    auto& h = state->laser_hazards[i];
                    h.update(game_state, dt);
                }

                return 0;
            },
            (void*)update_packet_data
        );

        Thread_Pool::add_job(
            [](void* ctx) {
                auto packet = (Entity_Loop_Update_Packet*) ctx;
                Game_State* game_state = packet->game_state;
                Gameplay_Data* state = &packet->game_state->gameplay_data;
                f32 dt = packet->dt;

                for (int i = 0; i < (s32)state->enemies.size; ++i) {
                    auto& e = state->enemies[i];
                    e.update(game_state, dt);
                }

                return 0;
            },
            (void*)update_packet_data
        );

        Thread_Pool::synchronize_tasks();
        // NOTE: this is "hard" data dependency
        // since it's kind of noticable if pickups deviate more.

        {
            auto packet = (Entity_Loop_Update_Packet*) update_packet_data;
            Game_State* game_state = packet->game_state;
            Gameplay_Data* state = &packet->game_state->gameplay_data;
            f32 dt = packet->dt;

            for (int i = 0; i < (s32)state->pickups.size; ++i) {
                auto& e = state->pickups[i];
                e.update(game_state, dt);
            }

            state->player.update(game_state, dt);
            state->pet.update(game_state, dt);
            if (!state->disable_grazing)
                state->player.handle_grazing_behavior(game_state, dt);
        }
#else
        // Single-Threaded path for truth testing.
        {
            auto packet = (Entity_Loop_Update_Packet*) update_packet_data;
            Game_State* game_state = packet->game_state;
            Gameplay_Data* state = &packet->game_state->gameplay_data;
            f32 dt = packet->dt;

            for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
                auto& h = state->explosion_hazards[i];
                h.update(game_state, dt);
            }

            for (int i = 0; i < (int)state->bullets.size; ++i) {
                auto& b = state->bullets[i];
                b.update(packet->game_state, dt);
            }

            for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
                auto& h = state->laser_hazards[i];
                h.update(game_state, dt);
            }

            for (int i = 0; i < (s32)state->enemies.size; ++i) {
                auto& e = state->enemies[i];
                e.update(game_state, dt);
            }

            for (int i = 0; i < (s32)state->pickups.size; ++i) {
                auto& e = state->pickups[i];
                e.update(game_state, dt);
            }

            state->player.update(game_state, dt);
            state->pet.update(game_state, dt);
            if (!state->disable_grazing)
                state->player.handle_grazing_behavior(game_state, dt);
        }

#endif
        // Update all particle emitters
        // while we wait.
        update_particle_emitters(state->particle_emitters, &state->particle_pool, &state->prng, dt);
        update_particle_emitters(state->ui_particle_emitters, &state->ui_particle_pool, &state->prng_unessential, dt);
        state->particle_pool.update(this->state, dt);
        state->ui_particle_pool.update(this->state, dt);

        handle_all_explosions(dt);
        handle_all_lasers(dt);
        handle_player_pickup_collisions(dt);
        handle_player_enemy_collisions(dt);
        handle_all_bullet_collisions(dt);
        handle_bomb_usage(dt);
        handle_all_dead_entities(dt);
        state->reify_all_creation_queues();

        camera_update(&state->main_camera, dt);
    }
}

// NOTE: this does not actually simulate the same way as
// the actual game loop, so this needs adjustment in the future.
// Technically the replay system is actually working, but this is a relatively
// specific edge-case (and more for debugging anyway) so if I want to save a headache
// I'd just probably remove this from the game.
void Game::simulate_game_frames_until(int nth_frame) {
    auto state = &this->state->gameplay_data;

    int  desired_frame = nth_frame;
    bool  simulate_frame = true;

    Audio::disable();
    {
        auto update_packet_data = (Entity_Loop_Update_Packet*)Global_Engine()->scratch_arena.push_unaligned(sizeof(Entity_Loop_Update_Packet));
        update_packet_data->dt = FIXED_TICKTIME;
        update_packet_data->game_state = this->state;

        // NOTE: only frames that come before require a full resimulation
        // since the timeline runs from a lua coroutine, I need to maintain coroutine state
        // which isn't possible in lua.
        //
        // Yeah.
        if (desired_frame < state->recording.playback_frame_index) {
            // partial clean up of resources...
            state->unload_all_script_loaded_resources(this->state, this->state->resources);
            state->prng = state->recording.old_prng;
            state->recording.playback_frame_index = 0;
            state->recording.frames_run = 0;
            reset_stage_simulation_state();
            _debugprintf("Restart to lead up to.");
        } else {
            desired_frame -= state->recording.playback_frame_index;
            _debugprintf("Faster simulation?");
        }

        while (desired_frame) {
            this->state->coroutine_tasks.schedule_by_type(this->state, FIXED_TICKTIME, GAME_TASK_SOURCE_GAME);
            simulate_game_frame(update_packet_data);
            if (!this->state->dialogue_state.in_conversation) {
              if (state->complete_stage.stage != GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE) {
                state->current_stage_timer += FIXED_TICKTIME;
              }
            } else {
                // no dialogue during replays
                // might still play the waits the dialogues have... Which is a problem...
                this->state->dialogue_state.in_conversation = false;
            }
            desired_frame -= 1;
        }
    }
    Audio::enable();
}

#include "dialogue_ui.cpp"

GAME_SCREEN(update_and_render_game_ingame) {
    auto state = &this->state->gameplay_data;

    {
        s32 stage_id = this->state->mainmenu_data.stage_id_level_select;
        s32 level_id = this->state->mainmenu_data.stage_id_level_in_stage_select;

        if (state->recording.in_playback) {
            Discord_Integration::update_activity(
                discord_activity()
                .State(string_literal("Reflecting and growing."))
                .Details(string_from_cstring(format_temp("Watching replay of Stage %d-%d", stage_id+1, level_id+1)))
                .Large_Image(DISCORD_GAMEICON_ASSET_KEY)
            );
        } else {
            Discord_Integration::update_activity(
                discord_timestamped_activity(state->started_system_time)
                .State(string_literal("Conquering perils!"))
                .Details(string_from_cstring(format_temp("Stage %d-%d - Score: %d", stage_id+1, level_id+1, state->current_score)))
                .Large_Image(DISCORD_GAMEICON_ASSET_KEY)
            );
        }
    }

    V2 resolution = V2(game_render_commands->screen_width, game_render_commands->screen_height);
    {
        state->play_area.x      = resolution.x / 2 - state->play_area.width * 0.75;
        state->play_area.height = resolution.y;

        state->main_camera.xy.x = -state->play_area.x;
    }

    if (Action::is_pressed(ACTION_MENU)) {
        bool allow_pausing = true;

        if (this->state->ui_state == UI_STATE_DEAD_MAYBE_RETRY) {
            allow_pausing = false;
        }

        if (this->state->ui_state == UI_STATE_REPLAY_ASK_TO_SAVE) {
          allow_pausing = false;
        }

        if (this->state->gameplay_data.complete_stage.stage != GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE) {
            allow_pausing = false;
        }

        if (this->state->deathanimation_data.phase != DEATH_ANIMATION_PHASE_INACTIVE) {
            allow_pausing = false;
        }

        if (this->state->gameplay_data.triggered_stage_completion_cutscene == true) {
          allow_pausing = false;
        }

        if (allow_pausing) {
            if (this->state->ui_state != UI_STATE_PAUSED) {
                switch_ui(UI_STATE_PAUSED);
            } else {
                switch_ui(UI_STATE_INACTIVE);
            }
        }
    }

    game_render_commands->camera = state->main_camera;

    // HandleFocusUIFadeIn
    {
        const f32 T_SPEED_MOD = 8;
        bool should_do_fade =
            (state->player.under_focus) ||
            (state->player.burst_ray_attack_ability_timer > 0.0f);

        if (should_do_fade) {
            state->focus_tint_fade_t += dt*T_SPEED_MOD;
        } else {
            state->focus_tint_fade_t -= dt*T_SPEED_MOD;
        }

        state->focus_tint_fade_t =
            clamp<f32>(state->focus_tint_fade_t, 0.0f, 1.0f);
    }

    // HandleInvalidUsageFlash
    {
        if (state->invalid_usage_flash_count > 0) {
            if (state->invalid_usage_flash_t < 0.05f) {
                state->invalid_usage_flash_t += dt;
            } else {
                state->invalid_usage_flash_t = 0;
                state->invalid_usage_flash_count -= 1;
            }
        }
    }

    // draw play area borders / Game UI
    // I'd like to have the UI fade in / animate all fancy like when I can
    {
      auto border_color = color32u8(0, 15, 18, 255);

      int play_area_width = state->play_area.width;
      int play_area_height = state->play_area.height;
      int play_area_x = state->play_area.x;

      // These should also be nice images in the future.
      {
#if 1 // NEO BKG
        auto modulation_shadow = color32u8_to_color32f32(color32u8(10 / 2, 10 / 2, 32 / 2, 255));
        {
          auto modulation = color32u8_to_color32f32(color32u8(200, 200, 200, 255));
          f32 shadow_width = 64 + normalized_sinf(Global_Engine()->global_elapsed_time) * 48;
          // NOTE(jerry):
          // background is hard-coded to assume the two supplied marquee images in the resources folder.
          // This should be good for a decent amount of widescreen resolutions without having borders.
          int bkg_image_width = 854;
          int bkg_image_height = 480;

          auto focus_mod_color = color32u8(25, 0, 50, 175 * state->focus_tint_fade_t);
          auto invalid_usage_color = color32u8(200, 15, 35, 175);
          bool is_flashing_invalid_usage = state->invalid_usage_flash_count > 0 && (state->invalid_usage_flash_count % 2) == 0;
          // left border
          {
              auto marquee_bkg = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_marquee_bkrnd_neo[0]);
              render_commands_push_image(
                  ui_render_commands,
                  marquee_bkg,
                  rectangle_f32(play_area_x - bkg_image_width, 0, bkg_image_width, bkg_image_height),
                  RECTANGLE_F32_NULL,
                  modulation,
                  0,
                  BLEND_MODE_ALPHA
              );
              render_commands_push_image(
                  ui_render_commands,
                  graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_vignette_borders[0]),
                  rectangle_f32((play_area_x + 1) - shadow_width, 0, shadow_width, resolution.y),
                  RECTANGLE_F32_NULL,
                  modulation_shadow,
                  0,
                  BLEND_MODE_ALPHA
              );

              render_commands_push_quad(
                  ui_render_commands, rectangle_f32(play_area_x - bkg_image_width, 0, bkg_image_width, bkg_image_height),
                  focus_mod_color, 
                  BLEND_MODE_ALPHA
              );

              if (is_flashing_invalid_usage) {
                  render_commands_push_quad(
                      ui_render_commands, rectangle_f32(play_area_x - bkg_image_width, 0, bkg_image_width, bkg_image_height),
                      invalid_usage_color, 
                      BLEND_MODE_ALPHA
                  );
              }
          }
          // right border
          {
              auto marquee_bkg = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_marquee_bkrnd_neo[1]);
              render_commands_push_image(
                  ui_render_commands,
                  marquee_bkg,
                  rectangle_f32(play_area_x + play_area_width, 0, bkg_image_width, bkg_image_height),
                  RECTANGLE_F32_NULL,
                  modulation,
                  0,
                  BLEND_MODE_ALPHA
              );
              render_commands_push_image(
                  ui_render_commands,
                  graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_vignette_borders[0]),
                  rectangle_f32(play_area_x + play_area_width - 0.05f, 0, shadow_width, resolution.y),
                  RECTANGLE_F32_NULL,
                  modulation_shadow,
                  DRAW_IMAGE_FLIP_HORIZONTALLY,
                  BLEND_MODE_ALPHA
              );

              render_commands_push_quad(
                  ui_render_commands, rectangle_f32(play_area_x + play_area_width, 0, bkg_image_width, bkg_image_height),
                  focus_mod_color,
                  BLEND_MODE_ALPHA
              );

              if (is_flashing_invalid_usage) {
                  render_commands_push_quad(
                      ui_render_commands, rectangle_f32(play_area_x + play_area_width, 0, bkg_image_width, bkg_image_height),
                      invalid_usage_color, 
                      BLEND_MODE_ALPHA
                  );
              }
          }
        }
#else
        auto marquee_bkg = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_marquee_bkrnd);
        // NOTE: playing with colors
        //auto modulation = color32f32(0.1, 0.35, 0.8, 1);
        auto modulation = color32u8_to_color32f32(color32u8(155, 188, 255, 255));
        auto modulation_shadow = color32u8_to_color32f32(color32u8(10, 10, 32, 255));
        // auto modulation_shadow = color32u8_to_color32f32(color32u8(255, 10, 32, 255));

        f32 shadow_width = 64 + normalized_sinf(Global_Engine()->global_elapsed_time) * 48;
        // left border
        render_commands_push_image(
          ui_render_commands,
          marquee_bkg,
          rectangle_f32(0, 0, play_area_x, resolution.y),
          rectangle_f32(0, 0, play_area_x, marquee_bkg->height),
          modulation,
          0,
          BLEND_MODE_ALPHA
        );
        render_commands_push_image(
          ui_render_commands,
          graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_vignette_borders[0]),
          rectangle_f32(play_area_x - shadow_width, 0, shadow_width, resolution.y),
          RECTANGLE_F32_NULL,
          modulation_shadow,
          0,
          BLEND_MODE_ALPHA
        );
        // right border
        render_commands_push_image(
          ui_render_commands,
          marquee_bkg,
          rectangle_f32(play_area_x + play_area_width, 0, resolution.x - play_area_width, resolution.y),
          rectangle_f32(play_area_width + play_area_x, 0, marquee_bkg->width, marquee_bkg->height),
          modulation,
          0,
          BLEND_MODE_ALPHA
        );
        render_commands_push_image(
          ui_render_commands,
          graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_vignette_borders[0]),
          rectangle_f32(play_area_x + play_area_width, 0, shadow_width, resolution.y),
          RECTANGLE_F32_NULL,
          modulation_shadow,
          DRAW_IMAGE_FLIP_HORIZONTALLY,
          BLEND_MODE_ALPHA
        );
#endif

        this->state->set_led_primary_color(
          color32u8(255 * modulation_shadow.r,
            255 * modulation_shadow.g,
            255 * modulation_shadow.b,
            255)
        );
      }

      // DrawSideUI()
      {
        f32 ui_cursor_y = 50;
        f32 ui_cursor_x_left = 0;
        // NOTE: really need to adjust the layout
        // Render_OverlayBoxShadow
        {
          // center with remaining space...
          f32 border_left = play_area_x + play_area_width;
          f32 border_right = min<f32>(resolution.x, 1000); // place an artifical limit on centering (for super widescreen)
          f32 width_of_right_side = border_right - border_left;
          f32 box_w = 200;
          f32 box_x = (width_of_right_side / 2.0f) - box_w/2;

          render_commands_push_quad(
            ui_render_commands,
            rectangle_f32(border_left + box_x, 30, box_w, 480-25),
            color32u8(45 / 2, 50 / 2, 100 / 2, 230),
            BLEND_MODE_ALPHA
          );

          ui_cursor_x_left = box_x + border_left;
        }
        // Render_Score
        // Draw score and other stats like attack power or speed or something
        {
          auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
          auto font1 = resources->get_font(MENU_FONT_COLOR_GOLD);
          auto text = string_clone(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("SCORE: %d", state->current_score)));

          // show scoring notifications (for interesting scoring reasons like picking up points or killing an enemy)
          // you'll gradually accumulate score just from surviving on a map...
          // NOTE: hitmarker scores are rendered on the game layer.
          {
            for (s32 index = 0; index < state->score_notifications.size; ++index) {
              auto& s = state->score_notifications[index];
              auto score_text =
                string_clone(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("%d", s.additional_score)));
              s.lifetime.update(dt);

              if (s.lifetime.triggered()) {
                state->score_notifications.pop_and_swap(index);
                continue;
              }

              render_commands_push_text(ui_render_commands,
                font1,
                2,
                V2(ui_cursor_x_left + 25 + font_cache_text_width(font, string_literal("SCORE: "), 2),
                  45 + normalized_sinf(s.lifetime.percentage()) * -GAMEPLAY_UI_SCORE_NOTIFICATION_RISE_AMOUNT),
                score_text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
          }

          render_commands_push_text(
            ui_render_commands, 
            font, 
            2, 
            V2(ui_cursor_x_left + 20, ui_cursor_y),
            text, 
            color32f32(1, 1, 1, 1), 
            BLEND_MODE_ALPHA
          );
          ui_cursor_y += 30;
        }
        // Render_Time
        {
          auto font = resources->get_font(MENU_FONT_COLOR_STEEL);
          auto font1 = resources->get_font(MENU_FONT_COLOR_GOLD);
          s32 hours = 0;
          s32 minutes = 0;
          s32 seconds = 0;
          {
            seconds = (s32)state->current_stage_timer % 60;
            minutes = (s32)state->current_stage_timer / 60;
            hours = minutes / 60;
          }
          auto text = string_clone(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("TIME: %02d:%02d:%02d", hours, minutes, seconds)));
          render_commands_push_text(
            ui_render_commands, 
            font, 
            2, 
            V2(ui_cursor_x_left + 20, ui_cursor_y),
            text, 
            color32f32(1, 1, 1, 1), 
            BLEND_MODE_ALPHA
          );
          ui_cursor_y += 30;
        }

        // Render_Lives
        {
          auto font = resources->get_font(MENU_FONT_COLOR_LIME);
          f32  lives_widget_position_x = (ui_cursor_x_left + 20);
          render_commands_push_text(
            ui_render_commands, 
            font, 
            2, 
            V2(lives_widget_position_x, ui_cursor_y), 
            string_literal("LIVES"), 
            color32f32(1, 1, 1, 1), 
            BLEND_MODE_ALPHA
          );
          ui_cursor_y += 30;
          V2   lives_widget_position = V2(lives_widget_position_x, ui_cursor_y);
          for (unsigned index = 0; index < MAX_TRIES_ALLOWED; ++index) {
            if (index && (index % 5) == 0) {
              lives_widget_position.y += 36;
              lives_widget_position.x = lives_widget_position_x;
            }

            // ProcessParticleSpawnRequestQueue
            state->process_particle_spawn_request_queue(resources, index, lives_widget_position);

            auto destination_rect =
              rectangle_f32(
                lives_widget_position.x,
                lives_widget_position.y + sinf(Global_Engine()->global_elapsed_time) * 3,
                32,
                32
              );

            auto modulation = color32f32(1, 1, 1, 1);
            auto image = resources->ui_hp_icons[UI_HP_ICON_TYPE_LIVING];

            if ((index + 1) > state->tries) {
              image = resources->ui_hp_icons[UI_HP_ICON_TYPE_DEAD];
            }

            if ((index + 1) > state->max_tries) {
              modulation = color32f32(0.15f, 0.15f, 0.15f, 1);
            }
            else {
              if (state->show_damage_player_will_take) {
                if ((index + 1) > state->player_damage_level_taken) {
                  modulation = color32f32(0.85f, 0.05f, 0.18f, 1);
                }
              }
            }

            auto image_buffer = graphics_assets_get_image_by_id(&resources->graphics_assets, image);

            render_commands_push_image_ext2(
              ui_render_commands,
              image_buffer,
              destination_rect,
              RECTANGLE_F32_NULL,
              modulation,
              V2(0, 0),
              0,
              0,
              NO_FLAGS,
              BLEND_MODE_ALPHA
            );
            lives_widget_position.x += 35;
          }
          ui_cursor_y = lives_widget_position.y + 36 + 15;
        }

        // Render Burst Meter
        {
          auto font = resources->get_font(MENU_FONT_COLOR_SKYBLUE);
          f32 widget_x = ui_cursor_x_left + 20;
          f32 bar_max_width = 165;
          f32 player_charge_percentage = state->player.get_burst_charge_percent();
          s32 tier_count = get_burst_mode_rank_count();
          auto current_tier = state->player.get_burst_rank();
          f32 percent_per_tier = 1.0f / tier_count;
          // pick better colors.
          local color32u8 bar_portion_colors[] = {
            color32u8(200, 200, 222, 255),
            color32u8(225, 200, 200, 255),
            color32u8(235, 140, 140, 255),
            color32u8(255, 0, 0, 255),
          };

          render_commands_push_text(
            ui_render_commands,
            font,
            2,
            V2(widget_x, ui_cursor_y),
            string_literal("BURST CHARGE"),
            color32f32(1, 1, 1, 1),
            BLEND_MODE_ALPHA
          );
          ui_cursor_y += 25;
          render_commands_push_quad(
            ui_render_commands,
            rectangle_f32(widget_x, ui_cursor_y, bar_max_width, 15),
            color32u8(12,19,12,255),
            BLEND_MODE_ALPHA
          );

          if (state->player.burst_charge_halt_regeneration) {
              f32 alpha;

              if ((state->player.burst_charge_flash_count % 2) == 0) {
                  alpha = state->player.burst_charge_halt_flash_t/(PLAYER_BURST_FLASH_T*0.8);
              } else {
                  alpha = 1.0 - state->player.burst_charge_halt_flash_t/(PLAYER_BURST_FLASH_T*0.8);
              }

              alpha = clamp<f32>(alpha, 0.0f, 1.0f);
              color32u8 color = color32u8(80, 45, 220, 255 * alpha); // better color please?
              render_commands_push_quad(
                  ui_render_commands,
                  rectangle_f32(widget_x, ui_cursor_y, bar_max_width, 15),
                  color,
                  // color32u8(12,19,12,255),
                  BLEND_MODE_ALPHA
              );
          } else {
              f32 x_cursor = widget_x;
              f32 slice_width = bar_max_width / tier_count;
              s32 tier_index = 0;
              for (tier_index = 0; tier_index < current_tier; ++tier_index) {
                  render_commands_push_quad(
                      ui_render_commands,
                      rectangle_f32(x_cursor, ui_cursor_y, slice_width, 15),
                      bar_portion_colors[tier_index],
                      BLEND_MODE_ALPHA
                  );
                  x_cursor += slice_width;
              }

              f32 currently_filled_upto_percent = (f32)tier_index/tier_count;
              render_commands_push_quad(
                  ui_render_commands,
                  rectangle_f32(
                      x_cursor, ui_cursor_y, 
                      slice_width * (player_charge_percentage-currently_filled_upto_percent)/percent_per_tier,
                      15),
                  bar_portion_colors[tier_index],
                  BLEND_MODE_ALPHA
              );
          }
          ui_cursor_y += 20;
        }

        // Render Boss HP
        {
          state->boss_health_displays.position = V2(state->play_area.x, 10);
          state->boss_health_displays.update(this->state, dt);
          state->boss_health_displays.render(ui_render_commands, this->state);
        }
        // UpdateRender_Gameplay_Alert
        {
          auto& alert = state->game_alert;

          if (alert.enabled) {
            if (alert.timer > 0.0f) {
              const f32 default_alert_scale = 4.0f;
              const f32 alpha = saturate<f32>(alert.timer/(alert.timer_max / 6.0f));
              auto font = resources->get_font(alert.font_variation);

              string text = string_from_cstring(alert.text);
              f32 text_width = font_cache_text_width(font, text, default_alert_scale);
              f32 text_height = font_cache_text_height(font) * default_alert_scale;

              render_commands_push_text_wobbly(
                ui_render_commands, font,
                default_alert_scale,
                V2(state->play_area.x + state->play_area.width/2 - text_width/2, state->play_area.height/2 - text_height/2),
                text,
                color32f32(1, 1, 1, alpha), BLEND_MODE_ALPHA
              );

              alert.timer -= dt;
            }
            else {
              alert.enabled = false;
            }
          }
        }

        // Replay_Demo_UI
        if (state->recording.in_playback && this->state->ui_state == UI_STATE_INACTIVE) {
          auto& viewer_ui = state->demo_viewer;
          //f32 y = play_area_height - 180;
          f32 y = ui_cursor_y;

          auto media_button_string =
            (viewer_ui.paused) ?
            string_literal("[PLAY]") :
            string_literal("[PAUSE]");

          GameUI::set_ui_id(0);
          GameUI::begin_frame(ui_render_commands, &resources->graphics_assets);
          GameUI::set_wobbly_contribution(1.0f);

          bool demo_recording_ui_allowed = !state->triggered_stage_completion_cutscene;

          if (GameUI::button(V2(ui_cursor_x_left + 20, y), string_literal("[RESTART]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
            // partial clean up of resources...
            state->unload_all_script_loaded_resources(this->state, this->state->resources);
            state->prng = state->recording.old_prng;
            state->recording.playback_frame_index = 0;
            state->recording.frames_run = 0;
            reset_stage_simulation_state();
          }
          y += 30;
#ifndef RELEASE
          if (GameUI::button(V2(ui_cursor_x_left + 20, y), string_literal("[TO END]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
            viewer_ui.arbitrary_frame_visit = true;
            simulate_game_frames_until(state->recording.frame_count);
            viewer_ui.arbitrary_frame_visit = false;
            viewer_ui.paused = true;
          }
          y += 30;
#endif
          if (GameUI::button(V2(ui_cursor_x_left + 20, y), media_button_string, color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
            viewer_ui.paused ^= 1;
          }
          y += 30;
          if (GameUI::button(V2(ui_cursor_x_left + 20, y), string_from_cstring(format_temp("[TIMESCALE %f]", replay_timescale_choices[viewer_ui.timescale_index])), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
            viewer_ui.timescale_index += 1;
            if (viewer_ui.timescale_index >= array_count(replay_timescale_choices)) {
              viewer_ui.timescale_index = 0;
            }
          }
          y += 30;
          GameUI::label(V2(ui_cursor_x_left + 20, y), string_from_cstring(format_temp("FRAME %d/%d", state->recording.playback_frame_index + 1, state->recording.frame_count)), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed);
          y += 30;
#ifndef RELEASE
          if (viewer_ui.paused) {
            int  desired_frame = state->recording.playback_frame_index;
            bool change_to_frame = false;


            f32 width_of_biggest_button = font_cache_text_width(resources->get_font(MENU_FONT_COLOR_GOLD), string_literal("[-5]"), 2) * 1.2;
            if (GameUI::button(V2(ui_cursor_x_left + 20 + width_of_biggest_button * 0, y), string_literal("[-]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
              desired_frame -= 1;
              change_to_frame = true;
            }
            if (GameUI::button(V2(ui_cursor_x_left + 20 + width_of_biggest_button * 1, y), string_literal("[-5]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
              desired_frame -= 5;
              change_to_frame = true;
            }
            if (GameUI::button(V2(ui_cursor_x_left + 20 + width_of_biggest_button * 2, y), string_literal("[+5]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
              desired_frame += 5;
              change_to_frame = true;
            }
            if (GameUI::button(V2(ui_cursor_x_left + 20 + width_of_biggest_button * 3, y), string_literal("[+]"), color32f32(1, 1, 1, 1), 2, demo_recording_ui_allowed) == WIDGET_ACTION_ACTIVATE) {
              desired_frame += 1;
              change_to_frame = true;
            }

            if (change_to_frame) {
              viewer_ui.arbitrary_frame_visit = true;
              simulate_game_frames_until(desired_frame);
              viewer_ui.arbitrary_frame_visit = false;
            }
          }
#endif

          GameUI::end_frame();
          GameUI::update(dt);
        }
      }
    }
    {
        state->ui_particle_pool.draw(ui_render_commands, resources);
    }
    // Rendering Dialogue UI
    bool in_conversation = this->state->dialogue_state.in_conversation;

    // main game update things
    if (!state->paused_from_death &&
        this->state->ui_state == UI_STATE_INACTIVE &&
        (!state->triggered_stage_completion_cutscene ||
        state->triggered_stage_completion_cutscene && state->complete_stage.stage < GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN)) {
        auto update_packet_data = (Entity_Loop_Update_Packet*)Global_Engine()->scratch_arena.push_unaligned(sizeof(Entity_Loop_Update_Packet));
        update_packet_data->dt = FIXED_TICKTIME;
        update_packet_data->game_state = this->state;

        float timescale      = 1.0f;
        bool  simulate_frame = true;

        // Special case frame simulation
        if (state->recording.in_playback) {
            timescale *= replay_timescale_choices[state->demo_viewer.timescale_index];
            if (state->demo_viewer.paused) {
                simulate_frame = false;
            }
        }

        if (simulate_frame) {
            // Avoid total death spiral...
            if (dt >= (1.0f / 24.0f)) {
                dt = 1.0f/60.0f;
            }

            state->fixed_tickrate_timer += dt * timescale;

            while (state->fixed_tickrate_timer >= FIXED_TICKTIME) {
                simulate_game_frame(update_packet_data);
                state->fixed_tickrate_timer -= FIXED_TICKTIME;

                if (!this->state->dialogue_state.in_conversation) {
                    if (state->complete_stage.stage == GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE) {
                      state->current_stage_timer += FIXED_TICKTIME;
                    }
                }
            }

            // NOTE: do not advance time or do any animation deviations while
            // in conversation mode.
            if (!this->state->dialogue_state.in_conversation) {
                state->fixed_tickrate_remainder = clamp<f32>(state->fixed_tickrate_timer/(dt), 0.0f, 1.0f);
            }
        }
    }

    // out of tick things
    // death animation.
    // NOTE: technically this will happen as long as the flag is appropriately triggered
    // and the flashing happens independently of the particle systems.
    // NOTE: need to see what this actually means about game frame timings.
    {
        auto& deathanimation_data = this->state->deathanimation_data;

        if (deathanimation_data.phase != DEATH_ANIMATION_PHASE_INACTIVE) {
            // NOTE: I need to update the particles here because the particles technically update during the
            // regular fixed frame update.
            // NOTE: the original fixed particle update is guaranteed to not happen by this point so this is not a big deal.
            {
                if (deathanimation_data.player_explosion_emitter.flags & PARTICLE_EMITTER_FLAGS_ACTIVE) {
                    deathanimation_data.player_explosion_emitter.update(&state->death_particle_pool, &state->prng_unessential, dt);
                }

                state->death_particle_pool.update(this->state, dt);
            }
        }
        switch (deathanimation_data.phase) {
            case DEATH_ANIMATION_PHASE_INACTIVE: {
                // nothing needs to happen.
            } break;
            case DEATH_ANIMATION_PHASE_FLASH: {
                if (deathanimation_data.flash_count == 0 && deathanimation_data.flashing == false) {
                    deathanimation_data.phase = DEATH_ANIMATION_PHASE_LINGER;
                } else {
                    deathanimation_data.flash_t += dt;
                    if (deathanimation_data.flash_t >= DEATH_ANIMATION_MAX_T_PER_FLASH) {
                        deathanimation_data.flash_t = 0.0f;
                        deathanimation_data.flashing ^= true;

                        if (deathanimation_data.flashing) {
                            deathanimation_data.flash_count -= 1;
                        }
                    }
                }
            } break;
            case DEATH_ANIMATION_PHASE_LINGER: {
                if (deathanimation_data.t >= DEATH_ANIMATION_LINGER_MAX_T) {
                    switch_ui(UI_STATE_DEAD_MAYBE_RETRY);
                    deathanimation_data.phase = DEATH_ANIMATION_PHASE_INACTIVE;
                } else {
                    deathanimation_data.t += dt;
                }
            } break;
        }
    }

    update_and_render_dialogue_ui(ui_render_commands, dt);
    handle_ui_update_and_render(ui_render_commands, dt);

    // Handle transitions or stage specific data
    // so like cutscene/coroutine required sort of behaviors...
    if (this->state->ui_state == UI_STATE_INACTIVE) {
        if (!Transitions::fading()) {
            if (state->intro.stage != GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE) {
                ingame_update_introduction_sequence(ui_render_commands, resources, dt);
            } else {
                bool can_finish_stage = state->stage_completed;

                if (!state->triggered_stage_completion_cutscene && can_finish_stage) {
                    state->triggered_stage_completion_cutscene = true;
                    state->stage_completed                     = false;

                    Audio::play_music_transition_into(resources->title_reprisal_music, 500, 500);
                    state->complete_stage.begin_sequence();
                    state->player.reset_movement();
                }

                if (state->triggered_stage_completion_cutscene) {
                    ingame_update_complete_stage_sequence(ui_render_commands, resources, dt);
                }
            }
        }
    }

    // main game rendering
    // draw stage specific things if I need to.
    { 
        int play_area_width = state->play_area.width;
        int play_area_height = state->play_area.height;
        auto bkg_color = color32u8(32 * 2, 45 * 2, 80 * 2, 255);
        render_commands_push_quad(game_render_commands,
                                  rectangle_f32(0,
                                                0,
                                                play_area_width,
                                                play_area_height),
                                  bkg_color, BLEND_MODE_ALPHA);

        stage_draw(&state->stage_state, dt, game_render_commands, this->state);
    }

    
    Transitions::update_and_render(ui_render_commands, dt);

    // HACKME(jerry)
    if (this->state->screen_mode == GAME_SCREEN_INGAME) {
      if (this->state->ui_state == UI_STATE_PAUSED) {
        Audio::pause_music();
      }
      else if (this->state->ui_state == UI_STATE_INACTIVE) {
        Audio::resume_music();
      }
    }


    state->simple_scrollable_backgrounds.draw_background(game_render_commands, this->state->resources);
    state->update_and_render_all_background_scriptable_render_objects(this->state->resources, game_render_commands, dt);
    {
        for (unsigned border_index = 0; border_index < PLAY_AREA_EDGE_ID_COUNT; ++border_index) {
            auto& border_flash = state->border_flashes[border_index];

            if (border_flash.flash_id_type == BORDER_FLASH_ID_TYPE_NONE) {
                continue;
            }

            const f32 BORDER_DISPLAY_RADIUS = 32.0f;

            local const color32f32 border_flash_colors[BORDER_FLASH_ID_TYPE_COUNT] = {
                color32f32(0,0,0,0),
                color32f32(0.5, 0.5, 1, 0.5),
                color32f32(0.75, 0.15, 0.12, 0.5),
                RGBA32f32(239, 239, 44, 128),
                RGBA32f32(195, 255, 104, 128),
                RGBA32f32(239, 239, 44, 128),
            };

            auto image = resources->ui_border_effect[border_flash.flash_id_type-1];
            image_buffer* border_flash_image = graphics_assets_get_image_by_id(&resources->graphics_assets, image);
            rectangle_f32 border_rectangle;

            switch (border_index) {
                case PLAY_AREA_EDGE_ID_TOP: {
                    border_rectangle.x = 0;
                    border_rectangle.y = 0;
                    border_rectangle.w = PLAY_AREA_WIDTH_PX;
                    border_rectangle.h = BORDER_DISPLAY_RADIUS;
                } break;
                case PLAY_AREA_EDGE_ID_BOTTOM: {
                    border_rectangle.x = 0;
                    border_rectangle.y = 480 - BORDER_DISPLAY_RADIUS;
                    border_rectangle.w = PLAY_AREA_WIDTH_PX;
                    border_rectangle.h = BORDER_DISPLAY_RADIUS;
                } break;
                case PLAY_AREA_EDGE_ID_LEFT: {
                    border_rectangle.x = 0;
                    border_rectangle.y = 0;
                    border_rectangle.w = BORDER_DISPLAY_RADIUS;
                    border_rectangle.h = 480;
                } break;
                case PLAY_AREA_EDGE_ID_RIGHT: {
                    border_rectangle.x = PLAY_AREA_WIDTH_PX - BORDER_DISPLAY_RADIUS;
                    border_rectangle.y = 0;
                    border_rectangle.w = BORDER_DISPLAY_RADIUS;
                    border_rectangle.h = 480;
                } break;
                    bad_case;
            }

            if (!border_flash.delay_between_flash) {
                auto flash_color = border_flash_colors[border_flash.flash_id_type];
                V2 border_rectangle_center = V2((border_rectangle.x*2 + border_rectangle.w)/2,
                                                (border_rectangle.y*2 + border_rectangle.h)/2);

                render_commands_push_quad(
                    game_render_commands, border_rectangle,
                    color32f32_to_color32u8(flash_color),
                    BLEND_MODE_ALPHA);

                if (border_flash_image) {
                    rectangle_f32 dest;
                    dest.x = border_rectangle_center.x - border_flash_image->width/2;
                    dest.y = border_rectangle_center.y - border_flash_image->height/2;
                    dest.w = border_flash_image->width;
                    dest.h = border_flash_image->height;
                    render_commands_push_image(
                        game_render_commands,
                        border_flash_image,
                        dest,
                        RECTANGLE_F32_NULL,
                        color32f32(flash_color.r, flash_color.g, flash_color.b, 1.0f),
                        0, BLEND_MODE_ALPHA
                    );
                } else {
                    auto font = resources->get_font(MENU_FONT_COLOR_STEEL);
                    render_commands_push_text(
                        game_render_commands, font, 2,
                        border_rectangle_center,
                        border_flash_id_strings[border_flash.flash_id_type],
                        color32f32_WHITE,
                        BLEND_MODE_ALPHA);
                }
            }

            if (border_flash.per_flash_length >= 0.0f) {
                border_flash.per_flash_length -= dt;
            } else {
                border_flash.per_flash_length = BORDER_NOTIFY_PER_FLASH_LENGTH;
                border_flash.flash_count -= 1;
                border_flash.delay_between_flash ^= true;
            }

            if (border_flash.flash_count <= 0) {
                border_flash.flash_id_type = BORDER_FLASH_ID_TYPE_NONE;
            }
        }

        for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
            auto& h = state->explosion_hazards[i];
            h.draw(this->state, game_render_commands, resources);
        }

        for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
            auto& h = state->laser_hazards[i];
            h.draw(this->state, game_render_commands, resources);
        }

        for (int i = 0; i < (int)state->bullets.size; ++i) {
            auto& b = state->bullets[i];
            b.draw(this->state, game_render_commands, resources);
        }

        for (int i = 0; i < (s32)state->enemies.size; ++i) {
            auto& e = state->enemies[i];
            e.draw(this->state, game_render_commands, resources);
        }

        for (int i = state->death_explosions.size-1; i >= 0; --i) {
          auto& e = state->death_explosions[i];
          if (e.dead()) {
            state->death_explosions.pop_and_swap(i);
          }
          else {
            e.draw(this->state, game_render_commands, resources);
            e.update(this->state, dt);
          }
        }

        for (int i = 0; i < (s32)state->pickups.size; ++i) {
            auto& pe = state->pickups[i];
            pe.draw(this->state, game_render_commands, resources);
        }

        {
            auto font1 = resources->get_font(MENU_FONT_COLOR_GOLD);

            for (s32 index = 0; index < state->hit_score_notifications.size; ++index) {
                auto& s = state->hit_score_notifications[index];
                auto text = string_from_cstring(format_temp("%d", s.additional_score));
                s.lifetime.update(dt);

                if (s.lifetime.triggered()) {
                    state->hit_score_notifications.pop_and_swap(index);
                    continue;
                }

                f32 text_size  = 2 + s.lifetime.percentage();
                f32 text_width = font_cache_text_width(font1, text, text_size);
                render_commands_push_text(game_render_commands,
                                          font1,
                                          text_size,
                                          s.where + V2(-text_width/2, normalized_sinf(s.lifetime.percentage()) * -GAMEPLAY_UI_SCORE_NOTIFICATION_RISE_AMOUNT),
                                          text, color32f32(1,1,1, 1 - s.lifetime.percentage()), BLEND_MODE_ALPHA); 
            }
        }

        state->player.draw(this->state, game_render_commands, resources);
        state->pet.draw(this->state, game_render_commands, resources);
        state->particle_pool.draw(game_render_commands, resources);
        state->death_particle_pool.draw(game_render_commands, resources);
        state->stage_exit_particle_pool.draw(game_render_commands, resources);
        state->update_and_render_focus_mode_hitboxes(this->state, game_render_commands, resources, dt);
    }
    state->simple_scrollable_backgrounds.draw_foreground(game_render_commands, this->state->resources);
    state->update_and_render_all_foreground_scriptable_render_objects(this->state->resources, game_render_commands, dt);
}

bool Game_Resources::sprite_id_should_be_rotated(sprite_id id) {
    for (int i = 0; i < projectile_sprites_requiring_rotation_count; ++i) {
        auto projectile_sprite_id = projectile_sprites[projectile_sprites_requiring_rotation[i]];
        if (projectile_sprite_id.index == id.index) {
            return true;
        }
    }

    return false;
}

void Game_State::set_led_primary_color(color32u8 color) {
    led_state.primary_color = color;
}

void Game_State::set_led_target_color_anim(color32u8 color, f32 anim_length, bool overridable, bool fade_back_when_done) {
    if (led_state.can_override) {
        set_led_target_color_anim_force(color, anim_length, overridable, fade_back_when_done);
    }
}

void Game_State::set_led_target_color_anim_force(color32u8 color, f32 anim_length, bool overridable, bool fade_back_when_done) {
    led_state.animation_t         = 0.0f;
    led_state.animation_max_t     = anim_length;
    led_state.target_color        = color;
    led_state.fade_back_when_done = fade_back_when_done;
    led_state.fade_phase          = 0;
    led_state.can_override        = overridable;
    led_state.finished_anim       = false;
}

void Game::check_and_unlock_perfect_clear_achievement(void) {
    auto perfect_clear_achievement = Achievements::get(ACHIEVEMENT_ID_UNTOUCHABLE);

    bool all_previous_unlocked = true;
    for (s32 achievement_id = ACHIEVEMENT_ID_STAGE1_FLAWLESS;
         achievement_id <= ACHIEVEMENT_ID_STAGE3_FLAWLESS;
         ++achievement_id) {
        auto achievement = Achievements::get(achievement_id);

        if (!achievement->complete()) {
            all_previous_unlocked = false;
            break;
        }
    }

    if (all_previous_unlocked) {
        perfect_clear_achievement->report();
    }
}

void Game::check_and_unlock_platinum_achievement(void) {
    auto platinum_achievement = Achievements::get(ACHIEVEMENT_ID_PLATINUM);
    {
        // NOTE: the platinum_achievement is the last achievement id
        bool all_previous_unlocked = true;
        for (s32 i = 0; i < ACHIEVEMENT_ID_PLATINUM; ++i) {
            auto achievement = Achievements::get(i);
            if (!achievement->complete()) {
                all_previous_unlocked = false;
                break;
            }
        }

        if (all_previous_unlocked) {
            platinum_achievement->report();
        }
    }
}

void Game::update_and_render(Graphics_Driver* driver, f32 dt) {
    V2 resolution = driver->resolution();
    bool take_screenshot = false;

    auto game_render_commands = render_commands(&Global_Engine()->scratch_arena, 50000, camera(V2(0, 0), 1));
    auto ui_render_commands   = render_commands(&Global_Engine()->scratch_arena, 16384,  camera(V2(0, 0), 1));

    {
        ui_render_commands.screen_width  = game_render_commands.screen_width  = resolution.x;
        ui_render_commands.screen_height = game_render_commands.screen_height = resolution.y;
    }

    if (Action::is_pressed(ACTION_SCREENSHOT)) {
        _debugprintf("Saved a screenshot!"); // picture sound?
        take_screenshot = true;
    }

    state->coroutine_tasks.schedule_by_type(state, dt, GAME_TASK_SOURCE_UI);
    state->coroutine_tasks.schedule_by_type(state, dt, GAME_TASK_SOURCE_GAME);
    // shouldn't happen during replays.
    if (this->state->coroutine_tasks.need_to_address_error()) {
        switch_ui(UI_STATE_REVIEW_SCRIPT_ERROR);
    }

    switch (state->screen_mode) {
        case GAME_SCREEN_TITLE_SCREEN: {
            update_and_render_game_title_screen(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_OPENING: {
            update_and_render_game_opening(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_ENDING: {
            update_and_render_game_ending(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_MAIN_MENU: {
            update_and_render_game_main_menu(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_INGAME: {
            update_and_render_game_ingame(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_CREDITS: {
            update_and_render_game_credits(&game_render_commands, &ui_render_commands, dt);
        } break;
        case GAME_SCREEN_TRAILER_CUTAWAY: {
            update_and_render_game_trailer_cutaway_screen(&game_render_commands, &ui_render_commands, dt);
        } break;
    }

    // Achievement related updates.
    {
        check_and_unlock_perfect_clear_achievement();
        check_and_unlock_platinum_achievement();

        // Polling based achievement updating.
        {
            // also updating the achievement sprites while I'm at it.
            {
                resources->locked_trophy_sprite_instance.animate(&resources->graphics_assets, dt, 0.10f);
                resources->unlocked_trophy_sprite_instance.animate(&resources->graphics_assets, dt, 0.10f);
            }
            for (s32 i = 0; i < array_count(achievement_list); ++i) {
                auto achievement = Achievements::get(i);
                if (achievement->complete() && achievement->notify_unlock()) {
                    _debugprintf("Hi, you've just unlocked the: %.*s achievement",
                                 achievement->id_name.length,
                                 achievement->id_name.data
                    );

                    notify_new_achievement(achievement->id);
                }
            }
        }
    }
    update_and_render_achievement_notifications(&ui_render_commands, dt);

    driver->consume_render_commands(&game_render_commands);
    driver->consume_render_commands(&ui_render_commands);
    total_playtime += dt;

    // Update controller LED_State
    {
        auto& led_state = state->led_state;
        if (!led_state.finished_anim) {
            if (led_state.fade_back_when_done) {
                switch (led_state.fade_phase) {
                    case 0: { // fade_forward
                        if (led_state.animation_t < led_state.animation_max_t) {
                            led_state.animation_t += dt;
                        } else {
                            led_state.fade_phase = 1;
                        }
                    } break;
                    case 1: { // fade back
                        if (led_state.animation_t > 0) {
                            led_state.animation_t -= dt;
                        } else {
                            led_state.finished_anim = true;
                        }
                    } break;
                }
            } else {
                if (led_state.animation_t < led_state.animation_max_t) {
                    led_state.animation_t += dt;
                } else {
                    led_state.finished_anim = true;
                }
            }
        } else {
            led_state.can_override = true;
            led_state.fade_phase = 0;
            led_state.animation_t  = 0;
        }

        led_state.animation_t = clamp<f32>(led_state.animation_t, 0.0f, led_state.animation_max_t);

        f32 effective_t = led_state.animation_t / led_state.animation_max_t;
        auto present_color = color32u8(
            lerp_f32(led_state.primary_color.r, led_state.target_color.r, effective_t),
            lerp_f32(led_state.primary_color.g, led_state.target_color.g, effective_t),
            lerp_f32(led_state.primary_color.b, led_state.target_color.b, effective_t),
            0
        );

        controller_set_led(Input::get_gamepad(0), present_color.r, present_color.g, present_color.b);
    }

    if (take_screenshot) {
        driver->screenshot((char*)"screenshot.png");
    }

    graphics_assets_commit_unloaded_assets(&resources->graphics_assets);
}

// NOTE: should deduplicate later?
void Game_State::kill_all_bullets() {
    auto state = &gameplay_data;

    for (s32 bullet_index = 0; bullet_index < state->bullets.size; ++bullet_index) {
        auto& b = state->bullets[bullet_index];

        if (b.die)
            continue;

        if (b.source_type == BULLET_SOURCE_PLAYER)
            continue;

        b.kill();
#if 0
        spawn_game_entity_death_particle_emitter(state->particle_emitters, b.position, resources, 1);
#endif
    }
}

void Game_State::kill_all_enemies() {
    auto state = &gameplay_data;

    for (s32 enemy_index = 0; enemy_index < state->enemies.size; ++enemy_index) {
        auto& e = state->enemies[enemy_index];

        if (e.die)
            continue;

        e.kill();
#if 0
        spawn_game_entity_death_particle_emitter(state->particle_emitters, e.position, resources, 0);
        spawn_game_entity_death_particle_emitter(state->particle_emitters, e.position, resources, 1);
#endif
    }
}

void Game_State::convert_bullets_to_score_pickups(float radius, float value) {
    auto state = &gameplay_data;

    for (s32 bullet_index = 0; bullet_index < state->bullets.size; ++bullet_index) {
        auto& b = state->bullets[bullet_index];

        if (b.die)
            continue;

        if (V2_distance_sq(state->player.position, b.position) >= radius*radius)
            continue;

        if (b.source_type == BULLET_SOURCE_PLAYER)
            continue;

        b.kill();
        // This will overflow, but the effect is pretty minor that I don't think it matters tbh...
        // spawn_game_entity_death_particle_emitter(state->particle_emitters, b.position, resources);

        if (state->disable_bullet_to_points) {
            continue;
        }
        auto pe = pickup_score_entity(
            this,
            b.position,
            b.position,
            value
        );
        pe.seek_towards_player = true;
        pe.sprite.modulation = color32f32(242.0f / 255.0f, 121.0f / 255.0f, 53.0f / 255.0f, 1.0f);
        state->add_pickup_entity(pe);
    }
}

void Game_State::convert_enemies_to_score_pickups(float radius, float value) {
    auto state = &gameplay_data;

    for (s32 enemy_index = 0; enemy_index < state->enemies.size; ++enemy_index) {
        auto& e = state->enemies[enemy_index];

        if (e.die)
            continue;

        if (V2_distance_sq(state->player.position, e.position) >= radius*radius)
            continue;

        e.kill();
        spawn_game_entity_death_particle_emitter(state->particle_emitters, e.position, resources, 1);

        if (state->disable_enemy_to_points) {
            continue;
        }

        auto pe = pickup_score_entity(
            this,
            e.position,
            e.position,
            e.score_value/2 + value
        );
        pe.sprite.modulation = color32f32(254.0f / 255.0f, 121.0f / 255.0f, 104.0f / 255.0f, 1.0f);
        pe.seek_towards_player = true;
        state->add_pickup_entity(pe);
    }
}

void Game::kill_all_bullets() {
    state->kill_all_bullets();
}
void Game::kill_all_enemies() {
    state->kill_all_enemies();
}
void Game::convert_bullets_to_score_pickups(float radius, float value) {
    state->convert_bullets_to_score_pickups(radius, value);
}
void Game::convert_enemies_to_score_pickups(float radius, float value) {
    state->convert_enemies_to_score_pickups(radius, value);
}

void Game::handle_bomb_usage(f32 dt) {
    auto state = &this->state->gameplay_data;

    if (!state->queue_bomb_use) {
        return;
    }

    auto pet_data = game_get_pet_data(state->selected_pet);
    auto& player  = state->player;

    player.handle_bomb_usage(this->state, pet_data->bomb_pattern_id);
    state->burst_usage_count++;
    state->queue_bomb_use = false;
}

void Game::on_player_death() {
    auto state = &this->state->gameplay_data;
    _debugprintf("On finish death");

    Audio::play(resources->hit_sounds[0]);
    state->death_count++;
    if (safely_resurrect_player()) {
        _debugprintf("Resurrected player?");
    } else {
        // TODO: Will have to change if
        // I have "CONTINUES" support
        _debugprintf("record status (%d) %d\n", state->recording.frame_count, state->recording.in_playback);
        if (state->recording.in_playback) {
            _debugprintf("TODO handle continues?");
            state->triggered_stage_completion_cutscene = true;
            state->complete_stage.begin_sequence(true);
        } else {
            // Start the game over fade animation.
            // switch_ui(UI_STATE_DEAD_MAYBE_RETRY);

            // Start death animation.
            {
                state->player.visible = false;
            }
            {
              Audio::stop_music();
              auto& deathanimation_data                           = this->state->deathanimation_data;
                deathanimation_data.phase                           = DEATH_ANIMATION_PHASE_FLASH;
                deathanimation_data.t                               = 0.0f;
                deathanimation_data.flash_t                         = 0.0f;
                deathanimation_data.flash_count                     = DEATH_ANIMATION_FLASH_AMOUNT;
                deathanimation_data.flashing                        = false;
                deathanimation_data.player_explosion_emitter.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE;
                deathanimation_data.player_explosion_emitter.shape  = particle_emit_shape_point(state->player.position);
                deathanimation_data.player_explosion_emitter.reset();
                Audio::play(resources->death_sound);
            }
        }
    }
}

void Game::cleanup_dead_entities(void) {
#if 0
    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->laser_hazards.size; ++i) {
                auto& h = state->laser_hazards[i];
                if (h.die) {state->laser_hazards.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->pickups.size; ++i) {
                auto& pe = state->pickups[i];
                if (pe.die) {state->pickups.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->bullets.size; ++i) {
                auto& b = state->bullets[i];
                b.disable_all_particle_emitters();
                if (b.die) {state->bullets.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->enemies.size; ++i) {
                auto& e = state->enemies[i];
                e.disable_all_particle_emitters();
                if (e.die) {state->enemies.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->explosion_hazards.size; ++i) {
                auto& h = state->explosion_hazards[i];
                if (h.exploded) {state->explosion_hazards.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::synchronize_tasks();
#else
    Gameplay_Data* state = &this->state->gameplay_data;
    for (int i = 0; i < state->laser_hazards.size; ++i) {
        auto& h = state->laser_hazards[i];
        if (h.die) {
          state->laser_hazards.pop_and_swap(i);
        }
    }
    for (int i = 0; i < state->pickups.size; ++i) {
        auto& pe = state->pickups[i];
        if (pe.die) {
          state->pickups.pop_and_swap(i);
        }
    }
    for (int i = 0; i < state->bullets.size; ++i) {
        auto& b = state->bullets[i];
        b.disable_all_particle_emitters();
        if (b.die) {
          state->bullets.pop_and_swap(i);
        }
    }
    for (int i = 0; i < state->enemies.size; ++i) {
        auto& e = state->enemies[i];
        e.disable_all_particle_emitters();
        if (e.die) {
          state->enemies.pop_and_swap(i);
        }
    }
    for (int i = 0; i < state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];
        if (h.exploded) {
          Audio::play(resources->random_explosion_sound(&state->prng_unessential), Audio::get_volume_sound() * 0.7 * -128);
          state->explosion_hazards.pop_and_swap(i);
        }
    }
#endif
}

void Game::handle_all_dead_entities(f32 dt) {
    auto state = &this->state->gameplay_data;

    /*
      NOTE:

      The transition system uses the callback system which can "pause" execution and I only want to
      clean up entities consistently so I have to do this weird thing.
    */
    if (state->player.die) {
        if (state->recording.in_playback &&
            (state->demo_viewer.arbitrary_frame_visit ||
             state->demo_viewer.timescale_index != DEFAULT_DEMO_VIEWER_TIMESCALE_INDEX))
        {
            // NOTE:
            // unideal simulation conditions.
            // the transition would interrupt the simulation, so I cannot play the transition
            // without risking a desync.

            on_player_death();
            cleanup_dead_entities();
        } else {
            // NOTE(jerry): 2/7/25
            // I don't really like using the transition system to do this, since
            // it can screw with how I reason about the determinism, so that's why
            // this has been so special-cased, as otherwise a separate transition/animation
            // system would need to be written just for this transition.
            if (state->paused_from_death == false) {
                state->paused_from_death = true;
                Transitions::do_color_transition_in(color32f32(1, 1, 1, 1), 0.10f, 0.05f);

                Transitions::register_on_finish(
                    [&](void*) {
                        Transitions::do_color_transition_out(
                            color32f32(1, 1, 1, 1),
                            0.1f,
                            0.025f
                        );

                        Transitions::register_on_finish(
                            [&](void*) {
                                on_player_death();
                                cleanup_dead_entities();
                            }
                        );
                    }
                );
            }
        }
    } else {
        cleanup_dead_entities();
    }
}

void Game::handle_all_lasers(f32 dt) {
    auto state = &this->state->gameplay_data;
    for (int i = 0; i < state->laser_hazards.size; ++i) {
        auto& h = state->laser_hazards[i];

        if (h.ready()) {
            // played sound + camera hit
            if (!h.already_emitted) {
                h.already_emitted = true;
                Audio::play(resources->random_explosion_sound(&state->prng_unessential), Audio::get_volume_sound() * 0.7 * -128);
                controller_rumble(Input::get_gamepad(0), 0.5f, 0.5f, 150);
                camera_traumatize(&state->main_camera, 0.045f);
            }

            auto laser_rect  = h.get_rect(&state->play_area);
            auto player_rect = state->player.get_rect();

            if (!DebugUI::godmode_enabled()) {
                if (rectangle_f32_intersect(player_rect, laser_rect)) {
                    state->player.kill();
                }
            }
        }
    }
}

// NOTE: hazards only work on players which is neat.
void Game::handle_all_explosions(f32 dt) {
    auto state = &this->state->gameplay_data;
    for (int i = 0; i < state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];

        if (h.exploded) {
            controller_rumble(Input::get_gamepad(0), 0.8f, 0.8f, 150);
            camera_traumatize(&state->main_camera, 0.15f);
            // check explosion against all entities
            // by all entities, I just mean the player right now.
            {
                auto explosion_circle = circle_f32(h.position.x, h.position.y, h.radius);
                auto player_circle    = circle_f32(state->player.position.x, state->player.position.y, state->player.scale.x);

                if (!DebugUI::godmode_enabled()) {
                    if (circle_f32_intersect(explosion_circle, player_circle)) {
                        state->player.kill();
                    }
                }
            }
        }
    }
}

image_id Gameplay_Data::script_load_image(Game_Resources* resources, char* where) {
    auto img_id = graphics_assets_load_image(
        &resources->graphics_assets,
        string_from_cstring(where)
    );

    bool already_exists = false;
    for (s32 index = 0; index < script_loaded_images.size; ++index) {
        if (script_loaded_images[index].index == img_id.index) {
            already_exists = true;
            break;
        }
    }

    if (!already_exists) {
        script_loaded_images.push(img_id);
    }

    return img_id;
}

Audio::Sound_ID Gameplay_Data::script_load_sound(Game_Resources* resources, char* where, bool streamed) {
    auto sound_id = Audio::load(where, streamed);
    bool already_exists = false;

    for (s32 index = 0; index < script_loaded_sounds.size; ++index) {
        if (sound_id_match(script_loaded_sounds[index], sound_id)) {
            already_exists = true;
            break;
        }
    }

    if (!already_exists) {
        script_loaded_sounds.push(sound_id);
    }

    return sound_id;
}

void Gameplay_Data::process_particle_spawn_request_queue(Game_Resources* resources, s32 index, V2 current_cursor) {
    Gameplay_Data_Particle_Spawn_Request* target = nullptr;
    bool found = false;
    s32  request_index;

    for (request_index = 0;
         request_index < ui_particle_spawn_queue_count;
         ++request_index) {
        target = ui_particle_spawn_queue + request_index;
        if (target->data == index) {
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    switch (target->type) {
        case GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_NONE: {
        } break;
        case GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_LOST_LIFE: {
            _debugprintf("lost life particle spawn!");
            spawn_game_entity_death_particle_emitter(ui_particle_emitters, current_cursor, resources, 0);
        } break;
        case GAMEPLAY_DATA_PARTICLE_SPAWN_REQUEST_TYPE_GAINED_LIFE: {
            _debugprintf("gained life particle spawn!");
            spawn_game_entity_death_particle_emitter(ui_particle_emitters, current_cursor, resources, 1);
        } break;
    }

    ui_particle_spawn_queue[request_index] =
        ui_particle_spawn_queue[--ui_particle_spawn_queue_count];
}

void Gameplay_Data::spawn_death_explosion(V2 position) {
  auto explosion = death_explosions.alloc();
  explosion->position = position;
  explosion->frame_index = 0;
  explosion->timer = 0;
  explosion->frame_count = 3;
  explosion->scale = random_ranged_float(&prng_unessential, 0.5f, 1.5f);
}

// TODO(jerry): Spatial partition all the bullets some how. Probably going to use another spatial hash.
void Game::handle_all_bullet_collisions(f32 dt) {
    auto state = &this->state->gameplay_data;

    for (s32 bullet_index = 0; bullet_index < state->bullets.size; ++bullet_index) {
        auto& b = state->bullets[bullet_index];
        bool hit_death = false;
        auto bullet_rect = b.get_rect();

        if (b.die) {
            continue;
        }

        if (b.source_type == BULLET_SOURCE_NEUTRAL || b.source_type == BULLET_SOURCE_PLAYER) {
            for (s32 enemy_index = 0; enemy_index < state->enemies.size; ++enemy_index) {
                auto& e = state->enemies[enemy_index];
                auto enemy_rect = e.get_rect();

                if (e.die) {
                    continue;
                }

                if (rectangle_f32_intersect(enemy_rect, bullet_rect)) {
                    e.damage(1);
                    spawn_game_entity_hit_particle_emitter(state->particle_emitters, e.position, resources);

                    if (e.die) {
                        state->notify_score_with_hitmarker(e.score_value * e.death_multiplier, e.position);   

                        if (!(state->recording.in_playback)) {
                            Achievements::get(ACHIEVEMENT_ID_KILLER)->report((s32)1);
                            Achievements::get(ACHIEVEMENT_ID_MURDERER)->report((s32)1);
                            Achievements::get(ACHIEVEMENT_ID_SLAYER)->report((s32)1);
                        }

                        state->spawn_death_explosion(e.position);
                        // adjust sound todo
                        Audio::play(resources->random_explosion_sound(&state->prng_unessential), Audio::get_volume_sound() * 0.80 * -128);
#if 0
                        spawn_game_entity_death_particle_emitter(state->particle_emitters, e.position, resources, 0);
                        spawn_game_entity_death_particle_emitter(state->particle_emitters, e.position, resources, 1);
#endif
                    } else {
                        state->notify_score_with_hitmarker(e.score_value, e.position);
                    }
                    b.die = true;
                    hit_death = true;
                    break;
                }
            }
        }

        if (b.flags & BULLET_FLAGS_BREAKS_OTHER_BULLETS) {
            /*
             * TODO(jerry):
             * need performance enhancement here!
             */
            for (s32 other_bullet_index = 0;
                 other_bullet_index < state->bullets.size;
                 ++other_bullet_index) {
                auto& ob = state->bullets[other_bullet_index];

                if (ob.source_type == b.source_type) {
                    continue;
                }

                if (other_bullet_index == bullet_index) {
                    continue;
                }

                if (ob.die) {
                    continue;
                }

                auto other_bullet_rect = ob.get_rect();

                if (rectangle_f32_intersect(other_bullet_rect, bullet_rect)) {
                    hit_death = true;
                    b.die = true;
                    ob.die = true;
                    state->notify_score_with_hitmarker(15, b.position);
                    break;
                }
            }
        }


        if (!DebugUI::godmode_enabled()) {
            if (!b.die && b.source_type == BULLET_SOURCE_NEUTRAL || b.source_type == BULLET_SOURCE_ENEMY) {
                auto& p = state->player;
                auto player_rect = p.get_rect();

                if (p.burst_absorption_shield_ability_timer > 0) {
                    convert_bullets_to_score_pickups(
                        PLAYER_BURST_SHIELD_ABILITY_RADIUS,
                      500
                    );
                } else {
                    if (rectangle_f32_intersect(player_rect, bullet_rect)) {
                        if (p.kill()) {
                            spawn_game_entity_death_particle_emitter(state->particle_emitters, p.position, resources, 1);
                            b.die = true;
                            hit_death = true;
                        }
                        break;
                    }
                }
            }
        }

        if (b.die && hit_death) {
            auto resources = this->state->resources;
            Audio::play(resources->random_hit_sound(&state->prng));
        }
    }
}

void Game::handle_player_enemy_collisions(f32 dt) {
    auto& p = state->gameplay_data.player;
    auto player_rect = p.get_rect();

    for (s32 enemy_index = 0; enemy_index < state->gameplay_data.enemies.size; ++enemy_index) {
        auto& e = state->gameplay_data.enemies[enemy_index];
        auto enemy_rect = e.get_rect();

        if (e.die) {
            continue;
        }

        if (!DebugUI::godmode_enabled()) {
            if (rectangle_f32_intersect(player_rect, enemy_rect)) {
                p.kill();
            }
        }
    }
}

void Game::handle_player_pickup_collisions(f32 dt) {
    auto& p = state->gameplay_data.player;
    auto player_position = p.get_real_position();
    auto p_pickup_rect_w = 20;
    auto p_pickup_rect_h = 40;

    auto player_rect = rectangle_f32(
      (player_position.x) - p_pickup_rect_w/2,
      (player_position.y) - p_pickup_rect_h/2,
      p_pickup_rect_w,
      p_pickup_rect_h
    );

    for (s32 pickup_index = 0; pickup_index < state->gameplay_data.pickups.size; ++pickup_index) {
        auto& pe          = state->gameplay_data.pickups[pickup_index];
        auto  pickup_rect = pe.get_rect();

        if (pe.die) {
            continue;
        }

        if (rectangle_f32_intersect(player_rect, pickup_rect)) {
            pe.on_picked_up(state);
            Audio::play(resources->score_pickup_sound);
        }
    }
}

// Play_Area
bool Play_Area::is_inside_absolute(rectangle_f32 rect) const {
    return (rectangle_f32_intersect(rect, rectangle_f32(x, 0, width, height)));
}

bool Play_Area::is_inside_logical(rectangle_f32 rect) const {
    return (rectangle_f32_intersect(rect, rectangle_f32(0, 0, width, height)));
}

void Play_Area::set_all_edge_behaviors_to(u8 value) {
    for (int i = 0; i < 4; ++i) {
        edge_behaviors[i] = value;
    }
}

void Game::switch_screen(s32 screen) {
#if 0
    // TODO(jerry):
    // honestly, these probably don't use too much memory so I'm not
    // super concerned about trying to free the memory used by the intro/ending,
    // but might be worth shaving about 4 MB ish. 
    if (state->screen_mode == GAME_SCREEN_ENDING) {
        state->ending_data.unload_all_assets(resources);
    }
#endif

    state->last_screen_mode = state->screen_mode;
    state->screen_mode      = screen;

    // special case setup code will be here

    switch (screen) {
        case GAME_SCREEN_TITLE_SCREEN:
        case GAME_SCREEN_OPENING: {
        } break;
        case GAME_SCREEN_MAIN_MENU: {
            if (state->last_screen_mode == GAME_SCREEN_INGAME) {
                save_game();
                cleanup_game_simulation();
            }
        } break;
        case GAME_SCREEN_INGAME: {
        } break;
    }
}
void Game::switch_ui(s32 ui) {
    state->last_ui_state = state->ui_state;
    state->ui_state = ui;

    switch (ui) {
        case UI_STATE_ACHIEVEMENTS: {
            state->achievement_menu.page = 0;
        } break;
        case UI_STATE_PET_SELECT: {
            state->mainmenu_data.stage_pet_selection = 0;
        } break;
        case UI_STATE_REPLAY_COLLECTION: {
            state->gameplay_data.demo_collection_ui.current_page = 0;
        } break;
        case UI_STATE_DEAD_MAYBE_RETRY: {
            auto& death_screen_data = state->deathscreen_data;
            death_screen_data.reset();
        } break;
        case UI_STATE_CONTROLS: {
            control_menu_temp_data.trying_to_bind_controls = 0;
            Action::copy_action_map(Action::get_action_map(), control_menu_temp_data.temp_action_map);
        } break;
        default: {
            // unused
        } break;
    }
}

void Game::notify_new_achievement(s32 id) {
    Achievement_Notification notification;
    {
        notification.phase = ACHIEVEMENT_NOTIFICATION_PHASE_APPEAR;
        notification.id    = id;
        notification.timer = 0.0f;
    }
    _debugprintf("okay, notify achievement (%d)", id);
    achievement_state.notifications.push(notification);
}

bool Game::save_game() {
    _debugprintf("Attempting to save game.");
    string full_save_path = prefqpath(save_file_name);
    if (OS_file_exists(full_save_path)) {
        _debugprintf("NOTE: overriding old save file.");
    }

    auto serializer = open_write_file_serializer(full_save_path);
    serializer.expected_endianess = ENDIANESS_LITTLE;
    serialize_game_state(&serializer);
    serializer_finish(&serializer);

    return true;
}

bool Game::load_game() {
    string full_save_path = prefqpath(save_file_name);

    if (OS_file_exists(full_save_path)) {
        _debugprintf("Attempting to load save game.");
        auto serializer = open_read_file_serializer(full_save_path);
        serializer.expected_endianess = ENDIANESS_LITTLE;
        // NOTE: serialize game_state should be allowed to fail.
        auto updated_save_data = serialize_game_state(&serializer);
        serializer_finish(&serializer);

        update_from_save_data(&updated_save_data);
        _debugprintf("Hopefully loaded.");
        return true;
    } else {
        _debugprintf("Save file does not exist. Nothing to load.");
        return false;
    }
}

Save_File Game::construct_save_data() {
    Save_File save_data;

    {
        save_data.version = SAVE_FILE_VERSION_CURRENT;
        {
            for (int stage_index = 0; stage_index < 4; ++stage_index) {
                for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
                    auto& level = stage_list[stage_index].levels[level_index];
                    save_data.stage_last_scores[stage_index][level_index] = level.last_score;
                    save_data.stage_best_scores[stage_index][level_index] = level.best_score;
                    save_data.stage_completions[stage_index][level_index] = level.attempts;
                    save_data.stage_attempts[stage_index][level_index]    = level.completions;
                }
            }
        }
        {
            for (int stage_index = 0; stage_index < 4; ++stage_index) {
                auto& stage = stage_list[stage_index];
                save_data.stage_unlocks[stage_index] = stage.unlocked_levels;
            }
        }
        {
            save_data.post_game = can_access_stage(3);
        }
#ifdef BUILD_DEMO
        {
          save_data.beat_demo = can_access_stage(1);
        }
#endif
        {
            save_data.playtime = total_playtime;
        }
        {
            save_data.first_load = 1;
        }
        {
            save_data.pets_unlocked = state->gameplay_data.unlocked_pets;
        }
    }

    return save_data;
}

void Game::update_from_save_data(Save_File* save_data) {
    for (int stage_index = 0; stage_index < 4; ++stage_index) {
        auto& stage = stage_list[stage_index];
        stage.unlocked_levels = save_data->stage_unlocks[stage_index];

        for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
            auto& level = stage.levels[level_index];
            level.last_score  = save_data->stage_last_scores[stage_index][level_index];
            level.best_score  = save_data->stage_best_scores[stage_index][level_index];
            level.attempts    = save_data->stage_attempts[stage_index][level_index];
            level.completions = save_data->stage_completions[stage_index][level_index];
        }
    }

    total_playtime = save_data->playtime;

    /*
      Affect some state based on save data.

      These are various things to prevent cutscenes from triggering when they
      shouldn't.
    */

    // Prevent congratulations cutscene from showing up
    {
        auto state = &this->state->mainmenu_data;
        bool have_postgame_access = can_access_stage(3);
        bool likely_completed_demo = can_access_stage(1);
        {
            state->cutscene1.triggered = have_postgame_access;
        }
        {
            state->cutscene4.triggered = likely_completed_demo;
        }
#if 0
        // Enable postgame portal if we're in the postgame
        {
            auto& portal = state->portals[3]; 
            portal.visible = can_access_stage(3);
        }    
#endif
    }
    {
        auto state = &this->state->gameplay_data;
        state->unlocked_pets = save_data->pets_unlocked;
    }
}

local void serialize_achievement(struct binary_serializer* serializer, Achievement* achievement) {
    // NOTE: assume achievements are serialized in order.

    switch (achievement->progress_type) {
        case ACHIEVEMENT_PROGRESS_TYPE_INT: {
            serialize_s32(serializer, &achievement->progress.as_int.min);
            serialize_s32(serializer, &achievement->progress.as_int.max);
            serialize_s32(serializer, &achievement->progress.as_int.current);
        } break;
        case ACHIEVEMENT_PROGRESS_TYPE_FLOAT: {
            serialize_f32(serializer, &achievement->progress.as_float.min);
            serialize_f32(serializer, &achievement->progress.as_float.max);
            serialize_f32(serializer, &achievement->progress.as_float.current);
        } break;
    }

    serialize_s8(serializer, &achievement->achieved);
    serialize_s8(serializer, &achievement->notified_of_unlock);
}

Save_File Game::serialize_game_state(struct binary_serializer* serializer) {
    Save_File save_data = construct_save_data();

    // Serialize fields
    {
        serialize_s32(serializer, &save_data.version);
        _debugprintf("savefile version id: %s", save_file_version_strings[save_data.version].data);
        // only one version for now anyways.
        switch (save_data.version) {
            case SAVE_FILE_VERSION_PRERELEASE0:
            case SAVE_FILE_VERSION_PRERELEASE1:
            case SAVE_FILE_VERSION_PRERELEASE2:
            case SAVE_FILE_VERSION_PRERELEASE3:
            {
                _debugprintf("Rejecting prerelease save file");
                return save_data;
            } break;

            case SAVE_FILE_VERSION_PRERELEASE5: {
                for (int stage_index = 0; stage_index < 4; ++stage_index) {
                    for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
                        serialize_s32(serializer, &save_data.stage_last_scores[stage_index][level_index]);
                        serialize_s32(serializer, &save_data.stage_best_scores[stage_index][level_index]);
                        serialize_s32(serializer, &save_data.stage_attempts[stage_index][level_index]);
                        serialize_s32(serializer, &save_data.stage_completions[stage_index][level_index]);
                    }
                }
                for (int stage_index = 0; stage_index < 4; ++stage_index) {
                    serialize_s32(serializer, &save_data.stage_unlocks[stage_index]);
                }
                
                serialize_u8(serializer, &save_data.post_game);
                serialize_f32(serializer, &save_data.playtime);
                serialize_s32(serializer, &save_data.first_load);
                serialize_s32(serializer, &save_data.pets_unlocked);

                // serialize all achievements
                // NOTE: slices are writable in my "library of code" so I'm
                // just going to take advantage of this.
                {
                    auto achievements = Achievements::get_all();

                    for (int achievement_index = 0; achievement_index < achievements.length; ++achievement_index) {
                        auto achievement = &achievements[achievement_index];
                        serialize_achievement(serializer, achievement);
                    }
                }
            } break;

            case SAVE_FILE_VERSION_CURRENT:
            default: {
              for (int stage_index = 0; stage_index < 4; ++stage_index) {
                for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
                  serialize_s32(serializer, &save_data.stage_last_scores[stage_index][level_index]);
                  serialize_s32(serializer, &save_data.stage_best_scores[stage_index][level_index]);
                  serialize_s32(serializer, &save_data.stage_attempts[stage_index][level_index]);
                  serialize_s32(serializer, &save_data.stage_completions[stage_index][level_index]);
                }
              }
              for (int stage_index = 0; stage_index < 4; ++stage_index) {
                serialize_s32(serializer, &save_data.stage_unlocks[stage_index]);
              }

              serialize_u8(serializer, &save_data.post_game);
              serialize_u8(serializer, &save_data.beat_demo);
              serialize_f32(serializer, &save_data.playtime);
              serialize_s32(serializer, &save_data.first_load);
              serialize_s32(serializer, &save_data.pets_unlocked);

              // serialize all achievements
              // NOTE: slices are writable in my "library of code" so I'm
              // just going to take advantage of this.
              {
                auto achievements = Achievements::get_all();

                for (int achievement_index = 0; achievement_index < achievements.length; ++achievement_index) {
                  auto achievement = &achievements[achievement_index];
                  serialize_achievement(serializer, achievement);
                }
              }
            } break;
        }
    }
    
    // unimplemented("save mode");
    return save_data;
}


/*
  NOTE: this is useful for the game level design functionalities,
  and I'm aware of a generational counter, but I don't legitimately
  need one.

  This allows me to reference entities in a simple way, and I know
  that there will always be less entities than bullets.

  These are persistent references.

  I... don't expect these ids to wrap over in the course of a gameplay
  session...
*/
u64 _bullet_entity_uid = 0;
u64 _enemy_entity_uid = 0;
void UID::reset() {
    _bullet_entity_uid = _enemy_entity_uid = 0;
}

u64 UID::bullet_uid() {
    return ++_bullet_entity_uid;
}

u64 UID::enemy_uid() {
    return ++_enemy_entity_uid;
}


#include "credits_mode.cpp"
#include "title_screen_mode.cpp"
#include "main_menu_mode.cpp"
#include "opening_mode.cpp"
#include "ending_mode.cpp"
#include "trailer_cutaway_mode.cpp"

// Boss_Healthbar_Displays
// This is a big parameter list, but that's alright
// this is not meant to be reusable.
local void render_boss_health_bar(
    struct render_commands* ui_render_commands,
    V2 position,
    f32 percentage,
    string text, // HP
    string text2, // BOSSNAME
    f32 alpha,
    f32 r,
    f32 text_scale,
    Game_Resources* resources
) {
    {
        // The only time I'm rendering any lines.
        auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
        auto full_bar = rectangle_f32(position.x, position.y, PLAY_AREA_WIDTH_PX - 20, r);
        auto health_bar = full_bar;
        health_bar.w *= percentage;

        f32 boss_name_width = font_cache_text_width(font, text2, text_scale);
        f32 text_height = font_cache_text_height(font) * text_scale;

        // Inner Bar
        {
            render_commands_push_quad_ext(
                ui_render_commands,
                full_bar,
                color32u8(128,25,45,255*alpha),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );
            render_commands_push_quad_ext(
                ui_render_commands,
                rectangle_f32(full_bar.x, full_bar.y, full_bar.w, full_bar.h/2),
                color32u8(128/2,25/2,45/2,255*alpha),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );
        }
        
        // Main Healthbar
        // NOTE(jerry): hardcoded for R = 4 since I don't want to draw a sprite
        // so this will be done in code.
        {
            // Main Green
            render_commands_push_quad_ext(
                ui_render_commands,
                health_bar,
                color32u8(15,200,45,255*alpha),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );

            // highlight
            render_commands_push_quad_ext(
                ui_render_commands,
                rectangle_f32(health_bar.x, health_bar.y+0, health_bar.w, 1),
                color32u8(255,255,255,alpha*255),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );
            render_commands_push_quad_ext(
                ui_render_commands,
                rectangle_f32(health_bar.x, health_bar.y+1, health_bar.w, 1),
                color32u8(200,255,200,alpha*255),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );
            // Darker Green
            render_commands_push_quad_ext(
                ui_render_commands,
                rectangle_f32(health_bar.x, health_bar.y+health_bar.h-1, health_bar.w, 2),
                color32u8(15*0.5,200*0.5,45*0.5,255*alpha),
                V2(0.5, 0.5),
                0,
                BLEND_MODE_ALPHA
            );
        }

        render_commands_push_text(
          ui_render_commands,
          font,
          text_scale,
          V2(full_bar.x + full_bar.w / 2 - boss_name_width / 2-1, position.y + full_bar.h / 2 - text_height / 2-1),
          text2,
          color32f32(0,0,0, alpha),
          BLEND_MODE_ALPHA);
        render_commands_push_text(
            ui_render_commands,
            font,
            text_scale,
            V2(full_bar.x + full_bar.w/2 - boss_name_width/2, position.y + full_bar.h/2 - text_height/2),
            text2,
            color32f32(1,1,1,alpha),
            BLEND_MODE_ALPHA);
    }
}

// NOTE: these times are selected to reduce the possibility
// of being stuck in weird "inbetween" positions in normal play.
// Also to mostly be non-intrusive.
#define BOSS_HEALTHBAR_DISPLAY_SPAWN_TIME    (2.000f)
#define BOSS_HEALTHBAR_DISPLAY_READJUST_TIME (0.100f)
#define BOSS_HEALTHBAR_DISPLAY_DESPAWN_TIME  (1.000f)
#define BOSS_HEALTHBAR_DISPLAY_OFFSET_X      (10)
#define BOSS_HEALTHBAR_DISPLAY_RADIUS        (4)
#define BOSS_HEALTHBAR_DISPLAY_Y_ADV         (BOSS_HEALTHBAR_DISPLAY_RADIUS * 2.5)

V2 Boss_Healthbar_Displays::element_position_for(s32 idx) {
    return V2(0, idx * BOSS_HEALTHBAR_DISPLAY_Y_ADV);
}

void Boss_Healthbar_Displays::add(u64 entity_uid, string name) {
    for (s32 healthbar_index = 0; healthbar_index < displays.size; ++healthbar_index) {
        auto& display = displays[healthbar_index];

        if (display.entity_uid == entity_uid)
            return;
    }

    auto display = Boss_Healthbar_Display{
        .entity_uid = entity_uid,
    };
    _debugprintf("(%d)%.*s\n", name.length, name.length, name.data);
    // NOTE(jerry):
    // buggy if new enemies respawn after a while...
    for (int i = 0; i < name.length; ++i) display.bossnamebuffer[i] = name.data[i];
    V2 element_position           = element_position_for(displays.size);
    display.start_position_target = element_position;
    display.end_position_target   = element_position + V2(BOSS_HEALTHBAR_DISPLAY_OFFSET_X, 0);
    display.animation_t = 0.0f;
    displays.push(display);
}

void Boss_Healthbar_Displays::remove(u64 entity_uid) {
    // queue for removal.
    for (s32 healthbar_index = 0; healthbar_index < displays.size; ++healthbar_index) {
        auto& display = displays[healthbar_index];

        if (display.entity_uid == entity_uid) {
            // This position doesn't really matter too much... Just hope it looks fine
            // to me.
            display.animation_state = BOSS_HEALTHBAR_ANIMATION_DISPLAY_DESPAWN;
            display.animation_t     = 0.0f;
            display.start_position_target = display.position;
            display.end_position_target   = display.position + V2(PLAY_AREA_WIDTH_PX, 0);
            break;
        }
    }
}

void Boss_Healthbar_Displays::update(Game_State* state, f32 dt) {
    bool any_removed           = false;
    s32 earliest_removed_index = displays.size+1;

    for (s32 healthbar_index = 0; healthbar_index < displays.size; ++healthbar_index) {
        auto& display = displays[healthbar_index];

        if (state->gameplay_data.lookup_enemy(display.entity_uid) == nullptr &&
            display.animation_state != BOSS_HEALTHBAR_ANIMATION_DISPLAY_DESPAWN)
        {
            remove(display.entity_uid);
        }

        // need the hp bars to fall in order.
        switch (display.animation_state) {
            case BOSS_HEALTHBAR_ANIMATION_DISPLAY_SPAWN: {
                const f32 max_t = BOSS_HEALTHBAR_DISPLAY_SPAWN_TIME;
                f32 effective_t = (display.animation_t / max_t);
                display.alpha = clamp<f32>(effective_t, 0.0f, 1.0f);
                display.position.x = lerp_f32(display.start_position_target.x, display.end_position_target.x, effective_t);
                display.position.y = lerp_f32(display.start_position_target.y, display.end_position_target.y, effective_t);

                if (!display.emitter){
                    display.emitter = state->gameplay_data.ui_particle_emitters.alloc();
                } 
                
                if (display.emitter) {
                    auto& emitter = *display.emitter;

                    emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                        PARTICLE_EMITTER_FLAGS_USE_ANGULAR |
                        PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE;
                    emitter.sprite = sprite_instance(state->resources->circle_sprite);
                    emitter.sprite.scale            = V2(0.125/4, 0.125/4);
                    emitter.modulation              = color32f32(0.12, 0.85, 0.12, 1);
                    emitter.target_modulation       = color32f32(59/255.0f, 59/255.0f, 56/255.0f, 127/255.0f);
                    emitter.lifetime                = 1.25f;
                    emitter.scale_variance          = V2(-0.005, 0.005);
                    emitter.velocity_x_variance     = V2(-30, 100);
                    emitter.velocity_y_variance     = V2(100, 205);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-50, 350);
                    emitter.lifetime_variance       = V2(-0.25f, 1.0f);
                    emitter.emission_max_timer      = 0.045f;
                    emitter.max_emissions           = -1;
                    emitter.emit_per_emission       = 18;
                    emitter.scale                   = 1;
                }

                if (display.animation_t >= max_t) {
                    display.animation_state = BOSS_HEALTHBAR_ANIMATION_DISPLAY_IDLE;
                    display.animation_t = 0.0f;
                    if (display.emitter) display.emitter->flags = 0; // disable emitter
                    display.emitter = 0;
                }

                display.animation_t += dt;
            } break;
            case BOSS_HEALTHBAR_ANIMATION_DISPLAY_IDLE: {/* do nothing. */} break;
            case BOSS_HEALTHBAR_ANIMATION_DISPLAY_FALL_INTO_ORDER: {
                const f32 max_t = BOSS_HEALTHBAR_DISPLAY_READJUST_TIME;
                f32 effective_t = (display.animation_t / max_t);
                display.position.x = lerp_f32(display.start_position_target.x, display.end_position_target.x, effective_t);
                display.position.y = lerp_f32(display.start_position_target.y, display.end_position_target.y, effective_t);

                if (display.animation_t >= max_t) {
                    display.animation_state = BOSS_HEALTHBAR_ANIMATION_DISPLAY_IDLE;
                    display.animation_t = 0.0f;
                }
                display.animation_t += dt;
            } break;
            case BOSS_HEALTHBAR_ANIMATION_DISPLAY_DESPAWN: {
                const f32 max_t = BOSS_HEALTHBAR_DISPLAY_DESPAWN_TIME;
                f32 effective_t = (display.animation_t / max_t);
                f32 effective_t2 = (display.animation_t / (max_t/2));
                display.alpha = clamp<f32>(1.0 - effective_t2, 0.0f, 1.0f);
                display.position.x = lerp_f32(display.start_position_target.x, display.end_position_target.x, effective_t);
                display.position.y = lerp_f32(display.start_position_target.y, display.end_position_target.y, effective_t);

                if (display.animation_t >= max_t+0.15f) {
                    displays.pop_and_swap(healthbar_index);
                    if (healthbar_index < earliest_removed_index) {
                        earliest_removed_index = healthbar_index;
                        any_removed = true;
                    }

                    break;
                }

                display.animation_t += dt;
            } break;
        }
    }

    // Start_Shifting_All_Remaining_Bars
    if (any_removed) {
        for (s32 healthbar_index = earliest_removed_index; healthbar_index < displays.size; ++healthbar_index) {
            auto& display = displays[healthbar_index];
            display.animation_state = BOSS_HEALTHBAR_ANIMATION_DISPLAY_FALL_INTO_ORDER;
            display.animation_t     = 0.0f;
            display.start_position_target = display.position;
            display.end_position_target   = display.position - V2(0, BOSS_HEALTHBAR_DISPLAY_Y_ADV);
        }
    }
}

void Boss_Healthbar_Displays::render(struct render_commands* ui_commands, Game_State* state) {
    // I need to see where the UI starts here so I can reposition stuff..
    if (DebugUI::enabled()) {
        render_commands_push_quad(
            ui_commands,
            rectangle_f32(position.x, position.y, 30, 30), color32u8(0, 0, 255, 128), BLEND_MODE_ALPHA);
    }

    for (s32 healthbar_index = 0; healthbar_index < displays.size; ++healthbar_index) { // todo fix.
        auto& display = displays[healthbar_index];
        Enemy_Entity* e = state->gameplay_data.lookup_enemy(display.entity_uid);
        f32 percentage = 0.0f;
        string name = string_from_cstring(display.bossnamebuffer);
        string hp   = string_literal("???");

        auto display_position = position + display.position;
        if (display.animation_state == BOSS_HEALTHBAR_ANIMATION_DISPLAY_SPAWN) {
            percentage = saturate(display.animation_t/BOSS_HEALTHBAR_DISPLAY_SPAWN_TIME);

            if (display.emitter) {
              auto& emitter = *display.emitter;
              auto full_bar = rectangle_f32(display_position.x, display_position.y, PLAY_AREA_WIDTH_PX - 20, BOSS_HEALTHBAR_DISPLAY_RADIUS);
              auto health_bar = full_bar;
              health_bar.w *= percentage;
              emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                PARTICLE_EMITTER_FLAGS_USE_ANGULAR |
                PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE;
              display.emitter->shape =
                particle_emit_shape_quad(
                  V2(health_bar.x + health_bar.w / 2, health_bar.y + health_bar.h / 2),
                  V2(health_bar.w / 2, health_bar.h / 2),
                  true
                );
            }
        } else {
            if (e) {
                percentage = e->hp_percentage();
                hp = memory_arena_push_string(
                    &Global_Engine()->scratch_arena,
                    string_from_cstring(format_temp("%d/%d", e->hp, e->max_hp))
                );
            }
        }

        render_boss_health_bar(
            ui_commands,
            display_position,
            percentage,
            hp,
            name,
            display.alpha,
            BOSS_HEALTHBAR_DISPLAY_RADIUS,
            1,
            state->resources
        );
    }
}

// Visual_Sparkling_Star_Data
void Visual_Sparkling_Star_Data::update(f32 dt) {
    if (!hide) {
        switch (frame_index) {
            case 8:
            case 0: {
                if (anim_timer >= 0.06f) {
                    if (frame_index == 8) {
                        hide = true;
                        frame_index = 0;
                    } else {
                        frame_index = 1;
                    }
                    anim_timer = 0.0f;
                }
            } break;
            case 7:
            case 1: {
                if (anim_timer >= 0.07f) {
                    if (frame_index == 7) {
                        frame_index = 8;
                    } else {
                        frame_index = 2;
                    }
                    anim_timer = 0.0f;
                }
            } break;
            case 6:
            case 2: {
                if (anim_timer >= 0.07f) {
                    if (frame_index == 6) {
                        frame_index = 7;
                    } else {
                        frame_index = 3;
                    }
                    anim_timer = 0.0f;
                }
            } break;
            case 5:
            case 3: {
                if (anim_timer >= 0.07f) {
                    if (frame_index == 5) {
                        frame_index = 6;
                    } else {
                        frame_index = 4;
                    }
                    anim_timer = 0.0f;
                }
            } break;
            case 4: {
                if (anim_timer >= 0.12f) {
                    frame_index = 5;
                    anim_timer = 0.0f;
                }
            } break;
        }
        frame_index = clamp<s32>(frame_index, 0, 8);

        anim_timer += dt;
    } else {
        visibility_delay_timer -= dt;
        if (visibility_delay_timer <= 0.0f) {
            visibility_delay_timer = max_visibility_delay_timer;
            hide = false;
        }
    }
}

void Visual_Sparkling_Star_Data::draw(struct render_commands* commands, Game_Resources* resources) {
    if (hide) {
        return;
    }

    // auto& texture_atlas = resources->
    auto sprite       = graphics_get_sprite_by_id(&resources->graphics_assets, resources->projectile_sprites[PROJECTILE_SPRITE_SPARKLING_STAR]);
    auto sprite_frame = sprite_get_frame(sprite, frame_index);
    auto sprite_image = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
    auto source_rect  = sprite_frame->source_rect;

    V2 sprite_dimensions = V2(16,16) * scale;
    render_commands_push_image(commands,
                               sprite_image,
                               rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                               RECTANGLE_F32_NULL,
                               color32f32(1.0, 1.0, 1.0, 1.0),
                               0,
                               BLEND_MODE_ALPHA);

}
// End Visual_Sparkling_Star_Data

// DeathScreen_Data
void DeathScreen_Data::reset(void) {
    black_fade_t = 0.0f;
    text_fade_t  = 0.0f;
    phase        = DEATH_SCREEN_PHASE_SLIDE_IN;
}
// End DeathScreen_Data

#include "demo_recording.cpp"

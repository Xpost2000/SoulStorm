// NOTE: these should be exclusively render commands in the future. I just don't do it quite yet
// but it would help for rendering in OpenGL, and I could add game specific render commands later...

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "fade_transition.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"
#include "achievements.h"

#include "game_ui.h"

// TODO: finish main menu implementation

// This will be hard-coded, since this is how my game design idea is.

// TBH, I think some of the names are cool from other mythologies but I seem
// to have middle-school level writing sometimes :)

// You know, now that I look at this further. This sounds a lot like Dante's Inferno...

// I kinda wanna draw a cute icon for each of the levels.
// TODO: need to lock stages with their prerequisities
local Stage stage_list[] = {
    #include "stage_list.h"
};

// Should not change in the future.
enum Achievement_ID_List {
    ACHIEVEMENT_ID_TEST_ACHIEVEMENT0,

    ACHIEVEMENT_ID_STAGE1,

    ACHIEVEMENT_ID_STAGE2,

    ACHIEVEMENT_ID_STAGE3,

    ACHIEVEMENT_ID_STAGE4,

    // ACHIEVEMENT_ID_STAGE1_FLAWLESS,
    // ACHIEVEMENT_ID_STAGE2_FLAWLESS,
    // ACHIEVEMENT_ID_STAGE3_FLAWLESS,
    // ACHIEVEMENT_ID_STAGE4_FLAWLESS,

    // play as every character type(?)
    // ACHIEVEMENT_ID_JACK_OF_ALL_TRADES,
    // ACHIEVEMENT_ID_MERCIFUL,
    ACHIEVEMENT_ID_KILLER,
    ACHIEVEMENT_ID_MURDERER,
    ACHIEVEMENT_ID_SLAYER,

    ACHIEVEMENT_ID_PLATINUM
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
local void game_complete_stage_level(s32 id, s32 level_id) {
    auto& stage = stage_list[id];

    {
        /*
          The way the game structure is setup is that I assume levels
          must be unlocked in linear order, but there can definitely be
          some clean up as to some of this logic.

          Whether I do this or not is a good question, but it's not too much code to
          fix up.
        */
        if ((level_id+1) > stage.unlocked_levels) {
            /*
             * NOTE:
             *
             *  This will increment on the last level to unlock a "stage 4" which doesn't
             *  exist, but will just be used as a marker for allowing the next stage to be unlocked.
             */
            _debugprintf("Completed stage (%d-%d) and unlocked the next stage!", id+1, level_id+1);
            stage.unlocked_levels += 1;
        } else {
            _debugprintf("Completed stage (%d-%d) another time!", id+1, level_id+1);
        }
    }

    // Check for all stages completed.
    if (stage.unlocked_levels >= 4) {
        assertion(ACHIEVEMENT_ID_STAGE1 + id <= ACHIEVEMENT_ID_STAGE4 && "Invalid achievement for stage id.");
        auto achievement = Achievements::get(ACHIEVEMENT_ID_STAGE1 + id);
        achievement->report();
    }
}

Game::Game() {
    
}

Game::~Game() {
    
}


void Game::init_graphics_resources(Graphics_Driver* driver) {
    if (!initialized) {
        return;
    }

    for (unsigned index = 0; index < array_count(menu_font_variation_string_names); ++index) {
        string current = menu_font_variation_string_names[index];
        resources->menu_fonts[index] = graphics_assets_load_bitmap_font(&resources->graphics_assets, current, 5, 12, 5, 20);
        driver->upload_font(&resources->graphics_assets, resources->menu_fonts[index]);
    }

    resources->circle = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/circle256.png"));
    driver->upload_texture(&resources->graphics_assets, resources->circle);
}

void Game::init_audio_resources() {
    if (!initialized) {
        return;
    }
}

void Game::setup_stage_start() {
    state->tries = MAX_BASE_TRIES;

    auto state             = &this->state->gameplay_data;
    state->stage_state = stage_null();

    state->player.position = V2(state->play_area.width / 2, 300);
    state->player.hp       = 1;
    state->player.die      = false;
    state->player.scale    = V2(15, 15);

    state->bullets.clear();
    state->explosion_hazards.clear();
    state->laser_hazards.clear();

    // setup introduction badge
    {
        state->intro.stage       = GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_FADE_IN;
        state->intro.stage_timer = Timer(0.25f);
    }

    // setup end sequence information
    {
        state->triggered_stage_completion_cutscene = false;
        state->complete_stage.stage                = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;
    }
}

void Game::init(Graphics_Driver* driver) {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state = (Game_State*)arena->push_unaligned(sizeof(*this->state)); (new (this->state) Game_State);

    if (load_preferences_from_disk(&preferences, string_literal("preferences.txt"))) {
        confirm_preferences(&preferences);
    } else {
        // the main code will provide us with a default
        // preferences struct.
    }
    update_preferences(&temp_preferences, &preferences);

    resources->graphics_assets   = graphics_assets_create(arena, 16, 256);

    // gameplay_data initialize
    {
        auto state = &this->state->gameplay_data;
        state->bullets           = Fixed_Array<Bullet>(arena, 10000);
        state->explosion_hazards = Fixed_Array<Explosion_Hazard>(arena, 256);
        state->laser_hazards     = Fixed_Array<Laser_Hazard>(arena, 128);
        state->prng              = random_state();
        state->main_camera       = camera(V2(0, 0), 1.0);
        state->main_camera.rng   = &state->prng;
    }

    // mainmenu_data initialize
    {
        auto state = &this->state->mainmenu_data;
        auto resolution = driver->resolution();

        
        state->player.position      = V2(resolution.x / 2, resolution.y / 2);
        state->player.scale         = V2(15, 15);
        state->player.velocity      = V2(0, 0);
        // I need to utilize a camera effect which relies on centering
        // for polish reasons.
        state->main_camera          = camera(V2(resolution.x/2, resolution.y/2), 1.0);
        state->main_camera.centered = true;

        state->portals = Fixed_Array<MainMenu_Stage_Portal>(arena, 4);
        {
            // initialize all portals here for the main menu
            // portals should be spaced differently based on what's unlocked
            // but I'll just place them manually
            {
                auto& portal = state->portals[0]; 
                portal.stage_id = 0;
                portal.scale = V2(15, 15);
                portal.position = V2(100-30, 100);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
            }

            {
                auto& portal = state->portals[1]; 
                portal.stage_id = 1;
                portal.scale = V2(15, 15);
                portal.position = V2(350-30, 100);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
            }

            {
                auto& portal = state->portals[2]; 
                portal.stage_id = 2;
                portal.scale = V2(15, 15);
                portal.position = V2(550-30, 100);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.prerequisites[1] = 1;
            }

            {
                auto& portal = state->portals[3]; 
                portal.stage_id = 3;
                portal.scale = V2(15, 15);
                portal.position = V2(350-30, 400);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.prerequisites[1] = 1;
                portal.prerequisites[2] = 2;
            }

            state->portals.size = 4;
        }
    }

    initialized = true;
    init_graphics_resources(driver);
    init_audio_resources();

    Achievements::init_achievements(arena, make_slice<Achievement>(achievement_list, array_count(achievement_list)));
    GameUI::initialize(arena);
}

void Game::deinit() {
    
}

/*
 * some testing things
 */

void spawn_bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(direction.x * magnitude, direction.y * magnitude) + V2(direction.x * magnitude/2, direction.y * magnitude/2);
        };

    state->gameplay_data.bullets.push(bullet);
}

void spawn_bullet_circling_down_homing(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->gameplay_data.player.velocity;
        };

    state->gameplay_data.bullets.push(bullet);
}

void spawn_bullet_circling_down_homing2(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);

    Timer until_release(2.0f);
    bool triggered = false;
    f32 t_hit = 0;

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) mutable {
            until_release.start();
            if (until_release.triggered()) {
                triggered = true;
                t_hit = self->t_since_spawn;
            }
            until_release.update(dt);

            if (triggered)
                self->velocity = additional + (additional.normalized() * 50 * self->t_since_spawn - t_hit);
            else
                self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->gameplay_data.player.velocity;
        };

    state->gameplay_data.bullets.push(bullet);
}

void spawn_bullet_circling_down(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            f32 t_extra = self->t_since_spawn;
            f32 t = (factor + t_extra);
            self->velocity = V2(-sinf(t) * factor2,
                                cos(t) * factor2) + additional;
        };

    state->gameplay_data.bullets.push(bullet);
}

bool Game::can_access_stage(s32 id) {
    auto state = &this->state->mainmenu_data;
    auto& stage_portal = state->portals[id];

    for (int i = 0; i < array_count(stage_portal.prerequisites); ++i) {
        if (stage_portal.prerequisites[i] == -1) {
            _debugprintf("Null prerequisite.");
        } else {
            s32 prerequisite_index = stage_portal.prerequisites[i];
            auto& stage = stage_list[prerequisite_index];

            // Refer to 'game_complete_stage_level'
            if (stage.unlocked_levels <= 3) {
                _debugprintf("Failed to reach stage (%d)", id);
                return false;
            }
        }
    }

    return true;
}

void Game::update_and_render_options_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::begin_frame(commands);
    {

        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::label(V2(100, y), string_literal("OPTIONS"), color32f32(1, 1, 1, 1), 4);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            auto display_modes = Global_Engine()->driver->get_display_modes();

            Fixed_Array<string> options_list = Fixed_Array<string>(&Global_Engine()->scratch_arena, display_modes.length);
            for (s32 index = 0; index < display_modes.length; ++index) {
                auto&  dm = display_modes[index];
                string s = string_clone(&Global_Engine()->scratch_arena, string_from_cstring(format_temp("%d x %d", dm.width, dm.height)));
                options_list.push(s);
            }

            GameUI::option_selector(V2(100, y), string_literal("Resolution: "), color32f32(1, 1, 1, 1), 2, options_list.data, options_list.size, &temp_preferences.resolution_option_index);
            y += 30;
        }
        if (GameUI::checkbox(V2(100, y), string_literal("Fullscreen "), color32f32(1, 1, 1, 1), 2, &temp_preferences.fullscreen)) {}
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
            update_preferences(&preferences, &temp_preferences);
            confirm_preferences(&preferences);

            // NOTE: readjust the camera.
            {
                state->mainmenu_data.main_camera = camera(V2(commands->screen_width/2, commands->screen_height/2), 1.0);
                state->mainmenu_data.main_camera.centered = true;
            }
        }
        y += 30;
        if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(state->last_ui_state);
            update_preferences(&preferences, &temp_preferences);
            confirm_preferences(&preferences);
            save_preferences_to_disk(&preferences, string_literal("preferences.txt"));

            // NOTE: readjust the camera.
            {
                state->mainmenu_data.main_camera = camera(V2(commands->screen_width/2, commands->screen_height/2), 1.0);
                state->mainmenu_data.main_camera.centered = true;
            }
        }
        y += 30;
        if (GameUI::button(V2(100, y), string_literal("Back"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
            temp_preferences = preferences;
            switch_ui(state->last_ui_state);
        }
        y += 30;
    }
    GameUI::end_frame();
}

void Game::update_and_render_pause_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::begin_frame(commands);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        if (GameUI::button(V2(100, y), string_literal("Resume"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_INACTIVE);
        }
        y += 30;

        if (state->screen_mode != GAME_SCREEN_MAIN_MENU) {
            if (GameUI::button(V2(100, y), string_literal("Return To Menu"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                _debugprintf("return to main menu.");

                Transitions::do_shuteye_in(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );
                

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        state->ui_state    = UI_STATE_INACTIVE;
                        switch_screen(GAME_SCREEN_MAIN_MENU);
                        _debugprintf("Hi menu.");

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            }
            y += 30;
        }

        if (GameUI::button(V2(100, y), string_literal("Options"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            _debugprintf("Open the options menu I guess");
            // I'd personally like to animate these, but it requires some more dirty code if
            // I'm doing it from scratch like this.
            switch_ui(UI_STATE_OPTIONS);
        }
        y += 30;

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

        if (GameUI::button(V2(100, y), string_literal("Exit To Windows"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            Transitions::do_color_transition_in(
                color32f32(0, 0, 0, 1),
                0.15f,
                0.3f
            );
                

            Transitions::register_on_finish(
                [&](void*) mutable {
                    Global_Engine()->die();
                }
            );
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::update_and_render_stage_select_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::begin_frame(commands);
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

            for (int i = 0; i < MAX_LEVELS_PER_STAGE; ++i) {
                auto& level = stage.levels[i];
                string name = {};

                bool is_unlocked = met_all_prerequisites && i < stage.unlocked_levels;
                if (is_unlocked) {
                    name = level.name;
                } else {
                    name = string_literal("???");
                }

                auto s = format_temp("%d - %d: %.*s", stage_id, (i+1), name.length, name.data);
                s32 button_status = (GameUI::button(V2(100, y), string_from_cstring(s), color32f32(1, 1, 1, 1), 2, is_unlocked && !Transitions::fading()));
                y += 30;

                if (button_status == WIDGET_ACTION_ACTIVATE) {
                    enter_level = i;
                } else if (button_status == WIDGET_ACTION_HOT) {
                    display_level_icon = i;
                }
            }

            if (display_level_icon != -1) {
                // I don't have icon images yet, so the best I have is just printing text.
                auto& level = stage.levels[display_level_icon];
                color32f32 color = color32f32(1, 1, 1, 1);
                if (level.boss_stage) color = color32f32(1, 0, 0, 1);
                GameUI::label(V2(commands->screen_width - 250, commands->screen_height/2), level.subtitle, color, 2);
            }

            if (enter_level != -1) {
                state->mainmenu_data.stage_id_level_in_stage_select = enter_level;
                Transitions::do_shuteye_in(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );

                /*
                 * this looks a little funny. Too promise-like.
                 */
                Transitions::register_on_finish(
                    [&](void*) mutable {
                        _debugprintf("I would load stage-level : (%d - %d)'s script and get ready to play!", stage_id, enter_level);
                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_INGAME);
                        _debugprintf("off to the game you go!");
                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );

                        Transitions::register_on_finish(
                            [&](void*) mutable {
                                _debugprintf("Set up stage introduction cutscene");
                            }
                        );
                    }
                );
            }
        }

        if (GameUI::button(V2(100, y), string_literal("Cancel"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_INACTIVE);

            // undo the camera zoom

            if (camera_not_already_interpolating_for(&state->mainmenu_data.main_camera, V2(commands->screen_width/2, commands->screen_height/2), 1.0)) {
                camera_set_point_to_interpolate(
                    &state->mainmenu_data.main_camera,
                    V2(commands->screen_width/2, commands->screen_height/2),
                    1.0
                );
            }
        }
        y += 30;
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::update_and_render_game_death_maybe_retry_menu(struct render_commands* commands, f32 dt) {
    if (state->gameplay_data.paused_from_death) {
        render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    }
    GameUI::begin_frame(commands);
    {
        
        f32 alpha = 1.0f;
        if (Transitions::fading()) {
            alpha = 1 - Transitions::fade_t();
        }

        // would like to work on polishing/fading in text but again.
        // in time.
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("DEAD"), color32f32(1, 1, 1, alpha), 4);
        y += 45;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            string text = {};
            bool out_of_tries = state->tries <= 0;

            if (out_of_tries) {
                text = string_literal("No more lives");
            } else {
                text = string_from_cstring(format_temp("Retry (%d tries)", state->tries));
            }

            if (GameUI::button(V2(50, y), text, color32f32(1, 1, 1, alpha), 2, !out_of_tries && !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Transitions::do_color_transition_out(
                    color32f32(0, 0, 0, 0.5),
                    0.15f,
                    0.20f
                );

                state->gameplay_data.paused_from_death  = false;
                state->tries                           -= 1;

                state->gameplay_data.player.invincibility_time.start();
                state->gameplay_data.player.heal(1);
                // resurrect the player with iframes.

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_INACTIVE);
                    }
                );
            }
        }
        y += 30;
        if (GameUI::button(V2(50, y), string_literal("Give up"), color32f32(1, 1, 1, alpha), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            Transitions::do_shuteye_in(
                color32f32(0, 0, 0, 1.0),
                0.15f,
                0.30f
            );

            state->gameplay_data.paused_from_death = false;

            // Return back to the main menu.
            Transitions::register_on_finish(
                [&](void*) mutable {
                    Transitions::do_shuteye_out(
                        color32f32(0, 0, 0, 1.0),
                        0.15f,
                        0.30f
                    );
                    switch_ui(UI_STATE_INACTIVE);
                    switch_screen(GAME_SCREEN_MAIN_MENU);
                }
            );
        }
        y += 30;
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::handle_ui_update_and_render(struct render_commands* commands, f32 dt) {
    switch (state->ui_state) {
        case UI_STATE_INACTIVE: {
            return;
        } break;
        case UI_STATE_PAUSED: {
            update_and_render_pause_menu(commands, dt);
        } break;
        case UI_STATE_OPTIONS: {
            update_and_render_options_menu(commands, dt); 
        } break;
        case UI_STATE_STAGE_SELECT: {
            update_and_render_stage_select_menu(commands, dt); 
        } break;
        case UI_STATE_DEAD_MAYBE_RETRY: {
            update_and_render_game_death_maybe_retry_menu(commands, dt);
        } break;
        default: {
            unimplemented("Unknown ui state type");
        } break;
    }
}

void Game::update_and_render_game_opening(Graphics_Driver* driver, f32 dt) {
    unimplemented("No opening yet.");
}

void Game::ingame_update_introduction_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt) {
    auto& intro_state = state->gameplay_data.intro;
    auto& timer       = intro_state.stage_timer;

    timer.start();
    
    // NOTE: this is okay because I retain the state of the main menu intentionally.
    s32   stage_id = state->mainmenu_data.stage_id_level_select;
    auto& stage    = stage_list[stage_id];

    auto timer_percentage = clamp<f32>(intro_state.stage_timer.percentage(), 0.0f, 1.0f);

    auto title_font    = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    /*
     * I understand this code pattern for writing UI tends to duplicate a lot,
     * but personally I like doing this for animated UI since it's simple and pretty easy to write
     * yet also allowing me to do somewhat interesting UI things.
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
                string title = stage.name;
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
                string title = stage.name;
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 35), title, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
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
                string title = stage.name;
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 35), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
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
                string title = stage.name;
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, effective_t), BLEND_MODE_ALPHA);
            }
            {
                string title = stage.subtitle;
                auto   text_width = font_cache_text_width(title_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y + 35), title, color32f32(1, 1, 1, effective_t), BLEND_MODE_ALPHA);
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

void Game::ingame_update_complete_stage_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt) {
    auto& complete_stage_state = state->gameplay_data.complete_stage;
    auto& timer       = complete_stage_state.stage_timer;

    timer.start();
    auto title_font    = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    f32 timer_percentage = timer.percentage();

    switch (complete_stage_state.stage) {
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN: {
            render_commands_push_quad(commands, rectangle_f32(0, commands->screen_height/2-32, commands->screen_width, 64), color32u8(0, 0, 0, 128 * timer_percentage), BLEND_MODE_ALPHA);
            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, commands->screen_height/2-32, commands->screen_width, 64), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                string title = string_literal("LEVEL COMPLETE");
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                string title = string_literal("*insert score here*");
                auto   text_width = font_cache_text_width(subtitle_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }

            if (timer.triggered()) {
                timer = Timer(0.45f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, commands->screen_height/2-32, commands->screen_width, 64), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                string title = string_literal("LEVEL COMPLETE");
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                string title = string_literal("*insert score here*");
                auto   text_width = font_cache_text_width(subtitle_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }

            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, commands->screen_height/2-32, commands->screen_width, 64), color32u8(0, 0, 0, 128 * (1 - timer_percentage)), BLEND_MODE_ALPHA);
            {
                string title = string_literal("LEVEL COMPLETE");
                auto   text_width = font_cache_text_width(title_font, title, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1 - timer_percentage), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                string title = string_literal("*insert score here*");
                auto   text_width = font_cache_text_width(subtitle_font, title, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), title, color32f32(1, 1, 1, 1 - timer_percentage), BLEND_MODE_ALPHA);
            }

            if (timer.triggered())  {
                s32 stage_id = state->mainmenu_data.stage_id_level_select;
                s32 level_id = state->mainmenu_data.stage_id_level_in_stage_select;
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;

                /*
                 * A neat thing to do would just be setup the main menu to preview
                 *
                 * the next portal
                 * (maybe when I have particle effects during the polishing stage,
                 *  zoom in on it and make it more stronger, maybe with an accompanying message.)
                 *
                 *
                 * I mean trying to write any real sequence code in C++ is never really pretty :/
                 */

                game_complete_stage_level(stage_id, level_id);

                Transitions::do_color_transition_out(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        state->ui_state    = UI_STATE_INACTIVE;
                        switch_screen(GAME_SCREEN_MAIN_MENU);
                        _debugprintf("Hi main menu. We can do some more stuff like demonstrate if we unlocked a new stage!");

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE: {} break;
    }
}


void Game::update_and_render_game_ingame(Graphics_Driver* driver, f32 dt) {
    auto state = &this->state->gameplay_data;
    V2 resolution = driver->resolution();
    {
        state->play_area.x      = resolution.x / 2 - state->play_area.width / 2;
        state->play_area.height = resolution.y;

        // state->play_area.set_all_edge_behaviors_to(PLAY_AREA_EDGE_WRAPPING);
        state->play_area.set_all_edge_behaviors_to(PLAY_AREA_EDGE_BLOCKING);
        state->play_area.edge_behavior_top    = PLAY_AREA_EDGE_WRAPPING;
        state->play_area.edge_behavior_bottom = PLAY_AREA_EDGE_WRAPPING;

        state->main_camera.xy.x = -state->play_area.x;
    }

    if (Input::is_key_pressed(KEY_ESCAPE)) {
        if (this->state->ui_state != UI_STATE_PAUSED) {
            switch_ui(UI_STATE_PAUSED);
        } else {
            switch_ui(UI_STATE_INACTIVE);
        }
    }

    if (Input::is_key_pressed(KEY_T)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down(this->state, position, t, r, V2(0, 60));
        }
    }
    if (Input::is_key_pressed(KEY_U)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down_homing(this->state, position, t, r, V2(0, 0));
        }
    }
    if (Input::is_key_pressed(KEY_I)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down_homing2(this->state, position, t, r, V2(0, 100));
        }
    }
    if (Input::is_key_pressed(KEY_P)) {
        int amount = 250;
        int r = 500;
        f32 step = r / (amount/2);
        int ll = 0;
        for (int i = 0; i < amount; ++i) {
            if (ll > 0) {
                ll--;
                continue;
            }
            // spawn_bullet_circling_down(state, position, t, 150, V2(0, 60));
            f32 t = degree_to_radians(i * (360.0f/amount));
            f32 ner = r - (step) * i;
            if ((i % 5) == 0) {
                ll = 15;

            }
            if (ner <= 0) continue;
            V2 position = V2(cosf(t) * ner, sinf(t) * ner) + state->player.position;
            spawn_bullet_circling_down_homing(this->state, position, t, ner, V2(0, 0));
        }
    }

    if (Input::is_key_pressed(KEY_I)) {
        state->player.kill();
    }

    if (Input::is_key_pressed(KEY_X)) {
        Explosion_Hazard h = Explosion_Hazard(state->player.position, 125, 0.5f, 1.0f);
        state->explosion_hazards.push(h);
    }
    if (Input::is_key_pressed(KEY_C)) {
        // Laser_Hazard h = Laser_Hazard(state->player.position.y, 30.0f, LASER_HAZARD_DIRECTION_HORIZONTAL, 1.0f, 15.0f);
        // state->laser_hazards.push(h);

        // NOTE: these could just act as subscribers in the future.
        //       to death events but it's not too big of a deal for now.
        Achievements::get(ACHIEVEMENT_ID_KILLER)->report((s32)500);
        Achievements::get(ACHIEVEMENT_ID_MURDERER)->report((s32)500);
        Achievements::get(ACHIEVEMENT_ID_SLAYER)->report((s32)500);
    }

    if (Input::is_key_pressed(KEY_V)) {
        // Laser_Hazard h = Laser_Hazard(state->player.position.x, 30.0f, LASER_HAZARD_DIRECTION_VERTICAL, 1.0f, 15.0f);
        // state->laser_hazards.push(h);
        // test unlock all stage achievements
        Achievements::get(ACHIEVEMENT_ID_TEST_ACHIEVEMENT0)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE1)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE2)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE3)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE4)->report();
    }


    // for all of our entities and stuff.
    // will have a separate one for the UI.
    auto game_render_commands = render_commands(&Global_Engine()->scratch_arena, 12000, state->main_camera);
    auto ui_render_commands   = render_commands(&Global_Engine()->scratch_arena, 8192, camera(V2(0, 0), 1));
    {
        game_render_commands.screen_width  = ui_render_commands.screen_width = resolution.x;
        game_render_commands.screen_height = ui_render_commands.screen_height = resolution.y;
    }

    // draw play area borders / Game UI
    // I'd like to have the UI fade in / animate all fancy like when I can
    {
        auto border_color = color32u8(128, 128, 128, 255);
 
        int play_area_width = state->play_area.width;
        int play_area_x     = state->play_area.x;

        // left border
        render_commands_push_quad(&ui_render_commands,
                                  rectangle_f32(0,
                                                0,
                                                play_area_x,
                                                resolution.y),
                                  border_color, BLEND_MODE_ALPHA);
        // right border
        render_commands_push_quad(&ui_render_commands,
                                  rectangle_f32(play_area_x + play_area_width,
                                                0,
                                                resolution.x - play_area_width,
                                                resolution.y),
                                  border_color, BLEND_MODE_ALPHA);
    }

    if (!state->paused_from_death && this->state->ui_state == UI_STATE_INACTIVE && !state->triggered_stage_completion_cutscene) {
        // NOTE: add stage update
        //       to start reading the script.
        if (Input::is_key_down(KEY_SPACE)) {
            // okay these are normal real bullets
            if (state->player.attack()) {
                spawn_bullet_upwards_linear(this->state, state->player.position + V2(-10, 0), V2(0, -1), 1250.0f);
                spawn_bullet_upwards_linear(this->state, state->player.position + V2(10, 0), V2(0, -1), 1250.0f);
            }
        }

        for (int i = 0; i < (int)state->bullets.size; ++i) {
            auto& b = state->bullets[i];
            b.update(this->state, dt);
        }

        for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
            auto& h = state->explosion_hazards[i];
            h.update(this->state, dt);
        }

        for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
            auto& h = state->laser_hazards[i];
            h.update(this->state, dt);
        }

        state->player.update(this->state, dt);

        handle_all_explosions(dt);
        handle_all_lasers(dt);
        handle_all_dead_entities(dt);
    } else {
        handle_ui_update_and_render(&ui_render_commands, dt);
    }

    // Handle transitions or stage specific data
    // so like cutscene/coroutine required sort of behaviors...
    {
        if (!Transitions::fading()) {
            if (state->intro.stage != GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE) {
                ingame_update_introduction_sequence(&ui_render_commands, resources, dt);
            } else {
                auto& stage_state = state->stage_state;
                bool can_finish_stage = stage_state.update(&stage_state, dt, state);

                if (!state->triggered_stage_completion_cutscene && can_finish_stage) {
                    state->triggered_stage_completion_cutscene = true;
                    {
                        state->complete_stage.stage       = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN;
                        state->complete_stage.stage_timer = Timer(0.35f);
                    }
                }

                ingame_update_complete_stage_sequence(&ui_render_commands, resources, dt);
            }
        }
    }

    // main game rendering
    { // draw stage specific things if I need to.
        auto& stage_state = state->stage_state;
        // NOTE: post processing will need a separate pass, but mostly with specific render commands.
        stage_state.draw(&stage_state, dt, &game_render_commands, state);
    }
    {
        for (int i = 0; i < (int)state->bullets.size; ++i) {
            auto& b = state->bullets[i];
            b.draw(this->state, &game_render_commands, resources);
        }

        for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
            auto& h = state->explosion_hazards[i];
            h.draw(this->state, &game_render_commands, resources);
        }

        for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
            auto& h = state->laser_hazards[i];
            h.draw(this->state, &game_render_commands, resources);
        }

        state->player.draw(this->state, &game_render_commands, resources);

        Transitions::update_and_render(&ui_render_commands, dt);
    }

    driver->clear_color_buffer(color32u8(255, 255, 255, 255));
    driver->consume_render_commands(&game_render_commands);
    driver->consume_render_commands(&ui_render_commands);

    camera_update(&state->main_camera, dt);
}

void Game::update_and_render(Graphics_Driver* driver, f32 dt) {
    switch (state->screen_mode) {
        case GAME_SCREEN_TITLE_SCREEN: {
            update_and_render_game_title_screen(driver, dt);
        } break;
        case GAME_SCREEN_OPENING: {
            update_and_render_game_opening(driver, dt);
        } break;
        case GAME_SCREEN_MAIN_MENU: {
            update_and_render_game_main_menu(driver, dt);
        } break;
        case GAME_SCREEN_INGAME: {
            update_and_render_game_ingame(driver, dt);
        } break;
        case GAME_SCREEN_CREDITS: {
            update_and_render_game_credits(driver, dt);
        } break;
    }

    // always check for the platinum achievement unlock
    {
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

    // Polling based achievement updating.
    // TODO: this will notify the achievement UI when I add it
    {
        for (s32 i = 0; i < array_count(achievement_list); ++i) {
            auto achievement = Achievements::get(i);
            if (achievement->complete() && achievement->notify_unlock()) {
                _debugprintf("Hi, you've just unlocked the: %.*s achievement",
                             achievement->id_name.length,
                             achievement->id_name.data
                );
            }
        }
    }
}

void Game::handle_all_dead_entities(f32 dt) {
    auto state = &this->state->gameplay_data;
    for (int i = 0; i < state->bullets.size; ++i) {
        auto& b = state->bullets[i];

        if (b.die) {
            state->bullets.pop_and_swap(i);
        }
    }

    if (state->player.die) {
        if (state->paused_from_death == false) {
            state->paused_from_death = true;
            Transitions::do_color_transition_in(
                color32f32(0, 0, 0, 0.5),
                0.10f,
                0.45f
            );

            Transitions::register_on_finish(
                [&](void*) mutable {
                    Transitions::clear_effect();
                    switch_ui(UI_STATE_DEAD_MAYBE_RETRY);
                }
            );
            // would like to slowly grayscale or something
            // but for now we'll fade.
            // Global_Engine()->die();
        }
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
                camera_traumatize(&state->main_camera, 0.25f);
            }

            auto laser_rect  = h.get_rect(&state->play_area);
            auto player_rect = state->player.get_rect();

            if (rectangle_f32_intersect(player_rect, laser_rect)) {
                state->player.kill();
            }
        }

        if (h.die) {
            _debugprintf("bye bye laser");

            state->laser_hazards.pop_and_swap(i);
        }
    }
}

void Game::handle_all_explosions(f32 dt) {
    auto state = &this->state->gameplay_data;
    for (int i = 0; i < state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];

        if (h.exploded) {
            _debugprintf("biggest boom ever.");
            camera_traumatize(&state->main_camera, 0.25f);
            // check explosion against all entities
            // by all entities, I just mean the player right now.
            {
                auto explosion_circle = circle_f32(h.position.x, h.position.y, h.radius);
                auto player_circle    = circle_f32(state->player.position.x, state->player.position.y, state->player.scale.x);

                if (circle_f32_intersect(explosion_circle, player_circle)) {
                    state->player.kill();
                }
            }
            state->explosion_hazards.pop_and_swap(i);
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
    state->last_screen_mode = state->screen_mode;
    state->screen_mode      = screen;

    // special case setup code will be here

    switch (screen) {
        case GAME_SCREEN_INGAME: {
            setup_stage_start();
        } break;
    }
}
void Game::switch_ui(s32 ui) {
    state->last_ui_state = state->ui_state;
    state->ui_state = ui;
}

#include "credits_mode.cpp"
#include "title_screen_mode.cpp"
#include "main_menu_mode.cpp"

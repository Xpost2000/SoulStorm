// NOTE: these should be exclusively render commands in the future. I just don't do it quite yet
// but it would help for rendering in OpenGL, and I could add game specific render commands later...

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "fade_transition.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"

#include "game_ui.h"

// TODO: finish main menu implementation

// This will be hard-coded, since this is how my game design idea is.

// TBH, I think some of the names are cool from other mythologies but I seem
// to have middle-school level writing sometimes :)

// You know, now that I look at this further. This sounds a lot like Dante's Inferno...

// I kinda wanna draw a cute icon for each of the levels.
local Stage stage_list[] = {
    // Stage 1
    // easy and slow themed.
    {
        string_literal("Limbo"),
        string_literal("The Endless Nothing"),
        0,
        {
            {
                string_literal("Gates of Eternity"),
                string_literal("The beginnings of escape."),
                0
            },
            {
                string_literal("Empty Sea"),
                string_literal("The path of wayward souls."),
                0,
            },
            {
                string_literal("Reaper's Gate"),
                string_literal(""),
                1,
            },
        }
    },
    
    // Stage 2
    // this is "normal"
    {
        string_literal("Fiery Gates"),
        string_literal("Lost Paradise"),
        0,
        {
            {
                string_literal("River Styx"),
                string_literal("Washing away with forgetfulness."),
                0
            },
            {
                string_literal("Fiery Sojourn"),
                string_literal(""),
                0,
            },
            {
                string_literal("Cerberus"),
                string_literal("The Loyal Gatekeeper"),
                1,
            },
        }
    },

    // Stage 3
    // this is "hard"
    {
        string_literal("Inferno"),
        string_literal("A place for the sinful."),
        0,
        {
            {
                string_literal("Greed"),
                string_literal("For those who want too much."),
                0
            },
            {
                string_literal("Heresy"),
                string_literal("For those who don't believe."),
                0,
            },
            {
                string_literal("Treachery"),
                string_literal("For those who betray themselves and everyone else."),
                1,
            },
        }
    },

    // Stage 4: Bonus
    // This is going to be a bonus stage that honestly I'm not sure if
    // I can design to be beatable since I'm not a pro LOL.
    // NOTE: all stages are boss stages.
    {
        string_literal("Insanity"),
        string_literal("Battling away for eternity."),
        0,
        {
            {
                string_literal("Pride"),
                string_literal(""),
                1
            },
            {
                string_literal("Hubris"),
                string_literal(""),
                1,
            },
            {
                string_literal("Arrogance"),
                string_literal(""),
                1,
            },
        }
    },
};

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

void Game::init(Graphics_Driver* driver) {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state = (Game_State*)arena->push_unaligned(sizeof(*this->state)); (new (this->state) Game_State);

    resources->graphics_assets   = graphics_assets_create(arena, 16, 256);

    state->paused = false;

    // gameplay_data initialize
    {
        auto state             = &this->state->gameplay_data;
        state->player.position = V2(state->play_area.width / 2, 300);
        state->player.scale    = V2(15, 15);

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

        state->player.position = V2(resolution.x / 2, resolution.y / 2);
        state->player.scale    = V2(15, 15);
        state->player.velocity = V2(0, 0);
        state->main_camera     = camera(V2(0, 0), 1.0);

        state->portals = Fixed_Array<MainMenu_Stage_Portal>(arena, 4);
        {
            // initialize all portals here for the main menu
            // portals should be spaced differently based on what's unlocked
            // but I'll just place them manually
            {
                auto& portal = state->portals[0]; 
                portal.stage_id = 0;
                portal.scale = V2(15, 15);
                portal.position = V2(100, 100);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
            }

            {
                auto& portal = state->portals[1]; 
                portal.stage_id = 1;
                portal.scale = V2(15, 15);
                portal.position = V2(350, 100);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
            }

            {
                auto& portal = state->portals[2]; 
                portal.stage_id = 2;
                portal.scale = V2(15, 15);
                portal.position = V2(550, 100);
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
                portal.position = V2(350, 400);
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

    GameUI::initialize(arena);
}

void Game::deinit() {
    
}

/*
 * some testing things
 */

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

void spawn_bullet_linear(Game_State* state, V2 position, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = additional;
        };
    bullet.lifetime = Timer(5.0f);

    state->gameplay_data.bullets.push(bullet);
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
        if (GameUI::button(V2(100, y), string_literal("Resume"), color32f32(1, 1, 1, 1), 2)) {
            state->paused = false;
        }

        if (state->screen_mode != GAME_SCREEN_MAIN_MENU) {
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Return To Menu"), color32f32(1, 1, 1, 1), 2)) {
                _debugprintf("return to main menu.");
            }
        }

        y += 30;
        if (GameUI::button(V2(100, y), string_literal("Options"), color32f32(1, 1, 1, 1), 2)) {
            _debugprintf("Open the options menu I guess");
        }
        y += 30;

        if (state->screen_mode != GAME_SCREEN_CREDITS) {
            if (GameUI::button(V2(100, y), string_literal("Credits"), color32f32(1, 1, 1, 1), 2)) {
                _debugprintf("Open the credits screen I guess");
            }
            y += 30;
        }

        if (GameUI::button(V2(100, y), string_literal("Exit To Windows"), color32f32(1, 1, 1, 1), 2)) {
            Global_Engine()->die();
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::update_and_render_game_opening(Graphics_Driver* driver, f32 dt) {
    state->screen_mode = GAME_SCREEN_INGAME;
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
        this->state->paused ^= 1;
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
    if (Input::is_key_pressed(KEY_Y)) {
        int amount = 5;
        for (int i = 0; i < amount; ++i) {
            V2 position = V2(i * 40, 0) + state->player.position;
            // spawn_bullet_circling_down(state, position, t, 150, V2(0, 60));
            spawn_bullet_linear(this->state, position, V2(0, 150));
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

    if (Input::is_key_pressed(KEY_X)) {
        Explosion_Hazard h = Explosion_Hazard(state->player.position, 125, 0.5f, 1.0f);
        state->explosion_hazards.push(h);
    }
    if (Input::is_key_pressed(KEY_C)) {
        Laser_Hazard h = Laser_Hazard(state->player.position.y, 30.0f, LASER_HAZARD_DIRECTION_HORIZONTAL, 1.0f, 15.0f);
        state->laser_hazards.push(h);
    }

    if (Input::is_key_pressed(KEY_V)) {
        Laser_Hazard h = Laser_Hazard(state->player.position.x, 30.0f, LASER_HAZARD_DIRECTION_VERTICAL, 1.0f, 15.0f);
        state->laser_hazards.push(h);
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


    // void render_commands_push_text(struct render_commands* commands, struct font_cache* font, f32 scale, V2 xy, string cstring, union color32f32 rgba, u8 blend_mode);
    render_commands_push_quad(&ui_render_commands, rectangle_f32(100, 100, 100, 100), color32u8(0, 255, 0, 255), BLEND_MODE_ALPHA);
    render_commands_push_text(&ui_render_commands, resources->get_font(MENU_FONT_COLOR_BLOODRED), 2, V2(100, 100), string_literal("I am a brave new world"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
    render_commands_push_text(&ui_render_commands, resources->get_font(MENU_FONT_COLOR_WHITE), 2, V2(100, 150), string_literal("hahahahhaah"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);

    if (!this->state->paused) {
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
    } else {
        update_and_render_pause_menu(&ui_render_commands, dt);
    }

    // main game rendering
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

    handle_all_explosions(dt);
    handle_all_lasers(dt);
    handle_all_dead_entities(dt);
    camera_update(&state->main_camera, dt);
}

void Game::update_and_render_game_credits(Graphics_Driver* driver, f32 dt) {
    
}

void Game::update_and_render(Graphics_Driver* driver, f32 dt) {
    switch (state->screen_mode) {
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
        Global_Engine()->die();
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
                _debugprintf("Hi, I died.");
                state->player.die = true;
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
                    _debugprintf("Hi, I died.");
                    state->player.die = true;
                }
            }
            state->explosion_hazards.pop_and_swap(i);
        }
    }
}

// Play_Area
bool Play_Area::is_inside_absolute(rectangle_f32 rect) {
    return (rectangle_f32_intersect(rect, rectangle_f32(x, 0, width, height)));
}

bool Play_Area::is_inside_logical(rectangle_f32 rect) {
    return (rectangle_f32_intersect(rect, rectangle_f32(0, 0, width, height)));
}

void Play_Area::set_all_edge_behaviors_to(u8 value) {
    for (int i = 0; i < 4; ++i) {
        edge_behaviors[i] = value;
    }
}

#include "main_menu_mode.cpp"

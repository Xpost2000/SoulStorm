/*
  As per usual my game.cpp mega-translation unit is just
  where I glue all the game code together, so this isn't exactly
  the most stellar of code.
 */

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "fade_transition.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"
#include "achievements.h"

#include "file_buffer.h"
#include "game_ui.h"

#include "action_mapper.h"

#define DEFAULT_AUTO_SCORE_INTERVAL (0.055f)

// This will be hard-coded, since this is how my game design idea is.

// TBH, I think some of the names are cool from other mythologies but I seem
// to have middle-school level writing sometimes :)

// You know, now that I look at this further. This sounds a lot like Dante's Inferno...

// I kinda wanna draw a cute icon for each of the levels.
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
    GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE,
};

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

local int game_complete_stage_level(s32 id, s32 level_id) {
    auto& stage  = stage_list[id];
    s32   result = 0;
    {
        /*
          The way the game structure is setup is that I assume levels
          must be unlocked in linear order, but there can definitely be
          some clean up as to some of this logic.

          Whether I do this or not is a good question, but it's not too much code to
          fix up.
        */
        if ((level_id+1) >= stage.unlocked_levels) {
            /*
             * NOTE:
             *
             *  This will increment on the last level to unlock a "stage 4" which doesn't
             *  exist, but will just be used as a marker for allowing the next stage to be unlocked.
             */
            _debugprintf("Completed stage (%d-%d) and unlocked the next stage!", id+1, level_id+1);
            stage.unlocked_levels += 1;

            result = GAME_COMPLETE_STAGE_UNLOCK_NEXT_LEVEL;
        } else {
            _debugprintf("Completed stage (%d-%d) another time!", id+1, level_id+1);
            result = GAME_COMPLETE_STAGE_UNLOCK_LEVEL_REPLAY; 
        }
    }

    // Check for all stages completed.
    if (stage.unlocked_levels > MAX_LEVELS_PER_STAGE) {
        assertion(ACHIEVEMENT_ID_STAGE1 + id <= ACHIEVEMENT_ID_STAGE4 && "Invalid achievement for stage id.");
        auto achievement = Achievements::get(ACHIEVEMENT_ID_STAGE1 + id);
        achievement->report();

        result = GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE;
    }

    return result;
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

    auto state             = &this->state->gameplay_data;

    {
        s32 stage_id = this->state->mainmenu_data.stage_id_level_select;
        s32 level_id = this->state->mainmenu_data.stage_id_level_in_stage_select;
        // NOTE: check the ids later.

        if (stage_id == 0 && level_id == 0) {
            state->stage_state = stage_native_stage_1_1();
        } else {
            state->stage_state = stage_null();
        }
    }

    state->player.position = V2(state->play_area.width / 2, 300);
    state->player.hp       = 1;
    state->player.die      = false;
    state->player.scale    = V2(15, 15);

    state->bullets.clear();
    state->explosion_hazards.clear();
    state->laser_hazards.clear();
    state->score_notifications.clear();
    state->hit_score_notifications.clear();

    state->tries = MAX_BASE_TRIES;
    state->current_score = 0;
    state->auto_score_timer = 0;

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

void test_inf_task(jdr_duffcoroutine_t* co) {
    // JDR_Coroutine_Start(co, Start);
    // while (true) {
    //     _debugprintf("I... Hope I got scheduled? Going to wait for a bit okay?");
    //     JDR_Coroutine_YieldNR();
    // }
    // JDR_Coroutine_End;
}

void Game::init(Graphics_Driver* driver) {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state = (Game_State*)arena->push_unaligned(sizeof(*this->state)); (new (this->state) Game_State);

    if (load_preferences_from_disk(&preferences, string_literal("preferences.lua"))) {
        confirm_preferences(&preferences);
    } else {
        // the main code will provide us with a default
        // preferences struct.
    }
    update_preferences(&temp_preferences, &preferences);

    if (!Action::load(string_literal("controls.lua"))) {
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

            Action::register_action_keys(ACTION_SCREENSHOT, KEY_F12, KEY_UNKNOWN, 1.0f);
        }

        Action::save(string_literal("controls.lua"));
    }

    resources->graphics_assets   = graphics_assets_create(arena, 16, 256);
    // initialize achievement notifier
    {
        achievement_state.notifications = Fixed_Array<Achievement_Notification>(arena, ACHIEVEMENT_ID_COUNT);
    }

    // Initialize coroutine scheduler
    {
        state->coroutine_tasks.tasks = Fixed_Array<Game_Task>(arena, 128);

#if 1
        // test eternal task
        {
            state->coroutine_tasks.add_global_task(
                [](jdr_duffcoroutine_t* co) {
                    JDR_Coroutine_Start(co, Start);
                    while (true) {
                        _debugprintf("I... Hope I got scheduled? Going to wait for a bit okay?");
                        TASK_WAIT(5.0f);
                        _debugprintf("Hopefully five seconds");
                        TASK_WAIT(3.0f);
                        _debugprintf("Wait for me more!");
                        TASK_WAIT(3.0f);
                    }
                    JDR_Coroutine_End;
                }
            );
        }
#endif
    }

    // gameplay_data initialize
    {
        auto state                     = &this->state->gameplay_data;
        state->bullets                 = Fixed_Array<Bullet>(arena, MAX_BULLETS);
        state->explosion_hazards       = Fixed_Array<Explosion_Hazard>(arena, MAX_EXPLOSION_HAZARDS);
        state->laser_hazards           = Fixed_Array<Laser_Hazard>(arena, MAX_LASER_HAZARDS);
        state->enemies                 = Fixed_Array<Enemy_Entity>(arena, MAX_ENEMIES);
        state->score_notifications     = Fixed_Array<Gameplay_UI_Score_Notification>(arena, MAX_SCORE_NOTIFICATIONS);
        state->hit_score_notifications = Fixed_Array<Gameplay_UI_Hitmark_Score_Notification>(arena, MAX_SCORE_NOTIFICATIONS);
        state->prng                    = random_state();
        state->main_camera             = camera(V2(0, 0), 1.0);
        state->main_camera.rng         = &state->prng;

        // creation queues
        {
            // It's easier to just use more memory, and then remerge at the end...
            state->to_create_player_bullets = Fixed_Array<Bullet>(arena, MAX_BULLETS);
            state->to_create_enemy_bullets  = Fixed_Array<Bullet>(arena, MAX_BULLETS);
            state->to_create_enemies        = Fixed_Array<Enemy_Entity>(arena, MAX_ENEMIES);
        }
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

        // initializing all the stars' positions
        {
            auto bkg_slow_stars = state->star_positions[0];
            auto bkg_faster_stars = state->star_positions[1];
            auto& prng = this->state->gameplay_data.prng;
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                bkg_slow_stars[i] = V2(random_ranged_float(&prng, -640, 640),
                                       random_ranged_float(&prng, -480, 480));
                bkg_faster_stars[i] = V2(random_ranged_float(&prng, -640, 640),
                                         random_ranged_float(&prng, -480, 480));
            }
        }
    }

    initialized = true;
    init_graphics_resources(driver);
    init_audio_resources();

    Achievements::init_achievements(arena, make_slice<Achievement>(achievement_list, array_count(achievement_list)));
    GameUI::initialize(arena);

    load_game();
}

void Game::deinit() {
    
}

// Gameplay_Data
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

void Gameplay_Data::reify_all_creation_queues() {
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

            state->to_create_player_bullets.clear();
            state->to_create_enemy_bullets.clear();
            return 0;
        },
        this
    );

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*)ctx;

            for (int i = 0; i < (int)state->to_create_enemies.size; ++i) {
                auto& e = state->to_create_enemies[i];
                state->enemies.push(e);
            }

            state->to_create_enemies.clear();
            return 0;
        },
        this
    );

    Thread_Pool::synchronize_tasks();
}

bool Gameplay_Data::any_hazards() const {
    return (laser_hazards.size > 0) || (explosion_hazards.size > 0);
}

bool Gameplay_Data::any_enemies() const {
    return (enemies.size > 0);
}

bool Gameplay_Data::any_bullets() const {
    return (bullets.size > 0);
}

bool Gameplay_Data::any_living_danger() const {
    return any_enemies() || any_hazards();
}

void Gameplay_Data::notify_score(s32 amount, bool interesting) {
    current_score += amount;

    if (!interesting)
        return;

    auto notification = Gameplay_UI_Score_Notification {amount};
    notification.lifetime.start();
    score_notifications.push(notification);
}

void Gameplay_Data::notify_score_with_hitmarker(s32 amount, V2 where) {
    notify_score(amount, true);
    auto notification = Gameplay_UI_Hitmark_Score_Notification {where, amount};
    notification.lifetime.start();
    hit_score_notifications.push(notification);
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

void Game::update_and_render_options_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_options_menu");
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
            save_preferences_to_disk(&preferences, string_literal("preferences.lua"));

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

        if (Action::is_pressed(ACTION_CANCEL)) {
            switch_ui(state->last_ui_state);
        }
    }
    GameUI::end_frame();
}

void Game::update_and_render_confirm_back_to_main_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_confirm_back_to_main_menu");
    GameUI::begin_frame(commands);
    {
        f32 y = 100;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(50, y), string_literal("CONFIRMATION?"), color32f32(1, 1, 1, 1), 4);
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        y += 45;
        GameUI::label(V2(50, y), string_literal("Are you sure? You will lose your current stage progress."), color32f32(1, 1, 1, 1), 2);
        y += 30;

        if (GameUI::button(V2(100, y), string_literal("Confirm"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            Transitions::do_shuteye_in(
                color32f32(0, 0, 0, 1),
                0.15f,
                0.3f
            );
                

            Transitions::register_on_finish(
                [&](void*) mutable {
                    state->ui_state    = UI_STATE_INACTIVE;
                    switch_screen(GAME_SCREEN_MAIN_MENU);

                    Transitions::do_shuteye_out(
                        color32f32(0, 0, 0, 1),
                        0.15f,
                        0.3f
                    );
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

void Game::update_and_render_confirm_exit_to_windows(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_confirm_exit_to_windows");
    GameUI::begin_frame(commands);
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
                [&](void*) mutable {
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

void Game::update_and_render_pause_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_pause_menu");
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
                if (state->screen_mode != GAME_SCREEN_INGAME) {
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
                } else {
                    state->ui_state = UI_STATE_CONFIRM_BACK_TO_MAIN_MENU;
                }
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

        if (GameUI::button(V2(100, y), string_literal("Achievements"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
            switch_ui(UI_STATE_ACHIEVEMENTS);
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

void Game::update_and_render_stage_select_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

    GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
    GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

    GameUI::set_ui_id((char*)"ui_stage_select_menu");
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

            if (display_level_icon != -1) {
                // I don't have icon images yet, so the best I have is just printing text.
                auto& level = stage.levels[display_level_icon];
                color32f32 color = color32f32(1, 1, 1, 1);
                if (level.boss_stage) color = color32f32(1, 0, 0, 1);
                // TODO: figure out an okay layout later.
                GameUI::label(V2(commands->screen_width - 250, commands->screen_height/2 - 150), string_from_cstring(format_temp("Best Score: %d", level.best_score)), color, 2);
                GameUI::label(V2(commands->screen_width - 250, commands->screen_height/2 - 100), string_from_cstring(format_temp("Last Score: %d", level.last_score)), color, 2);
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

        if (Action::is_pressed(ACTION_CANCEL)) {
            switch_ui(state->last_ui_state);
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::update_and_render_game_death_maybe_retry_menu(struct render_commands* commands, f32 dt) {
    if (state->gameplay_data.paused_from_death) {
        render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    }
    GameUI::set_ui_id((char*)"ui_gameover_menu");
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
            bool out_of_tries = state->gameplay_data.tries <= 0;

            if (out_of_tries) {
                text = string_literal("No more lives");
            } else {
                text = string_from_cstring(format_temp("Retry (%d tries)", state->gameplay_data.tries));
            }

            if (GameUI::button(V2(50, y), text, color32f32(1, 1, 1, alpha), 2, !out_of_tries && !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Transitions::do_color_transition_out(
                    color32f32(0, 0, 0, 0.5),
                    0.15f,
                    0.20f
                );

                state->gameplay_data.paused_from_death  = false;
                state->gameplay_data.tries             -= 1;

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
                    save_game();
                }
            );
        }
        y += 30;

        {
            if (Action::is_pressed(ACTION_CANCEL)) {
                switch_ui(state->last_ui_state);
            }
        }
    }
    GameUI::end_frame();
    GameUI::update(dt);
}

void Game::update_and_render_achievement_notifications(struct render_commands* commands, f32 dt) {
    // NOTE: notifications have to be deleted in order
    //       so I have to loop in reverse to delete them in the same loop.
    auto& notifications = achievement_state.notifications;

    f32 y_cursor_from_bottom = 0;


    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    for (s32 index = notifications.size-1; index >= 0; index--) {
        auto& notification = notifications[index];
        auto  achievement  = Achievements::get(notification.id);

        // achievements will be from bottom left.
        // NOTE: adjust visual box size
        rectangle_f32 rectangle = rectangle_f32(
            0, commands->screen_height - (y_cursor_from_bottom + 60), 150, 60);

        // insert description somehow;
        switch (notification.phase) {
            case ACHIEVEMENT_NOTIFICATION_PHASE_APPEAR: {
                const f32 MAX_PHASE_TIME = 0.35f;
                f32 percentage_t = notification.timer / MAX_PHASE_TIME;
                f32 rect_y = lerp_f32(commands->screen_height - (y_cursor_from_bottom), commands->screen_height - (y_cursor_from_bottom + 60), percentage_t);

                rectangle.y = rect_y;
                render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255 * percentage_t), BLEND_MODE_ALPHA);
                {
                    string text = achievement->name;
                    render_commands_push_text(commands, subtitle_font, 4, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, percentage_t), BLEND_MODE_ALPHA);
                }

                if (notification.timer >= MAX_PHASE_TIME) {
                    notification.phase = ACHIEVEMENT_NOTIFICATION_PHASE_LINGER;
                    notification.timer = 0;
                }
                y_cursor_from_bottom += 60;
            } break;
            case ACHIEVEMENT_NOTIFICATION_PHASE_LINGER: {
                const f32 MAX_PHASE_TIME = 0.55f;
                f32 percentage_t = notification.timer / MAX_PHASE_TIME;

                render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);
                {
                    string text = achievement->name;
                    render_commands_push_text(commands, subtitle_font, 4, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
                }

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
                    f32 percentage_t = notification.timer / MAX_PHASE_TIME;

                    render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255 * (1 - percentage_t)), BLEND_MODE_ALPHA);
                    {
                        string text = achievement->name;
                        render_commands_push_text(commands, subtitle_font, 4, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, 1 - percentage_t), BLEND_MODE_ALPHA);
                    }

                    y_cursor_from_bottom += 60;
                }
            } break;
        }

        notification.timer += dt;
    }
}

void Game::update_and_render_achievements_menu(struct render_commands* commands, f32 dt) {
    render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
    auto achievements = Achievements::get_all();
    // I want a very simple layout so it'll just be a basic "list", not a table.
    const s32 MAX_ACHIEVEMENTS_PER_COLUMN = 4;
    const s32 page_count = achievements.length / MAX_ACHIEVEMENTS_PER_COLUMN + ((achievements.length%MAX_ACHIEVEMENTS_PER_COLUMN) > 0);

    auto& achievement_menu = state->achievement_menu;
    GameUI::set_ui_id((char*)"ui_achievements_menu");

    // render achievement boxes.
    // Will need to make some nice UI for all of this but at least that's art stuff.
    {

        auto locked_title_font       = resources->get_font(MENU_FONT_COLOR_STEEL);
        auto unlocked_title_font       = resources->get_font(MENU_FONT_COLOR_GOLD);
        auto description_font = resources->get_font(MENU_FONT_COLOR_WHITE);

        for (s32 i = 0; i < MAX_ACHIEVEMENTS_PER_COLUMN; ++i) {
            s32 actual_i = i + MAX_ACHIEVEMENTS_PER_COLUMN * achievement_menu.page;
            if (actual_i >= achievements.length) break;

            auto& achievement = achievements[actual_i];

            rectangle_f32 rectangle = rectangle_f32(30, i * 65 + 80, 300, 60);
            render_commands_push_quad(commands, rectangle, color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);
            {
                string text = achievement.name;
                render_commands_push_text(commands,
                                          (achievement.achieved) ? unlocked_title_font : locked_title_font,
                                          2, V2(rectangle.x+10, rectangle.y+10), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                string text = achievement.description;
                render_commands_push_text(commands, description_font, 2, V2(rectangle.x+10, rectangle.y+35), text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
        }
    }

    GameUI::begin_frame(commands);
    {
        GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
        GameUI::label(V2(0, 0), string_literal("ACHIEVEMENTS"), color32f32(1, 1, 1, 1), 4);

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
    switch (state->ui_state) {
        case UI_STATE_INACTIVE: {
            if (state->screen_mode != GAME_SCREEN_CREDITS) {
                GameUI::set_ui_id(0);
            }
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
        case UI_STATE_OPTIONS: {
            update_and_render_options_menu(commands, dt); 
        } break;
        case UI_STATE_STAGE_SELECT: {
            update_and_render_stage_select_menu(commands, dt); 
        } break;
        case UI_STATE_DEAD_MAYBE_RETRY: {
            update_and_render_game_death_maybe_retry_menu(commands, dt);
        } break;
        case UI_STATE_ACHIEVEMENTS: {
            update_and_render_achievements_menu(commands, dt);
        } break;
        default: {
            unimplemented("Unknown ui state type");
        } break;
    }

    DebugUI::render(commands, resources->get_font(MENU_FONT_COLOR_WHITE));
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
    auto& timer                = complete_stage_state.stage_timer;

    auto title_font    = resources->get_font(MENU_FONT_COLOR_BLOODRED);
    auto subtitle_font = resources->get_font(MENU_FONT_COLOR_GOLD);

    f32 timer_percentage = timer.percentage();
    if (timer_percentage > 1.0) timer_percentage = 1.0f;

    s32 stage_id = state->mainmenu_data.stage_id_level_select;
    s32 level_id = state->mainmenu_data.stage_id_level_in_stage_select;

    timer.start();

    string level_complete_text = string_literal("LEVEL COMPLETE");
    string score_string_result = string_from_cstring(format_temp(
                                                         (game_will_be_new_high_score(stage_id, level_id, state->gameplay_data.current_score)) ?
                                                         "New High Score: %d" :
                                                         "Score: %d",
                                                         state->gameplay_data.current_score));

    switch (complete_stage_state.stage) {
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN: {
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128 * timer_percentage), BLEND_MODE_ALPHA);
            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_SHOW_SCORE: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                auto   text_width = font_cache_text_width(subtitle_font, score_string_result, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), score_string_result, color32f32(1, 1, 1, timer_percentage), BLEND_MODE_ALPHA);
            }

            if (timer.triggered()) {
                timer = Timer(0.45f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_WAIT_UNTIL_FADE_OUT: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                auto   text_width = font_cache_text_width(subtitle_font, score_string_result, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), score_string_result, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
            }

            if (timer.triggered()) {
                timer = Timer(0.35f);
                timer.reset();
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT;
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_OUT: {
            f32 rect_y = commands->screen_height/2-32;
            render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128 * (1 - timer_percentage)), BLEND_MODE_ALPHA);
            {
                auto   text_width = font_cache_text_width(title_font, level_complete_text, 4);
                render_commands_push_text(commands, title_font, 4, V2(commands->screen_width/2 - text_width/2, rect_y), level_complete_text, color32f32(1, 1, 1, 1 - timer_percentage), BLEND_MODE_ALPHA);
            }
            {
                rect_y += 64;
                auto   text_width = font_cache_text_width(subtitle_font, score_string_result, 2);
                render_commands_push_text(commands, subtitle_font, 2, V2(commands->screen_width/2 - text_width/2, rect_y), score_string_result, color32f32(1, 1, 1, 1 - timer_percentage), BLEND_MODE_ALPHA);
            }

            if (timer.triggered())  {
                _debugprintf("Okay. Transition bye?");
                complete_stage_state.stage = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE;

                game_update_stage_score(stage_id, level_id, state->gameplay_data.current_score);
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

                s32 completion_type = game_complete_stage_level(stage_id, level_id);

                Transitions::do_color_transition_in(
                    color32f32(0, 0, 0, 1),
                    0.25f,
                    0.5f
                );

                switch (completion_type) {
                    case GAME_COMPLETE_STAGE_UNLOCK_NEXT_STAGE: {
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
                    } break;
                    case GAME_COMPLETE_STAGE_UNLOCK_NEXT_LEVEL: {
                        Transitions::register_on_finish(
                            [&](void*) mutable {
                                state->ui_state    = UI_STATE_INACTIVE;
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
                                state->ui_state    = UI_STATE_STAGE_SELECT;
                                switch_screen(GAME_SCREEN_MAIN_MENU);
                                _debugprintf("Hi main menu. We can do some more stuff like demonstrate if we unlocked a new stage!");

                                Transitions::do_shuteye_out(
                                    color32f32(0, 0, 0, 1),
                                    0.15f,
                                    0.3f
                                );
                            }
                        );
                    } break;
                }

                // Register save game on any level completion.
                save_game();
            }
        } break;
        case GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_NONE: {} break;
    }

    timer.update(dt);
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

    if (Action::is_pressed(ACTION_MENU)) {
        if (this->state->ui_state != UI_STATE_PAUSED) {
            switch_ui(UI_STATE_PAUSED);
        } else {
            switch_ui(UI_STATE_INACTIVE);
        }
    }
#if 0
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

    if (Input::is_key_pressed(KEY_X)) {
        Explosion_Hazard h = Explosion_Hazard(state->player.position, 125, 0.5f, 1.0f);
        state->explosion_hazards.push(h);
    }
#endif

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
        auto border_color = color32u8(0, 15, 18, 255);
 
        int play_area_width = state->play_area.width;
        int play_area_x     = state->play_area.x;

        // These should also be nice images in the future.
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

        // Render_Score
        // Draw score and other stats like attack power or speed or something
        {
            auto font = resources->get_font(MENU_FONT_COLOR_STEEL);
            auto font1 = resources->get_font(MENU_FONT_COLOR_GOLD);
            auto text = string_from_cstring(format_temp("Score: %d", state->current_score));

            // show scoring notifications (for interesting scoring reasons like picking up points or killing an enemy)
            // you'll gradually accumulate score just from surviving on a map...
            // NOTE: hitmarker scores are rendered on the game layer.
            {
                for (s32 index = 0; index < state->score_notifications.size; ++index) {
                    auto& s = state->score_notifications[index];
                    auto text = string_from_cstring(format_temp("%d", s.additional_score));
                    s.lifetime.update(dt);

                    if (s.lifetime.triggered()) {
                        state->score_notifications.pop_and_swap(index);
                        continue;
                    }

                    render_commands_push_text(&ui_render_commands,
                                              font1,
                                              2,
                                              V2(play_area_x+play_area_width + 10,
                                                 100 + normalized_sinf(s.lifetime.percentage()) * -GAMEPLAY_UI_SCORE_NOTIFICATION_RISE_AMOUNT),
                                              text, color32f32(1,1,1,1), BLEND_MODE_ALPHA); 
                }
            }

            render_commands_push_text(&ui_render_commands, font, 2, V2(play_area_x+play_area_width + 10, 100), text, color32f32(1,1,1,1), BLEND_MODE_ALPHA); 
        }
    }

    if (!state->paused_from_death && this->state->ui_state == UI_STATE_INACTIVE && !state->triggered_stage_completion_cutscene) {
        // Update Auto Score
        {
            if (state->auto_score_timer >= DEFAULT_AUTO_SCORE_INTERVAL) {
                state->current_score    += 1;
                state->auto_score_timer  = 0;
            } else {
                state->auto_score_timer += dt;
            }
        }

        struct Entity_Loop_Update_Packet {
            Game_State* game_state;
            f32         dt;
        };

        auto update_packet_data = (Entity_Loop_Update_Packet*)Global_Engine()->scratch_arena.push_unaligned(sizeof(Entity_Loop_Update_Packet));
        update_packet_data->dt = dt;
        update_packet_data->game_state = this->state;

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

        Thread_Pool::add_job(
            [](void* ctx) {
                auto packet = (Entity_Loop_Update_Packet*) ctx;
                Game_State* game_state = packet->game_state;
                Gameplay_Data* state = &packet->game_state->gameplay_data;
                f32 dt = packet->dt;
                state->player.update(game_state, dt);
                return 0;
            },
            (void*)update_packet_data
        );

        // these need to play sounds and a few other non-thread safe behaviors
        // and besides, might as well have these guys just burn some time while we wait
        // for the heavier loads...
        handle_all_explosions(dt);
        handle_all_lasers(dt);

        Thread_Pool::synchronize_tasks();

        // NOTE: these deliberately have to be after,
        // because I need clean up to happen at the end exactly. 

        handle_all_bullet_collisions(dt);
        handle_all_dead_entities(dt);

        // Actually spawn the stuff we wanted to make...
        state->reify_all_creation_queues();
    }
    handle_ui_update_and_render(&ui_render_commands, dt);

    // Handle transitions or stage specific data
    // so like cutscene/coroutine required sort of behaviors...
    if (this->state->ui_state == UI_STATE_INACTIVE) {
        if (!Transitions::fading()) {
            if (state->intro.stage != GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE) {
                ingame_update_introduction_sequence(&ui_render_commands, resources, dt);
            } else {
                bool can_finish_stage = stage_update(&state->stage_state, dt, this->state);

                if (!state->triggered_stage_completion_cutscene && can_finish_stage) {
                    state->triggered_stage_completion_cutscene = true;
                    {
                        state->complete_stage.stage       = GAMEPLAY_STAGE_COMPLETE_STAGE_SEQUENCE_STAGE_FADE_IN;
                        state->complete_stage.stage_timer = Timer(0.35f);
                    }
                }

                if (state->triggered_stage_completion_cutscene) {
                    ingame_update_complete_stage_sequence(&ui_render_commands, resources, dt);
                }
            }
        }
    }

    // main game rendering
    // draw stage specific things if I need to.
    { 
        stage_draw(&state->stage_state, dt, &game_render_commands, this->state);
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

        for (int i = 0; i < (s32)state->enemies.size; ++i) {
            auto& e = state->enemies[i];
            e.draw(this->state, &game_render_commands, resources);
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
                render_commands_push_text(&game_render_commands,
                                          font1,
                                          text_size,
                                          s.where + V2(-text_width/2, normalized_sinf(s.lifetime.percentage()) * -GAMEPLAY_UI_SCORE_NOTIFICATION_RISE_AMOUNT),
                                          text, color32f32(1,1,1, 1 - s.lifetime.percentage()), BLEND_MODE_ALPHA); 
            }
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
    if (Action::is_pressed(ACTION_SCREENSHOT)) {
        _debugprintf("Saved a screenshot!"); // picture sound?
        driver->screenshot((char*)"screenshot.png");
    }

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

    state->coroutine_tasks.scheduler(state, dt);

    // Achievement related updates.
    {
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
        {
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

    // Achievement notifications are omnipresent.
    // I want them to render on top of the transitions.
    {
        auto resolution                  = driver->resolution();
        auto ui_render_commands          = render_commands(&Global_Engine()->scratch_arena, 512, camera(V2(0, 0), 1));
        ui_render_commands.screen_width  = resolution.x;
        ui_render_commands.screen_height = resolution.y;

        update_and_render_achievement_notifications(&ui_render_commands, dt);
        driver->consume_render_commands(&ui_render_commands);
    }

    total_playtime += dt;
}

void Game::handle_all_dead_entities(f32 dt) {
    auto state = &this->state->gameplay_data;

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
            for (int i = 0; i < state->bullets.size; ++i) {
                auto& b = state->bullets[i];
                if (b.die) {state->bullets.pop_and_swap(i);}
            }
            return 0;
        }, state);

    Thread_Pool::add_job(
        [](void* ctx) {
            Gameplay_Data* state = (Gameplay_Data*) ctx;
            for (int i = 0; i < state->enemies.size; ++i) {
                auto& e = state->enemies[i];
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
    }
}

// NOTE: hazards only work on players which is neat.
void Game::handle_all_explosions(f32 dt) {
    auto state = &this->state->gameplay_data;
    for (int i = 0; i < state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];

        if (h.exploded) {
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
        }
    }
}

// TODO: Spatial partition all the bullets some how. Probably going to use another spatial hash.
void Game::handle_all_bullet_collisions(f32 dt) {
    // NOTE: for now entities will die in one hit.
    auto state = &this->state->gameplay_data;

    for (s32 bullet_index = 0; bullet_index < state->bullets.size; ++bullet_index) {
        auto& b = state->bullets[bullet_index];
        auto bullet_rect = b.get_rect();

        // NOTE: does not account for invulnerability right now.
        if (b.source_type == BULLET_SOURCE_NEUTRAL || b.source_type == BULLET_SOURCE_PLAYER) {
            for (s32 enemy_index = 0; enemy_index < state->enemies.size; ++enemy_index) {
                auto& e = state->enemies[enemy_index];
                auto enemy_rect = e.get_rect();

                if (rectangle_f32_intersect(enemy_rect, bullet_rect)) {
                    e.damage(1);
                    if (e.die) {
                        state->notify_score_with_hitmarker(e.score_value * e.death_multiplier, e.position);   
                    } else {
                        state->notify_score_with_hitmarker(e.score_value, e.position);
                    }
                    b.die = true;
                    break;
                }
            }
        }

        if (b.source_type == BULLET_SOURCE_NEUTRAL || b.source_type == BULLET_SOURCE_ENEMY) {
            auto& p = state->player;
            auto player_rect = p.get_rect();

            if (rectangle_f32_intersect(player_rect, bullet_rect)) {
                if (p.kill()) {
                    b.die = true;
                }
                break;
            }
        }
    }
    // unimplemented("Not done");
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

    switch (ui) {
        case UI_STATE_ACHIEVEMENTS: {
            state->achievement_menu.page = 0;
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

// save game information
local string save_file_name = string_literal("game_save.save");

void Game::save_game() {
    _debugprintf("Attempting to save game.");
    if (OS_file_exists(save_file_name)) {
        _debugprintf("NOTE: overriding old save file.");
    }

    auto serializer = open_write_file_serializer(save_file_name);
    serializer.expected_endianess = ENDIANESS_LITTLE;
    serialize_game_state(&serializer);
    serializer_finish(&serializer);
}

void Game::load_game() {
    if (OS_file_exists(save_file_name)) {
        _debugprintf("Attempting to load save game.");
        auto serializer = open_read_file_serializer(save_file_name);
        serializer.expected_endianess = ENDIANESS_LITTLE;
        auto updated_save_data = serialize_game_state(&serializer);
        serializer_finish(&serializer);

        update_from_save_data(&updated_save_data);
        _debugprintf("Hopefully loaded.");
    } else {
        _debugprintf("Save file does not exist. Nothing to load.");
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
        {
            save_data.playtime = total_playtime;
        }
    }

    return save_data;
}

void Game::update_from_save_data(Save_File* save_data) {
    // TODO: nothing for scores yet

    for (int stage_index = 0; stage_index < 4; ++stage_index) {
        auto& stage = stage_list[stage_index];
        stage.unlocked_levels = save_data->stage_unlocks[stage_index];

        for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
            auto& level = stage.levels[level_index];
            level.last_score = save_data->stage_last_scores[stage_index][level_index];
            level.best_score = save_data->stage_best_scores[stage_index][level_index];
        }
    }

    total_playtime = save_data->playtime;
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

        // only one version for now anyways.
        switch (save_data.version) {
            case SAVE_FILE_VERSION_0: {
                _debugprintf("Rejecting prerelease save file");
                return save_data;
            } break;
            default: {
                for (int stage_index = 0; stage_index < 4; ++stage_index) {
                    for (int level_index = 0; level_index < MAX_LEVELS_PER_STAGE; ++level_index) {
                        serialize_s32(serializer, &save_data.stage_last_scores[stage_index][level_index]);
                        serialize_s32(serializer, &save_data.stage_best_scores[stage_index][level_index]);
                    }
                }
                for (int stage_index = 0; stage_index < 4; ++stage_index) {
                    serialize_s32(serializer, &save_data.stage_unlocks[stage_index]);
                }
                serialize_u8(serializer, &save_data.post_game);
                serialize_f32(serializer, &save_data.playtime);

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

// Game_Task_Scheduler
s32 Game_Task_Scheduler::first_avaliable_task() {
    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];

        if (task.source == GAME_TASK_AVALIABLE                                  ||
            jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED) {
            return index;
        }
    }

    return -1;
}

s32 Game_Task_Scheduler::add_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_GAME;
    task.associated_state = current_screen_state;
    task.essential        = essential;
    task.userdata         = (void*)state;
    task.coroutine        = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.yielded;

    return first_free;
}

s32 Game_Task_Scheduler::add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential) {
    s32 current_ui_state = state->ui_state;
    s32 first_free       = first_avaliable_task();

    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_UI;
    task.associated_state = current_ui_state;
    task.essential        = essential;
    task.userdata         = (void*)state;
    task.coroutine        = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.yielded;

    return first_free;
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f) {
    s32 first_free       = first_avaliable_task();
    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_ALWAYS;
    task.associated_state = -1;
    task.essential        = true;
    task.userdata         = (void*)0;
    task.coroutine        = jdr_coroutine_new(f);
    // NOTE: need userdata info.
    task.coroutine.userdata = &task.yielded;

    return first_free;
}

bool Game_Task_Scheduler::kill_task(s32 index) {
    auto& task            = tasks[index];
    if (task.source != GAME_TASK_AVALIABLE) {
        task.source = GAME_TASK_AVALIABLE;
    } else {
        return false; // already dead.
    }
    return true; 
}

void Game_Task_Scheduler::scheduler(struct Game_State* state, f32 dt) {
    s32 current_ui_state     = state->ui_state;
    s32 current_screen_state = state->screen_mode;
    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];

        if (jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED || task.source == GAME_TASK_AVALIABLE) continue;

        {
            switch (task.yielded.reason) {
                case TASK_YIELD_REASON_NONE: {} break;
                case TASK_YIELD_REASON_WAIT_FOR_SECONDS: {
                    if (task.yielded.timer < task.yielded.timer_max) {
                        /*
                         * Special case for Game tasks, which should logically
                         * not advance if the game is in any UI.
                         */
                        if (task.source == GAME_TASK_SOURCE_GAME && current_ui_state != UI_STATE_INACTIVE)
                            break;

                        task.yielded.timer += dt;
                        continue;
                    } else {
                        task.yielded.reason = TASK_YIELD_REASON_NONE;
                    }
                } break;
            }
        }

        if ((task.source == GAME_TASK_SOURCE_ALWAYS)                                          ||
            (task.source == GAME_TASK_SOURCE_UI && task.associated_state == current_ui_state) ||
            (task.source == GAME_TASK_SOURCE_GAME && task.associated_state == current_screen_state)) {
            jdr_resume(&task.coroutine);
        }
    }
}

#include "credits_mode.cpp"
#include "title_screen_mode.cpp"
#include "main_menu_mode.cpp"

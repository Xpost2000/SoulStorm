// NOTE: meant to be included inside of game.cpp
// main menu code

// cutscene coroutines.
void cutscene_completed_maingame_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = Global_Engine()->driver->resolution();

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);

    *trauma_timer = 0;

    main_menu_state->screen_message_add(string_literal("Congratulations!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("I hope you enjoyed your time!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("Feel free to maximize your score!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("Otherwise, I hope to see you again!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}


#if 0
    // post game portal focus and spawn.
    // Using a similar animation to the player.
    {
        auto focus_portal = &main_menu_state->portals[3];
        auto position = focus_portal->position;
        position *= 1.5f;
        camera_set_point_to_interpolate(&camera, position, 1.5f);
    }

    main_menu_state->screen_message_add(string_literal("Prepare yourself well."));
    main_menu_state->screen_message_add(string_literal("Your final challenge awaits."));
    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    // do the same shake and spawn effect.
    // maybe play a sound?
    while ((*trauma_timer) < 1.5f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, clamp<f32>(*trauma_timer + 0.35f, 0.15f, 0.35f));
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.7f, 0.7f, 350);
    camera_traumatize(&camera, 0.58f);
    {
        auto& focus_portal = main_menu_state->portals[3];
        focus_portal.visible = true;
    }

    camera_set_point_to_interpolate(&camera, V2(resolution.x/2, resolution.y/2), 1.0f);

    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }
#endif
    TASK_WAIT(0.5f);

    main_menu_state->cutscene1.phase = 0;
    JDR_Coroutine_End;
}

void cutscene_introduction_fasttrack_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = Global_Engine()->driver->resolution();

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);
    *trauma_timer = 0.0f;
    main_menu_state->player.visible = false;

    TASK_WAIT(0.25f);
    main_menu_state->screen_message_add(string_literal("Welcome back!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("There's always more to do!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}

    while ((*trauma_timer) < 0.45f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, 0.17f);
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.5f, 0.5f, 350);
    camera_traumatize(&camera, 0.55f);

    main_menu_state->player.visible = true;
    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    main_menu_state->cutscene2.phase = 0;
    JDR_Coroutine_End;
}

void cutscene_introduction_firsttime_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = Global_Engine()->driver->resolution();

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);
    *trauma_timer = 0.0f;
    main_menu_state->player.visible = false;

    TASK_WAIT(1.0f);
    camera_set_point_to_interpolate(&camera, V2(resolution.x, resolution.y), 2.0);

    // wait for camera to finish manually...
    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    while ((*trauma_timer) < 1.5f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, 0.15f);
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.4f, 0.4f, 350);
    camera_traumatize(&camera, 0.47f);

    main_menu_state->player.visible = true;
    main_menu_state->screen_message_add(string_literal("Now the adventure begins..."));

    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    TASK_WAIT(1.0f);
    camera_set_point_to_interpolate(&camera, V2(resolution.x/2, resolution.y/2), 1.0);

    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    main_menu_state->cutscene2.phase = 0;
    JDR_Coroutine_End;
}


// NOTE: this main menu entity code is pretty similar to the
//       main entity code, but I just don't wanna have the baggage of all the gameplay
//       specific stuff.

void MainMenu_Stage_Portal::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (!visible) return;

    auto r = get_rect();

    auto& stage_details   = stage_list[stage_id];
    auto  name_string     = stage_details.name;
    auto  subtitle_string = stage_details.subtitle;

    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 255, 0, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA);

    // sort of like a debug renderer.
    // This should really just be some sort of colored vortex with a particle system.
    // Maybe some special effect which fades into a themed background type.
    render_commands_push_text(commands,
                              resources->get_font(MENU_FONT_COLOR_GOLD),
                              1, position,
                              name_string, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
    render_commands_push_text(commands,
                              resources->get_font(MENU_FONT_COLOR_GOLD),
                              1, position + V2(0,scale.y),
                              subtitle_string, color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
}

rectangle_f32 MainMenu_Stage_Portal::get_rect() {
    return rectangle_f32(
        position.x - scale.x,
        position.y - scale.y,
        scale.x*2,
        scale.y*2
    );
}

// MainMenu_Player
rectangle_f32 MainMenu_Player::get_rect() {
    return rectangle_f32(
        position.x - scale.x,
        position.y - scale.y,
        scale.x*2,
        scale.y*2
    );
}

void MainMenu_Player::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (!visible) return;
    auto r = get_rect();

    // black rectangles for default
    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 0, 0, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA);

    render_commands_push_image(commands,
                               graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                               rectangle_f32(position.x - scale.x, position.y - scale.x, scale.x*2, scale.x*2),
                               RECTANGLE_F32_NULL,
                               color32f32(1.0, 0, 1.0, 0.5f),
                               0,
                               BLEND_MODE_ALPHA);
}

void MainMenu_Player::update(MainMenu_Data* state, f32 dt) {
    V2 axes = V2(Action::value(ACTION_MOVE_LEFT) + Action::value(ACTION_MOVE_RIGHT), Action::value(ACTION_MOVE_UP) + Action::value(ACTION_MOVE_DOWN));
    if (axes.magnitude_sq() > 1.0f) axes = axes.normalized();

    const float UNIT_SPEED = 350;

    velocity.x = axes[0] * UNIT_SPEED;
    velocity.y = axes[1] * UNIT_SPEED;

    position += velocity * dt;
}

// MainMenu_Player End

// MainMenu_Pet
// Appreciate interaction cannot be randomly decided.
local f32 pet_weight_actions_by_type[GAME_PET_ID_COUNT][MAIN_MENU_PET_MAIN_ACTIONS] = {
    // cat
    { // Should be lazier and move less.
        0.2,
        0.1,
        0.2,
        0.3,
        0.2,
        0.07
    },
    // dog
    {
        0.1,
        0.1,
        0.1,
        0.5,
        0.2,
        0.07
    },  
    // fish
    {
        0.3,
        0.1,
        0.2,
        0.2,
        0.2,
        0.07
    }   
};

// Everything is assumed to be +/- 0.10 seconds
local f32 pet_action_lengths[GAME_PET_ID_COUNT][MAIN_MENU_PET_MAIN_ACTIONS] = {
    // cat
    {
        5,
        5,
        5,
        2.0f,
        0.10f,
        0.10f,
    },
    // dog
    {
        3,
        3,
        3,
        2.0f,
        0.10f,
        0.10f,
    },
    // fish
    {
        1.5,
        1.7,
        1.5,
        4.5f,
        0.15f,
        0.10f,
    }
};

local f32 pet_velocities[GAME_PET_ID_COUNT] = {
    100,
    120,
    50
};

void MainMenu_Pet::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    // pets are always visible.
    auto r = get_rect();

    // black rectangles for default
    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(255, 0, 0, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA);
}

void MainMenu_Pet::update(MainMenu_Data* state, f32 dt) {
    switch (current_action) {
        case MAIN_MENU_PET_ACTION_IDLE: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_IDLE1: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_IDLE2: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_MOVING: {
            position += current_direction * pet_velocities[type] * dt;
        } break;
        case MAIN_MENU_PET_ACTION_MAKE_NOISE: {
            if (action_timer <= 0.0f) {
                _debugprintf("woof.");
            }
        } break;
        case MAIN_MENU_PET_ACTION_POOP: {
            if (action_timer <= 0.0f) {
                _debugprintf("dumpy.");
                state->spawn_poop(position);
            }
        } break;
        case MAIN_MENU_PET_ACTION_APPRECIATE_INTERACTION: {
            // smile :)
            // spawn a heart or something.
            // otherwise nothing.
        } break;
    }

    if (action_timer <= 0.0f) {
        decide_new_action(&state->prng);
    } else {
        action_timer -= dt;
    }
}

void MainMenu_Pet::decide_new_action(random_state* prng) {
    current_action = random_weighted_selection(
        prng,
        pet_weight_actions_by_type[type],
        MAIN_MENU_PET_MAIN_ACTIONS
    );

    action_timer = pet_action_lengths[type][current_action] + random_ranged_float(prng, -0.1, 0.1);

    current_direction = V2_direction_from_degree(random_ranged_float(prng, -360.0f, 360.0f));
}

rectangle_f32 MainMenu_Pet::get_rect(void) {
    return rectangle_f32(
        position.x - 16,
        position.y - 16,
        32,
        32
    );
}

// MainMenu_Pet End

// MainMenu_Clutter_Poop
void MainMenu_Clutter_Poop::update(f32 dt) {
    if (lifetime <= 0.0f) {
        dead = true;
    } else {
        lifetime -= dt;
    }
}

void MainMenu_Clutter_Poop::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (dead) return;

    auto r = rectangle_f32(
        position.x - 8,
        position.y - 8,
        16,
        16
    );

    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 0, 255, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA
    );
}
// MainMenu_Clutter_Poop end

// MainMenu_Sparkling_Star_Data
void MainMenu_Sparkling_Star_Data::update(f32 dt) {
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

void MainMenu_Sparkling_Star_Data::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
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
// End MainMenu_Sparkling_Star_Data

void MainMenu_Data::spawn_poop(V2 where) {
    MainMenu_Clutter_Poop poop;
    poop.position = where;
    clutter_poops.push(poop);
}

void MainMenu_Data::cleanup_all_dead_poops(void) {
    for (int i = 0; i < clutter_poops.size; ++i) {
        if (clutter_poops[i].dead) {
            clutter_poops.pop_and_swap(i);
        }
    }
}

void MainMenu_Data::start_completed_maingame_cutscene(Game_State* game_state) {
    if (!cutscene1.triggered) {
        _debugprintf("Starting main game completion cutscene!");
        cutscene1.triggered        = true;
        cutscene1.phase            = 1;
        game_state->coroutine_tasks.add_task(game_state, cutscene_completed_maingame_task);
    }
}

void MainMenu_Data::start_introduction_cutscene(Game_State* game_state, bool fasttrack){
    if (!cutscene2.triggered) {
        cutscene2.phase     = 1;
        cutscene2.triggered = true;
        _debugprintf("Triggering cutscene introduction");

        if (fasttrack) {
            _debugprintf("Fast track intro");
            game_state->coroutine_tasks.add_task(game_state, cutscene_introduction_fasttrack_task);
        } else {
            _debugprintf("Standard intro");
            game_state->coroutine_tasks.add_task(game_state, cutscene_introduction_firsttime_task);
        }
    }
}

bool MainMenu_Data::cutscene_active() {
    return cutscene1.phase != 0 || cutscene2.phase != 0;
}

bool MainMenu_Data::screen_messages_finished() {
    return (screen_message_fade_t <= 0.0f && screen_messages.size == 0);
}

void MainMenu_Data::screen_message_add(string message) {
    MainMenu_ScreenMessage new_message;
    new_message.phase = MAIN_MENU_SCREEN_MESSAGE_APPEAR;
    new_message.timer = 0.0f;
    new_message.text = message;
    screen_messages.push(new_message);
}

void Game::mainmenu_data_initialize(Graphics_Driver* driver) {
    {
        auto state = &this->state->mainmenu_data;
        // auto resolution = driver->resolution();
        auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

        state->particle_pool.init(arena, 1024);
        
        state->player.position      = V2(resolution.x / 2, resolution.y / 2);
        state->player.scale         = V2(15, 15);
        state->player.velocity      = V2(0, 0);
        // I need to utilize a camera effect which relies on centering
        // for polish reasons.
        state->main_camera          = camera(V2(resolution.x/2, resolution.y/2), 1.0);
        state->main_camera.centered = true;
        state->main_camera.rng      = &state->prng;
        state->prng                 = random_state();

        state->screen_messages = Fixed_Array<MainMenu_ScreenMessage>(arena, 32);
        state->clutter_poops   = Fixed_Array<MainMenu_Clutter_Poop>(arena, 64);
        state->portals = Fixed_Array<MainMenu_Stage_Portal>(arena, 4);
        {
            // initialize all portals here for the main menu
            // portals should be spaced differently based on what's unlocked
            // but I'll just place them manually
            {
                auto& portal = state->portals[0]; 
                portal.stage_id = 0;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.visible = true;

                {
                    auto& emitter = portal.emitter_main;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
                    emitter.scale  = 1.0f;
                    emitter.lifetime = 1.0f;
                    emitter.velocity_x_variance = V2(-100, 100);
                    emitter.velocity_y_variance = V2(-100, 100);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-100, 100);
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.emission_max_timer = 0.025f;
                }

                {
                    auto& emitter = portal.emitter_vortex;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]);
                    emitter.scale  = 0.5f;
                    emitter.lifetime = 2.0f;
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.use_attraction_point = true;
                    emitter.attraction_force     = 100.0f;
                    emitter.emission_max_timer = 0.025f;
                }
            }

            {
                auto& portal = state->portals[1]; 
                portal.stage_id = 1;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.visible = true;

                {
                    auto& emitter = portal.emitter_main;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
                    emitter.scale  = 1.0f;
                    emitter.lifetime = 1.0f;
                    emitter.velocity_x_variance = V2(-100, 100);
                    emitter.velocity_y_variance = V2(-100, 100);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-100, 100);
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.emission_max_timer = 0.025f;
                }

                {
                    auto& emitter = portal.emitter_vortex;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]);
                    emitter.scale  = 0.5;
                    emitter.lifetime = 2.0f;
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.use_attraction_point = true;
                    emitter.attraction_force     = 100.0f;
                    emitter.emission_max_timer = 0.025f;
                }
            }

            {
                auto& portal = state->portals[2]; 
                portal.stage_id = 2;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.prerequisites[1] = 1;
                portal.visible = true;

                {
                    auto& emitter = portal.emitter_main;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
                    emitter.scale  = 1.0f;
                    emitter.lifetime = 1.0f;
                    emitter.velocity_x_variance = V2(-100, 100);
                    emitter.velocity_y_variance = V2(-100, 100);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-100, 100);
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.emission_max_timer = 0.025f;
                }

                {
                    auto& emitter = portal.emitter_vortex;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]);
                    emitter.scale  = 0.5f;
                    emitter.lifetime = 2.0f;
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.use_attraction_point = true;
                    emitter.attraction_force     = 100.0f;
                    emitter.emission_max_timer = 0.025f;
                }
            }

            // NOTE: postgame portal.
            // NOTE: unused, I do not have time to make more levels :(
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

                {
                    auto& emitter = portal.emitter_main;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
                    emitter.scale  = 1.0f;
                    emitter.lifetime = 1.0f;
                    emitter.velocity_x_variance = V2(-100, 100);
                    emitter.velocity_y_variance = V2(-100, 100);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-100, 100);
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.emission_max_timer = 0.025f;
                }

                {
                    auto& emitter = portal.emitter_vortex;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]);
                    emitter.scale  = 0.5;
                    emitter.lifetime = 2.0f;
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.use_attraction_point = true;
                    emitter.attraction_force     = 100.0f;
                    emitter.emission_max_timer = 0.025f;
                }
            }

            state->portals.size = 4;
        }

        // initializing all the stars' positions
        {
            auto bkg_slow_stars = state->star_positions[0];
            auto bkg_faster_stars = state->star_positions[1];
            auto& prng = this->state->mainmenu_data.prng;

            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                bkg_slow_stars[i] = V2(random_ranged_float(&prng, -854, 854),
                                       random_ranged_float(&prng, -480, 480));
                bkg_faster_stars[i] = V2(random_ranged_float(&prng, -854, 854),
                                         random_ranged_float(&prng, -480, 480));
            }

            auto& sparkling_stars = state->sparkling_stars;
            for (int i = 0; i < MAX_MAINMENU_SPARKLING_STARS; ++i) {
                auto& star = sparkling_stars[i];
                star.visibility_delay_timer = star.max_visibility_delay_timer = random_ranged_float(&prng, 1.25f, 7.00f);
                star.anim_timer = 0.0f;
                star.frame_index = 0;
                star.position = V2(random_ranged_float(&prng, -800, 800), random_ranged_float(&prng, -480, 480));
                star.scale = random_ranged_float(&prng, 1.0f, 1.75f);
            }
        }

        // initialize pets
        {
            // regardless of whether they've been "spawned/visible" I'll seed all of their initial actions
            auto prng = &this->state->mainmenu_data.prng;
            for (int i = 0; i < array_count(state->pets); ++i) {
                state->pets[i].type = i;
                state->pets[i].decide_new_action(prng);
                state->pets[i].position = V2(random_ranged_float(prng, 130.0f, 360.0f),
                                             random_ranged_float(prng, 130.0f, 360.0f));
            }
        }
    }
}

void MainMenu_Data::adjust_entities_for_screen_resolution(int new_screen_width, int new_screen_height) {
    auto& main_menu_state = *this;
    {
        if (main_menu_state.last_screen_width  != new_screen_width ||
            main_menu_state.last_screen_height != new_screen_height) {
            main_menu_state.last_screen_width   = new_screen_width;
            main_menu_state.last_screen_height  = new_screen_height;
            main_menu_state.main_camera.xy = V2(new_screen_width/2, new_screen_height/2);
        }

        // Also portals will be fitted to be at the same "relative" place
        // based on resolution.

        // There is no way to hook into resolution changes, and there is no other way
        // to make the "playable" main menu look nice without hacking this in anyway.

        // NOTE: only the three playable portals (there is a fourth postgame portal that I'm just disabling, and
        // unlikely to re-enable. Ever.)
        {
            f32 screen_width = (new_screen_width);
            f32 screen_third = (screen_width*0.9f / 3.0f);
            for (int i = 0; i < 3; ++i) {
                auto& portal = main_menu_state.portals[i];
                portal.position             = V2(screen_width * 0.1f + (screen_third*1.15) * i, 80);
                portal.emitter_main.shape   = particle_emit_shape_circle(portal.position, 25.0f);
                portal.emitter_vortex.attraction_point     = portal.position;
                portal.emitter_vortex.shape = particle_emit_shape_circle(portal.position, 100.0f);
            }
        }
    }
}

void Game::update_and_render_game_main_menu(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt) {
    auto& main_menu_state = state->mainmenu_data;
    game_render_commands->camera = main_menu_state.main_camera;

    main_menu_state.adjust_entities_for_screen_resolution(game_render_commands->screen_width, game_render_commands->screen_height);
    // TODO: Fix Draw_Main_Menu_Stars
    //       this looks bad sometimes when the resolution is adjusted or something similar
    //       so I need to be careful!
    //
    //       However it should mostly be okay!
    {
        f32 brightness = normalized_sinf(Global_Engine()->global_elapsed_time)/2;
        render_commands_push_quad_ext(
            game_render_commands,
            rectangle_f32(-300, -300, game_render_commands->screen_width*2, game_render_commands->screen_height*2),
            color32u8(brightness, brightness, brightness, 255),
            V2(0, 0), 0,
            BLEND_MODE_ALPHA);

        // background stars are slower

        auto bkg_slow_stars = main_menu_state.star_positions[0];
        auto bkg_faster_stars = main_menu_state.star_positions[1];
        // forward stars are moving more
        {
            // uh. I hope this looks fine.
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                if (dt >= 1) break; // significant frame drop? Do not update (mostly when changing resolution)
                bkg_slow_stars[i].x += dt * 10.0f * (normalized_sinf(i)+0.25);
                bkg_slow_stars[i].y += dt * 15.0f * (normalized_sinf(Global_Engine()->global_elapsed_time)+0.25);

                bkg_faster_stars[i].x += dt * 27.0f * (normalized_sinf(i*25)+0.15);
                bkg_faster_stars[i].y += dt * 39.0f * (sinf(Global_Engine()->global_elapsed_time)/2+0.25);

                if (bkg_faster_stars[i].x > game_render_commands->screen_width*1.5f)  bkg_faster_stars[i].x = -150;
                if (bkg_faster_stars[i].y > game_render_commands->screen_height*1.5f) bkg_faster_stars[i].y = -150;
                if (bkg_slow_stars[i].x > game_render_commands->screen_width*1.5f)  bkg_slow_stars[i].x     = -150;
                if (bkg_slow_stars[i].y > game_render_commands->screen_height*1.5f) bkg_slow_stars[i].y     = -150;
            }

            // Need to make these have different sizes.
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                auto r = rectangle_f32(bkg_slow_stars[i].x, bkg_slow_stars[i].y, 1, 1);
                auto r1 = rectangle_f32(bkg_faster_stars[i].x-30, bkg_faster_stars[i].y+20, 0.5, 0.5);
                auto r2 = rectangle_f32(bkg_faster_stars[i].x-5, bkg_faster_stars[i].y-45, 0.5, 0.5);

                render_commands_push_quad_ext(
                    game_render_commands,
                    r,
                    color32u8(230, 230, 255, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    game_render_commands,
                    r2,
                    color32u8(200, 200, 255, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    game_render_commands,
                    r1,
                    color32u8(100, 100, 245, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );
            }
        }

        for (int i = 0; i < MAX_MAINMENU_SPARKLING_STARS; ++i) {
            main_menu_state.sparkling_stars[i].update(dt);
            main_menu_state.sparkling_stars[i].draw(&main_menu_state, game_render_commands, resources);
        }
    }

#ifndef RELEASE
    if (Input::is_key_pressed(KEY_U)) {
        Achievements::get(ACHIEVEMENT_ID_STAGE3)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE2)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE1)->report();
    }
    if (Input::is_key_pressed(KEY_I)) {
        Achievements::get(ACHIEVEMENT_ID_STAGE3)->_deunlock();
        Achievements::get(ACHIEVEMENT_ID_STAGE2)->_deunlock();
        Achievements::get(ACHIEVEMENT_ID_STAGE1)->_deunlock();
    }
#endif

    if (Action::is_pressed(ACTION_MENU)) {
        if (this->state->ui_state == UI_STATE_STAGE_SELECT ||
            main_menu_state.cutscene1.phase != 0          ||
            main_menu_state.cutscene2.phase != 0
        ) {
            
        } else {
            if (this->state->ui_state != UI_STATE_PAUSED) {
                switch_ui(UI_STATE_PAUSED);
            } else {
                switch_ui(UI_STATE_INACTIVE);
            }
        }
    }

    if (state->ui_state == UI_STATE_INACTIVE) {
        if (!main_menu_state.cutscene_active()) {
            main_menu_state.player.update(&main_menu_state, dt);
        }

        for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
            main_menu_state.pets[i].update(&main_menu_state, dt);
        }

        for (int i = 0; i < main_menu_state.clutter_poops.size; ++i) {
            main_menu_state.clutter_poops[i].update(dt);
        }
    }

    main_menu_state.cleanup_all_dead_poops();

    for (int i = 0; i < main_menu_state.portals.size; ++i) {
        auto& p = main_menu_state.portals[i];
        p.emitter_main.active   = p.visible;

        p.emitter_main.update(&main_menu_state.particle_pool, &main_menu_state.prng, dt);
        p.emitter_vortex.update(&main_menu_state.particle_pool, &main_menu_state.prng, dt);
    }
    main_menu_state.particle_pool.update(state, dt);

    for (int i = 0; i < main_menu_state.portals.size; ++i) {
        auto& p = main_menu_state.portals[i];
        p.draw(&main_menu_state, game_render_commands, resources);
    }

    main_menu_state.particle_pool.draw(game_render_commands, resources);

    for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
        main_menu_state.pets[i].draw(&main_menu_state, game_render_commands, resources);
    }

    for (int i = 0; i < main_menu_state.clutter_poops.size; ++i) {
        main_menu_state.clutter_poops[i].draw(&main_menu_state, game_render_commands, resources);
    }

    main_menu_state.player.draw(&main_menu_state, game_render_commands, resources);
    handle_ui_update_and_render(ui_render_commands, dt);

    // Wrap player to edges
    {
        s32 screen_width = game_render_commands->screen_width;
        s32 screen_height = game_render_commands->screen_height;

        if ((main_menu_state.player.position.x) < 0)                 main_menu_state.player.position.x = screen_width;
        if ((main_menu_state.player.position.x) > screen_width) main_menu_state.player.position.x = 0;
        if ((main_menu_state.player.position.y) < 0)                 main_menu_state.player.position.y = screen_height;
        if ((main_menu_state.player.position.y) > screen_height) main_menu_state.player.position.y = 0;

        // wrap all pets to edges as well
        for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
            auto& pet = main_menu_state.pets[i];
            if ((pet.position.x) < 0)            pet.position.x = screen_width;
            if ((pet.position.x) > screen_width) pet.position.x = 0;
            if ((pet.position.y) < 0)            pet.position.y = screen_height;
            if ((pet.position.y) > screen_height)pet.position.y = 0;
        }
    }

    // do a fancy camera zoom in effect
    { //... NOTE: must resist the urge. To throw everything into coroutines...
        MainMenu_Stage_Portal* focus_portal = nullptr;
        auto circle_player = circle_f32(main_menu_state.player.position.x, main_menu_state.player.position.y, main_menu_state.player.scale.x);
        for (int i = 0; i < main_menu_state.portals.size; ++i) {
            auto& p = main_menu_state.portals[i];
            auto circle_portal = circle_f32(p.position.x, p.position.y, p.scale.x * 3.5f);

            if (!p.visible) continue;

            if (circle_f32_intersect(circle_portal, circle_player)) {
                focus_portal = &p;
                break;
            }

            p.triggered_level_selection = false;
            p.emitter_vortex.active = false;
        } 

        if (focus_portal) {
            if (!focus_portal->triggered_level_selection) {
                focus_portal->triggered_level_selection = true;

                focus_portal->emitter_vortex.reset();
                focus_portal->emitter_vortex.active = true;

                main_menu_state.stage_id_level_select = focus_portal->stage_id;
                switch_ui(UI_STATE_STAGE_SELECT);

                auto position = focus_portal->position;
                position *= 1.5f;

                camera_set_point_to_interpolate(&main_menu_state.main_camera, position, 1.5f);
            }
        } else {
            if (main_menu_state.last_focus_portal != nullptr) {
                V2 resolution = V2(game_render_commands->screen_width, game_render_commands->screen_height);
                if (!camera_already_interpolating_for(&main_menu_state.main_camera, V2(resolution.x/2, resolution.y/2), 1.0f)) {
                    camera_set_point_to_interpolate(
                        &main_menu_state.main_camera,
                        V2(resolution.x/2, resolution.y/2),
                        1.0
                    );
                }
            }
        }

        main_menu_state.last_focus_portal = focus_portal;
    }

    // screen messages
    if (state->ui_state == UI_STATE_INACTIVE) {
        if (main_menu_state.screen_messages.size > 0) {
            main_menu_state.screen_message_fade_t += dt;
        } else {
            main_menu_state.screen_message_fade_t -= dt;
        }

        main_menu_state.screen_message_fade_t = clamp<f32>(main_menu_state.screen_message_fade_t, 0.0f, 1.0f);

        auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
        f32 text_scale = 4.0f;

        render_commands_push_quad_ext(
            ui_render_commands,
            rectangle_f32(0, 0, ui_render_commands->screen_width, ui_render_commands->screen_height),
            color32u8(0, 0, 0, main_menu_state.screen_message_fade_t * 150),
            V2(0, 0), 0,
            BLEND_MODE_ALPHA
        );

        // one message per "screen"
        if (main_menu_state.screen_messages.size > 0) {
            s32 message_index = 0;
            auto& message = main_menu_state.screen_messages[message_index];

            f32 text_width       = font_cache_text_width(font, message.text, text_scale);
            f32 text_height      = font_cache_text_height(font) * text_scale;
            V2  message_position = V2(ui_render_commands->screen_width/2 - text_width/2.0f, ui_render_commands->screen_height/2 - text_height/2);

            // TODO: fix alignment and visual look.
            switch (message.phase) {
                case MAIN_MENU_SCREEN_MESSAGE_APPEAR: {
                    const f32 PHASE_MAX = 0.35f;
                    message.timer += dt;
                    f32 alpha = clamp<f32>(message.timer/PHASE_MAX, 0.0f, 1.0f);

                    render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1, alpha), BLEND_MODE_ALPHA);
                    if (message.timer >= PHASE_MAX) {
                        message.phase = MAIN_MENU_SCREEN_MESSAGE_WAIT_FOR_CONTINUE;
                        message.timer = 0.0f;
                    }
                } break;
                case MAIN_MENU_SCREEN_MESSAGE_WAIT_FOR_CONTINUE: {
                    render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1,1), BLEND_MODE_ALPHA);
                    if (Input::any_key_down() || Input::controller_any_button_down(Input::get_gamepad(0))) {
                        message.phase = MAIN_MENU_SCREEN_MESSAGE_DISAPPEAR;
                    }
                } break;
                case MAIN_MENU_SCREEN_MESSAGE_DISAPPEAR: {
                    const f32 PHASE_MAX = 0.25f;
                    message.timer += dt;
                    f32 alpha = clamp<f32>(1 - message.timer/PHASE_MAX, 0.0f, 1.0f);
                    render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1,alpha), BLEND_MODE_ALPHA);

                    if (message.timer >= PHASE_MAX) {
                        message.timer = 0.0f;

                        main_menu_state.screen_messages.pop_and_swap(message_index);
                    }
                } break;
            }
        }
    }

    Transitions::update_and_render(ui_render_commands, dt);

    game_render_commands->should_clear_buffer = true;
    game_render_commands->clear_buffer_color = color32u8(32, 45, 80, 255);
    camera_update(&main_menu_state.main_camera, dt);
}

// end main menu code

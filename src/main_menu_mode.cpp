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

        if (fasttrack) {
            game_state->coroutine_tasks.add_task(game_state, cutscene_introduction_fasttrack_task);
        } else {
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

void Game::update_and_render_game_main_menu(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt) {
    auto& main_menu_state = state->mainmenu_data;
    game_render_commands->camera = main_menu_state.main_camera;

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
                bkg_slow_stars[i].x += dt * 10.0f * (normalized_sinf(i)+0.25);
                bkg_slow_stars[i].y += dt * 10.0f * (normalized_sinf(Global_Engine()->global_elapsed_time)+0.25);

                bkg_faster_stars[i].x += dt * 250.0f * (normalized_sinf(i*25)+0.15);
                bkg_faster_stars[i].y += dt * 250.0f * (sinf(Global_Engine()->global_elapsed_time)/4+0.25);

                if (bkg_faster_stars[i].x > game_render_commands->screen_width*1.5f)  bkg_faster_stars[i].x = -150;
                if (bkg_faster_stars[i].y > game_render_commands->screen_height*1.5f) bkg_faster_stars[i].y = -150;
                if (bkg_slow_stars[i].x > game_render_commands->screen_width*1.5f)  bkg_slow_stars[i].x     = -150;
                if (bkg_slow_stars[i].y > game_render_commands->screen_height*1.5f) bkg_slow_stars[i].y     = -150;
            }

            // Need to make these have different sizes.
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                auto r = rectangle_f32(bkg_slow_stars[i].x, bkg_slow_stars[i].y, 1, 1);
                auto r1 = rectangle_f32(bkg_faster_stars[i].x, bkg_faster_stars[i].y, 0.5, 0.5);
                auto r2 = rectangle_f32(bkg_faster_stars[i].x-45, bkg_faster_stars[i].y-45, 0.5, 0.5);

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
                    color32u8(200, 245, 200, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    game_render_commands,
                    r1,
                    color32u8(200, 200, 245, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );
            }
        }
    }


    if (Action::is_pressed(ACTION_MENU)) {
        if (this->state->ui_state == UI_STATE_STAGE_SELECT) {
            
        } else {
            if (this->state->ui_state != UI_STATE_PAUSED) {
                switch_ui(UI_STATE_PAUSED);
            } else {
                switch_ui(UI_STATE_INACTIVE);
            }
        }
    }

    // TODO/FIXME: DEBUG FOR RECORD PLAYBACK!
    // I NEED A UI!
    if (Input::is_key_pressed(KEY_F5)) {
        if (OS_file_exists(string_literal("game.recording"))) {
            _debugprintf("Loading recording");

            auto serializer = open_read_file_serializer(string_literal("game.recording"));
            serializer.expected_endianess = ENDIANESS_LITTLE;

            gameplay_recording_file_serialize(
                &state->gameplay_data.recording,
                &Global_Engine()->main_arena,
                &serializer
            );
            gameplay_recording_file_start_playback(
                &state->gameplay_data.recording
            );
            serializer_finish(&serializer);

            Transitions::do_shuteye_in(
                color32f32(0, 0, 0, 1),
                0.15f,
                0.3f
            );

            _debugprintf("Playback on stage (%d, %d)", state->gameplay_data.recording.stage_id+1, state->gameplay_data.recording.level_id+1);
            Transitions::register_on_finish(
                [&](void*) mutable {
                    this->state->mainmenu_data.stage_id_level_select          = state->gameplay_data.recording.stage_id;
                    this->state->mainmenu_data.stage_id_level_in_stage_select = state->gameplay_data.recording.level_id;

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
            _debugprintf("no debug recording");
        }
    }

    if (state->ui_state == UI_STATE_INACTIVE && !main_menu_state.cutscene_active()) {
        main_menu_state.player.update(&main_menu_state, dt);
    }

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

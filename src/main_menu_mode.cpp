// NOTE: meant to be included inside of game.cpp
// main menu code

// NOTE: this main menu entity code is pretty similar to the
//       main entity code, but I just don't wanna have the baggage of all the gameplay
//       specific stuff.

void MainMenu_Stage_Portal::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
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

void Game::update_and_render_game_main_menu(Graphics_Driver* driver, f32 dt) {
    auto& main_menu_state = state->mainmenu_data;

    auto game_render_commands = render_commands(&Global_Engine()->scratch_arena, 12000, main_menu_state.main_camera);
    auto ui_render_commands   = render_commands(&Global_Engine()->scratch_arena, 8192, camera(V2(0, 0), 1));
    {
        V2 resolution = driver->resolution();
        game_render_commands.screen_width  = ui_render_commands.screen_width = resolution.x;
        game_render_commands.screen_height = ui_render_commands.screen_height = resolution.y;
    }

    // Draw main menu stars...
    {
        f32 brightness = normalized_sinf(Global_Engine()->global_elapsed_time)/2;
        render_commands_push_quad_ext(
            &game_render_commands,
            rectangle_f32(-300, -300, game_render_commands.screen_width*2, game_render_commands.screen_height*2),
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

                if (bkg_faster_stars[i].x > game_render_commands.screen_width)  bkg_faster_stars[i].x = 0;
                if (bkg_faster_stars[i].y > game_render_commands.screen_height) bkg_faster_stars[i].y = 0;

                if (bkg_slow_stars[i].x > game_render_commands.screen_width)  bkg_slow_stars[i].x = 0;
                if (bkg_slow_stars[i].y > game_render_commands.screen_height) bkg_slow_stars[i].y = 0;
            }

            // Need to make these have different sizes.
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                auto r = rectangle_f32(bkg_slow_stars[i].x, bkg_slow_stars[i].y, 1, 1);
                auto r1 = rectangle_f32(bkg_faster_stars[i].x, bkg_faster_stars[i].y, 0.5, 0.5);
                auto r2 = rectangle_f32(bkg_faster_stars[i].x-45, bkg_faster_stars[i].y-45, 0.5, 0.5);

                render_commands_push_quad_ext(
                    &game_render_commands,
                    r,
                    color32u8(230, 230, 255, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    &game_render_commands,
                    r2,
                    color32u8(200, 245, 200, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    &game_render_commands,
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

    if (state->ui_state == UI_STATE_INACTIVE) {

        // debug achievement testing.
        if (Input::is_key_pressed(KEY_C)) {
            // Laser_Hazard h = Laser_Hazard(state->player.position.y, 30.0f, LASER_HAZARD_DIRECTION_HORIZONTAL, 1.0f, 15.0f);
            // state->laser_hazards.push(h);

            // NOTE: these could just act as subscribers in the future.
            //       to death events but it's not too big of a deal for now.
            Achievements::get(ACHIEVEMENT_ID_KILLER)->report((s32)999999);
            Achievements::get(ACHIEVEMENT_ID_MURDERER)->report((s32)999999);
            Achievements::get(ACHIEVEMENT_ID_SLAYER)->report((s32)9999999);
        }


        main_menu_state.player.update(&main_menu_state, dt);
    }

    handle_ui_update_and_render(&ui_render_commands, dt);

    // Wrap player to edges
    {
        V2   resolution = driver->resolution();

        if ((main_menu_state.player.position.x) < 0)                 main_menu_state.player.position.x = resolution.x;
        if ((main_menu_state.player.position.x) > (s32)resolution.x) main_menu_state.player.position.x = 0;
        if ((main_menu_state.player.position.y) < 0)                 main_menu_state.player.position.y = resolution.y;
        if ((main_menu_state.player.position.y) > (s32)resolution.y) main_menu_state.player.position.y = 0;
    }

    // do a fancy camera zoom in effect
    {
        MainMenu_Stage_Portal* focus_portal = nullptr;
        auto circle_player = circle_f32(main_menu_state.player.position.x, main_menu_state.player.position.y, main_menu_state.player.scale.x);
        for (int i = 0; i < main_menu_state.portals.size; ++i) {
            auto& p = main_menu_state.portals[i];
            auto circle_portal = circle_f32(p.position.x, p.position.y, p.scale.x * 3.5f);

            if (circle_f32_intersect(circle_portal, circle_player)) {
                focus_portal = &p;
                break;
            }
            p.triggered_level_selection = false;
        } 

        if (focus_portal) {
            if (!focus_portal->triggered_level_selection) {
                focus_portal->triggered_level_selection = true;

                main_menu_state.stage_id_level_select = focus_portal->stage_id;
                switch_ui(UI_STATE_STAGE_SELECT);

                auto position = focus_portal->position;
                position *= 1.5f;

                camera_set_point_to_interpolate(&main_menu_state.main_camera, position, 1.5f);
            }
        } else {
            if (main_menu_state.last_focus_portal != nullptr) {
                V2 resolution = driver->resolution();
                if (camera_not_already_interpolating_for(&main_menu_state.main_camera, V2(resolution.x/2, resolution.y/2), 1.0f)) {
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

    for (int i = 0; i < main_menu_state.portals.size; ++i) {
        auto& p = main_menu_state.portals[i];
        p.draw(&main_menu_state, &game_render_commands, resources);
    }

    main_menu_state.player.draw(&main_menu_state, &game_render_commands, resources);

    Transitions::update_and_render(&ui_render_commands, dt);

    driver->clear_color_buffer(color32u8(32, 45, 80, 255));
    driver->consume_render_commands(&game_render_commands);
    driver->consume_render_commands(&ui_render_commands);

    camera_update(&main_menu_state.main_camera, dt);
}

// end main menu code

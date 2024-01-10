// NOTE: meant to be included inside of game.cpp
// title screen code

void TitleScreen_MainCharacter_Puppet::update(f32 dt) {
    
}

void TitleScreen_MainCharacter_Puppet::draw(struct render_commands* commands, Game_Resources* resources) {
    V2 arm_position_offset  = V2(5, 0);
    V2 head_position_offset = V2(0, -110);
    V2 eye_position_offset   = V2(30, -65);
    V2 eye_spacing_offset   = V2(20, 0);
    V2 eyebrow_spacing_offset = V2(0, -20);

    f32 scale     = 2;
    s32 eye_frame = 0;

    V2 head_position          = position + head_position_offset * scale;
    V2 right_eye_position     = position + (eye_position_offset + eye_spacing_offset) * scale;
    V2 left_eye_position      = position + (eye_position_offset - eye_spacing_offset) * scale;
    V2 right_eyebrow_position = right_eye_position + eyebrow_spacing_offset * scale;
    V2 left_eyebrow_position  = left_eye_position + eyebrow_spacing_offset * scale;
    V2 torso_position         = position;
    V2 left_arm_position      = position - arm_position_offset * scale;
    V2 right_arm_position     = position + (arm_position_offset - V2(3, 0)) * scale;

    {
        V2   position          = left_arm_position;
        V2   position1         = right_arm_position;
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_arm);
        V2   sprite_dimensions = V2(image->width, image->height) * scale;
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position1.x, position1.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   DRAW_IMAGE_FLIP_HORIZONTALLY,
                                   BLEND_MODE_ALPHA);
    }

    {
        V2   position          = torso_position;
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_torso);
        V2   sprite_dimensions = V2(image->width, image->height) * scale;
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
    }

    {
        V2   position          = head_position;
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_head);
        V2   sprite_dimensions = V2(image->width, image->height) * scale;
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
    }

#if 0 // These look a little dumb after seeing them...
    {
        V2   position          = left_eyebrow_position;
        V2   position1         = right_eyebrow_position;
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_eye_brow);
        V2   sprite_dimensions = V2(image->width, image->height) * (scale);
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   DRAW_IMAGE_FLIP_HORIZONTALLY,
                                   BLEND_MODE_ALPHA);
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position1.x, position1.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
    }
#endif

    {
        V2   position          = left_eye_position;
        V2   position1         = right_eye_position;
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_eyes[eye_frame]);
        V2   sprite_dimensions = V2(image->width, image->height) * scale;
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position.x, position.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
        render_commands_push_image(commands,
                                   image,
                                   rectangle_f32(position1.x, position1.y, sprite_dimensions.x, sprite_dimensions.y),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 1.0, 1.0, 1.0),
                                   0,
                                   BLEND_MODE_ALPHA);
    }
}

/*
  The title screen is only for when you first open
  the game.

  This is the only time you can see it, all other interactions
  are from the ingame/main menu screens.

  Although I do want to make this more elaborate, for now I'll just
  use the pause menu code as place holder.
*/
void Game::title_data_initialize(Graphics_Driver* driver) {
    auto state = &this->state->titlescreen_data;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);
    state->main_camera          = camera(V2(resolution.x/2, resolution.y/2), 1.0);
    state->main_camera.centered = true;
    state->main_camera.rng      = &state->prng;
    state->prng                 = random_state();

    // initializing all the stars' positions
    {
        auto& prng = state->prng;

        for (int i = 0; i < array_count(state->star_positions); ++i) {
            state->star_positions[i] = V2(random_ranged_float(&prng, -854, 854),
                                   random_ranged_float(&prng, -480, 480));
        }

        auto& sparkling_stars = state->sparkling_stars;
        for (int i = 0; i < array_count(state->sparkling_stars); ++i) {
            auto& star = sparkling_stars[i];
            star.visibility_delay_timer = star.max_visibility_delay_timer = random_ranged_float(&prng, 3.25f, 7.00f);
            star.anim_timer = 0.0f;
            star.frame_index = 0;
            star.position = V2(random_ranged_float(&prng, -800, 800), random_ranged_float(&prng, -480, 480));
            star.scale = random_ranged_float(&prng, 1.0f, 1.75f);
        }
    }
}

void Game::update_and_render_game_title_screen(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt) {
    auto commands = ui_render_commands;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    // setup camera for rendering main part
    {
        s32 new_screen_width = game_render_commands->screen_width;
        s32 new_screen_height = game_render_commands->screen_height;
        if (state->titlescreen_data.last_screen_width  != new_screen_width ||
            state->titlescreen_data.last_screen_height != new_screen_height) {
            state->titlescreen_data.last_screen_width   = new_screen_width;
            state->titlescreen_data.last_screen_height  = new_screen_height;
            state->titlescreen_data.main_camera.xy = V2(new_screen_width/2, new_screen_height/2);
        }

        // move puppet to center of screen
        {
            state->titlescreen_data.puppet.position = V2(new_screen_width/2, new_screen_height/2);
        }
    }

    game_render_commands->camera = state->titlescreen_data.main_camera;

    {
        // uh. I hope this looks fine.
#if 0
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
#endif

        // Need to make these have different sizes.
        for (int i = 0; i < array_count(state->titlescreen_data.star_positions); ++i) {
            auto r = rectangle_f32(state->titlescreen_data.star_positions[i].x, state->titlescreen_data.star_positions[i].y, 1, 1);

            render_commands_push_quad_ext(
                game_render_commands,
                r,
                color32u8(230, 230, 255, 255),
                V2(0, 0), 0,
                BLEND_MODE_ALPHA
            );
        }

        for (int i = 0; i < array_count(state->titlescreen_data.sparkling_stars); ++i) {
            state->titlescreen_data.sparkling_stars[i].update(dt);
            state->titlescreen_data.sparkling_stars[i].draw(game_render_commands, resources);
        }

        state->titlescreen_data.puppet.update(dt);
        state->titlescreen_data.puppet.draw(game_render_commands, resources);
    }

    if (state->ui_state != UI_STATE_INACTIVE) {
        handle_ui_update_and_render(commands, dt);
    } else {
        GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

        GameUI::set_ui_id((char*)"ui_titlescreen_menu");
        GameUI::begin_frame(commands, &resources->graphics_assets);
        {
            f32 y = 100;
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));

            y += 45;

            if (GameUI::button(V2(100, y), string_literal("Play"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Transitions::do_shuteye_in(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );

                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_MAIN_MENU);

                        // no save file? Going to start
                        if (!load_game()) {
                            // first time, we'll load the full cutscene...
                            auto state = &this->state->mainmenu_data;
                            state->start_introduction_cutscene(this->state, false);
                            save_game();
                            _debugprintf("First time cutscene load");
                        } else {
                            auto state = &this->state->mainmenu_data;
                            state->start_introduction_cutscene(this->state, true);
                            _debugprintf("Found save file, loading regular continuation cutscene");
                        }
                    }
                );
            }
            y += 30;

            if (GameUI::button(V2(100, y), string_literal("Options"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_OPTIONS);
            }
            y += 30;

            if (GameUI::button(V2(100, y), string_literal("Achievements"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_ACHIEVEMENTS);
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Credits"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                Transitions::do_shuteye_in(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );
                

                Transitions::register_on_finish(
                    [&](void*) mutable {
                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_CREDITS);

                        Transitions::do_color_transition_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );
                    }
                );
            }
            y += 30;
            if (GameUI::button(V2(100, y), string_literal("Exit To Windows"), color32f32(1, 1, 1, 1), 2, !Transitions::fading()) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_CONFIRM_EXIT_TO_WINDOWS);
            }
            // GameUI::ninepatch(&resources->ui_texture_atlas, resources->ui_chunky, V2(100, 100), 3, 3, color32f32(0,0,1,1), 3);
        }
        GameUI::end_frame();
    }

    Transitions::update_and_render(ui_render_commands, dt);
    game_render_commands->should_clear_buffer = true;
    game_render_commands->clear_buffer_color = color32u8(32/2, 45/2, 80/2, 255);
    camera_update(&state->titlescreen_data.main_camera, dt);
}

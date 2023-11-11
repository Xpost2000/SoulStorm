// NOTE: meant to be included inside of game.cpp
// title screen code

/*
  The title screen is only for when you first open
  the game.

  This is the only time you can see it, all other interactions
  are from the ingame/main menu screens.

  Although I do want to make this more elaborate, for now I'll just
  use the pause menu code as place holder.
*/
void Game::update_and_render_game_title_screen(Graphics_Driver* driver, f32 dt) {
    auto ui_render_commands = render_commands(&Global_Engine()->scratch_arena, 8192, camera(V2(0, 0), 1));
    {
        auto resolution = driver->resolution();
        ui_render_commands.screen_width =  resolution.x;
        ui_render_commands.screen_height = resolution.y;
    }

    auto commands = &ui_render_commands;
    if (state->ui_state != UI_STATE_INACTIVE) {
        handle_ui_update_and_render(commands, dt);
    } else {
        render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32u8(0, 0, 0, 128), BLEND_MODE_ALPHA);

        GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

        GameUI::set_ui_id((char*)"ui_options_menu");
        GameUI::begin_frame(commands);
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
                        switch_ui(UI_STATE_INACTIVE);
                        switch_screen(GAME_SCREEN_MAIN_MENU);

                        Transitions::do_shuteye_out(
                            color32f32(0, 0, 0, 1),
                            0.15f,
                            0.3f
                        );

                        // no save file? Going to start
                        if (!load_game()) {
                            // first time, we'll load the full cutscene...
                            auto state = &this->state->mainmenu_data;
                            state->start_introduction_cutscene(this->state, false);
                        } else {
                            auto state = &this->state->mainmenu_data;
                            state->start_introduction_cutscene(this->state, true);
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
        }
        GameUI::end_frame();
    }

    Transitions::update_and_render(&ui_render_commands, dt);
    driver->consume_render_commands(commands);
}

// I could do something interesting like having a few
// mini games or something for a sequence.

// but that's for later.

/*
  For now, I'll just let it be a text page equivalent to a txt file,

  maybe dancing sprites and concept art when I make that, because for some reason
  I'm too stingy to shill out for art.

  Although I like drawing stuff.
*/

local void open_web_browser(const char* url) {
    // yes I know this is unsafe.
#ifdef _WIN32
    system(format_temp("explorer.exe %s", url));
#else
    unimplemented("no url command");
#endif
}

void Game::update_and_render_game_credits(Graphics_Driver* driver, f32 dt) {
    auto ui_render_commands = render_commands(&Global_Engine()->scratch_arena, 8192, camera(V2(0, 0), 1));
    {
        auto resolution = driver->resolution();
        ui_render_commands.screen_width =  resolution.x;
        ui_render_commands.screen_height = resolution.y;
    }

    GameUI::begin_frame(&ui_render_commands);
    {
        f32 y = 30;
        {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("SOULSTORM CREDITS"), color32f32(1, 1, 1, 1), 2);
        }
        y += 45/2;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("EXTERNAL"), color32f32(1, 1, 1, 1), 2);
        }
        y += 45/2;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("SOURCE CODE IS PUBLIC DOMAIN"), color32f32(1, 1, 1, 1), 2);
        }
        y += 45/2;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            if (GameUI::button(V2(100, y), string_literal("gnsh fonts - https://opengameart.org/content/bitmap-font-0"), color32f32(1, 1, 1, 1), 1) == WIDGET_ACTION_ACTIVATE) {
                open_web_browser("https://opengameart.org/content/bitmap-font-0");
            }
            y += 30/2;
            if (GameUI::button(V2(100, y), string_literal("stb_image, stb_image_write - http://nothings.org/"), color32f32(1, 1, 1, 1), 1) == WIDGET_ACTION_ACTIVATE) {
                open_web_browser("http://nothings.org/");
            }
            y += 30/2;
            if (GameUI::button(V2(100, y), string_literal("SDL2, SDL2_mixer - https://www.libsdl.org/"), color32f32(1, 1, 1, 1), 1) == WIDGET_ACTION_ACTIVATE) {
                open_web_browser("https://www.libsdl.org/");
            }
            y += 30/2;
            if (GameUI::button(V2(100, y), string_literal("lua - https://www.lua.org/"), color32f32(1, 1, 1, 1), 1) == WIDGET_ACTION_ACTIVATE) {
                open_web_browser("https://www.lua.org/");
            }
            y += 30/2;
        }
        {
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(50, y), string_literal("BY"), color32f32(1, 1, 1, 1), 2);
        }
        y += 45/2;
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            if (GameUI::button(V2(100, y), string_literal("(engine & gameplay programmer, design) jerry zhu / xpost2000"), color32f32(1, 1, 1, 1), 1) == WIDGET_ACTION_ACTIVATE) {
                open_web_browser("https://xpost2000.itch.io/");
            }
            y += 30/2;
        }
        GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
        {
            GameUI::label(V2(100, y), string_literal("thank you for playing!"), color32f32(1, 1, 1, 1), 1);
            y += 30/2;
        }
    }

    if (GameUI::button(V2(50, driver->resolution().y - 35), string_literal("Back"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
        Transitions::do_color_transition_in(
            color32f32(0, 0, 0, 1),
            0.15f,
            0.3f
        );
                

        Transitions::register_on_finish(
            [&](void*) mutable {
                switch_ui(state->last_ui_state);
                switch_screen(state->last_screen_mode);

                Transitions::do_shuteye_out(
                    color32f32(0, 0, 0, 1),
                    0.15f,
                    0.3f
                );
            }
        );
    }
    GameUI::end_frame();
    GameUI::update(dt);

    Transitions::update_and_render(&ui_render_commands, dt);
    driver->clear_color_buffer(color32u8(0, 0, 0, 255));
    driver->consume_render_commands(&ui_render_commands);
}
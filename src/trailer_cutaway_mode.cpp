#define COMPILE_IN_TRAILER_CLIPPING_CODE 1
#if COMPILE_IN_TRAILER_CLIPPING_CODE == 0
void Game::trailer_cutaway_data_initialize(Graphics_Driver* driver) { }
GAME_SCREEN(update_and_render_game_trailer_cutaway_screen) { }
#else
void Game::trailer_cutaway_data_initialize(Graphics_Driver* driver) {
    auto state = &this->state->trailer_cutaway_data;
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

        state->puppet.prng = &prng;
        state->puppet.position = V2(0, 50); // push down just a little
    }
}

GAME_SCREEN(update_and_render_game_trailer_cutaway_screen) {
    auto commands = ui_render_commands;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);
    game_render_commands->camera = state->trailer_cutaway_data.main_camera;
    state->trailer_cutaway_data.main_camera.zoom = 1.0f;
    state->trailer_cutaway_data.main_camera.xy = V2(0,0);

    auto trailer_data = &this->state->trailer_cutaway_data;

    bool should_draw_and_update_stars = true;
    bool should_draw_and_update_puppet = true;

    switch (trailer_data->anim_phase) {
        case TRAILER_CUTAWAY_ANIMATION_PHASE_MAKING_CHOICES: {
            should_draw_and_update_stars = false;
            should_draw_and_update_puppet = false;
            GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
            GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::set_ui_id((char*)"ui_trailer_maker_hoices");
            GameUI::begin_frame(commands, &resources->graphics_assets);
            GameUI::set_wobbly_contribution(1.0f);
            {
                f32 y = 25;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
                GameUI::label(V2(50, y), string_literal("TRAILER CHOICE MAKER"), color32f32(1, 1, 1, 1), 4);
                y += 45;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
                GameUI::label(V2(50, y), string_literal("todo"), color32f32(1, 1, 1, 1), 2);
                y += 45;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
                if (GameUI::button(V2(100, y), string_literal("todo"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
                }
                y += 45;
                if (GameUI::button(V2(100, y), string_literal("RETURN TO MENU"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
                    switch_screen(GAME_SCREEN_TITLE_SCREEN);
                }
            }
            GameUI::end_frame();
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_IDLE_BLINK: {
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_LOOK_UP: {
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_PAN_UPWARDS_TO_SHOW_LOGO: {
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_IN_OTHER_INFO: {
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_TO_BLACK: {
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_RELEASE_DATE_FADE_IN: {
            should_draw_and_update_stars = false;
            should_draw_and_update_puppet = false;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_ALL_AWAY: {
            should_draw_and_update_stars = false;
            should_draw_and_update_puppet = false;
        } break;
        default: {} break;
    }

    // NOTE(jerry):
    // Animate / Draw Stars, copied from the title screen.
    // *should* be refactored, but this happens exactly twice so
    // it doesn't really matter.

    // UpdateAndDrawStars()
    if (should_draw_and_update_stars) {
        for (int i = 0; i < array_count(state->trailer_cutaway_data.star_positions); ++i)
        {
            auto r = rectangle_f32(state->trailer_cutaway_data.star_positions[i].x, state->trailer_cutaway_data.star_positions[i].y, 1, 1);

            render_commands_push_quad_ext(
                game_render_commands,
                r,
                color32u8(230, 230, 255, 255),
                V2(0, 0), 0,
                BLEND_MODE_ALPHA);
        }

        for (int i = 0; i < array_count(state->trailer_cutaway_data.sparkling_stars); ++i)
        {
            state->trailer_cutaway_data.sparkling_stars[i].update(dt);
            state->trailer_cutaway_data.sparkling_stars[i].draw(game_render_commands, resources);
        }
    }

    if (should_draw_and_update_puppet) {
        state->trailer_cutaway_data.puppet.update(dt);
        state->trailer_cutaway_data.puppet.draw(dt, game_render_commands, resources);
    }

    Transitions::update_and_render(ui_render_commands, dt);
    game_render_commands->should_clear_buffer = true;
    game_render_commands->clear_buffer_color = color32u8(32/2, 45/2, 80/2, 255);
    camera_update(&state->trailer_cutaway_data.main_camera, dt);
}
#endif
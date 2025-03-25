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
                                   random_ranged_float(&prng, -960, 960));
        }

        auto& sparkling_stars = state->sparkling_stars;
        for (int i = 0; i < array_count(state->sparkling_stars); ++i) {
            auto& star = sparkling_stars[i];
            star.visibility_delay_timer = star.max_visibility_delay_timer = random_ranged_float(&prng, 3.25f, 7.00f);
            star.anim_timer = 0.0f;
            star.frame_index = 0;
            star.position = V2(random_ranged_float(&prng, -800, 800), random_ranged_float(&prng, -960, 960));
            star.scale = random_ranged_float(&prng, 1.0f, 1.75f);
        }

        state->puppet.prng = &prng;
        state->puppet.position = V2(0, 50); // push down just a little
    }
}

static bool gui_bitwise_checkbox(V2 where, string label, color32f32 color, f32 scale, int bitpos, u32* flag)
{
    u32 mask = (1 << bitpos);
    bool v = (((*flag) & mask) >> bitpos);
    string on_or_off = (v) ? string_literal(": on") : string_literal(": off");
    string final_label = string_concatenate(&Global_Engine()->scratch_arena, label, on_or_off);

    s32 button = GameUI::button(where, final_label, color, scale);

    if (button == WIDGET_ACTION_ACTIVATE) {
        *flag ^= mask;  
    }

    return button;
}

GAME_SCREEN(update_and_render_game_trailer_cutaway_screen) {
    auto commands = ui_render_commands;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);
    game_render_commands->camera = state->trailer_cutaway_data.main_camera;

    V2 pan_up_offset = V2(0, -550);

    state->trailer_cutaway_data.main_camera.zoom = 1.0f;
    state->trailer_cutaway_data.main_camera.xy = V2(0,0);

    auto trailer_data = &this->state->trailer_cutaway_data;

    bool should_draw_and_update_stars = true;
    bool should_draw_and_update_puppet = true;
    bool should_draw_and_update_logoset = false;

    f32 ui_x_title = resolution.x / 2 - 150;
    f32 ui_y_title = resolution.y / 2 - 50;

    f32 ui_y_title_start = ui_y_title;
    f32 ui_y_title_final = 100;
    f32 title_alpha = 0.0f;

    f32 logos_alpha = 0.0f;
    f32 black_screen_alpha = 0.0f;
    f32 release_date_alpha = 0.0f;

    switch (trailer_data->anim_phase) {
        case TRAILER_CUTAWAY_ANIMATION_PHASE_MAKING_CHOICES: {
            should_draw_and_update_stars = false;
            should_draw_and_update_puppet = false;
            should_draw_and_update_logoset = false;
            GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
            GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::set_ui_id((char*)"ui_trailer_maker_hoices");
            GameUI::begin_frame(commands, &resources->graphics_assets);
            GameUI::set_wobbly_contribution(1.0f);
            {
                f32 y = 25;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
                GameUI::label(V2(50, y), string_literal("PLATFORM FLAGS"), color32f32(1, 1, 1, 1), 4);
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
                y += 45;
                for (unsigned index = 0; index < TRAILER_CUTAWAY_PLATFORM_LOGO_COUNT; ++index) {
                    gui_bitwise_checkbox(
                        V2(100, y), 
                        g_platform_logo_strings[index],
                         color32f32(1,1,1,1), 2, index, 
                         &trailer_data->platform_mask_flags
                    );
                    y += 45/2;
                }
                y += 45/2;
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
                GameUI::label(V2(50, y), string_literal("STOREFRONT FLAGS"), color32f32(1, 1, 1, 1), 4);
                GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));
                y += 45;
                for (unsigned index = 0; index < TRAILER_CUTAWAY_STOREFRONT_LOGO_COUNT; ++index) {
                    gui_bitwise_checkbox(
                        V2(100, y), 
                        g_storefront_strings[index],
                         color32f32(1,1,1,1), 2, index, 
                         &trailer_data->storefront_mask_flags
                    );
                    y += 45/2;
                }
                y += 45;
                if (GameUI::button(V2(100, y), string_literal("BEGSEQ"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
                    Audio::stop_music();
                    trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_IDLE_BLINK;

                    // reinit puppet
                    trailer_data->puppet = TitleScreen_MainCharacter_Puppet();
                    trailer_data->puppet.prng = &trailer_data->prng;
                    trailer_data->puppet.position = V2(0, 50); // push down just a little
                }
                y += 45/2;
                if (GameUI::button(V2(100, y), string_literal("RETURN TO MENU"), color32f32(1, 1, 1, 1), 2) == WIDGET_ACTION_ACTIVATE) {
                    switch_screen(GAME_SCREEN_TITLE_SCREEN);
                }
            }
            GameUI::end_frame();
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_IDLE_BLINK: {
            const f32 MAX_T = 1.5f;

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_LOOK_UP;
                trailer_data->anim_t = 0.0f;
            }
            trailer_data->anim_t += dt;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_LOOK_UP: {
            const f32 MAX_T = 0.65f;
            V2 eye_look_target = V2(0, -100) - trailer_data->puppet.position;
            trailer_data->puppet.set_new_eye_target(eye_look_target.normalized(), 1.5f);

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_PAN_UPWARDS_TO_SHOW_LOGO;
                trailer_data->anim_t = 0.0f;
            }
            trailer_data->anim_t += dt;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_PAN_UPWARDS_TO_SHOW_LOGO: {
            const f32 MAX_T  = 1.55f;
            f32 effective_t  = saturate<f32>((trailer_data->anim_t) / (MAX_T-0.3));
            f32 effective_t2 = saturate<f32>((trailer_data->anim_t-0.5) / (MAX_T - 0.5));
            state->trailer_cutaway_data.main_camera.xy.x = quadratic_ease_in_out_f32(0, pan_up_offset.x, effective_t);
            state->trailer_cutaway_data.main_camera.xy.y = quadratic_ease_in_out_f32(0, pan_up_offset.y, effective_t);

            ui_y_title = quadratic_ease_in_out_f32(ui_y_title_start, ui_y_title_final, effective_t2);
            title_alpha = lerp_f32(0.0f, 1.0f, effective_t2);

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_IN_OTHER_INFO;
                trailer_data->anim_t = 0.0f;
            }
            trailer_data->anim_t += dt;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_IN_OTHER_INFO: {
            const f32 MAX_T = 2.0f;

            should_draw_and_update_logoset = true;
            title_alpha = 1.0f;
            ui_y_title = ui_y_title_final;
            logos_alpha = trailer_data->anim_t;

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_TO_BLACK;
                trailer_data->anim_t = 0.0f;
            }
            trailer_data->anim_t += dt;
            state->trailer_cutaway_data.main_camera.xy = V2(0, 0) + pan_up_offset;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_TO_BLACK: {
            const f32 MAX_T = 1.5f;
            const f32 MAX_T2 = 1.0f;
            f32 effective_t = trailer_data->anim_t / MAX_T2;
            should_draw_and_update_logoset = true;
            title_alpha = 1.0f;
            ui_y_title = ui_y_title_final;
            logos_alpha = 2.5;
            black_screen_alpha = saturate(effective_t);

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_RELEASE_DATE_FADE_IN;
                trailer_data->anim_t = 0.0f;
            }
            trailer_data->anim_t += dt;
            state->trailer_cutaway_data.main_camera.xy = V2(0, 0) + pan_up_offset;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_RELEASE_DATE_FADE_IN: {
            logos_alpha = 2.5;
            should_draw_and_update_logoset = true;
            title_alpha = 1.0f;
            ui_y_title = ui_y_title_final;
            black_screen_alpha = 1.0f;

            const f32 MAX_T = 2.00f * 2;
            // FadeOutPlatformAlphas
            if (trailer_data->anim_t < MAX_T/2) {
                f32 effective_t = saturate(trailer_data->anim_t / (MAX_T/2));
                logos_alpha = lerp_f32(2.5f, 0.0f, effective_t);
            } else {
                logos_alpha = 0.0f;
                f32 effective_t = saturate((trailer_data->anim_t-(MAX_T/2))/ (MAX_T / 2));
                release_date_alpha = lerp_f32(0.0f, 1.0f, effective_t);
            }

            if (trailer_data->anim_t >= (MAX_T+1.0f)) {
                trailer_data->anim_t = 0.0f;
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_ALL_AWAY;
            }
            trailer_data->anim_t += dt;

            state->trailer_cutaway_data.main_camera.xy = V2(0, 0) + pan_up_offset;
            should_draw_and_update_stars = false;
            should_draw_and_update_puppet = false;
        } break;
        case TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_ALL_AWAY: {
            const f32 MAX_T = 2.0f;
            state->trailer_cutaway_data.main_camera.xy = V2(0, 0) + pan_up_offset;
            should_draw_and_update_logoset = true;
            ui_y_title = ui_y_title_final;
            black_screen_alpha = 1.0f;
            logos_alpha = 0.0f;

            f32 effective_t = saturate(trailer_data->anim_t / (MAX_T-1.0f));
            release_date_alpha = lerp_f32(1.0f, 0.0f, effective_t);
            title_alpha = lerp_f32(1.0f, 0.0f, effective_t);

            if (trailer_data->anim_t >= MAX_T) {
                trailer_data->anim_t = 0.0f;
                trailer_data->anim_phase = TRAILER_CUTAWAY_ANIMATION_PHASE_MAKING_CHOICES;
            }
            trailer_data->anim_t += dt;

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

    // BlackScreen()
    {
        render_commands_push_quad(
            commands,
            rectangle_f32(-100, -100, 1024, 1024),
            color32u8(0, 0, 0, 255 * black_screen_alpha),
            BLEND_MODE_ALPHA
        );
    }

    game_ui_draw_title_logo(commands, V2(ui_x_title, ui_y_title), 1.25, title_alpha);
    // DrawReleaseDate
    {
        auto font = resources->get_font(MENU_FONT_COLOR_GOLD);
        string text = string_literal("RELEASING Q2 2025\nWISHLIST ON STEAM!");
        f32 text_width = font_cache_text_width(font, text, 4.0f);
        render_commands_push_text(
            commands,
            font,
            4.0f,
            V2(resolution.x/2 - text_width/2, resolution.y/2),
            text,
            color32f32(1, 1, 1, release_date_alpha),
            BLEND_MODE_ALPHA
        );
    }
    // DrawPlatformLogos
    {
        f32 dx = resolution.x / 2 - 100;
        f32 y = resolution.y / 2;
        {
            f32 effective_alpha = logos_alpha*2;
            auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
            render_commands_push_text(
                commands,
                font,
                2.0f,
                V2(dx, y-24),
                string_literal("PLATFORMS"),
                color32f32(1, 1, 1, saturate(effective_alpha)),
                BLEND_MODE_ALPHA
            );
        }

        for (s32 index = 0; index < array_count(resources->trailer_platform_logos); ++index) {
            if (trailer_data->platform_mask_flags & (1 << index)) {
                image_id img = resources->trailer_platform_logos[index];
                f32 effective_alpha = logos_alpha;
                render_commands_push_image(
                    commands, graphics_assets_get_image_by_id(&resources->graphics_assets, img),
                    rectangle_f32(dx, y, 32, 32),
                    RECTANGLE_F32_NULL, color32f32(1, 1, 1, saturate<f32>(effective_alpha - index * 0.30f)),
                    NO_FLAGS, BLEND_MODE_ALPHA
                );

                dx += 38;
            }
        }
    }
    // DrawStoreFrontLogos
    {
        f32 dx = resolution.x / 2 - 100;
        f32 y = resolution.y / 2 + 64;
        {
            f32 effective_alpha = logos_alpha * 2;
            auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
            render_commands_push_text(
                commands,
                font,
                2.0f,
                V2(dx, y - 24),
                string_literal("STOREFRONTS"),
                color32f32(1, 1, 1, saturate(effective_alpha)),
                BLEND_MODE_ALPHA
            );
        }
        for (s32 index = 0; index < array_count(resources->trailer_storefront_logos); ++index) {
            if (trailer_data->storefront_mask_flags & (1 << index)) {
                image_id img = resources->trailer_storefront_logos[index];
                f32 effective_alpha = logos_alpha;
                render_commands_push_image(
                    commands, graphics_assets_get_image_by_id(&resources->graphics_assets, img),
                    rectangle_f32(dx, y, 32, 32),
                    RECTANGLE_F32_NULL, color32f32(1, 1, 1, saturate<f32>(effective_alpha - index * 0.5f)),
                    NO_FLAGS, BLEND_MODE_ALPHA
                );

                dx += 38;
            }
        }
    }

    Transitions::update_and_render(ui_render_commands, dt);
    game_render_commands->should_clear_buffer = true;
    game_render_commands->clear_buffer_color = color32u8(32/2, 45/2, 80/2, 255);
    camera_update(&state->trailer_cutaway_data.main_camera, dt);
}
#endif
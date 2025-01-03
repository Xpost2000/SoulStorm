#include "discord_rich_presence_integration.h"
/*
  NOTE: Images should be drawn as 16:9 and maybe scroll in 4:3 mode,
  I don't want to draw multiple image variations or crop them or whatever, so I think it'd just be
  more convenient to have one image size...

  draw the images at some multiple of (854, 480) for 16:9 resolution fit.

  TODO: add controller skips
*/

#define SLIDE_RESOLUTION_W (854)
#define SLIDE_RESOLUTION_H (480)

void Game::opening_data_initialize(Graphics_Driver* driver) {
    auto& state = this->state->opening_data;
    state.slide_count = 0;
    state.slide_index = 0;
    state.fade_timer  = 0.0f;
    state.skipper_visibility_t = 0.0f;
    state.skipper_progress_t = 0.0f;
    state.phase = OPENING_MODE_PHASE_LOGO;
    /*
     * I'm not an excellent writer, but I also don't need a very complicated story.
     */
    _debugprintf("Initialize opening scene");
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/1.png"));
        slide.slide_caption = string_literal("Once upon a time, in the emptiness of space");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/2.png"));
        slide.slide_caption = string_literal("Was a small, little boy.");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/3.png"));
        slide.slide_caption = string_literal("Alone with nothing but his thoughts.");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/4.png"));
        slide.slide_caption = string_literal("He loved staring at the far-off stars");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/5.png"));
        slide.slide_caption = string_literal("Eventually he would find friends in the void");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/6.png"));
        slide.slide_caption = string_literal("He loved playing with his extrastellar friends");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/7.png"));
        slide.slide_caption = string_literal("However, one day mysterious portals appeared");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/8.png"));
        slide.slide_caption = string_literal("His animal friends, curious and naive");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/9.png"));
        slide.slide_caption = string_literal("Got absorbed into the portals");
    }
    {
        auto& slide = state.slides[state.slide_count++];
        slide.slide_image = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/10.png"));
        slide.slide_caption = string_literal("And so the boy set on an adventure to rescue them.");
    }
    // Extra non-text slides are okay.
}

void OpeningMode_Data::reset_all_slides(void) {
  for (unsigned index = 0; index < slide_count; ++index) {
    auto& slide              = slides[index];
    slide.display_phase = 0;
    slide.shown_characters = 0;
    slide.timer = 0;
  }
}

void OpeningMode_Data::update_slide(OpeningMode_SlideData* slide, f32 dt) {
    switch (slide->display_phase) {
        case OPENING_MODE_SLIDE_DATA_PHASE_DISPLAY_DELAY: {
            if (slide->timer >= OPENING_MODE_DISPLAY_DELAY_TIME) {
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_TYPE_TEXT;
                slide->timer = 0.0f;
            } else {
                slide->timer += dt;
            }
        } break;
        case OPENING_MODE_SLIDE_DATA_PHASE_TYPE_TEXT: {
            if (slide->shown_characters >= slide->slide_caption.length) {
                slide->timer = 0.0f;
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_READ_DELAY;
            } else {
                if (slide->timer >= OPENING_MODE_SLIDE_TEXT_TYPE_SPEED) {
                    slide->timer = 0.0f;
                    slide->shown_characters += 1;
                } else {
                    slide->timer += dt;
                }
            }
        } break;
        case OPENING_MODE_SLIDE_DATA_PHASE_READ_DELAY: {
            if (slide->timer >= OPENING_MODE_DISPLAY_READ_DELAY_TIME) {
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_FADE_TEXT;
                slide->timer = 0.0f;
            } else {
                slide->timer += dt;
            }
        } break;
        case OPENING_MODE_SLIDE_DATA_PHASE_FADE_TEXT: {
            if (slide->timer >= OPENING_MODE_DISPLAY_FADE_TEXT_TIME) {
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_FADE_DELAY;
                slide->timer = 0.0f;
            } else {
                slide->timer += dt;
            }
        } break;
        case OPENING_MODE_SLIDE_DATA_PHASE_FADE_DELAY: {
            if (slide->timer >= OPENING_MODE_DISPLAY_FADE_DELAY_TIME) {
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_CROSSFADE;
                slide->timer = 0.0f;
            } else {
                slide->timer += dt;
            }
        } break;
        case OPENING_MODE_SLIDE_DATA_PHASE_CROSSFADE: {
            if (slide->timer >= OPENING_MODE_DISPLAY_FADE_CROSSFADE_TIME) {
                slide->display_phase = OPENING_MODE_SLIDE_DATA_PHASE_CROSSFADE;

                if (slide_index+1 >= slide_count) {
                    fade_timer = 0.0f;
                    phase = OPENING_MODE_PHASE_FADE_OUT;
                } else {
                    slide->timer = 0.0f;
                    slide_index += 1;
                }
            } else {
                slide->timer += dt;
            }
        } break;
    }
}

void OpeningMode_Data::update_and_render_skipper_box(f32 dt, Game_Resources* resources, struct render_commands* ui_render_commands) {
    if (phase < OPENING_MODE_PHASE_FADE_OUT && phase != OPENING_MODE_PHASE_LOGO) {
        f32 skip_box_x = 35;
        f32 skip_box_y = 20;
        f32 skip_box_w = 90;
        f32 skip_box_h = 15;

        f32 alpha    = clamp<f32>(skipper_visibility_t / OPENING_MODE_DATA_SKIPPER_VISIBILITY_MAX_T, 0.0f, 1.0f);
        f32 progress = clamp<f32>(skipper_progress_t / OPENING_MODE_DATA_SKIPPER_PROGRESS_MAX_T, 0.0f, 1.0f);

        render_commands_push_quad(
            ui_render_commands,
            rectangle_f32(skip_box_x, skip_box_y, skip_box_w, skip_box_h),
            color32u8(0,0,0, alpha * 255),
            BLEND_MODE_ALPHA
        );

        f32 inner_diameter = 4;
        render_commands_push_quad(
            ui_render_commands,
            rectangle_f32(skip_box_x+inner_diameter/2, skip_box_y+inner_diameter/2, (skip_box_w - inner_diameter) * progress, skip_box_h - inner_diameter),
            color32u8(255, 255, 255, alpha * 255),
            BLEND_MODE_ALPHA
        );

        auto font       = resources->get_font(MENU_FONT_COLOR_ORANGE);
        f32  font_scale = 1;
        render_commands_push_text(
            ui_render_commands,
            font,
            font_scale,
            V2(skip_box_x, 10),
            string_literal("Any input to skip"),
            color32f32(1, 1, 1, alpha),
            BLEND_MODE_ALPHA
        );

        {
            game_controller* controller = Input::get_game_controller(0); 

            bool controller_input = controller && Input::controller_any_button_down(controller);
            bool keyboard_input   = Input::any_key_down();

            if (controller_input || keyboard_input) {
                if (skipper_visibility_t < (OPENING_MODE_DATA_SKIPPER_VISIBILITY_MAX_T+0.15)) {
                    skipper_visibility_t += dt;
                }

                if (skipper_visibility_t > 0.0f) {
                    if (skipper_progress_t < OPENING_MODE_DATA_SKIPPER_PROGRESS_MAX_T + 0.10) {
                        skipper_progress_t += dt;
                    }
                }
            } else {
                if (skipper_visibility_t > 0.0f) {
                    skipper_visibility_t -= dt;
                }

                if (skipper_progress_t > 0.0f) {
                    skipper_progress_t -= dt;
                }
            }

            if (skipper_progress_t >= (OPENING_MODE_DATA_SKIPPER_PROGRESS_MAX_T + 0.10)) {
                fade_timer = 0;
                phase = OPENING_MODE_PHASE_FADE_OUT;
                return;
            }
        }
    }
}

GAME_SCREEN(update_and_render_game_opening) {
    auto& state = this->state->opening_data;

    OpeningMode_SlideData* first_slide = &state.slides[state.slide_index];
    OpeningMode_SlideData* next_slide  = &state.slides[state.slide_index+1];
    if (state.slide_index+1 >= state.slide_count) {
        next_slide = nullptr;
    }

    {
        struct rectangle_f32 slide_rectangle =
            rectangle_f32(
                ui_render_commands->screen_width/2 - SLIDE_RESOLUTION_W/2,
                ui_render_commands->screen_height/2 - SLIDE_RESOLUTION_H/2,
                SLIDE_RESOLUTION_W,
                SLIDE_RESOLUTION_H
            ); 
        // draw the next slide (on below, for the crossfade)
        if (next_slide) {
            render_commands_push_image(
                ui_render_commands,
                graphics_assets_get_image_by_id(&resources->graphics_assets, next_slide->slide_image),
                slide_rectangle,
                RECTANGLE_F32_NULL,
                color32f32(1, 1, 1, 1),
                0,
                BLEND_MODE_ALPHA
            );
        }

        // render the current slide (on top)
        {
            f32 alpha = 1.0f;
            f32 text_alpha = 1.0f;
            f32 box_alpha  = 1.0f;
            s32 shown_characters = first_slide->shown_characters;

            if (state.phase == OPENING_MODE_PHASE_FADE_IN) {
                box_alpha = clamp<f32>((state.fade_timer-0.2) / OPENING_MODE_FADE_TIMER_MAX, 0.0f, 1.0f);
            }

            if (first_slide->display_phase == OPENING_MODE_SLIDE_DATA_PHASE_CROSSFADE) {
                f32 target_alpha = 1 - clamp<f32>(first_slide->timer/OPENING_MODE_DISPLAY_FADE_CROSSFADE_TIME, 0.0f, 1.0f);
                if (next_slide) {
                    alpha = target_alpha;
                } else {
                    box_alpha = target_alpha;
                }
            }

            if (first_slide->display_phase >= OPENING_MODE_SLIDE_DATA_PHASE_FADE_TEXT) {
                if (first_slide->display_phase == OPENING_MODE_SLIDE_DATA_PHASE_FADE_TEXT) {
                    text_alpha = 1 - clamp<f32>(first_slide->timer/OPENING_MODE_DISPLAY_FADE_TEXT_TIME, 0.0f, 1.0f);
                } else {
                    text_alpha = 0.0f;
                }
            } else {
                text_alpha = 1.0f;
            }

            render_commands_push_image(
                ui_render_commands,
                graphics_assets_get_image_by_id(&resources->graphics_assets, first_slide->slide_image),
                slide_rectangle,
                RECTANGLE_F32_NULL,
                color32f32(1, 1, 1, alpha),
                0,
                BLEND_MODE_ALPHA
            );

            {
                auto   font           = resources->get_font(MENU_FONT_COLOR_WHITE);
                f32 font_scale = 2;

                if (!is_4_by_3_resolution(ui_render_commands->screen_width, ui_render_commands->screen_height)) {
                    font_scale = 3.0f;
                }

                float  text_height    = font_cache_text_height(font) * font_scale;
                string displayed_text = string_slice(first_slide->slide_caption, 0, first_slide->shown_characters);
                f32    text_ascent   = text_height*3.5;
                V2     text_xy        = V2(30, ui_render_commands->screen_height - text_ascent);

                {
                    // background for text to make it easier to read.
                    render_commands_push_quad(
                        ui_render_commands,
                        rectangle_f32(0, ui_render_commands->screen_height - text_ascent-10, ui_render_commands->screen_width, text_height+10),
                        color32u8(0,0,0, 96 * box_alpha),
                        BLEND_MODE_ALPHA
                    );
                }
                {
                    render_commands_push_text(
                        ui_render_commands,
                        font,
                        font_scale,
                        text_xy,
                        displayed_text,
                        color32f32(1, 1, 1, text_alpha),
                        BLEND_MODE_ALPHA
                    );
                }
            }
        }
    }

    // NOTE: this is after, so I can draw the transition fades myself.
    switch (state.phase) {
        case OPENING_MODE_PHASE_LOGO: {
            f32    font_scale = 8.0f;
            auto   font       = resources->get_font(MENU_FONT_COLOR_STEEL);
            auto&  logo_data  = state.logo_presentation;
            V2     resolution = V2(ui_render_commands->screen_width, ui_render_commands->screen_height);
            float fade_alpha  = 0;
            string fulltext   = string_literal("By Xpost2000");

            if (logo_data.untyping_text == false) {
                if (logo_data.characters_visible < fulltext.length) {
                    if (logo_data.type_timer <= 0.0f) {
                        logo_data.characters_visible += 1;
                        logo_data.type_timer = OPENING_MODE_LOGO_TYPE_SPEED;

                        Audio::play(this->state->resources->opening_beep_type);
                    } else {
                        logo_data.type_timer -= dt;
                    }
                } else {
                    if (logo_data.delay_timer > 0) {
                        logo_data.delay_timer -= dt;
                    } else {
                        logo_data.delay_timer = 1.5f;
                        logo_data.untyping_text = true;
                    }
                }
            } else {
                if (logo_data.characters_visible > 0) {
                    if (logo_data.type_timer <= 0.0f) {
                        logo_data.characters_visible -= 1;
                        logo_data.type_timer = OPENING_MODE_LOGO_TYPE_SPEED;
                    } else {
                        logo_data.type_timer -= dt;
                    }
                } else {
                    fade_alpha = clamp<f32>(1 - (logo_data.delay_timer/1.40f), 0, 1);
                    if (logo_data.delay_timer > 0) {
                        logo_data.delay_timer -= dt;
                    } else {
                        state.phase = OPENING_MODE_PHASE_FADE_IN;
                    }
                }
            }

            string presented_portion = string_concatenate(&Global_Engine()->scratch_arena, string_slice(fulltext, 0, logo_data.characters_visible), string_literal("_"));
            float text_width_full = font_cache_text_width(font, string_concatenate(&Global_Engine()->scratch_arena, fulltext, string_literal("_ ")), font_scale);
            float  text_width        = font_cache_text_width(font, presented_portion, font_scale);
            float  text_height       = font_cache_text_height(font) * font_scale;

            {
                render_commands_push_quad(
                    ui_render_commands,
                    rectangle_f32(0, 0, resolution.x, resolution.y),
                    color32u8(0, 0, 0, 255),
                    BLEND_MODE_ALPHA
                );

                {
                    // background for text to make it easier to read.
                    render_commands_push_quad(
                        ui_render_commands,
                        rectangle_f32(resolution.x/2 - (text_width_full/2) - 7.5f, resolution.y/2 - (text_height*1.5f)/2, 15, text_height*1.5f),
                        color32u8(255, 255, 255, 255),
                        BLEND_MODE_ALPHA
                    );
                }

                {
                    render_commands_push_text(
                        ui_render_commands,
                        font,
                        font_scale,
                        V2(resolution.x/2 - text_width/2, resolution.y/2 - text_height/2),
                        presented_portion,
                        color32f32(1, 1, 1, 1),
                        BLEND_MODE_ALPHA
                    );
                }
            }
            {
                render_commands_push_quad(
                    ui_render_commands,
                    rectangle_f32(0, 0, resolution.x, resolution.y),
                    color32u8(0, 0, 0, 255 * fade_alpha),
                    BLEND_MODE_ALPHA
                );
            }
        } break;
        case OPENING_MODE_PHASE_FADE_IN: {
            f32 alpha = clamp<f32>(state.fade_timer / OPENING_MODE_FADE_TIMER_MAX, 0.0f, 1.0f);
            render_commands_push_quad(
                ui_render_commands,
                rectangle_f32(0, 0, ui_render_commands->screen_width, ui_render_commands->screen_height),
                color32u8(0,0,0, (1.0f - alpha) * 255),
                BLEND_MODE_ALPHA
            );

            if (state.fade_timer >= OPENING_MODE_FADE_TIMER_MAX) {
                state.phase = OPENING_MODE_PHASE_SLIDESHOW;
                state.fade_timer = 0.0;
            } else {
                state.fade_timer += dt;
            }
        } break;
        case OPENING_MODE_PHASE_SLIDESHOW: {
            state.update_slide(first_slide, dt);
        } break;
        case OPENING_MODE_PHASE_FADE_OUT: {
            f32 alpha = clamp<f32>(state.fade_timer / OPENING_MODE_FADE_TIMER_ENDING_MAX, 0.0f, 1.0f);
            render_commands_push_quad(
                ui_render_commands,
                rectangle_f32(0, 0, ui_render_commands->screen_width, ui_render_commands->screen_height),
                color32u8(0,0,0, alpha * 255),
                BLEND_MODE_ALPHA
            );

            if (state.fade_timer >= (OPENING_MODE_FADE_TIMER_ENDING_MAX+0.35)) {
                state.phase = OPENING_MODE_PHASE_SLIDESHOW;
                state.fade_timer = 0.0;

                state.reset_all_slides();
                switch_screen(GAME_SCREEN_TITLE_SCREEN);

                Transitions::do_color_transition_out(
                    color32f32(0, 0, 0, 1),
                    0.17f,
                    2.35f
                );
                return;
            } else {
                state.fade_timer += dt;
            }
        } break;
    }


    state.update_and_render_skipper_box(dt, resources, ui_render_commands);
    ui_render_commands->should_clear_buffer = true;
}

void OpeningMode_Data::unload_all_assets(Game_Resources* resources) {
    for (int index = 0; index < slide_count; ++index) {
        auto& slide = slides[index];
        graphics_assets_unload_image(&resources->graphics_assets, slide.slide_image);
    }
}

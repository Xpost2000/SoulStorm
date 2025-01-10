#include "discord_rich_presence_integration.h"

/*
 * Creating a very elaborate ending screen is not exactly in my list of things
 * I'd like to do, so something simple with a FIN screen is good enough.
 *
 * The game sort of invokes a shareware feel anyway, so it's pretty par for the
 * course.
 */

// included in game.cpp
#define SLIDE_RESOLUTION_W (854)
#define SLIDE_RESOLUTION_H (480)

void Game::ending_data_initialize(Graphics_Driver* driver) {
    auto& state        = this->state->ending_data;
    state.timer        = 0.0f;
    state.phase        = ENDING_MODE_PHASE_FADE_IN_FROM_BLACK;
    state.ending_slide = graphics_assets_load_image(&resources->graphics_assets, string_literal("./res/img/opening_slides/5.png"));
}

void EndingMode_Data::unload_all_assets(Game_Resources* resources) {
    graphics_assets_unload_image(&resources->graphics_assets, ending_slide);
}

GAME_SCREEN(update_and_render_game_ending) {
    auto& state = this->state->ending_data;

    struct rectangle_f32 slide_rectangle =
        rectangle_f32(
            ui_render_commands->screen_width/2 - SLIDE_RESOLUTION_W/2,
            ui_render_commands->screen_height/2 - SLIDE_RESOLUTION_H/2,
            SLIDE_RESOLUTION_W,
            SLIDE_RESOLUTION_H
        ); 

    render_commands_push_image(
        ui_render_commands,
        graphics_assets_get_image_by_id(&resources->graphics_assets, state.ending_slide),
        slide_rectangle,
        RECTANGLE_F32_NULL,
        color32f32(1, 1, 1, 1),
        0,
        BLEND_MODE_ALPHA
    );

    // FIN text
    {
        auto font = resources->get_font(MENU_FONT_COLOR_BLUE);
        f32  font_scale = 4;

        float text_height = font_cache_text_height(font) * font_scale;

        // background for text to make it easier to read.
        render_commands_push_quad(
            ui_render_commands,
            rectangle_f32(0, 0, ui_render_commands->screen_width, text_height+10),
            color32u8(0,0,0, 200),
            BLEND_MODE_ALPHA
        );
        render_commands_push_text(
            ui_render_commands,
            font,
            font_scale,
            V2(20, 10),
            string_literal("FIN~~ :)"),
            color32f32(1, 1, 1, 1),
            BLEND_MODE_ALPHA
        );
    }

    switch (state.phase) {
        case ENDING_MODE_PHASE_FADE_IN_FROM_BLACK: {
            f32 effective_t = clamp<f32>(state.timer / ENDING_MODE_FADE_TIME, 0.0f, 1.0f);

            if (state.timer < ENDING_MODE_FADE_TIME) {
                state.timer += dt;
            } else {
                Audio::play_music_transition_into(resources->title_reprisal_music, 1000, 1000, 0, 0);
                state.phase = ENDING_MODE_PHASE_LINGER;
                state.timer = 0;
            }

            render_commands_push_quad(
                ui_render_commands,
                rectangle_f32(0, 0, 9999, 9999),
                color32u8(0, 0, 0, (1.0 - effective_t) * 255),
                BLEND_MODE_ALPHA
            );
        } break;
        case ENDING_MODE_PHASE_LINGER: {
            f32 effective_t = clamp<f32>(state.timer / ENDING_MODE_FADE_TIME, 0.0f, 1.0f);

            if (state.timer < ENDING_MODE_LINGER_TIME) {
                state.timer += dt;
            } else {
                state.phase = ENDING_MODE_PHASE_FADE_OUT_TO_BLACK;
                state.timer = 0;
                Audio::stop_music();
            }
        } break;
        case ENDING_MODE_PHASE_FADE_OUT_TO_BLACK: {
            f32 effective_t = clamp<f32>(state.timer / ENDING_MODE_FADE_TIME, 0.0f, 1.0f);

            if (state.timer < ENDING_MODE_FADE_TIME) {
                state.timer += dt;
            } else {
                switch_screen(GAME_SCREEN_MAIN_MENU);
                return;
            }

            render_commands_push_quad(
                ui_render_commands,
                rectangle_f32(0, 0, 9999, 9999),
                color32u8(0, 0, 0, (effective_t) * 255),
                BLEND_MODE_ALPHA
            );
        } break;
        default: {} break;
    }
}

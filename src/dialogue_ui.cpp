// NOTE:
// included in game.cpp

void Game::update_and_render_dialogue_speaker(struct render_commands* commands, f32 dt, s32 speaker_index) {
    auto& dialogue_state = this->state->dialogue_state;
    auto  font           = resources->get_font(MENU_FONT_COLOR_WHITE);
    {
        auto&      speaker           = dialogue_state.speakers[speaker_index];
        if (!speaker.visible) return;

        auto       speaker_img        = graphics_assets_get_image_by_id(&resources->graphics_assets, speaker.image);
        V2         sprite_image_scale = V2(speaker.image_scale.x, speaker.image_scale.y);
        V2         sprite_image_size  = V2(speaker_img->width * speaker.image_scale.x,
                                          speaker_img->height * speaker.image_scale.y);
        V2         final_position     = V2(30, commands->screen_height - sprite_image_size.y) + speaker.offset_position;
        V2         position           = V2(30, commands->screen_height - sprite_image_size.y) + speaker.offset_position;
        color32f32 modulation         = speaker.modulation;

        {
            f32 effective_t = clamp<f32>(speaker.animation.t / speaker.animation.max_t, 0.0f, 1.0f);
            switch (speaker.animation.type) {
                case DIALOGUE_SPEAKER_ANIMATION_NONE: {} break;
                case DIALOGUE_SPEAKER_ANIMATION_FADE_IN: {
                    modulation.a = lerp_f32(0.0f, 1.0f, effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_FADE_OUT: {
                    modulation.a = lerp_f32(0.0f, 1.0f, 1.0f-effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_SLIDE_IN: {
                    position.x = quadratic_ease_in_f32(-sprite_image_size.x, final_position.x, effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_SLIDE_OUT: {
                    position.x = quadratic_ease_in_f32(-sprite_image_size.x, final_position.x, 1.0f-effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_IN: {
                    position.x = quadratic_ease_in_f32(-sprite_image_size.x, final_position.x, effective_t);
                    modulation.a = lerp_f32(0.0f, 1.0f, effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_OUT: {
                    position.x = quadratic_ease_in_f32(-sprite_image_size.x, final_position.x, 1.0f-effective_t);
                    modulation.a = lerp_f32(0.0f, 1.0f, 1.0f-effective_t);
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_FOCUS_OUT: {
                    const f32 focus_scale = 0.90f;
                    const f32 value       = 0.85f;
                    sprite_image_scale.x = lerp_f32(speaker.image_scale.x, speaker.image_scale.x * focus_scale, effective_t);
                    sprite_image_scale.y = lerp_f32(speaker.image_scale.y, speaker.image_scale.y * focus_scale, effective_t);
                    modulation.r = lerp_f32(1.0f, value, effective_t);
                    modulation.g = lerp_f32(1.0f, value, effective_t);
                    modulation.b = lerp_f32(1.0f, value, effective_t);
                    sprite_image_size = V2(speaker_img->width * sprite_image_scale.x, speaker_img->height * sprite_image_scale.y);
                    position          = V2(30, commands->screen_height - sprite_image_size.y) + speaker.offset_position;
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_FOCUS_IN: {
                    const f32 focus_scale = 0.90f;
                    const f32 value       = 0.85f;
                    sprite_image_scale.x = lerp_f32(speaker.image_scale.x, speaker.image_scale.x * focus_scale, 1.0 - effective_t);
                    sprite_image_scale.y = lerp_f32(speaker.image_scale.y, speaker.image_scale.y * focus_scale, 1.0 - effective_t);
                    modulation.r = lerp_f32(1.0f, value, 1.0 - effective_t);
                    modulation.g = lerp_f32(1.0f, value, 1.0 - effective_t);
                    modulation.b = lerp_f32(1.0f, value, 1.0 - effective_t);
                    sprite_image_size = V2(speaker_img->width * sprite_image_scale.x, speaker_img->height * sprite_image_scale.y);
                    position          = V2(30, commands->screen_height - sprite_image_size.y) + speaker.offset_position;
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_SHAKE: {
                    // not here.
                    _debugprintf("%f, %f\n", speaker.animation.shake_offset.x, speaker.animation.shake_offset.y);
                    position.x += speaker.animation.shake_offset.x;
                    position.y += speaker.animation.shake_offset.y;
                } break;
                case DIALOGUE_SPEAKER_ANIMATION_JUMP: {
                    f32 remapped_effective_t = (effective_t/2.0f);
                    if (effective_t <= 0.5) {
                        position.y = lerp_f32(final_position.y, final_position.y - speaker.animation.param0, remapped_effective_t/0.25f);
                    } else {
                        remapped_effective_t -= 0.25f;
                        position.y = lerp_f32(final_position.y, final_position.y - speaker.animation.param0, 1.0f - remapped_effective_t/0.25f);
                    }
                } break;
            }
        }

        // NOTE: maybe put as fixed timestep? 
        if (speaker.animation.type != DIALOGUE_SPEAKER_ANIMATION_NONE) {
            if (speaker.animation.t < speaker.animation.max_t) {
                speaker.animation.t += dt;
            } else {
                speaker.animation.t = speaker.animation.max_t;

                if (speaker.animation.type == DIALOGUE_SPEAKER_ANIMATION_JUMP) {
                    speaker.animation.times -= 1;
                    if (speaker.animation.times > 1) {
                        speaker.animation.t = 0.0f;
                    } else {
                        speaker.animation.type = DIALOGUE_SPEAKER_ANIMATION_NONE;
                    }
                } else if (speaker.animation.type == DIALOGUE_SPEAKER_ANIMATION_SHAKE) {
                    speaker.animation.times -= 1;
                    if (speaker.animation.times > 1) {
                        speaker.animation.shake_offset = V2(
                            random_ranged_float(&state->gameplay_data.prng_unessential, -1.0, 1.0) * speaker.animation.param0,
                            random_ranged_float(&state->gameplay_data.prng_unessential, -1.0, 1.0) * speaker.animation.param0
                        );
                        _debugprintf("new shake offset");
                        speaker.animation.t = 0.0f;
                    } else {
                        speaker.animation.type = DIALOGUE_SPEAKER_ANIMATION_NONE;
                    }
                }
            }
        }

        if (speaker_index == 1) {
            position.x = commands->screen_width - (position.x+sprite_image_size.x);
        }

        auto destination_rect        = rectangle_f32(
            position.x,
            position.y,
            sprite_image_size.x,
            sprite_image_size.y
        );


        if (speaker.visible) {
            render_commands_push_image_ext2(
                commands,
                speaker_img,
                destination_rect,
                RECTANGLE_F32_NULL,
                modulation,
                V2(0.5, 0.5),
                0,
                0,
                // unused angle_x
                DRAW_IMAGE_FLIP_HORIZONTALLY * speaker.mirrored, // flags param not used
                // blend mode param not used
                BLEND_MODE_ALPHA
            );
        }
    }
}

void Game::update_and_render_dialogue_ui(struct render_commands* commands, f32 dt) {
    bool in_conversation = this->state->dialogue_state.in_conversation;
    auto& dialogue_state = this->state->dialogue_state;
    auto  font           = resources->get_font(MENU_FONT_COLOR_WHITE);
    if (in_conversation) {
        // render the characters
        {
            update_and_render_dialogue_speaker(commands, dt, 0);
            update_and_render_dialogue_speaker(commands, dt, 1);
        }

        // render the text itself
        float dialogue_box_width    = commands->screen_width * 0.75;
        float dialogue_box_height   = 120;
        V2    dialogue_box_position = V2(40, commands->screen_height - (15 + dialogue_box_height));

        auto text                 = string_slice(
            string_from_cstring(dialogue_state.current_line),
            0,
            dialogue_state.shown_characters
        );


        render_commands_push_quad(
            commands,
            rectangle_f32(dialogue_box_position.x, dialogue_box_position.y,
                          dialogue_box_width, dialogue_box_height),
            color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA
        );

        render_commands_push_text(
            commands, font, 2, dialogue_box_position + V2(10, 10),
            text, color32f32(1,1,1,1), BLEND_MODE_ALPHA
        ); 

        // update some dialogue ui.
        {
            if (dialogue_state.shown_characters < dialogue_state.length && dialogue_state.speaking_lines_of_dialogue) {
                if (dialogue_state.type_timer <= 0.0f) {
                    dialogue_state.type_timer = DIALOGUE_TYPE_SPEED;
                    dialogue_state.shown_characters += 1;
                } else {
                    dialogue_state.type_timer -= dt;
                }
            } else {
                dialogue_state.speaking_lines_of_dialogue = false;
                // show the continue dialogue line
                // should animate nicely later
                render_commands_push_text(
                    commands, font, 2, dialogue_box_position + V2(dialogue_box_width - 100, dialogue_box_height - 20),
                    string_literal("Continue"), color32f32(1,1,1,1), BLEND_MODE_ALPHA
                ); 
            }
        }

        if (Action::is_pressed(ACTION_ACTION)) {
            if (dialogue_state.speaking_lines_of_dialogue && dialogue_state.shown_characters < dialogue_state.length) {
                dialogue_state.shown_characters = dialogue_state.length;
            } else {
                // allows the lua script to resume.
                dialogue_state.confirm_continue = true;
            }
        }
    }
}

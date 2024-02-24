#include "discord_rich_presence_integration.h"
// NOTE: meant to be included inside of game.cpp
// title screen code

void TitleScreen_MainCharacter_Puppet::set_new_eye_target(V2 new_target, f32 time_to_goto) {
    max_look_target_anim_t = time_to_goto;
    look_target_anim_t     = 0.0f;
    
    initial_eye_look_target = eye_look_target;
    final_eye_look_target   = new_target;
    _debugprintf("initialeyelook: %f, %f, to eyelook: %f, %f (%f)",
                 initial_eye_look_target.x,
                 initial_eye_look_target.y,
                 final_eye_look_target.x,
                 final_eye_look_target.y,
                 max_look_target_anim_t);
}

void TitleScreen_MainCharacter_Puppet::update_blinking(f32 dt) {
    if (time_between_blinks > 0.0f) {
        time_between_blinks -= dt;
    } else {
        switch (eye_frame) {
            case 0:
            case 1:
            case 2:
            case 4:
            case 5: {
                if (blink_timer >= 0.05) {
                    eye_frame++;
                    blink_timer = 0.0f;
                } else {
                    blink_timer += dt;
                }
            } break;
            case 3: { // hold closed eye
                if (blink_timer >= 1.00f) {
                    eye_frame++;
                    blink_timer = 0.0f;
                } else {
                    blink_timer += dt;
                }
            } break;
            case 6: { // allow eye to stay open
                blink_timer = 0;
                time_between_blinks = TITLESCREEN_MAINCHARACTER_PUPPET_TIME_BETWEEN_BLINKS;
                eye_frame = 0;
            } break;
        }
    }
}

void TitleScreen_MainCharacter_Puppet::update(f32 dt) {
    update_blinking(dt);

    if (!allow_looking_random)
        return;

    if (time_between_finding_new_look_target <= 0.0f) {
        s32 random_look = random_ranged_integer(prng, 0, 7);
        V2 eye_direction = V2_direction_from_degree(random_ranged_float(prng, -360.0f, 360.0f));
        f32 time_to_look = random_ranged_float(prng, 1.5f, 2.2f);

        if (random_look < 3) {
            eye_direction = V2(0, 0);
        }
        set_new_eye_target(eye_direction.normalized(), time_to_look);
        time_between_finding_new_look_target = random_ranged_float(prng, 3.0f, 6.0f);
    } else {
        if (look_target_anim_t < max_look_target_anim_t) {
            // _debugprintf("looking at target (%f/%f) (%f, %f)", look_target_anim_t, max_look_target_anim_t, eye_look_target.x, eye_look_target.y);
            
            f32 effective_t = clamp<f32>(look_target_anim_t / max_look_target_anim_t, 0.0f, 1.0f);
            look_target_anim_t += dt;
        
            eye_look_target.x = ease_out_back_f32(initial_eye_look_target.x, final_eye_look_target.x, effective_t);
            eye_look_target.y = ease_out_back_f32(initial_eye_look_target.y, final_eye_look_target.y, effective_t);
        } else {
            // _debugprintf("waiting to look at something else (%f)", time_between_finding_new_look_target);
            time_between_finding_new_look_target -= dt;
        }
    }

}

void TitleScreen_MainCharacter_Puppet::update_breathing_position_behavior(
    f32 dt,
    V2& head_position,
    V2& torso_position,
    V2& left_arm_position,
    V2& right_arm_position
) {
    // Breathing behaviors
    {
        head_position += (V2(0, -7 * clamp<f32>((normalized_sinf(Global_Engine()->global_elapsed_time) + 0.25), 0.15, 1.0f))) * scale;
        torso_position += (V2(0, -3 * clamp<f32>((normalized_cosf(Global_Engine()->global_elapsed_time * 1.45) + 0.15), 0.35, 1.0f))) * scale;
        left_arm_position += (V2(0, -5.5 * clamp<f32>((normalized_sinf(Global_Engine()->global_elapsed_time * 1.00) + 0.15), 0.35, 1.0f))) * scale;
        right_arm_position += (V2(0, -5.5 * clamp<f32>((normalized_sinf(Global_Engine()->global_elapsed_time * 1.00) + 0.15), 0.35, 1.0f))) * scale;
    }
}

void TitleScreen_MainCharacter_Puppet::update_head_for_eye_look_behavior(
    f32 dt,
    V2 eye_look_direction,
    V2& head_position
) {
    //eye_look_direction = eye_look_direction.normalized();
    {
        if (f32_close_enough(eye_look_direction.magnitude_sq(), 0.0f)) {
            // no modifications to position;
        }
        else {
            V2 eye_look_offset_distance = V2(5, 10);
            head_position.x += eye_look_direction.x * scale * ((eye_look_offset_distance.x));
            head_position.y += eye_look_direction.y * scale * ((eye_look_offset_distance.y));
        } 
    }
}

void TitleScreen_MainCharacter_Puppet::update_eye_look_behavior(
    f32 dt,
    V2 eye_look_direction,
    V2& left_eye_position,
    V2& right_eye_position
) {
    //eye_look_direction = eye_look_direction.normalized();
    // Eye look positions
    {
        if (f32_close_enough(eye_look_direction.magnitude_sq(), 0.0f)) {
            // no modifications to position;
        }
        else {
            int direction = sign_f32(eye_look_direction.x);

            f32 right_eye_influence_extra_weight = 0;
            f32 left_eye_influence_extra_weight = 0;
            f32 alternate_eye_decay_influence_factor = 0.4785;

            if (direction == -1) {
                left_eye_influence_extra_weight = eye_look_direction.magnitude();
                right_eye_influence_extra_weight = 1 - left_eye_influence_extra_weight * alternate_eye_decay_influence_factor;
            } else if (direction == 1) {
                right_eye_influence_extra_weight = eye_look_direction.magnitude();
                left_eye_influence_extra_weight = 1 - right_eye_influence_extra_weight * alternate_eye_decay_influence_factor;
            }

            left_eye_influence_extra_weight = clamp<f32>(left_eye_influence_extra_weight, 0.0f, 1.0f);
            right_eye_influence_extra_weight = clamp<f32>(right_eye_influence_extra_weight, 0.0f, 1.0f);

            V2 eye_look_offset_distance = V2(5, 10);

            left_eye_position.x  += eye_look_direction.x * scale * ((eye_look_offset_distance.x) * (1 + left_eye_influence_extra_weight));
            right_eye_position.x += eye_look_direction.x * scale * ((eye_look_offset_distance.x) * (1 + right_eye_influence_extra_weight));
            left_eye_position.y += eye_look_direction.y * scale * ((eye_look_offset_distance.y));
            right_eye_position.y += eye_look_direction.y * scale * ((eye_look_offset_distance.y));
        } 
    }
}

void TitleScreen_MainCharacter_Puppet::draw(f32 dt, struct render_commands* commands, Game_Resources* resources) {
    V2 old_position = position;
    position += V2(0, sinf(Global_Engine()->global_elapsed_time + 125) * 10 * scale);
    
    V2 arm_position_offset  = V2(5, 0);
    V2 head_position_offset = V2(0, -110);
    V2 eye_position_offset   = V2(30, 42);
    V2 eye_spacing_offset   = V2(20, 0);
    V2 eyebrow_spacing_offset = V2(0, -20);

    local s32 eye_frame_table[] = { 0, 1, 2, 3, 2, 1, 0 };
    s32       eye_sprite_frame  = eye_frame_table[eye_frame];
    eye_sprite_frame            = clamp<s32>(eye_sprite_frame, 0, 3); // incase I accidentally overrun bounds again...

    V2 torso_position;
    {
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_torso);
        V2   sprite_dimensions = V2(image->width, image->height) * scale;
        torso_position = position - V2(sprite_dimensions.x/2, 0);
    }

    V2 head_position          = torso_position + head_position_offset * scale;
    V2 left_arm_position      = torso_position - arm_position_offset * scale;
    V2 right_arm_position     = torso_position + (arm_position_offset - V2(3, 0)) * scale;

    // V2 target = V2(1, 0);
    V2 target = eye_look_target;
    {
        update_breathing_position_behavior(dt, head_position, torso_position, left_arm_position, right_arm_position);
        update_head_for_eye_look_behavior(dt, target, head_position);
    }

    V2 right_eye_position = head_position + (eye_position_offset + eye_spacing_offset) * scale;
    V2 left_eye_position = head_position + (eye_position_offset - eye_spacing_offset) * scale;
   
    {
        // V2 target = V2(cosf(Global_Engine()->global_elapsed_time), sinf(Global_Engine()->global_elapsed_time));
        update_eye_look_behavior(dt, target, left_eye_position, right_eye_position);
    }

    V2 right_eyebrow_position = right_eye_position + eyebrow_spacing_offset * scale;
    V2 left_eyebrow_position = left_eye_position + eyebrow_spacing_offset * scale;

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
        auto image             = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->title_screen_puppet_eyes[eye_sprite_frame]);
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
    position = old_position;
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

        state->puppet.prng = &prng;
    }
}

void Game::title_screen_replay_opening(void) {
    Transitions::do_shuteye_in(
        color32f32(0, 0, 0, 1),
        0.15f,
        0.3f
    );
    state->titlescreen_data.attract_mode_timer = 0.0f;

    Transitions::register_on_finish(
        [&](void*) mutable {
            opening_data_initialize(Global_Engine()->driver);
            this->state->opening_data.phase = OPENING_MODE_PHASE_FADE_IN;
            switch_ui(UI_STATE_INACTIVE);
            switch_screen(GAME_SCREEN_OPENING);
        }
    );
}

GAME_SCREEN(update_and_render_game_title_screen) {
    Discord_Integration::update_activity(
        discord_activity()
        .Details(string_literal("About to start a new adventure!"))
        .Large_Image(DISCORD_GAMEICON_ASSET_KEY)
    );

    {
        auto& state = this->state->titlescreen_data;
        if (state.phase == TITLE_SCREEN_ANIMATION_PHASE_IDLE) {
            state.attract_mode_timer += dt;
            if (Input::any_input_activity()) {
                state.attract_mode_timer = 0.0f;
            } else {
                if (state.attract_mode_timer >= ATTRACT_MODE_TIMER_MAX && !Transitions::fading()) {
                    title_screen_replay_opening();
                }
            }
        }
    }

    auto commands = ui_render_commands;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    f32 ui_x_title = 50.0f; // ends at 50
    f32 ui_x = 100.0f;       // ends at 100
    // setup camera for rendering main part
    {
        s32 new_screen_width = game_render_commands->screen_width;
        s32 new_screen_height = game_render_commands->screen_height;
        {
            if (state->titlescreen_data.last_screen_width  != new_screen_width ||
                state->titlescreen_data.last_screen_height != new_screen_height) {
                state->titlescreen_data.last_screen_width   = new_screen_width;
                state->titlescreen_data.last_screen_height  = new_screen_height;
                state->titlescreen_data.main_camera.xy = V2(new_screen_width/2, new_screen_height/2);
            }
        }

        // Handle_Title_Screen_Animation_Phase
        {
            auto& titlescreen_data = state->titlescreen_data;
            const f32 max_zoom = 2.5f;
            switch (titlescreen_data.phase) {
                case TITLE_SCREEN_ANIMATION_PHASE_CLOSE_UP_OF_FACE: {
                    const f32 phase_max_t = 2.55f;
                    GameUI::set_all_visual_alpha(0.0f);

                    titlescreen_data.main_camera.zoom     = max_zoom;
                    titlescreen_data.main_camera.xy       = V2(-new_screen_width/2 * titlescreen_data.main_camera.zoom, -new_screen_height/2 * titlescreen_data.main_camera.zoom);

                    if (titlescreen_data.anim_timer < phase_max_t) {
                        titlescreen_data.anim_timer += dt;
                    } else {
                        titlescreen_data.anim_timer = 0.0f;
                        titlescreen_data.phase      = TITLE_SCREEN_ANIMATION_PHASE_ZOOM_OUT;
                    }

                    // move puppet to center of screen
                    {
                        state->titlescreen_data.puppet.position = V2(-new_screen_width/2, -new_screen_height/2+55);
                    }
                } break;
                case TITLE_SCREEN_ANIMATION_PHASE_ZOOM_OUT: {
                    const f32 phase_max_t = 2.65f;
                    const f32 effective_t = clamp<f32>(titlescreen_data.anim_timer/phase_max_t, 0.0f, 1.0f);

                    GameUI::set_all_visual_alpha(0.0f);
                    titlescreen_data.main_camera.zoom     = ease_out_back_f32(max_zoom, 1.0f, effective_t);
                    titlescreen_data.main_camera.xy       = V2(-new_screen_width/2 * titlescreen_data.main_camera.zoom, -new_screen_height/2 * titlescreen_data.main_camera.zoom);

                    if (titlescreen_data.anim_timer < phase_max_t) {
                        titlescreen_data.anim_timer += dt;
                    } else {
                        titlescreen_data.anim_timer = 0.0f;
                        titlescreen_data.phase      = TITLE_SCREEN_ANIMATION_PHASE_MOVE_PUPPET_TO_RIGHT_AND_FADE_IN_MENU;
                    }
                } break;
                case TITLE_SCREEN_ANIMATION_PHASE_MOVE_PUPPET_TO_RIGHT_AND_FADE_IN_MENU: {
                    const f32 phase_max_t = 2.00f;
                    const f32 effective_t2 = clamp<f32>(titlescreen_data.anim_timer/(phase_max_t/2), 0.0f, 1.0f);
                    const f32 effective_t = clamp<f32>(titlescreen_data.anim_timer/phase_max_t, 0.0f, 1.0f);

                    titlescreen_data.main_camera.zoom     = 1.0f;
                    titlescreen_data.main_camera.xy       = V2(-new_screen_width/2 * titlescreen_data.main_camera.zoom, -new_screen_height/2 * titlescreen_data.main_camera.zoom);
                    GameUI::set_all_visual_alpha(effective_t);

                    ui_x       = quadratic_ease_in_out_f32(-100,  100, effective_t);
                    ui_x_title = quadratic_ease_in_out_f32(-120,  50, effective_t);

                    if (titlescreen_data.anim_timer < phase_max_t) {
                        titlescreen_data.anim_timer += dt;
                    } else {
                        titlescreen_data.anim_timer = 0.0f;
                        titlescreen_data.phase      = TITLE_SCREEN_ANIMATION_PHASE_IDLE;
                        titlescreen_data.puppet.allow_looking_random = true;
                        
                    }
                    {
                        state->titlescreen_data.puppet.position = V2(quadratic_ease_in_out_f32(-new_screen_width/2,  -new_screen_width/2 + new_screen_width/4, effective_t2), -new_screen_height/2+55);
                    }
                } break;
                case TITLE_SCREEN_ANIMATION_PHASE_IDLE: {
                    GameUI::set_all_visual_alpha(1.0f);
                    titlescreen_data.main_camera.xy = V2(-new_screen_width / 2 * titlescreen_data.main_camera.zoom, -new_screen_height / 2 * titlescreen_data.main_camera.zoom);
                    state->titlescreen_data.puppet.position = V2(-new_screen_width/2 + new_screen_width/4, -new_screen_height/2+55);
                } break;
            }
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
        state->titlescreen_data.puppet.draw(dt, game_render_commands, resources);
    }

    if (state->ui_state != UI_STATE_INACTIVE) {
        handle_ui_update_and_render(commands, dt);
    } else {
        GameUI::set_font_active(resources->get_font(MENU_FONT_COLOR_BLOODRED));
        GameUI::set_font_selected(resources->get_font(MENU_FONT_COLOR_GOLD));

        bool ui_active = !Transitions::fading() && (state->titlescreen_data.phase == TITLE_SCREEN_ANIMATION_PHASE_IDLE);

        GameUI::set_ui_id((char*)"ui_titlescreen_menu");
        GameUI::begin_frame(commands, &resources->graphics_assets);
        {
            f32 y = 100;
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_GOLD));
            GameUI::label(V2(ui_x_title, y), string_literal("SOULSTORM"), color32f32(1, 1, 1, 1), 4);
            GameUI::set_font(resources->get_font(MENU_FONT_COLOR_WHITE));

            y += 45;

            if (GameUI::button(V2(ui_x, y), string_literal("Play"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
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

            if (GameUI::button(V2(ui_x, y), string_literal("Options"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_OPTIONS);
            }
            y += 30;

            if (GameUI::button(V2(ui_x, y), string_literal("Achievements"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
                switch_ui(UI_STATE_ACHIEVEMENTS);
            }
            y += 30;
            if (GameUI::button(V2(ui_x, y), string_literal("Credits"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
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
            if (GameUI::button(V2(ui_x, y), string_literal("Opening"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
                title_screen_replay_opening();
            }
            y += 30;
            if (GameUI::button(V2(ui_x, y), string_literal("Exit To Windows"), color32f32(1, 1, 1, 1), 2, ui_active) == WIDGET_ACTION_ACTIVATE) {
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

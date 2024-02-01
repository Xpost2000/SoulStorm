// NOTE: meant to be included inside of game.cpp
// main menu code

// cutscene coroutines.
// these cutscenes are really good copy and paste fu.
void cutscene_unlocked_pet_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);
    TASK_WAIT(0.7f);
    {
        *trauma_timer = 0;
        while ((*trauma_timer) < 0.35f) {
            controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
            camera_set_trauma(&camera, 0.17f);
            *trauma_timer += dt;
            JDR_Coroutine_YieldNR();
        }
    }
    TASK_WAIT(0.25f);
    {
        *trauma_timer = 0;
        while ((*trauma_timer) < 0.25f) {
            controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
            camera_set_trauma(&camera, 0.17f);
            *trauma_timer += dt;
            JDR_Coroutine_YieldNR();
        }
    }
    TASK_WAIT(0.25f);
    {
        *trauma_timer = 0;
        while ((*trauma_timer) < 0.25f) {
            controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
            camera_set_trauma(&camera, 0.17f);
            *trauma_timer += dt;
            JDR_Coroutine_YieldNR();
        }
    }

    TASK_WAIT(0.25f);
    // Show pop up for current pet
    camera_set_point_to_interpolate(&camera, V2(resolution.x, resolution.y), 2.0);
    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    *trauma_timer = 0;
    TASK_WAIT(0.25f);
    while ((*trauma_timer) < 0.65f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, 0.17f);
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    {
        auto& current_pet = main_menu_state->pets[state->gameplay_data.unlocked_pets++];
        current_pet.position = V2(resolution.x/2, resolution.y/2);
    }
    TASK_WAIT(0.5f);

    main_menu_state->cutscene3.phase = MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_IN_UNLOCK_BOX;
    main_menu_state->cutscene3.timer = 0.0f;
    JDR_Coroutine_End;
}

void cutscene_completed_maingame_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);

    *trauma_timer = 0;

    main_menu_state->screen_message_add(string_literal("Congratulations!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("I hope you enjoyed your time!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("Feel free to maximize your score!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("Otherwise, I hope to see you again!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}


    // NOTE: should I make the portals unlock through cutscene? maybe
    // prolly.
#if 0
    // post game portal focus and spawn.
    // Using a similar animation to the player.
    {
        auto focus_portal = &main_menu_state->portals[3];
        auto position = focus_portal->position;
        position *= 1.5f;
        camera_set_point_to_interpolate(&camera, position, 1.5f);
    }

    main_menu_state->screen_message_add(string_literal("Prepare yourself well."));
    main_menu_state->screen_message_add(string_literal("Your final challenge awaits."));
    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    // do the same shake and spawn effect.
    // maybe play a sound?
    while ((*trauma_timer) < 1.5f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, clamp<f32>(*trauma_timer + 0.35f, 0.15f, 0.35f));
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.7f, 0.7f, 350);
    camera_traumatize(&camera, 0.58f);
    {
        auto& focus_portal = main_menu_state->portals[3];
        focus_portal.visible = true;
    }

    camera_set_point_to_interpolate(&camera, V2(resolution.x/2, resolution.y/2), 1.0f);

    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }
#endif
    TASK_WAIT(0.5f);

    main_menu_state->cutscene1.phase = 0;
    JDR_Coroutine_End;
}

void cutscene_introduction_fasttrack_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);
    *trauma_timer = 0.0f;
    main_menu_state->player.visible = false;

    TASK_WAIT(0.25f);
    main_menu_state->screen_message_add(string_literal("Welcome back!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}
    main_menu_state->screen_message_add(string_literal("There's always more to do!"));
    while (!main_menu_state->screen_messages_finished()) {JDR_Coroutine_YieldNR();}

    while ((*trauma_timer) < 0.45f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, 0.17f);
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.5f, 0.5f, 350);
    camera_traumatize(&camera, 0.55f);

    main_menu_state->player.visible = true;
    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    main_menu_state->cutscene2.phase = 0;
    JDR_Coroutine_End;
}

void cutscene_introduction_firsttime_task(jdr_duffcoroutine_t* co) {
    _jdr_bind_current(co);
    Game_State* state           = ((Game_Task_Userdata*)(co->userdata))->game_state;
    auto        main_menu_state = &state->mainmenu_data;
    f32         dt              = ((Game_Task_Userdata*)(co->userdata))->dt;
    V2          resolution      = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

    auto& camera = main_menu_state->main_camera;
    float* trauma_timer = (float*)_jdr_alloc_var(sizeof(*trauma_timer));

    JDR_Coroutine_Start(co, Start);
    *trauma_timer = 0.0f;
    main_menu_state->player.visible = false;

    TASK_WAIT(1.0f);
    camera_set_point_to_interpolate(&camera, V2(resolution.x, resolution.y), 2.0);

    // wait for camera to finish manually...
    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    while ((*trauma_timer) < 1.5f) {
        controller_rumble(Input::get_gamepad(0), 0.25f, 0.25f, 10);
        camera_set_trauma(&camera, 0.15f);
        *trauma_timer += dt;
        JDR_Coroutine_YieldNR();
    }

    controller_rumble(Input::get_gamepad(0), 0.4f, 0.4f, 350);
    camera_traumatize(&camera, 0.47f);

    main_menu_state->player.visible = true;
    main_menu_state->screen_message_add(string_literal("Now the adventure begins..."));

    while (!main_menu_state->screen_messages_finished()) {
        JDR_Coroutine_YieldNR();
    }

    TASK_WAIT(1.0f);
    camera_set_point_to_interpolate(&camera, V2(resolution.x/2, resolution.y/2), 1.0);

    while (camera_interpolating(&camera)) {
        JDR_Coroutine_YieldNR();
    }

    main_menu_state->cutscene2.phase = 0;
    JDR_Coroutine_End;
}

void MainMenu_Stage_Portal::update(MainMenu_Data* const state, f32 dt) {
    animation_t += dt;

    if (animation_t >= MAIN_MENU_STAGE_PORTAL_TIME_UNTIL_NEXT_FRAME) {
        if (triggered_level_selection) {
            frame_index += 1;
        } else {
            frame_index -= 1;
        }

        if (frame_index <= 0) {
            frame_index = 0;   
        } else if (frame_index >= PORTAL_IMAGE_FRAME_COUNT) {
            frame_index = PORTAL_IMAGE_FRAME_COUNT-1;
        }

        animation_t = 0.0f;
    }
}

local color32f32 portal_color_from_stage_id(s32 stage_id) {
    // TODO: pick better colors.
    switch (stage_id) {
        case 0: {
            return color32f32(210/255.0f, 77/255.0f, 87/255.0f, 1);
            // return color32f32(1, 0, 0, 1);
        } break;
        case 1: {
            // return color32f32(0, 1, 0, 1);
            return color32f32(194/255.0f, 249/255.0f, 112/255.0f, 1);
        } break;
        case 2: {
            // return color32f32(0, 0, 1, 1);
            return color32f32(3/255.0f, 138/255.0f, 255/255.0f, 1.0f);
        } break;
    }

    return color32f32(1, 1, 1, 1);
}

void MainMenu_Stage_Portal::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (!visible) return;

    auto r = get_rect();

    auto& stage_details   = stage_list[stage_id];
    auto  name_string     = stage_details.name;
    auto  subtitle_string = stage_details.subtitle;

#if 0
#ifndef RELEASE
    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 255, 0, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA);
#endif
#endif
    {
        image_id sprite_image = resources->main_menu_portal_images[frame_index];
        auto image_object = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_image);
        f32  image_scale  = 1.50f + normalized_sinf(Global_Engine()->global_elapsed_time * 5.0f) * 0.15f;
        V2   image_size   = V2(image_object->width, image_object->height);

        auto portal_color = portal_color_from_stage_id(stage_id);

        render_commands_push_image(
            commands,
            image_object,
            // magic numbers are just to align the sprite.
            rectangle_f32(
                r.x - (image_size.x * (image_scale - 1.0f))/2,
                r.y - (image_size.y*image_scale * 0.35f),
                image_size.x*image_scale,
                image_size.y*image_scale
            ),
            RECTANGLE_F32_NULL,
            color32f32(portal_color.r, portal_color.g, portal_color.b, 0.35f),
            0,
            BLEND_MODE_ADDITIVE
        );

        // Hmm. This is the one time I use additive blending. LOL
        render_commands_push_image(
            commands,
            image_object,
            // magic numbers are just to align the sprite.
            rectangle_f32(
                r.x - (image_size.x * (image_scale - 1.0f))/2,
                r.y - (image_size.y*image_scale * 0.35f),
                image_size.x*image_scale,
                image_size.y*image_scale
            ),
            RECTANGLE_F32_NULL,
            color32f32(portal_color.r, portal_color.g, portal_color.b, normalized_sinf(Global_Engine()->global_elapsed_time * 1.5f) * 0.45f + 0.35f),
            0,
            BLEND_MODE_ADDITIVE
        );
    }

#ifndef RELEASE
    if (DebugUI::enabled()) {
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
#endif
}

rectangle_f32 MainMenu_Stage_Portal::get_rect() {
    return rectangle_f32(
        position.x - scale.x,
        position.y - scale.y,
        scale.x*2,
        scale.y*2
    );
}

// MainMenu_Player
rectangle_f32 MainMenu_Player::get_rect() {
    return rectangle_f32(
        position.x - scale.x,
        position.y - scale.y,
        scale.x*2,
        scale.y*2
    );
}

void MainMenu_Player::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (!visible) return;
    auto r = get_rect();

    // black rectangles for default
#ifndef RELEASE
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
#endif
    image_id sprite_image = resources->hero_images[HERO_IMAGE_FRAME_IDLE0];

    if (!state->cutscene_active()) {
        switch (facing_direction) {
            case MAIN_MENU_PLAYER_FACING_DIRECTION_FORWARD: {
                sprite_image = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_FRONT];
            } break;
            case MAIN_MENU_PLAYER_FACING_DIRECTION_BACK: {
                sprite_image = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_BACK];
            } break;
            case MAIN_MENU_PLAYER_FACING_DIRECTION_LEFT: {
                sprite_image = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_LEFT];
            } break;
            case MAIN_MENU_PLAYER_FACING_DIRECTION_RIGHT: {
                sprite_image = resources->hero_images[HERO_IMAGE_FRAME_FLOAT_RIGHT];
            } break;
        }
    }

    auto image_object = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_image);
    f32  image_scale  = 1.25f;
    V2   image_size   = V2(image_object->width, image_object->height);
    V2   hover_offset = V2(0, sinf(Global_Engine()->global_elapsed_time * 2.852) * 5) * image_scale;

    render_commands_push_image(
        commands,
        image_object,
        // magic numbers are just to align the sprite.
        rectangle_f32(
            position.x - scale.x - ((image_size.x*image_scale) * 0.15f) + hover_offset.x,
            position.y - scale.x - ((image_size.y*image_scale) * 0.85f) + hover_offset.y,
            image_size.x*image_scale,
            image_size.y*image_scale
        ),
        RECTANGLE_F32_NULL,
        color32f32(1.0, 1.0f, 1.0, 1.0f),
        0,
        BLEND_MODE_ALPHA
    );
}

void MainMenu_Player::update(MainMenu_Data* state, f32 dt) {
    V2 axes = V2(Action::value(ACTION_MOVE_LEFT) + Action::value(ACTION_MOVE_RIGHT), Action::value(ACTION_MOVE_UP) + Action::value(ACTION_MOVE_DOWN));
    if (axes.magnitude_sq() > 1.0f) axes = axes.normalized();
    f32 axes_magnitude = axes.magnitude_sq();

    const float UNIT_SPEED = 350;

    velocity.x = axes[0] * UNIT_SPEED;
    velocity.y = axes[1] * UNIT_SPEED;

    position += velocity * dt;

    // update facing direction
    if (!f32_close_enough(axes_magnitude, 0.0f)){
        bool should_be_horizontal = fabs(floorf(velocity.x)) > fabs(floorf(velocity.y));
        if (should_be_horizontal)  {
            if (velocity.x < 0.0f) {
                facing_direction = MAIN_MENU_PLAYER_FACING_DIRECTION_LEFT;
            } else {
                facing_direction = MAIN_MENU_PLAYER_FACING_DIRECTION_RIGHT;
            }
        } else {
            if (velocity.y < 0.0f) {
                facing_direction = MAIN_MENU_PLAYER_FACING_DIRECTION_BACK;
            } else {
                facing_direction = MAIN_MENU_PLAYER_FACING_DIRECTION_FORWARD;
            }
        }
    }
}

// MainMenu_Player End

// MainMenu_Pet
// Appreciate interaction cannot be randomly decided.
local f32 pet_weight_actions_by_type[GAME_PET_ID_COUNT][MAIN_MENU_PET_MAIN_ACTIONS] = {
    // cat
    { // Should be lazier and move less.
        0.2,
        0.1,
        0.2,
        0.3,
        0.2,
        0.07
    },
    // dog
    {
        0.1,
        0.1,
        0.1,
        0.6,
        0.1,
        0.07
    },  
    // fish
    {
        0.3,
        0.1,
        0.2,
        0.2,
        0.2,
        0.07
    }   
};

// Everything is assumed to be +/- 0.10 seconds
local f32 pet_action_lengths[GAME_PET_ID_COUNT][MAIN_MENU_PET_MAIN_ACTIONS] = {
    // cat
    {
        5,
        5,
        5,
        2.0f,
        0.10f,
        0.10f,
    },
    // dog
    {
        3,
        3,
        3,
        2.0f,
        0.10f,
        0.10f,
    },
    // fish
    {
        1.5,
        1.7,
        1.5,
        4.5f,
        0.15f,
        0.10f,
    }
};

local f32 pet_velocities[GAME_PET_ID_COUNT] = {
    100,
    120,
    50
};

void MainMenu_Pet::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    // pets are always visible.
    // (provided their index is spawned.)
    auto r = get_rect();
    image_id* image_set = resources->pet_images[type];

#ifndef RELEASE
    // black rectangles for default
    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(255, 0, 0, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA);
#endif

    // Default to front facing sprite since I drew those better,
    // and also I think it makes more sense to visually show.
    image_id image = image_set[1];
    if (current_action == MAIN_MENU_PET_ACTION_MOVING) {
        // back, front, left, right :: is the order I load them in
        s32 direction_sprite_id = 0;

        s32 sign_x      = sign_f32(current_direction.x);
        s32 sign_y      = sign_f32(current_direction.y);
        f32 x_magnitude = fabs(current_direction.x);
        f32 y_magnitude = fabs(current_direction.y);

        if (x_magnitude > y_magnitude) {
            if (sign_x == 1) {
                direction_sprite_id = PET_IMAGE_SPRITE_FACING_DIRECTION_LEFT;
            } else {
                direction_sprite_id = PET_IMAGE_SPRITE_FACING_DIRECTION_RIGHT;
            }
        } else {
            if (sign_y == 1) {
                direction_sprite_id = PET_IMAGE_SPRITE_FACING_DIRECTION_BACK;
            } else {
                direction_sprite_id = PET_IMAGE_SPRITE_FACING_DIRECTION_FRONT;
            }
        }

        image = image_set[direction_sprite_id];
    }

    // They will float just like the player
    {
        auto image_object = graphics_assets_get_image_by_id(&resources->graphics_assets, image);
        f32  image_scale  = 1.25f;
        V2   image_size   = V2(image_object->width, image_object->height);
        V2   hover_offset = V2(0, sinf(Global_Engine()->global_elapsed_time * 2.852) * 5) * image_scale;

        render_commands_push_image(
            commands,
            image_object,
            // magic numbers are just to align the sprite.
            rectangle_f32(
                position.x - r.w/2 - ((image_size.x*image_scale) * 0.15f) + hover_offset.x,
                position.y - ((image_size.y*image_scale) * 0.85f) + hover_offset.y,
                image_size.x*image_scale,
                image_size.y*image_scale
            ),
            RECTANGLE_F32_NULL,
            color32f32(1.0, 1.0f, 1.0, 1.0f),
            0,
            BLEND_MODE_ALPHA
        );
    }
}

void MainMenu_Pet::update(MainMenu_Data* state, f32 dt) {
    switch (current_action) {
        case MAIN_MENU_PET_ACTION_IDLE: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_IDLE1: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_IDLE2: {
            // nothing.
        } break;
        case MAIN_MENU_PET_ACTION_MOVING: {
            position += current_direction * pet_velocities[type] * dt;
        } break;
        case MAIN_MENU_PET_ACTION_MAKE_NOISE: {
            if (action_timer <= 0.0f) {
                _debugprintf("woof.");
            }
        } break;
        case MAIN_MENU_PET_ACTION_POOP: {
            if (action_timer <= 0.0f) {
                _debugprintf("dumpy.");
                state->spawn_poop(position);
            }
        } break;
        case MAIN_MENU_PET_ACTION_APPRECIATE_INTERACTION: {
            // smile :)
            // spawn a heart or something.
            // otherwise nothing.
        } break;
    }

    if (action_timer <= 0.0f) {
        decide_new_action(&state->prng);
    } else {
        action_timer -= dt;
    }
}

void MainMenu_Pet::decide_new_action(random_state* prng) {
    current_action = random_weighted_selection(
        prng,
        pet_weight_actions_by_type[type],
        MAIN_MENU_PET_MAIN_ACTIONS
    );

    action_timer = pet_action_lengths[type][current_action] + random_ranged_float(prng, -0.1, 0.1);

    current_direction = V2_direction_from_degree(random_ranged_float(prng, -360.0f, 360.0f));
}

rectangle_f32 MainMenu_Pet::get_rect(void) {
    return rectangle_f32(
        position.x - 16,
        position.y - 16,
        32,
        32
    );
}

// MainMenu_Pet End

// MainMenu_Clutter_Poop
void MainMenu_Clutter_Poop::update(f32 dt) {
    if (lifetime <= 0.0f) {
        dead = true;
    } else {
        lifetime -= dt;
    }
}

void MainMenu_Clutter_Poop::draw(MainMenu_Data* const state, struct render_commands* commands, Game_Resources* resources) {
    if (dead) return;

    auto r = rectangle_f32(
        position.x - 8,
        position.y - 8,
        16,
        16
    );

    render_commands_push_quad_ext(
        commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 0, 255, 255),
        V2(0, 0), 0,
        BLEND_MODE_ALPHA
    );
}
// MainMenu_Clutter_Poop end

void MainMenu_Data::spawn_poop(V2 where) {
    MainMenu_Clutter_Poop poop;
    poop.position = where;
    clutter_poops.push(poop);
}

void MainMenu_Data::cleanup_all_dead_poops(void) {
    for (int i = 0; i < clutter_poops.size; ++i) {
        if (clutter_poops[i].dead) {
            clutter_poops.pop_and_swap(i);
        }
    }
}

// NOTE: I have to remap this because
// the images do not intuitively cycle :/
local s32 _remap_pet_facing_direction_to_sprite_index(s32 index) {
    switch (index) {
        // Front
        case 0: {
            return PET_IMAGE_SPRITE_FACING_DIRECTION_FRONT;
        } break;
        // Right
        case 1: {
            return PET_IMAGE_SPRITE_FACING_DIRECTION_RIGHT;
        } break;
        // Back
        case 2: {
            return PET_IMAGE_SPRITE_FACING_DIRECTION_BACK;
        } break;
        // Left
        case 3: {
            return PET_IMAGE_SPRITE_FACING_DIRECTION_LEFT;
        } break;
    }

    return -1;
}

void MainMenu_Data::start_unlock_pet_cutscene(Game_State* game_state) {
    if (!cutscene3.triggered) {
        _debugprintf("Starting pet unlock cutscene!");
        cutscene3.triggered            = true;
        cutscene3.phase                = 1;
        cutscene3.pet_spin_timer       = 0;
        cutscene3.pet_facing_direction = 0;
        game_state->coroutine_tasks.add_task(game_state, cutscene_unlocked_pet_task);
    }
}

void MainMenu_Data::start_completed_maingame_cutscene(Game_State* game_state) {
    if (!cutscene1.triggered) {
        _debugprintf("Starting main game completion cutscene!");
        cutscene1.triggered        = true;
        cutscene1.phase            = 1;
        game_state->coroutine_tasks.add_task(game_state, cutscene_completed_maingame_task);
    }
}

void MainMenu_Data::start_introduction_cutscene(Game_State* game_state, bool fasttrack){
    if (!cutscene2.triggered) {
        cutscene2.phase     = 1;
        cutscene2.triggered = true;
        _debugprintf("Triggering cutscene introduction");

        if (fasttrack) {
            _debugprintf("Fast track intro");
            game_state->coroutine_tasks.add_task(game_state, cutscene_introduction_fasttrack_task);
        } else {
            _debugprintf("Standard intro");
            game_state->coroutine_tasks.add_task(game_state, cutscene_introduction_firsttime_task);
        }
    }
}

bool MainMenu_Data::cutscene_active() {
    return cutscene1.phase != 0 || cutscene2.phase != 0 || cutscene3.phase != 0;
}

bool MainMenu_Data::screen_messages_finished() {
    return (screen_message_fade_t <= 0.0f && screen_messages.size == 0);
}

void MainMenu_Data::screen_message_add(string message) {
    MainMenu_ScreenMessage new_message;
    new_message.phase = MAIN_MENU_SCREEN_MESSAGE_APPEAR;
    new_message.timer = 0.0f;
    new_message.text = message;
    screen_messages.push(new_message);
}

local void initialize_portal_particle_emitters(MainMenu_Stage_Portal* portal,
                                               Game_Resources* resources,
                                               s32 sprite_id_for_main,
                                               s32 sprite_id_for_vortex_sprite) {
    {
        auto& emitter = portal->emitter_main;
        emitter.sprite = sprite_instance(resources->projectile_sprites[sprite_id_for_main]);
        emitter.scale  = 0.25f;
        emitter.emit_per_emission = 4;
        emitter.lifetime = 0.75f;
        emitter.velocity_x_variance = V2(-100, 100);
        emitter.velocity_y_variance = V2(-100, 100);
        emitter.acceleration_x_variance = V2(-10, 10);
        emitter.acceleration_y_variance = V2(-10, 10);
        emitter.lifetime_variance   = V2(-0.5f, 1.0f);
        emitter.emission_max_timer = 0.028f;
    }

    {
        auto& emitter = portal->emitter_vortex;
        emitter.sprite = sprite_instance(resources->projectile_sprites[sprite_id_for_vortex_sprite]);
        emitter.emit_per_emission = 16;
        emitter.scale  = 0.15f;
        emitter.lifetime = 2.0f;
        emitter.lifetime_variance   = V2(-0.5f, 1.0f);
        emitter.use_attraction_point = true;
        emitter.attraction_force     = 100.0f;
        emitter.emission_max_timer = 0.028f;
    }
}

void Game::mainmenu_data_initialize(Graphics_Driver* driver) {
    {
        auto state = &this->state->mainmenu_data;
        // auto resolution = driver->resolution();
        auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);

        state->particle_pool.init(arena, 1024);
        
        state->player.position      = V2(resolution.x / 2, resolution.y / 2);
        state->player.scale         = V2(15, 15);
        state->player.velocity      = V2(0, 0);
        // I need to utilize a camera effect which relies on centering
        // for polish reasons.
        state->main_camera          = camera(V2(resolution.x/2, resolution.y/2), 1.0);
        state->main_camera.centered = true;
        state->main_camera.rng      = &state->prng;
        state->prng                 = random_state();

        state->screen_messages = Fixed_Array<MainMenu_ScreenMessage>(arena, 32);
        state->clutter_poops   = Fixed_Array<MainMenu_Clutter_Poop>(arena, 64);
        state->portals = Fixed_Array<MainMenu_Stage_Portal>(arena, 4);
        {
            // initialize all portals here for the main menu
            // portals should be spaced differently based on what's unlocked
            // but I'll just place them manually
            {
                auto& portal = state->portals[0]; 
                portal.stage_id = 0;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.visible = true;

                initialize_portal_particle_emitters(
                    &portal,
                    this->state->resources,
                    PROJECTILE_SPRITE_RED_ELECTRIC,
                    PROJECTILE_SPRITE_RED
                );
            }

            {
                auto& portal = state->portals[1]; 
                portal.stage_id = 1;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.visible = true;

                initialize_portal_particle_emitters(
                    &portal,
                    this->state->resources,
                    PROJECTILE_SPRITE_GREEN_ELECTRIC,
                    PROJECTILE_SPRITE_GREEN
                );
            }

            {
                auto& portal = state->portals[2]; 
                portal.stage_id = 2;
                portal.scale = V2(15, 15);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.prerequisites[1] = 1;
                portal.visible = true;

                initialize_portal_particle_emitters(
                    &portal,
                    this->state->resources,
                    PROJECTILE_SPRITE_BLUE_ELECTRIC,
                    PROJECTILE_SPRITE_BLUE
                );
            }

            // NOTE: postgame portal.
            // NOTE: unused, I do not have time to make more levels :(
            {
                auto& portal = state->portals[3]; 
                portal.stage_id = 3;
                portal.scale = V2(15, 15);
                portal.position = V2(350-30, 400);
                for (int i = 0; i < array_count(portal.prerequisites); ++i) {
                    portal.prerequisites[i] = -1;
                }
                portal.prerequisites[0] = 0;
                portal.prerequisites[1] = 1;
                portal.prerequisites[2] = 2;

                {
                    auto& emitter = portal.emitter_main;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
                    emitter.scale  = 1.0f;
                    emitter.lifetime = 1.0f;
                    emitter.velocity_x_variance = V2(-100, 100);
                    emitter.velocity_y_variance = V2(-100, 100);
                    emitter.acceleration_x_variance = V2(-100, 100);
                    emitter.acceleration_y_variance = V2(-100, 100);
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.emission_max_timer = 0.025f;
                }

                {
                    auto& emitter = portal.emitter_vortex;
                    emitter.sprite = sprite_instance(this->state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]);
                    emitter.scale  = 0.5;
                    emitter.lifetime = 2.0f;
                    emitter.lifetime_variance   = V2(-0.5f, 1.0f);
                    emitter.use_attraction_point = true;
                    emitter.attraction_force     = 100.0f;
                    emitter.emission_max_timer = 0.025f;
                }
            }

            state->portals.size = 4;
        }

        // initializing all the stars' positions
        {
            auto bkg_slow_stars = state->star_positions[0];
            auto bkg_faster_stars = state->star_positions[1];
            auto& prng = this->state->mainmenu_data.prng;

            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                bkg_slow_stars[i] = V2(random_ranged_float(&prng, -854, 854),
                                       random_ranged_float(&prng, -480, 480));
                bkg_faster_stars[i] = V2(random_ranged_float(&prng, -854, 854),
                                         random_ranged_float(&prng, -480, 480));
            }

            auto& sparkling_stars = state->sparkling_stars;
            for (int i = 0; i < MAX_MAINMENU_SPARKLING_STARS; ++i) {
                auto& star = sparkling_stars[i];
                star.visibility_delay_timer = star.max_visibility_delay_timer = random_ranged_float(&prng, 1.25f, 7.00f);
                star.anim_timer = 0.0f;
                star.frame_index = 0;
                star.position = V2(random_ranged_float(&prng, -800, 800), random_ranged_float(&prng, -480, 480));
                star.scale = random_ranged_float(&prng, 1.0f, 1.75f);
            }
        }

        // initialize pets
        {
            // regardless of whether they've been "spawned/visible" I'll seed all of their initial actions
            auto prng = &this->state->mainmenu_data.prng;
            for (int i = 0; i < array_count(state->pets); ++i) {
                state->pets[i].type = i;
                state->pets[i].decide_new_action(prng);
                state->pets[i].position = V2(random_ranged_float(prng, 130.0f, 360.0f),
                                             random_ranged_float(prng, 130.0f, 360.0f));
            }
        }
    }
}

void MainMenu_Data::adjust_entities_for_screen_resolution(int new_screen_width, int new_screen_height) {
    auto& main_menu_state = *this;
    {
        if (main_menu_state.last_screen_width  != new_screen_width ||
            main_menu_state.last_screen_height != new_screen_height) {
            main_menu_state.last_screen_width   = new_screen_width;
            main_menu_state.last_screen_height  = new_screen_height;
            main_menu_state.main_camera.xy = V2(new_screen_width/2, new_screen_height/2);
        }

        // Also portals will be fitted to be at the same "relative" place
        // based on resolution.

        // There is no way to hook into resolution changes, and there is no other way
        // to make the "playable" main menu look nice without hacking this in anyway.

        // NOTE: only the three playable portals (there is a fourth postgame portal that I'm just disabling, and
        // unlikely to re-enable. Ever.)
        {
            f32 screen_width = (new_screen_width);
            f32 screen_third = (screen_width*0.9f / 3.0f);
            for (int i = 0; i < 3; ++i) {
                auto& portal = main_menu_state.portals[i];
                portal.position             = V2(screen_width * 0.1f + (screen_third*1.15) * i, 80);
                portal.emitter_main.shape   = particle_emit_shape_circle(portal.position, 35.0f);
                portal.emitter_vortex.attraction_point     = portal.position;
                portal.emitter_vortex.shape = particle_emit_shape_circle(portal.position, 80.0f);
            }
        }
    }
}

void Game::update_and_render_game_main_menu(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt) {
    auto& main_menu_state = state->mainmenu_data;
    V2 resolution = V2(game_render_commands->screen_width, game_render_commands->screen_height);
    game_render_commands->camera = main_menu_state.main_camera;

    main_menu_state.adjust_entities_for_screen_resolution(game_render_commands->screen_width, game_render_commands->screen_height);
    // TODO: Fix Draw_Main_Menu_Stars
    //       this looks bad sometimes when the resolution is adjusted or something similar
    //       so I need to be careful!
    //
    //       However it should mostly be okay!
    {
        f32 brightness = clamp<f32>(normalized_sinf(Global_Engine()->global_elapsed_time), 0.75, 0.87f);
        render_commands_push_quad_ext(
            game_render_commands,
            rectangle_f32(-300, -300, game_render_commands->screen_width*2, game_render_commands->screen_height*2),
            color32u8(brightness*(32/2), brightness*(45/2), brightness*(80/2), 255),
            V2(0, 0), 0,
            BLEND_MODE_ALPHA);

        // background stars are slower
        auto bkg_slow_stars = main_menu_state.star_positions[0];
        auto bkg_faster_stars = main_menu_state.star_positions[1];
        // forward stars are moving more
        {
            // uh. I hope this looks fine.
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

            // Need to make these have different sizes.
            for (int i = 0; i < MAX_MAINMENU_OUTERSPACE_STARS; ++i) {
                auto r = rectangle_f32(bkg_slow_stars[i].x, bkg_slow_stars[i].y, 1, 1);
                auto r1 = rectangle_f32(bkg_faster_stars[i].x-30, bkg_faster_stars[i].y+20, 0.5, 0.5);
                auto r2 = rectangle_f32(bkg_faster_stars[i].x-5, bkg_faster_stars[i].y-45, 0.5, 0.5);

                render_commands_push_quad_ext(
                    game_render_commands,
                    r,
                    color32u8(230, 230, 255, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    game_render_commands,
                    r2,
                    color32u8(200, 200, 255, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );

                render_commands_push_quad_ext(
                    game_render_commands,
                    r1,
                    color32u8(100, 100, 245, 255),
                    V2(0, 0), 0,
                    BLEND_MODE_ALPHA
                );
            }
        }

        for (int i = 0; i < MAX_MAINMENU_SPARKLING_STARS; ++i) {
            main_menu_state.sparkling_stars[i].update(dt);
            main_menu_state.sparkling_stars[i].draw(game_render_commands, resources);
        }
    }

#ifndef RELEASE
    if (Input::is_key_pressed(KEY_U)) {
        Achievements::get(ACHIEVEMENT_ID_STAGE3)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE2)->report();
        Achievements::get(ACHIEVEMENT_ID_STAGE1)->report();
    }
    if (Input::is_key_pressed(KEY_I)) {
        Achievements::get(ACHIEVEMENT_ID_STAGE3)->_deunlock();
        Achievements::get(ACHIEVEMENT_ID_STAGE2)->_deunlock();
        Achievements::get(ACHIEVEMENT_ID_STAGE1)->_deunlock();
    }
    if (Input::is_key_pressed(KEY_O)) {
        main_menu_state.start_unlock_pet_cutscene(state);
    }
#endif

    if (Action::is_pressed(ACTION_MENU)) {
        if (this->state->ui_state == UI_STATE_STAGE_SELECT || main_menu_state.cutscene_active()) {
            
        } else {
            if (this->state->ui_state != UI_STATE_PAUSED) {
                switch_ui(UI_STATE_PAUSED);
            } else {
                switch_ui(UI_STATE_INACTIVE);
            }
        }
    }

    if (state->ui_state == UI_STATE_INACTIVE) {
        if (!main_menu_state.cutscene_active()) {
            main_menu_state.player.update(&main_menu_state, dt);
        }

        // I do not want the pets to move on screen until their unlock cutscene
        // is over, because otherwise something might look weird.
        if (main_menu_state.cutscene3.phase == 0) {
            for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
                main_menu_state.pets[i].update(&main_menu_state, dt);
            }
        }

        for (int i = 0; i < main_menu_state.clutter_poops.size; ++i) {
            main_menu_state.clutter_poops[i].update(dt);
        }
    }

    main_menu_state.cleanup_all_dead_poops();

    for (int i = 0; i < main_menu_state.portals.size; ++i) {
        auto& p = main_menu_state.portals[i];
        p.emitter_main.active   = p.visible;

        p.emitter_main.update(&main_menu_state.particle_pool, &main_menu_state.prng, dt);
        p.emitter_vortex.update(&main_menu_state.particle_pool, &main_menu_state.prng, dt);
    }
    main_menu_state.particle_pool.update(state, dt);

    for (int i = 0; i < main_menu_state.portals.size; ++i) {
        auto& p = main_menu_state.portals[i];

        p.update(&main_menu_state, dt);
        p.draw(&main_menu_state, game_render_commands, resources);
    }

    main_menu_state.particle_pool.draw(game_render_commands, resources);

    for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
        main_menu_state.pets[i].draw(&main_menu_state, game_render_commands, resources);
    }

    for (int i = 0; i < main_menu_state.clutter_poops.size; ++i) {
        main_menu_state.clutter_poops[i].draw(&main_menu_state, game_render_commands, resources);
    }

    main_menu_state.player.draw(&main_menu_state, game_render_commands, resources);
    handle_ui_update_and_render(ui_render_commands, dt);

    // Wrap player to edges
    {
        s32 screen_width = game_render_commands->screen_width;
        s32 screen_height = game_render_commands->screen_height;

        if ((main_menu_state.player.position.x) < 0)                 main_menu_state.player.position.x = screen_width;
        if ((main_menu_state.player.position.x) > screen_width) main_menu_state.player.position.x = 0;
        if ((main_menu_state.player.position.y) < 0)                 main_menu_state.player.position.y = screen_height;
        if ((main_menu_state.player.position.y) > screen_height) main_menu_state.player.position.y = 0;

        // wrap all pets to edges as well
        for (int i = 0; i < this->state->gameplay_data.unlocked_pets; ++i) {
            auto& pet = main_menu_state.pets[i];
            if ((pet.position.x) < 0)            pet.position.x = screen_width;
            if ((pet.position.x) > screen_width) pet.position.x = 0;
            if ((pet.position.y) < 0)            pet.position.y = screen_height;
            if ((pet.position.y) > screen_height)pet.position.y = 0;
        }
    }

    // do a fancy camera zoom in effect
    { //... NOTE: must resist the urge. To throw everything into coroutines...
        MainMenu_Stage_Portal* focus_portal = nullptr;
        auto circle_player = circle_f32(main_menu_state.player.position.x, main_menu_state.player.position.y, main_menu_state.player.scale.x);
        for (int i = 0; i < main_menu_state.portals.size; ++i) {
            auto& p = main_menu_state.portals[i];
            auto circle_portal = circle_f32(p.position.x, p.position.y, p.scale.x * 3.5f);

            if (!p.visible) continue;

            if (circle_f32_intersect(circle_portal, circle_player)) {
                focus_portal = &p;
                break;
            }

            p.triggered_level_selection = false;
            p.emitter_vortex.active = false;
        } 

        if (focus_portal) {
            if (!focus_portal->triggered_level_selection) {
                focus_portal->triggered_level_selection = true;

                focus_portal->emitter_vortex.reset();
                focus_portal->emitter_vortex.active = true;

                main_menu_state.stage_id_level_select = focus_portal->stage_id;
                switch_ui(UI_STATE_STAGE_SELECT);

                auto position = focus_portal->position;
                position *= 1.5f;

                camera_set_point_to_interpolate(&main_menu_state.main_camera, position, 1.5f);
            }
        } else {
            if (main_menu_state.last_focus_portal != nullptr) {
                V2 resolution = V2(game_render_commands->screen_width, game_render_commands->screen_height);
                if (!camera_already_interpolating_for(&main_menu_state.main_camera, V2(resolution.x/2, resolution.y/2), 1.0f)) {
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

    // screen messages
    if (state->ui_state == UI_STATE_INACTIVE) {
        // Screen_Message
        {
            if (main_menu_state.screen_messages.size > 0) {
                main_menu_state.screen_message_fade_t += dt;
            } else {
                main_menu_state.screen_message_fade_t -= dt;
            }

            main_menu_state.screen_message_fade_t = clamp<f32>(main_menu_state.screen_message_fade_t, 0.0f, 1.0f);

            auto font = resources->get_font(MENU_FONT_COLOR_WHITE);
            f32 text_scale = 4.0f;

            render_commands_push_quad_ext(
                ui_render_commands,
                rectangle_f32(0, 0, ui_render_commands->screen_width, ui_render_commands->screen_height),
                color32u8(0, 0, 0, main_menu_state.screen_message_fade_t * 150),
                V2(0, 0), 0,
                BLEND_MODE_ALPHA
            );

            // one message per "screen"
            if (main_menu_state.screen_messages.size > 0) {
                s32 message_index = 0;
                auto& message = main_menu_state.screen_messages[message_index];

                f32 text_width       = font_cache_text_width(font, message.text, text_scale);
                f32 text_height      = font_cache_text_height(font) * text_scale;
                V2  message_position = V2(ui_render_commands->screen_width/2 - text_width/2.0f, ui_render_commands->screen_height/2 - text_height/2);

                // TODO: fix alignment and visual look.
                switch (message.phase) {
                    case MAIN_MENU_SCREEN_MESSAGE_APPEAR: {
                        const f32 PHASE_MAX = 0.35f;
                        message.timer += dt;
                        f32 alpha = clamp<f32>(message.timer/PHASE_MAX, 0.0f, 1.0f);

                        render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1, alpha), BLEND_MODE_ALPHA);
                        if (message.timer >= PHASE_MAX) {
                            message.phase = MAIN_MENU_SCREEN_MESSAGE_WAIT_FOR_CONTINUE;
                            message.timer = 0.0f;
                        }
                    } break;
                    case MAIN_MENU_SCREEN_MESSAGE_WAIT_FOR_CONTINUE: {
                        render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1,1), BLEND_MODE_ALPHA);
                        if (Input::any_key_down() || Input::controller_any_button_down(Input::get_gamepad(0))) {
                            message.phase = MAIN_MENU_SCREEN_MESSAGE_DISAPPEAR;
                        }
                    } break;
                    case MAIN_MENU_SCREEN_MESSAGE_DISAPPEAR: {
                        const f32 PHASE_MAX = 0.25f;
                        message.timer += dt;
                        f32 alpha = clamp<f32>(1 - message.timer/PHASE_MAX, 0.0f, 1.0f);
                        render_commands_push_text(ui_render_commands, font, text_scale, message_position, message.text, color32f32(1,1,1,alpha), BLEND_MODE_ALPHA);

                        if (message.timer >= PHASE_MAX) {
                            message.timer = 0.0f;

                            main_menu_state.screen_messages.pop_and_swap(message_index);
                        }
                    } break;
                }
            }
        }

        // Unlock Pet Cutscene
        {
            auto& cutscene_state = main_menu_state.cutscene3;
            if (cutscene_state.phase != MAIN_MENU_UNLOCK_PET_CUTSCENE_PHASE_OFF) {
                cutscene_state.pet_spin_timer += dt;

                if (cutscene_state.pet_spin_timer >= 0.200f) {
                    cutscene_state.pet_spin_timer          = 0;
                    cutscene_state.pet_facing_direction   += 1;
                    cutscene_state.pet_facing_direction   %= 4;
                }
            }

            switch (cutscene_state.phase) {
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_PHASE_OFF:
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_COROUTINE_TASK: {} break;
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_IN_UNLOCK_BOX: {
                    const f32 PHASE_MAX = 0.55f/* + 0.15f */;
                    f32 fade_alpha = clamp<f32>((cutscene_state.timer-0.15) / PHASE_MAX, 0.0f, 1.0f);

                    render_commands_push_quad_ext(
                        ui_render_commands,
                        rectangle_f32(0, 0, resolution.x, resolution.y),
                        color32u8(2, 5, 15, 100 * fade_alpha),
                        V2(0, 0), 0,
                        BLEND_MODE_ALPHA
                    );

                    if (cutscene_state.timer >= PHASE_MAX) {
                        cutscene_state.phase = MAIN_MENU_UNLOCK_PET_CUTSCENE_POP_IN_PET;
                        cutscene_state.timer = 0;
                    } else {
                        cutscene_state.timer += dt;
                    }
                } break;
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_POP_IN_PET: {
                    render_commands_push_quad_ext(
                        ui_render_commands,
                        rectangle_f32(0, 0, resolution.x, resolution.y),
                        color32u8(2, 5, 15, 100),
                        V2(0, 0), 0,
                        BLEND_MODE_ALPHA
                    );

                    /*
                     * Yeah, I'm really writing this kind of timing code, thankfully it's only one place
                     * so I'm not really bothered by this, but there's not really a super duper convenient way
                     * of writing this in C++.
                     */
                    cutscene_state.timer += dt;
                    if (cutscene_state.timer >= 0.15f && cutscene_state.timer < 0.225f) {
                        const f32 length_of_fade_in   = (0.225f - 0.15f);
                        const f32 effective_fade_in_t = cutscene_state.timer - 0.15f;
                        const f32 alpha               = clamp<f32>(effective_fade_in_t / length_of_fade_in, 0.0f, 1.0f);

                        render_commands_push_quad_ext(
                            ui_render_commands,
                            rectangle_f32(0, 0, resolution.x, resolution.y),
                            color32u8(255, 255, 255, 255 * alpha),
                            V2(0, 0), 0,
                            BLEND_MODE_ALPHA
                        );
                    } else if (cutscene_state.timer >= 0.225f && cutscene_state.timer < 0.300f) {
                        // linger
                        const f32 length_of_fade_in   = (0.300f - 0.225f);
                        const f32 effective_fade_in_t = cutscene_state.timer - 0.225f;

                        render_commands_push_quad_ext(
                            ui_render_commands,
                            rectangle_f32(0, 0, resolution.x, resolution.y),
                            color32u8(255, 255, 255, 255),
                            V2(0, 0), 0,
                            BLEND_MODE_ALPHA
                        );
                    } else if (cutscene_state.timer >= 0.300f && cutscene_state.timer < 0.470f) {
                        // fade out
                        const f32 length_of_fade_in   = (0.470f - 0.300f);
                        const f32 effective_fade_in_t = cutscene_state.timer - 0.300f;
                        const f32 alpha               = clamp<f32>(effective_fade_in_t / length_of_fade_in, 0.0f, 1.0f);

                        render_commands_push_quad_ext(
                            ui_render_commands,
                            rectangle_f32(0, 0, resolution.x, resolution.y),
                            color32u8(255, 255, 255, 255 * (1.0f - alpha)),
                            V2(0, 0), 0,
                            BLEND_MODE_ALPHA
                        );
                    } else if (cutscene_state.timer >= 0.470f) {
                        cutscene_state.phase = MAIN_MENU_UNLOCK_PET_CUTSCENE_IDLE;
                        cutscene_state.timer = 0;
                    }
                } break;
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_IDLE: {
                    render_commands_push_quad_ext(
                        ui_render_commands,
                        rectangle_f32(0, 0, resolution.x, resolution.y),
                        color32u8(2, 5, 15, 100),
                        V2(0, 0), 0,
                        BLEND_MODE_ALPHA
                    );

                    {
                        auto& texture_atlas = resources->ui_texture_atlas;
                        auto box_width = 800;
                        auto box_height = 800;
                        render_commands_push_image_ext(
                            ui_render_commands,
                            graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id),
                            rectangle_f32(resolution.x/2 - box_width/2, resolution.y/2 - box_height/2, box_width, box_height), // TODO: fix this image, it's not very good looking but it's good enough for placeholdering
                            texture_atlas.get_subrect(resources->ui_rays_gradient),
                            color32f32(0.8,0.8,1, 0.9),
                            V2(0.5,0.5),
                            Global_Engine()->global_elapsed_time * 200.0f,
                            NO_FLAGS,
                            BLEND_MODE_ALPHA
                        );
                    }

                    {
                        render_commands_push_quad_ext(
                            ui_render_commands,
                            rectangle_f32(0, 0, resolution.x, 40),
                            color32u8(0, 0, 0, 255),
                            V2(0, 0), 0,
                            BLEND_MODE_ALPHA
                        );
                        render_commands_push_text(
                            ui_render_commands,
                            resources->get_font(MENU_FONT_COLOR_WHITE),
                            2, V2(10, 10),
                            string_literal("Pet Unlocked! Try it on 'Stage Select'!"),
                            color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA
                        );
                    }

                    // Draw pet spin around
                    {
                        s32       type      = (state->gameplay_data.unlocked_pets-1);
                        image_id* image_set = resources->pet_images[type];
                        image_id  image     = image_set[_remap_pet_facing_direction_to_sprite_index(cutscene_state.pet_facing_direction)];

                        // I know this segmenet gets copied and pasted frequently, but this literally could not be shorter in any capacity unfortunately
                        // and as this is a custom engine without any real UI workflow, well yeah this is kind of what happens.

                        // At least it looks nice, but I admit it would be nice to have some drag and drop.
                        {
                            auto image_object = graphics_assets_get_image_by_id(&resources->graphics_assets, image);
                            f32  image_scale  = 4;
                            V2   image_size   = V2(image_object->width, image_object->height);
                            V2   hover_offset = V2(0, sinf(Global_Engine()->global_elapsed_time * 2.852) * 5) * image_scale;

                            V2   position     = V2(resolution.x / 2, resolution.y / 2);
                            render_commands_push_image(
                                ui_render_commands,
                                image_object,
                                // magic numbers are just to align the sprite.
                                rectangle_f32(
                                    position.x - ((image_size.x*image_scale)) * 0.5f + hover_offset.x,
                                    position.y - ((image_size.y*image_scale)) * 0.5f + hover_offset.y,
                                    image_size.x*image_scale,
                                    image_size.y*image_scale
                                ),
                                RECTANGLE_F32_NULL,
                                color32f32(1.0, 1.0f, 1.0, 1.0f),
                                0,
                                BLEND_MODE_ALPHA
                            );
                        }
                    }

                    if (cutscene_state.timer >= 4.45f) {
                        cutscene_state.phase = MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_OUT_UNLOCK_BOX;
                        cutscene_state.timer = 0.0f;
                    } else {
                        cutscene_state.timer += dt;
                    }
                } break;
                case MAIN_MENU_UNLOCK_PET_CUTSCENE_FADE_OUT_UNLOCK_BOX: {
                    const f32 phase_length = 1.5f;
                    f32 alpha = 1.0f - clamp<f32>(cutscene_state.timer / phase_length, 0.0f, 1.0f);
                    {
                        render_commands_push_quad_ext(
                            ui_render_commands,
                            rectangle_f32(0, 0, resolution.x, resolution.y),
                            color32u8(2, 5, 15, 100 * alpha),
                            V2(0, 0), 0,
                            BLEND_MODE_ALPHA
                        );

                        {
                            auto& texture_atlas = resources->ui_texture_atlas;
                            auto box_width = 800;
                            auto box_height = 800;
                            render_commands_push_image_ext(
                                ui_render_commands,
                                graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id),
                                rectangle_f32(resolution.x/2 - box_width/2, resolution.y/2 - box_height/2, box_width, box_height), // TODO: fix this image, it's not very good looking but it's good enough for placeholdering
                                texture_atlas.get_subrect(resources->ui_rays_gradient),
                                color32f32(0.8,0.8,1, 0.9 * alpha),
                                V2(0.5,0.5),
                                Global_Engine()->global_elapsed_time * 200.0f,
                                NO_FLAGS,
                                BLEND_MODE_ALPHA
                            );
                        }

                        {
                            render_commands_push_quad_ext(
                                ui_render_commands,
                                rectangle_f32(0, 0, resolution.x, 40),
                                color32u8(0, 0, 0, 255 * alpha),
                                V2(0, 0), 0,
                                BLEND_MODE_ALPHA
                            );
                            render_commands_push_text(
                                ui_render_commands,
                                resources->get_font(MENU_FONT_COLOR_WHITE),
                                2, V2(10, 10),
                                string_literal("Pet Unlocked! Try it on 'Stage Select'!"),
                                color32f32(1, 1, 1, 1 * alpha), BLEND_MODE_ALPHA
                            );
                        }

                        {
                            s32       type      = (state->gameplay_data.unlocked_pets-1);
                            image_id* image_set = resources->pet_images[type];
                            image_id  image     = image_set[_remap_pet_facing_direction_to_sprite_index(cutscene_state.pet_facing_direction)];

                            // I know this segmenet gets copied and pasted frequently, but this literally could not be shorter in any capacity unfortunately
                            // and as this is a custom engine without any real UI workflow, well yeah this is kind of what happens.

                            // At least it looks nice, but I admit it would be nice to have some drag and drop.
                            {
                                auto image_object = graphics_assets_get_image_by_id(&resources->graphics_assets, image);
                                f32  image_scale  = 4;
                                V2   image_size   = V2(image_object->width, image_object->height);
                                V2   hover_offset = V2(0, sinf(Global_Engine()->global_elapsed_time * 2.852) * 5) * image_scale;

                                V2   position     = V2(resolution.x / 2, resolution.y / 2);
                                render_commands_push_image(
                                    ui_render_commands,
                                    image_object,
                                    // magic numbers are just to align the sprite.
                                    rectangle_f32(
                                        position.x - ((image_size.x*image_scale)) * 0.5f + hover_offset.x,
                                        position.y - ((image_size.y*image_scale) * 0.5f) + hover_offset.y,
                                        image_size.x*image_scale,
                                        image_size.y*image_scale
                                    ),
                                    RECTANGLE_F32_NULL,
                                    color32f32(1.0, 1.0f, 1.0, alpha),
                                    0,
                                    BLEND_MODE_ALPHA
                                );
                            }
                        }
                    }

                    if (cutscene_state.timer >= (phase_length + 1.0f)) {
                        camera_set_point_to_interpolate(&main_menu_state.main_camera, V2(resolution.x/2, resolution.y/2), 1.0);
                        cutscene_state.phase  = MAIN_MENU_UNLOCK_PET_CUTSCENE_PHASE_OFF;
                        cutscene_state.triggered = false;
                    } else {
                        cutscene_state.timer += dt;
                    }
                } break;
            }
        }
    }

    Transitions::update_and_render(ui_render_commands, dt);

    game_render_commands->should_clear_buffer = true;
    game_render_commands->clear_buffer_color = color32u8(32, 45, 80, 255);
    camera_update(&main_menu_state.main_camera, dt);
}

// end main menu code

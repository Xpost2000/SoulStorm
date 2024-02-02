#include "action_mapper.h"

#include "entity.h"
#include "input.h"

#include "game_state.h"

#include "engine.h"
// Timer

Timer::Timer() {}
Timer::Timer(f32 hit) :
    running(false), t(0.0f), hit_t(hit), max_t(hit) {
    
}

void Timer::start() {
    if (t >= hit_t) {
        // nothing
    } else {
        running = true;
    }
}

void Timer::trigger_immediate() {
    t = hit_t;
    running = true; // so that way it can be triggered().
}

void Timer::update(f32 dt) {
    if (running) {
        t += dt;
    }
}

void Timer::stop() {
    running = false; 
}

void Timer::reset() {
    running = false;
    t = 0;
}

bool Timer::triggered() {
    if (running)  {
        if (t >= hit_t) {
            t = 0.0f;
            running = false;
            return true;
        }
    }

    return false;
}

float Timer::percentage() {
    return (t / hit_t);
}

// Entity Base
f32 Entity::hp_percentage() {
    return (f32)hp / max_hp;
}
bool Entity::touching_left_border(const Play_Area& play_area, bool as_point) {
    if (as_point)
        return (position.x < 0);
    else
        return (position.x - scale.x < 0);
}

bool Entity::touching_right_border(const Play_Area& play_area, bool as_point) {
    if (as_point)
        return (position.x > play_area.width);
    else
        return (position.x + scale.x > play_area.width);
}

bool Entity::touching_top_border(const Play_Area& play_area, bool as_point) {
    if (as_point)
        return (position.y < 0);
    else
        return (position.y - scale.y < 0);
}

bool Entity::touching_bottom_border(const Play_Area& play_area, bool as_point) {
    if (as_point)
        return (position.y > play_area.height);
    else
        return (position.y + scale.y > play_area.height);
}

bool Entity::clamp_to_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area)) {
        velocity.x = 0;
        last_position.x =
            position.x = scale.x;
        return true;
    }
    return false;
}

bool Entity::clamp_to_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area)) {
        velocity.x = 0;
        last_position.x =
            position.x = play_area.width - scale.x;
        return true;
    }
    return false;
}

bool Entity::clamp_to_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area)) {
        velocity.y = 0;
        last_position.y =
            position.y = scale.y;
        return true;
    }
    return false;
}

bool Entity::clamp_to_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area)) {
        velocity.y = 0;
        last_position.y =
            position.y = play_area.height - scale.y;
        return true;
    }
    return false;
}

bool Entity::wrap_from_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area, true)) {
        last_position.x =
            position.x = play_area.width;
        return true;
    }
    return false;
}

bool Entity::wrap_from_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area, true)) {
        last_position.x =
            position.x = 0;
        return true;
    }
    return false;
}

bool Entity::wrap_from_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area, true)) {
        last_position.y =
            position.y = play_area.height;
        return true;
    }
    return false;
}

bool Entity::wrap_from_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area, true)) {
        last_position.y =
            position.y = 0;
        return true;
    }
    return false;
}

bool Entity::damage(s32 dmg) {
    if (invincibility_time.running) {
        return false;
    }
    hp -= dmg;
    if (hp <= 0) die = true;

    return true;
}

bool Entity::heal(s32 hp) {
    hp += hp;
    if (hp > 0) die = false;

    return true;
}

void Entity::begin_invincibility(bool visual_flash, f32 timer) {
    invincibility_show_flashing = visual_flash;
    invincibility_time          = Timer(timer);
    invincibility_time.start();
    flashing = false;
    invincibility_time_flash_period.reset();
}
void Entity::end_invincibility() {
    flashing = false;
    invincibility_time.stop();
}

bool Entity::kill() {
    _debugprintf("trying to die");
    return damage(9999999); // lol
}

bool Entity::wrap_border(s32 edge, const Play_Area& play_area) {
    switch (edge) {
        case 0: return wrap_from_top_border(play_area); break;
        case 1: return wrap_from_bottom_border(play_area); break;
        case 2: return wrap_from_left_border(play_area); break;
        case 3: return wrap_from_right_border(play_area); break;
    }

    return false;
}

bool Entity::clamp_border(s32 edge, const Play_Area& play_area) {
    switch (edge) {
        case 0: return clamp_to_top_border(play_area); break;
        case 1: return clamp_to_bottom_border(play_area); break;
        case 2: return clamp_to_left_border(play_area); break;
        case 3: return clamp_to_right_border(play_area); break;
    }

    return false;
}

bool Entity::touching_border(s32 edge, const Play_Area& play_area, bool as_point) {
    switch (edge) {
        case 0: return touching_top_border(play_area, as_point); break;
        case 1: return touching_bottom_border(play_area, as_point); break;
        case 2: return touching_left_border(play_area, as_point); break;
        case 3: return touching_right_border(play_area, as_point); break;
    }

    return false;
}

void Entity::handle_play_area_edge_behavior(const Play_Area& play_area) {
    s32 edge_behavior_top    = edge_top_behavior_override;
    s32 edge_behavior_bottom = edge_bottom_behavior_override;
    s32 edge_behavior_left   = edge_left_behavior_override;
    s32 edge_behavior_right  = edge_right_behavior_override;

    if (edge_behavior_top == -1)    edge_behavior_top    = play_area.edge_behaviors[0];
    if (edge_behavior_bottom == -1) edge_behavior_bottom = play_area.edge_behaviors[1];
    if (edge_behavior_left == -1)   edge_behavior_left   = play_area.edge_behaviors[2];
    if (edge_behavior_right == -1)  edge_behavior_right  = play_area.edge_behaviors[3];

    const s32 edges[4] = { edge_behavior_top, edge_behavior_bottom, edge_behavior_left, edge_behavior_right };

    for (s32 edge_index = 0; edge_index < 4; ++edge_index) {
        s32 edge = edges[edge_index];

        switch (edge) {
            case PLAY_AREA_EDGE_DEADLY:
            case PLAY_AREA_EDGE_BLOCKING: {
                if (clamp_border(edge_index, play_area))
                    if (edge == PLAY_AREA_EDGE_DEADLY) {
                        kill();
                    }
            } break;
            case PLAY_AREA_EDGE_WRAPPING: {
                wrap_border(edge_index, play_area);
            } break;
            case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
        }
    }
}

void Entity::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    if (!visible)
        return;

    auto r = get_rect();

    V2 interpolated_position = V2(
        lerp_f32(last_position.x, position.x, state->gameplay_data.fixed_tickrate_remainder),
        lerp_f32(last_position.y, position.y, state->gameplay_data.fixed_tickrate_remainder)
    );

    r.x = interpolated_position.x - scale.x;
    r.y = interpolated_position.y - scale.y;

    if (sprite.id.index != 0) {
        // NOTE: entity updates are responsible for actually
        //       animating their own sprites!
        auto  sprite_object     = graphics_get_sprite_by_id(&resources->graphics_assets, sprite.id);
        auto  sprite_frame      = sprite_get_frame(sprite_object, sprite.frame);
        auto  sprite_img        = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
        V2    sprite_image_size = V2(sprite_img->width, sprite_img->height);
        float facing_angle      = sprite.angle_offset;

        // NOTE: specifically needed for bullet disks which must be rotated to look correct.
        // I would personally... Like to not have this here, but it's not a big deal imo.
        if (resources->sprite_id_should_be_rotated(sprite.id)) {
            auto velocity_direction = velocity.normalized();
            facing_angle = radians_to_degrees(
                atan2(velocity_direction.y,
                      velocity_direction.x)
            );
        }


        // NOTE: provided sprites are 32x32, so that will be the "assumed" scale for my development purposes
        // although obviously none of the code here requires that. It's just my convention.
        sprite_image_size.x *= sprite.scale.x; sprite_image_size.y *= sprite.scale.y;

        for (s32 trail_ghost_index = 0; trail_ghost_index < trail_ghost_count; ++trail_ghost_index) {
            auto& ghost = trail_ghosts[trail_ghost_index];
            V2 sprite_position = V2(
                ghost.position.x + (sprite.offset.x - sprite_image_size.x/2),
                ghost.position.y + (sprite.offset.y - sprite_image_size.y/2)
            );

            auto modulation = trail_ghost_modulation;
            modulation.a *= (ghost.alpha/trail_ghost_max_alpha);

            render_commands_push_image_ext(
                render_commands,
                sprite_img,
                rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                sprite_frame->source_rect,
                modulation,
                V2(0.5, 0.5),
                facing_angle,
                0,
                BLEND_MODE_ALPHA
            );
        }

        V2 sprite_position = V2(
            interpolated_position.x + (sprite.offset.x - sprite_image_size.x/2),
            interpolated_position.y + (sprite.offset.y - sprite_image_size.y/2)
        );

        render_commands_push_image_ext(
            render_commands,
            sprite_img,
            rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
            sprite_frame->source_rect,
            sprite.modulation,
            V2(0.5, 0.5),
            facing_angle,
            0,
            BLEND_MODE_ALPHA
        );

        // This would hurt in hardware because I'm not planning
        // to optimize the hardware renderers that much, which means I'm incuring
        // the wrath of a state change...
        if (flashing) {
            render_commands_push_image(
                render_commands,
                sprite_img,
                rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                sprite_frame->source_rect,
                sprite.modulation,
                0,
                BLEND_MODE_ADDITIVE
            );
        }
    } else {
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color32u8(0, 0, 255 * flashing, 255),
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );

        render_commands_push_image(render_commands,
                                   graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                                   rectangle_f32(position.x - scale.x, position.y - scale.x, scale.x*2, scale.x*2),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1.0, 0, 1.0, 0.5f),
                                   0,
                                   BLEND_MODE_ALPHA);


        // center point
        render_commands_push_quad(
            render_commands,
            rectangle_f32(position.x - 1, position.y-1, 2, 2),
            color32u8(255, 0, 0, 255),
            BLEND_MODE_ALPHA
        );
    }

    if (DebugUI::enabled()) {
        // grazing "volume"
#if 0
        render_commands_push_image(render_commands,
                                   graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                                   rectangle_f32(position.x - PLAYER_GRAZE_RADIUS, position.y - PLAYER_GRAZE_RADIUS, PLAYER_GRAZE_RADIUS*2, PLAYER_GRAZE_RADIUS*2),
                                   RECTANGLE_F32_NULL,
                                   color32f32(0.0, 1.0f, 0.0, 0.1f),
                                   0,
                                   BLEND_MODE_ALPHA);
#endif

        // hitbox
        render_commands_push_quad_ext(
            render_commands,
            rectangle_f32(r.x, r.y, r.w, r.h),
            color32u8(0, 0, 255, 100),
            V2(0.5, 0.5), 0,
            BLEND_MODE_ALPHA
        );

        render_commands_push_quad(
            render_commands,
            rectangle_f32(position.x - 1, position.y-1, 2, 2),
            color32u8(255, 0, 0, 255),
            BLEND_MODE_ALPHA
        );
    }
}

void Entity::update_ghost_trails(f32 dt) {
    if (trail_ghost_record_timer <= 0.0f && trail_ghost_count < trail_ghost_limit) {
        auto& ghost = trail_ghosts[trail_ghost_count++];
        ghost.position = position;
        ghost.alpha    = trail_ghost_max_alpha;
        trail_ghost_record_timer = trail_ghost_record_timer_max;
    } else {
        trail_ghost_record_timer -= dt;
    }

    for (s32 trail_ghost_index = 0; trail_ghost_index < trail_ghost_count; ++trail_ghost_index) {
        auto& ghost = trail_ghosts[trail_ghost_index];
        ghost.alpha -= dt;

        if (ghost.alpha <= 0.0f) {
            trail_ghosts[trail_ghost_index] = trail_ghosts[--trail_ghost_count];
            continue;
        }
    }
}

void Entity::handle_out_of_bounds_behavior(const Play_Area& play_area, f32 dt) {
    if (!allow_out_of_bounds_survival && !play_area.is_inside_logical(get_rect())) {
        cleanup_time.start();
        cleanup_time.update(dt);

        if (cleanup_time.triggered()) {
            // mark for deletion / death
            die = true;
        }
    } else {
        cleanup_time.stop();
        cleanup_time.reset();
    }
}

void Entity::handle_invincibility_behavior(f32 dt) {
    if (invincibility_time.running) {
        invincibility_time_flash_period.start();
        if (invincibility_show_flashing && invincibility_time_flash_period.triggered()) {
            flashing ^= true;
            invincibility_time_flash_period.reset();
        }
        invincibility_time.triggered();
    } else {
        flashing = false;
    }
    invincibility_time_flash_period.update(dt);
    invincibility_time.update(dt);
}

bool Entity::attack() {
    if (!firing) {
        firing = true;
        firing_t = 0;
    }

    return firing_t <= 0.0f;
}

void Entity::stop_attack() {
    firing = false;
}

void Entity::update_firing_behavior(f32 dt) {
    if (firing) {
        if (firing_t <= 0.0f) {
            firing_t = firing_cooldown;
        }
    } else {
    }

    firing_t -= dt;
}

// I'm fairly confident this could be vectorized with SSE,
// considering this update loop was designed for threading I don't see why
// it couldn't, but it would take time that I would not like to spend to rewrite
// at least some parts of it as SSE code...
void Entity::update(Game_State* state, f32 dt) {
    if (!visible)
        return;

    const auto& play_area = state->gameplay_data.play_area;

    update_ghost_trails(dt);
    handle_out_of_bounds_behavior(play_area, dt);
    handle_invincibility_behavior(dt);
    update_firing_behavior(dt);

    {
        float velocity_mag = velocity.magnitude();
        if (velocity_mag > fabs(maximum_speed)) {
            velocity_mag = fabs(maximum_speed);

            auto velocity_direction = velocity.normalized();
            auto new_velocity       = velocity_direction * velocity_mag;
            velocity                = new_velocity;
        }
    }

    last_position       = position;
    position           += velocity * dt;
    t_since_spawn      += dt;

    for (unsigned particle_emitter_index = 0; particle_emitter_index < ENTITY_MAX_PARTICLE_EMITTERS; ++particle_emitter_index) {
        auto& emitter = emitters[particle_emitter_index];
        emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng, dt);
    }
}

rectangle_f32 Entity::get_rect() {
    return rectangle_f32(
        (position.x + relative_position.x) - scale.x,
        (position.y + relative_position.y) - scale.y,
        scale.x*2,
        scale.y*2
    );
}

// Cosmetic Pet Actor
void Cosmetic_Pet::update(Game_State* state, f32 dt) {
    if (!visible) {
        return;
    }

    Entity::update(state, dt); 
}

void Cosmetic_Pet::set_id(s32 id, Game_Resources* resources) {
    if (id == GAME_PET_ID_NONE) {
        visible = false;
    } else {
        visible = true;
    }

    sprite = sprite_instance(resources->pet_sprites[id]);
    this->id = id;
}

// PlayerActor
s32 Player::currently_grazing(Game_State* state) {
    s32 grazed_bullets = 0;
    for (s32 bullet_index = 0; bullet_index < state->gameplay_data.bullets.size; ++bullet_index) {
        auto& b = state->gameplay_data.bullets[bullet_index];

        if (b.die) {
            continue;
        }

        if (b.source_type == BULLET_SOURCE_PLAYER) {
            continue;
        }

        auto distance = V2_distance_sq(b.position, position);
        if (distance <= (PLAYER_GRAZE_RADIUS*PLAYER_GRAZE_RADIUS)) {
            grazed_bullets += 1;
        }
    }

    return grazed_bullets;
}

void Player::handle_grazing_behavior(Game_State* state, f32 dt) {
    s32 grazing = currently_grazing(state);

    if (grazing) {
        if (grazing_delay <= 0.0f) {
            f32 score_modifier = get_grazing_score_modifier(grazing);

            if (grazing_award_timer <= 0.0f) {
                // award score to the game
                state->gameplay_data.notify_score(
                    score_modifier * GRAZING_DEFAULT_SCORE_AWARD,
                    true
                );
                // sound?
                grazing_award_timer = GRAZING_SCORE_AWARD_DELAY;
            } else{
                grazing_award_timer -= dt;
            }

            if (grazing_award_score_pickup_timer <= 0.0f) {
                f32 angle_degrees   = random_ranged_float(&state->gameplay_data.prng, 0.0f, 360.0f); 
                f32 radius_variance = random_ranged_float(&state->gameplay_data.prng, 0.78f, 1.125f);

                V2 pickup_position_target = position + (V2_direction_from_degree(angle_degrees) * GRAZING_SCORE_FIRE_RADIUS * radius_variance);

                Pickup_Entity pickup = pickup_score_entity(
                    state,
                    position,
                    pickup_position_target,
                    (GRAZING_DEFAULT_SCORE_AWARD + 20) * score_modifier
                );

                state->gameplay_data.add_pickup_entity(pickup);
                grazing_award_score_pickup_timer = GRAZING_SCORE_AWARD_PICKUP_DELAY;
            } else {
                grazing_award_score_pickup_timer -= dt;
            }


            // Tint the led towards grazing.
            {
                // direct access
                // NOTE: the stage will reset the color every frame anyway.
                auto& led_state = state->led_state;
                f32 effective_t = clamp<f32>(time_spent_grazing / 2.25, 0.0f, 1.0f);
                led_state.primary_color.r = lerp_f32(led_state.primary_color.r, 30,  effective_t);
                led_state.primary_color.g = lerp_f32(led_state.primary_color.g, 250, effective_t);
                led_state.primary_color.b = lerp_f32(led_state.primary_color.b, 40,  effective_t);
            }
            time_spent_grazing += dt;
        } else{
            grazing_delay -= dt;
        }
    } else {
        grazing_award_timer = 0.0f;
        grazing_delay       = PLAYER_DEFAULT_GRAZING_DELAY;
        time_spent_grazing  = 0.0f;
    }
}

f32 Player::get_grazing_score_modifier(s32 amount) {
    f32 base = 1.0f + clamp<f32>(time_spent_grazing / 3.5f, 0.0f, 2.5f);
    base += clamp<f32>(amount / 10.0f, 0.0f, 1.5f);
    return base;
}

void Player::update(Game_State* state, f32 dt) {
    if (!visible) {
        auto& emitter = emitters[0];
        emitter.active = false;
        return;
    }

    const auto& play_area    = state->gameplay_data.play_area;
    const auto& input_packet = state->gameplay_data.current_input_packet;
    auto pet_data            = game_get_pet_data(state->gameplay_data.selected_pet);
    // unfortunately the action mapper system doesn't exist
    // here like it did in the last project, so I'll have to use key inputs
    // and gamepad power.
    bool firing   = input_packet.actions & BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_ACTION_BIT);
    bool focusing = input_packet.actions & BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_FOCUS_BIT);
    bool use_bomb = input_packet.actions & BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_USE_BOMB_BIT);

    under_focus = focusing;

    V2 axes = gameplay_frame_input_packet_quantify_axes(input_packet);
    float UNIT_SPEED = ((under_focus) ? 225 : 325) * pet_data->speed_modifier;

    velocity.x = axes[0] * UNIT_SPEED;
    velocity.y = axes[1] * UNIT_SPEED;

    Entity::update(state, dt);

    handle_play_area_edge_behavior(play_area);

    // Particle Emitter
    {
        auto& emitter = emitters[0];
        emitter.active = true;
        emitter.sprite = sprite_instance(state->resources->projectile_sprites[PROJECTILE_SPRITE_SPARKLING_STAR]);

        auto r = get_rect();
        f32 left   = r.x;
        f32 bottom = r.y + r.h;

        emitter.scale  = 1.0f;
        emitter.emit_per_emission = 8;
        emitter.lifetime = 0.65f;
        emitter.velocity_x_variance = V2(-10, 50);
        emitter.velocity_y_variance = V2(-10, 50);
        emitter.acceleration_x_variance = V2(0, 10);
        emitter.acceleration_y_variance = V2(0, 20);
        emitter.lifetime_variance   = V2(-0.1f, 0.7f);
        emitter.emission_max_timer = 0.045f;
        emitter.shape = particle_emit_shape_line(V2(left, bottom), V2(left + r.w*1.5f, bottom));
    }

    // Sprite animation
    {
        s32 frame_start = 0;
        s32 frame_end   = 1;

        // Procedural animation components
        s32 sign = sign_f32(axes[0]);
        {
            const f32 MINIMUM_MAGNITUDE_TO_CONSIDER_LEANING                    = 0.360f;
            const f32 MINIMUM_ADDITIONAL_MAGNITUDE_TO_CONSIDER_ROTATON_LEANING = 0.12f;
            const f32 MINIMUM_MAGNITUDE_TO_CONSIDER_ROTATION_LEANING           = MINIMUM_MAGNITUDE_TO_CONSIDER_LEANING + MINIMUM_ADDITIONAL_MAGNITUDE_TO_CONSIDER_ROTATON_LEANING;
            const f32 MAX_ANGLE_LEAN                                           =
                (under_focus) ?
                27.5f :
                45.5f;
            f32       horizontal_axis_magnitude                                = fabs(axes[0]);

            // angle
            {
                const f32 effective_t = clamp<f32>(sprite.angle_offset / (MAX_ANGLE_LEAN*0.90), 0.0f, 1.0f);
                f32 decay_factor = lerp_f32(0.020f, 0.170f, effective_t);
                sprite.angle_offset *= pow(decay_factor, dt);
            }

            if (horizontal_axis_magnitude >= MINIMUM_MAGNITUDE_TO_CONSIDER_LEANING) {
                if (sign == -1) {
                    frame_start = 1;
                    frame_end = 2;
                } else if (sign == 1) {
                    frame_start = 2;
                    frame_end = 3;
                }


                if (horizontal_axis_magnitude >= (MINIMUM_MAGNITUDE_TO_CONSIDER_ROTATION_LEANING)) {
                    sprite.angle_offset = ((horizontal_axis_magnitude - MINIMUM_MAGNITUDE_TO_CONSIDER_ROTATION_LEANING) * sign) * MAX_ANGLE_LEAN;
                }
            }

            sprite.offset.y = sinf(t_since_spawn * 0.775) * 6.5 + 8; // NOTE: +7 is to adjust visual location to make the hit box look less "unintuitive"
        }

        sprite.animate(
            &state->resources->graphics_assets,
            dt,
            0.035,
            frame_start,
            frame_end
        );
    }

    switch (pet_data->attack_pattern_id) {
        // for now I'll only use one attack pattern, if I want more... I'll
        // think about it.
        default: {
            firing_cooldown = (under_focus) ? (DEFAULT_FIRING_COOLDOWN/2) : DEFAULT_FIRING_COOLDOWN;

            if (firing) {
                // okay these are normal real bullets
                if (attack()) {
                    auto resources = state->resources;

                    state->set_led_target_color_anim(
                        color32u8(255, 0, 0, 255),
                        DEFAULT_FIRING_COOLDOWN/4,
                        false,
                        true
                    );
                    Audio::play(
                        resources->random_attack_sound(
                            &state->gameplay_data.prng
                        )
                    );

                    controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
                    if (under_focus) {
                        spawn_bullet_arc_pattern1(state, position, 3, 15, V2(5, 5), V2(0, -1), 1000.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_DISK);
                    } else {
                        // spawn_bullet_line_pattern1(state, position, 1, 20.0f, V2(5, 5), V2(0, -1), 1550.0f, BULLET_SOURCE_PLAYER);
                        spawn_bullet_arc_pattern1(state, position, 3, 45, V2(5, 5), V2(0, -1), 650.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_ELECTRIC);
                    }
                } else {
                }
            } else {
                stop_attack();
            }

        } break;
    }

    if (use_bomb) {
        state->gameplay_data.queue_bomb_use = true;
    }
}

// BulletEntity
void Bullet::handle_lifetime(f32 dt) {
    if (lifetime.t == -1) {
        // live until killed by something else
    } else {
        lifetime.start();
        lifetime.update(dt);

        // if I add some special effects like a nice fade out.
        // would also be cool to add.
        if (lifetime.triggered()) {
            die = true;
        }
    }
}

void Bullet::handle_movement(Game_State* state, f32 dt) {
    if (velocity_function) {
        velocity = V2(0, 0);
        velocity_function(this, state, dt);
    } else {
        velocity += acceleration * dt;
    }
}

void Bullet::update(Game_State* state, f32 dt) {
    if (!visible)
        return;

    handle_lifetime(dt);
    handle_movement(state, dt);

    sprite.animate(
        &state->resources->graphics_assets,
        dt,
        0.035
    );

    Entity::update(state, dt);
}

void Bullet::reset_movement() {
    velocity_function = nullptr;
    velocity          = V2(0, 0);
    acceleration      = V2(0, 0);
}

// Enemy_Entity

void Enemy_Entity::update(Game_State* state, f32 dt) {
    if (!visible)
        return;

    const auto& play_area = state->gameplay_data.play_area;

    auto rect = get_rect();

    if (!play_area.is_inside_logical(rect)) {
        outside_boundaries_lifetime_timer.start();
        outside_boundaries_lifetime_timer.update(dt);

        if (outside_boundaries_lifetime_timer.triggered()) {
            die = true;
        }
    } else {
        outside_boundaries_lifetime_timer.stop();
        outside_boundaries_lifetime_timer.reset();
    }

    if (velocity_function) {
        velocity = V2(0, 0);
        velocity_function(this, state, dt);
    } else {
        velocity += acceleration * dt;
    }

    Entity::update(state, dt);
    handle_play_area_edge_behavior(play_area);
}

void Enemy_Entity::try_and_fire(Game_State* state, f32 dt) {
    if (attack()) {
        if (on_fire_function) {
            on_fire_function(this, state, dt);
        }
    }
}

void Enemy_Entity::reset_movement() {
    velocity_function = nullptr;
    velocity          = V2(0, 0);
    acceleration      = V2(0, 0);
}

// Hazard Warning
Hazard_Warning::Hazard_Warning(f32 amount_of_time_for_warning)
    : warning_flash_timer(amount_of_time_for_warning),
      show_flash_timer(DEFAULT_SHOW_FLASH_WARNING_TIMER),
      flash_warning_times(0),
      presenting_flash(false)
{
    
}

Hazard_Warning::Hazard_Warning() {
    
}

void Hazard_Warning::update(f32 dt) {
    if (flash_warning_times < DEFAULT_SHOW_FLASH_WARNING_TIMES) {
        if (!presenting_flash) {
            warning_flash_timer.start();

            if (warning_flash_timer.triggered()) {
                presenting_flash = true;
            }
        } else {
            show_flash_timer.start();

            if (show_flash_timer.triggered()) {
                // also play a sound.

                presenting_flash     = false;
                flash_warning_times += 1;

                warning_flash_timer.reset();
                show_flash_timer.reset();

            }
        }
    }

    warning_flash_timer.update(dt);
    show_flash_timer.update(dt);
}

bool Hazard_Warning::finished_presenting() {
    return flash_warning_times >= DEFAULT_SHOW_FLASH_WARNING_TIMES;
}

// Explosion Hazard
Explosion_Hazard::Explosion_Hazard() {}

Explosion_Hazard::Explosion_Hazard(V2 position, f32 radius, f32 amount_of_time_for_warning, f32 time_until_explosion)
    : position(position),
      radius(radius),
      warning(amount_of_time_for_warning),
      explosion_timer(time_until_explosion),
      exploded(false)
{

}

void Explosion_Hazard::update(Game_State* state, f32 dt) {
    if (warning.finished_presenting()) {
        explosion_timer.start();

        if (explosion_timer.triggered()) {
            // a big bang!
            exploded = true;
        }
    } else {
        on_presenting_flash_events = false;
        if (warning.presenting_flash) {
            // played sound
            if (!on_presenting_flash_events) {
                on_presenting_flash_events = true;
            }
        }
    }

    warning.update(dt);
    explosion_timer.update(dt);
}

void Explosion_Hazard::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    bool show_explosion_warning = warning.finished_presenting();

    if (show_explosion_warning) {
        render_commands_push_image(render_commands,
                                   graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                                   rectangle_f32(position.x - radius, position.y - radius, radius*2, radius*2),
                                   RECTANGLE_F32_NULL,
                                   color32f32(0, 0, 0, 1),
                                   0,
                                   BLEND_MODE_ALPHA);

        f32 explosion_percentage = explosion_timer.percentage();
        f32 adjusted_radius      = radius * explosion_percentage;
        render_commands_push_image(render_commands,
                                   graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                                   rectangle_f32(position.x - adjusted_radius, position.y - adjusted_radius, adjusted_radius*2, adjusted_radius*2),
                                   RECTANGLE_F32_NULL,
                                   color32f32(1, 0, 0, explosion_percentage),
                                   0,
                                   BLEND_MODE_ALPHA);
    } else {
        if (warning.presenting_flash) {
            render_commands_push_text(render_commands,
                                      resources->get_font(MENU_FONT_COLOR_GOLD),
                                      2, position,
                                      string_literal("!!"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
        }
    }
}

// Laser_Hazard
Laser_Hazard::Laser_Hazard(float position, float radius, int direction, float amount_of_time_for_warning, float how_long_to_live)
    : position(position),
      direction(direction),
      warning(amount_of_time_for_warning),
      lifetime(how_long_to_live),
      radius(radius)
{
    
}

Laser_Hazard::Laser_Hazard() {
    
}

rectangle_f32 Laser_Hazard::get_rect(const Play_Area* area) {
    f32 adjusted_position = position - radius;
    switch (direction) {
        case LASER_HAZARD_DIRECTION_HORIZONTAL: {
            return rectangle_f32(0, adjusted_position, area->width, radius*2);
        } break;
        case LASER_HAZARD_DIRECTION_VERTICAL: {
            return rectangle_f32(adjusted_position, 0, radius*2, area->height);
        } break;
    }

    // ?
    return RECTANGLE_F32_NULL;
}

bool Laser_Hazard::ready() {
    return warning.finished_presenting();
}

void Laser_Hazard::update(Game_State* state, f32 dt) {
    if (warning.finished_presenting()) {
        lifetime.start();
        // default update
        position += velocity * dt;

        if (lifetime.t == -1) {
            // live until killed by something else
        } else {
            lifetime.update(dt);

            if (lifetime.triggered()) {
                die = true;
            }
        }
    } else {
        on_presenting_flash_events = false;
        if (warning.presenting_flash) {
            if (!on_presenting_flash_events) {
                on_presenting_flash_events = true;
            }
        }
    }

    warning.update(dt);
}

void Laser_Hazard::draw(Game_State* state, struct render_commands* render_commands, Game_Resources* resources) {
    const auto& play_area = state->gameplay_data.play_area;
    auto        rectangle = get_rect(&play_area);

    if (!ready()) {
        if (warning.presenting_flash) {
            render_commands_push_quad(
                render_commands,
                rectangle,
                color32u8(0, 0, 0, 64),
                BLEND_MODE_ALPHA);
            render_commands_push_text(render_commands,
                                      resources->get_font(MENU_FONT_COLOR_GOLD),
                                      2, V2(rectangle.x + rectangle.w/2, rectangle.y + rectangle.h/2),
                                      string_literal("!!!"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
        } else {
        
        }
    } else {
        render_commands_push_quad(render_commands, rectangle, color32u8(255, 0, 0, 255), BLEND_MODE_ALPHA);
    }
}

// pickup entity
Pickup_Entity pickup_entity_generic(Game_State* state, s32 type, V2 start, V2 end, s32 value) {
    // clamp all pickups to be within game boundaries
    {
        auto& play_area = state->gameplay_data.play_area;
        start.x = clamp<f32>(start.x, 0.0f, play_area.width);
        end.x   = clamp<f32>(end.x, 0.0f, play_area.width);
        start.y = clamp<f32>(start.y, 0.0f, play_area.height);
        end.y   = clamp<f32>(end.y, 0.0f, play_area.height);
    }

    Pickup_Entity result;
    result.position = start;
    result.last_position = result.position;
    result.scale    = V2(5, 10);
    result.type = type;
    result.value = value;
    result.animation_start_position = start;
    result.animation_end_position = end;
    result.animation_t = 0.0f;
    return result;
}

Pickup_Entity pickup_score_entity(Game_State* state, V2 start, V2 end, s32 value) {
    Pickup_Entity score_entity = pickup_entity_generic(state, PICKUP_SCORE, start, end, value);
    score_entity.sprite = sprite_instance(state->resources->point_pickup_sprite);
    score_entity.sprite.scale = V2(0.60f, 0.60f);
    score_entity.sprite.modulation = color32f32(255.0f / 255.0f, 215.0f / 255.0f, 0.0f, 1.0f);
    // unimplemented("pick_score_entity not needed yet?");
    return score_entity;
}

Pickup_Entity pickup_attack_power_entity(Game_State* state, V2 start, V2 end, s32 value) {
    Pickup_Entity attack_power_entity = pickup_entity_generic(state, PICKUP_ATTACKPOWER, start, end, value);
    unimplemented("pick_attack_power_entity not needed yet?");
    return attack_power_entity;
}

Pickup_Entity pickup_life_entity(Game_State* state, V2 start, V2 end) {
    Pickup_Entity life_entity = pickup_entity_generic(state, PICKUP_LIFE, start, end, 1);
    unimplemented("pick_life_entity not needed yet?");
    return life_entity;
}

void Pickup_Entity::chase_player_update(Game_State* state, f32 dt) {
    Player* p                   = &state->gameplay_data.player;
    auto    direction_to_player = V2_direction(position, p->position);

    if (t_since_spawn >= PICKUP_ENTITY_AUTO_ATTRACT_DELAY) {
        velocity =
            direction_to_player * (100 + ((t_since_spawn - PICKUP_ENTITY_AUTO_ATTRACT_DELAY) * 150));
    }
}

void Pickup_Entity::default_behavior_update(Game_State* state, f32 dt) {
    if (animation_t < PICKUP_ENTITY_ANIMATION_T_LENGTH) {
        f32 t = clamp<f32>(animation_t/PICKUP_ENTITY_ANIMATION_T_LENGTH, 0.0f, 1.0f);
        position.x = lerp_f32(animation_start_position.x, animation_end_position.x, t);
        position.y = lerp_f32(animation_start_position.y, animation_end_position.y, t);

        animation_t += 2*dt*dt + dt;
    } else {
        lifetime.start();

        f32 t_p = lifetime.percentage();

        if (t_p <= 0.325f) {
            if (fading_t <= 0.0f) {
                fading_t = PICKUP_ENTITY_FADE_T_LENGTH;
                fade_phase += 1;
            } else {
                fading_t -= dt;
            }
        } else {
            fade_phase = 0;
        }

        // This shouldn't affect the actual hitbox, however the base entities
        // don't have a concept of "visual and logical separation" outside of their
        // sprite, which I'm not rendering right now b/c I don't have sprites for these yet.
        // I'll draw them later!
        position.y = animation_end_position.y + sinf(t_since_spawn) * (scale.y);
        lifetime.update(dt);
    }
}

void Pickup_Entity::update(Game_State* state, f32 dt) {
    if (!visible)
        return;

    if (awarded || (!seek_towards_player && lifetime.triggered())) {
        die = true;  
        return;
    }

    if (!seek_towards_player) {
        default_behavior_update(state, dt);
    } else {
        chase_player_update(state, dt);
    }

    sprite.animate(
        &state->resources->graphics_assets,
        dt,
        0.085
    );
    Entity::update(state, dt);
}

void Pickup_Entity::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    if (!visible)
        return;

    auto  sprite_img = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_border_vignette);
    V2    sprite_image_size = V2(32, 32) * (1 + normalized_sinf(Global_Engine()->global_elapsed_time*1.25 + 1234) * 0.55);

    V2 interpolated_position = V2(
        lerp_f32(last_position.x, position.x, state->gameplay_data.fixed_tickrate_remainder),
        lerp_f32(last_position.y, position.y, state->gameplay_data.fixed_tickrate_remainder)
    );

    V2 sprite_position = V2(
        interpolated_position.x + (sprite.offset.x - sprite_image_size.x/2),
        interpolated_position.y + (sprite.offset.y - sprite_image_size.y/2)
    );

    render_commands_push_image_ext(
        render_commands,
        sprite_img,
        rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
        RECTANGLE_F32_NULL,
        color32f32(
            sprite.modulation.r,
            sprite.modulation.g,
            sprite.modulation.b,
            sprite.modulation.a * 0.45
        ),
        V2(0, 0),
        0,
        0,
        BLEND_MODE_ADDITIVE
    );
    Entity::draw(
        state,
        render_commands,
        resources
    );
}

void Pickup_Entity::on_picked_up(Game_State* state) {
    if (!visible)
        return;

    if (awarded || die)
        return;

    if (!seek_towards_player) {
        if (animation_t < PICKUP_ENTITY_ANIMATION_T_LENGTH)
            return;
    }

    awarded = true;
    die     = true;

    // POLISH/TODO: would like polish animation.
    // but I haven't thought of the look too much right now so it's fine.
    // Try this after checkpoint 1 or something.

    switch (type) {
        case PICKUP_SCORE: {
            // unimplemented("pick_score_entity not needed yet?");
            state->gameplay_data.notify_score(value, true);
        } break;
        case PICKUP_ATTACKPOWER: {
            unimplemented("pick_attack_power_entity not needed yet?");
        } break;
        case PICKUP_LIFE: {
            unimplemented("pick_life_entity not needed yet?");
            // Need to notify tries as well.
            // TODO: rename the notify score system to be a UI notifier
            // whenever values change.
            state->gameplay_data.tries += 1;
        } break;
    }
}

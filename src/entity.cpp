// TODO(jerry): check the validity of some prng factors
// some stuff should probably not use the regular prng gen
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
void Entity::reset_movement(void) {
    velocity = V2(0,0);
    acceleration = V2(0,0);
}

void Entity::post_init_apply_current_game_rules(void) {
  /*
    NOTE(jerry):
      I was unaware that the C++ compiler was able to figure out
      my g_game_rules struct started with a constant value even though
      it's a variable... This is confusing as I thought constructors only
      worked with explicitly constant values (like literals...)

      So while this causes my code to compile straight away, it doesn't
      actually do what I want...

      So this method will reinitialize fields that technically depend on
      a dynamically chaning variable.
  */
  score_value = DEFAULT_ENTITY_SCORE_VALUE_PER_HIT;
  death_multiplier = DEFAULT_ENTITY_SCORE_KILL_VALUE_MULTIPLIER;
  hit_flash_timer = Timer(DAMAGE_FLASH_TIME_PERIOD);
  firing_cooldown = DEFAULT_FIRING_COOLDOWN;
  cleanup_time = Timer(ENTITY_TIME_BEFORE_OUT_OF_BOUNDS_DELETION);
  invincibility_time_flash_period = Timer(INVINCIBILITY_FLASH_TIME_PERIOD);
  invincibility_time = Timer(PLAYER_INVINICIBILITY_TIME);
}

void Entity::set_sprite_frame_region(s32 a, s32 b) {
    sprite_frame_begin = a;
    sprite_frame_end   = b;
}

void Entity::reset_sprite_animation_frames(void)  {
    set_sprite_frame_region(-1, -1);
}

void Entity::disable_all_particle_emitters(void) {
    for (s32 particle_emitter_index = 0; particle_emitter_index < ENTITY_MAX_PARTICLE_EMITTERS; ++particle_emitter_index) {
        auto& particle_emitter = emitters[particle_emitter_index];
        particle_emitter.flags &= ~PARTICLE_EMITTER_FLAGS_ACTIVE;
    }
}

s32 Entity::update_sprite_leaning_influence(f32 dt, V2 axes, Entity_Rotation_Lean_Params params) {
    // Procedural animation components
    s32 result                    = ROTATION_LEAN_INFLUENCE_DIRECTION_NOT_ROTATING;
    s32 sign                      = sign_f32(axes[0]);
    f32 horizontal_axis_magnitude = fabs(axes[0]);

    const f32 minimum_magnitude_to_consider_leaning                     = params.minimum_magnitude_to_consider_leaning;
    const f32 minimum_additional_magnitude_to_consider_rotation_leaning = params.minimum_additional_magnitude_to_consider_rotation_leaning;
    const f32 minimum_magnitude_to_consider_rotation_leaning            = minimum_magnitude_to_consider_leaning + minimum_additional_magnitude_to_consider_rotation_leaning;
    const f32 max_angle_lean                                            = params.max_angle_lean;
    const f32 decay_factor_min                                          = params.decay_factor_min;
    const f32 decay_factor_max                                          = params.decay_factor_max;
    {

        // angle
        {
            const f32 effective_t   = clamp<f32>(sprite.angle_offset / (max_angle_lean*0.90), 0.0f, 1.0f);
            f32       decay_factor  = lerp_f32(decay_factor_min, decay_factor_max, effective_t);
            sprite.angle_offset    *= pow(decay_factor, dt);
        }

        if (horizontal_axis_magnitude >= minimum_magnitude_to_consider_leaning) {
            if (sign == -1) {
                result = ROTATION_LEAN_INFLUENCE_DIRECTION_LEFT;
            } else if (sign == 1) {
                result = ROTATION_LEAN_INFLUENCE_DIRECTION_RIGHT;
            }


            if (horizontal_axis_magnitude >= (minimum_magnitude_to_consider_rotation_leaning)) {
                sprite.angle_offset = ((horizontal_axis_magnitude - minimum_magnitude_to_consider_rotation_leaning) * sign) * max_angle_lean;
            }
        }
    }

    return result;
}

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

void Entity::set_position(V2 new_position) { // teleport position
    last_position = position = new_position;
}

void Entity::set_position_x(f32 new_x) {
    last_position.x = position.x = new_x;
}

void Entity::set_position_y(f32 new_y) {
    last_position.y = position.y = new_y;
}

bool Entity::clamp_to_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area)) {
        velocity.x = 0;
        set_position_x(scale.x);
        return true;
    }
    return false;
}

bool Entity::clamp_to_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area)) {
        velocity.x = 0;
        set_position_x(play_area.width - scale.x);
        return true;
    }
    return false;
}

bool Entity::clamp_to_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area)) {
        velocity.y = 0;
        set_position_y(scale.y);
        return true;
    }
    return false;
}

bool Entity::clamp_to_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area)) {
        velocity.y = 0;
        set_position_y(play_area.height - scale.y);
        return true;
    }
    return false;
}

bool Entity::wrap_from_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area, true)) {
        set_position_x(play_area.width);
        return true;
    }
    return false;
}

bool Entity::wrap_from_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area, true)) {
        set_position_x(0);
        return true;
    }
    return false;
}

bool Entity::wrap_from_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area, true)) {
        set_position_y(play_area.height);
        return true;
    }
    return false;
}

bool Entity::wrap_from_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area, true)) {
        set_position_y(0);
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

    hit_flash_timer.start();
    hit_flash_count = 4;
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

s32 Entity::handle_play_area_edge_behavior(const Play_Area& play_area) {
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
                bool interacted = false;
                if (clamp_border(edge_index, play_area)) {
                    if (edge == PLAY_AREA_EDGE_DEADLY) {
                        end_invincibility();
                        kill();
                    }
                    interacted = true;
                }

                if (interacted) {
                    return edge;
                }
            } break;
            case PLAY_AREA_EDGE_WRAPPING: {
                if (wrap_border(edge_index, play_area))  {
                    return edge;
                }
            } break;
            case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
        }
    }

    return -1;
}

void Entity::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    if (!visible)
        return;

    auto r = get_rect();

    V2 interpolated_position = V2(
        lerp_f32(last_position.x, position.x, state->gameplay_data.fixed_tickrate_remainder),
        lerp_f32(last_position.y, position.y, state->gameplay_data.fixed_tickrate_remainder)
    ) + relative_position;

    r.x = interpolated_position.x - scale.x;
    r.y = interpolated_position.y - scale.y;

    if (sprite.id.index != 0) {
        Texture_Atlas& texture_atlas = resources->gameplay_texture_atlas;

        // NOTE: entity updates are responsible for actually
        //       animating their own sprites!
        auto  sprite_object     = graphics_get_sprite_by_id(&resources->graphics_assets, sprite.id);
        auto  sprite_frame      = sprite_get_frame(sprite_object, sprite.frame);
        auto  sprite_img        = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
        V2    sprite_image_size = V2(sprite_img->width, sprite_img->height);
        float facing_angle      = sprite.angle_offset;

        struct rectangle_f32 source_rectangle = texture_atlas.get_subrect(sprite_frame->img, sprite_frame->source_rect);
        struct image_buffer* image = graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id);

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
                image,
                rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                source_rectangle,
                modulation,
                V2(0.5, 0.5),
                facing_angle,
                0,
                shadow_entity_draw_blend_mode
            );
        }

        V2 sprite_position = V2(
            interpolated_position.x + (sprite.offset.x - sprite_image_size.x/2),
            interpolated_position.y + (sprite.offset.y - sprite_image_size.y/2)
        );

        render_commands_push_image_ext(
            render_commands,
            image,
            rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
            source_rectangle,
            sprite.modulation,
            V2(0.5, 0.5),
            facing_angle,
            0,
            entity_draw_blend_mode
        );

        // This would hurt in hardware because I'm not planning
        // to optimize the hardware renderers that much, which means I'm incuring
        // the wrath of a state change...
        if (flashing) {
            // flash harder
            for (int i = 0; i < 3; ++i) {
                render_commands_push_image_ext(
                    render_commands,
                    image,
                    rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                    source_rectangle,
                    sprite.modulation,
                    V2(0.5, 0.5),
                    facing_angle,
                    0,
                    BLEND_MODE_ADDITIVE
                );
            }
        }

        // Different type of flashing...
        {
            bool show_damage_flash = ((hit_flash_count % 2) == 0) && hit_flash_count;
            if (show_damage_flash) {
                // flash harder
                for (int i = 0; i < 3; ++i) {
                    render_commands_push_image_ext(
                        render_commands,
                        image,
                        rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                        source_rectangle,
                        color32f32(1, 0, 0, 1),
                        V2(0.5, 0.5),
                        facing_angle,
                        0,
                        BLEND_MODE_ADDITIVE
                    );
                }
            }
        }
    } else {
        // NOTE: this is lowkey debug code, which is the default thing
        // used if you don't have a sprite. It's better than making invisible enemies.
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
            rectangle_f32(position.x+relative_position.x - 1, position.y+relative_position.y-1, 2, 2),
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
            rectangle_f32(position.x+relative_position.x - 1, position.y+relative_position.y-1, 2, 2),
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

    {
        hit_flash_timer.update(dt);
        if (hit_flash_timer.triggered()) {
            hit_flash_count -= 1;
            if (hit_flash_count) {
                hit_flash_timer.reset();
                hit_flash_timer.start();
            }
        }
    }

    last_position       = position;
    velocity           += acceleration * dt;
    position           += velocity * dt;
    t_since_spawn      += dt;

    for (unsigned particle_emitter_index = 0; particle_emitter_index < ENTITY_MAX_PARTICLE_EMITTERS; ++particle_emitter_index) {
        auto& emitter = emitters[particle_emitter_index];
        emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
    }
}

V2 Entity::get_real_position(void) {
    return position + relative_position;
}

rectangle_f32 Entity::get_rect() {
    V2 real_position = get_real_position();

    return rectangle_f32(
        (real_position.x) - scale.x,
        (real_position.y) - scale.y,
        scale.x*2,
        scale.y*2
    );
}

// Cosmetic Pet Actor
void Cosmetic_Pet::fire_weapon(Game_State* state) {
#if 0
    auto resources = state->resources;
    switch (id) {
        case GAME_PET_ID_CAT: {
            firing_cooldown = DEFAULT_FIRING_COOLDOWN;
            Audio::play(
                resources->random_attack_sound(
                    &state->gameplay_data.prng
                )
            );
            spawn_bullet_arc_pattern1(state, position, 3, 15, V2(5, 5), V2(0, -1), 1000.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_DISK);
        } break;
        case GAME_PET_ID_DOG: {
            return;
        } break;
        case GAME_PET_ID_FISH: {
            return;
        } break;
    }
#else
    // Pets do not attack yet, or are not planned to? Just in case I guess.
    return;
#endif
}

void Cosmetic_Pet::update(Game_State* state, f32 dt) {
    if (!visible) {
        disable_all_particle_emitters();
        return;
    }

    auto& player = state->gameplay_data.player;

    const auto& play_area = state->gameplay_data.play_area;
    /*
     * NOTE:
     * make this animation look more elaborate.
     *
     *
     * But TL;DR pet will hover around the player and switch sides depending on where
     * the player is. This might ship tbh, since making this look nicer is a lot of work.
     *
     * TODO:
     * Looks a little janky...
     */
    f32 radius_hover = 85;

    V2 player_position   = player.get_real_position();
    V2 target_position   = V2(player_position.x - radius_hover, player_position.y);
    {
        if (player_position.x <= (play_area.width/2) * 0.72) {
            target_position   = V2(player_position.x + radius_hover, player_position.y);
        }

        V2 direction_towards = V2_direction(position, target_position);
        f32 distance         = V2_distance_sq(target_position, position);

        f32 percent      = clamp<f32>(fabs(distance-radius_hover) / radius_hover, 0.0f, 1.0f);
        f32 max_move_mag = 125 + ((percent*percent) * 47);
        // make this look a little nicer.
        velocity = direction_towards * max_move_mag * percent;
    }

    // same particle emitter as player
    {
        auto& emitter = emitters[0];
        emitter.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE;
        emitter.sprite = sprite_instance(state->resources->projectile_sprites[PROJECTILE_SPRITE_SPARKLING_STAR]);

        auto r = get_rect();
        f32 left   = position.x - 5;
        f32 bottom = position.y + 34;

        emitter.scale  = 1.0f;
        emitter.emit_per_emission = 4;
        emitter.lifetime = 0.65f;
        emitter.velocity_x_variance = V2(-10, 50);
        emitter.velocity_y_variance = V2(-10, 50);
        emitter.acceleration_x_variance = V2(0, 10);
        emitter.acceleration_y_variance = V2(0, 20);
        emitter.lifetime_variance   = V2(-0.1f, 0.7f);
        emitter.emission_max_timer = 0.045f;
        emitter.shape = particle_emit_shape_line(V2(left, bottom), V2(left + r.w*2.5, bottom));
    }
    {
        V2 direction_towards = V2_direction(position, target_position);
        s32 leaning_direction =
            update_sprite_leaning_influence(
                dt,
                direction_towards,
                Entity_Rotation_Lean_Params(40.5f)
            );
    }
    sprite.offset.y = sinf(t_since_spawn * 0.795) * 13.5 + 5;

    const auto& input_packet = state->gameplay_data.current_input_packet;
    Entity::update(state, dt); 

    bool firing = input_packet.actions & BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_ACTION_BIT);

    if (firing) {
        if (attack()) {
            fire_weapon(state);
        } else {
            
        }
    } else {
        stop_attack(); 
    }
}

void Cosmetic_Pet::set_id(s32 id, Game_Resources* resources) {
    _debugprintf("Pet id set to: %d", id);
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

// --- PLAYER_BURST_FLASH_T

void Player::reset_burst_charge_status(void) {
    burst_charge_disabled                 = false;
    burst_charge_halt_regeneration        = false;
    burst_charge                          = 0.0f;
    burst_charge_flash_count              = 0;
    burst_charge_halt_flash_t             = 0.0f;
    burst_absorption_shield_ability_timer = 0.0f;
    burst_ray_attack_ability_timer        = 0.0f;
}

void Player::halt_burst_abilities(void) {
    burst_absorption_shield_ability_timer = 0.0f;
    burst_ray_attack_ability_timer = 0.0f;
}

void Player::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    Entity::draw(state, render_commands, resources);

    if (burst_absorption_shield_ability_timer > 0.0f) {
        Texture_Atlas&       texture_atlas    = resources->gameplay_texture_atlas;
        struct rectangle_f32 source_rectangle = texture_atlas.get_subrect(resources->bubbleshield_circle);
        struct image_buffer* image            = graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id);

        rectangle_f32 r;
        auto pos = get_real_position();
        float radius = PLAYER_BURST_SHIELD_ABILITY_RADIUS*0.85;
        r.x = pos.x - radius;
        r.y = pos.y - radius;
        r.w = radius * 2;
        r.h = radius * 2;

        color32f32 modulation = color32f32(
            1,1,1,
            0.65 * burst_absorption_shield_ability_timer/(PLAYER_BURST_SHIELD_ABILITY_MAX_T*0.85));

        render_commands_push_image_ext(
            render_commands,
            image,
            r,
            source_rectangle,
            modulation,
            V2(0.5, 0.5),
            0.0,
            0,
            BLEND_MODE_ALPHA
        );
    }
}

void Player::disable_burst_charge_regeneration(void) {
    just_burned_out = true;
    burst_charge_disabled = true;
}

void Player::enable_burst_charge_regeneration(void) {
    burst_charge_disabled = false;
}

void Player::halt_burst_charge_regeneration(s32 flash_count_required) {
    just_burned_out = true;
    burst_charge = 0.0f;
    burst_charge_halt_regeneration = true;
    burst_charge_flash_count = flash_count_required;
    burst_charge_recharge_t = 0;
    // This is a magic number as the mapping isn't exactly that obvious since it's
    // not *really* synced to the thing that actually determines how long the charge takes.
    burst_charge_recharge_max_t = (flash_count_required*1.200) * PLAYER_BURST_FLASH_T;
}

s32 Player::get_burst_rank(void) {
    s32 tier_count = get_burst_mode_rank_count();
    f32 charge_percentage = (burst_charge / PLAYER_BURST_CHARGE_CAPACITY);
    auto current_tier = (s32)(charge_percentage * tier_count);
    return current_tier;
}

f32 Player::get_burst_charge_percent(void) {
    f32 charge_percentage = (burst_charge / PLAYER_BURST_CHARGE_CAPACITY);
    return charge_percentage;
}

void Player::handle_grazing_behavior(Game_State* state, f32 dt) {
    s32 grazing = currently_grazing(state);

    if (grazing) {
        if (grazing_delay <= 0.0f) {
            f32 score_modifier = get_grazing_score_modifier(grazing);
            burst_charge += 0.050;
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
                    (GRAZING_DEFAULT_SCORE_AWARD + 100) * score_modifier
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

// NOTE(jerry):
// this is a preliminary balancing idea as I am unsure as to why
s32 Player::get_burst_ability_usage(s32 id) {
    if (id != last_used_tier) {
        return 0;
    }

    return burst_ability_streak_usage;
}

void Player::add_burst_ability_usage(s32 id) {
    if (last_used_tier != id) {
        burst_ability_streak_usage = 0;
        last_used_tier = id;
    }

    burst_ability_streak_usage += 1;
}


typedef void (*Burst_Fire)(Player* player, Game_State* state, u32 _unused);
typedef bool (*Burst_Bomb)(Player* player, Game_State* state, u32 _unused);

struct Player_Burst_Action {
    Burst_Fire firing;
    Burst_Fire firing_wide;
    Burst_Bomb bomb;
};

void player_burst_fire_wide_focus_tier0(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_wide_focus_tier1(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_wide_focus_tier2(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_wide_focus_tier3(Player* player, Game_State* state, u32 _unused);

void player_burst_fire_focus_tier0(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_focus_tier1(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_focus_tier2(Player* player, Game_State* state, u32 _unused);
void player_burst_fire_focus_tier3(Player* player, Game_State* state, u32 _unused);

bool player_burst_bomb_focus_tier0(Player* player, Game_State* state, u32 _unused);
bool player_burst_bomb_focus_neutralizer_ray(Player* player, Game_State* state, u32 _unused);
bool player_burst_bomb_focus_bullet_shield(Player* player, Game_State* state, u32 _unused);
bool player_burst_bomb_focus_bkg_clear(Player* player, Game_State* state, u32 _unused);

s32 calculate_amount_of_burst_depletion_flashes_for(f32 seconds)
{
    f32 rounded = (s32)ceilf(seconds / PLAYER_BURST_FLASH_T);
    return (s32) rounded;
}

// NOTE(jerry): ranks are evenly divided for now!
local Player_Burst_Action g_player_burst_actions[] = {
    {
        player_burst_fire_focus_tier0,
        player_burst_fire_wide_focus_tier0,
        player_burst_bomb_focus_tier0,
    }, // TIER 0, no bomb, default focus attack.
    {
        player_burst_fire_focus_tier1,
        player_burst_fire_wide_focus_tier1,
        player_burst_bomb_focus_neutralizer_ray,
    },
    {
        player_burst_fire_focus_tier2,
        player_burst_fire_wide_focus_tier2,
        player_burst_bomb_focus_bullet_shield,
    },
    {
        player_burst_fire_focus_tier3,
        player_burst_fire_wide_focus_tier3,
        player_burst_bomb_focus_bkg_clear,
    },
};

int get_burst_mode_rank_count(void) {
    return array_count(g_player_burst_actions);
}

void player_burst_fire_focus_tier0(Player* player, Game_State* state, u32 _unused) {
    auto resources = state->resources;
    player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 1.15);
    state->set_led_target_color_anim(
        color32u8(255, 0, 0, 255),
        DEFAULT_FIRING_COOLDOWN / 4,
        false,
        true
    );
    Audio::play(
        resources->random_attack_sound(
            &state->gameplay_data.prng_unessential
        )
    );
    controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
    spawn_bullet_arc_pattern1(
        state,
        player->position,
        3,
        35/2.0f,
        V2(5, 5),
        V2(0, -1),
        800.0f,
        BULLET_SOURCE_PLAYER,
        PROJECTILE_SPRITE_BLUE_STROBING
    );

    player->drain_speed = PLAYER_BURST_TIER0_ATTACK_DRAIN_SPEED;
}

void player_burst_fire_focus_tier1(Player* player, Game_State* state, u32 _unused) {
    auto resources = state->resources;
    player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 1.5);
    state->set_led_target_color_anim(
        color32u8(255, 0, 0, 255),
        DEFAULT_FIRING_COOLDOWN / 4,
        false,
        true
    );
    Audio::play(
        resources->random_attack_sound(
            &state->gameplay_data.prng_unessential
        )
    );
    controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    3,
    35/2.0f,
    V2(5, 5),
    V2(0, -1),
    800.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_DISK
  );
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    3,
    35 / 4.0f,
    V2(5, 5),
    V2(0, -1),
    800.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_DISK
  );

  player->drain_speed = PLAYER_BURST_TIER1_ATTACK_DRAIN_SPEED;
}

void player_burst_fire_focus_tier2(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 2);
  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );
  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 85);
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    3,
    35/2.0f,
    V2(5, 5),
    V2(0, -1),
    850.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_RED_DISK,
    0
  );
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    3,
    35 / 4.0f,
    V2(5, 5),
    V2(0, -1),
    850.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_RED_DISK,
    0
  );
  player->drain_speed = PLAYER_BURST_TIER2_ATTACK_DRAIN_SPEED;
}

void player_burst_fire_focus_tier3(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 3);
  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );
  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 85);
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    4,
    35/4.0f,
    V2(5, 5),
    V2(0, -1),
    900.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_GREEN_DISK,
    0
  );
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    4,
    35 / 6.0f,
    V2(5, 5),
    V2(0, -1),
    890.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_GREEN_DISK,
    0
  );
  player->drain_speed = PLAYER_BURST_TIER3_ATTACK_DRAIN_SPEED;
}

void player_burst_fire_wide_focus_tier0(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = DEFAULT_FIRING_COOLDOWN;

  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );

  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
  // spawn_bullet_line_pattern1(state, position, 1, 20.0f, V2(5, 5), V2(0, -1), 1550.0f, BULLET_SOURCE_PLAYER);
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    3,
    45,
    V2(5, 5),
    V2(0, -1),
    650.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_ELECTRIC
  );
}

void player_burst_fire_wide_focus_tier1(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 1.5);
  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );
  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    4,
    50,
    V2(5, 5),
    V2(0, -1),
    800.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_ELECTRIC
  );
}

void player_burst_fire_wide_focus_tier2(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 1.5);
  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );
  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 85);
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    2,
    45,
    V2(5, 5),
    V2(0, -1),
    850.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_RED_ELECTRIC,
    0
  );
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    3,
    30,
    V2(5, 5),
    V2(0, -1),
    800.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_ELECTRIC
  );
}

void player_burst_fire_wide_focus_tier3(Player* player, Game_State* state, u32 _unused) {
  auto resources = state->resources;
  player->firing_cooldown = (DEFAULT_FIRING_COOLDOWN / 2);
  state->set_led_target_color_anim(
    color32u8(255, 0, 0, 255),
    DEFAULT_FIRING_COOLDOWN / 4,
    false,
    true
  );
  Audio::play(
    resources->random_attack_sound(
      &state->gameplay_data.prng_unessential
    )
  );
  controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 85);
  spawn_bullet_arc_pattern2_trailed(
    state,
    player->position,
    3,
    45,
    V2(5, 5),
    V2(0, -1),
    850.0f,
    0.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_RED_ELECTRIC,
    0
  );
  spawn_bullet_arc_pattern1(
    state,
    player->position,
    3,
    30,
    V2(5, 5),
    V2(0, -1),
    800.0f,
    BULLET_SOURCE_PLAYER,
    PROJECTILE_SPRITE_BLUE_ELECTRIC
  );
}

bool player_burst_bomb_focus_tier0(Player* player, Game_State* state, u32 _unused) {
  // tier0 has no bomb.
  return false;
}

bool player_burst_bomb_focus_neutralizer_ray(Player* player, Game_State* state, u32 _unused) {
    player->add_burst_ability_usage(1);
    s32 usage_count = player->get_burst_ability_usage(1);

    // bad design.
#if 0
    const s32 SCORE_COST = 25000;
    // NOTE(jerry):
    // This is the only burst ability that will not use lives, so it
    // is always the "last" line of defense.
    if (state->gameplay_data.current_score < SCORE_COST) { // Maybe have it vary based on score?
        return false;
    }
    state->gameplay_data.current_score -= SCORE_COST;
#endif

    player->halt_burst_charge_regeneration(
        // NOTE(jerry):
        // this to discourage it's spamming since otherwise it builds up too fast.
        calculate_amount_of_burst_depletion_flashes_for(PLAYER_BURST_RAY_COOLDOWN_DEPLETION_APPROX)
    );
    player->burst_ray_attack_ability_timer = PLAYER_BURST_RAY_ABILITY_MAX_T;
    player->current_burst_ability_max_t = PLAYER_BURST_RAY_ABILITY_MAX_T;
    return true;
}

bool player_burst_bomb_focus_bullet_shield(Player* player, Game_State* state, u32 _unused) {
    if (PLAYER_BURST_SHIELD_REQUIRES_AT_LEAST_ONE_LIFE && state->gameplay_data.tries < 1) {
        return false;
    }

    player->add_burst_ability_usage(2);
    s32 usage_count = player->get_burst_ability_usage(2);
    player->halt_burst_charge_regeneration(
        calculate_amount_of_burst_depletion_flashes_for(PLAYER_BURST_SHIELD_COOLDOWN_DEPLETION_APPROX)
    );
    state->gameplay_data.notify_score(PLAYER_BURST_SHIELD_USE_SCORE_AWARD, true);
    Audio::play(state->resources->random_explosion_sound(&state->gameplay_data.prng_unessential));
    controller_rumble(Input::get_gamepad(0), 0.7f, 0.7f, 200);
    camera_traumatize(&state->gameplay_data.main_camera, 0.5f);
    player->burst_absorption_shield_ability_timer = PLAYER_BURST_SHIELD_ABILITY_MAX_T;
    player->current_burst_ability_max_t = PLAYER_BURST_SHIELD_ABILITY_MAX_T;
    return true;
}

bool player_burst_bomb_focus_bkg_clear(Player* player, Game_State* state, u32 _unused) {
    if (PLAYER_BURST_BOMB_REQUIRES_AT_LEAST_ONE_LIFE && state->gameplay_data.tries < 1) {
        return false;
    }
    player->add_burst_ability_usage(3);
    s32 usage_count = player->get_burst_ability_usage(3);
    auto resources = state->resources;

#if 0
    // NOTE(jerry):
    /*
     * I do not personally think this is a good idea because killing enemies would
     * break the level flow in a detrimental way...
     */
    state->convert_enemies_to_score_pickups();
#endif
    state->convert_bullets_to_score_pickups();
    state->gameplay_data.notify_score(PLAYER_BURST_BOMB_USE_SCORE_AWARD, true);
    state->set_led_target_color_anim_force(color32u8(255, 165, 0, 255), 0.08, false, true);
    Audio::play(resources->random_hit_sound(&state->gameplay_data.prng_unessential));
    controller_rumble(Input::get_gamepad(0), 0.7f, 0.7f, 200);
    camera_traumatize(&state->gameplay_data.main_camera, 0.5f);

    state->gameplay_data.remove_life();
    player->halt_burst_charge_regeneration(
        calculate_amount_of_burst_depletion_flashes_for(PLAYER_BURST_BOMB_COOLDOWN_DEPLETION_APPROX)
    );

    player->begin_invincibility(true, PLAYER_BURST_BOMB_INVINCIBILITY_T);
    return true;
}

Player_Burst_Action* get_player_burst_action_set(Player* player) {
  int rank_count = get_burst_mode_rank_count();
  int per_rank = ((int)PLAYER_BURST_CHARGE_CAPACITY / rank_count);
  int tier_rank = (int)floorf(player->burst_charge / per_rank);
  tier_rank = clamp<int>(tier_rank, 0, rank_count-1);
  return g_player_burst_actions + tier_rank;
}


void Player::handle_bomb_usage(Game_State* state, u32 bomb_pattern_id) {
    auto& gameplay_data = state->gameplay_data;
    auto  resources = state->resources;
    auto action_set = get_player_burst_action_set(this);

    bool successful = false;
    switch (bomb_pattern_id) {
    case BOMB_PATTERN_DEFAULT: {
      successful = action_set->bomb(this, state, bomb_pattern_id);
    } break;
    case BOMB_PATTERN_EXTRA1: {
      unimplemented("BOMB_PATTERN_EXTRA1");
    } break;
    case BOMB_PATTERN_EXTRA2: {
      unimplemented("BOMB_PATTERN_EXTRA2");
    } break;
    case BOMB_PATTERN_EXTRA3: {
      unimplemented("BOMB_PATTERN_EXTRA3");
    } break;
    }

    if (successful) {
      burst_charge = 0.0f;
      state->gameplay_data.just_used_bomb = true;
    }
    else {
      if (state->gameplay_data.invalid_usage_flash_count <= 0) {
        state->gameplay_data.invalid_usage_flash_count = 16;
      }
    }
}

void Player::fire_burst_ray_laser(Game_State* state) {
    auto resources = state->resources;
    /* Burst Attack Laser */
    {
        firing_cooldown = (DEFAULT_FIRING_COOLDOWN/3.25);
        // NOTE(jerry):
        /*
         * since the entity_prototypes.cpp stuff was never developed
         * very much, I don't feel like trying to refactor it to
         * be more tunable, so for now this projectile set will be made inline.
         */
        state->set_led_target_color_anim(
            color32u8(255, 0, 0, 255),
            DEFAULT_FIRING_COOLDOWN/4,
            false,
            true
        );
        s32 bullet_count = 1;
        Audio::play(
            resources->random_attack_sound(
                &state->gameplay_data.prng_unessential
            )
        );
        for (s32 bullet_index = 0; bullet_index < bullet_count; ++bullet_index) {
            auto position = get_real_position();
            {
              auto bullet = bullet_upwards_linear(
                state, position, V2(0, -1), 500.0f,
                PROJECTILE_SPRITE_HOT_PINK_DISK, BULLET_SOURCE_PLAYER);
              bullet.flags |= BULLET_FLAGS_BREAKS_OTHER_BULLETS;
              bullet.trail_ghost_limit = 16;
              bullet.trail_ghost_record_timer_max = 0.0000f;
              bullet.trail_ghost_modulation = color32f32(1.0, 1.0, 1.0, 1.0);
              bullet.trail_ghost_max_alpha = 0.15f;
              bullet.entity_draw_blend_mode = BLEND_MODE_ADDITIVE;
              bullet.shadow_entity_draw_blend_mode = BLEND_MODE_ADDITIVE;
              state->gameplay_data.add_bullet(
                bullet
              );
            }
        }
    }
}

void Player::fire_weapon(Game_State* state, u32 attack_pattern_id) {
    auto resources = state->resources;
    switch (attack_pattern_id) {
        case ATTACK_PATTERN_DEFAULT: { // Default Attack Pattern
            auto action_set = get_player_burst_action_set(this);

            if (under_focus) {
                action_set->firing(this, state, attack_pattern_id);
            } else {
                action_set->firing_wide(this, state, attack_pattern_id);
            }
        } break;
        case ATTACK_PATTERN_EXTRA1: {
            unimplemented("ATTACK_PATTERN_EXTRA1");
        } break;
        case ATTACK_PATTERN_EXTRA2: {
            unimplemented("ATTACK_PATTERN_EXTRA2");
        } break;
        case ATTACK_PATTERN_EXTRA3: {
            unimplemented("ATTACK_PATTERN_EXTRA3");
        } break;
    }
}

local f32 burst_charge_decay_rate(s32 rank) {
#if 0
    switch (rank) {
        case 0: {
            return PLAYER_BURST_TIER0_PASSIVE_DECAY_SPEED;
        } break;
        case 1: {
            return PLAYER_BURST_TIER1_PASSIVE_DECAY_SPEED;
        } break;
        case 2: {
            return PLAYER_BURST_TIER2_PASSIVE_DECAY_SPEED;
        } break;
        case 3: {
            // At higher burst level, it's harder to drain because any attack will drain it massively.
            return PLAYER_BURST_TIER3_PASSIVE_DECAY_SPEED;
        } break;
    }
    return 100;
#else
  return 0;
#endif
}

void Player::handle_burst_charging_behavior(Game_State* state, f32 dt) {
    bool enabled_cheat = false;
    int  rank_count    = get_burst_mode_rank_count();
    int  per_rank      = ((int)PLAYER_BURST_CHARGE_CAPACITY / rank_count);

    {
        s32 burstmode_cheat;
        if ((burstmode_cheat = DebugUI::forced_burstmode_value()) != -1) {
            enabled_cheat = true;
            burst_charge = burstmode_cheat * per_rank+1;
            burst_charge_halt_regeneration = 0;
            burst_charge_flash_count = 0;
        }
    }

    if (!enabled_cheat) {
        // NOTE(jerry):
        // the stamina drain recovery is still allowed here
        // as it would be problematic if disabling the entire burst charge
        // would artificially extend your inability to act with focus.
        if (burst_charge_halt_regeneration) {
            if (burst_charge_flash_count) {
                if (burst_charge_halt_flash_t < PLAYER_BURST_FLASH_T) {
                    burst_charge_recharge_t += dt;
                    burst_charge_halt_flash_t += dt;
                } else {
                    burst_charge_halt_flash_t =  0.0f;
                    burst_charge_flash_count  -= 1;
                }
            } else {
                burst_charge_halt_regeneration = false;
            }
        }

        if (burst_charge_disabled) {
            burst_charge = 0.0f;
        } else {
            if (!burst_charge_halt_regeneration) {
                f32 charge_speed = PLAYER_DEFAULT_BURST_CHARGE_SPEED; // make charging harder

                if (under_focus && firing) {
                    burst_charge -= dt * drain_speed;
                } else {
                    burst_charge += dt * charge_speed;
                }

                int tier_rank;
                {
                    tier_rank = (int)floorf(burst_charge / per_rank);
                    tier_rank = clamp<int>(tier_rank, 0, rank_count);
                    if (tier_rank < 0) tier_rank = 0;
                }
                drain_speed = burst_charge_decay_rate(tier_rank);
            }
        }
    }
}

void Player::show_border_status_when_close(Game_State* state) {
  // This is not a real game rule, so I can just hardcode this here...
  const auto& play_area = state->gameplay_data.play_area;
  const f32 BORDER_SHOW_BEHAVIOR_THRESHOLD_PX = 32;

  f32 distance_to_left_border = fabs(0 - position.x);
  f32 distance_to_right_border = fabs(play_area.width - position.x);
  f32 distance_to_top_border = fabs(0 - position.y);
  f32 distance_to_bottom_border = fabs(play_area.height - position.y);

  if (distance_to_left_border <= BORDER_SHOW_BEHAVIOR_THRESHOLD_PX) {
    state->gameplay_data.border_notify(PLAY_AREA_EDGE_ID_LEFT, state->gameplay_data.play_area.edge_behavior_left + 1, 1, true);
  }

  if (distance_to_right_border <= BORDER_SHOW_BEHAVIOR_THRESHOLD_PX) {
    state->gameplay_data.border_notify(PLAY_AREA_EDGE_ID_RIGHT, state->gameplay_data.play_area.edge_behavior_right + 1, 1, true);
  }

  if (distance_to_top_border <= BORDER_SHOW_BEHAVIOR_THRESHOLD_PX) {
    state->gameplay_data.border_notify(PLAY_AREA_EDGE_ID_TOP, state->gameplay_data.play_area.edge_behavior_top + 1, 1, true);
  }

  if (distance_to_bottom_border <= BORDER_SHOW_BEHAVIOR_THRESHOLD_PX) {
    state->gameplay_data.border_notify(PLAY_AREA_EDGE_ID_BOTTOM, state->gameplay_data.play_area.edge_behavior_bottom + 1, 1, true);
  }
}

void Player::update(Game_State* state, f32 dt) {
    if (!visible) {
        disable_all_particle_emitters();
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

    // NOTE(jerry):
    // This is to protect the player from an infinite burn out loop
    // because players did not seem to understand the resource management
    // early on, and this was probably far too punishing.
#if 0
    if (just_burned_out) {
        if (focusing) {
            focusing = false;
        } else {
            just_burned_out = false;
        }
    }
#endif

    under_focus = focusing;
#if 0
    // NOTE(jerry): You cannot focus if you're attempting to
    //              recover.
    if (!burst_charge_halt_regeneration) {
        under_focus = focusing;
    } else {
        if (focusing) {
            if (state->gameplay_data.invalid_usage_flash_count <= 0) {
                state->gameplay_data.invalid_usage_flash_count = 8;
            }
        }
        under_focus = false;
    }
#endif

    V2 axes = gameplay_frame_input_packet_quantify_axes(input_packet);
    float UNIT_SPEED = ((under_focus) ? 150 : 235) * pet_data->speed_modifier;

    if (burst_absorption_shield_ability_timer > 0.0f) {
        UNIT_SPEED *= 0.675f;
    }

    if (burst_ray_attack_ability_timer > 0.0f) {
        UNIT_SPEED *= 0.675f;
    }

    if (!state->gameplay_data.triggered_stage_completion_cutscene) {
        velocity.x = axes[0] * UNIT_SPEED;
        velocity.y = axes[1] * UNIT_SPEED;
        acceleration.x = 0;
        acceleration.y = 0;
        Entity::update(state, dt);
        switch (handle_play_area_edge_behavior(play_area)) {
            case PLAY_AREA_EDGE_DEADLY: {
                // move the player back to their default position
                // this is more "fair" to do in my opinion.
                position = last_position =
                    V2(state->gameplay_data.play_area.width / 2, 300);
            } break;
            default: {} break;
        }
    } else {
        // cutscene is taking control.
        Entity::update(state, dt);
        axes = velocity.normalized();
    }

    // Particle Emitter
    {
        auto& emitter = emitters[0];
        emitter.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE;
        emitter.sprite = sprite_instance(state->resources->projectile_sprites[PROJECTILE_SPRITE_SPARKLING_STAR]);

        auto r = get_rect();
        f32 left   = position.x - 5;
        f32 bottom = position.y + 34;

        emitter.scale  = 1.0f;
        emitter.emit_per_emission = 8;
        emitter.lifetime = 0.65f;
        emitter.velocity_x_variance = V2(-10, 50);
        emitter.velocity_y_variance = V2(-10, 50);
        emitter.acceleration_x_variance = V2(0, 10);
        emitter.acceleration_y_variance = V2(0, 20);
        emitter.lifetime_variance   = V2(-0.1f, 0.7f);
        emitter.emission_max_timer = 0.085f;
        emitter.shape = particle_emit_shape_line(V2(left, bottom), V2(left + r.w*2, bottom));
    }

    // Handle Particle Emitter - Shield/Laser
    {
        auto& emitter = emitters[1];
        if (burst_absorption_shield_ability_timer > 0) {
            firing = true;
            // hackme
            burst_charge = 2;

            firing_cooldown = DEFAULT_FIRING_COOLDOWN * 1.15;
            emitter.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE | PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
            emitter.sprite = sprite_instance(state->resources->projectile_sprites[PROJECTILE_SPRITE_SPARKLING_STAR]);
            emitter.scale  = 0.75f;
            emitter.emit_per_emission = 6;
            emitter.lifetime = 0.65f;
            emitter.velocity_x_variance = V2(-10, 50);
            emitter.velocity_y_variance = V2(-10, 50);
            emitter.acceleration_x_variance = V2(0, 10);
            emitter.acceleration_y_variance = V2(0, 20);
            emitter.lifetime_variance   = V2(-0.1f, 0.7f);
            emitter.emission_max_timer = 0.020f;
            emitter.shape = particle_emit_shape_circle(
                get_real_position(),
                PLAYER_BURST_SHIELD_ABILITY_RADIUS,
                true
            );
            emitter.modulation = color32f32(1, 1, 0, 1);
            // turn off original emitter for now.
            {
                auto& emitter = emitters[0];
                emitter.flags &= ~PARTICLE_EMITTER_FLAGS_ACTIVE;
            }
            if (attack()) {
              auto resources = state->resources;
              state->set_led_target_color_anim(
                color32u8(255, 0, 0, 255),
                DEFAULT_FIRING_COOLDOWN / 2,
                false,
                true
              );
              Audio::play(
                resources->random_attack_sound(
                  &state->gameplay_data.prng_unessential
                )
              );
              controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 85);
            
              // NOTE(jerry) (4/28/2025): 
              // Dirty code, I'm tired today and just want to get stuff in quick
              //
              // Game overhaul will take a few days to adjust things, but it is for the better.
              for (int i = 0; i < 360; i += 18) {
                f32 angle = i + burst_absorption_shield_ability_timer*30;
                V2 current_arc_direction = V2_direction_from_degree(angle);
                V2 position = this->position + current_arc_direction;
                auto b = bullet_upwards_linear(state, position, current_arc_direction, 650.0f, PROJECTILE_SPRITE_GREEN_ELECTRIC, BULLET_SOURCE_PLAYER);
                b.flags |= BULLET_FLAGS_BREAKS_OTHER_BULLETS;
                state->gameplay_data.add_bullet(
                  b
                );
              }
              for (int i = 0; i < 360; i += 18) {
                f32 angle = i - burst_absorption_shield_ability_timer * 30;
                V2 current_arc_direction = V2_direction_from_degree(angle);
                V2 position = this->position + current_arc_direction;
                auto b = bullet_upwards_linear(state, position, current_arc_direction, 650.0f, PROJECTILE_SPRITE_GREEN_ELECTRIC, BULLET_SOURCE_PLAYER);
                b.flags |= BULLET_FLAGS_BREAKS_OTHER_BULLETS;
                state->gameplay_data.add_bullet(
                  b
                );
              }
            }
        } else if (burst_ray_attack_ability_timer > 0) {
          burst_charge = 2;
          emitter.flags |= PARTICLE_EMITTER_FLAGS_ACTIVE | PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
          emitter.sprite = sprite_instance(state->resources->projectile_sprites[PROJECTILE_SPRITE_RED_ELECTRIC]);
          emitter.scale = 0.12f;
          emitter.emit_per_emission = 5;
          emitter.lifetime = 0.75f;
          emitter.scale_variance = V2(-0.25, 0.35);
          emitter.velocity_x_variance = V2(-160, 160);
          emitter.velocity_y_variance = V2(-20, 40);
          emitter.acceleration_x_variance = V2(0, 0);
          emitter.acceleration_y_variance = V2(98, 120);
          emitter.lifetime_variance = V2(-0.1f, 0.7f);
          emitter.emission_max_timer = 0.020f;
          auto center = get_real_position();
          center.y -= 20.0f;
          emitter.shape = particle_emit_shape_circle(
           center, 
            6.0f,
            true
          );
          emitter.modulation = color32f32(1, 1, 1, 1);
          // turn off original emitter for now.
          {
            auto& emitter = emitters[0];
            emitter.flags &= ~PARTICLE_EMITTER_FLAGS_ACTIVE;
          }
        } else {
          emitter.flags &= ~PARTICLE_EMITTER_FLAGS_ACTIVE;
        }
        burst_absorption_shield_ability_timer -= dt;
    }

    // Sprite animation
    {
        sprite_frame_begin = 0;
        sprite_frame_end   = 1;

        f32 lean_angle =
            (under_focus) ?
            27.5f :
            45.5f;
        s32 leaning_direction =
            update_sprite_leaning_influence(
                dt,
                axes,
                Entity_Rotation_Lean_Params(lean_angle)
            );

        switch (leaning_direction) {
            case ROTATION_LEAN_INFLUENCE_DIRECTION_LEFT: {
                sprite_frame_begin = 1;
                sprite_frame_end = 2;
            } break;
            case ROTATION_LEAN_INFLUENCE_DIRECTION_RIGHT: {
                sprite_frame_begin = 2;
                sprite_frame_end = 3;
            } break;
            default: {} break;
        }

        sprite.offset.y = sinf(t_since_spawn * 2.5) * 0.5 + 8; // NOTE: +7 is to adjust visual location to make the hit box look less "unintuitive"
        sprite.animate(
            &state->resources->graphics_assets,
            dt,
            0.035,
            sprite_frame_begin,
            sprite_frame_end
        );
    }

    handle_burst_charging_behavior(state, dt);
    show_border_status_when_close(state);

    if (burst_ray_attack_ability_timer > 0.0f) {
        fire_burst_ray_laser(state);
        burst_ray_attack_ability_timer -= dt;
    } else {
        if (!burst_charge_halt_regeneration) {
            if (firing) {
                if (attack()) {
                    fire_weapon(state, pet_data->attack_pattern_id);
                }
            } else {
                stop_attack();
            }

            if (use_bomb) {
                state->gameplay_data.queue_bomb_use = true;
            }
        } else {
            if (firing || use_bomb) {
                if (state->gameplay_data.invalid_usage_flash_count <= 0) {
                    state->gameplay_data.invalid_usage_flash_count = 8;
                }
            }
        }
    }


    burst_charge = clamp<f32>(
        burst_charge, 0.0f, PLAYER_BURST_CHARGE_CAPACITY
    );

    if (!burst_charge_halt_regeneration) {
        if (f32_close_enough(burst_charge, 0.0f)) {
            _debugprintf("burn out!");
            halt_burst_charge_regeneration(
                calculate_amount_of_burst_depletion_flashes_for(
                    2.5f
                )
            );
        }
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
    }
}

void Bullet::update(Game_State* state, f32 dt) {
    if (!visible)
        return;

    // entity_draw_blend_mode = BLEND_MODE_ADDITIVE;
    // shadow_entity_draw_blend_mode = BLEND_MODE_ADDITIVE;
    // sprite.modulation = color32f32(0.5,0.5,0.5,1.0f);

    handle_lifetime(dt);
    handle_movement(state, dt);

    sprite.animate(
        &state->resources->graphics_assets,
        dt,
        0.035
    );

    Entity::update(state, dt);

    // Projectiles outside of the play area should stop existing.
    auto r = get_rect();
    r.w = 300;
    r.h = 300;
    r.x -= r.w / 2;
    r.y -= r.h / 2;
    if (!state->gameplay_data.play_area.is_inside_logical(r)) {
      die = true;
    }
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
    }

    Entity::update(state, dt);
    if (sprite.id.index != 0) {
        sprite.animate(
            &state->resources->graphics_assets,
            dt,
            0.125
        );
    }
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
    auto resources = state->resources;

    if (warning.finished_presenting()) {
        explosion_timer.start();

        if (explosion_timer.triggered()) {
            // a big bang!
            exploded = true;

            // Spawn explosion particles here
            // explosion heat ring
            {
                auto& emitter = outer_ring_emitter;
                emitter.reset();
                emitter.sprite                  = sprite_instance(resources->circle_sprite);
                emitter.sprite.scale            = V2(0.250, 0.250);
                emitter.shape                   = particle_emit_shape_circle(position, radius, false);
                emitter.modulation              = outer_ring_color;
                emitter.target_modulation       = color32f32(59/255.0f, 59/255.0f, 56/255.0f, 127/255.0f);
                emitter.lifetime                = 0.75f;
                emitter.scale_variance          = V2(-0.15, 0.15f);
                emitter.angle_range             = V2(-360, 360);
                emitter.velocity                = V2(90.0f);
                emitter.velocity_x_variance     = V2(15, 15);
                emitter.acceleration_x_variance = V2(0, 20);
                emitter.lifetime_variance       = V2(-0.25f, 0.2f);
                emitter.emission_max_timer      = 0.035f;
                emitter.max_emissions           = 1;
                emitter.emit_per_emission       = 128;
                emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                    PARTICLE_EMITTER_FLAGS_USE_ANGULAR |
                    PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE |
                    PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
                emitter.scale                   = 1;
                emitter.blend_mode              = BLEND_MODE_ALPHA;
            }

            // inner
            {
                auto& emitter = inner_emitter;
                emitter.reset();
                emitter.sprite                  = sprite_instance(resources->circle_sprite);
                emitter.sprite.scale            = V2(0.185/2, 0.185/2);
                emitter.shape                   = particle_emit_shape_point(position);
                emitter.modulation              = inner_ring_color;
                emitter.target_modulation       = color32f32(59/255.0f, 59/255.0f, 56/255.0f, 127/255.0f);
                emitter.lifetime                = 1.25;
                emitter.scale_variance          = V2(-0.085, 0.085);
                emitter.angle_range             = V2(-360, 360);
                emitter.velocity                = V2(70.0f);
                emitter.velocity_x_variance     = V2(25, 45);
                emitter.acceleration_x_variance = V2(0, 20);
                emitter.lifetime_variance       = V2(-0.25f, 0.2f);
                emitter.emission_max_timer      = 0.035f;
                emitter.max_emissions           = 1;
                emitter.emit_per_emission       = 256;
                emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                    PARTICLE_EMITTER_FLAGS_USE_ANGULAR |
                    PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE |
                    PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
                emitter.scale                   = 1;
                emitter.blend_mode              = BLEND_MODE_ALPHA;
            }

            //gray
            {
                auto& emitter = dust_emitter;
                emitter.reset();
                emitter.sprite                  = sprite_instance(resources->circle_sprite);
                emitter.sprite.scale            = V2(0.075f, 0.075);
                emitter.shape                   = particle_emit_shape_circle(position, radius, true);
                emitter.modulation              = color32f32(108/255.0f, 122/255.0f, 137/255.0f, 1.0f);
                emitter.lifetime                = 2.0f;
                emitter.scale_variance          = V2(-0.055, 0.085);
                emitter.angle_range             = V2(-360, 360);
                emitter.velocity                = V2(40.0f);
                emitter.velocity_x_variance     = V2(15, 35);
                emitter.acceleration_x_variance = V2(0, 20);
                emitter.lifetime_variance       = V2(-0.25f, 0.2f);
                emitter.emission_max_timer      = 0.035f;
                emitter.max_emissions           = 1;
                emitter.emit_per_emission       = 128;
                emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                    PARTICLE_EMITTER_FLAGS_USE_ANGULAR;
                emitter.scale                   = 1;
                emitter.blend_mode              = BLEND_MODE_ADDITIVE;
            }
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

    inner_emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
    outer_ring_emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
    dust_emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
}

void Explosion_Hazard::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    bool show_explosion_warning = warning.finished_presenting();

    if (show_explosion_warning) {
        render_commands_push_image(render_commands,
                                   graphics_assets_get_image_by_id(&resources->graphics_assets, resources->circle),
                                   rectangle_f32(position.x - radius, position.y - radius, radius*2, radius*2),
                                   RECTANGLE_F32_NULL,
                                   color32f32(0, 0, 0, 0.5f),
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
                                      resources->get_font(MENU_FONT_COLOR_BLOODRED),
                                      2, position,
                                      string_literal("!!!!"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
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
    const auto& play_area = state->gameplay_data.play_area;
    auto        rectangle = get_rect(&play_area);

    if (warning.finished_presenting()) {
        lifetime.start();
        // default update
        position += velocity * dt;

        if (lifetime.t == -1) {
            // live until killed by something else
        } else {
            lifetime.update(dt);

            if (projectile_sprite_id == -1) {
                projectile_sprite_id = PROJECTILE_SPRITE_RED_ELECTRIC;
            }
            
            {
                auto& emitter = outer_ring_emitter;
                emitter.sprite                  = sprite_instance(state->resources->projectile_sprites[projectile_sprite_id]);
                emitter.sprite.scale            = V2(0.67f);
                emitter.shape                   = particle_emit_shape_quad(V2(rectangle.x + rectangle.w/2, rectangle.y + rectangle.h/2), V2(rectangle.w/2, rectangle.h/2), true);
                emitter.modulation              = color32f32(1, 1, 1, 1);
                emitter.lifetime                = 1.00f;
                // emitter.lifetime                = 1.20f;
                emitter.scale_variance          = V2(-0.25, 0.25f);
                emitter.lifetime_variance       = V2(-0.25f, 0.2f);
                emitter.emission_max_timer      = 0.030f;
                emitter.max_emissions           = -1;
                emitter.emit_per_emission       = 16;
                // emitter.flame_mode              = true;
                emitter.flags                  = PARTICLE_EMITTER_FLAGS_ACTIVE;
                emitter.scale                   = 1;
                emitter.blend_mode              = BLEND_MODE_ADDITIVE;
            }

            // //gray
            {
                auto& emitter = dust_emitter;
                emitter.reset();
                emitter.sprite = sprite_instance(state->resources->projectile_sprites[projectile_sprite_id]);
                emitter.sprite.scale            = V2(0.255f, 0.255);
                emitter.shape                   = particle_emit_shape_quad(V2(rectangle.x + rectangle.w/2, rectangle.y + rectangle.h/2), V2(rectangle.w/2, rectangle.h/2), true);
                emitter.modulation              = color32f32(108/255.0f, 122/255.0f, 137/255.0f, 1.0f);
                emitter.lifetime                = 0.3f;
                // emitter.lifetime                = 2.0f;
                emitter.scale_variance          = V2(-0.055, 0.085);
                emitter.angle_range             = V2(-360, 360);
                emitter.velocity                = V2(40.0f);
                emitter.velocity_x_variance     = V2(15, 35);
                emitter.acceleration_x_variance = V2(0, 20);
                emitter.lifetime_variance       = V2(-0.25f, 0.2f);
                emitter.emission_max_timer      = 0.055f;
                emitter.max_emissions           = -1;
                emitter.emit_per_emission       = 4;
                emitter.flags = PARTICLE_EMITTER_FLAGS_ACTIVE |
                    PARTICLE_EMITTER_FLAGS_USE_ANGULAR;
                emitter.scale                   = 1;
                emitter.blend_mode              = BLEND_MODE_ADDITIVE;
            }

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

    dust_emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
    outer_ring_emitter.update(&state->gameplay_data.particle_pool, &state->gameplay_data.prng_unessential, dt);
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
    score_entity.sprite.scale = V2(1, 1);
    score_entity.sprite.modulation = color32f32(255.0f / 255.0f, 215.0f / 255.0f, 0.0f, 1.0f);
    // unimplemented("pick_score_entity not needed yet?");
    return score_entity;
}

Pickup_Entity pickup_attack_power_entity(Game_State* state, V2 start, V2 end, s32 value) {
    Pickup_Entity attack_power_entity = pickup_entity_generic(state, PICKUP_ATTACKPOWER, start, end, value);
    attack_power_entity.sprite = sprite_instance(state->resources->point_pickup_sprite);
    attack_power_entity.sprite.scale = V2(1, 1);
    attack_power_entity.sprite.modulation = color32f32(1.0f, 1.0f, 1.0f, 1.0f);
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
        position.y = animation_start_position.y + (t_since_spawn-PICKUP_ENTITY_ANIMATION_T_LENGTH) * 100; /*+ sinf(t_since_spawn) * (scale.y);*/
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
        0.125
    );
    Entity::update(state, dt);
}

void Pickup_Entity::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    if (!visible)
        return;

#if 0 // NOTE(jerry): To keep performance higher, this vignette should be replaced with something placed within the atlas
      // for now, I cannot in good conscience keep this here, since it increases draw-call count.
    auto  sprite_img = graphics_assets_get_image_by_id(&resources->graphics_assets, resources->ui_border_vignette);
    V2    sprite_image_size = V2(32, 32) * (1 + normalized_sinf(Global_Engine()->global_elapsed_time*1.25 + 1234) * 0.75);

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
            sprite.modulation.a * 0.25
        ),
        V2(0, 0),
        0,
        0,
        BLEND_MODE_ADDITIVE
    );
#endif
    sprite.modulation.a = 1.0f - lifetime.percentage();
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
            /*unimplemented("pick_attack_power_entity not needed yet?");*/
            state->gameplay_data.player.burst_charge += value;
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

#define DEATH_EXPLOSION_SPRITE_ANIMATION_TIME (0.045f)

void DeathExplosion::update(Game_State* state, f32 dt) {
  if (timer >= DEATH_EXPLOSION_SPRITE_ANIMATION_TIME) {
    frame_index++;
    timer = 0;
  }
  else {
    timer += dt;
  }
}

// NOTE(jerry): hard-coded
void DeathExplosion::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
  if (dead()) {
    return;
  }

  image_id frame = resources->explosion_image[frame_index];
  image_buffer* sprite_img = graphics_assets_get_image_by_id(&resources->graphics_assets, frame);

  V2 sprite_image_size = V2(64, 64) * scale;
  V2 sprite_position = position - sprite_image_size/2;

  render_commands_push_image_ext(
    render_commands,
    sprite_img,
    rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
    RECTANGLE_F32_NULL,
    color32f32(
      1,
      1,
      1,
      1
    ),
    V2(0, 0),
    0,
    0,
    BLEND_MODE_ADDITIVE
  );
}

bool DeathExplosion::dead(void) {
  return frame_index >= frame_count; // hardcoded;
}

// Simple scrollable background entities
Simple_Scrollable_Background_Entities::Simple_Scrollable_Background_Entities(
    Memory_Arena* arena,
    s32 count
)
{
    s32 padded_count = (count + 7) & ~7;
    backgrounds      = Fixed_Array<Simple_Scrollable_Background_Entity_Main_Data>(arena, padded_count);
    scroll_xs        = Fixed_Array<f32>(arena, padded_count);
    scroll_ys        = Fixed_Array<f32>(arena, padded_count);
    scroll_speed_xs  = Fixed_Array<f32>(arena, padded_count);
    scroll_speed_ys  = Fixed_Array<f32>(arena, padded_count);
}

void Simple_Scrollable_Background_Entities::clear(void) {
    scroll_xs.zero();
    scroll_ys.zero();
    scroll_speed_xs.zero();
    scroll_speed_ys.zero();
    backgrounds.zero();

    foreground_start = nullptr;
    background_start = nullptr;
    foreground_end = nullptr;
    background_end = nullptr;
}

void Simple_Scrollable_Background_Entities::update(f32 dt) {
    // please vectorize!
    s32 count = scroll_ys.size;
    for (s32 index = 0; index < count; ++index) {
        scroll_xs[index] += scroll_speed_xs[index];
        scroll_ys[index] += scroll_speed_ys[index];
    }
}

void Simple_Scrollable_Background_Entities::draw_list(
    Simple_Scrollable_Background_Entity_Main_Data* list,
    struct render_commands* render_commands,
    Game_Resources* resources
)
{
    for (Simple_Scrollable_Background_Entity_Main_Data* cursor = list;
         cursor;
         cursor = cursor->next) {
        s32 index = (s32)(cursor - backgrounds.data);
        f32 scroll_x = scroll_xs[index];
        f32 scroll_y = scroll_ys[index];

        auto destination_rect =
            rectangle_f32(
                0,
                0,
                cursor->scale.x,
                cursor->scale.y
            );
        auto src_rect =
            rectangle_f32(
                scroll_x,
                scroll_y,
                SIMPLE_BACKGROUND_WIDTH,
                SIMPLE_BACKGROUND_HEIGHT
            );
        render_commands_push_image_ext2(
            render_commands,
            graphics_assets_get_image_by_id(&resources->graphics_assets, cursor->image_id),
            destination_rect,
            src_rect,
            color32f32(1,1,1,1), // modulation is always white for these guys
            V2(0,0), // rotation_centre
            0, // unused angle_z
            0, // unused angle_y
            // unused angle_x
            0, // flags param not used
            // blend mode param not used
            BLEND_MODE_ALPHA
        );
    }
}

void Simple_Scrollable_Background_Entities::draw_foreground(
    struct render_commands* render_commands,
    Game_Resources* resources
)
{
    draw_list(foreground_start, render_commands, resources);
}

void Simple_Scrollable_Background_Entities::draw_background(
    struct render_commands* render_commands,
    Game_Resources* resources
)
{
    draw_list(background_start, render_commands, resources);
}

Simple_Scrollable_Background_Entity_Bundle
Simple_Scrollable_Background_Entities::allocate_background(s32 layer)
{
    Simple_Scrollable_Background_Entity_Bundle result = {};

    // NOTE(jerry): all parallel
    auto current                = backgrounds.alloc();
    auto current_scroll_speed_x = scroll_speed_xs.alloc();
    auto current_scroll_speed_y = scroll_speed_ys.alloc();
    auto current_scroll_x       = scroll_xs.alloc();
    auto current_scroll_y       = scroll_ys.alloc();

    switch (layer) {
        case SIMPLE_SCROLLABLE_BACKGROUND_ENTITY_LAYER_BACKGROUND: {
            if (!background_start) {
                background_start = background_end = current;
                background_end->next = nullptr;
            } else {
                background_end->next = current;
                background_end = current;
            }
        } break;
        case SIMPLE_SCROLLABLE_BACKGROUND_ENTITY_LAYER_FOREGROUND: {
            if (!foreground_start) {
                foreground_start = foreground_end = current;
                foreground_end->next = nullptr;
            } else {
                foreground_end->next = current;
                foreground_end = current;
            }
        } break;
    }

    // AssembleDataBundle()
    {
        result.image_id       = &current->image_id;
        result.scale          = &current->scale;
        result.scroll_x       = current_scroll_x;
        result.scroll_y       = current_scroll_y;
        result.scroll_speed_x = current_scroll_speed_x;
        result.scroll_speed_y = current_scroll_speed_y;
    }

    return result;
}

Simple_Scrollable_Background_Entity_Bundle
Simple_Scrollable_Background_Entities::get_background(s32 index)
{
    Simple_Scrollable_Background_Entity_Bundle result = {}; 
    auto background_data = &backgrounds[index];
    auto scroll_x        = &scroll_xs[index];
    auto scroll_y        = &scroll_ys[index];
    auto scroll_speed_x  = &scroll_speed_xs[index];
    auto scroll_speed_y  = &scroll_speed_ys[index];
    {
        result.image_id       = &background_data->image_id;
        result.scale          = &background_data->scale;
        result.scroll_x       = scroll_x;
        result.scroll_y       = scroll_y;
        result.scroll_speed_x = scroll_speed_x;
        result.scroll_speed_y = scroll_speed_y;
    }
    return result;
}

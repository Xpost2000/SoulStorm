#include "action_mapper.h"

#include "entity.h"
#include "input.h"

#include "game_state.h"

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
        position.x = scale.x;
        return true;
    }
    return false;
}

bool Entity::clamp_to_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area)) {
        velocity.x = 0;
        position.x = play_area.width - scale.x;
        return true;
    }
    return false;
}

bool Entity::clamp_to_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area)) {
        velocity.y = 0;
        position.y = scale.y;
        return true;
    }
    return false;
}

bool Entity::clamp_to_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area)) {
        velocity.y = 0;
        position.y = play_area.height - scale.y;
        return true;
    }
    return false;
}

bool Entity::wrap_from_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area, true)) {
        position.x = play_area.width;
        return true;
    }
    return false;
}

bool Entity::wrap_from_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area, true)) {
        position.x = 0;
        return true;
    }
    return false;
}

bool Entity::wrap_from_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area, true)) {
        position.y = play_area.height;
        return true;
    }
    return false;
}

bool Entity::wrap_from_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area, true)) {
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
    auto r = get_rect();

    if (sprite.id.index != 0) {
        // NOTE: entity updates are responsible for actually
        //       animating their own sprites!
        auto sprite_object   = graphics_get_sprite_by_id(&resources->graphics_assets, sprite.id);
        auto sprite_frame    = sprite_get_frame(sprite_object, sprite.frame);
        auto sprite_img = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
        V2 sprite_image_size = V2(sprite_img->width, sprite_img->height);

        // NOTE: provided sprites are 32x32
        sprite_image_size.x *= sprite.scale.x; sprite_image_size.y *= sprite.scale.y;

        for (s32 trail_ghost_index = 0; trail_ghost_index < trail_ghost_count; ++trail_ghost_index) {
            auto& ghost = trail_ghosts[trail_ghost_index];
            V2 sprite_position = V2(
                ghost.position.x + (sprite.offset.x - sprite_image_size.x/2),
                ghost.position.y + (sprite.offset.y - sprite_image_size.y/2)
            );

            auto modulation = trail_ghost_modulation;
            modulation.a *= (ghost.alpha/trail_ghost_max_alpha);

            render_commands_push_image(
                render_commands,
                sprite_img,
                rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                sprite_frame->source_rect,
                modulation,
                0,
                BLEND_MODE_ALPHA
            );
        }

        V2 sprite_position = V2(
            position.x + (sprite.offset.x - sprite_image_size.x/2),
            position.y + (sprite.offset.y - sprite_image_size.y/2)
        );
        render_commands_push_image(
            render_commands,
            sprite_img,
            rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
            sprite_frame->source_rect,
            color32f32(1.0, 1.0, 1.0, 1.0),
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
                color32f32(1.0, 1.0, 1.0, 1.0),
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

bool Entity::attack() {
    if (!firing) {
        firing = true;
        firing_t = 0;
    }

    return firing_t <= 0.0f;
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

void Entity::update_firing_behavior(f32 dt) {
    if (firing) {
        if (firing_t <= 0.0f) {
            firing_t = firing_cooldown;
        } else {
            firing_t -= dt;
        }
    } else {
        firing_t = 0;
    }
}

void Entity::update(Game_State* state, f32 dt) {
    const auto& play_area = state->gameplay_data.play_area;

    update_ghost_trails(dt);
    handle_out_of_bounds_behavior(play_area, dt);
    handle_invincibility_behavior(dt);
    update_firing_behavior(dt);

    position           += velocity * dt;
    t_since_spawn      += dt;
}

rectangle_f32 Entity::get_rect() {
    return rectangle_f32(
        position.x - scale.x,
        position.y - scale.y,
        scale.x*2,
        scale.y*2
    );
}

// PlayerActor
s32 Player::currently_grazing(Game_State* state) {
    s32 grazed_bullets = 0;
    for (s32 bullet_index = 0; bullet_index < state->gameplay_data.bullets.size; ++bullet_index) {
        auto& b = state->gameplay_data.bullets[bullet_index];

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
    const auto& play_area = state->gameplay_data.play_area;
    // unfortunately the action mapper system doesn't exist
    // here like it did in the last project, so I'll have to use key inputs
    // and gamepad power.
    bool firing   = Action::is_down(ACTION_ACTION);
    bool focusing = Action::is_down(ACTION_FOCUS);
    bool use_bomb = Action::is_pressed(ACTION_USE_BOMB);

    under_focus = focusing;

    V2 axes = V2(Action::value(ACTION_MOVE_LEFT) + Action::value(ACTION_MOVE_RIGHT), Action::value(ACTION_MOVE_UP) + Action::value(ACTION_MOVE_DOWN));
    if (axes.magnitude_sq() > 1.0f) axes = axes.normalized();

    float UNIT_SPEED = (under_focus) ? 225 : 325;

    velocity.x = axes[0] * UNIT_SPEED;
    velocity.y = axes[1] * UNIT_SPEED;

    Entity::update(state, dt);

    handle_play_area_edge_behavior(play_area);

    firing_cooldown = (under_focus) ? (DEFAULT_FIRING_COOLDOWN/2) : DEFAULT_FIRING_COOLDOWN;

    if (firing) {
        // okay these are normal real bullets
        if (attack()) {
            controller_rumble(Input::get_gamepad(0), 0.25f, 0.63f, 100);
            if (under_focus) {
                spawn_bullet_arc_pattern1(state, position, 3, 15, V2(5, 5), V2(0, -1), 1000.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_STROBING);
            } else {
                // spawn_bullet_line_pattern1(state, position, 1, 20.0f, V2(5, 5), V2(0, -1), 1550.0f, BULLET_SOURCE_PLAYER);
                
                spawn_bullet_arc_pattern1(state, position, 3, 45, V2(5, 5), V2(0, -1), 650.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_ELECTRIC);
            }
        }
    }

    if (use_bomb) {
        _debugprintf("Use bomb!");
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
    if (awarded || (!seek_towards_player && lifetime.triggered())) {
        die = true;  
        return;
    }

    if (!seek_towards_player) {
        default_behavior_update(state, dt);
    } else {
        chase_player_update(state, dt);
    }

    Entity::update(state, dt);
}

void Pickup_Entity::on_picked_up(Game_State* state) {
    if (!seek_towards_player) {
        if (awarded || die)
            return;
        if (animation_t < PICKUP_ENTITY_ANIMATION_T_LENGTH)
            return;
    }

    awarded = true;

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

// BEGIN_LUA_BINDINGS
/*
* 
* Start of all the lua bindings. They're relatively
* straight forward procedural bindings to lua.
* 
* They're generally formatted so as to provide a simple DSL
* to make levels...
* 
* There's just a lot of typing here, and not many ways to avoid it
* to be honest without dragging in a giant lua binding library.
* 
* NOTE: most, if not all of this is not actually error checked because that's
* an obscene additional amount of code. If the level script is bad. I'm okay with it
* crashing...
* 
*/

int _lua_bind_spawn_bullet_arc_pattern2(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    spawn_bullet_arc_pattern2(
        state,
        V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)),
        luaL_checkinteger(L, 3),
        luaL_checkinteger(L, 4),
        V2(luaL_checknumber(L, 5), luaL_checknumber(L, 6)),
        V2(luaL_checknumber(L, 7), luaL_checknumber(L, 8)),
        luaL_checknumber(L, 9),
        luaL_checknumber(L, 10),
        luaL_checkinteger(L, 11),
        luaL_checkinteger(L, 12)
    );
    return 0;
}

// ENEMY ENTITY

int _lua_bind_enemy_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = enemy_generic( state, V2(0,0), V2(10,10), nullptr);
    state->gameplay_data.add_enemy_entity(e);
    lua_pushinteger(L, e.uid);
    return 1;
}

int _lua_bind_enemy_time_since_spawn(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->t_since_spawn);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_valid(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    lua_pushboolean(L, e != nullptr);
    return 1;
}

int _lua_bind_enemy_set_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->position.x = luaL_checknumber(L, 2);
        e->position.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->scale.x = luaL_checknumber(L, 2);
        e->scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_velocity(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->velocity.x = luaL_checknumber(L, 2);
        e->velocity.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_acceleration(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->acceleration.x = luaL_checknumber(L, 2);
        e->acceleration.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->max_hp = e->hp = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_set_task(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    char* task_name = (char*)lua_tostring(L, 2);

    /*
     * NOTE: not sure how to do this right now.
     * I would like to copy items between stacks.
     */
    Lua_Task_Extra_Parameter_Variant extra_parameters[128] = {};
    s32 remaining = lua_gettop(L)-3;

    for (s32 index = 0; index < remaining; ++index) {
        s32 stack_index = 3 + index;
        if (lua_isnumber(L, stack_index)) extra_parameters[index] = ltep_variant_number(lua_tonumber(L, stack_index));
        else if (lua_isstring(L, stack_index)) extra_parameters[index] = ltep_variant_string((char*)lua_tostring(L, stack_index));
        else if (lua_isinteger(L, stack_index)) extra_parameters[index] = ltep_variant_integer(lua_tointeger(L, stack_index));
        else if (lua_isboolean(L, stack_index)) extra_parameters[index] = ltep_variant_boolean(lua_toboolean(L, stack_index));
    }

    state->coroutine_tasks.add_enemy_lua_game_task(state, state->coroutine_tasks.L, task_name, uid, make_slice(extra_parameters, remaining));
    return 0;
}

int _lua_bind_enemy_reset_movement(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->reset_movement();
    }
    return 0;
}

int _lua_bind_enemy_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_acceleration_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_acceleration_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushinteger(L, e->hp);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_hp_percent(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->hp_percentage());
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    lua_pushlightuserdata(L, e);
    return 1;
}

int _lua_bind_enemy_start_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_limit = luaL_checkinteger(L, 2);
    }
    return 0;
}
int _lua_bind_enemy_set_trail_modulation(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_modulation = color32f32(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), 1);
        e->trail_ghost_max_alpha = luaL_checknumber(L, 5);
    }
    return 0;
}
int _lua_bind_enemy_set_trail_record_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_record_timer_max = luaL_checknumber(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_stop_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_limit = 0;
    }
    return 0;
}

int _lua_bind_enemy_kill(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->kill();
    }
    return 0;
}

int _lua_bind_enemy_hurt(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->damage(luaL_checkinteger(L, 2));
    }
    return 0;
}

int _lua_bind_enemy_begin_invincibility(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->begin_invincibility(lua_toboolean(L, 2), luaL_checknumber(L, 3));
    }
    return 0;
}

int _lua_bind_enemy_end_invincibility(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->end_invincibility();
    }
    return 0;
}

int _lua_bind_enemy_set_oob_deletion(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        e->allow_out_of_bounds_survival = lua_toboolean(L, 2);
    }

    return 0;
}

int _lua_bind_enemy_show_boss_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        state->gameplay_data.boss_health_displays.add(
            uid,
            string_from_cstring((char*)lua_tostring(L, 2))
        );
    }

    return 0;
}

int _lua_bind_enemy_hide_boss_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        state->gameplay_data.boss_health_displays.remove(uid);
    }

    return 0;
}
// BULLET ENTITY
int _lua_bind_bullet_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = bullet_generic( state, V2(0,0), V2(10,10), BULLET_SOURCE_NEUTRAL, nullptr, PROJECTILE_SPRITE_BLUE);
    state->gameplay_data.add_bullet(e);
    lua_pushinteger(L, e.uid);
    return 1;
}

int _lua_bind_bullet_set_source(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->source_type = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_bullet_time_since_spawn(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->t_since_spawn);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_valid(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    lua_pushboolean(L, e != nullptr);
    return 1;
}

int _lua_bind_bullet_set_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->position.x = luaL_checknumber(L, 2);
        e->position.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->scale.x = luaL_checknumber(L, 2);
        e->scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_visual_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->sprite.scale.x = luaL_checknumber(L, 2);
        e->sprite.scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_velocity(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->velocity.x = luaL_checknumber(L, 2);
        e->velocity.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_acceleration(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->acceleration.x = luaL_checknumber(L, 2);
        e->acceleration.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_task(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    char* task_name = (char*)lua_tostring(L, 2);

    Lua_Task_Extra_Parameter_Variant extra_parameters[128] = {};
    s32 remaining = lua_gettop(L)-3;

    for (s32 index = 0; index < remaining; ++index) {
        s32 stack_index = 3 + index;
        if (lua_isnumber(L, stack_index)) extra_parameters[index] = ltep_variant_number(lua_tonumber(L, stack_index));
        else if (lua_isstring(L, stack_index)) extra_parameters[index] = ltep_variant_string((char*)lua_tostring(L, stack_index));
        else if (lua_isinteger(L, stack_index)) extra_parameters[index] = ltep_variant_integer(lua_tointeger(L, stack_index));
        else if (lua_isboolean(L, stack_index)) extra_parameters[index] = ltep_variant_boolean(lua_toboolean(L, stack_index));
    }

    state->coroutine_tasks.add_bullet_lua_game_task(state, state->coroutine_tasks.L, task_name, uid, make_slice(extra_parameters, remaining));
    return 0;
}

int _lua_bind_bullet_reset_movement(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->reset_movement();
    }
    return 0;
}

int _lua_bind_bullet_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_acceleration_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_acceleration_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    lua_pushlightuserdata(L, e);
    return 1;
}

int _lua_bind_bullet_start_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_limit = luaL_checkinteger(L, 2);
    }
    return 0;
}
int _lua_bind_bullet_set_trail_modulation(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_modulation = color32f32(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), 1);
        e->trail_ghost_max_alpha = luaL_checknumber(L, 5);
    }
    return 0;
}
int _lua_bind_bullet_set_trail_record_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_record_timer_max = luaL_checknumber(L, 2);
    }
    return 0;
}

int _lua_bind_bullet_stop_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_limit = 0;
    }
    return 0;
}

int _lua_bind_bullet_set_visual(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->sprite = sprite_instance(
            state->resources->projectile_sprites[
                luaL_checkinteger(L, 2)
            ]
        );
    }
    return 0; 
}

int _lua_bind_bullet_set_lifetime(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->lifetime = Timer(luaL_checknumber(L, 2));
    }
    return 0; 
}
int _lua_bind_bullet_lifetime(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.t);
        return 1;
    }
    return 0; 
}
int _lua_bind_bullet_lifetime_max(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.max_t);
        return 1;
    }
    return 0; 
}
int _lua_bind_bullet_lifetime_percent(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.percentage());
        return 1;
    }
    return 0; 
}

// Misc entities
int _lua_bind_explosion_hazard_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    V2 position = V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    f32 radius = luaL_checknumber(L, 3);
    f32 warning_time = luaL_checknumber(L, 4);
    f32 time_until_explosion = luaL_checknumber(L, 5);

    state->gameplay_data.add_explosion_hazard(
        Explosion_Hazard(
            position,
            radius,
            warning_time,
            time_until_explosion
        )
    );
    return 0;
}

int _lua_bind_laser_hazard_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    f32 position = luaL_checknumber(L, 1);
    f32 radius = luaL_checknumber(L, 2);
    s32 direction = luaL_checkinteger(L, 3);
    f32 warning_time = luaL_checknumber(L, 4);
    f32 lifetime = luaL_checknumber(L, 5);

    state->gameplay_data.add_laser_hazard(
        Laser_Hazard(
            position,
            radius,
            direction,
            warning_time,
            lifetime
        )
    );
    return 0;
}

// Player bindings
int _lua_bind_player_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    lua_pushlightuserdata(L, e);
    return 1;
}
int _lua_bind_player_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->hp);
        return 1;
    }
    return 0;
}
int _lua_bind_player_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_player_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_player_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_player_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

void bind_entity_lualib(lua_State* L) {
    /*
        NOTE: the lib is only in scalar values which isn't very good, but it's
            simple and flexible enough to play with.

        Everything will probably remain as individual parameters since it requires less library/parsing
        work, but there are bindings to the math functions inside this engine...

        NOTE: I am distinctly aware that I could make these functions more generic if I just "buff" the uid,
        but I like being more explicit for stuff.

        Technically speaking bullets and enemies are actually extremely similar...
     */
    {
        // behavior setting
        lua_register(L, "_enemy_to_ptr",             _lua_bind_enemy_to_ptr);
        lua_register(L, "enemy_new",                 _lua_bind_enemy_new);
        lua_register(L, "enemy_valid",               _lua_bind_enemy_valid);
        lua_register(L, "enemy_set_position",        _lua_bind_enemy_set_position);
        lua_register(L, "enemy_set_scale",           _lua_bind_enemy_set_scale);
        lua_register(L, "enemy_set_velocity",        _lua_bind_enemy_set_velocity);
        lua_register(L, "enemy_set_acceleration",    _lua_bind_enemy_set_acceleration);
        lua_register(L, "enemy_set_hp",              _lua_bind_enemy_set_hp);
        lua_register(L, "enemy_set_task",            _lua_bind_enemy_set_task);
        lua_register(L, "enemy_reset_movement",      _lua_bind_enemy_reset_movement);
        lua_register(L, "enemy_kill",                _lua_bind_enemy_kill);
        lua_register(L, "enemy_hurt",                _lua_bind_enemy_hurt);
        lua_register(L, "enemy_start_trail",         _lua_bind_enemy_start_trail);
        lua_register(L, "enemy_set_trail_modulation",         _lua_bind_enemy_set_trail_modulation);
        lua_register(L, "enemy_set_trail_record_speed",          _lua_bind_enemy_set_trail_record_speed);
        lua_register(L, "enemy_stop_trail",             _lua_bind_enemy_stop_trail);
        lua_register(L, "enemy_begin_invincibility", _lua_bind_enemy_begin_invincibility);
        lua_register(L, "enemy_end_invincibility",   _lua_bind_enemy_end_invincibility);
        lua_register(L, "enemy_set_oob_deletion",    _lua_bind_enemy_set_oob_deletion);

        lua_register(L, "enemy_show_boss_hp",    _lua_bind_enemy_show_boss_hp);
        lua_register(L, "enemy_hide_boss_hp",    _lua_bind_enemy_hide_boss_hp);

        // reading
        lua_register(L, "enemy_time_since_spawn",       _lua_bind_enemy_time_since_spawn);
        lua_register(L, "enemy_position_x", _lua_bind_enemy_position_x);
        lua_register(L, "enemy_position_y", _lua_bind_enemy_position_y);
        lua_register(L, "enemy_velocity_x", _lua_bind_enemy_velocity_x);
        lua_register(L, "enemy_velocity_y", _lua_bind_enemy_velocity_y);
        lua_register(L, "enemy_acceleration_x", _lua_bind_enemy_acceleration_x);
        lua_register(L, "enemy_acceleration_y", _lua_bind_enemy_acceleration_y);
        lua_register(L, "enemy_hp",         _lua_bind_enemy_hp);
        lua_register(L, "enemy_hp_percent", _lua_bind_enemy_hp_percent);

        // bullet behavior setting. (there is no reason to read from a bullet)
        lua_register(L, "_bullet_ptr",                   _lua_bind_bullet_to_ptr);
        lua_register(L, "bullet_new",                    _lua_bind_bullet_new);
        lua_register(L, "bullet_set_source",             _lua_bind_bullet_set_source);
        lua_register(L, "bullet_time_since_spawn",       _lua_bind_bullet_time_since_spawn);
        lua_register(L, "bullet_valid",                  _lua_bind_bullet_valid);
        lua_register(L, "bullet_set_position",           _lua_bind_bullet_set_position);
        lua_register(L, "bullet_set_scale",              _lua_bind_bullet_set_scale);
        lua_register(L, "bullet_set_visual_scale",       _lua_bind_bullet_set_visual_scale);
        lua_register(L, "bullet_set_velocity",           _lua_bind_bullet_set_velocity);
        lua_register(L, "bullet_set_acceleration",       _lua_bind_bullet_set_acceleration);
        lua_register(L, "bullet_start_trail",            _lua_bind_bullet_start_trail);
        lua_register(L, "bullet_set_trail_modulation",   _lua_bind_bullet_set_trail_modulation);
        lua_register(L, "bullet_set_trail_record_speed", _lua_bind_bullet_set_trail_record_speed);
        lua_register(L, "bullet_stop_trail",             _lua_bind_bullet_stop_trail);
        lua_register(L, "bullet_set_visual",             _lua_bind_bullet_set_visual);
        lua_register(L, "bullet_set_task",               _lua_bind_bullet_set_task);
        lua_register(L, "bullet_reset_movement",         _lua_bind_bullet_reset_movement);
        lua_register(L, "bullet_set_lifetime",   _lua_bind_bullet_set_lifetime);

        // These might be okay to read for a bullet.
        lua_register(L, "bullet_lifetime",   _lua_bind_bullet_lifetime);
        lua_register(L, "bullet_lifetime_max",   _lua_bind_bullet_lifetime_max);
        lua_register(L, "bullet_lifetime_percent",   _lua_bind_bullet_lifetime_percent);

        // Player is READONLY. All things done to the player are only through the engine code.
        lua_register(L, "_player_ptr",       _lua_bind_player_to_ptr);
        lua_register(L, "player_position_x", _lua_bind_player_position_x);
        lua_register(L, "player_position_y", _lua_bind_player_position_y);
        lua_register(L, "player_velocity_x", _lua_bind_player_velocity_x);
        lua_register(L, "player_velocity_y", _lua_bind_player_velocity_y);
        lua_register(L, "player_hp",         _lua_bind_player_hp);

        lua_register(L, "explosion_hazard_new", _lua_bind_explosion_hazard_new);
        lua_register(L, "laser_hazard_new",     _lua_bind_laser_hazard_new);
    }
}

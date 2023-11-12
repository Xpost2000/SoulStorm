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
        _debugprintf("I am invincible. You cannot hurt me.");
        return false;
    }
    _debugprintf("ouchie (%d dmg)", dmg);
    hp -= dmg;
    if (hp <= 0) die = true;

    return true;
}

bool Entity::heal(s32 hp) {
    _debugprintf("healed (%d hp)", hp);
    hp += hp;
    if (hp > 0) die = false;

    return true;
}

bool Entity::kill() {
    _debugprintf("trying to die");
    return damage(9999999); // lol
}

void Entity::handle_play_area_edge_behavior(const Play_Area& play_area) {
    s32 edge_behavior_top    = edge_top_behavior_override;
    s32 edge_behavior_bottom = edge_bottom_behavior_override;
    s32 edge_behavior_left   = edge_left_behavior_override;
    s32 edge_behavior_right  = edge_right_behavior_override;

    if (edge_behavior_top == -1) edge_behavior_top       = play_area.edge_behaviors[0];
    if (edge_behavior_bottom == -1) edge_behavior_bottom = play_area.edge_behaviors[1];
    if (edge_behavior_left == -1) edge_behavior_left     = play_area.edge_behaviors[2];
    if (edge_behavior_right == -1) edge_behavior_right   = play_area.edge_behaviors[3];

    // NOTE: these behaviors are mostly for the players... Enemies will just act
    // as if it was "wrapping" behavior by default.

    // refactor this to maybe be a little less unwieldy, but it's not too big of a deal
    // since this will not change much afterwards anyways...
    switch (edge_behavior_top) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_top_border(play_area))
                if (play_area.edge_behaviors[0] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    kill();
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_top_border(play_area);
        } break;
        case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
    }

    switch (edge_behavior_bottom) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_bottom_border(play_area))
                if (play_area.edge_behaviors[1] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    kill();
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_bottom_border(play_area);
        } break;
        case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
    }

    switch (edge_behavior_left) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_left_border(play_area))
                if (play_area.edge_behaviors[2] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    kill();
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_left_border(play_area);
        } break;
        case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
    }

    switch (edge_behavior_right) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_right_border(play_area))
                if (play_area.edge_behaviors[3] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    die = true;
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_right_border(play_area);
        } break;
        case PLAY_AREA_EDGE_PASSTHROUGH: {} break;
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
        sprite_image_size.x *= sprite.scale.x; sprite_image_size.y *= sprite.scale.y;


        for (s32 trail_ghost_index = 0; trail_ghost_index < trail_ghost_count; ++trail_ghost_index) {
            auto& ghost = trail_ghosts[trail_ghost_index];
            V2 sprite_position = V2(
                ghost.position.x - scale.x/2 + (sprite.offset.x - sprite_image_size.x/2),
                ghost.position.y - scale.x/2 + (sprite.offset.y - sprite_image_size.y/2)
            );

            auto modulation = trail_ghost_modulation;
            modulation.a *= (ghost.alpha/trail_ghost_max_alpha);

            render_commands_push_image(
                render_commands,
                sprite_img,
                rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
                // sprite_frame->source_rect,
                sprite_frame->source_rect,
                modulation,
                0,
                BLEND_MODE_ALPHA
            );
        }

        V2 sprite_position = V2(
            position.x - scale.x/2 + (sprite.offset.x - sprite_image_size.x/2),
            position.y - scale.x/2 + (sprite.offset.y - sprite_image_size.y/2)
        );
        render_commands_push_image(
            render_commands,
            sprite_img,
            rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
            // sprite_frame->source_rect,
            sprite_frame->source_rect,
            color32f32(1.0, 1.0, 1.0, 1.0),
            0,
            BLEND_MODE_ALPHA
        );
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
}

bool Entity::attack() {
    if (!firing) {
        firing = true;
        firing_t = 0;
    }

    return firing_t <= 0.0f;
}

void Entity::update(Game_State* state, f32 dt) {
    const auto& play_area = state->gameplay_data.play_area;

    // update ghost trails
    {
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

    position           += velocity * dt;
    t_since_spawn      += dt;

    {
        auto rect = get_rect();
        if (!play_area.is_inside_logical(rect)) {
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

    if (invincibility_time.running) {
        invincibility_time_flash_period.start();
        if (invincibility_time_flash_period.triggered()) {
            _debugprintf("flash");
            flashing ^= true;
            invincibility_time_flash_period.reset();
        }
        invincibility_time.triggered();
    } else {
        flashing = false;
    }

    // firing timing logic
    if (firing) {
        if (firing_t <= 0.0f) {
            firing_t = firing_cooldown;
        } else {
            firing_t -= dt;
        }
    } else {
        firing_t = 0;
    }

    invincibility_time_flash_period.update(dt);
    invincibility_time.update(dt);
    // NOTE: until I have more time to implement sub behaviors on the play area
    // for now the default behavior is clamping
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
void Player::update(Game_State* state, f32 dt) {
    const auto& play_area = state->gameplay_data.play_area;
    // unfortunately the action mapper system doesn't exist
    // here like it did in the last project, so I'll have to use key inputs
    // and gamepad power.
    bool firing   = Action::is_down(ACTION_ACTION);
    bool focusing = Action::is_down(ACTION_FOCUS);

    under_focus = focusing;

    V2 axes = V2(Action::value(ACTION_MOVE_LEFT) + Action::value(ACTION_MOVE_RIGHT), Action::value(ACTION_MOVE_UP) + Action::value(ACTION_MOVE_DOWN));
    if (axes.magnitude_sq() > 1.0f) axes = axes.normalized();

    float UNIT_SPEED = (under_focus) ? 225 : 550;

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
                spawn_bullet_arc_pattern1(state, position, 3, 15, V2(5, 5), V2(0, -1), 1000.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_NEGATIVE_STROBING);
            } else {
                // spawn_bullet_line_pattern1(state, position, 1, 20.0f, V2(5, 5), V2(0, -1), 1550.0f, BULLET_SOURCE_PLAYER);
                
                spawn_bullet_arc_pattern1(state, position, 3, 45, V2(5, 5), V2(0, -1), 650.0f, BULLET_SOURCE_PLAYER, PROJECTILE_SPRITE_BLUE_ELECTRIC);
            }
        }
    }
}

// BulletEntity
void Bullet::update(Game_State* state, f32 dt) {
    const auto& play_area = state->gameplay_data.play_area;

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

    if (velocity_function) {
        velocity = V2(0, 0);
        velocity_function(this, state, dt);
    } else {
        velocity += acceleration * dt;
    }

    sprite.animate(
        &state->resources->graphics_assets,
        dt,
        0.035
    );

    Entity::update(state, dt);
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

int _lua_bind_spawn_bullet_arc_pattern2(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));

    // if (lua_gettop(L) != 12) // ?
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

int _lua_bind_enemy_new(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto e = enemy_generic( state, V2(0,0), V2(10,10), nullptr);
    state->gameplay_data.add_enemy_entity(e);
    lua_pushinteger(L, e.uid);
    return 1;
}

int _lua_bind_enemy_valid(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    lua_pushboolean(L, e != nullptr);
    return 1;
}

int _lua_bind_enemy_set_position(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->position.x = luaL_checknumber(L, 2);
        e->position.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_scale(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->scale.x = luaL_checknumber(L, 2);
        e->scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_velocity(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->velocity.x = luaL_checknumber(L, 2);
        e->velocity.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_acceleration(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->acceleration.x = luaL_checknumber(L, 2);
        e->acceleration.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_hp(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->hp = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_set_task(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    char* task_name = (char*)lua_tostring(L, 2);
    _debugprintf("set task?");

    /*
     * NOTE: not sure how to do this right now.
     * I would like to copy items between stacks.
     */
    s32 remaining = lua_gettop(L)-3;
    _debugprintf("%d remaining items in the stack?", remaining);
    state->coroutine_tasks.add_enemy_lua_game_task(state, state->coroutine_tasks.L, task_name, uid, 0);
    return 0;
}

int _lua_bind_enemy_reset_movement(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->reset_movement();
    }
    return 0;
}

int _lua_bind_enemy_position_x(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_position_y(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_x(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_y(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_hp(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->hp);
        return 1;
    }
    return 0;
}

void bind_entity_lualib(lua_State* L) {
    {
        lua_register(L, "enemy_new", _lua_bind_enemy_new);
        lua_register(L, "enemy_valid", _lua_bind_enemy_valid);
        lua_register(L, "enemy_set_position", _lua_bind_enemy_set_position);
        lua_register(L, "enemy_set_scale", _lua_bind_enemy_set_scale);
        lua_register(L, "enemy_set_velocity", _lua_bind_enemy_set_velocity);
        lua_register(L, "enemy_set_acceleration", _lua_bind_enemy_set_acceleration);
        lua_register(L, "enemy_set_hp", _lua_bind_enemy_set_hp);
        lua_register(L, "enemy_set_task", _lua_bind_enemy_set_task);
        lua_register(L, "enemy_reset_movement", _lua_bind_enemy_reset_movement);

        lua_register(L, "enemy_position_x", _lua_bind_enemy_position_x);
        lua_register(L, "enemy_position_y", _lua_bind_enemy_position_y);
        lua_register(L, "enemy_velocity_x", _lua_bind_enemy_velocity_x);
        lua_register(L, "enemy_velocity_y", _lua_bind_enemy_velocity_y);
        lua_register(L, "enemy_hp", _lua_bind_enemy_hp);
        lua_register(L, "spawn_bullet_arc_pattern2", _lua_bind_spawn_bullet_arc_pattern2);
    }
}

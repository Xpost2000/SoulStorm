#include "entity.h"
#include "input.h"

#include"game_state.h"

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

void Timer::update(f32 dt) {
    if (running) {
        t += dt;
    }
}

void Timer::stop() {
    running = false; 
}

void Timer::reset() {
    _debugprintf("Timer reset.");
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

void Entity::handle_play_area_edge_behavior(const Play_Area& play_area) {
    // TODO: killing, since there is no concept of HP yet.
    // NOTE: these behaviors are mostly for the players... Enemies will just act
    // as if it was "wrapping" behavior by default.

    // refactor this to maybe be a little less unwieldy, but it's not too big of a deal
    // since this will not change much afterwards anyways...
    switch (play_area.edge_behaviors[0]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_top_border(play_area))
                if (play_area.edge_behaviors[0] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    die = true;
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_top_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[1]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_bottom_border(play_area))
                if (play_area.edge_behaviors[1] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    die = true;
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_bottom_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[2]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            if (clamp_to_left_border(play_area))
                if (play_area.edge_behaviors[2] == PLAY_AREA_EDGE_DEADLY) {
                    // extra killing code.
                    die = true;
                }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_left_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[3]) {
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
    }
}

void Entity::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    auto r = get_rect();

    // black rectangles for default
    render_commands_push_quad_ext(
        render_commands,
        rectangle_f32(r.x, r.y, r.w, r.h),
        color32u8(0, 0, 0, 255),
        V2(0.5, 0.5), 45,
        BLEND_MODE_ALPHA);

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
        BLEND_MODE_ALPHA);
}

void Entity::update(Game_State* const state, f32 dt) {
    const auto& play_area = state->play_area;

    position      += velocity * dt;
    t_since_spawn += dt;

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
void Player::update(Game_State* const state, f32 dt) {
    const auto& play_area = state->play_area;
    // unfortunately the action mapper system doesn't exist
    // here like it did in the last project, so I'll have to use key inputs
    // and gamepad power.
    auto gamepad = Input::get_gamepad(0);

    V2 axes = V2(
        1 * (Input::is_key_down(KEY_D) || Input::is_key_down(KEY_RIGHT)) + (-1) * (Input::is_key_down(KEY_A) || Input::is_key_down(KEY_LEFT)),
        1 * (Input::is_key_down(KEY_S) || Input::is_key_down(KEY_DOWN)) + (-1) * (Input::is_key_down(KEY_W) || Input::is_key_down(KEY_UP))
    );

    axes = axes.normalized();

    if (fabs(axes[0]) < fabs(gamepad->left_stick.axes[0])) {
        axes[0] = gamepad->left_stick.axes[0];
    }
    if (fabs(axes[1]) < fabs(gamepad->left_stick.axes[1])) {
        axes[1] = gamepad->left_stick.axes[1];
    }

    const float UNIT_SPEED = 350;

    velocity.x = axes[0] * UNIT_SPEED;
    velocity.y = axes[1] * UNIT_SPEED;

    Entity::update(state, dt);

    handle_play_area_edge_behavior(play_area);
}

// BulletEntity
void Bullet::update(Game_State* const state, f32 dt) {
    const auto& play_area = state->play_area;

    velocity = V2(0, 0);
    if (velocity_function) {
        velocity_function(this, state, dt);
    }

    Entity::update(state, dt);

    // NOTE: needs flags to determine whether it obeys play area restrictions.
    wrap_from_right_border(play_area);
    wrap_from_top_border(play_area);
    wrap_from_bottom_border(play_area);
    wrap_from_left_border(play_area);
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

void Explosion_Hazard::update(Game_State* const state, f32 dt) {
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

void Laser_Hazard::update(Game_State* const state, f32 dt) {
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

void Laser_Hazard::draw(Game_State* const state, struct render_commands* render_commands, Game_Resources* resources) {
    const auto& play_area = state->play_area;
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

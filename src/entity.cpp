#include "entity.h"
#include "input.h"

#include"game_state.h"

// Timer
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
    running = false;
    t = max_t;
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
bool Entity::touching_left_border(const Play_Area& play_area) {
    return (position.x-scale.x/2 < 0);
}

bool Entity::touching_right_border(const Play_Area& play_area) {
    return (position.x+scale.x/2 > play_area.width);
}

bool Entity::touching_top_border(const Play_Area& play_area) {
    return (position.y-scale.y/2 < 0);
}

bool Entity::touching_bottom_border(const Play_Area& play_area) {
    return (position.y+scale.y/2 >play_area.height);
}

void Entity::clamp_to_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area)) {
        position.x = scale.x/2;
    }
}

void Entity::clamp_to_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area)) {
        position.x = play_area.width - scale.x/2;
    }
}

void Entity::clamp_to_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area)) {
        position.y = scale.y/2;
    }
}

void Entity::clamp_to_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area)) {
        position.y = play_area.height - scale.y/2;
    }
}

void Entity::wrap_from_left_border(const Play_Area& play_area) {
    if (touching_left_border(play_area)) {
        position.x = play_area.width - scale.x/2;
    }
}

void Entity::wrap_from_right_border(const Play_Area& play_area) {
    if (touching_right_border(play_area)) {
        position.x = scale.x/2;
    }
}

void Entity::wrap_from_top_border(const Play_Area& play_area) {
    if (touching_top_border(play_area)) {
        position.y = play_area.height - scale.y/2;
    }
}

void Entity::wrap_from_bottom_border(const Play_Area& play_area) {
    if (touching_bottom_border(play_area)) {
        position.y = scale.y/2;
    }
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
            clamp_to_top_border(play_area);
            if (play_area.edge_behaviors[0] == PLAY_AREA_EDGE_DEADLY) {
                // extra killing code.
            }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_top_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[1]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            clamp_to_bottom_border(play_area);
            if (play_area.edge_behaviors[1] == PLAY_AREA_EDGE_DEADLY) {
                // extra killing code.
            }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_bottom_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[2]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            clamp_to_left_border(play_area);
            if (play_area.edge_behaviors[2] == PLAY_AREA_EDGE_DEADLY) {
                // extra killing code.
            }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_left_border(play_area);
        } break;
    }

    switch (play_area.edge_behaviors[3]) {
        case PLAY_AREA_EDGE_DEADLY:
        case PLAY_AREA_EDGE_BLOCKING: {
            clamp_to_right_border(play_area);
            if (play_area.edge_behaviors[3] == PLAY_AREA_EDGE_DEADLY) {
                // extra killing code.
            }
        } break;
        case PLAY_AREA_EDGE_WRAPPING: {
            wrap_from_right_border(play_area);
        } break;
    }
}

void Entity::draw(Game_State* const state, software_framebuffer* framebuffer, Game_Resources* resources) {
    const auto& play_area       = state->play_area;
    int         play_area_width = play_area.width;
    int         play_area_x     = play_area.x;

    auto r = get_rect();

    // black rectangles for default
    software_framebuffer_draw_quad(
        framebuffer,
        rectangle_f32(r.x + play_area_x, r.y, r.w, r.h),
        color32u8(0, 0, 0, 255),
        BLEND_MODE_ALPHA);

    // center point
    software_framebuffer_draw_quad(
        framebuffer,
        rectangle_f32(position.x + play_area_x-1, position.y-1, 2, 2),
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
        position.x - scale.x/2,
        position.y - scale.y/2,
        scale.x,
        scale.y
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
    velocity = V2(0, 0);
    if (velocity_function) {
        velocity_function(this, state, dt);
    }

    Entity::update(state, dt);
}

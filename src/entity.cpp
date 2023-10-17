#include "entity.h"
#include "input.h"

// Entity Base
void Entity::draw(software_framebuffer* framebuffer) {
    auto r = get_rect();
    // black rectangles for default
    software_framebuffer_draw_quad(framebuffer, rectangle_f32(r.x, r.y, r.w, r.h), color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);
}

void Entity::update(f32 dt) {
    position += velocity * dt;
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
void Player::update(f32 dt) {
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

    Entity::update(dt);
}

// BulletEntity
void Bullet::update(f32 dt) {
    velocity = V2(0, 0);
    Entity::update(dt);
}

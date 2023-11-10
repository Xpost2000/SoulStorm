#include "entity.h"
#include "game_state.h"

/*
  Here's a bunch of velocity functions that I compose together to make some interesting behaviors or patterns,
  with hopefully not much hassle. A lot of them are obvious, but it's just to make the
  pattern code more obvious to read.
*/
inline local V2 velocity_linear(V2 direction, f32 magnitude) {
    direction = direction.normalized();
    return direction * magnitude;
}

inline local V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, f32 accel_magnitude) {
    direction = direction.normalized();
    return direction * magnitude + (direction * (accel_magnitude * t));
}

inline local V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, V2 accel_direction, f32 accel_magnitude) {
    direction = direction.normalized();
    return direction * magnitude + (accel_direction * (accel_magnitude * t));
}

inline local V2 velocity_follow(Entity* entity) {
    return entity->velocity;
}

inline local V2 velocity_circle_orbit(f32 t, f32 spawn_theta, f32 r) {
    f32 point_in_cycle = t + spawn_theta;
    V2 result = V2(-sinf(point_in_cycle) * r, cos(point_in_cycle) * r);
    return result;
}

inline local V2 velocity_cyclic_sine(f32 t, V2 direction) {
    direction = direction.normalized();
    return direction * sinf(t);
}

inline local V2 velocity_cyclic_cosine(f32 t, V2 direction) {
    direction = direction.normalized();
    return direction * cosf(t);
}

Bullet bullet_generic(Game_State* state, V2 position, V2 scale, s32 source, Bullet_Entity_Velocity_Fn velocity) {
    Bullet bullet;
    bullet.position    = position;
    bullet.scale       = scale;
    //bullet.lifetime    = Timer(3.0f);
    bullet.source_type = source;

    bullet.velocity_function = velocity;
    state->gameplay_data.add_bullet(bullet);
    return bullet;
}

Bullet bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 source) {
    Bullet bullet = bullet_generic(state, position, V2(5, 5), source,
        [=](Bullet* self, Game_State* const state, f32 dt) {
        self->velocity += velocity_linear(direction, magnitude);
    }
    );
    bullet.lifetime = Timer(3.0f);
    return bullet;
}

Enemy_Entity enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed) {
    Enemy_Entity enemy = enemy_generic(
        state,
        position, scale, 0.055f,
        [direction, speed](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_linear(direction, speed);
        },
        [](Enemy_Entity* self, Game_State* state, f32 dt) {
            
        }
    );

    return enemy;
}

Enemy_Entity enemy_generic(Game_State* state, V2 position, V2 scale, f32 fire_cooldown, Enemy_Entity_Velocity_Fn velocity, Enemy_Entity_Fire_Fn fire) {
    Enemy_Entity enemy;

    enemy.position = position;
    enemy.scale    = scale;
    enemy.firing_cooldown = fire_cooldown;

    enemy.velocity_function = velocity;
    enemy.on_fire_function  = fire;

    enemy.edge_top_behavior_override = enemy.edge_bottom_behavior_override =
        enemy.edge_left_behavior_override = enemy.edge_right_behavior_override = PLAY_AREA_EDGE_PASSTHROUGH;

    return enemy;
}

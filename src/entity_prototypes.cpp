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

void spawn_bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 source) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);
    bullet.source_type = source;

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(direction.x * magnitude, direction.y * magnitude) + V2(direction.x * magnitude/2, direction.y * magnitude/2);
        };

    state->gameplay_data.add_bullet(bullet);
}

void spawn_bullet_circling_down_homing(Game_State* state, V2 position, f32 factor, f32 factor2, s32 source, V2 additional) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);
    bullet.source_type = source;

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->gameplay_data.player.velocity;
        };

    state->gameplay_data.add_bullet(bullet);
}

void spawn_bullet_circling_down_homing2(Game_State* state, V2 position, f32 factor, f32 factor2, s32 source, V2 additional) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);
    bullet.source_type = source;

    Timer until_release(2.0f);
    bool triggered = false;
    f32 t_hit = 0;

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) mutable {
            until_release.start();
            if (until_release.triggered()) {
                triggered = true;
                t_hit = self->t_since_spawn;
            }
            until_release.update(dt);

            if (triggered)
                self->velocity = additional + (additional.normalized() * 50 * self->t_since_spawn - t_hit);
            else
                self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->gameplay_data.player.velocity;
        };

    state->gameplay_data.add_bullet(bullet);
}

void spawn_bullet_circling_down(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);
    bullet.lifetime = Timer(3.0f);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            f32 t_extra = self->t_since_spawn;
            f32 t = (factor + t_extra);
            self->velocity = V2(-sinf(t) * factor2,
                                cos(t) * factor2) + additional;
        };

    state->gameplay_data.add_bullet(bullet);
}

void spawn_enemy_linear_movement_with_circling_down_attack(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed) {
    Enemy_Entity enemy;

    enemy.position = position;
    enemy.scale    = scale;
    enemy.firing_cooldown = 0.055f;

    enemy.velocity_function =
        [direction, speed](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity = direction.normalized() * speed;
        };

    enemy.on_fire_function =
        [](Enemy_Entity* self, Game_State* state, f32 dt){
            spawn_bullet_circling_down(state, self->position, 0.35f, 50.0f, V2(0, -100));
        };

    enemy.edge_top_behavior_override = enemy.edge_bottom_behavior_override =
        enemy.edge_left_behavior_override = enemy.edge_right_behavior_override = PLAY_AREA_EDGE_PASSTHROUGH;
    state->gameplay_data.add_enemy_entity(enemy);
}

void spawn_enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed) {
    Enemy_Entity enemy;

    enemy.position = position;
    enemy.scale    = scale;
    enemy.firing_cooldown = 0.055f;

    enemy.velocity_function =
        [direction, speed](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity = direction.normalized() * speed;
        };

    enemy.edge_top_behavior_override = enemy.edge_bottom_behavior_override =
        enemy.edge_left_behavior_override = enemy.edge_right_behavior_override = PLAY_AREA_EDGE_PASSTHROUGH;
    state->gameplay_data.add_enemy_entity(enemy);
}

void spawn_enemy_generic(Game_State* state, V2 position, V2 scale, f32 fire_cooldown, Enemy_Entity_Velocity_Fn velocity, Enemy_Entity_Fire_Fn fire) {
    Enemy_Entity enemy;

    enemy.position = position;
    enemy.scale    = scale;
    enemy.firing_cooldown = fire_cooldown;

    enemy.velocity_function = velocity;
    enemy.on_fire_function  = fire;

    enemy.edge_top_behavior_override = enemy.edge_bottom_behavior_override =
        enemy.edge_left_behavior_override = enemy.edge_right_behavior_override = PLAY_AREA_EDGE_PASSTHROUGH;
    state->gameplay_data.add_enemy_entity(enemy);
}

void spawn_bullet_generic(Game_State* state, V2 position, V2 scale, s32 source, Bullet_Entity_Velocity_Fn velocity) {
    Bullet bullet;
    bullet.position    = position;
    bullet.scale       = V2(5,5);
    bullet.lifetime    = Timer(3.0f);
    bullet.source_type = source;

    bullet.velocity_function = velocity;
    state->gameplay_data.add_bullet(bullet);
}

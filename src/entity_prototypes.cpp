#include "entity.h"
#include "game_state.h"

#include "game_uid_generator.h"

/*
  Here's a bunch of velocity functions that I compose together to make some interesting behaviors or patterns,
  with hopefully not much hassle. A lot of them are obvious, but it's just to make the
  pattern code more obvious to read.
*/
V2 velocity_linear(V2 direction, f32 magnitude) {
    direction = direction.normalized();
    return direction * magnitude;
}

V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, f32 accel_magnitude) {
    direction = direction.normalized();
    return direction * magnitude + (direction * (accel_magnitude * t));
}

V2 velocity_linear_with_accel(V2 direction, f32 magnitude, f32 t, V2 accel_direction, f32 accel_magnitude) {
    direction = direction.normalized();
    return direction * magnitude + (accel_direction * (accel_magnitude * t));
}

V2 velocity_follow(Entity* entity) {
    if (entity)
        return entity->velocity;

    return V2(0, 0);
}

V2 velocity_circle_orbit(f32 t, f32 spawn_theta, f32 r) {
    f32 point_in_cycle = t + spawn_theta;
    V2 result = V2(-sinf(point_in_cycle) * r, cos(point_in_cycle) * r);
    return result;
}

V2 velocity_cyclic_sine(f32 t, V2 direction) {
    direction = direction.normalized();
    return direction * sinf(t);
}

V2 velocity_cyclic_cosine(f32 t, V2 direction) {
    direction = direction.normalized();
    return direction * cosf(t);
}

Bullet bullet_generic(Game_State* state, V2 position, V2 scale, s32 source, Bullet_Entity_Velocity_Fn velocity, s32 visual) {
    Bullet bullet = {};
    bullet.uid = UID::bullet_uid();
    bullet.position    = position;
    bullet.last_position = bullet.position;
    bullet.scale       = scale;
    //bullet.lifetime    = Timer(3.0f);
    bullet.source_type = source;

    bullet.sprite = sprite_instance(
        // state->resources->projectile_sprites[PROJECTILE_SPRITE_BLUE]
        state->resources->projectile_sprites[visual]
        // state->resources->projectile_sprites[PROJECTILE_SPRITE_NEGATIVE_ELECTRIC]
    );
    // bullet.trail_ghost_limit = 8;
    // bullet.sprite.scale = V2(1, 1);
    bullet.sprite.scale = V2(0.5, 0.5);
    bullet.velocity_function = velocity;
    state->gameplay_data.add_bullet(bullet);
    return bullet;
}

Bullet bullet_upwards_linear(Game_State* state, V2 position, V2 direction, f32 magnitude, s32 visual, s32 source) {
    Bullet bullet = bullet_generic(
        state, position, V2(5, 5), source,
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_linear(direction, magnitude);
        },
        visual
    );
    bullet.die      = false;
    bullet.lifetime = Timer(6.0f);
    return bullet;
}

Enemy_Entity enemy_circular_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 r) {
    Enemy_Entity enemy = enemy_generic(
        state,
        position, scale,
        [direction, speed, r](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_circle_orbit(self->t_since_spawn, 0, r);
            self->velocity += velocity_linear(direction, speed);
        }
    );

    return enemy;
}

Enemy_Entity enemy_sine_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 amp) {
    Enemy_Entity enemy = enemy_generic(
        state,
        position, scale,
        [direction, speed, amp](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_cyclic_sine(self->t_since_spawn, direction) * amp;
            self->velocity += velocity_linear(direction, speed);
        }
    );

    return enemy;
}

Enemy_Entity enemy_cosine_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed, f32 amp) {
    Enemy_Entity enemy = enemy_generic(
        state,
        position, scale,
        [direction, speed, amp](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_cyclic_cosine(self->t_since_spawn, direction) * amp;
            self->velocity += velocity_linear(direction, speed);
        }
    );

    return enemy;
}

Enemy_Entity enemy_linear_movement(Game_State* state, V2 position, V2 scale, V2 direction, f32 speed) {
    Enemy_Entity enemy = enemy_generic(
        state,
        position, scale,
        [direction, speed](Enemy_Entity* self, Game_State* const state, f32 dt) {
            self->velocity += velocity_linear(direction, speed);
        }
    );

    return enemy;
}

Enemy_Entity enemy_generic(Game_State* state, V2 position, V2 scale, Enemy_Entity_Velocity_Fn velocity) {
    Enemy_Entity enemy = {};
    enemy.uid = UID::enemy_uid();

    enemy.last_position = enemy.position;
    enemy.position      = position;
    enemy.scale         = scale;
    enemy.die           = false;

    enemy.velocity_function = velocity;

    enemy.edge_top_behavior_override = enemy.edge_bottom_behavior_override =
        enemy.edge_left_behavior_override = enemy.edge_right_behavior_override = PLAY_AREA_EDGE_PASSTHROUGH;

    return enemy;
}

void spawn_bullet_line_pattern1(Game_State* state, V2 center, s32 how_many, f32 spacing, V2 scale, V2 direction, f32 speed, s32 source, s32 visual) {
    direction = direction.normalized();

    f32 pattern_width = spacing * how_many + scale.x * how_many;
    V2 perpendicular_direction = V2_perpendicular(direction);
    center -= (perpendicular_direction * (pattern_width/2));

    for (s32 i = 0; i < how_many; ++i) {
        state->gameplay_data.add_bullet(
            bullet_upwards_linear(state, center, direction, speed, visual, source)
        );

        center += perpendicular_direction * (spacing + scale.x);
    }
}

void spawn_bullet_arc_pattern1(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, s32 source, s32 visual) {
    spawn_bullet_arc_pattern2(
        state,
        center,
        how_many,
        arc_degrees,
        scale,
        direction,
        speed,
        0.0f,
        source,
        visual
    );
}

void spawn_bullet_arc_pattern2_trailed(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, f32 distance_from_center, s32 source, s32 visual, s32 trailcount) {
  direction = direction.normalized();
  f32 arc_sub_length = (f32)arc_degrees / (how_many);

  f32 direction_angle = radians_to_degrees(atan2(direction.y, direction.x));
  for (s32 i = 0; i < how_many; ++i) {
    f32 angle = direction_angle + arc_sub_length * (i - (how_many / 2)); // adjust the center projectile to be 0 degrees
    V2 current_arc_direction = V2_direction_from_degree(angle);
    V2 position = center + current_arc_direction * distance_from_center;
    auto b = bullet_upwards_linear(state, position, current_arc_direction, speed, visual, source);
    b.trail_ghost_limit = trailcount;
    b.trail_ghost_modulation = color32f32(0.8,0.8,0.8,1);
    b.trail_ghost_max_alpha = 1;
    state->gameplay_data.add_bullet(
      b
    );
  }
}

void spawn_bullet_arc_pattern2(Game_State* state, V2 center, s32 how_many, s32 arc_degrees, V2 scale, V2 direction, f32 speed, f32 distance_from_center, s32 source, s32 visual) {
  spawn_bullet_arc_pattern2_trailed(
    state, 
    center, 
    how_many, 
    arc_degrees, 
    scale, 
    direction, 
    speed, 
    distance_from_center, 
    source, 
    visual, 
    0
  );
}

Enemy_Entity enemy_generic_with_task(Game_State* state, V2 position, V2 scale, jdr_duffcoroutine_fn task) {
    auto result = enemy_generic(
        state, position, scale, nullptr
    );
    state->coroutine_tasks.add_task(state, task, (void*)result.uid);
    return result;
}

Bullet bullet_generic_with_task(Game_State* state, V2 position, V2 scale, s32 source, jdr_duffcoroutine_fn task, s32 visual) {
    auto result = bullet_generic(
        state, position, scale, source, nullptr, visual
    );

    state->coroutine_tasks.add_task(state, task, (void*)result.uid);
    return result;
}

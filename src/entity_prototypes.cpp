#include "entity.h"
#include "game_state.h"

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

    state->gameplay_data.bullets.push(bullet);
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

    state->gameplay_data.bullets.push(bullet);
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

    state->gameplay_data.bullets.push(bullet);
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

    state->gameplay_data.bullets.push(bullet);
}

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "common.h"
#include "render_commands.h"
#include "fixed_array.h"
#include "prng.h"

struct Particle {
    V2 position;
    V2 velocity;
    V2 acceleration;

    f32 scale;
    f32 lifetime;
    Sprite_Instance sprite;
    color32f32 modulation;

    bool use_attraction_point;
    V2 attraction_point;
    f32 attraction_force;
};

enum Particle_Emit_Shape_Type {
    PARTICLE_EMIT_SHAPE_POINT,
    PARTICLE_EMIT_SHAPE_CIRCLE,
    PARTICLE_EMIT_SHAPE_QUAD,
    PARTICLE_EMIT_SHAPE_LINE,
};

struct Particle_Emit_Shape_Point {
    V2 position;
};

struct Particle_Emit_Shape_Circle {
    V2 center;
    f32 radius;
};

struct Particle_Emit_Shape_Quad {
    V2 center;
    V2 half_dimensions;
};

struct Particle_Emit_Shape_Line {
    V2 start;
    V2 end;
};

struct Particle_Emit_Shape {
    s32  type;
    bool filled_shape;
    union {
        Particle_Emit_Shape_Point  point;
        Particle_Emit_Shape_Circle circle;
        Particle_Emit_Shape_Quad   quad;
        Particle_Emit_Shape_Line   line;
    };

    V2 emit_position(random_state* prng);
};

struct Particle_Pool;
struct Particle_Emitter {
    Particle_Emit_Shape shape;

    Sprite_Instance sprite;
    f32             scale;
    V2              velocity;
    V2              acceleration;
    color32f32      modulation;
    f32             lifetime;

    V2 scale_variance; // particles are going to be "square/quads" only.
    V2 velocity_x_variance;
    V2 velocity_y_variance;
    V2 acceleration_x_variance;
    V2 acceleration_y_variance;
    V2 lifetime_variance;

    bool active = false;
    s32 max_emissions = -1;
    s32 emissions = 0;
    s32 emit_per_emission = 1;

    f32 emission_timer = 0;
    f32 emission_delay_timer = 0;

    f32 emission_max_timer = 0;
    f32 emission_max_delay_timer = 0;

    void update(Particle_Pool* pool, f32 dt);
    void reset();
};

struct Game_Resources;
/*
  So unlike Legends, I decided it would just be best to allow different particle
  pools, since Legends' particle system was uh... Really painful to get working anywhere
  but the main game.

  Which was a critical mistake, since the architecture of both this game and Legends made
  game_screen states, effectively "independent" things.
*/
struct Particle_Pool {
    Particle_Pool(Memory_Arena* arena, s32 amount);

    void update(f32 dt);
    void draw(struct render_commands* commands, Game_Resources* resources);

    Fixed_Array<Particle> particles;
};

#endif

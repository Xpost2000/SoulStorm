#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "common.h"
#include "render_commands.h"
#include "fixed_array.h"
#include "prng.h"

// Would like to simd this one day.
struct Particle {
    V2 position;
    V2 velocity;
    V2 acceleration;
    u32 blend_mode = BLEND_MODE_ALPHA;
    bool flame_mode = false;

    f32 scale;
    f32 lifetime, lifetime_max;
    Sprite_Instance sprite;

    color32f32 modulation;
    color32f32 target_modulation;

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
    Particle_Emit_Shape();
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

Particle_Emit_Shape particle_emit_shape_point(V2 point);
Particle_Emit_Shape particle_emit_shape_circle(V2 center, f32 radius, bool filled=false);
Particle_Emit_Shape particle_emit_shape_quad(V2 center, V2 half_lengths, bool filled=false);
Particle_Emit_Shape particle_emit_shape_line(V2 start, V2 end);

struct Particle_Pool;
// NOTE: particle sizes are based off image size.
struct Particle_Emitter {
    Particle_Emit_Shape shape;

    Sprite_Instance sprite;
    V2              velocity = V2(0,0);
    V2              acceleration = V2(0,0);
    color32f32      modulation = color32f32(1, 1, 1, 1);

    // only if use_color_fade is on.
    color32f32      target_modulation = color32f32(1, 1, 1, 1);

    V2 scale_variance          = V2(0,0); // particles are going to be "square/quads" only.
    V2 velocity_x_variance     = V2(0,0);
    V2 velocity_y_variance     = V2(0,0);
    V2 acceleration_x_variance = V2(0,0);
    V2 acceleration_y_variance = V2(0,0);
    V2 lifetime_variance       = V2(0,0);

    // NOTE: only active if use_angular == true.
    // will source velocity and acceleration from the x components
    // of the respective variable
    V2 angle_range             = V2(0,0);
    V2 attraction_point;

    // NOTE: need to refer back to legends-jrpg for a more robust particle
    // system since I liked some of the stuff I did there,
    bool use_color_fade       = false;
    bool use_attraction_point = false;
    bool use_angular          = false;
    bool flame_mode           = false;
    bool active = false;

    u32 blend_mode = BLEND_MODE_ALPHA;

    f32 attraction_force;
    f32 scale;
    f32 lifetime;

    s32 max_emissions = -1;
    s32 emissions = 0;
    s32 emit_per_emission = 1;

    f32 emission_timer = 0;

    f32 emission_max_timer = 0;

    void update(Particle_Pool* pool, random_state* prng, f32 dt);
    void reset();
};

struct lua_State;
void bind_particle_emitter_lualib(lua_State* L);

struct Game_State;
struct Game_Resources;
/*
  So unlike Legends, I decided it would just be best to allow different particle
  pools, since Legends' particle system was uh... Really painful to get working anywhere
  but the main game.

  Which was a critical mistake, since the architecture of both this game and Legends made
  game_screen states, effectively "independent" things.
*/
struct Particle_Pool {
    void init(Memory_Arena* arena, s32 amount);

    void update(Game_State* state, f32 dt);
    void draw(struct render_commands* commands, Game_Resources* resources);
    void clear();

    Fixed_Array<Particle> particles;
};

#endif

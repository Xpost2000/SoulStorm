#include "game.h"
#include "game_state.h"
#include "particle_system.h"

#include "lua_binding_macro.h"

GAME_LUA_MODULE(
particle_emitter,
"src/particle_system_lua_bindings_generated.cpp",
"Support library to access particle emitter objects implemented natively in C++.",
"This provides access to the particle emitter interface within the engine,\
through the form of accessor functions.\
\
\
The entire interface including all collision shapes are open for access. However it directly mirrors\
the in-engine C++ API, and thus has some oddities in usage. Any pointers to Particle_Emitters must be\
obtained through other game APIs, so you should refer to those."
)

/*
 * NOTE:
 *
 * While I've been generally comfortable using IDs and such,
 * particle systems are using pointers as light userdata.
 *
 *
 * Also unfortunately, this is a pretty direct 1-1 with the C API in-engine mostly
 * because it is tiresome to make more ergonomic bindings.
 *
 * I am a little bothered by this, but I'm mostly trying to finish this before I run out
 * of steam.
 *
 */
local Particle_Emitter* userdata_to_emitter(lua_State* L, s32 i) {
    void* p = lua_touserdata(L, i);
    return (Particle_Emitter*)p;
}

// int _lua_bind_particle_emitter_set_angle_range(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_angle_range,
"p: Particle_Emitter*, x: number, y: number",
"Set the angle range used for particle emission.",
"Set the angle range used for particle emission in angular mode.\
Angular mode must be enabled separately."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->angle_range = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_attraction_point(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_attraction_point,
"p: Particle_Emitter*, point: V2",
"Set an attraction point for particles to move towards.",
"Set an attraction point used by particles to move towards with respect to a force.\
Requires enabling the use_attraction_point flag separately."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->attraction_point = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_attraction_force(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_attraction_force,
"p: Particle_Emitter*, force: number",
"Set the attraction force for particles when moving towards an attraction point.",
"Set the attraction force for particles that have an attraction point. Units are in pixels.\
Requires enabling the use_attraction_point flag separately."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->attraction_force = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_scale(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_scale,
"p: Particle_Emitter*, scale: number",
"Set the scale for particles emitted.",
"Set the scale for particles emitted from this emitter. Particles are square."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->scale = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_lifetime(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_lifetime,
"p: Particle_Emitter*, lifetime: number",
"Set the lifetime of particles from this emitter.",
"Set the base/average lifetime of particles from this emitter."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->lifetime = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_max_emissions(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_max_emissions,
"p: Particle_Emitter*, max_emissions: number",
"Set the maximum amount of emissions from this emitter.",
"Set the maximum amount of emissions from this emitter, and stop emitting after."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->max_emissions = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emit_per_emission(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emit_per_emission,
"p: Particle_Emitter*, emit_per_emission: number",
"Set the burst rate of this emitter.",
"Emit _emit_per_emission_ particles on every emission at once."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->emit_per_emission = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emission_max_timer(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emission_max_timer,
"p: Particle_Emitter*, delay: number",
"Set the delay time per emission for this emitter.",
"Set the wait time per emission for this emitter."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->emission_max_timer = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_scale_variance(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_scale_variance,
"p: Particle_Emitter*, x: number, y: number",
"Set the scale variance for particles from this emitter.",
"Set the scale variance for particles based on a vector range from this emitter. (min, max)"
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->scale_variance = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_velocity_x_variance(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_velocity_x_variance,
"p: Particle_Emitter*, x: number, y: number",
"Set the x velocity variance for particles from this emitter.",
"Set the x velocity variance for particles based on a vector range from this emitter. (min, max)"
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity_x_variance = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_velocity_y_variance(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_velocity_y_variance,
"p: Particle_Emitter*, x: number, y: number",
"Set the y velocity variance for particles from this emitter.",
"Set the y velocity variance for particles based on a vector range from this emitter. (min, max)"
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity_y_variance = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_acceleration_x_variance(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_acceleration_x_variance,
"p: Particle_Emitter*, x: number, y: number",
"Set the x acceleration variance for particles from this emitter.",
"Set the x acceleration variance for particles based on a vector range from this emitter. (min, max)"
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration_x_variance = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_acceleration_y_variance(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_acceleration_y_variance,
"p: Particle_Emitter*, x: number, y: number",
"Set the y acceleration variance for particles from this emitter.",
"Set the y acceleration variance for particles based on a vector range from this emitter. (min, max)"
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration_y_variance = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_velocity(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_velocity,
"p: Particle_Emitter*, x: number, y: number",
"Set the velocity for particles from this emitter.",
"Set the base/average velocity for particles for this emitter."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_acceleration(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_acceleration,
"p: Particle_Emitter*, x: number, y: number",
"Set the acceleration for particles from this emitter.",
"Set the base/average acceleration for particles for this emitter."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration = V2(a, b);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_modulation(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_modulation,
"p: Particle_Emitter*, r: number, g: number, b: number, a: number",
"Set the color modulation for particles from this emitter.",
"Set the color modulation for particles for this emitter as a 4 tuple of [0.0-1.0]."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    f32  c = luaL_checknumber(L, 4);
    f32  d = luaL_checknumber(L, 5);
    if (e) {
        e->modulation = color32f32(a,b,c,d);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_target_modulation(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_target_modulation,
"p: Particle_Emitter*, r: number, g: number, b: number, a: number",
"Set the target color modulation for particles from this emitter.",
"Set the target color modulation for particles for this emitter as a 4 tuple of [0.0-1.0].\
Target color fading must be enabled separately."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    f32  c = luaL_checknumber(L, 4);
    f32  d = luaL_checknumber(L, 5);
    if (e) {
        e->target_modulation = color32f32(a,b,c,d);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_blend_mode(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_blend_mode,
"p: Particle_Emitter*, blendmode: number_id",
"Set the blend mode used for particles from this emitter.",
"Set the blend mode used for particles from this emitter. Refer to common.lua for constants."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->blend_mode = a;
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_use_flame_mode(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_use_flame_mode,
"p: Particle_Emitter*, b: number",
"Set the flame mode rendering style for particles.",
"Set the flame mode rendering style for particles which will render a particle twice, once alpha blended\
then additively blended on top for a neat glow effect.."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        if (a) {
            e->flags |= PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
        }
        else {
            e->flags &= ~PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE;
        }
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_use_angular(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_use_angular,
"p: Particle_Emitter*, b: number",
"Set the angular mode flag",
"Set the angular mode flag. Will change behavior of velocity and acceleration fields.\
Magnitudes will be source from the x component of both fields. Also set angular_range for emission direction."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        if (a) {
            e->flags |= PARTICLE_EMITTER_FLAGS_USE_ANGULAR;
        }
        else {
            e->flags &= ~PARTICLE_EMITTER_FLAGS_USE_ANGULAR;
        }
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_use_attraction_point(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_use_attraction_point,
"p: Particle_Emitter*, b: number",
"Set the use attraction point mode flag",
"Set the use attraction point mode flag. Make sure to set attraction force and attraction point."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        if (a) {
            e->flags |= PARTICLE_EMITTER_FLAGS_USE_ATTRACTION_POINT;
        }
        else {
            e->flags &= ~PARTICLE_EMITTER_FLAGS_USE_ATTRACTION_POINT;
        }
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_use_color_fade(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_use_color_fade,
"p: Particle_Emitter*, b: number",
"Set the color fading flag for this emitter.",
"Set the color fading flag for this emitter. Make sure to set target_color separately."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        if (a) {
            e->flags |= PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE;
        }
        else {
            e->flags &= ~PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE;
        }
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_active(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_active,
"p: Particle_Emitter*, b: number",
"Set the active flag for this emitter.",
"Set the active flag for this emitter. It will not emit when this is false."
)
{
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        if (a) {
            e->flags |= PARTICLE_EMITTER_FLAGS_ACTIVE;
        }
        else {
            e->flags &= ~PARTICLE_EMITTER_FLAGS_ACTIVE;
        }
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emit_shape_point(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emit_shape_point,
"p: Particle_Emitter*, x: number, y: number",
"Set the emission shape of this emitter to be a singular point.",
"Set the emission shape of this emitter to be a singular point."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->shape = particle_emit_shape_point(V2(a, b));
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emit_shape_circle(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emit_shape_circle,
"p: Particle_Emitter*, x: number, y: number, r: number, filled: boolean",
"Set the emission shape of this emitter to be a circle.",
"Set the emission shape of this emitter to be a circle."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    f32  c = luaL_checknumber(L, 4);
    s32  d = lua_toboolean(L, 5);
    if (e) {
        e->shape = particle_emit_shape_circle(V2(a, b), c, d);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emit_shape_quad(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emit_shape_quad,
"p: Particle_Emitter*, x: number, y: number, w: number, h: number, filled: boolean",
"Set the emission shape of this emitter to be a quad/rectangle.",
"Set the emission shape of this emitter to be a quad/rectangle."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    f32  c = luaL_checknumber(L, 4);
    f32  d = luaL_checknumber(L, 5);
    s32  f = lua_toboolean(L, 5);
    if (e) {
        e->shape = particle_emit_shape_quad(V2(a, b), V2(c,d), f);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_emit_shape_line(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_emit_shape_line,
"p: Particle_Emitter*, x: number, y: number, xend: number, yend: number",
"Set the emission shape of this emitter to be a line.",
"Set the emission shape of this emitter to be a line."
)
{
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    f32  c = luaL_checknumber(L, 4);
    f32  d = luaL_checknumber(L, 5);
    if (e) {
        e->shape = particle_emit_shape_line(V2(a, b), V2(c,d));
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_sprite_circle(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_sprite_circle,
"p: Particle_Emitter*",
"Set the particle emitter to use circles for its particles.",
"Set the particle emitter to use circles for its particles."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    if (e) {
        e->sprite = sprite_instance(state->resources->circle_sprite);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_sprite_entity(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_sprite_entity,
"p: Particle_Emitter*, id: number",
"Set the particle emitter to use the entity sprite specified by id.",
"Set the particle emitter to use the entity sprite specified by id. Check the manifest files for ids."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    s32  id = luaL_checkinteger(L, 2);
    if (e) {
        e->sprite = sprite_instance(state->resources->entity_sprites[id]);
    }
    return 0;
}

// int _lua_bind_particle_emitter_set_sprite_projectile(lua_State* L)
GAME_LUA_PROC(
particle_emitter_set_sprite_projectile,
"p: Particle_Emitter*, id: number",
"Set the particle emitter to use the projectile sprite sprite specified by id.",
"Set the particle emitter to use the projectile sprite sprite specified by id. Check the manifest files for ids."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    s32  id = luaL_checkinteger(L, 2);
    if (e) {
        e->sprite = sprite_instance(state->resources->projectile_sprites[id]);
    }
    return 0;
}

#include "particle_system_lua_bindings_generated.cpp"

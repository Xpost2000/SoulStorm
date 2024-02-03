#include "game.h"
#include "game_state.h"
#include "particle_system.h"

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

int _lua_bind_particle_emitter_set_angle_range(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->angle_range = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_attraction_point(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->attraction_point = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_attraction_force(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->attraction_force = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_scale(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->scale = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_lifetime(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->lifetime = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_max_emissions(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->max_emissions = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_emit_per_emission(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->emit_per_emission = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_emission_max_timer(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    if (e) {
        e->emission_max_timer = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_scale_variance(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->scale_variance = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_velocity_x_variance(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity_x_variance = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_velocity_y_variance(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity_y_variance = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_acceleration_x_variance(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration_x_variance = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_acceleration_y_variance(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration_y_variance = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_velocity(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->velocity = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_acceleration(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->acceleration = V2(a, b);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_modulation(lua_State* L) {
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

int _lua_bind_particle_emitter_set_target_modulation(lua_State* L) {
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

int _lua_bind_particle_emitter_set_blend_mode(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = luaL_checkinteger(L, 2);
    if (e) {
        e->blend_mode = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_use_flame_mode(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        e->flame_mode = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_use_angular(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        e->use_angular = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_use_attraction_point(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        e->use_attraction_point = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_use_color_fade(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        e->use_color_fade = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_active(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    s32  a = lua_toboolean(L, 2);
    if (e) {
        e->active = a;
    }
    return 0;
}

int _lua_bind_particle_emitter_set_emit_shape_point(lua_State* L) {
    auto e = userdata_to_emitter(L, 1);
    f32  a = luaL_checknumber(L, 2);
    f32  b = luaL_checknumber(L, 3);
    if (e) {
        e->shape = particle_emit_shape_point(V2(a, b));
    }
    return 0;
}

int _lua_bind_particle_emitter_set_emit_shape_circle(lua_State* L) {
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

int _lua_bind_particle_emitter_set_emit_shape_quad(lua_State* L) {
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

int _lua_bind_particle_emitter_set_emit_shape_line(lua_State* L) {
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

int _lua_bind_particle_emitter_set_sprite_circle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    if (e) {
        e->sprite = sprite_instance(state->resources->circle_sprite);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_sprite_entity(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    s32  id = luaL_checkinteger(L, 2);
    if (e) {
        e->sprite = sprite_instance(state->resources->entity_sprites[id]);
    }
    return 0;
}

int _lua_bind_particle_emitter_set_sprite_projectile(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = userdata_to_emitter(L, 1);
    s32  id = luaL_checkinteger(L, 2);
    if (e) {
        e->sprite = sprite_instance(state->resources->projectile_sprites[id]);
    }
    return 0;
}

void bind_particle_emitter_lualib(lua_State* L) {
#if 0
    // NOTE: particle emitters are write-only.
    lua_register(L, "particle_emitter_set_angle_range",             _lua_bind_particle_emitter_set_angle_range);
    lua_register(L, "particle_emitter_set_attraction_point",        _lua_bind_particle_emitter_set_attraction_point);
    lua_register(L, "particle_emitter_set_attraction_force",        _lua_bind_particle_emitter_set_attraction_force);
    lua_register(L, "particle_emitter_set_scale",                   _lua_bind_particle_emitter_set_scale);
    lua_register(L, "particle_emitter_set_lifetime",                _lua_bind_particle_emitter_set_lifetime);
    lua_register(L, "particle_emitter_set_max_emissions",           _lua_bind_particle_emitter_set_max_emissions);
    lua_register(L, "particle_emitter_set_emit_per_emission",       _lua_bind_particle_emitter_set_emit_per_emission);
    lua_register(L, "particle_emitter_set_emission_max_timer",      _lua_bind_particle_emitter_set_emission_max_timer);
    lua_register(L, "particle_emitter_set_scale_variance",          _lua_bind_particle_emitter_set_scale_variance);
    lua_register(L, "particle_emitter_set_velocity_x_variance",     _lua_bind_particle_emitter_set_velocity_x_variance);
    lua_register(L, "particle_emitter_set_velocity_y_variance",     _lua_bind_particle_emitter_set_velocity_y_variance);
    lua_register(L, "particle_emitter_set_acceleration_x_variance", _lua_bind_particle_emitter_set_acceleration_x_variance);
    lua_register(L, "particle_emitter_set_acceleration_y_variance", _lua_bind_particle_emitter_set_acceleration_y_variance);
    lua_register(L, "particle_emitter_set_velocity",                _lua_bind_particle_emitter_set_velocity);
    lua_register(L, "particle_emitter_set_acceleration",            _lua_bind_particle_emitter_set_acceleration);
    lua_register(L, "particle_emitter_set_modulation",              _lua_bind_particle_emitter_set_modulation);
    lua_register(L, "particle_emitter_set_target_modulation",       _lua_bind_particle_emitter_set_target_modulation);
    lua_register(L, "particle_emitter_set_blend_mode",              _lua_bind_particle_emitter_set_blend_mode);
    lua_register(L, "particle_emitter_set_use_flame_mode",          _lua_bind_particle_emitter_set_use_flame_mode);
    lua_register(L, "particle_emitter_set_use_angular",             _lua_bind_particle_emitter_set_use_angular);
    lua_register(L, "particle_emitter_set_use_attraction_point",    _lua_bind_particle_emitter_set_use_attraction_point);
    lua_register(L, "particle_emitter_set_use_color_fade",          _lua_bind_particle_emitter_set_use_color_fade);
    lua_register(L, "particle_emitter_set_active",                  _lua_bind_particle_emitter_set_active);
    lua_register(L, "particle_emitter_set_emit_shape_point",        _lua_bind_particle_emitter_set_emit_shape_point);
    lua_register(L, "particle_emitter_set_emit_shape_circle",       _lua_bind_particle_emitter_set_emit_shape_circle);
    lua_register(L, "particle_emitter_set_emit_shape_quad",         _lua_bind_particle_emitter_set_emit_shape_quad);
    lua_register(L, "particle_emitter_set_emit_shape_line",         _lua_bind_particle_emitter_set_emit_shape_line);
    // Sprites are segregated into various different categories and therefore sourcing
    // arbitrary sprites isn't really easy...
    //
    // Generally sprites are either one of the projectile sprites since they're particle-like
    // or the circle sprite. Entity sprite added just incase.
    //
    // There are no particle dedicated sprites.
    lua_register(L, "particle_emitter_set_sprite_circle",           _lua_bind_particle_emitter_set_sprite_circle);
    lua_register(L, "particle_emitter_set_sprite_entity",           _lua_bind_particle_emitter_set_sprite_entity);
    lua_register(L, "particle_emitter_set_sprite_projectile",       _lua_bind_particle_emitter_set_sprite_projectile);
#endif
}

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

void bind_particle_emitter_lualib(lua_State* L) {
#if 0
    // NOTE: particle emitters are read-only.
    lua_register(L, "particle_emitter_set_angle_range", );
    lua_register(L, "particle_emitter_set_attraction_point", );
    lua_register(L, "particle_emitter_set_attraction_force", );
    lua_register(L, "particle_emitter_set_scale", );
    lua_register(L, "particle_emitter_set_lifetime", );
    lua_register(L, "particle_emitter_set_max_emissions", );
    lua_register(L, "particle_emitter_set_emit_per_emission", );
    lua_register(L, "particle_emitter_set_emission_max_timer", );
    lua_register(L, "particle_emitter_set_scale_variance", );
    lua_register(L, "particle_emitter_set_velocity_x_variance", );
    lua_register(L, "particle_emitter_set_velocity_y_variance", );
    lua_register(L, "particle_emitter_set_acceleration_x_variance", );
    lua_register(L, "particle_emitter_set_acceleration_y_variance", );
    lua_register(L, "particle_emitter_set_velocity", );
    lua_register(L, "particle_emitter_set_acceleration", );
    lua_register(L, "particle_emitter_set_modulation", );
    lua_register(L, "particle_emitter_set_target_modulation", );

    lua_register(L, "particle_emitter_set_blend_mode", );
    lua_register(L, "particle_emitter_set_use_flame_mode", );
    lua_register(L, "particle_emitter_set_use_angular", );
    lua_register(L, "particle_emitter_set_use_attraction_point", );
    lua_register(L, "particle_emitter_set_use_color_fade", );
    lua_register(L, "particle_emitter_set_active", );

    lua_register(L, "particle_emitter_set_emit_shape_point", );
    lua_register(L, "particle_emitter_set_emit_shape_circle", );
    lua_register(L, "particle_emitter_set_emit_shape_quad", );
    lua_register(L, "particle_emitter_set_emit_shape_line", );

    // Sprites are segregated into various different categories and therefore sourcing
    // arbitrary sprites isn't really easy...
    //
    // Generally sprites are either one of the projectile sprites since they're particle-like
    // or the circle sprite. Entity sprite added just incase.
    //
    // There are no particle dedicated sprites.
    lua_register(L, "particle_emitter_set_sprite_circle", );
    lua_register(L, "particle_emitter_set_sprite_entity", );
    lua_register(L, "particle_emitter_set_sprite_projectile", );
#endif
}

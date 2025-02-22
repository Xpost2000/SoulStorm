#include "entity.h"
#include "game_state.h"

#include "lua_binding_macro.h"
// BEGIN_LUA_BINDINGS

GAME_LUA_MODULE(
entity,
"src/entity_lua_bindings_generated.cpp",
"Support library to access game entities implemented natively in C++.",
"This provides access to the entity interface within the engine,\
through the form of accessor functions. As well as the ability to create the various\
entities inside the Bullet Hell game\
The entire interface including all collision shapes are open for access."
)

int _lua_bind_spawn_bullet_arc_pattern2(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    spawn_bullet_arc_pattern2(
        state,
        V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)),
        luaL_checkinteger(L, 3),
        luaL_checkinteger(L, 4),
        V2(luaL_checknumber(L, 5), luaL_checknumber(L, 6)),
        V2(luaL_checknumber(L, 7), luaL_checknumber(L, 8)),
        luaL_checknumber(L, 9),
        luaL_checknumber(L, 10),
        luaL_checkinteger(L, 11),
        luaL_checkinteger(L, 12)
    );
    return 0;
}

// ENEMY ENTITY

int _lua_bind_enemy_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = enemy_generic(state, V2(0, 0), V2(10, 10), nullptr);
    state->gameplay_data.add_enemy_entity(e);
    lua_pushinteger(L, e.uid);
    return 1;
}

int _lua_bind_enemy_time_since_spawn(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->t_since_spawn);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_visual_loop_completions(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->sprite.loop_completions);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_valid(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    lua_pushboolean(L, e != nullptr);
    return 1;
}

int _lua_bind_enemy_set_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->position.x = luaL_checknumber(L, 2);
        e->position.y = luaL_checknumber(L, 3);
        e->last_position = e->position;
    }
    return 0;
}

int _lua_bind_enemy_set_animation_frame(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->sprite.frame = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_set_relative_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->relative_position.x = luaL_checknumber(L, 2);
        e->relative_position.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_reset_relative_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->relative_position.x = 0;
        e->relative_position.y = 0;
    }
    return 0;
}

int _lua_bind_enemy_set_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->scale.x = luaL_checknumber(L, 2);
        e->scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_visual_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->sprite.scale.x = luaL_checknumber(L, 2);
        e->sprite.scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_velocity(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->velocity.x = luaL_checknumber(L, 2);
        e->velocity.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_acceleration(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->acceleration.x = luaL_checknumber(L, 2);
        e->acceleration.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->max_hp = e->hp = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_set_task(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    char* task_name = (char*)lua_tostring(L, 2);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        lua_remove(L, 1); lua_remove(L, 1);
        s32 remaining = lua_gettop(L);

        _debugprintf("entity async task with %d elements [%d actual stack top]", remaining, lua_gettop(L));

        state->coroutine_tasks.add_enemy_lua_game_task(
            state,
            L,
            state->coroutine_tasks.L,
            task_name,
            uid
        );
    }

    return 0;
}

int _lua_bind_enemy_reset_movement(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->reset_movement();
    }
    return 0;
}

int _lua_bind_enemy_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_relative_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->relative_position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_relative_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->relative_position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_acceleration_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_acceleration_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_set_max_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        e->maximum_speed = luaL_checknumber(L, 2);
    }

    return 0;
}

int _lua_bind_enemy_get_max_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        lua_pushnumber(L, e->maximum_speed);
        return 1;
    }

    return 0;
}

int _lua_bind_enemy_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushinteger(L, e->hp);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_hp_percent(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushnumber(L, e->hp_percentage());
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_get_particle_emitter(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        lua_pushlightuserdata(L, &e->emitters[luaL_checkinteger(L, 2)]);
        return 1;
    }
    return 0;
}

int _lua_bind_enemy_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    lua_pushlightuserdata(L, e);
    return 1;
}

int _lua_bind_enemy_start_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_limit = luaL_checkinteger(L, 2);
    }
    return 0;
}
int _lua_bind_enemy_set_trail_modulation(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_modulation = color32f32(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), 1);
        e->trail_ghost_max_alpha = luaL_checknumber(L, 5);
    }
    return 0;
}
int _lua_bind_enemy_set_trail_record_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_record_timer_max = luaL_checknumber(L, 2);
    }
    return 0;
}

int _lua_bind_enemy_stop_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->trail_ghost_limit = 0;
    }
    return 0;
}

int _lua_bind_enemy_kill(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->kill();
    }
    return 0;
}

int _lua_bind_enemy_hurt(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->damage(luaL_checkinteger(L, 2));
    }
    return 0;
}

int _lua_bind_enemy_begin_invincibility(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->begin_invincibility(lua_toboolean(L, 2), luaL_checknumber(L, 3));
    }
    return 0;
}

int _lua_bind_enemy_end_invincibility(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->end_invincibility();
    }
    return 0;
}

int _lua_bind_enemy_set_oob_deletion(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        e->allow_out_of_bounds_survival = lua_toboolean(L, 2);
    }

    return 0;
}

int _lua_bind_enemy_show_boss_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        state->gameplay_data.boss_health_displays.add(
            uid,
            string_from_cstring((char*)lua_tostring(L, 2))
        );
    }

    return 0;
}

int _lua_bind_enemy_hide_boss_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);

    if (e) {
        state->gameplay_data.boss_health_displays.remove(uid);
    }

    return 0;
}
// BULLET ENTITY
int _lua_bind_bullet_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = bullet_generic(
        state, V2(0, 0), V2(10, 10),
        luaL_checkinteger(L, 1),
        nullptr,
        PROJECTILE_SPRITE_BLUE);
    // state->gameplay_data.add_bullet(e);
    lua_pushinteger(L, e.uid);
    return 1;
}

int _lua_bind_bullet_set_source(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->source_type = luaL_checkinteger(L, 2);
    }
    return 0;
}

int _lua_bind_bullet_time_since_spawn(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->t_since_spawn);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_valid(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    lua_pushboolean(L, e != nullptr);
    return 1;
}

int _lua_bind_bullet_set_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->position.x = luaL_checknumber(L, 2);
        e->position.y = luaL_checknumber(L, 3);
        e->last_position = e->position;
    }
    return 0;
}

int _lua_bind_bullet_set_relative_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->relative_position.x = luaL_checknumber(L, 2);
        e->relative_position.y = luaL_checknumber(L, 3);
        // e->last_position = e->position;
    }
    return 0;
}

int _lua_bind_bullet_reset_relative_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->relative_position.x = 0;
        e->relative_position.y = 0;
        // e->last_position = e->position;
    }
    return 0;
}

int _lua_bind_bullet_set_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->scale.x = luaL_checknumber(L, 2);
        e->scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_enemy_set_visual(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_enemy(uid);
    if (e) {
        e->sprite = sprite_instance(
            state->resources->entity_sprites[
                luaL_checkinteger(L, 2)
            ]
        );
    }
    return 0;
}

int _lua_bind_bullet_set_visual_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->sprite.scale.x = luaL_checknumber(L, 2);
        e->sprite.scale.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_velocity(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->velocity.x = luaL_checknumber(L, 2);
        e->velocity.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_acceleration(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->acceleration.x = luaL_checknumber(L, 2);
        e->acceleration.y = luaL_checknumber(L, 3);
    }
    return 0;
}

int _lua_bind_bullet_set_task(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    char* task_name = (char*)lua_tostring(L, 2);

    lua_remove(L, 1); lua_remove(L, 1);

    auto e = state->gameplay_data.lookup_bullet(uid);

	if (e) {
		s32 remaining = lua_gettop(L);
		_debugprintf("bullet async task with %d elements [%d actual stack top]", remaining, lua_gettop(L));
		state->coroutine_tasks.add_bullet_lua_game_task(
			state,
			L,
			state->coroutine_tasks.L,
			task_name,
			uid
		);
	}

    return 0;
}

int _lua_bind_bullet_set_max_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);

    if (e) {
        e->maximum_speed = luaL_checknumber(L, 2);
    }

    return 0;
}

int _lua_bind_bullet_get_max_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);

    if (e) {
        lua_pushnumber(L, e->maximum_speed);
        return 1;
    }

    return 0;
}

int _lua_bind_bullet_reset_movement(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->reset_movement();
    }
    return 0;
}

int _lua_bind_bullet_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_relative_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->relative_position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_relative_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->relative_position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_acceleration_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_acceleration_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    lua_pushlightuserdata(L, e);
    return 1;
}

int _lua_bind_bullet_start_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_limit = luaL_checkinteger(L, 2);
    }
    return 0;
}
int _lua_bind_bullet_set_trail_modulation(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_modulation = color32f32(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), 1);
        e->trail_ghost_max_alpha = luaL_checknumber(L, 5);
    }
    return 0;
}
int _lua_bind_bullet_set_trail_record_speed(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_record_timer_max = luaL_checknumber(L, 2);
    }
    return 0;
}

int _lua_bind_bullet_stop_trail(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->trail_ghost_limit = 0;
    }
    return 0;
}

int _lua_bind_bullet_set_visual(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->sprite = sprite_instance(
            state->resources->projectile_sprites[
                luaL_checkinteger(L, 2)
            ]
        );
    }
    return 0;
}

int _lua_bind_bullet_set_lifetime(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->lifetime = Timer(luaL_checknumber(L, 2));
    }
    return 0;
}
int _lua_bind_bullet_lifetime(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.t);
        return 1;
    }
    return 0;
}
int _lua_bind_bullet_lifetime_max(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.max_t);
        return 1;
    }
    return 0;
}
int _lua_bind_bullet_lifetime_percent(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushnumber(L, e->lifetime.percentage());
        return 1;
    }
    return 0;
}

int _lua_bind_bullet_get_particle_emitter(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        lua_pushlightuserdata(L, &e->emitters[luaL_checkinteger(L, 2)]);
        return 1;
    }
    return 0;
}


int _lua_bind_bullet_kill(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    u64 uid = luaL_checkinteger(L, 1);
    auto e = state->gameplay_data.lookup_bullet(uid);
    if (e) {
        e->die = true;
        return 0;
    }
    return 0;
}

// Misc entities
int _lua_bind_explosion_hazard_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    V2 position = V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    f32 radius = luaL_checknumber(L, 3);
    f32 warning_time = luaL_checknumber(L, 4);
    f32 time_until_explosion = luaL_checknumber(L, 5);

    state->gameplay_data.add_explosion_hazard(
        Explosion_Hazard(
            position,
            radius,
            warning_time,
            time_until_explosion
        )
    );
    return 0;
}

int _lua_bind_laser_hazard_new(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);

    f32 position = luaL_checknumber(L, 1);
    f32 radius = luaL_checknumber(L, 2);
    s32 direction = luaL_checkinteger(L, 3);
    f32 warning_time = luaL_checknumber(L, 4);
    f32 lifetime = luaL_checknumber(L, 5);
    s32 projectile_sprite_type = -1;
    
    if (lua_gettop(L) >= 6) {
      projectile_sprite_type = lua_tointeger(L, 6);
    }

    auto laser = Laser_Hazard(
      position,
      radius,
      direction,
      warning_time,
      lifetime
    );
    laser.projectile_sprite_id = projectile_sprite_type;

    state->gameplay_data.add_laser_hazard(
      laser  
    );
    return 0;
}

// Player bindings
int _lua_bind_player_to_ptr(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    lua_pushlightuserdata(L, e);
    return 1;
}
int _lua_bind_player_hp(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->hp);
        return 1;
    }
    return 0;
}
int _lua_bind_player_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->position.x);
        return 1;
    }
    return 0;
}

int _lua_bind_player_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->position.y);
        return 1;
    }
    return 0;
}

int _lua_bind_player_velocity_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->velocity.x);
        return 1;
    }
    return 0;
}

int _lua_bind_player_velocity_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto e = &state->gameplay_data.player;
    if (e) {
        lua_pushnumber(L, e->velocity.y);
        return 1;
    }
    return 0;
}

int _lua_bind_player_disable_burst_charge(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  auto e = &state->gameplay_data.player;
  if (e) {
    e->disable_burst_charge_regeneration();
  }
  return 0;
}

int _lua_bind_player_enable_burst_charge(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  auto e = &state->gameplay_data.player;
  if (e) {
    e->enable_burst_charge_regeneration();
  }
  return 0;
}

int _lua_bind_halt_burst_charge_regen(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  auto e = &state->gameplay_data.player;
  if (e) {
    int flashes = luaL_checkinteger(L, 0);
    e->halt_burst_charge_regeneration(flashes);
  }
  return 0;
}

// function Generic_Infinite_Stage_ScrollV_Ref(image_resource_location, scrollspeed, offx, offy, layer)
int _lua_bind_spawn_simple_scrollable_background_vertical(lua_State* L) {
    Game_State* state        = lua_binding_get_gamestate(L);
    char*       image_path   = (char*) lua_tostring(L, 1);
    f32         scroll_speed = luaL_checknumber(L, 2);
    f32         offx         = luaL_checknumber(L, 3);
    f32         offy         = luaL_checknumber(L, 4);
    s32         layer        = luaL_checkinteger(L, 5);

    auto bkg = state->gameplay_data.simple_scrollable_backgrounds.allocate_background(
        layer
    );

    *bkg.image_id      = state->gameplay_data.script_load_image(state->resources, image_path);
    // NOTE(jerry): hard-coded
    *bkg.scale         = V2(SIMPLE_BACKGROUND_WIDTH, SIMPLE_BACKGROUND_HEIGHT);
    *bkg.scroll_y      = offy;
    *bkg.scroll_x      = offx;
    *bkg.scroll_speed_y = scroll_speed;
    *bkg.scroll_speed_x = 0;

    return 0;
}

void bind_entity_lualib(lua_State* L) {
    /*
        NOTE: the lib is only in scalar values which isn't very good, but it's
            simple and flexible enough to play with.

        Everything will probably remain as individual parameters since it requires less library/parsing
        work, but there are bindings to the math functions inside this engine...

        NOTE: I am distinctly aware that I could make these functions more generic if I just "buff" the uid,
        but I like being more explicit for stuff.

        Technically speaking bullets and enemies are actually extremely similar...
     */
    {
        // behavior setting
        lua_register(L, "_enemy_to_ptr", _lua_bind_enemy_to_ptr);
        lua_register(L, "enemy_new", _lua_bind_enemy_new);
        lua_register(L, "enemy_valid", _lua_bind_enemy_valid);
        lua_register(L, "enemy_set_position", _lua_bind_enemy_set_position);
        lua_register(L, "enemy_set_relative_position", _lua_bind_enemy_set_relative_position);
        lua_register(L, "enemy_reset_relative_position", _lua_bind_enemy_reset_relative_position);
        lua_register(L, "enemy_set_visual_scale", _lua_bind_enemy_set_visual_scale);
        lua_register(L, "enemy_set_visual", _lua_bind_enemy_set_visual);
        lua_register(L, "enemy_set_scale", _lua_bind_enemy_set_scale);
        lua_register(L, "enemy_set_velocity", _lua_bind_enemy_set_velocity);
        lua_register(L, "enemy_set_acceleration", _lua_bind_enemy_set_acceleration);
        lua_register(L, "enemy_set_animation_frame", _lua_bind_enemy_set_animation_frame);
        lua_register(L, "enemy_set_hp", _lua_bind_enemy_set_hp);
        lua_register(L, "enemy_set_task", _lua_bind_enemy_set_task);
        lua_register(L, "enemy_reset_movement", _lua_bind_enemy_reset_movement);
        lua_register(L, "enemy_kill", _lua_bind_enemy_kill);
        lua_register(L, "enemy_hurt", _lua_bind_enemy_hurt);
        lua_register(L, "enemy_start_trail", _lua_bind_enemy_start_trail);
        lua_register(L, "enemy_set_trail_modulation", _lua_bind_enemy_set_trail_modulation);
        lua_register(L, "enemy_set_trail_record_speed", _lua_bind_enemy_set_trail_record_speed);
        lua_register(L, "enemy_stop_trail", _lua_bind_enemy_stop_trail);
        lua_register(L, "enemy_begin_invincibility", _lua_bind_enemy_begin_invincibility);
        lua_register(L, "enemy_end_invincibility", _lua_bind_enemy_end_invincibility);
        lua_register(L, "enemy_set_oob_deletion", _lua_bind_enemy_set_oob_deletion);

        lua_register(L, "enemy_show_boss_hp", _lua_bind_enemy_show_boss_hp);
        lua_register(L, "enemy_hide_boss_hp", _lua_bind_enemy_hide_boss_hp);

        // reading
        lua_register(L, "enemy_visual_loop_completions", _lua_bind_enemy_visual_loop_completions);
        lua_register(L, "enemy_time_since_spawn", _lua_bind_enemy_time_since_spawn);
        lua_register(L, "enemy_position_x", _lua_bind_enemy_position_x);
        lua_register(L, "enemy_position_y", _lua_bind_enemy_position_y);
        lua_register(L, "enemy_relative_position_x", _lua_bind_enemy_relative_position_x);
        lua_register(L, "enemy_relative_position_y", _lua_bind_enemy_relative_position_y);
        lua_register(L, "enemy_velocity_x", _lua_bind_enemy_velocity_x);
        lua_register(L, "enemy_velocity_y", _lua_bind_enemy_velocity_y);
        lua_register(L, "enemy_acceleration_x", _lua_bind_enemy_acceleration_x);
        lua_register(L, "enemy_acceleration_y", _lua_bind_enemy_acceleration_y);
        lua_register(L, "enemy_set_max_speed", _lua_bind_enemy_set_max_speed);
        lua_register(L, "enemy_get_max_speed", _lua_bind_enemy_get_max_speed);
        lua_register(L, "enemy_hp", _lua_bind_enemy_hp);
        lua_register(L, "enemy_hp_percent", _lua_bind_enemy_hp_percent);
        lua_register(L, "enemy_get_particle_emitter", _lua_bind_enemy_get_particle_emitter);

        // bullet behavior setting. (there is no reason to read from a bullet)
        lua_register(L, "_bullet_ptr", _lua_bind_bullet_to_ptr);
        lua_register(L, "bullet_new", _lua_bind_bullet_new);
        lua_register(L, "bullet_set_source", _lua_bind_bullet_set_source);
        lua_register(L, "bullet_time_since_spawn", _lua_bind_bullet_time_since_spawn);
        lua_register(L, "bullet_valid", _lua_bind_bullet_valid);
        lua_register(L, "bullet_set_position", _lua_bind_bullet_set_position);
        lua_register(L, "bullet_set_scale", _lua_bind_bullet_set_scale);
        lua_register(L, "bullet_set_visual_scale", _lua_bind_bullet_set_visual_scale);
        lua_register(L, "bullet_set_velocity", _lua_bind_bullet_set_velocity);
        lua_register(L, "bullet_set_acceleration", _lua_bind_bullet_set_acceleration);
        lua_register(L, "bullet_start_trail", _lua_bind_bullet_start_trail);
        lua_register(L, "bullet_set_trail_modulation", _lua_bind_bullet_set_trail_modulation);
        lua_register(L, "bullet_set_trail_record_speed", _lua_bind_bullet_set_trail_record_speed);
        lua_register(L, "bullet_stop_trail", _lua_bind_bullet_stop_trail);
        lua_register(L, "bullet_set_visual", _lua_bind_bullet_set_visual);
        lua_register(L, "bullet_set_task", _lua_bind_bullet_set_task);
        lua_register(L, "bullet_set_relative_position", _lua_bind_bullet_set_relative_position);
        lua_register(L, "bullet_reset_relative_position", _lua_bind_bullet_reset_relative_position);
        lua_register(L, "bullet_set_max_speed", _lua_bind_bullet_set_max_speed);
        lua_register(L, "bullet_get_max_speed", _lua_bind_bullet_get_max_speed);
        lua_register(L, "bullet_kill", _lua_bind_bullet_kill);
        lua_register(L, "bullet_reset_movement", _lua_bind_bullet_reset_movement);
        lua_register(L, "bullet_set_lifetime", _lua_bind_bullet_set_lifetime);

        // These might be okay to read for a bullet.
        lua_register(L, "bullet_position_x", _lua_bind_bullet_position_x);
        lua_register(L, "bullet_position_y", _lua_bind_bullet_position_y);
        lua_register(L, "bullet_relative_position_x", _lua_bind_bullet_relative_position_x);
        lua_register(L, "bullet_relative_position_y", _lua_bind_bullet_relative_position_y);
        lua_register(L, "bullet_velocity_x", _lua_bind_bullet_velocity_x);
        lua_register(L, "bullet_velocity_y", _lua_bind_bullet_velocity_y);
        lua_register(L, "bullet_lifetime", _lua_bind_bullet_lifetime);
        lua_register(L, "bullet_lifetime_max", _lua_bind_bullet_lifetime_max);
        lua_register(L, "bullet_lifetime_percent", _lua_bind_bullet_lifetime_percent);
        lua_register(L, "bullet_get_particle_emitter", _lua_bind_bullet_get_particle_emitter);

        // Player is READONLY. All things done to the player are only through the engine code.
        lua_register(L, "_player_ptr", _lua_bind_player_to_ptr);
        lua_register(L, "player_position_x", _lua_bind_player_position_x);
        lua_register(L, "player_position_y", _lua_bind_player_position_y);
        lua_register(L, "player_velocity_x", _lua_bind_player_velocity_x);
        lua_register(L, "player_velocity_y", _lua_bind_player_velocity_y);
        lua_register(L, "player_hp", _lua_bind_player_hp);
        // except for these I guess...
        lua_register(L, "disable_burst_charge", _lua_bind_player_disable_burst_charge);
        lua_register(L, "enable_burst_charge", _lua_bind_player_enable_burst_charge);
        lua_register(L, "halt_burst_charge_regen", _lua_bind_halt_burst_charge_regen);

        lua_register(L, "explosion_hazard_new", _lua_bind_explosion_hazard_new);
        lua_register(L, "laser_hazard_new", _lua_bind_laser_hazard_new);

        // Simple_Scrollable_Background_Entities
        lua_register(L, "Generic_Infinite_Stage_ScrollV", _lua_bind_spawn_simple_scrollable_background_vertical);
    }
}

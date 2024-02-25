#include "game.h"
#include "entity.h"
#include "game_state.h"
#include "game_uid_generator.h"
#include "virtual_file_system.h"

#include "lua_binding_macro.h"

GAME_LUA_MODULE(
core,
"src/core_lua_bindings_generated.cpp",
"Core library to work with the Bullet Hell scripting environment.",
"This provides access to some core functionalities that allow for scripting levels\
within the engine. Some wrappers for some basic behaviors to support the replay system\
as well as things awaiters for coroutine support.\
\
There is a lua support module that also has some useful features, but this is only for the\
engine implemented part. Refer to common.lua for the module."
)

GAME_LUA_PROC(
t_wait,
"seconds: number",
"Wait for the specified number of seconds.",
"Yield for the specified number of seconds, and return execution after finishing.\
This is a coroutine."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");
    f32         wait_time = luaL_checknumber(L, 1);

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_SECONDS;
    task.userdata.yielded.timer = 0.0f;
    task.userdata.yielded.timer_max = wait_time;

    // _debugprintf("LuaThread(%p) wants to wait for %f seconds", L, wait_time);
    // _debugprintf("LuaThread(%p)(taskid: %d) wants to wait for %f", L, task_id, wait_time);

    return lua_yield(L, 0);
}

GAME_LUA_PROC(
t_fwait,
"seconds: number",
"Wait for the specified number of seconds.",
"Yield for the specified number of seconds, and return execution after finishing.\
This is a coroutine."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");
    f32         wait_time = luaL_checknumber(L, 1);

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_SECONDS;
    task.userdata.yielded.timer = 0.0f;
    task.userdata.yielded.timer_max = wait_time;

    // _debugprintf("LuaThread(%p) wants to wait for %f seconds", L, wait_time);
    // _debugprintf("LuaThread(%p)(taskid: %d) wants to wait for %f", L, task_id, wait_time);

    return lua_yield(L, 0);
}

GAME_LUA_PROC(
t_yield,
"",
"Yield execution here.",
"Yield execution. This is a coroutine procedure."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_COMPLETE_STAGE;

    return lua_yield(L, 0);
}

GAME_LUA_PROC(
t_complete_stage,
"",
"Yield execution and set stage completed flag.",
"Yield execution and set stage completed flag. Should be the last function in stage_main."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_COMPLETE_STAGE;

    return lua_yield(L, 0);
}

GAME_LUA_PROC(
t_wait_for_no_danger,
"",
"Yield execution and until there are no more hazards.",
"Yield execution and until there are no more hazards. These include bullets and enemies but not environmental hazards."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_NO_DANGER_ON_STAGE;

    return lua_yield(L, 0);
}

GAME_LUA_PROC(
prng_ranged_float,
"a: number, b: number -> number",
"Return a ranged random float.",
"Return a ranged random float from the engine PRNG system. Use this instead of luas prng,\
otherwise your stage will break the replay system."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    f32 a = luaL_checknumber(L, 1);
    f32 b = luaL_checknumber(L, 2);
    lua_pushnumber(L, random_ranged_float(&state->gameplay_data.prng, a, b));
    return 1;
}

GAME_LUA_PROC(
prng_ranged_integer,
"a: number, b: number -> number",
"Return a ranged random integer.",
"Return a ranged random integer from the engine PRNG system. Use this instead of luas prng,\
otherwise your stage will break the replay system."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    s32 a = luaL_checkinteger(L, 1);
    s32 b = luaL_checkinteger(L, 2);
    lua_pushnumber(L, random_ranged_integer(&state->gameplay_data.prng, a, b));
    return 1;
}

GAME_LUA_PROC(
prng_normalized_float,
"-> number",
"Return a random normalized float.",
"Return a random normalized float from the engine PRNG system. Use this instead of luas prng,\
otherwise your stage will break the replay system."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushnumber(L, random_float(&state->gameplay_data.prng));
    return 1;
}


GAME_LUA_PROC(
camera_traumatize,
"trauma: number",
"Traumatize the gameplay camera.",
"Traumatize the gameplay camera as if through a punch. Which will repeatedly shake it and eventually wear off."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    camera_traumatize(&state->gameplay_data.main_camera, luaL_checknumber(L, 1));
    return 0;
}

GAME_LUA_PROC(
camera_set_trauma,
"trauma: number",
"Set trauma level to gameplay camera.",
"Traumatize the gameplay camera. Which will repeatedly shake it and eventually wear off."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    camera_set_trauma(&state->gameplay_data.main_camera, luaL_checknumber(L, 1));
    return 0;
}

GAME_LUA_PROC(
async_task,
"function_name: string, ...: varargs",
"Fire an arbitrary lua function as an async task.",
"Given a lua function name as a string, and arbitrary arguments. Fire it in the background as a task."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    char* task_name = (char*)lua_tostring(L, 1);
    lua_remove(L, 1);
    _debugprintf("Assign async task %s", task_name);

    s32 remaining = lua_gettop(L);
    _debugprintf("Async task with %d elements [%d actual stack top]", remaining, lua_gettop(L));

    state->coroutine_tasks.add_lua_game_task(
        state,
        L,
        state->coroutine_tasks.L,
        task_name
    );
    return 0;
}

GAME_LUA_PROC(
alloc_particle_emitter,
"-> Particle_Emitter*",
"(not safe)Allocate a particle emitter.",
"Allocate an avaliable particle emitter from the gameplay data pool."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto gameplay_data = state->gameplay_data;
    Particle_Emitter* emitter = gameplay_data.particle_emitters.alloc();
    
    if (emitter) {
        lua_pushlightuserdata(L, emitter);
        return 1;
    }

    return 0;
}

GAME_LUA_PROC(
global_elapsed_time,
"-> number",
"Return the total amount of elapsed time.",
"Return the total amount of elapsed time since engine startup."
)
{
    auto engine = Global_Engine();
    lua_pushnumber(L, engine->global_elapsed_time);
    return 1;
}

GAME_LUA_PROC(
ticktime,
"-> number",
"Return the fixed tickrate.",
"Return the fixed tickrate of the engine for update purposes."
)
{
    lua_pushnumber(L, FIXED_TICKTIME);
    return 1;
}

GAME_LUA_PROC(
engine_dofile,
"filename: string -> EVAL CODE",
"Wrapper for dofile.",
"Wrapper for dofile that understand the VFS for the game. Use this instead of dofile to load archived files."
)
{
    int _lua_bind_engine_dofile(lua_State* L);
    return _lua_bind_engine_dofile(L); // in virtual_file_system.cpp
}

#include "core_lua_bindings_generated.cpp"

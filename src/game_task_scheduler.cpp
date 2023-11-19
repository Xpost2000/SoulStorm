#include "game_task_scheduler.h"
#include "game_state.h"

Game_Task_Scheduler::Game_Task_Scheduler() {

}

Game_Task_Scheduler::Game_Task_Scheduler(Memory_Arena* arena, s32 task_count) {
    tasks           = Fixed_Array<Game_Task>(arena, task_count);
    active_task_ids = Fixed_Array<s32>(arena, task_count);
}

// Game_Task_Scheduler
s32 Game_Task_Scheduler::first_avaliable_task() {
    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];

        if (task.source == GAME_TASK_AVALIABLE                                  ||
            jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED ||
            task.last_L_C_status                  == JDR_DUFFCOROUTINE_FINISHED) {
            return index;
        }
    }

    return -1;
}

s32 Game_Task_Scheduler::add_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential) {
    return add_task(state, f, nullptr, essential);
}

s32 Game_Task_Scheduler::add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential) {
    return add_ui_task(state, f, nullptr, essential);
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f) {
   return add_global_task(f, nullptr);
}

s32 Game_Task_Scheduler::add_task(struct Game_State* state, jdr_duffcoroutine_fn f, void* userdata, bool essential) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return -1;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = essential;
    task.userdata.game_state = state;
    task.userdata.userdata = userdata;
    task.coroutine           = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    active_task_ids.push(first_free);
    _debugprintf("add task (%d active tasks)", (int)active_task_ids.size);
    return first_free;
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f, void* userdata) {
    s32 first_free       = first_avaliable_task();
    if (first_free == -1) return -1;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_ALWAYS;
    task.associated_state = -1;
    task.essential        = true;
    task.coroutine        = jdr_coroutine_new(f);
    task.userdata.userdata = userdata;
    // NOTE: need userdata info.
    task.coroutine.userdata = &task.userdata;

    active_task_ids.push(first_free);
    return first_free;
}

s32 Game_Task_Scheduler::add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, void* userdata, bool essential) {
    s32 current_ui_state = state->ui_state;
    s32 first_free       = first_avaliable_task();

    if (first_free == -1) return -1;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_UI;
    task.associated_state = current_ui_state;
    task.essential        = essential;
    task.userdata.game_state = state;
    task.coroutine        = jdr_coroutine_new(f);
    task.userdata.userdata = userdata;
    task.coroutine.userdata = &task.userdata;

    active_task_ids.push(first_free);
    return first_free;
}

// NOTE: careful about memory leaks regarding lua tasks.
s32 Game_Task_Scheduler::add_lua_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, bool essential) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1)   return -1;
    if (this->L == nullptr) this->L = L;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = essential;
    task.userdata.game_state = state;

    lua_getglobal(L, "_coroutinetable");
    task.L_C                 = lua_newthread(L);
    {
        size_t current_len = lua_rawlen(L, -2);
        lua_rawseti(L, -2, current_len+1);
        task.thread_table_location = current_len+1;
    }
    luaL_dostring(L, "print(_coroutinetable);");
    lua_settop(L, 0);

    lua_getglobal(task.L_C, fn_name);
    if (caller_co) {
        _debugprintf("called from subtask. Copying extra args!");
        task.nargs = lua_gettop(caller_co);
        lua_xmove(caller_co, task.L_C, task.nargs);
    }

    // push_all_variants_to_lua_stack(task.L_C, parameters);
    // task.nargs = parameters.length;

    _debugprintf("Lua task (%p) assigned to coroutine on (%d args!) : %s", task.L_C, task.nargs, fn_name);
    cstring_copy(fn_name, task.fn_name, array_count(task.fn_name)-1);
    active_task_ids.push(first_free);
    return first_free;
}

s32 Game_Task_Scheduler::add_lua_entity_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, u64 uid, s8 type) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return -1;
    this->L = L;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = false;
    task.userdata.game_state = state;

    lua_getglobal(L, "_coroutinetable");
    task.L_C                 = lua_newthread(L);
    {
        size_t current_len = lua_rawlen(L, -2);
        lua_rawseti(L, -2, current_len+1);
        task.thread_table_location = current_len+1;
    }
    // task.top_location        = lua_gettop(L);
    luaL_dostring(L, "print(_coroutinetable);");
    lua_settop(L, 0);

    lua_getglobal(task.L_C, fn_name);
    lua_pushinteger(task.L_C, uid);

    task.nargs = 1;
    task.uid   = uid;
    task.uid_type = type;
    if (caller_co) {
        _debugprintf("called from subtask. Copying extra args!");
        int args = lua_gettop(caller_co);
        task.nargs += args;
        lua_xmove(caller_co, task.L_C, args);
    }

    _debugprintf("Lua task (%p) assigned to coroutine on (called with %d arguments) : %s", task.L_C, task.nargs, fn_name);
    cstring_copy(fn_name, task.fn_name, array_count(task.fn_name)-1);
    active_task_ids.push(first_free);
    return first_free;
}

s32 Game_Task_Scheduler::add_bullet_lua_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, u64 uid) {
    return add_lua_entity_game_task(state, caller_co, L, fn_name, uid, 1);
}

s32 Game_Task_Scheduler::add_enemy_lua_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, u64 uid) {
    return add_lua_entity_game_task(state, caller_co, L, fn_name, uid, 2);
}

bool Game_Task_Scheduler::kill_task(s32 index) {
    auto& task            = tasks[index];
    if (task.source != GAME_TASK_AVALIABLE) {
        task.source = GAME_TASK_AVALIABLE;
        task.last_L_C_status = JDR_DUFFCOROUTINE_FINISHED;
    } else {
        return false; // already dead.
    }
    return true; 
}

s32 Game_Task_Scheduler::search_for_lua_task(lua_State* L) {
    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        if (task.L_C == L) {
            return active_task_ids[index];
        } else {
            continue;
        }
    }
    return -1;
}

void Game_Task_Scheduler::abort_all_lua_tasks() {
    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        if (task.L_C) {
            kill_task(active_task_ids[index]);
        }
    }
}

void Game_Task_Scheduler::scheduler(struct Game_State* state, f32 dt) {
    s32 current_ui_state     = state->ui_state;
    s32 current_screen_state = state->screen_mode;

    // cleanup dead tasks
    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        bool delete_task = false;
        if (task.L_C) {
            switch (task.uid_type) {
                case 0: {} break;
                case 1: {
                    auto b = state->gameplay_data.lookup_bullet(task.uid);
                    if (!b) {
                        _debugprintf("Associated task master(bullet) is dead.");
                        delete_task = true;
                    }
                } break;
                case 2: {
                    auto e = state->gameplay_data.lookup_enemy(task.uid);
                    if (!e) {
                        _debugprintf("Associated task master(enemy) is dead.");
                        delete_task = true;
                    }
                } break;
            }
            if (task.last_L_C_status == JDR_DUFFCOROUTINE_FINISHED) {
                delete_task = true;
            }

            if (delete_task) {
                // deregister self from the stack...
                lua_getglobal(L, "_coroutinetable");
                // lua_rawgeti(L, -1, task.thread_table_location);
                lua_pushnil(L);
                lua_rawseti(L, -2, task.thread_table_location);
                // lua_remove(L, task.top_location);
            }
        } else {
            if (jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED) {
                delete_task = true;
            }
        }

        if (delete_task) {
            // NOTE: This is dangerous?
            _debugprintf("Killed task(%s)", task.fn_name);
            zero_memory(&task, sizeof(task));
            active_task_ids.pop_and_swap(index);   
        }
    }

    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        task.userdata.dt = dt;

        {
            switch (task.userdata.yielded.reason) {
                case TASK_YIELD_REASON_NONE: {} break;
                case TASK_YIELD_REASON_WAIT_FOR_NO_DANGER_ON_STAGE: {
                    if (!state->gameplay_data.any_hazards())
                        task.userdata.yielded.reason = TASK_YIELD_REASON_NONE;
                    continue;
                } break;
                case TASK_YIELD_REASON_COMPLETE_STAGE: {
                    state->gameplay_data.stage_completed = true;
                    task.userdata.yielded.reason = TASK_YIELD_REASON_NONE;
                } break;
                case TASK_YIELD_REASON_WAIT_FOR_INTRODUCTION_SEQUENCE_TO_COMPLETE: {
                    if (state->gameplay_data.intro.stage != GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE) {
                        continue;
                    }
                } break;
                case TASK_YIELD_REASON_WAIT_FOR_SECONDS: {
                    if (task.userdata.yielded.timer < task.userdata.yielded.timer_max) {
                        /*
                         * Special case for Game tasks, which should logically
                         * not advance if the game is in any UI.
                         */
                        if (task.source == GAME_TASK_SOURCE_GAME &&
                            task.associated_state == GAME_SCREEN_INGAME &&
                            (
                                current_screen_state != GAME_SCREEN_INGAME ||
                                state->gameplay_data.paused_from_death ||
                                state->gameplay_data.triggered_stage_completion_cutscene ||
                                current_ui_state != UI_STATE_INACTIVE
                            ))
                            break;

                        task.userdata.yielded.timer += dt;
                        continue;
                    } else {
                        task.userdata.yielded.reason = TASK_YIELD_REASON_NONE;
                    }
                } break;
            }
        }

        if (
            (task.source == GAME_TASK_SOURCE_ALWAYS)                                          ||
            (task.source == GAME_TASK_SOURCE_UI && task.associated_state == current_ui_state) ||
            (task.source == GAME_TASK_SOURCE_GAME && task.associated_state == current_screen_state)
        ) {
            /*
             * special case for gameplay.
             * This will tell me how legal it is to run the game.
             */
            if ((task.source == GAME_TASK_SOURCE_GAME &&
                 task.associated_state == GAME_SCREEN_INGAME &&
                 (
                     state->gameplay_data.paused_from_death ||
                     state->gameplay_data.triggered_stage_completion_cutscene ||
                     current_ui_state != UI_STATE_INACTIVE
                 )
                )
            ) {
                continue;
            }

            // We either have lua powered tasks or native tasks.
            if (task.L_C && task.last_L_C_status != JDR_DUFFCOROUTINE_FINISHED) {
                s32 _nres;
                s32 status = lua_resume(task.L_C, L, task.nargs, &_nres);
                if (status == LUA_YIELD) { 
                    status = JDR_DUFFCOROUTINE_SUSPENDED;
                }
                else if (status != LUA_YIELD) { 
                    static const char* lua_codes[] = { "LUA_OK", "LUA_YIELD", "LUA_ERRRUN", "LUA_ERRSYNTAX", "LUA_ERRMEM", "LUA_ERRERR" };

                    _debugprintf("LuaTask(%s): Code Status: %d (%s)", task.fn_name, status, lua_codes[status]);
                    if (status != LUA_OK) {
                        _debugprintf("(%s) error?", lua_tostring(task.L_C,-1));
                    }
                    status = JDR_DUFFCOROUTINE_FINISHED; 
                }
                task.last_L_C_status = status;
            } else {
                if (task.coroutine.f) jdr_resume(&task.coroutine);
            }
        }
    }
    if (L) lua_gc(L, LUA_GCCOLLECT);
}

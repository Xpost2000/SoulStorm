#include "game_task_scheduler.h"
#include "game_state.h"

local void setup_generic_task(Game_Task* task, s32 source_type, s32 associated_state, bool essential, Game_State* state, void* userdata) {
    task->source              = source_type;
    task->associated_state    = associated_state;
    task->essential           = essential;
    task->userdata.game_state = state;
    task->userdata.userdata   = userdata;
}

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

        // We have lua active. Do not try to return.
        if (L && task.L_C) continue;

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
    setup_generic_task(&task, GAME_TASK_SOURCE_GAME, current_screen_state, essential, state, userdata);
    task.coroutine          = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    active_task_ids.push(first_free);
    _debugprintf("add task (%d active tasks)", (int)active_task_ids.size);
    return first_free;
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f, void* userdata) {
    s32 first_free       = first_avaliable_task();
    if (first_free == -1) return -1;

    auto& task             = tasks[first_free];
    setup_generic_task(&task, GAME_TASK_SOURCE_ALWAYS, -1, true, nullptr, userdata);
    task.coroutine         = jdr_coroutine_new(f);
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
    setup_generic_task(&task, GAME_TASK_SOURCE_UI, current_ui_state, essential, state, userdata);
    task.coroutine        = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    active_task_ids.push(first_free);
    return first_free;
}


void Game_Task_Scheduler::setup_lua_task(Game_Task* task, lua_State* caller_co, const char* fn_name) {
    lua_getglobal(L, "_coroutinetable");
    task->L_C = lua_newthread(L);

    // register self to global coroutine table in lua state
    // to avoid garbage collection until it's time.
    {
        size_t current_len = lua_rawlen(L, -2);
        lua_rawseti(L, -2, current_len+1);
        task->thread_table_location = current_len+1;
    }

    lua_settop(L, 0);

    lua_getglobal(task->L_C, fn_name);

    if (task->uid_type != 0) {
        task->nargs = 1;
        lua_pushinteger(task->L_C, task->uid);
    };

    if (caller_co) {
        _debugprintf("called from subtask. Copying extra args!");
        int args = lua_gettop(caller_co);
        task->nargs += args;
        lua_xmove(caller_co, task->L_C, args);
    }

    // _debugprintf("Lua task (%p) assigned to coroutine on (called with %d arguments) : %s", task->L_C, task->nargs, fn_name);
    cstring_copy((char*)fn_name, task->fn_name, array_count(task->fn_name)-1);
}

s32 Game_Task_Scheduler::add_lua_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, bool essential) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1)   return -1;
    if (this->L == nullptr) this->L = L;

    auto& task               = tasks[first_free];
    setup_generic_task(&task, GAME_TASK_SOURCE_GAME_FIXED, current_screen_state, essential, state, nullptr);
    setup_lua_task(&task, caller_co,fn_name);
    active_task_ids.push(first_free);
    return first_free;
}

s32 Game_Task_Scheduler::add_lua_entity_game_task(struct Game_State* state, lua_State* caller_co, lua_State* L, char* fn_name, u64 uid, s8 type) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return -1;
    if (this->L == nullptr) this->L = L;

    auto& task               = tasks[first_free];
    setup_generic_task(&task, GAME_TASK_SOURCE_GAME_FIXED, current_screen_state, false, state, nullptr);
    task.uid                 = uid;
    task.uid_type            = type;

    setup_lua_task(&task, caller_co, fn_name);
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

    deregister_all_dead_lua_threads();
}

void Game_Task_Scheduler::deregister_all_dead_lua_threads() {
    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        auto state = task.userdata.game_state;

        bool delete_task = false;
        if (task.L_C) {
            switch (task.uid_type) {
                case 0: {} break;
                case 1: {
                    auto b = state->gameplay_data.lookup_bullet(task.uid);
                    if (!b) {
                        _debugprintf("Associated task master(bullet) is dead. (%lld)", task.uid);
                        delete_task = true;
                    }
                } break;
                case 2: {
                    auto e = state->gameplay_data.lookup_enemy(task.uid);
                    if (!e) {
                        _debugprintf("Associated task master(enemy) is dead. (%lld)", task.uid);
                        delete_task = true;
                    }
                } break;
            }

            if (task.last_L_C_status == JDR_DUFFCOROUTINE_FINISHED) {
                delete_task = true;
            }

            if (delete_task && L) {
                // deregister self from the stack...
                lua_getglobal(L, "_coroutinetable");
                lua_pushnil(L);
                lua_rawseti(L, -2, task.thread_table_location);
            }
        }

        if (delete_task) {
            // NOTE: This is dangerous?
            _debugprintf("Killed lua task(%s)", task.fn_name);
            zero_memory(&task, sizeof(task));
            task.L_C = 0;
            active_task_ids.pop_and_swap(index);   
        }
    }

    // Run garbage collection cycle to hopefully collect orphan tasks.

    // maybe kill tasks later... (At the end of a level), so I'll turn off GC?
    // if (L) {lua_gc(L, LUA_GCCOLLECT, 0); }
}

void Game_Task_Scheduler::deregister_all_dead_standard_tasks() {
    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
        bool delete_task = false;

        if (task.L_C) {
            continue;
        }

        if (jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED) {
            delete_task = true;
        }

        if (delete_task) {
            // NOTE: This is dangerous?
            _debugprintf("Killed task(%s)", task.fn_name);
            zero_memory(&task, sizeof(task));
            active_task_ids.pop_and_swap(index);   
        }
    }
}

void Game_Task_Scheduler::schedule_by_type(struct Game_State* state, f32 dt, u8 type) {
    s32 current_ui_state     = state->ui_state;
    s32 current_screen_state = state->screen_mode;

    deregister_all_dead_lua_threads();
    deregister_all_dead_standard_tasks();

    const bool block_game_task_update =
        (
            current_screen_state != GAME_SCREEN_INGAME ||
            state->gameplay_data.paused_from_death ||
            state->gameplay_data.triggered_stage_completion_cutscene ||
            current_ui_state != UI_STATE_INACTIVE
        );

    for (s32 index = 0; index < active_task_ids.size; ++index) {
        auto& task = tasks[active_task_ids[index]];
       // _debugprintf("Active ID IDX: [%d] = %d", index, active_task_ids[index]);
        task.userdata.dt = dt;

        if (task.source != type) continue;

        const bool is_game_task = (task.source == GAME_TASK_SOURCE_GAME || task.source == GAME_TASK_SOURCE_GAME_FIXED);

        {
            switch (task.userdata.yielded.reason) {
                case TASK_YIELD_REASON_NONE: {
                    // NOTE: UID reliant tasks need to wait
                    // until their master has been born otherwise they might fail
                    // early, or otherwise quite before they're supposed to operate.
                    // (They might not work the same way even though they do operate on valid objects)
                    switch (task.uid_type) {
                        case 0: {} break;
                        case 1: {
                            if (!state->gameplay_data.bullet_spawned(task.uid))
                                continue;
                        } break;
                        case 2: {
                            if (!state->gameplay_data.entity_spawned(task.uid))
                                continue;
                        } break;
                    }
                } break;
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
                    if (is_game_task && task.associated_state == GAME_SCREEN_INGAME && block_game_task_update)
                        continue;

#if 0
                    bool game_task_skip_current_wait = is_game_task && !state->gameplay_data.any_hazards();
#else
                    bool game_task_skip_current_wait = false;
#endif
                    if (!game_task_skip_current_wait && task.userdata.yielded.timer < task.userdata.yielded.timer_max) {
                        task.userdata.yielded.timer += dt;
                        continue;
                    } else {
                        task.userdata.yielded.timer = 0;
                        task.userdata.yielded.reason = TASK_YIELD_REASON_NONE;
                    }
                } break;
            }
        }

        if (
            (task.source == GAME_TASK_SOURCE_ALWAYS)                                          ||
            (task.source == GAME_TASK_SOURCE_UI && task.associated_state == current_ui_state) ||
            (is_game_task && task.associated_state == current_screen_state)
        ) {
            if ((is_game_task && task.associated_state == GAME_SCREEN_INGAME && block_game_task_update)) {
                continue;
            }

            // We either have lua powered tasks or native tasks.
            if (task.L_C && task.last_L_C_status != JDR_DUFFCOROUTINE_FINISHED) {
                s32 _nres;
                s32 status = lua_resume(task.L_C, L, task.nargs, &_nres);
                if (_nres != 0) _debugprintf("_nres = %d", _nres);
                if (status == LUA_YIELD) { 
                    lua_gc(task.L_C, LUA_GCCOLLECT, 0);
                    status = JDR_DUFFCOROUTINE_SUSPENDED;
                }

                else if (status != LUA_YIELD) { 
                    static const char* lua_codes[] = {
                        "LUA_OK",
                        "LUA_YIELD",
                        "LUA_ERRRUN",
                        "LUA_ERRSYNTAX",
                        "LUA_ERRMEM",
                        "LUA_ERRERR"
                    };

                    // _debugprintf("LuaTask(%s): Code Status: %d (%s)", task.fn_name, status, lua_codes[status]);
                    if (status != LUA_OK) {
                        _debugprintf("LUA_ERROR (%s) error?", lua_tostring(task.L_C,-1));
                    }
                    status = JDR_DUFFCOROUTINE_FINISHED; 
                }
                task.last_L_C_status = status;
            } else {
                if (task.coroutine.f) jdr_resume(&task.coroutine);
            }
        }
    }
}

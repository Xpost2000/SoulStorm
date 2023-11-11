#include "game_task_scheduler.h"
#include "game_state.h"

// Game_Task_Scheduler
s32 Game_Task_Scheduler::first_avaliable_task() {
    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];

        if (task.source == GAME_TASK_AVALIABLE                                  ||
            jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED) {
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

    if (first_free == -1) return false;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = essential;
    task.userdata.game_state = state;
    task.userdata.userdata = userdata;
    task.coroutine           = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    return first_free;
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f, void* userdata) {
    s32 first_free       = first_avaliable_task();
    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_ALWAYS;
    task.associated_state = -1;
    task.essential        = true;
    task.coroutine        = jdr_coroutine_new(f);
    task.userdata.userdata = userdata;
    // NOTE: need userdata info.
    task.coroutine.userdata = &task.userdata;

    return first_free;
}

s32 Game_Task_Scheduler::add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, void* userdata, bool essential) {
    s32 current_ui_state = state->ui_state;
    s32 first_free       = first_avaliable_task();

    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_UI;
    task.associated_state = current_ui_state;
    task.essential        = essential;
    task.userdata.game_state = state;
    task.coroutine        = jdr_coroutine_new(f);
    task.userdata.userdata = userdata;
    task.coroutine.userdata = &task.userdata;

    return first_free;
}

// NOTE: careful about memory leaks regarding lua tasks.
s32  Game_Task_Scheduler::add_lua_game_task(Game_State* state, lua_State* L, char* fn_name, bool essential) {
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return false;
    this->L = L;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = essential;
    task.userdata.game_state = state;
    task.L_C                 = lua_newthread(L);

    lua_getglobal(task.L_C, fn_name);
    _debugprintf("Lua task (%p) assigned to coroutine on : %s", task.L_C, fn_name);
    return first_free;
}

bool Game_Task_Scheduler::kill_task(s32 index) {
    auto& task            = tasks[index];
    if (task.source != GAME_TASK_AVALIABLE) {
        task.source = GAME_TASK_AVALIABLE;
    } else {
        return false; // already dead.
    }
    return true; 
}

s32 Game_Task_Scheduler::search_for_lua_task(lua_State* L) {
    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];
        if (task.L_C && task.L_C == L) {
            return index;
        } else {
            continue;
        }
    }
    return -1;
}

void Game_Task_Scheduler::scheduler(struct Game_State* state, f32 dt) {
    s32 current_ui_state     = state->ui_state;
    s32 current_screen_state = state->screen_mode;

    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];
        task.userdata.dt = dt;

        if (task.L_C) {
            // NOTE: possible memory leak from lua. Need to invoke GC?
            if (task.last_L_C_status == JDR_DUFFCOROUTINE_FINISHED && task.source != GAME_TASK_AVALIABLE) {
                zero_memory(&task, sizeof(task));
            }
        } else {
            if (jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED && task.source != GAME_TASK_AVALIABLE) {
                zero_memory(&task, sizeof(task));
            }
        }

        {
            switch (task.userdata.yielded.reason) {
                case TASK_YIELD_REASON_NONE: {} break;
                case TASK_YIELD_REASON_COMPLETE_STAGE: {
                    state->gameplay_data.stage_completed = true;
                    task.userdata.yielded.reason = TASK_YIELD_REASON_NONE;
                } break;
                case TASK_YIELD_REASON_WAIT_FOR_SECONDS: {
                    if (task.userdata.yielded.timer < task.userdata.yielded.timer_max) {
                        _debugprintf("Waiting for timer to finish.");
                        /*
                         * Special case for Game tasks, which should logically
                         * not advance if the game is in any UI.
                         */
                        if (task.source == GAME_TASK_SOURCE_GAME && current_ui_state != UI_STATE_INACTIVE)
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
                     state->gameplay_data.intro.stage != GAMEPLAY_STAGE_INTRODUCTION_SEQUENCE_STAGE_NONE ||
                     state->gameplay_data.paused_from_death ||
                     state->gameplay_data.triggered_stage_completion_cutscene ||
                     current_ui_state != UI_STATE_INACTIVE
                 )
                )
            ) {
                continue;
            }

            // We either have lua powered tasks or native tasks.
            if (task.L_C) {
                s32 _nres;
                s32 status = lua_resume(task.L_C, L, 0, &_nres);
                task.last_L_C_status                 = status;
                if (status == LUA_YIELD)      status = JDR_DUFFCOROUTINE_SUSPENDED;
                if (status == LUA_OK)         status = JDR_DUFFCOROUTINE_FINISHED;
            } else {
                if (task.coroutine.f) jdr_resume(&task.coroutine);
            }
        }
    }
}

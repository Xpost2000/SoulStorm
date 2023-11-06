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
    s32 current_screen_state = state->screen_mode;
    s32 first_free           = first_avaliable_task();

    if (first_free == -1) return false;

    auto& task               = tasks[first_free];
    task.source              = GAME_TASK_SOURCE_GAME;
    task.associated_state    = current_screen_state;
    task.essential           = essential;
    task.userdata.game_state = state;
    task.coroutine           = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    return first_free;
}

s32 Game_Task_Scheduler::add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential) {
    s32 current_ui_state = state->ui_state;
    s32 first_free       = first_avaliable_task();

    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_UI;
    task.associated_state = current_ui_state;
    task.essential        = essential;
    task.userdata.game_state = state;
    task.coroutine        = jdr_coroutine_new(f);
    task.coroutine.userdata = &task.userdata;

    return first_free;
}

s32 Game_Task_Scheduler::add_global_task(jdr_duffcoroutine_fn f) {
    s32 first_free       = first_avaliable_task();
    if (first_free == -1) return false;

    auto& task            = tasks[first_free];
    task.source           = GAME_TASK_SOURCE_ALWAYS;
    task.associated_state = -1;
    task.essential        = true;
    task.coroutine        = jdr_coroutine_new(f);
    // NOTE: need userdata info.
    task.coroutine.userdata = &task.userdata;

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

void Game_Task_Scheduler::scheduler(struct Game_State* state, f32 dt) {
    s32 current_ui_state     = state->ui_state;
    s32 current_screen_state = state->screen_mode;

    for (s32 index = 0; index < tasks.capacity; ++index) {
        auto& task = tasks[index];

        if (jdr_coroutine_status(&task.coroutine) == JDR_DUFFCOROUTINE_FINISHED || task.source == GAME_TASK_AVALIABLE) {
            zero_memory(&task, sizeof(task));
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

        if ((task.source == GAME_TASK_SOURCE_ALWAYS)                                          ||
            (task.source == GAME_TASK_SOURCE_UI && task.associated_state == current_ui_state) ||
            (task.source == GAME_TASK_SOURCE_GAME && task.associated_state == current_screen_state)) {
            jdr_resume(&task.coroutine);
        }
    }
}

#ifndef GAME_TASK_SCHEDULER_H
#define GAME_TASK_SCHEDULER_H

#include "common.h"
#include "fixed_array.h"

/*
  Since the game is heavily based around state machines, this task
  scheduler actually needs to be aware of the current state of everything

  as when I change state, I don't want to keep performing tasks of previous
  states.

  This is just a "synchronous version" of my Thread_Pool

  NOTE: I'm not too sure how often I'll use it outside of the level code, (or otherwise really high level gameplay code)
  but it's a nice tool to have incase it pops up for anything else.

  NOTE: coroutines will execute at the end of the frame.
  TODO: should also schedule lua coroutines
*/
enum {
    GAME_TASK_AVALIABLE,
    GAME_TASK_SOURCE_UI,
    GAME_TASK_SOURCE_GAME,
    GAME_TASK_SOURCE_ALWAYS
};

enum { TASK_YIELD_REASON_NONE, TASK_YIELD_REASON_WAIT_FOR_SECONDS, };
struct Game_Task_Yield_Result {
    s32 reason = TASK_YIELD_REASON_NONE;
    f32 timer, timer_max;
};

#define TASK_WAIT(time) do {((Game_Task_Yield_Result*)(_jdr_current())->userdata)->timer_max = time; ((Game_Task_Yield_Result*)(_jdr_current())->userdata)->timer = 0.0f;((Game_Task_Yield_Result*)(_jdr_current())->userdata)->reason = TASK_YIELD_REASON_WAIT_FOR_SECONDS; JDR_Coroutine_YieldNR()} while(0)
struct Game_Task {
    u8              source = GAME_TASK_AVALIABLE;
    s32             associated_state;

    jdr_duffcoroutine_t coroutine;

    // Essential tasks will finish no matter what.
    // and cannot be killed normally.
    bool essential;
    Game_Task_Yield_Result yielded;

    /*
      NOTE:
      the coroutine's userdata will be a yield value,
      this userdata will be the game_state
    */
    void* userdata;
};


struct Game_State;
struct Game_Task_Scheduler {
    /*
      NOTE:
      To avoid changing code structure, lua coroutines/tasks are also
      here.

      Also for simplicity, since they're only for levels in that case,
      all lua tasks are GAME_TASKS
    */
    Fixed_Array<Game_Task> tasks;

    // NOTE: will override non-essential tasks.
    s32  add_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential=false);
    s32  add_global_task(jdr_duffcoroutine_fn f);
    s32  add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential=false);
    bool kill_task(s32 index);

    void scheduler(struct Game_State* state, f32 dt);
    s32  first_avaliable_task();
};

#endif

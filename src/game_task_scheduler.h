#ifndef GAME_TASK_SCHEDULER_H
#define GAME_TASK_SCHEDULER_H

#include "common.h"
#include "fixed_array.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

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

enum {
    TASK_YIELD_REASON_NONE,
    TASK_YIELD_REASON_WAIT_FOR_SECONDS,
    TASK_YIELD_REASON_COMPLETE_STAGE,
};
struct Game_Task_Yield_Result {
    s32 reason = TASK_YIELD_REASON_NONE;
    f32 timer, timer_max;
};

#define _Task_YieldData()     ((Game_Task_Yield_Result*)(_jdr_current())->userdata)
#define TASK_YIELD()          \
    do {JDR_Coroutine_YieldNR()} while(0)
#define TASK_WAIT(time)                                                 \
    do {_Task_YieldData()->timer_max = time; _Task_YieldData()->timer = 0.0f;_Task_YieldData()->reason = TASK_YIELD_REASON_WAIT_FOR_SECONDS; JDR_Coroutine_YieldNR()} while(0)
#define TASK_COMPLETE_STAGE() \
    do {_Task_YieldData()->reason    = TASK_YIELD_REASON_COMPLETE_STAGE; JDR_Coroutine_YieldNR()} while(0)

struct Game_State;

struct Game_Task_Userdata {
    // NOTE: This must be the first member in order for the above yielding
    // macros to work correctly!
    Game_Task_Yield_Result yielded;

    /*
      NOTE: game specific common task data.
      (
      since this is specifically designed for the stages to be written as coroutines,
      these are parameters that the stage update requires.

      This also happens to work for enemies/bullets or anything that looks like

      Object::method(Game_State* state, f32 dt), where userdata should be assumed to be the
      'this' pointer.

      I think this is more than enough for the needs of this game, but obviously a more
      fully-fledged scheduler would have more generic facilities.

      NOTE: I'm sandboxing the coroutine code from the lowest levels of the engine core, so it has no
      ability to render anything on it's own (and I'm not intending on exposing any real rendering APIs or
      anything that's shifty like the transitions API.) There are very few, and clear defined actions on
      most entity types like bosses.

      Not sure if bosses will use my coroutine system or just be written as state machines for simplicity...
      )

      Since this is intended to be for a game specific task manager
      it should be completely appropriate to allow this. It also
      means I don't get any funny questions regarding allocation
      of the userdata pointer since I often just need one more pointer.
    */
    Game_State*            game_state;
    void*                  userdata;
    f32                    dt; // current dt. Although we can look at the engine object for this.
};

struct Game_Task {
    u8              source = GAME_TASK_AVALIABLE;
    s32             associated_state;

    jdr_duffcoroutine_t coroutine;

    // Essential tasks will finish no matter what.
    // and cannot be killed normally.
    Game_Task_Userdata userdata;
    bool essential;

    /*
      NOTE:
      the coroutine's userdata will be a yield value,
      this userdata will be the game_state
    */

    // NOTE: I'm using this scheduler for lua code as well.
    lua_State* L_C = nullptr;
    s32        last_L_C_status = 0;
    s32        nargs = 0;
};


struct Game_Task_Scheduler {
    /*
      NOTE:
      To avoid changing code structure, lua coroutines/tasks are also
      here.

      Also for simplicity, since they're only for levels in that case,
      all lua tasks are GAME_TASKS
    */
    Fixed_Array<Game_Task> tasks;
    lua_State* L; // set this pointer to the currently owned lua task.

    // NOTE: will override non-essential tasks.
    s32  add_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential=false);
    s32  add_global_task(jdr_duffcoroutine_fn f);
    s32  add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, bool essential=false);

    /*
      NOTE:
      lua is specifically sandboxed to ONLY work on gameplay scenes.
    */
    s32  add_lua_game_task(struct Game_State* state, lua_State* L, char* fn_name, bool essential=false);
    s32  add_enemy_lua_game_task(struct Game_State* state, lua_State* L, char* fn_name, u64 uid, bool essential=false);
    s32  search_for_lua_task(lua_State* L);

    s32  add_task(struct Game_State* state, jdr_duffcoroutine_fn f, void* userdata, bool essential=false);
    s32  add_global_task(jdr_duffcoroutine_fn f, void* userdata);
    s32  add_ui_task(struct Game_State* state, jdr_duffcoroutine_fn f, void* userdata, bool essential=false);

    bool kill_task(s32 index);

    void scheduler(struct Game_State* state, f32 dt);
    s32  first_avaliable_task();
};

#endif

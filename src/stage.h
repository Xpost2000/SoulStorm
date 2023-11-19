#ifndef STAGE_H
#define STAGE_H

#include "common.h"

// The main goal is to obviously avoid keeping these in code, but until I get
// lua support, there's little reason to avoid doing this imo.
// only boss stages are intended to actually be kept in C++ since they may be more complicated.

// This is the most macro heavy section of the codebase, and I don't personally
// like it, but this is where a lot of the highest level code is anyway...

// STAGE DSL
#define STAGE_TASK_DECLS                                                \
        Game_State*  state = ((Game_Task_Userdata*)(co->userdata))->game_state; \
        Stage_State* stage = &state->gameplay_data.stage_state;         \
        auto         gameplay_state = &state->gameplay_data;            \
        f32          dt = ((Game_Task_Userdata*)(co->userdata))->dt
#define STAGE_TICK(name)                                          \
    void stage_##name##_tick(struct jdr_duffcoroutine* co)
#define STAGE_DRAW(name)                                                \
    void stage_##name##_draw(Stage_State* stage, f32 dt, struct render_commands* commands, Game_State* state)
#define STAGE(name)                                     \
    Stage_State stage_##name##(void)
#define DEF_LEVEL_DATA(name) struct Stage_##name##_Data
#define LEVEL_DATA(name, as) auto as = (struct Stage_##name##_Data*)(stage->stage_memory_buffer)
#define STAGE_WAIT_CLEARED_WAVE() do { while (gameplay_state->any_living_danger()) {TASK_YIELD();} } while(0)
#define WITH_ENTITY(e) for (Enemy_Entity* it = e; it; it = nullptr)
#define WAIT_FOR_SPAWN(id) while (!gameplay_state->entity_spawned(id)) {JDR_Coroutine_YieldNR();}

/*
 * Personally speaking, I think that function pointers are more
 * powerful in this case compared to using Inheritance to specify stage
 * differences.
 *
 * So this is a very C styled design.
 *
 * My current architectural plans is to allow both lua or native driven stages.
 *
 * The main design of a SHMUP / Bullet Hell is more through "Stage Choreography",
 * which is why I intended to do this through a script since I could load the "choreography" on
 * a level start, which means I could change the level inbetween attempts to playtest quickly.
 *
 * For more complex choreographys such as boss-fights, there would be too much time used on
 * doing more specific things for boss-fights, and I would rather program that using a finite state machine
 * within C++. (I am the only programmer, and I've decided this is the best way to go based on development speed.
 * IE: a simple level choreography with a basic mini-boss, would not need many complicated choreography primitives.)
 *
 */

/*
 * NOTE:
 * since most levels are pretty simple in choreography,
 * all the stage specific data is defined here and implemented in one file.
 */

// NOTE: should change to use gameplay data, but just in-case...

struct Stage_State;
struct Gameplay_Data;
struct render_commands;

typedef void (*Stage_Draw_Function)(Stage_State*, f32, struct render_commands*, Game_State*);

// NOTE: the stage state is separate from the Stage information
// which is in stages.h, which I think is fine for now. Not sure if that will change.
struct Stage_State {
    f32 timer;
    s16 phase; // This is for choreography phases in C++.
               // I.E.: Enemy waves.
               // If working in LUA, this will be handled through some coroutines.

    // I don't really know how much memory I actually
    // need, but my memory arena is relatively large so
    // might as well over-estimate...

    /*
     * While I generally like exposing things directly,
     * I would actually like to encapsulate the level code in
     * it's own files since this legitimately makes sense to do,
     *
     * IE: I don't care about looking at level 1 / level 3 code when
     * I'm working on level 2.
     *
     * Also frankly I shouldn't even care about looking at any of this
     * stage code when I'm working on a level.
     *
     * So I'm just going to unsafely cast this into an appropriate stage
     * data structure within the level file implementations.
     */
    char stage_memory_buffer[1];

    // TODO: post process drawing
    /*
      Also, I think I would prefer to keep this as a state machine powered
      behavior, unlike the update which I would more clearly just keep as
      a coroutine style update.

      Although I may toy with making a coroutine way as well.
     */
    Stage_Draw_Function   draw;
    jdr_duffcoroutine_fn tick_task;

    /*
      Technically speaking, the stages are the "lua environment",
      and not necessarily the task scheduler...

      I'm heavily limiting game access from here anyway.
     */
    lua_State*           L = nullptr;
};

Stage_State stage_load_from_lua(Game_State* state, const char* lua_filename);

STAGE(1_1);
STAGE(null);
/* Stage 2 */
/* Stage 3 */
/* Stage 4 */

void stage_draw(Stage_State* stage, f32 dt, struct render_commands* render_commands, Game_State* gameplay_data);
#undef STAGE
#define STAGE(name) stage_##name()
#endif

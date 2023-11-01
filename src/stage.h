#ifndef STAGE_H
#define STAGE_H

#include "common.h"

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
typedef bool (*Stage_Update_Function)(Stage_State*, f32, Game_State*);

// NOTE: the stage state is separate from the Stage information
// which is in stages.h, which I think is fine for now. Not sure if that will change.
struct Stage_State {
    f32 timer; // generic timer in case I don't want anything too specific.
    s16 phase; // This is for choreography phases in C++.
               // I.E.: Enemy waves.
               // If working in LUA, this will be handled through some coroutines.

    union {
        // stage data to be added at later dates.
    };

    // TODO: post process drawing
    Stage_Draw_Function   draw;

    // NOTE: the return value is whether the stage is "finished".
    //       When the stage is complete, it'll give the main loop permission
    //       to show the "stage completion" screen.
    Stage_Update_Function update;
};

Stage_State stage_load_from_lua(const char* lua_filename);

Stage_State stage_null(void); // A stage that finishes instantly.

// The main goal is to obviously avoid keeping these in code, but until I get
// lua support, there's little reason to avoid doing this imo.
// only boss stages are intended to actually be kept in C++ since they may be more complicated.
/* Stage 1 */
Stage_State stage_native_stage_1_1(void);
/* Stage 2 */
/* Stage 3 */
/* Stage 4 */

bool stage_update(Stage_State* stage, f32 dt, Game_State* gameplay_data);
void stage_draw(Stage_State* stage, f32 dt, struct render_commands* render_commands, Game_State* gameplay_data);

#endif

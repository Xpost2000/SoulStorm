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

struct Stage_State;
struct Gameplay_Data;
struct render_commands;

typedef void (*Stage_Draw_Function)(Stage_State*, f32, struct render_commands*, Gameplay_Data*);
typedef bool (*Stage_Update_Function)(Stage_State*, f32, Gameplay_Data*);

// NOTE: the stage state is separate from the Stage information
// which is in stages.h, which I think is fine for now. Not sure if that will change.
struct Stage_State {
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

#endif

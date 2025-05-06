#ifndef STAGES_H
#define STAGES_H

#define MAX_LEVELS_PER_STAGE (3)

struct Level {
    string name;
    string subtitle;
    u8     boss_stage;

    // stages will be associated with a lua script
    // to spawn stuff based off of a "timeline".

    // NOTE: include stage lua script name
    //       if it exists, and use it to load a Lua
    //       Stage_State.

    //       If we don't have one of those, I'll funnel
    //       it to the engine, which will create an appropriate
    //       Stage State per stage, which is mostly for the boss fights
    //       which are basically only in C++.

    s32 last_score;
    s32 best_score;

    s32 attempts;
    s32 completions;
};

struct Stage {
    string name;
    string subtitle;
    s32    unlocked_levels;
    s32    best_score_run;
    // all levels are expected to be filled.
    Level  levels[MAX_LEVELS_PER_STAGE];
};

#endif

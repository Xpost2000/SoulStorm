#ifndef STAGES_H
#define STAGES_H

#define MAX_LEVELS_PER_STAGE (4)

struct Level {
    string name;
    string subtitle;
    u8     boss_stage;

    // stages will be associated with a lua script
    // to spawn stuff based off of a "timeline".
};

struct Stage {
    string name;
    string subtitle;
    s32    unlocked_levels;
    // all levels are expected to be filled.
    Level  levels[MAX_LEVELS_PER_STAGE];
};

#endif

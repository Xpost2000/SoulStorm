#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include "common.h"
#include "stages.h"

enum Save_File_Version {
    SAVE_FILE_VERSION_PRERELEASE0, 
    SAVE_FILE_VERSION_PRERELEASE1, 
    SAVE_FILE_VERSION_PRERELEASE2, 
    SAVE_FILE_VERSION_PRERELEASE3, 
    SAVE_FILE_VERSION_PRERELEASE4, 
    SAVE_FILE_VERSION_COUNT,

    SAVE_FILE_VERSION_CURRENT = SAVE_FILE_VERSION_PRERELEASE4,
};

// NOTE: this is more of a header. Since everything after this is
// going to be achievement data.
// Achievement data is stored per save file.
struct Save_File {
    s32 version = SAVE_FILE_VERSION_CURRENT;

    s32 stage_last_scores[4][MAX_LEVELS_PER_STAGE];
    s32 stage_best_scores[4][MAX_LEVELS_PER_STAGE];
    s32 stage_attempts[4][MAX_LEVELS_PER_STAGE];
    s32 stage_completions[4][MAX_LEVELS_PER_STAGE];
    s32 stage_unlocks[4];
    s32 pets_unlocked;
    s32 first_load = 1; // always true initially
    u8  post_game;
    f32 playtime;
};

#endif

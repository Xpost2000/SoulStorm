#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include "common.h"
#include "stages.h"

enum Save_File_Version {
    SAVE_FILE_VERSION_0, 
    SAVE_FILE_VERSION_COUNT,

    SAVE_FILE_VERSION_CURRENT = SAVE_FILE_VERSION_0,
};

// NOTE: this is more of a header. Since everything after this is
// going to be achievement data.
// Achievement data is stored per save file.
struct Save_File {
    s32 version;
    s32 stage_scores[4][MAX_LEVELS_PER_STAGE];
    s32  stage_unlocks[4];
    u8  post_game;
    f32 playtime;
};

#endif
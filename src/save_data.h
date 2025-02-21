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
    /*
      NOTE(jerry): The game has primarily been using
      this variant of the save format, since this one has
      been effectively finalized.
    */
    SAVE_FILE_VERSION_PRERELEASE5,
    /*
    * Same as previous version, but tracking demo completion flag
    * to possibly(?) award something early if anyone who played the demo
    * decides to play the full game.
    */
    SAVE_FILE_VERSION_DEMO_AND_INITIAL_RELEASE_0_1,
    SAVE_FILE_VERSION_COUNT,

    SAVE_FILE_VERSION_CURRENT = SAVE_FILE_VERSION_DEMO_AND_INITIAL_RELEASE_0_1,
};

const local string save_file_version_strings[] = {
  string_literal("(save-file-version-prerelease0)"),
  string_literal("(save-file-version-prerelease1)"),
  string_literal("(save-file-version-prerelease2)"),
  string_literal("(save-file-version-prerelease3)"),
  string_literal("(save-file-version-prerelease4)"),
  string_literal("(save-file-version-prerelease5)"),
  string_literal("(save-file-version-demo-and-release-0-1)"),
};

// NOTE: this is more of a header. Since everything after this is
// going to be achievement data.
// Achievement data is stored per save file.
struct Save_File_PreRelease5 { // NOTE(jerry): for documentation
  s32 version = SAVE_FILE_VERSION_PRERELEASE5;

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
    u8  beat_demo; // set by demo and TODO(jerry): acknowledged by main executable (when beat_demo=2)
    f32 playtime;
};

#endif

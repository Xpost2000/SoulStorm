#ifndef GAME_PREFERENCES_H
#define GAME_PREFERENCES_H

#include "common.h"
#include "string.h"

// these are mostly engine settings
// things like controls are somewhere else
// (although a bullet hell doesn't really have many inputs...)
struct Game_Preferences {
    // NOTE: these are computed
    //       from the resolution_option_index + graphics_driver.
    s32  width;
    s32  height;

    s32  resolution_option_index;

    f32 music_volume;
    f32 sound_volume;

    bool fullscreen;
    bool controller_vibration;
};

// NOTE: implemented in main.cpp, since it's kind of platform layer code
// with the way I organized everything.
struct Game_Resources;
void confirm_preferences(Game_Preferences* preferences, Game_Resources* resources);
void update_preferences(Game_Preferences* a, Game_Preferences* b);
bool save_preferences_to_disk(Game_Preferences* preferences, string path);
bool load_preferences_from_disk(Game_Preferences* preferences, string path);

#endif

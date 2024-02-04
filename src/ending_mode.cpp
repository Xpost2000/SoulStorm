#include "discord_rich_presence_integration.h"

// included in game.cpp
#define SLIDE_RESOLUTION_W (854)
#define SLIDE_RESOLUTION_H (480)

GAME_SCREEN(update_and_render_game_ending) {
    // For now there's nothing here.
    switch_screen(GAME_SCREEN_MAIN_MENU);
}

#include "game_rules.h"

/*
 * This is something that should really be metaprogrammed, but
 * I don't really want to expend too much effort doing this...
 */

Game_Rules g_game_rules = {};
Game_Rules g_default_game_rules = {
};

Game_Rules serialize_game_rules(struct binary_serializer* serializer)
{
    Game_Rules result = {};
    serialize_s32(serializer, &result.version);
    switch (result.version) {
        case GAME_RULES_VERSION_0: {
            
        } break;
        default: {
            _debugprintf("Unknown game rules structure version. Using default game rules.");
            result = g_default_game_rules;
        } break;
    }
    return result;
}

void reset_game_rules(void)
{
    g_game_rules = g_default_game_rules;
}

#include "game_state.h"
#include "stage.h"

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#undef STAGE
// using X-macros
#define STAGE(name)                                     \
    Stage_State stage_##name##(void) {                  \
        Stage_State result = {};                        \
        {                                               \
            zero_memory(&result, sizeof(result));       \
            result.draw   = stage_##name##_draw;        \
            result.tick_task = stage_##name##_tick;     \
        }                                               \
        return result;                                  \
    }

Stage_State stage_load_from_lua(Game_State* state, const char* lua_filename) {
    Stage_State result = {};
    result.L = state->alloc_lua_bindings();
    s32 error = (luaL_dofile(result.L, lua_filename));
    switch (error) {
        case 0: { } break;
        default: {
            _debugprintf("Error in loading script. (%d)[ %s ]", error, lua_tostring(result.L, -1));
        } break;
    }
    

    if (error) {
        if (result.L) {
            lua_close(result.L);
            result.L = nullptr;
        }
        _debugprintf("Loading NULL stage as fallback!");
        return stage_null();
    } else {
        state->coroutine_tasks.add_lua_game_task(state, result.L, (char*)"stage_task");
    }

    return result;
}

void stage_draw(Stage_State* stage, f32 dt, struct render_commands* render_commands, Game_State* state) {
    if (stage->draw) {
        stage->draw(stage, dt, render_commands, state);
    }
}

// NOTE: do not compile the stages as translation units.
// include any new stages here.
// and update the stage_list.h as appropriate
#include "stages/null.cpp"
#include "stages/1_1.cpp"

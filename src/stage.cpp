#include "game_state.h"
#include "stage.h"
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

Stage_State stage_load_from_lua(const char* lua_filename) {
    Stage_State result = {};
    // NOTE: stages are going to be scheduled through the game_task_scheduler.
    // so they will all have a generic tick task
    unimplemented("No lua stages yet.");
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

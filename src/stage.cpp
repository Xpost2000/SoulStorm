#include "game_state.h"
#include "stage.h"

/*
  TODO: I need a nice way to describe the scenes in C++, and a minimal overhead method.
        so I'm expecting something similar to duff's device because there's not really many other
        elegant methods I can think of.

        I can either describe the entire level spawning as data but that's a pain to parse.
 */

/* NULL STAGE */
void stage_null_draw(Stage_State* stage, f32 dt, struct render_commands* commands, Game_State* state) {
    // _debugprintf("Null stage draw");
    return;
}

bool stage_null_update(Stage_State* stage, f32 dt, Game_State* state) {
    // _debugprintf("Null stage update");
    stage->timer += dt;
    if (stage->timer > 1.5f) return true;
    return false;
}

Stage_State stage_null(void) {
    Stage_State result = {};
    {
        result.draw   = stage_null_draw;
        result.update = stage_null_update;
        result.timer  = 0;
    }
    return result;
}
/* END OF NULL STAGE */

/* STAGE 1-1 NATIVE */

void stage_native_stage_1_1_draw(Stage_State* stage, f32 dt, struct render_commands* commands, Game_State* state) {
    return;
}

/*
  likely just going to use duff's device,
  since longjmp and setjmp are creepin me out.
*/
bool stage_native_stage_1_1_update(Stage_State* stage, f32 dt, Game_State* state) {
    auto gameplay_state = &state->gameplay_data;

    switch (stage->phase) {
        case 0: { // wave 1
            for (int i = 0; i < 6; ++i) 
                // spawn_enemy_linear_movement_with_circling_down_attack(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 150);
                spawn_enemy_linear_movement(state, V2(i * 35, -30), V2(10, 10), V2(0, 1), 150);

            
            stage->phase++;
            stage->timer = 0;
        } break;
        case 1: {
            if (!gameplay_state->any_living_danger()) {
                stage->phase++;
            }
        } break;
        default:
        case 2: {
            return true;
        } break;
    }
    return false;
}

Stage_State stage_native_stage_1_1() {
    Stage_State result = {};
    _debugprintf("native stage 1-1");
    {
        result.draw   = stage_native_stage_1_1_draw;
        result.update = stage_native_stage_1_1_update;
        result.timer  = 0;
        result.phase  = 0;
    }
    return result;
}
/* END OF STAGE 1-1 NATIVE */

Stage_State stage_load_from_lua(const char* lua_filename) {
    Stage_State result = {};
    unimplemented("No lua stages yet.");
    return result;
}

bool stage_update(Stage_State* stage, f32 dt, Game_State* state) {
    bool result = true;
    if (stage->update) {
        result = stage->update(stage, dt, state);
    }

    stage->timer += dt;
    return result;
}

void stage_draw(Stage_State* stage, f32 dt, struct render_commands* render_commands, Game_State* state) {
    if (stage->draw) {
        stage->draw(stage, dt, render_commands, state);
    }
}

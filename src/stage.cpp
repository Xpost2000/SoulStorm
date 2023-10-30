#include "stage.h"

void stage_null_draw(Stage_State* stage, f32 dt, struct render_commands* commands, Gameplay_Data* gameplay_state) {
    _debugprintf("Null stage draw");
    return;
}

bool stage_null_update(Stage_State* stage, f32 dt, Gameplay_Data* gameplay_state) {
    return true;
}

Stage_State stage_load_from_lua(const char* lua_filename) {
    Stage_State result = {};
    unimplemented("No lua stages yet.");
    return result;
}

Stage_State stage_null(void) {
    Stage_State result = {};
    {
        result.draw = stage_null_draw;
        result.update = stage_null_update;
    }
    return result;
}

GAME_LUA_MODULE(
gameplay_dialogue,
"src/dialogue_lua_bindings_generated.cpp",
"Bindings to manipulate the dialogue system.",
"This provides complete access to any dialogue functionality in the engine.\
It is intended to be as declarative as possible, and allows you to animate actors in\
a similar style to basic visual novels.\
\
\
There is no planned support for choices / modal dialogue. Please note that most/all of the procedures\
in this API are yielding coroutines! Additionally, only two speakers are supported."
)


GAME_LUA_PROC(
dialogue_start,
"",
"Begin dialogue mode.",
"Start dialogue mode and pause gameplay, will open a textbox. **None** of the dialogue procedures will work without this."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    dialogue_state.in_conversation = true;
    dialogue_state.confirm_continue = false;
    dialogue_state.shown_characters = 0;
    dialogue_state.length = 0;
    dialogue_state.phase = DIALOGUE_UI_ANIMATION_PHASE_INTRODUCTION;
    dialogue_state.type_timer = 0;
    dialogue_state.box_open_close_timer = 0;
    dialogue_state.tracked_image_count = 0;

    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");
    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_DIALOGUE_TO_FINISH_INTRODUCTION;
    return lua_yield(L, 0);
}

GAME_LUA_PROC(
dialogue_end,
"",
"End dialogue mode.",
"End dialogue mode and resume gameplay, will close textbox. **NOTE**: Actors must be manually terminated or animated away.\
if you do not do this, actors will abruptly disappear!"
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_DIALOGUE_FINISH;

    dialogue_state.phase = DIALOGUE_UI_ANIMATION_PHASE_BYE;
    {
        dialogue_state.type_timer = 0;
        dialogue_state.bye_optimal_untype_time_max =
            (DIALOGUE_BOX_EXPANSION_MAX_TIME * 0.10) / // NOTE: technically should be more advanced formula,
                                                      // the boxes disappear by tiles so technically it looks "faster"
            string_from_cstring(dialogue_state.current_line).length;
        _debugprintf("Optimal untype time max: %3.3f", dialogue_state.bye_optimal_untype_time_max);
    }

    return lua_yield(L, 0);
}

// int _lua_bind_dialogue_say_line(lua_State* L)
GAME_LUA_PROC(
dialogue_say_line,
"text: string",
"Start speaking a line of dialogue.",
"Start speaking a line of dialogue, by slowly typing it out."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_DIALOGUE_CONTINUE;
    char* line_string = (char*)lua_tostring(L, 1);
    // do not modify this
    // dialogue_state.shown_characters = 0;
    dialogue_state.type_timer = 0;
    dialogue_state.speaking_lines_of_dialogue = true;
    dialogue_state.phase = DIALOGUE_UI_ANIMATION_PHASE_UNWRITE_TEXT;
    cstring_copy(line_string, dialogue_state.next_line, sizeof(dialogue_state.next_line));
    return lua_yield(L, 0);
}

GAME_LUA_PROC(
dialogue_speaker_set_visibility,
"speaker: number [0 or 1], visibility: boolean",
"Toggle the visibility of a speaker.",
"Toggle the visibility of a speaker."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    bool        visibility = lua_toboolean(L, 2);
    dialogue_state.speakers[speaker_id].visible = visibility;
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_set_image_scale,
"speaker: number [0 or 1], scale_x: number, scale_y: number",
"Set the image scale of a speaker.",
"Set the image scale of a speaker."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    dialogue_state.speakers[speaker_id].image_scale =
        V2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_set_image,
"speaker: number [0 or 1], image_path: string",
"Set dialogue speaker image.",
"Load an image, and use it for the specified speaker. Will crash if the image does not exist!"
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);

    auto img_id = graphics_assets_load_image(
        &state->resources->graphics_assets,
        string_from_cstring((char*)lua_tostring(L, 2))
    );

    bool found = false;
    for (s32 tracked_image_index = 0;
        tracked_image_index < dialogue_state.tracked_image_count;
        ++tracked_image_index) {
        if (img_id.index == dialogue_state.tracked_images[tracked_image_index].index) {
            found = true;
            break;
        }
    }

    if (!found) {
        // NOTE: watch out for overflow.
        dialogue_state.tracked_images[dialogue_state.tracked_image_count++] = img_id;
    }

    dialogue_state.speakers[speaker_id].image = img_id;

    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_set_mirrored,
"speaker: number [0 or 1], mirror: boolean",
"Mirror the dialogue speaker when rendering.",
"Mirror the dialogue speaker when rendering."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    bool        mirror = lua_toboolean(L, 2);
    dialogue_state.speakers[speaker_id].mirrored = mirror;
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_set_position_offset,
"speaker: number [0 or 1], x: number, y: number",
"Set position offset for dialogue speaker sprite.",
"Set position offset for dialogue speaker sprite, this is in absolute pixel units."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    dialogue_state.speakers[speaker_id].offset_position =
        V2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_animation_fade_out,
"speaker: number [0 or 1], duration: number",
"Begin a fade out animation for the dialogue speaker.",
"Begin a fade out animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FADE_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_animation_fade_in,
"speaker: number [0 or 1], duration: number",
"Begin a fade in animation for the dialogue speaker.",
"Begin a fade in animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FADE_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;

    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_animation_slide_fade_out,
"speaker: number [0 or 1], duration: number",
"Begin a slide fade out animation for the dialogue speaker.",
"Begin a slide fade out animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
GAME_LUA_PROC(
dialogue_speaker_animation_slide_fade_in,
"speaker: number [0 or 1], duration: number",
"Begin a slide fade in animation for the dialogue speaker.",
"Begin a slide fade in animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;

    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_animation_jump,
"speaker: number [0 or 1], count: number, intensity: number, duration: number",
"Begin a jump animation for the dialogue speaker.",
"Begin a jump animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    int         times = luaL_checkinteger(L, 2);
    int         intensity = luaL_checkinteger(L, 3);
    float       max_t = luaL_checknumber(L, 4);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_JUMP;
    dialogue_state.speakers[speaker_id].animation.times = times;
    dialogue_state.speakers[speaker_id].animation.param0 = intensity;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}

GAME_LUA_PROC(
dialogue_speaker_animation_shake,
"speaker: number [0 or 1], count: number, intensity: number, duration: number",
"Begin a shaking/jitter animation for the dialogue speaker.",
"Begin a shaking/jitter animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    int         times = luaL_checkinteger(L, 2);
    int         intensity = luaL_checkinteger(L, 3);
    float       max_t = luaL_checknumber(L, 4);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_SHAKE;
    dialogue_state.speakers[speaker_id].animation.times = times;
    dialogue_state.speakers[speaker_id].animation.param0 = intensity;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
GAME_LUA_PROC(
dialogue_speaker_animation_focus_out,
"speaker: number [0 or 1], duration: number",
"Begin a focus out animation for the dialogue speaker.",
"Begin a focus out animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    float       max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FOCUS_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
GAME_LUA_PROC(
dialogue_speaker_animation_focus_in,
"speaker: number [0 or 1], duration: number",
"Begin a focus in animation for the dialogue speaker.",
"Begin a focus in animation for the dialogue speaker specified for duration."
)
{
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    float       max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FOCUS_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}

#include "dialogue_lua_bindings_generated.cpp"

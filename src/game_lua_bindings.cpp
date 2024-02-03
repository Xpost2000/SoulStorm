#include "game.h"
#include "entity.h"
#include "game_state.h"
#include "game_uid_generator.h"
#include "virtual_file_system.h"

// Lua bindings
int _lua_bind_Task_Yield_Wait(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");
    f32         wait_time = luaL_checknumber(L, 1);

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_SECONDS;
    task.userdata.yielded.timer = 0.0f;
    task.userdata.yielded.timer_max = wait_time;

    // _debugprintf("LuaThread(%p) wants to wait for %f seconds", L, wait_time);
    // _debugprintf("LuaThread(%p)(taskid: %d) wants to wait for %f", L, task_id, wait_time);

    return lua_yield(L, 0);
}

int _lua_bind_Task_Yield_Finish_Stage(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_COMPLETE_STAGE;

    return lua_yield(L, 0);
}

int _lua_bind_Task_Yield_Until_No_Danger(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_NO_DANGER_ON_STAGE;

    return lua_yield(L, 0);
}

int _lua_bind_Task_Yield_Wait_For_Stage_Intro(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    auto& task = state->coroutine_tasks.tasks[task_id];
    task.userdata.yielded.reason = TASK_YIELD_REASON_WAIT_FOR_INTRODUCTION_SEQUENCE_TO_COMPLETE;

    return lua_yield(L, 0);
}

int _lua_bind_Task_Yield(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 task_id = state->coroutine_tasks.search_for_lua_task(L);
    assertion(task_id != -1 && "Impossible? Or you're not using this from a task!");

    return lua_yield(L, 0);
}


int _lua_bind_any_living_danger(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushboolean(L, state->gameplay_data.any_living_danger());
    return 1;
}

int _lua_bind_prng_ranged_float(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    f32 a = luaL_checknumber(L, 1);
    f32 b = luaL_checknumber(L, 2);
    lua_pushnumber(L, random_ranged_float(&state->gameplay_data.prng, a, b));
    return 1;
}

int _lua_bind_prng_ranged_integer(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 a = luaL_checkinteger(L, 1);
    s32 b = luaL_checkinteger(L, 2);
    lua_pushnumber(L, random_ranged_integer(&state->gameplay_data.prng, a, b));
    return 1;
}

int _lua_bind_prng_normalized_float(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushnumber(L, random_float(&state->gameplay_data.prng));
    return 1;
}

int _lua_bind_play_area_width(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushinteger(L, state->gameplay_data.play_area.width);
    return 1;
}

int _lua_bind_play_area_height(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushinteger(L, state->gameplay_data.play_area.height);
    return 1;
}

int _lua_bind_camera_traumatize(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    camera_traumatize(&state->gameplay_data.main_camera, luaL_checknumber(L, 1));
    return 0;
}

int _lua_bind_camera_set_trauma(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    camera_set_trauma(&state->gameplay_data.main_camera, luaL_checknumber(L, 1));
    return 0;
}

int _lua_bind_async_task(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    char* task_name = (char*)lua_tostring(L, 1);
    lua_remove(L, 1);
    _debugprintf("Assign async task %s", task_name);

    s32 remaining = lua_gettop(L);
    _debugprintf("Async task with %d elements [%d actual stack top]", remaining, lua_gettop(L));

    state->coroutine_tasks.add_lua_game_task(
        state,
        L,
        state->coroutine_tasks.L,
        task_name
    );
    return 0;
}

int _lua_bind_create_render_object(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.add_scriptable_render_object(Scriptable_Render_Object{});
    lua_pushinteger(L, state->gameplay_data.scriptable_render_objects.size - 1);
    return 1;
}

int _lua_bind_render_object_set_layer(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.layer = luaL_checkinteger(L, 2);
    return 0;
}

int _lua_bind_render_object_set_position(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.position.x = luaL_checknumber(L, 2);
    scriptable_render_object.position.y = luaL_checknumber(L, 3);
    return 0;
}

int _lua_bind_render_object_set_src_rect(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.src_rect.x = luaL_checknumber(L, 2);
    scriptable_render_object.src_rect.y = luaL_checknumber(L, 3);
    scriptable_render_object.src_rect.w = luaL_checknumber(L, 4);
    scriptable_render_object.src_rect.h = luaL_checknumber(L, 5);
    return 0;
}

int _lua_bind_render_object_set_img_id(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.image_id.index = luaL_checkinteger(L, 2);
    return 0;
}

int _lua_bind_render_object_set_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.scale.x = luaL_checknumber(L, 2);
    scriptable_render_object.scale.y = luaL_checknumber(L, 3);
    return 0;
}

int _lua_bind_render_object_set_modulation(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.modulation.r = luaL_checknumber(L, 2) * 255;
    scriptable_render_object.modulation.g = luaL_checknumber(L, 3) * 255;
    scriptable_render_object.modulation.b = luaL_checknumber(L, 4) * 255;
    scriptable_render_object.modulation.a = luaL_checknumber(L, 5) * 255;
    return 0;
}

int _lua_bind_render_object_set_x_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.x_angle = luaL_checkinteger(L, 2);
    return 0;
}

int _lua_bind_render_object_set_y_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.y_angle = luaL_checkinteger(L, 2);
    return 0;
}

int _lua_bind_render_object_set_z_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    scriptable_render_object.z_angle = luaL_checkinteger(L, 2);
    return 0;
}

int _lua_bind_render_object_get_layer(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushinteger(L, scriptable_render_object.layer);
    return 1;
}

int _lua_bind_render_object_get_position_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.position.x);
    return 1;
}

int _lua_bind_render_object_get_position_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.position.y);
    return 1;
}

int _lua_bind_render_object_get_src_rect_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.src_rect.x);
    return 1;
}

int _lua_bind_render_object_get_src_rect_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.src_rect.y);
    return 1;
}

int _lua_bind_render_object_get_src_rect_w(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.src_rect.w);
    return 1;
}

int _lua_bind_render_object_get_src_rect_h(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.src_rect.h);
    return 1;
}

int _lua_bind_render_object_get_scale_x(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.scale.x);
    return 1;
}

int _lua_bind_render_object_get_scale_y(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.scale.y);
    return 1;
}

int _lua_bind_render_object_get_modulation_r(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.modulation.r / 255.0f);
    return 1;
}

int _lua_bind_render_object_get_modulation_g(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.modulation.g / 255.0f);
    return 1;
}

int _lua_bind_render_object_get_modulation_b(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.modulation.b / 255.0f);
    return 1;
}

int _lua_bind_render_object_get_modulation_a(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushnumber(L, scriptable_render_object.modulation.a / 255.0f);
    return 1;
}

int _lua_bind_render_object_get_x_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushinteger(L, scriptable_render_object.x_angle);
    return 1;
}

int _lua_bind_render_object_get_y_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushinteger(L, scriptable_render_object.y_angle);
    return 1;
}

int _lua_bind_render_object_get_z_angle(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& scriptable_render_object = state->gameplay_data.scriptable_render_objects[luaL_checkinteger(L, 1)];
    lua_pushinteger(L, scriptable_render_object.z_angle);
    return 1;
}

int _lua_bind_global_elapsed_time(lua_State* L) {
    auto engine = Global_Engine();
    lua_pushnumber(L, engine->global_elapsed_time);
    return 1;
}

int _lua_bind_ticktime(lua_State* L) {
    lua_pushnumber(L, FIXED_TICKTIME);
    return 1;
}

int _lua_bind_load_image(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;

    auto img_id = graphics_assets_load_image(
        &resources->graphics_assets,
        string_from_cstring((char*)lua_tostring(L, 1))
    );

    state->gameplay_data.script_loaded_images.push(img_id);

    lua_pushinteger(L, img_id.index);
    return 1;
}

int _lua_bind_load_sound(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    const char* filepath = lua_tostring(L, 1);
    auto sound_id = Audio::load(filepath, false);

    lua_pushinteger(L, sound_id.index);
    state->gameplay_data.script_loaded_sounds.push(sound_id);
    return 1;
}

int _lua_bind_load_music(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    const char* filepath = lua_tostring(L, 1);
    auto sound_id = Audio::load(filepath, true);

    lua_pushinteger(L, sound_id.index);
    state->gameplay_data.script_loaded_sounds.push(sound_id);
    return 1;
}

int _lua_bind_random_attack_sound(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    auto        result = resources->random_attack_sound(&state->gameplay_data.prng);
    lua_pushinteger(L, result.index);
    return 1;
}

int _lua_bind_play_area_edge_behavior(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& play_area = state->gameplay_data.play_area;
    int edge_id = luaL_checkinteger(L, 1);
    int edge_flag = luaL_checkinteger(L, 2);

    if (edge_id < 0) edge_id = 0;
    if (edge_id >= 3) edge_id = 3;

    play_area.edge_behaviors[edge_id] = edge_flag;
    return 0;
}

int _lua_bind_convert_all_bullets_to_score(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    float radius = 999999;

    if (lua_gettop(L) == 1) {
        radius = luaL_checknumber(L, 1);
    }

    state->convert_bullets_to_score_pickups(radius);
    return 0;
}

int _lua_bind_convert_all_enemies_to_score(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    float radius = 999999;

    if (lua_gettop(L) == 1) {
        radius = luaL_checknumber(L, 1);
    }

    state->convert_enemies_to_score_pickups(radius);
    return 0;
}

int _lua_bind_kill_all_bullets(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->kill_all_bullets();
    return 0;
}
int _lua_bind_kill_all_enemies(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->kill_all_enemies();
    return 0;
}

int _lua_bind_dialogue_start(lua_State* L) {
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

int _lua_bind_dialogue_end(lua_State* L) {
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

int _lua_bind_dialogue_say_line(lua_State* L) {
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

int _lua_bind_dialogue_speaker_set_visibility(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    bool        visibility = lua_toboolean(L, 2);
    dialogue_state.speakers[speaker_id].visible = visibility;
    return 0;
}

int _lua_bind_dialogue_speaker_set_image_scale(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    dialogue_state.speakers[speaker_id].image_scale =
        V2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int _lua_bind_dialogue_speaker_set_image(lua_State* L) {
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

int _lua_bind_dialogue_speaker_set_mirrored(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    bool        mirror = lua_toboolean(L, 2);
    dialogue_state.speakers[speaker_id].mirrored = mirror;
    return 0;
}

int _lua_bind_dialogue_speaker_set_position_offset(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    dialogue_state.speakers[speaker_id].offset_position =
        V2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int _lua_bind_dialogue_speaker_animation_fade_out(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FADE_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
int _lua_bind_dialogue_speaker_animation_fade_in(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FADE_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;

    return 0;
}

int _lua_bind_dialogue_speaker_animation_slide_fade_out(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
int _lua_bind_dialogue_speaker_animation_slide_fade_in(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    f32         max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_SLIDE_FADE_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;

    return 0;
}

int _lua_bind_dialogue_speaker_animation_jump(lua_State* L) {
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

int _lua_bind_dialogue_speaker_animation_shake(lua_State* L) {
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
int _lua_bind_dialogue_speaker_animation_focus_out(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    float       max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FOCUS_OUT;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}
int _lua_bind_dialogue_speaker_animation_focus_in(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto& dialogue_state = state->dialogue_state;
    int         speaker_id = luaL_checkinteger(L, 1);
    float       max_t = luaL_checknumber(L, 2);

    dialogue_state.speakers[speaker_id].animation.type = DIALOGUE_SPEAKER_ANIMATION_FOCUS_IN;
    dialogue_state.speakers[speaker_id].animation.t = 0.0f;
    dialogue_state.speakers[speaker_id].animation.max_t = max_t;
    return 0;
}

int _lua_bind_game_difficulty_binding(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto pet_data = game_get_pet_data(state->gameplay_data.selected_pet);
    lua_pushinteger(L, pet_data->difficulty_modifier);
    return 1;
}

lua_State* Game_State::alloc_lua_bindings() {
    lua_State* L = luaL_newstate();
    /*
      NOTE: the game uses coroutines to implement tasks which
      is a useful, albeit quite atypical usecase for coroutines in games.

      However, when every bullet needs to be intelligent and you don't want to write
      a state machine... Well... Yeah.
     */
    lua_checkstack(L, 512);
    luaL_openlibs(L);

    {
        lua_pushlightuserdata(L, this);
        lua_setglobal(L, "_gamestate"); // we'll store this implicitly

        lua_register(L, "t_wait", _lua_bind_Task_Yield_Wait);

        // TODO:
        // 1_1 is designed with all these timings being "absolute", and gradually
        // changing to "safe" waits is pretty hard.
        // although I'll add scaffolding code for it.
        lua_register(L, "t_fwait", _lua_bind_Task_Yield_Wait);

        lua_register(L, "t_yield", _lua_bind_Task_Yield);
        lua_register(L, "t_wait_for_no_danger", _lua_bind_Task_Yield_Until_No_Danger);
#if 0
        // Do not use this. It causes desync galore.
        // I'll just deal with the mandatory screen intrusion
        lua_register(L, "t_wait_for_stage_intro", _lua_bind_Task_Yield_Wait_For_Stage_Intro);
#endif
        lua_register(L, "t_complete_stage", _lua_bind_Task_Yield_Finish_Stage);

        lua_register(L, "any_living_danger", _lua_bind_any_living_danger);
        lua_register(L, "play_area_width", _lua_bind_play_area_width);
        lua_register(L, "play_area_height", _lua_bind_play_area_height);

        lua_register(L, "prng_ranged_float", _lua_bind_prng_ranged_float);
        lua_register(L, "prng_ranged_integer", _lua_bind_prng_ranged_integer);
        lua_register(L, "prng_normalized_float", _lua_bind_prng_normalized_float);

        lua_register(L, "camera_traumatize", _lua_bind_camera_traumatize);
        lua_register(L, "camera_set_trauma", _lua_bind_camera_set_trauma);

        lua_register(L, "play_area_set_edge_behavior", _lua_bind_play_area_edge_behavior);
    }

    /*
     * NOTE:
     *
     * While I was originally planning to hardcode the stage backgrounds in C++,
     * I don't really want to spend too much time in using my low-level APIs to build
     * stuff.
     *
     * Also, I do not have a hotloading solution so this is literally the next best thing.
     *
     */
    {
        // Some really basic engine bindings.
        lua_register(L, "load_image", _lua_bind_load_image);
        lua_register(L, "async_task", _lua_bind_async_task);
        lua_register(L, "random_attack_sound", _lua_bind_random_attack_sound);

        lua_register(L, "dialogue_start", _lua_bind_dialogue_start);
        lua_register(L, "dialogue_end", _lua_bind_dialogue_end);
        lua_register(L, "dialogue_say_line", _lua_bind_dialogue_say_line);
        lua_register(L, "dialogue_speaker_set_visibility", _lua_bind_dialogue_speaker_set_visibility);
        lua_register(L, "dialogue_speaker_set_image", _lua_bind_dialogue_speaker_set_image);
        lua_register(L, "dialogue_speaker_set_image_scale", _lua_bind_dialogue_speaker_set_image_scale);
        lua_register(L, "dialogue_speaker_set_mirrored", _lua_bind_dialogue_speaker_set_mirrored);
        lua_register(L, "dialogue_speaker_set_position_offset", _lua_bind_dialogue_speaker_set_position_offset);

        lua_register(L, "dialogue_speaker_animation_fade_out", _lua_bind_dialogue_speaker_animation_fade_out);
        lua_register(L, "dialogue_speaker_animation_fade_in", _lua_bind_dialogue_speaker_animation_fade_in);

        lua_register(L, "dialogue_speaker_animation_focus_out", _lua_bind_dialogue_speaker_animation_focus_out);
        lua_register(L, "dialogue_speaker_animation_focus_in", _lua_bind_dialogue_speaker_animation_focus_in);

        lua_register(L, "dialogue_speaker_animation_slide_fade_out", _lua_bind_dialogue_speaker_animation_slide_fade_out);
        lua_register(L, "dialogue_speaker_animation_slide_fade_in", _lua_bind_dialogue_speaker_animation_slide_fade_in);

        lua_register(L, "dialogue_speaker_animation_jump", _lua_bind_dialogue_speaker_animation_jump);
        lua_register(L, "dialogue_speaker_animation_shake", _lua_bind_dialogue_speaker_animation_shake);
#if 1
        /* These are per frame objects */
        /*
         * NOTE: I'm not a fan of doing per scalar things
         *       but I also don't think it's a big deal as well...
         */
        lua_register(L, "render_object_create", _lua_bind_create_render_object);
        lua_register(L, "render_object_set_layer", _lua_bind_render_object_set_layer);
        lua_register(L, "render_object_set_position", _lua_bind_render_object_set_position);
        lua_register(L, "render_object_set_src_rect", _lua_bind_render_object_set_src_rect);
        lua_register(L, "render_object_set_img_id", _lua_bind_render_object_set_img_id);
        lua_register(L, "render_object_set_scale", _lua_bind_render_object_set_scale);
        lua_register(L, "render_object_set_modulation", _lua_bind_render_object_set_modulation);
        lua_register(L, "render_object_set_x_angle", _lua_bind_render_object_set_x_angle);
        lua_register(L, "render_object_set_y_angle", _lua_bind_render_object_set_y_angle);
        lua_register(L, "render_object_set_z_angle", _lua_bind_render_object_set_z_angle);

        lua_register(L, "render_object_get_layer", _lua_bind_render_object_get_layer);
        lua_register(L, "render_object_get_position_x", _lua_bind_render_object_get_position_x);
        lua_register(L, "render_object_get_position_y", _lua_bind_render_object_get_position_y);
        lua_register(L, "render_object_get_src_rect_x", _lua_bind_render_object_get_src_rect_x);
        lua_register(L, "render_object_get_src_rect_y", _lua_bind_render_object_get_src_rect_y);
        lua_register(L, "render_object_get_src_rect_w", _lua_bind_render_object_get_src_rect_w);
        lua_register(L, "render_object_get_src_rect_h", _lua_bind_render_object_get_src_rect_h);
        lua_register(L, "render_object_get_scale_x", _lua_bind_render_object_get_scale_x);
        lua_register(L, "render_object_get_scale_y", _lua_bind_render_object_get_scale_y);
        lua_register(L, "render_object_get_modulation_r", _lua_bind_render_object_get_modulation_r);
        lua_register(L, "render_object_get_modulation_g", _lua_bind_render_object_get_modulation_g);
        lua_register(L, "render_object_get_modulation_b", _lua_bind_render_object_get_modulation_b);
        lua_register(L, "render_object_get_modulation_a", _lua_bind_render_object_get_modulation_a);
        lua_register(L, "render_object_get_x_angle", _lua_bind_render_object_get_x_angle);
        lua_register(L, "render_object_get_y_angle", _lua_bind_render_object_get_y_angle);
        lua_register(L, "render_object_get_z_angle", _lua_bind_render_object_get_z_angle);

        lua_register(L, "global_elapsed_time", _lua_bind_global_elapsed_time);
        lua_register(L, "ticktime", _lua_bind_ticktime);

        // NOTE: these should consider "boss filter" enemies
        // but I need ways to adjust entity flags
        lua_register(L, "convert_all_bullets_to_score", _lua_bind_convert_all_bullets_to_score);
        lua_register(L, "convert_all_enemies_to_score", _lua_bind_convert_all_enemies_to_score);
        lua_register(L, "kill_all_bullets", _lua_bind_kill_all_bullets);
        lua_register(L, "kill_all_enemies", _lua_bind_kill_all_enemies);
#endif
    }
    {lua_register(L, "get_difficulty_modifier", _lua_bind_game_difficulty_binding);}
    {bind_v2_lualib(L); }
    {bind_entity_lualib(L); }
    {bind_vfs_lualib(L);}
    {bind_particle_emitter_lualib(L);}
    {
        Audio::bind_audio_lualib(L);
        lua_register(L, "load_sound", _lua_bind_load_sound);
        lua_register(L, "load_music", _lua_bind_load_music);
    }

    // Register _threadtable
    {
        lua_newtable(L);
        lua_setglobal(L, "_coroutinetable");
    }


    _debugprintf("Allocated new lua state.");
    return L;
}

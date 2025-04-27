#include "game.h"
#include "entity.h"
#include "game_state.h"
#include "game_uid_generator.h"
#include "virtual_file_system.h"

#include "lua_binding_macro.h"

#include "core_lua_bindings.cpp"
#include "dialogue_lua_bindings.cpp"

// Lua bindings
int _lua_bind_any_living_danger(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    lua_pushboolean(L, state->gameplay_data.any_living_danger());
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

int _lua_bind_destroy_render_object(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    s32 scriptable_object_index =  luaL_checkinteger(L, 1);
    state->gameplay_data.scriptable_render_objects.pop_and_swap(scriptable_object_index);
    return 0;
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

int _lua_bind_load_image(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    char* filepath = (char*) lua_tostring(L, 1);
    auto img_id = state->gameplay_data.script_load_image(resources, filepath);
    lua_pushinteger(L, img_id.index);
    return 1;
}

int _lua_bind_load_sound(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    char* filepath = (char*) lua_tostring(L, 1);
    auto sound_id = state->gameplay_data.script_load_sound(resources, filepath);
    lua_pushinteger(L, sound_id.index);
    return 1;
}

int _lua_bind_load_music(lua_State* L) {
    lua_getglobal(L, "_gamestate");
    Game_State* state = (Game_State*)lua_touserdata(L, lua_gettop(L));
    auto        resources = state->resources;
    char* filepath = (char*) lua_tostring(L, 1);
    auto sound_id = state->gameplay_data.script_load_sound(resources, filepath, true);
    lua_pushinteger(L, sound_id.index);
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
    if (state->gameplay_data.allow_border_switch_flashing) {
        state->gameplay_data.border_notify(edge_id, edge_flag+1, BORDER_NOTIFY_FLASH_COUNT, true);
    }
    return 0;
}

int _lua_bind_play_area_border_notify(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    int edge_id = luaL_checkinteger(L, 1);
    int edge_flag = luaL_checkinteger(L, 2);

    if (edge_id < 0) edge_id = 0;
    if (edge_id >= 3) edge_id = 3;

    state->gameplay_data.border_notify(edge_id, edge_flag);
    return 0;
}

int _lua_bind_play_area_stop_all_border_notifications(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.border_stop_all_notifications();
    return 0;
}

int _lua_bind_play_area_stop_border_notify(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    int edge_id = luaL_checkinteger(L, 1);

    if (edge_id < 0) edge_id = 0;
    if (edge_id >= 3) edge_id = 3;

    state->gameplay_data.border_stop_notify(edge_id);
    return 0;
}

int _lua_bind_play_area_set_allow_border_switch_flashing(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    bool v = luaL_checkinteger(L, 1);
    state->gameplay_data.allow_border_switch_flashing = v;
    return 0;
}

int _lua_bind_play_area_notify_current_border_status(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.border_notify_current_status();
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

int _lua_bind_disable_grazing(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_grazing = true;
    return 0;
}

int _lua_bind_enable_grazing(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_grazing = false;
    return 0;
}

int _lua_bind_disable_bullet_to_points(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_bullet_to_points = true;
    return 0;
}

int _lua_bind_disable_enemy_to_points(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_enemy_to_points = true;
    return 0;
}

int _lua_bind_enable_bullet_to_points(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_bullet_to_points = false;
    return 0;
}

int _lua_bind_enable_enemy_to_points(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    state->gameplay_data.disable_enemy_to_points = false;
    return 0;
}

int _lua_bind_game_difficulty_binding(lua_State* L) {
    Game_State* state = lua_binding_get_gamestate(L);
    auto pet_data = game_get_pet_data(state->gameplay_data.selected_pet);
    lua_pushinteger(L, pet_data->difficulty_modifier);
    return 1;
}

int _lua_bind_show_damage_player_will_take(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  state->gameplay_data.show_damage_player_will_take = true;
  return 0;
}

int _lua_bind_hide_damage_player_will_take(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  state->gameplay_data.show_damage_player_will_take = false;
  return 0;
}

int _lua_bind_award_points(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  state->gameplay_data.notify_score(luaL_checkinteger(L, 1), true);
  return 0;
}

int _lua_bind_show_game_alert(lua_State* L) {
  Game_State* state = lua_binding_get_gamestate(L);
  char* cstring = (char*) lua_tostring(L, 1);
  int font_id = luaL_checkinteger(L, 2);
  float duration = luaL_checknumber(L, 3);
  auto& alert = state->gameplay_data.game_alert;
  alert.enabled = true;
  alert.timer = duration;
  alert.timer_max = duration;
  alert.font_variation = font_id;
  // todo check safety?
  strncpy(alert.text, cstring, sizeof(alert.text));
  return 0;
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

        bind_core_lualib(L);

        lua_register(L, "any_living_danger", _lua_bind_any_living_danger);
        lua_register(L, "play_area_width", _lua_bind_play_area_width);
        lua_register(L, "play_area_height", _lua_bind_play_area_height);

        lua_register(L, "play_area_set_edge_behavior", _lua_bind_play_area_edge_behavior);
        lua_register(L, "play_area_border_notify", _lua_bind_play_area_border_notify);
        lua_register(L, "play_area_stop_all_border_notifications", _lua_bind_play_area_stop_all_border_notifications);
        lua_register(L, "play_area_stop_border_notify", _lua_bind_play_area_stop_border_notify);
        lua_register(L, "play_area_set_allow_border_switch_flashing", _lua_bind_play_area_set_allow_border_switch_flashing);
        lua_register(L, "play_area_notify_current_border_status", _lua_bind_play_area_notify_current_border_status);
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
        lua_register(L, "random_attack_sound", _lua_bind_random_attack_sound);

        bind_gameplay_dialogue_lualib(L);
#if 1
        /* These are per frame objects */
        /*
         * NOTE: I'm not a fan of doing per scalar things
         *       but I also don't think it's a big deal as well...
         */
        lua_register(L, "render_object_create", _lua_bind_create_render_object);
        lua_register(L, "render_object_destroy", _lua_bind_destroy_render_object);
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

        // NOTE: these should consider "boss filter" enemies
        // but I need ways to adjust entity flags
        lua_register(L, "convert_all_bullets_to_score", _lua_bind_convert_all_bullets_to_score);
        lua_register(L, "convert_all_enemies_to_score", _lua_bind_convert_all_enemies_to_score);
        lua_register(L, "kill_all_bullets", _lua_bind_kill_all_bullets);
        lua_register(L, "kill_all_enemies", _lua_bind_kill_all_enemies);
        lua_register(L, "award_points", _lua_bind_award_points);

        lua_register(L, "disable_grazing", _lua_bind_disable_grazing);
        lua_register(L, "enable_grazing", _lua_bind_enable_grazing);

        lua_register(L, "disable_bullet_to_points", _lua_bind_disable_bullet_to_points);
        lua_register(L, "disable_enemy_to_points", _lua_bind_disable_enemy_to_points);

        lua_register(L, "enable_bullet_to_points", _lua_bind_enable_bullet_to_points);
        lua_register(L, "enable_enemy_to_points", _lua_bind_enable_enemy_to_points);

        lua_register(L, "show_damage_player_will_take", _lua_bind_show_damage_player_will_take);
        lua_register(L, "hide_damage_player_will_take", _lua_bind_hide_damage_player_will_take);
        lua_register(L, "show_gameplay_alert", _lua_bind_show_game_alert);
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

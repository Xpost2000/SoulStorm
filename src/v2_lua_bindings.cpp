#include "v2.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

void push_v2_as_lua(lua_State* L, V2 v) {
    lua_newtable(L);
    lua_pushnumber(L, v.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, v.y);
    lua_rawseti(L, -2, 2);
}

V2 v2_from_lua(lua_State* L, s32 index) {
    lua_rawgeti(L, index, 1);
    lua_rawgeti(L, index, 2);
    return V2(luaL_checknumber(L, -2), luaL_checknumber(L, -1));
}

void bind_v2_lualib(lua_State* L) {
    lua_register(L, "v2",
        [](lua_State* L) {
            auto v = V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
            push_v2_as_lua(L, v);
            return 1;
        });
    lua_register(L, "v2_add",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            push_v2_as_lua(L, v + v1);
            return 1;
        });
    lua_register(L, "v2_sub",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            push_v2_as_lua(L, v - v1);
            return 1;
        });
    lua_register(L, "v2_dot",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            lua_pushnumber(L, V2_dot(v, v1));
            return 1;
        });
    lua_register(L, "v2_distance",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            f32 distance = V2_distance(v, v1);
            lua_pushnumber(L, distance);
            return 1;
        });
    lua_register(L, "v2_distance_sq",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            f32 distance = V2_distance_sq(v, v1);
            lua_pushnumber(L, distance);
            return 1;
        });
    lua_register(L, "v2_direction",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            auto v1 = v2_from_lua(L, 2);
            push_v2_as_lua(L, V2_direction(v, v1));
            return 1;
        });
    lua_register(L, "v2_direction_from_degree",
        [](lua_State* L) {
            auto v1 = luaL_checknumber(L, 1);
            push_v2_as_lua(L, V2_direction_from_degree(v1));
            return 1;
        });

    lua_register(L, "v2_magnitude",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            lua_pushnumber(L, v.magnitude());
            return 1;
        });
    lua_register(L, "v2_perp",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            push_v2_as_lua(L, V2_perpendicular(v));
            return 1;
        });
    lua_register(L, "v2_normalized",
        [](lua_State* L) {
            auto v = v2_from_lua(L, 1);
            push_v2_as_lua(L, v.normalized());
            return 1;
        });
}

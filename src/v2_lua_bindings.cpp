#include "v2.h"

#include "lua_binding_macro.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

GAME_LUA_MODULE(
v2,
"src/v2_lua_bindings_generated.cpp",
"Support library for vector2 implemented natively in C++.",
"This just provides access to the V2 interface that is defined in C++,\
which supports all common mathematics operations along with some additional helpers."
)

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

GAME_LUA_PROC(
v2,
"x: number, y: number -> V2",
"Construction procedure for V2 objects.",
"Will return a lua table representing a V2. With the x component\
at index 1, and the y component at index 2."
)
{
    auto v = V2(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    push_v2_as_lua(L, v);
    return 1;
}

GAME_LUA_PROC(
v2_add,
"a: V2, b: V2 -> V2",
"Performs component-wise addition on two V2s",
"Will return a new lua table representing the sum of the two input vectors."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    push_v2_as_lua(L, v + v1);
    return 1;
}

GAME_LUA_PROC(
v2_sub,
"a: V2, b: V2 -> V2",
"Performs component-wise subtraction on two V2s",
"Will return a new lua table representing the difference of the two input vectors."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    push_v2_as_lua(L, v - v1);
    return 1;
}

GAME_LUA_PROC(
v2_dot,
"a: V2, b: V2 -> number",
"Performs the dot product operation on two V2s",
"Will perform the dot product operation, which is the sum of the component wise products\
of the two input vectors."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    lua_pushnumber(L, V2_dot(v, v1));
    return 1;
}

GAME_LUA_PROC(
v2_distance,
"a: V2, b: V2 -> number",
"Calculates the euclidean distance between two vectors.",
"Will calculate the euclidean distance between two vectors."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    f32 distance = V2_distance(v, v1);
    lua_pushnumber(L, distance);
    return 1;
}

GAME_LUA_PROC(
v2_distance_sq,
"a: V2, b: V2 -> number",
"Calculates the squared euclidean distance between two vectors.",
"Will calculate the squared euclidean distance between two vectors."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    f32 distance = V2_distance_sq(v, v1);
    lua_pushnumber(L, distance);
    return 1;
}

GAME_LUA_PROC(
v2_direction,
"a: V2, b: V2 -> V2",
"Calculate the direction between two vectors.",
"Will calculate the delta between two vectors as (b-a) and returning a unit vector\
representing the direction between them."
)
{
    auto v = v2_from_lua(L, 1);
    auto v1 = v2_from_lua(L, 2);
    push_v2_as_lua(L, V2_direction(v, v1));
    return 1;
}

GAME_LUA_PROC(
v2_direction_from_degree,
"angle_degrees: number",
"Calculate the direction cooresponding to the degree input.",
"Will calculate a unit vector which represents the provided degree input."
)
{
    auto v1 = luaL_checknumber(L, 1);
    push_v2_as_lua(L, V2_direction_from_degree(v1));
    return 1;
}

GAME_LUA_PROC(
v2_magnitude,
"a: V2",
"Calculate the magnitude of the input vector.",
"Calculate the magnitude of the input vector."
)
{
    auto v = v2_from_lua(L, 1);
    lua_pushnumber(L, v.magnitude());
    return 1;
}

GAME_LUA_PROC(
v2_perp,
"a: V2",
"Calculate a vector perpendicular to the input.",
"Calculate a vector perpendicular to the input. That is a vector whose dot product \
with the input is zero."
)
{
    auto v = v2_from_lua(L, 1);
    push_v2_as_lua(L, V2_perpendicular(v));
    return 1;
}

GAME_LUA_PROC(
v2_normalized,
"a: V2",
"Return a unit vector with the same direction as the input.",
"Return a unit vector with the same direction as the input."
)
{
    auto v = v2_from_lua(L, 1);
    push_v2_as_lua(L, v.normalized());
    return 1;
}

void bind_v2_lualib(lua_State* L) {
    lua_register(L, "v2",                       GAME_LUA_BINDING_NAME(v2));
    lua_register(L, "v2_add",                   GAME_LUA_BINDING_NAME(v2_add));
    lua_register(L, "v2_sub",                   GAME_LUA_BINDING_NAME(v2_sub));
    lua_register(L, "v2_dot",                   GAME_LUA_BINDING_NAME(v2_dot));
    lua_register(L, "v2_distance",              GAME_LUA_BINDING_NAME(v2_distance));
    lua_register(L, "v2_distance_sq",           GAME_LUA_BINDING_NAME(v2_distance_sq));
    lua_register(L, "v2_direction",             GAME_LUA_BINDING_NAME(v2_direction));
    lua_register(L, "v2_direction_from_degree", GAME_LUA_BINDING_NAME(v2_direction_from_degree));
    lua_register(L, "v2_magnitude",             GAME_LUA_BINDING_NAME(v2_magnitude));
    lua_register(L, "v2_perp",                  GAME_LUA_BINDING_NAME(v2_perp));
    lua_register(L, "v2_normalized",            GAME_LUA_BINDING_NAME(v2_normalized));
}

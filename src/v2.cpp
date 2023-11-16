#include "v2.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

V2::V2(f32 x, f32 y) : x(x), y(y) {
    
}

V2 V2::operator+(const V2& other) {
    return V2(x + other.x, y + other.y);
}

V2 V2::operator-(const V2& other) {
    return V2(x - other.x, y - other.y);
}

V2 V2::operator+(const f32 scalar) {
    return V2(x + scalar, y + scalar);
}

V2 V2::operator-(const f32 scalar) {
    return V2(x - scalar, y - scalar);
}

V2 V2::operator*(const f32 scalar) {
    return V2(x * scalar, y * scalar);
}

V2 V2::operator/(const f32 scalar) {
    return V2(x / scalar, y / scalar);
}

V2& V2::operator+=(const V2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

V2& V2::operator-=(const V2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

V2& V2::operator+=(f32 other) {
    x += other;
    y += other;
    return *this;
}

V2& V2::operator-=(f32 other) {
    x -= other;
    y -= other;
    return *this;
}

V2& V2::operator*=(f32 other) {
    x *= other;
    y *= other;
    return *this;
}

V2& V2::operator/=(f32 other) {
    x /= other;
    y /= other;
    return *this;
}

float V2::magnitude() const {
    return sqrtf(magnitude_sq());
}

float V2::magnitude_sq() const {
    return x*x + y*y;
}

V2 V2::normalized() const {
    auto mag = magnitude();
    if (mag == 0.0f) return *this;
    auto self = *this;
    return self / mag;
}

// free functions
V2 V2_lerp(V2 a, V2 b, V2 t) {
    V2 result(lerp_f32(a.x, b.x, t.x), lerp_f32(a.y, b.y, t.y));
    return result;
}
V2 V2_lerp_scalar(V2 a, V2 b, f32 t) {
    return V2(lerp_f32(a.x, b.x, t), lerp_f32(a.y, b.y, t));
}

float V2_dot(V2 a, V2 b) {
    return a.x * b.x + a.y * b.y;
}
float V2_distance(V2 a, V2 b) {
    return sqrtf(V2_distance_sq(a, b));
}
float V2_distance_sq(V2 a, V2 b) {
    f32 dx = b.x - a.x;
    f32 dy = b.y - a.y;
    
    return dx*dx + dy*dy;
}
V2 V2_direction(V2 a, V2 b) {
    V2 delta = b - a;
    return delta.normalized();
}

V2 V2_direction_from_degree(f32 x) {
    return V2(cosf(degree_to_radians(x)), sinf(degree_to_radians(x)));
}

V2 V2_perpendicular(V2 x) {
    return V2(-x.y, x.x);
}

void push_v2_as_lua(lua_State* L, V2 v) {
    lua_newtable(L);
    lua_pushnumber(L, v.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, v.y);
    lua_rawseti(L, -2, 2);
}

V2 v2_from_lua(lua_State* L, s32 index) {
    lua_rawgeti(L,index,1);
    lua_rawgeti(L,index,2);
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

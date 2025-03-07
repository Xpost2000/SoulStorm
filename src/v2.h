#ifndef V2_H
#define V2_H

#include "common.h"

union V2 {
    struct {
        f32 x, y;
    };
    f32 xy[2];

    // NOTE:
    // I dislike how basic constructors like this discount it from being a POD.
    // I'll change this and hope it doesn't break anything in a bit.
    V2() { xy[0] = xy[1] = 0.0f; }
    V2(f32 x, f32 y);
    V2(f32 k);

    V2 operator+(const V2& other);
    V2 operator-(const V2& other);

    V2 operator+(const f32 scalar);
    V2 operator-(const f32 scalar);
    V2 operator*(const f32 scalar);
    V2 operator/(const f32 scalar);

    V2& operator+=(const V2& other);
    V2& operator-=(const V2& other);
    V2& operator+=(f32 other);
    V2& operator-=(f32 other);
    V2& operator*=(f32 other);
    V2& operator/=(f32 other);

    f32& operator[](int index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
        }
        assertion(0 && "invalid index");
    }

    float magnitude() const;
    float magnitude_sq() const;

    V2 normalized() const;
};

V2 V2_lerp(V2 a, V2 b, V2 t);
V2 V2_lerp_scalar(V2 a, V2 b, f32 t);

float V2_dot(V2 a, V2 b);
float V2_distance(V2 a, V2 b);
float V2_distance_sq(V2 a, V2 b);
V2 V2_direction(V2 a, V2 b);
V2 V2_direction_from_degree(f32 x);
V2 V2_rotate(V2 x, f32 angle_z, f32 angle_y=0.0f, f32 angle_x=0.0f);
V2 V2_perpendicular(V2 x);

struct lua_State;
void bind_v2_lualib(lua_State* L);


#endif

#ifndef V2_H
#define V2_H

#include "common.h"

union V2 {
    struct {
        f32 x, y;
    };
    f32 xy[2];

    V2(f32 x, f32 y);

    V2 operator+(const V2& other);
    V2 operator-(const V2& other);

    V2 operator+(f32 scalar);
    V2 operator-(f32 scalar);
    V2 operator*(f32 scalar);
    V2 operator/(f32 scalar);

    V2& operator+=(const V2& other);
    V2& operator-=(const V2& other);
    V2& operator+=(const float& other);
    V2& operator-=(const float& other);
    V2& operator*=(const float& other);
    V2& operator/=(const float& other);

    float magnitude();
    float magnitude_sq();

    V2 normalized();
};

V2 V2_lerp(V2 a, V2 b, V2 t);
V2 V2_lerp_scalar(V2 a, V2 b, f32 t);

float V2_dot(V2 a, V2 b);
float V2_distance(V2 a, V2 b);
float V2_distance_sq(V2 a, V2 b);
V2 V2_direction(V2 a, V2 b);
V2 V2_direction_from_degree(f32 x);
V2 V2_perpendicular(V2 x);


#endif

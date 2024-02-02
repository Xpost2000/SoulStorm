#include "v2.h"

// NOTE: from software_renderer.cpp
inline local void _rotate_f32_xy_as_pseudo_zyx(f32* x, f32* y, f32 c=0, f32 s=0, f32 c1=0, f32 s1=0, f32 c2=0, f32 s2=1) {
    // z rot
    float _x = *x;
    float _y = *y;
    *x = floor(c * (_x) - s * (_y));
    *y = floor(s * (_x) + c * (_y));

#if 0
    // y rot
    *y = floor(c1 * (*y));
    // x rot
    *x = floor(s2 * (*x));
#endif
}


V2::V2(f32 k)
    : x(k),
      y(k){
    
}
V2::V2(f32 x, f32 y)
    : x(x),
      y(y) {
    
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

V2 V2_rotate(
    V2 x,
    f32 angle,
    f32 angle_y,
    f32 angle_x
) {
    f32 c = cosf(degree_to_radians(angle));
    f32 s = sinf(degree_to_radians(angle));
    f32 c1 = cosf(degree_to_radians(angle_y));
    f32 s1 = sinf(degree_to_radians(angle_y));
    f32 c2 = cosf(degree_to_radians(angle_x));
    f32 s2 = sinf(degree_to_radians(angle_x));
    _rotate_f32_xy_as_pseudo_zyx(&x.x, &x.y, c, s, c1, s1, c2, s2);

    return x;
}

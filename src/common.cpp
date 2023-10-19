#include "common.h"
#include "V2.h"

rectangle_f32 rectangle_f32_centered(rectangle_f32 center_region, f32 width, f32 height) {
    return rectangle_f32(
        center_region.x + (center_region.w/2) - (width/2),
        center_region.y + (center_region.h/2) - (height/2),
        width, height
    );
}

rectangle_f32 rectangle_f32_scale(rectangle_f32 a, f32 k) {
    a.x *= k;
    a.y *= k;
    a.w *= k;
    a.h *= k;
    return a;
}

bool rectangle_f32_intersect(rectangle_f32 a, rectangle_f32 b) {
    if (a.x < b.x + b.w && a.x + a.w > b.x &&
        a.y < b.y + b.h && a.y + a.h > b.y) {
        return true;
    }

    return false;
}

bool circle_f32_intersect(circle_f32 a, circle_f32 b) {
    V2 a_pos = V2(a.x, a.y);
    V2 b_pos = V2(b.x, b.y);

    V2 delta = b_pos - a_pos;
    f32 r_delta = b.r + a.r;

    _debugprintf("A <%3.3f, %3.3f> (%3.3f)", a_pos.x, a_pos.y, a.r);
    _debugprintf("B <%3.3f, %3.3f> (%3.3f)", b_pos.x, b_pos.y, b.r);

    return V2_dot(delta, delta) <= r_delta*r_delta;
}

void _debug_print_bitstring(u8* bytes, unsigned length) {
    unsigned bits = length * 8;
    _debugprintfhead();
    /* reverse print, higher addresses come first, lower addresses come last. To give obvious endian representation */
    for (s32 bit_index = bits-1; bit_index >= 0; --bit_index) {
        unsigned real_index   = (bit_index / 8);
        u8       current_byte = bytes[real_index];
        _debugprintf1("%d", (current_byte & BIT(bit_index % 8)) > 0);
        if (((bit_index) % 8) == 0) {
            _debugprintf1(" ");   
        }
    }
    _debugprintf1("\n");
}

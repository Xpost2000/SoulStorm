#include "camera.h"
#include "prng.h"

void camera_set_trauma(struct camera* camera, f32 trauma) {
    camera->trauma = trauma;

    if (camera->trauma >= MAX_CAMERA_TRAUMA) {
        camera->trauma = MAX_CAMERA_TRAUMA;
    }
}
void camera_traumatize(struct camera* camera, f32 trauma) {
    camera->trauma += trauma;

    if (camera->trauma >= MAX_CAMERA_TRAUMA) {
        camera->trauma = MAX_CAMERA_TRAUMA;
    }
}


void camera_set_point_to_interpolate(struct camera* camera, V2 point, f32 zoom) {
    camera->interpolation_t[0] = 0;
    camera->interpolation_t[1] = 0;
    camera->interpolation_t[2] = 0;
    camera->try_interpolation[0] = true;
    camera->try_interpolation[1] = true;
    camera->try_interpolation[2] = true;
    camera->start_interpolation_values[0] = camera->xy.x;
    camera->start_interpolation_values[1] = camera->xy.y;
    camera->start_interpolation_values[2] = camera->zoom;

    camera->tracking_xy = point;
    camera->tracking_zoom = zoom;
}
void camera_set_point_to_interpolate(struct camera* camera, V2 point) {
    camera->interpolation_t[0] = 0;
    camera->interpolation_t[1] = 0;
    camera->try_interpolation[0] = true;
    camera->try_interpolation[1] = true;
    camera->start_interpolation_values[0] = camera->xy.x;
    camera->start_interpolation_values[1] = camera->xy.y;

    camera->tracking_xy = point;
}

V2 camera_transform(struct camera* camera, V2 point, s32 screen_width, s32 screen_height) {
    point.x *= camera->zoom;
    point.y *= camera->zoom;

    if (camera->centered) {
        point.x += screen_width/2;
        point.y += screen_height/2;
    }

    point.x -= camera->xy.x;
    point.y -= camera->xy.y;

    return point;
}
struct rectangle_f32 camera_transform_rectangle(struct camera* camera, struct rectangle_f32 rectangle, s32 screen_width, s32 screen_height) {
    V2 rectangle_position = V2(rectangle.x, rectangle.y);
    rectangle_position       = camera_transform(camera, rectangle_position, screen_width, screen_height);
    rectangle.x              = rectangle_position.x;
    rectangle.y              = rectangle_position.y;
    rectangle.w             *= camera->zoom;
    rectangle.h             *= camera->zoom;

    return rectangle;
}

V2 camera_project(struct camera* camera, V2 point, s32 screen_width, s32 screen_height) {
    point.x += camera->xy.x;
    point.y += camera->xy.y;

    if (camera->centered) {
        point.x -= (screen_width / 2);
        point.y -= (screen_height / 2);
    }

    point.x /= camera->zoom;
    point.y /= camera->zoom;
    return point;
}

V2 camera_displacement_from_trauma(struct camera* camera) {
    struct random_state* rng           = camera->rng;
    if (!rng) {
        _debugprintf("No camera prng source? Weird");
        return V2(0, 0);
    }

    f32                  trauma_factor = camera->trauma;

    f32 random_x = random_ranged_integer(rng, -MAX_TRAUMA_DISPLACEMENT_X * trauma_factor * TRAUMA_FACTOR_WEIGHT_X, MAX_TRAUMA_DISPLACEMENT_X * trauma_factor * TRAUMA_FACTOR_WEIGHT_X);
    f32 random_y = random_ranged_integer(rng, -MAX_TRAUMA_DISPLACEMENT_Y * trauma_factor * TRAUMA_FACTOR_WEIGHT_Y, MAX_TRAUMA_DISPLACEMENT_Y * trauma_factor * TRAUMA_FACTOR_WEIGHT_Y);

    return V2(random_x, random_y);
}

struct rectangle_f32 camera_project_rectangle(struct camera* camera, struct rectangle_f32 rectangle, s32 screen_width, s32 screen_height) {
    V2 rectangle_position = V2(rectangle.x, rectangle.y);
    rectangle_position       = camera_project(camera, rectangle_position, screen_width, screen_height);
    rectangle.x              = rectangle_position.x;
    rectangle.y              = rectangle_position.y;
    rectangle.w             /= camera->zoom;
    rectangle.h             /= camera->zoom;

    return rectangle;
}

void camera_update(struct camera* camera, f32 dt) {
    camera->trauma_displacement = camera_displacement_from_trauma(camera);
    {
        if (camera->trauma > 0) {
            camera->trauma -= dt;
        } else {
            camera->trauma = 0;
        }
    }

    {
        const f32 lerp_component_speeds[3] = {
            1.25, 1.25, 1.25,
        };

        for (unsigned component_index = 0; component_index < 3; ++component_index) {
            if (camera->try_interpolation[component_index]) {
                if (camera->interpolation_t[component_index] < 1.0) {
                    camera_set_component(camera, component_index, quadratic_ease_in_f32(camera->start_interpolation_values[component_index],
                                                                                        camera_get_tracking_component(camera, component_index),
                                                                                        camera->interpolation_t[component_index]));
                    camera->interpolation_t[component_index] += dt * lerp_component_speeds[component_index];
                } else {
                    camera->try_interpolation[component_index] = false;
                }
            }
        }
    }
}

local f32* camera_resolve_component_ptr(struct camera* camera, s32 index) {
    f32* component_pointer = nullptr;

    switch (index) {
        case 0: {
            component_pointer = &camera->xy.x;
        } break;
        case 1: {
            component_pointer = &camera->xy.y;
        } break;
        case 2: {
            component_pointer = &camera->zoom;
        } break;
    }

    return component_pointer;
}

local f32* camera_resolve_tracking_component_ptr(struct camera* camera, s32 index) {
    f32* component_pointer = nullptr;

    switch (index) {
        case 0: {
            component_pointer = &camera->tracking_xy.x;
        } break;
        case 1: {
            component_pointer = &camera->tracking_xy.y;
        } break;
        case 2: {
            component_pointer = &camera->tracking_zoom;
        } break;
    }

    return component_pointer;
}

void camera_set_component(struct camera* camera, s32 index, f32 value) {
    f32* component_pointer = camera_resolve_component_ptr(camera, index);
    *component_pointer = value;
}

void camera_set_tracking_component(struct camera* camera, s32 index, f32 value) {
    f32* component_pointer = camera_resolve_tracking_component_ptr(camera, index);
    *component_pointer = value;
}

f32 camera_get_component(struct camera* camera, s32 index) {
    f32* component_pointer = camera_resolve_component_ptr(camera, index);
    return *component_pointer;
}

f32  camera_get_tracking_component(struct camera* camera, s32 index) {
    f32* component_pointer = camera_resolve_tracking_component_ptr(camera, index);
    return *component_pointer;
}

bool camera_already_interpolating_for(struct camera* camera, V2 where, f32 zoom) {
    return (
        (
            camera->tracking_xy.x == where.x &&
            camera->tracking_xy.y == where.y &&
            camera->tracking_zoom == zoom
        )
        &&
        (
            camera->interpolation_t[0] != 0.0f &&
            camera->interpolation_t[1] != 0.0f &&
            camera->interpolation_t[2] != 0.0f
        )
    );
}

bool camera_interpolating(struct camera* camera) {
    for (int i = 0; i < 3; ++i) {
        if (camera->try_interpolation[i]) {
            return true;
        }
    }

    return false;
}

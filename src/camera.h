// NOTE: implementation is in graphics.cpp
#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

#define MAX_CAMERA_TRAUMA (16)

#define TRAUMA_FACTOR_WEIGHT_X (0.367)
#define TRAUMA_FACTOR_WEIGHT_Y (0.24)

#define MAX_TRAUMA_DISPLACEMENT_X (150)
#define MAX_TRAUMA_DISPLACEMENT_Y (150)

struct random_state;
struct camera {
    random_state* rng;
    V2            xy;
    f32           zoom;
    u8            centered;

    /*
      NOTE:
      The region size where the camera may start to readjust to the player.
      
      Only used for the gameplay world camera, and specified in screen coordinates.
      
      Combat camera is free travel.
      
      All this data here is for camera updatings, only used by the game.
      Not the editor code.
      
      Would technically be a struct game_camera or something.
    */
    rectangle_f32 travel_bounds;
    V2                tracking_xy;
    f32               tracking_zoom;

    /* NOTE x/y/zoom */
    f32                  interpolation_t[3];
    f32                  start_interpolation_values[3];
    bool                 try_interpolation[3];

    f32 trauma;
};

void camera_set_trauma(struct camera* camera, f32 trauma);
void camera_traumatize(struct camera* camera, f32 trauma);

void camera_set_point_to_interpolate(struct camera* camera, V2 point);
V2 camera_transform(struct camera* camera, V2 point, s32 screen_width, s32 screen_height);
V2 camera_displacement_from_trauma(struct camera* camera);
rectangle_f32 camera_project_rectangle(struct camera* camera, rectangle_f32 rectangle, s32 screen_width, s32 screen_height);
V2 camera_project(struct camera* camera, V2 point, s32 screen_width, s32 screen_height);
rectangle_f32 camera_transform_rectangle(struct camera* camera, rectangle_f32 rectangle, s32 screen_width, s32 screen_height);

#define camera(XY, ZOOM) camera { nullptr, XY, ZOOM }
#define camera_centered(XY, ZOOM) camera { nullptr, XY, ZOOM, true}

#endif

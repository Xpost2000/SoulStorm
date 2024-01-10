#ifndef TITLE_SCREEN_MODE_H
#define TITLE_SCREEN_MODE_H

#define MAX_TITLE_SCREEN_OUTERSPACE_STARS (1024)
#define MAX_TITLE_SCREEN_SPARKLING_STARS  (256)

// this will not be random since that's a lot more work
#define TITLESCREEN_MAINCHARACTER_PUPPET_TIME_BETWEEN_BLINKS (2.25f)
struct TitleScreen_MainCharacter_Puppet {
    // NOTE: refers to center/torso
    V2  position;
    f32 scale               = 2.75;
    s32 eye_frame           = 0;
    f32 blink_timer         = 0.0f;
    f32 time_between_blinks = TITLESCREEN_MAINCHARACTER_PUPPET_TIME_BETWEEN_BLINKS;
    V2  eye_look_target     = V2(0,0);

    void update_blinking(f32 dt);

    void update_breathing_position_behavior(
        f32 dt,
        V2& head_position,
        V2& torso_position,
        V2& left_arm_position,
        V2& right_arm_position
    );

    void update_eye_look_behavior(
        f32 dt,
        V2 eye_look_direction,
        V2& left_eye_position,
        V2& right_eye_position
    );

    void update(f32 dt);
    void draw(f32 dt, struct render_commands* commands, Game_Resources* resources);
};

enum TitleScreen_Animation_Phase {
    TITLE_SCREEN_ANIMATION_PHASE_CLOSE_UP_OF_FACE,
    TITLE_SCREEN_ANIMATION_PHASE_ZOOM_OUT,
    TITLE_SCREEN_ANIMATION_PHASE_MOVE_PUPPET_TO_RIGHT_AND_FADE_IN_MENU,
    TITLE_SCREEN_ANIMATION_PHASE_IDLE,
};

struct TitleScreen_Data {
    // there might not be anything
    // here, but it's just to keep the pattern
    // Fill in with stuff?

    // NOTE: planning to just use coroutines to
    // do this as well, but would require more reified
    // menu items and stuff... However due to how static
    // this menu is intending to be... I might as well
    // just write this using a state machine.
    struct camera main_camera;
    random_state  prng;

    s32 phase      = TITLE_SCREEN_ANIMATION_PHASE_CLOSE_UP_OF_FACE;
    f32 anim_timer = 0.0f;

    V2 star_positions[MAX_TITLE_SCREEN_OUTERSPACE_STARS];
    Visual_Sparkling_Star_Data sparkling_stars[MAX_TITLE_SCREEN_SPARKLING_STARS];

    TitleScreen_MainCharacter_Puppet puppet;

    s32 last_screen_width = -1;
    s32 last_screen_height = -1;
};

#endif

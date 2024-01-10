#ifndef TITLE_SCREEN_MODE_H
#define TITLE_SCREEN_MODE_H

#define MAX_TITLE_SCREEN_OUTERSPACE_STARS (512)
#define MAX_TITLE_SCREEN_SPARKLING_STARS  (128)
struct TitleScreen_MainCharacter_Puppet {
    // NOTE: refers to center/torso
    V2 position;

    void update(f32 dt);
    void draw(struct render_commands* commands, Game_Resources* resources);
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

    V2 star_positions[MAX_TITLE_SCREEN_OUTERSPACE_STARS];
    Visual_Sparkling_Star_Data sparkling_stars[MAX_TITLE_SCREEN_SPARKLING_STARS];

    TitleScreen_MainCharacter_Puppet puppet;

    s32 last_screen_width = -1;
    s32 last_screen_height = -1;
};

#endif

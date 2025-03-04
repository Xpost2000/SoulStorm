#ifndef TRAILER_CUTAWAY_MODE_H
#define TRAILER_CUTAWAY_MODE_H

#define COMPILE_IN_TRAILER_CLIPPING_CODE 1

/*
    NOTE(jerry):
     My character's puppet doesn't have any rigging setup for any
     skeletal animation software, and I don't have much of these things in,
     so for now I'm animating this scene entirely through code since it's a little
     easier...

     Should you really do this? Eh, honestly doing it programmatically *does* have some
     advantages, but probably not really.
*/

enum Trailer_Cutaway_Animation_Phase {
    TRAILER_CUTAWAY_ANIMATION_PHASE_IDLE_BLINK,
    TRAILER_CUTAWAY_ANIMATION_PHASE_LOOK_UP,
    TRAILER_CUTAWAY_ANIMATION_PHASE_PAN_UPWARDS_TO_SHOW_LOGO,
    TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_IN_OTHER_INFO,
    TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_TO_BLACK,
    TRAILER_CUTAWAY_ANIMATION_PHASE_RELEASE_DATE_FADE_IN,
    TRAILER_CUTAWAY_ANIMATION_PHASE_FADE_ALL_AWAY,
    TRAILER_CUTAWAY_ANIMATION_PHASE_COUNT,
};

enum Trailer_Cutaway_Platform_Mask_Flags {
    TRALIER_CUTAWAY_PLATFORM_LOGO_NONE    = 0,
    TRAILER_CUTAWAY_PLATFORM_LOGO_WINDOWS = BIT(0),
    TRAILER_CUTAWAY_PLATFORM_LOGO_APPLE   = BIT(1),
    TRAILER_CUTAWAY_PLATFORM_LOGO_LINUX   = BIT(2),
    TRAILER_CUTAWAY_PLATFORM_LOGO_STEAM   = BIT(3),
    TRAILER_CUTAWAY_PLATFORM_LOGO_ITCH    = BIT(4),
    TRAILER_CUTAWAY_PLATFORM_LOGO_COUNT   = 6
};

struct Trailer_Cutaway_Mode_Data {
#ifdef COMPILE_IN_TRAILER_CLIPPING_CODE
    u32 platform_mask_flags;
    s32 anim_phase;
    f32 anim_t;
    V2 star_positions[MAX_TITLE_SCREEN_OUTERSPACE_STARS];
    Visual_Sparkling_Star_Data sparkling_stars[MAX_TITLE_SCREEN_SPARKLING_STARS];
    TitleScreen_MainCharacter_Puppet puppet; 
#endif
};

#endif
#ifndef OPENING_MODE_H
#define OPENING_MODE_H

/*
 * Opening mode screen "slide show things"
 *
 * Maybe a neat opening with my branding or whatever, idk
 * it would just be nice to have something animated for that.
 *
 * But that's not really as important as the actual story related slides.
 *
 * It's like the dialogue system sort of.
 *
 */

#include "particle_system.h"
#include "v2.h"
#include "camera.h"


// I would love to use coroutines for this...
// but the duff routines are not ergonomic enough imo to be used for
// more general purpose stuff like this.
#define OPENING_MODE_SLIDE_TEXT_TYPE_SPEED       (0.060f)
#define OPENING_MODE_DISPLAY_DELAY_TIME          (0.65f)
#define OPENING_MODE_DISPLAY_READ_DELAY_TIME     (2.25f)
#define OPENING_MODE_DISPLAY_FADE_TEXT_TIME      (0.45f)
#define OPENING_MODE_DISPLAY_FADE_DELAY_TIME     (1.25f)
#define OPENING_MODE_DISPLAY_FADE_CROSSFADE_TIME (1.25)
enum OpeningMode_SlideData_Phase {
    OPENING_MODE_SLIDE_DATA_PHASE_DISPLAY_DELAY = 0,
    OPENING_MODE_SLIDE_DATA_PHASE_TYPE_TEXT     = 1,
    OPENING_MODE_SLIDE_DATA_PHASE_READ_DELAY    = 2,
    OPENING_MODE_SLIDE_DATA_PHASE_FADE_TEXT     = 3,
    OPENING_MODE_SLIDE_DATA_PHASE_FADE_DELAY    = 4,
    OPENING_MODE_SLIDE_DATA_PHASE_CROSSFADE     = 5,
};

struct OpeningMode_SlideData {
    // NOTE: should be 854, 480, not enforced,
    // but this is what I assume as that's the intended design
    // setup.
    image_id slide_image;
    string   slide_caption;

    // Runtime data
    int   display_phase    = 0;
    int   shown_characters = 0;
    float timer            = 0;
};

// NOTE: this fade in will not utilize the transition system
// as I realize the callback system is a little too finnicky and unreliable
// and I should've used a different method. I can't exactly "reverse" a lot of
// the earlier decisions made (since I am not maintaining this forever), but
// I can at least do better for the future stuff.
enum OpeningMode_Phase {
    /*
     * I would like something more interesting for the "fade in phase".
     *
     *  Like having a few stars, that "blink", and then a bigger shooting star that
     *  flies through.
     *
     */
    OPENING_MODE_PHASE_FADE_IN = 0,
    OPENING_MODE_PHASE_SLIDESHOW = 1,
    OPENING_MODE_PHASE_FADE_OUT = 2,
};

#define OPENING_MODE_FADE_TIMER_MAX (3.0f)
#define OPENING_MODE_FADE_TIMER_ENDING_MAX (1.25f)

#define OPENING_MODE_DATA_SKIPPER_VISIBILITY_MAX_T (0.35f)
#define OPENING_MODE_DATA_SKIPPER_PROGRESS_MAX_T (1.0f)

struct OpeningMode_Data {
    int phase = OPENING_MODE_PHASE_FADE_IN;
    OpeningMode_SlideData slides[32];
    uint8_t               slide_count = 0; 
    uint8_t               slide_index = 0;
    f32                   fade_timer  = 0.0f;

    // NOTE: I need controller prompts and such in order to actually
    //       "show things", for now I'll have an abstract "skip bar"
    // so just add skip prompt.
    f32 skipper_visibility_t = 0.0f;
    f32 skipper_progress_t   = 0.0f;

    void unload_all_assets(Game_Resources* resources);
    void update_slide(OpeningMode_SlideData* slide, f32 dt);
    void update_and_render_skipper_box(f32 dt, Game_Resources* resources, struct render_commands* ui_render_commands);
};

#endif
#ifndef ENDING_MODE_H
#define ENDING_MODE_H

#define ENDING_MODE_FADE_TIME   (1.5f)
#define ENDING_MODE_LINGER_TIME (7.0f)

enum EndingMode_Phase {
    ENDING_MODE_PHASE_FADE_IN_FROM_BLACK = 0,
    ENDING_MODE_PHASE_LINGER             = 1,
    ENDING_MODE_PHASE_FADE_OUT_TO_BLACK  = 2,
    ENDING_MODE_PHASE_COUNT              = 3,
};

struct EndingMode_Data {
    // nothing for now.
    image_id ending_slide;
    s32 phase;
    f32 timer;

    void unload_all_assets(Game_Resources* resources);
};

#endif

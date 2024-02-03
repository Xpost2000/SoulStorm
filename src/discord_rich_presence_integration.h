#ifndef DISCORD_RICH_PRESENCE_INTEGRATION
#define DISCORD_RICH_PRESENCE_INTEGRATION

#include "common.h"

namespace Discord_Integration {
    void initialize(void);
    void deinitialize(void);
    void update_activity(string details, string state, string large_icon);
    void per_frame_update(f32 dt);
}

#endif

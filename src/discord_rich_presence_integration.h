#ifndef DISCORD_RICH_PRESENCE_INTEGRATION
#define DISCORD_RICH_PRESENCE_INTEGRATION

#include "common.h"

#define DISCORD_GAMEICON_ASSET_KEY string_literal("icondraft")

// using another builder pattern :)
struct Discord_Activity_Info {
    string state;
    string details;

    string large_image;
    string large_text;

    string small_image;
    string small_text;

    u64    timestamp_start;

    Discord_Activity_Info& State(string text) {
        this->state = text;
        return *this;
    }

    Discord_Activity_Info& Details(string text) {
        this->details = text;
        return *this;
    }

    Discord_Activity_Info& Large_Image(string text) {
        this->large_image = text;
        return *this;
    }

    Discord_Activity_Info& Large_Text(string text) {
        this->small_text = text;
        return *this;
    }

    Discord_Activity_Info& Small_Image(string text) {
        this->small_image = text;
        return *this;
    }

    Discord_Activity_Info& Small_Text(string text) {
        this->small_text = text;
        return *this;
    }
};

Discord_Activity_Info discord_activity(void);
Discord_Activity_Info discord_timestamped_activity(void);

namespace Discord_Integration {
    void initialize(void);
    void deinitialize(void);
    void update_activity(Discord_Activity_Info info);
    void per_frame_update(f32 dt);
}

#endif

#include "discord_rich_presence_integration.h"
#include "string.h"

#ifndef COMPILE_DISCORD_RICH_PRESENCE
Discord_Activity_Info discord_activity(void) {
    return Discord_Activity_Info {};
}

Discord_Activity_Info discord_timestamped_activity(u64 t) {
    return Discord_Activity_Info {
        .timestamp_start = t
    };
}

namespace Discord_Integration {
    void initialize(void) {
    }

    void deinitialize(void) {
    }

    void update_activity(Discord_Activity_Info info) {
    }

    void per_frame_update(f32 dt) {
    }
}

#else
#include <discord_game_sdk.h>

// NOTE: make your own discord api key.
// even if I'm sharing source code, sharing the key is a bad idea.
#include "discord_api_key.cpp"

local struct IDiscordCore* core;
local struct IDiscordUsers* users;
local struct IDiscordActivityManager* activities;
local IDiscordCoreEvents core_events;
local IDiscordActivityEvents activity_events;

void DISCORD_CALLBACK update_activity_callback(void* data, enum EDiscordResult result){
    if (result != DiscordResult_Ok) {
        _debugprintf("Discord integration error?");
    }
}

Discord_Activity_Info discord_activity(void) {
    return Discord_Activity_Info {};
}

Discord_Activity_Info discord_timestamped_activity(u64 t) {
    return Discord_Activity_Info {
        .timestamp_start = t
    };
}

namespace Discord_Integration {
    void initialize(void) {
        zero_memory(&core_events, sizeof(core_events));
        zero_memory(&activity_events, sizeof(activity_events));
        struct DiscordCreateParams params;
        params.client_id = DISCORD_APPLICATION_ID;
        params.flags = DiscordCreateFlags_Default;
        params.events = &core_events;
        params.activity_events = &activity_events;
        params.event_data = nullptr;

        DiscordCreate(DISCORD_VERSION, &params, &core);
    
        activities = core->get_activity_manager(core);
    }

    void deinitialize(void) {
        core->destroy(core);
    }

    void update_activity(Discord_Activity_Info info) {
        struct DiscordActivity activity;
        zero_memory(&activity, sizeof(activity));
        copy_string_into_cstring(info.state, activity.state, array_count(activity.state));
        copy_string_into_cstring(info.details, activity.details, array_count(activity.details));

        copy_string_into_cstring(info.large_image, activity.assets.large_image, array_count(activity.assets.large_image));
        copy_string_into_cstring(info.large_text, activity.assets.large_text, array_count(activity.assets.large_text));

        copy_string_into_cstring(info.small_image, activity.assets.small_image, array_count(activity.assets.small_image));
        copy_string_into_cstring(info.small_text, activity.assets.small_text, array_count(activity.assets.small_text));
        activity.timestamps.start = info.timestamp_start;
        activities->update_activity(activities, &activity, nullptr, update_activity_callback);
    }

    void per_frame_update(f32 dt) {
        core->run_callbacks(core);
    }
}

#endif

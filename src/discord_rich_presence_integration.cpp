#include "discord_rich_presence_integration.h"
#include <discord_game_sdk.h>

#include "string.h"

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

    void update_activity(string details, string state, string large_icon) {
        struct DiscordActivity activity;
        zero_memory(&activity, sizeof(activity));
        copy_string_into_cstring(details, activity.details, array_count(activity.details));
        copy_string_into_cstring(state, activity.state, array_count(activity.state));
        copy_string_into_cstring(large_icon, activity.assets.large_image, array_count(activity.assets.large_image));
        activities->update_activity(activities, &activity, nullptr, update_activity_callback);
    }

    void per_frame_update(f32 dt) {
        core->run_callbacks(core);
    }
}

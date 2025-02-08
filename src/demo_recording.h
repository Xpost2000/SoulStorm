#ifndef DEMO_RECORDING_H
#define DEMO_RECORDING_H

#define DEFAULT_DEMO_VIEWER_TIMESCALE_INDEX (3)
enum Gameplay_Recording_File_Version {
    GAMEPLAY_RECORDING_FILE_VERSION_1 = 0,

    // Pets added to recording file
    GAMEPLAY_RECORDING_FILE_VERSION_2 = 1,
    GAMEPLAY_RECORDING_FILE_CURRENT_VERSION = GAMEPLAY_RECORDING_FILE_VERSION_2
};

#define GAMEPLAY_FRAME_INPUT_PACKET_ACTION_ACTION_BIT   (0)
#define GAMEPLAY_FRAME_INPUT_PACKET_ACTION_FOCUS_BIT    (1)
#define GAMEPLAY_FRAME_INPUT_PACKET_ACTION_USE_BOMB_BIT (2)

// Describes a single frame of "unpaused" gameplay.
// Thankfully as a danmaku/bullet hell. There's not much data.
struct Gameplay_Frame_Input_Packet {
    u8 actions;
    s8 axes[2];
};
V2 gameplay_frame_input_packet_quantify_axes(const Gameplay_Frame_Input_Packet& input_packet);

struct Gameplay_Recording_File {
    // check for both version and tickrate.
    u16                          version;
    s16                          tickrate;
    struct random_state          start_prng; // unused?
    struct random_state          prng;
    struct random_state          old_prng; // For session restoration.
    s32                          frame_count;
    u8                           stage_id;
    u8                           level_id;
    s8                           selected_pet;

    /*
      NOTE:

      the game basically doesn't perform any allocations during game time,
      (I allocate render_object resources from lua, and obviously I don't control what memory
      lua itself allocates, but the main fixed game state arena never changes),

      the expectation is that the recording will attempt to eat up whatever remaining memory there is,
      and then just clears itself when it's done.
    */
    Gameplay_Frame_Input_Packet* frames;

    // runtime data
    Memory_Arena*                memory_arena = nullptr;
    u64                          memory_arena_cursor = 0;

    bool                         in_playback = false;
    s32                          playback_frame_index = 0;
    s32                          frames_run           = 0;
};
// NOTE: for what it's worth, I actually prefer this kind of C API
// and it's kind of habitual for me to write stuff like this in my own personal code anyway...
void                    gameplay_recording_file_start_recording(Gameplay_Recording_File* recording, struct random_state prng_state, Memory_Arena* arena);
void                    gameplay_recording_file_record_frame(Gameplay_Recording_File* recording, const Gameplay_Frame_Input_Packet& frame_input);
void                    gameplay_recording_file_stop_recording(Gameplay_Recording_File* recording);
void                    gameplay_recording_file_finish(Gameplay_Recording_File* recording);

// NOTE: if not "write" serializing, provide a memory arena argument so that the
// gameplay_recording data can be allocated.
bool                        gameplay_recording_file_serialize(Gameplay_Recording_File* recording, Memory_Arena* arena, struct binary_serializer* serializer);
// NOTE: assumes that serialize was opened...
void                        gameplay_recording_file_start_playback(Gameplay_Recording_File* recording);
Gameplay_Frame_Input_Packet gameplay_recording_file_next_frame(Gameplay_Recording_File* recording);
bool                        gameplay_recording_file_has_more_frames(Gameplay_Recording_File* recording);

struct Gameplay_Demo_Viewer {
    // NOTE  this is kind of expensive to compute,
    // it would be nice to cache, but this doesn't really super duper
    // work afaik.
    bool paused                = false;
    bool arbitrary_frame_visit = false; 
    int  timescale_index       = DEFAULT_DEMO_VIEWER_TIMESCALE_INDEX;
};

struct Gameplay_Demo_Collection_UI {
    int current_page = 0;
};


#endif

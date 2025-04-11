void gameplay_recording_file_start_recording(Gameplay_Recording_File* recording,
                                             struct random_state prng_state,
                                             Memory_Arena* arena) {
    _debugprintf(
        "Start recording:\n(prng: %d, %d, %d, %d,  %d)",
        prng_state.constant, prng_state.multiplier, prng_state.state, prng_state.seed, prng_state.modulus
    );

    recording->version                    = GAMEPLAY_RECORDING_FILE_CURRENT_VERSION;
    recording->tickrate                   = TICKRATE;
    recording->prng                       = recording->start_prng = prng_state;
    recording->frame_count                = 0;
    recording->memory_arena               = arena;
    recording->memory_arena_cursor        = arena->used;
    recording->in_playback                = false;
    recording->frames_run                 = 0;
    recording->frames                     = (Gameplay_Frame_Input_Packet*)recording->memory_arena->push_unaligned(0);
}

void gameplay_recording_file_record_frame(Gameplay_Recording_File* recording, const Gameplay_Frame_Input_Packet& frame_input) {
    assert(recording->memory_arena && "Cannot record frame without allocator");
    recording->memory_arena->push_unaligned(sizeof(frame_input));
    recording->frames[recording->frame_count++] = frame_input;
    recording->frames_run += 1;
}


void gameplay_recording_file_finish(Gameplay_Recording_File* recording) {
    if (recording->memory_arena) {
        recording->memory_arena->used = recording->memory_arena_cursor;
    }
    recording->memory_arena = nullptr;
    _debugprintf("Run through: %d frames of gameplay", recording->frames_run);
    _debugprintf("Playback: %d/%d frames", recording->playback_frame_index, recording->frame_count);
}
void gameplay_recording_file_stop_recording(Gameplay_Recording_File* recording) {
    // NOTE:
    // memory is deliberately not reset.
    // It is resident because the intention is to serialize if needed.
    // IE: it is fine for the remainder of a frame.
    gameplay_recording_file_finish(recording);
    recording->prng = recording->start_prng;
    _debugprintf("Recorded: %d frames", recording->frame_count);
}

bool gameplay_recording_file_serialize(Gameplay_Recording_File* recording, Memory_Arena* arena, struct binary_serializer* serializer) {
    serialize_u16(serializer, &recording->version);
    serialize_s16(serializer, &recording->tickrate);
    serialize_s32(serializer, &recording->prng.constant);
    serialize_s32(serializer, &recording->prng.multiplier);
    serialize_s32(serializer, &recording->prng.state);
    serialize_s32(serializer, &recording->prng.seed);
    serialize_s32(serializer, &recording->prng.modulus);
    serialize_s32(serializer, &recording->frame_count);
    serialize_u8(serializer,  &recording->stage_id);
    serialize_u8(serializer,  &recording->level_id);
    if (recording->version >= GAMEPLAY_RECORDING_FILE_VERSION_2) {
        serialize_s8(serializer,  &recording->selected_pet);
    } else {
        recording->selected_pet = GAME_PET_ID_NONE;
    }
    _debugprintf("Serializing recording version: %d", recording->version);
    _debugprintf("Serializing recording tickrate: %d", recording->tickrate);
    _debugprintf(
        "Serializing Recording PRNG:\n(prng: %d, %d, %d, %d, %d)",
        recording->prng.constant, recording->prng.multiplier, recording->prng.state, recording->prng.seed, recording->prng.modulus
    );

    if (recording->version >= GAMEPLAY_RECORDING_FILE_VERSION_3) {
        recording->applied_rules = serialize_game_rules(serializer);
    } else {
        recording->applied_rules = g_default_game_rules;
    }

#if 0 
    if (recording->version != GAMEPLAY_RECORDING_FILE_CURRENT_VERSION) {
        _debugprintf("Version mismatch");
        return false;
    }
#endif

    switch (recording->version) {
        case GAMEPLAY_RECORDING_FILE_VERSION_3: 
        case GAMEPLAY_RECORDING_FILE_VERSION_2: 
        {
            if (arena) {
                recording->memory_arena = arena;
                recording->memory_arena_cursor = arena->used;
                recording->frames =
                    (Gameplay_Frame_Input_Packet*)recording->memory_arena->push_unaligned(recording->frame_count * sizeof(*recording->frames));
            }

            for (s32 index = 0; index < recording->frame_count; ++index) {
                serialize_u8(serializer, &recording->frames[index].actions);
                serialize_s8(serializer, &recording->frames[index].axes[0]);
                serialize_s8(serializer, &recording->frames[index].axes[1]);
            }

            return true;
        } break;
        case GAMEPLAY_RECORDING_FILE_VERSION_1: 
        default: {
            // TODO: should make this error more pleasantly in the future.
            // Although the game has little to no UI to speak of.
            return false;
        } break;
    }

    return false;
}

void gameplay_recording_file_start_playback(Gameplay_Recording_File* recording) {
    recording->frames_run           = 0;
    recording->in_playback          = true;
    recording->playback_frame_index = 0;
    g_game_rules                    = recording->applied_rules;
}

bool gameplay_recording_file_has_more_frames(Gameplay_Recording_File* recording) {
    return recording->playback_frame_index < recording->frame_count;
}

Gameplay_Frame_Input_Packet gameplay_recording_file_next_frame(Gameplay_Recording_File* recording) {
    recording->frames_run += 1;
    return recording->frames[recording->playback_frame_index++];
}

V2 gameplay_frame_input_packet_quantify_axes(const Gameplay_Frame_Input_Packet& input_packet) {
    f32 x = (f32)input_packet.axes[0] / 127.0f;
    f32 y = (f32)input_packet.axes[1] / 127.0f;
    auto result = V2(x, y);
    apply_vector_quantization_deadzone_adjustment(result);
    return result;
}

void Gameplay_Data::build_current_input_packet() {
    {
        V2 axes = V2(Action::value(ACTION_MOVE_LEFT) + Action::value(ACTION_MOVE_RIGHT), Action::value(ACTION_MOVE_UP) + Action::value(ACTION_MOVE_DOWN));
        if (axes.magnitude_sq() > 1.0f) axes = axes.normalized();

        current_input_packet.actions=
            (BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_ACTION_BIT))  *Action::is_down(ACTION_ACTION) |
            (BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_FOCUS_BIT))   *Action::is_down(ACTION_FOCUS)  |
            ((BIT(GAMEPLAY_FRAME_INPUT_PACKET_ACTION_USE_BOMB_BIT))* (Action::is_down(ACTION_USE_BOMB) && !just_used_bomb))
            ;

        apply_vector_quantization_deadzone_adjustment(axes);
        current_input_packet.axes[0] = (s8)(axes[0] * 127.0f);
        current_input_packet.axes[1] = (s8)(axes[1] * 127.0f);
    }
}

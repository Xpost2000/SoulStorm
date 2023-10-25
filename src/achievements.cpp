#include "achievements.h"
#include "fixed_array.h"

namespace Achievements {
    local Fixed_Array<Achievement> global_achievements;

    void init_achievements(Memory_Arena* arena, Slice<Achievement> achievements) {
        global_achievements = Fixed_Array<Achievement>(arena, achievements.data, achievements.length);
    }

    void serialize_achievements(binary_serializer* serializer) {
        unimplemented("Need to setup serialization.");
    }

    Achievement* get(s16 id) {
        return &global_achievements[id];
    }

    Achievement* get(string id_name) {
        for (int i = 0; i < global_achievements.size; ++i) {
            auto& a = global_achievements[i];

            if (string_equal(a.id_name, id_name)) {
                return &a;
            }
        }

        return nullptr;
    }

    Slice<Achievement> get_all() {
        return make_slice<Achievement>(global_achievements.data, global_achievements.size);
    }
}

bool Achievement::report(s32 i) {
    switch (progress_type) {
        case ACHIEVEMENT_PROGRESS_TYPE_INT: {
            progress.as_int.increment(i);
            achieved = (progress.as_int.full());
        } break;
        case ACHIEVEMENT_PROGRESS_TYPE_FLOAT: {
            progress.as_float.increment(i);
            achieved = (progress.as_float.full());
        } break;
        default: {
            report();
        } break;
    }
    return complete();
}

bool Achievement::report(f32 i) {
    switch (progress_type) {
        case ACHIEVEMENT_PROGRESS_TYPE_INT: {
            progress.as_int.increment(i);
            achieved = (progress.as_int.full());
        } break;
        case ACHIEVEMENT_PROGRESS_TYPE_FLOAT: {
            progress.as_float.increment(i);
            achieved = (progress.as_float.full());
        } break;
        default: {
            report();
        } break;
    }
    return complete();
}

bool Achievement::report() {
    achieved = true;
    return complete();
}

bool Achievement::complete() {
    return achieved;
}

bool Achievement::notify_unlock() {
    if (!notified_of_unlock) {
        notified_of_unlock = true;
        return true;
    }

    return false;
}

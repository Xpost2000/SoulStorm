#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H
/*
 * NOTE: I need a UI to hook this up to later.
 */

#include "common.h"
#include "serializer.h"

template<typename T>
struct Bounded {
    void increment(T by = 1) {
        set(current + by);
    }

    void set(const T v) {
        T v1 = v;
        if (v1 < min) v1 = min;
        if (v1 > max) v1 = max;
        current = v1;
    }

    bool full() const  { return current  >= max; }
    bool empty() const { return current <= min; }

    T min, max;
    T current;
};

template<typename T>
Bounded<T> bounded(T min, T max) {
    Bounded result;
    result.min = result.current = min;
    result.max = max;
    return result;
}

enum Achievement_Progress_Type {
    ACHIEVEMENT_PROGRESS_TYPE_BOOL,
    ACHIEVEMENT_PROGRESS_TYPE_INT,
    ACHIEVEMENT_PROGRESS_TYPE_FLOAT,
};

struct Achievement {
    s16    id; // should link to an achievement table name in the game code
    s8     progress_type;
    s8     hidden;
    s8     achieved;

    string id_name;
    string name;
    string description;


    union {
        Bounded<s32> as_int;
        Bounded<f32> as_float;
    } progress;

    /*
     * We can never "re-lock" an achievement. Assume that
     * we will only ever make forward progress.
     *
     * These reports will return true if the achievement was achieved.
     */
    bool report(s32 i);
    bool report(f32 i);
    bool report(); // automatically unlock achievement regardless of anything
    bool complete();
};

/* Personally, I like namespaces over static classes. But that's just me. */
namespace Achievements {
    // provide an achievement list somewhere else, and have it copied to permenant memory.
    void         init_achievements(Memory_Arena* arena, Slice<Achievement> achievements);
    void         serialize_achievements(binary_serializer* serializer);
    Achievement* get(s16 id);
    Achievement* get(string id_name);

    Slice<Achievement> get_all();
}

#endif

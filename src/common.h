#ifndef COMMON_H
#define COMMON_H

#include <x86intrin.h>

// NOTE: I know there's a lot of redundantly compiled
// things because I static inline a lot of small functions.

// C stuff
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <cstdint>
#include <climits>
#include <cstdbool>

#include <cassert>
#include <ctime>
#include <cstring>

#include <vector>
#include <array>

#define _USE_MATH_DEFINES
#include <cmath>

#define assertion(x) assert(x)
#define unimplemented(x) assertion(false && x);
#define bad_case         default: { unimplemented ("no case"); } break
#define invalid_cases()  bad_case
#define array_count(x) (sizeof(x)/sizeof(x[0]))
#define local    static
#define internal static
#define safe_assignment(x) if(x) *x

#define BIT(x)             (1 << x)
#define BIT64(x) (uint64_t)(1LL << x)
#define Toggle_Bit(a, x) (a ^= x)
#define Set_Bit(a, x)    (a |= x)
#define Get_Bit(a, x)    (a & x)

#define Bytes(x)                    (uint64_t)(x)
#define Byte(x)                    (uint64_t)(x)
#define Kilobyte(x)                 (uint64_t)(x * 1024LL)
#define Megabyte(x)                 (uint64_t)(x * 1024LL * 1024LL)
#define Gigabyte(x)                 (uint64_t)(x * 1024LL * 1024LL * 1024LL)
#define Terabyte(x)                 (uint64_t)(x * 1024LL * 1024LL * 1024LL * 1024LL)

#ifndef RELEASE
#define _debugprintf(fmt, args...)   fprintf(stderr, "[%s:%d:%s()]: " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#define _debugprintfhead()   fprintf(stderr, "[%s:%d:%s()]: " ,__FILE__, __LINE__, __func__)
#define _debugprintf1(fmt, args...)  fprintf(stderr,  fmt, ##args)
#define DEBUG_CALL(fn) fn; _debugprintf("calling %s in [%s:%d:%s()]", #fn, __FILE__, __LINE__, __func__)
#else
#define _debugprintf(fmt, args...)  
#define _debugprintfhead(fmt, args...)
#define _debugprintf1(fmt, args...)
#define DEBUG_CALL(_) _;
#endif

#define unused(x) (void)(x)

#define Fixed_Array_Push(array, counter) &array[counter++]
#define Fixed_Array_Remove_And_Swap(array, where, counter) array[where] = array[--counter]

typedef char* cstring;

typedef float  f32;
typedef double f64;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8; /* byte */

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

/* I know about r-value references and std::move... */
#define Swap(a, b, type)                        \
    do {                                        \
        type tmp = a;                           \
        a = b;                                  \
        b = tmp;                                \
    } while (0)
#define XorSwap(a, b)                           \
    do {                                        \
        a ^= b;                                 \
        b ^= a;                                 \
        a ^= b;                                 \
    } while (0)

template <typename T>
T min(T a, T b) {
    if (a < b) return a;
    return b;
}
template <typename T>
T max(T a, T b) {
    if (a < b) return b;
    return a;
}
template <typename T>
T clamp(T x, T min, T max) {
    T i = (x < min) ? min : x;
    return (i > max)  ? max : i;
}

inline static s32 lerp_s32(s32 a, s32 b, f32 normalized_t) {
    return a * (1 - normalized_t) + (b * normalized_t);
}

inline static f32 lerp_f32(f32 a, f32 b, f32 normalized_t) {
    return a * (1 - normalized_t) + (b * normalized_t);
}

inline static f32 cubic_ease_in_f32(f32 a, f32 b, f32 normalized_t) {
    return (b - a) * (normalized_t * normalized_t * normalized_t) + a;
}

inline static f32 cubic_ease_out_f32(f32 a, f32 b, f32 normalized_t) {
    normalized_t -= 1;
    return (b - a) * (normalized_t * normalized_t * normalized_t) + a;
}

inline static f32 cubic_ease_in_out_f32(f32 a, f32 b, f32 normalized_t) {
    if (normalized_t < 0.5) {
        f32 new_t = 4 * normalized_t * normalized_t * normalized_t;
        normalized_t = new_t;
    } else {
        f32 new_t = pow(-2 * normalized_t + 2, 3) / 2;
        normalized_t = new_t;
    }

    return (b - a) * normalized_t + a;
}

inline static f32 quadratic_ease_in_f32(f32 a, f32 b, f32 normalized_t) {
    return (b - a) * (normalized_t * normalized_t) + a;
}

inline static f32 quadratic_ease_out_f32(f32 a, f32 b, f32 normalized_t) {
    return -(b - a) * ((normalized_t * normalized_t) - 2.0) + a;
}

inline static f32 quadratic_ease_in_out_f32(f32 a, f32 b, f32 normalized_t) {
    if (normalized_t < 0.5) {
        f32 new_t = (2 * normalized_t * normalized_t);
        normalized_t = new_t;
    } else {
        f32 new_t = 1 - pow(-2 * normalized_t + 2, 2) / 2;
        normalized_t = new_t;
    }

    return (b - a) * normalized_t + a;
}

#define EASE_BACK_C1 (1.70158)
#define EASE_BACK_C2 (EASE_BACK_C1*1.525)
#define EASE_BACK_C3 (EASE_BACK_C1+1)
inline static f32 ease_in_back_f32(f32 a, f32 b, f32 normalized_t) {
    f32 t = ((EASE_BACK_C3) * normalized_t * normalized_t * normalized_t )- ((EASE_BACK_C3) * normalized_t * normalized_t);
    return (b - a) * t + a;
}

inline static f32 ease_out_back_f32(f32 a, f32 b, f32 normalized_t) {
    f32 t = 1 + (EASE_BACK_C3) * pow(normalized_t - 1, 3) + (EASE_BACK_C1) * pow(normalized_t - 1, 2);
    return (b - a) * t + a;
}

inline static f32 ease_in_out_back_f32(f32 a, f32 b, f32 normalized_t) {
    if (normalized_t < 0.5) {
        f32 new_t = (pow(2 * normalized_t, 2) * ((EASE_BACK_C2+1) * 2 * normalized_t - EASE_BACK_C2))/2;
        normalized_t = new_t;
    } else {
        f32 new_t = (pow(2 * normalized_t - 2, 2) * ((EASE_BACK_C2 + 1) * ((normalized_t * 2 - 2) + EASE_BACK_C2) + 2))/2;
        normalized_t = new_t;
    }

    return (b - a) * normalized_t + a;
}

inline static f32 fractional_f32(f32 x) {
    return x - floor(x);
}
inline static f32 whole_f32(f32 x) {
    return x - fractional_f32(x);
}
inline static f32 step_f32(f32 x, f32 edge) {
    return (x < edge) ? 0 : 1;
}
inline static s32 step_s32(s32 x, s32 edge) {
    return (x < edge) ? 0 : 1;
}

inline static f32 pick_f32(f32 a, f32 b, s32 v) {
    return (v == 0) ? a : b;
}

inline static s32 pick_s32(s32 a, s32 b, s32 v) {
    return (v == 0) ? a : b;
}

inline static const f32 degree_to_radians(f32 x) {
    return x * M_PI/180.0f;
}

inline static const f32 radians_to_degrees(f32 x) {
    return x * 180.0f/M_PI;
}

/* if the order is confusing, most significant bit is first. */
static inline u16 packu16(u8 b0, u8 b1) {
    return ((u32)(b0) << 4) | ((u32)(b1));
}

static inline u32 packu32(u8 b0, u8 b1, u8 b2, u8 b3) {
    return ((u32)(b0) << 24) |
        ((u32)(b1) << 16)    |
        ((u32)(b2) << 8)     |
        ((u32)b3);
}

static inline u32 hash_bytes_fnv1a(u8* bytes, size_t length) {
    u32 offset_basis = 2166136261;
    u32 prime        = 16777619;

    u32 hash = offset_basis;

    for (unsigned index = 0; index < length; ++index) {
        hash ^= bytes[index];
        hash *= prime;
    }

    return hash;
}

static inline void zero_memory(void* memory, size_t amount) {
    for (u64 index = 0; index < amount; ++index) {
        ((u8*)memory)[index] = 0;
    }
}

#define TEMPORARY_STORAGE_BUFFER_SIZE (2048)
#define TEMPORARY_STORAGE_BUFFER_COUNT (8)
local char* format_temp(const char* fmt, ...) {
    local int current_buffer = 0;
    local char temporary_text_buffer[TEMPORARY_STORAGE_BUFFER_COUNT][TEMPORARY_STORAGE_BUFFER_SIZE] = {};

    char* target_buffer = temporary_text_buffer[current_buffer++];
    zero_memory(target_buffer, TEMPORARY_STORAGE_BUFFER_SIZE);
    {
        va_list args;
        va_start(args, fmt);
        int written = vsnprintf(target_buffer, TEMPORARY_STORAGE_BUFFER_SIZE-1, fmt, args);
        va_end(args);
    }

    if (current_buffer >= TEMPORARY_STORAGE_BUFFER_COUNT) {
        current_buffer = 0;
    }

    return target_buffer;
}
#define zero_array(x) zero_memory(x, array_count(x))
#define zero_struct(x) zero_memory(&x, sizeof(x));

/* TODO: add file utilities like read_entire_file and stuff */

#endif

#ifndef COMMON_H
#define COMMON_H
#define NO_FLAGS (0)

#include <immintrin.h> 
//#include <x86intrin.h>

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
#define M_PI (3.141592653589793238462643383279502884197)

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

inline static char* __shorten_path_length(char* original, int depth) {
    if (depth < 0) depth = 1;
    // assumed from __FILE__...
    int length = strlen(original);

    char* ptr = original + (length-1);
    while (depth && *ptr) {
        ptr--;
        if (*ptr == '\\' || *ptr == '/') depth--;
    }

    return ptr;
}

#ifndef RELEASE
#define _debugprintfhead()   fprintf(stderr, "[%s:%d:%s()]: " , __shorten_path_length((char*)__FILE__, 2), __LINE__, __func__)
// #define _debugprintf1(...)  do { _debugprintfhead(); fprintf(stderr, __VA_ARGS__); } while(0)
#define _debugprintf1(...)  do { DebugUI::print(format_temp("[%s:%d:%s()]: %s", __shorten_path_length((char*)__FILE__, 2), __LINE__, __func__,  format_temp(__VA_ARGS__))); _debugprintfhead(); fprintf(stderr, __VA_ARGS__); } while(0)
#define _debugprintf(...)   do { _debugprintf1(__VA_ARGS__); fprintf(stderr, "\n"); } while (0) 
#define DEBUG_CALL(fn) fn; _debugprintf("calling %s in [%s:%d:%s()]", #fn, __FILE__, __LINE__, __func__)
#else
// #define _debugprintf(fmt, ...)  
// #define _debugprintfhead(fmt, ...)
// #define _debugprintf1(fmt, ...)
#define _debugprintf(...)  
#define _debugprintfhead()
#define _debugprintf1(...)
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

// NOTE: not sure what header I included for this to be a compilation error on MSVC

#undef min
#undef max
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

inline static s32 sign_s32(s32 a) {
    if (a > 0) return 1;
    if (a < 0) return -1;
    return 0;
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
#define TEMPORARY_STORAGE_BUFFER_COUNT (32)
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

struct rectangle_f32 {
    f32 x; f32 y; f32 w; f32 h;
};
struct rectangle_s32 {
    s32 x; s32 y; s32 w; s32 h;
};
struct rectangle_s16 {
    s16 x; s16 y; s16 w; s16 h;
};

struct circle_f32 {
    f32 x; f32 y; f32 r;
};

#define circle_f32(X,Y,R) circle_f32 {(f32)X, (f32)Y, (f32) R}

#define rectangle_f32(X,Y,W,H) rectangle_f32 {(f32)X,(f32)Y,(f32)W,(f32)H}
#define rectangle_s32(X,Y,W,H) rectangle_s32 {(s32)X,(s32)Y,(s32)W,(s32)H}
#define rectangle_s16(X,Y,W,H) rectangle_s16 {(s16)X,(s16)Y,(s16)W,(s16)H}

#define RECTANGLE_F32_NULL rectangle_f32(0,0,0,0)
#define RECTANGLE_S32_NULL rectangle_s32(0,0,0,0)
#define RECTANGLE_S16_NULL rectangle_s16(0,0,0,0)

rectangle_f32 rectangle_f32_centered(rectangle_f32 center_region, f32 width, f32 height);
rectangle_f32 rectangle_f32_scale(rectangle_f32 a, f32 k);

bool rectangle_f32_intersect(rectangle_f32 a, rectangle_f32 b);

bool circle_f32_intersect(circle_f32 a, circle_f32 b);

static inline void memory_set8(void* memory, size_t amount, u8 value) {
    u8* memory_u8 = (u8*)memory;
    for (u64 index = 0; index < amount; ++index) {
        memory_u8[index] = value;
    }
}
static inline void memory_set16(void* memory, size_t amount, u16 value) {
    u16* memory_u16 = (u16*)memory;
    for (u64 index = 0; index < amount/2; ++index) {
        memory_u16[index] = value;
    }
}
static inline void memory_set32(void* memory, size_t amount, u32 value) {
    u32* memory_u32 = (u32*)memory;
    for (u64 index = 0; index < amount/4; ++index) {
        memory_u32[index] = value;
    }
}
static inline void memory_set64(void* memory, size_t amount, u64 value) {
    u64* memory_u64 = (u64*)memory;
    for (u64 index = 0; index < amount/8; ++index) {
        memory_u64[index] = value;
    }
}

static inline void memory_copy(void* source, void* destination, u64 amount) {
    memcpy(destination, source, amount);
}

// endian stuff
enum endianess {
    ENDIANESS_LITTLE = 1,
    ENDIANESS_BIG    = 2,
};

#define ByteUnion(BITS, TYPE)                   \
    union {                                     \
        TYPE as_ ## TYPE;                           \
        u8   as_bytes[BITS/8];               \
    }

static inline enum endianess system_get_endian(void) {
    ByteUnion(32, u32) x;
    x.as_u32 = 1;

    if (x.as_bytes[0]) {
        return ENDIANESS_LITTLE;
    }

    return ENDIANESS_BIG;
}

/* NOTE: I am horrified by how I use the preprocessor sometimes, */
#define Define_ByteSwap_Procedures(BITCOUNT)                            \
    inline local u ## BITCOUNT byteswap_u##BITCOUNT(u##BITCOUNT input) { \
        ByteUnion(BITCOUNT, u##BITCOUNT) x;                             \
        x.as_u##BITCOUNT = input;                                       \
        for (unsigned byte_index = 0; byte_index < sizeof(u##BITCOUNT)/2; ++byte_index) { \
            XorSwap(x.as_bytes[byte_index], x.as_bytes[(sizeof(u##BITCOUNT)-1) - byte_index]); \
        }                                                               \
        return x.as_u##BITCOUNT;                                        \
    }                                                                   \
    inline local s ## BITCOUNT byteswap_s##BITCOUNT(s##BITCOUNT input) { \
        ByteUnion(BITCOUNT, s##BITCOUNT) x;                             \
        x.as_s##BITCOUNT = input;                                       \
        for (unsigned byte_index = 0; byte_index < sizeof(s##BITCOUNT)/2; ++byte_index) { \
            XorSwap(x.as_bytes[byte_index], x.as_bytes[(sizeof(u##BITCOUNT)-1) - byte_index]); \
        }                                                               \
        return x.as_s##BITCOUNT;                                        \
    }                                                                   \
    inline local void inplace_byteswap_u##BITCOUNT(u##BITCOUNT *input) { \
        assertion(input && "cannot byteswap nullptr?");                 \
        u##BITCOUNT copy = *input;                                      \
        copy = byteswap_u##BITCOUNT(copy);                              \
        *input = copy;                                                  \
    }                                                                   \
    inline local void inplace_byteswap_s##BITCOUNT(s##BITCOUNT *input) { \
        assertion(input && "cannot byteswap nullptr?");                 \
        s##BITCOUNT copy = *input;                                      \
        copy = byteswap_s##BITCOUNT(copy);                              \
        *input = copy;                                                  \
    }

Define_ByteSwap_Procedures(64);
Define_ByteSwap_Procedures(32);
Define_ByteSwap_Procedures(16);
inline local u8 byteswap_u8(u8 input) {
    return input;
}
inline local s8 byteswap_s8(s8 input) {
    return input;
}
inline local void inplace_byteswap_s8(s8* input) {
    return;
}
inline local void inplace_byteswap_u8(u8* input) {
    return;
}
inline local f32 byteswap_f32(f32 input) {
    ByteUnion(32, f32) x;
    x.as_f32 = input;
    for (unsigned byte_index = 0; byte_index < sizeof(f32)/2; ++byte_index) {
        XorSwap(x.as_bytes[byte_index], x.as_bytes[(sizeof(f32)-1) - byte_index]);
    }
    return x.as_f32;
}
inline local f64 byteswap_f64(f64 input) {
    ByteUnion(64, f64) x;
    x.as_f64 = input;
    for (unsigned byte_index = 0; byte_index < sizeof(f64)/2; ++byte_index) {
        XorSwap(x.as_bytes[byte_index], x.as_bytes[(sizeof(f64)-1) - byte_index]);
    }
    return x.as_f64;
}
inline local void inplace_byteswap_f32(f32 *input) {
    assertion(input && "cannot byteswap nullptr?");
    f32 copy = *input;
    copy = byteswap_f32(copy);
    *input = copy;
}
inline local void inplace_byteswap_f64(f64 *input) {
    assertion(input && "cannot byteswap nullptr?");
    f64 copy = *input;
    copy = byteswap_f32(copy);
    *input = copy;
}

void _debug_print_bitstring(u8* bytes, unsigned length);

// helper slice type
template <typename T>
struct Slice {
    T*       data;
    s32      length;

    T& operator[](int index) {
        assertion(index >= 0 && index < length && "Invalid slice index length!");
        return data[index];
    }
};

template<typename T>
Slice<T> make_slice(T* data, s32 length) {
    Slice<T> result = {};
    result.data   = data;
    result.length = length;
    return result;
}

struct string{
    s32   length;
    char* data;
};
#define string_literal(x) string { sizeof(x)-1, (char*)x }
#define string_null       string { 0, (char*) 0 }

#include "debug_ui.h"

#endif

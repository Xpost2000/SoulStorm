#ifndef STRING_H
#define STRING_H
#include "common.h"
#include "memory_arena.h"

#define _string_unwrap(x) x.length, x.data

bool is_whitespace(char c);
bool is_alphabetic(char c);
bool is_alphabetic_lowercase(char c);
bool is_alphabetic_uppercase(char c);
bool is_numeric(char c);
bool is_numeric_with_decimal(char c);
bool is_valid_real_number(const char* str);
bool is_valid_integer(const char* str);

char character_lowercase(char c);
char character_uppercase(char c);

s32 string_to_s32(string s);
f32 string_to_f32(string s);

string string_from_cstring(cstring str);
string string_from_cstring_length_counted(cstring str, u64 length);
string string_slice(string a, s32 start, s32 end);
bool   string_equal(string a, string b);
bool   string_equal_case_insensitive(string a, string b);
bool   string_is_substring(string a, string substring);

bool cstring_equal(cstring a, cstring b);
u64  cstring_length(const char* cstring);
void cstring_copy(cstring source, cstring destination, u64 destination_length);
u64  copy_string_into_cstring(string source, char* destination, u64 destination_length);

string string_concatenate(Memory_Arena* arena, string a, string b);
string string_clone(Memory_Arena* arena, string a);
string longest_string_in_list(string* strings, s32 length);

struct string_array {
    s32 count;
    string* strings;
};
struct string_array string_split(Memory_Arena* arena, string string, char separator);
string memory_arena_push_string(Memory_Arena* arena, string to_copy);
string unixify_pathname(Memory_Arena* arena, string pathname);

#endif

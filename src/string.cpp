#define MAX_START_LOCATION_TABLE_SIZE_FOR_SUBSTRING (8192)
#include "string.h"

bool is_whitespace(char c) {
    if ((c == ' ') ||
        (c == '\t') ||
        (c == '\r') ||
        (c == '\n')) {
        return true;
    }

    return false;
}

bool is_alphabetic_lowercase(char c) {
    if (c >= 'a' && c <= 'z') {
        return true;
    }

    return false;
}

bool is_alphabetic_uppercase(char c) {
    if (c >= 'A' && c <= 'Z') {
        return true;
    }

    return false;
}

bool is_alphabetic(char c) {
    return is_alphabetic_lowercase(c) || is_alphabetic_uppercase(c);
}

bool is_numeric(char c) {
    if (c == '-') return true;
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
}

bool is_numeric_with_decimal(char c) {
    return is_numeric(c) || c == '.' || c == '-';
}

bool is_valid_real_number(const char* str) {
    bool found_one_decimal = false;

    auto length = strlen(str);
    for (unsigned index = 0; index < length; ++index) {
        if (str[index] == '.') {
            if (!found_one_decimal) {
                found_one_decimal = true;
            } else {
                return false;
            }
        } else if (!is_numeric(str[index])) {
            return false;
        }
    }

    return true;
}

bool is_valid_integer(const char* str) {
    auto length = strlen(str);
    for (unsigned index = 0; index < length; ++index) {
        if (!is_numeric(str[index])) {
            return false;
        }
    }

    return true;
}

char character_lowercase(char c) {
    if (is_alphabetic_uppercase(c)) {
        return (c + 32);
    }

    return c;
}

char character_uppercase(char c) {
    if (is_alphabetic_lowercase(c)) {
        return (c - 32);
    }

    return c;
}

string string_from_cstring(cstring str) {
    return string {
        .length = (s32)cstring_length(str),
        .data   = str,
    };
}

string string_from_cstring_length_counted(cstring str, u64 length) {
    return string {
        .length = (s32)length,
        .data   = str
    };
}

bool string_equal(string a, string b) {
    if (a.length == b.length) {
        for (unsigned index = 0; index < (u32)a.length; ++index) {
            if (a.data[index] != b.data[index])
                return false;
        }

        return true;
    }

    return false;
}

bool string_equal_case_insensitive(string a, string b) {
    if (a.length == b.length) {
        for (unsigned index = 0; index < (u32)a.length; ++index) {
            if (character_lowercase(a.data[index]) != character_lowercase(b.data[index]))
                return false;
        }

        return true;
    }

    return false;
}

u64 cstring_length(const char* cstring) {
    char* cursor = (char*)cstring;

    while (*cursor) {
        cursor++;
    }

    return (u64)(cursor - cstring);
}

void cstring_copy(cstring source, cstring destination, u64 destination_length) {
    u64 source_length = cstring_length(source);

    // How do I program C++/C so much and still forget this?
    if (source_length > destination_length) source_length = destination_length;

    s32 written = 0;
    for (u64 index = 0; index < destination_length && index < source_length; ++index, ++written) {
        destination[index] = source[index];
    }

    destination[written] = 0;
}

bool cstring_equal(cstring a, cstring b) {
    u64 a_length = cstring_length(a);
    u64 b_length = cstring_length(b);

    if (a_length == b_length) {
        for (unsigned index = 0; index < a_length; ++index) {
            if (a[index] != b[index])
                return false;
        }

        return true;
    }
    
    return false;
}

string string_slice(string a, s32 start, s32 end) {
    if (end == -1) {
        end = start + a.length;   
    }

    return string {
        .length = end - start,
        .data   = a.data + start
    };
}

string string_concatenate(Memory_Arena* arena, string a, string b) {
    s32 string_length = a.length + b.length;
    string new_string = {};
    new_string.data = (char*)arena->push_unaligned(string_length+1);
    new_string.length = string_length;

    s32 write_index = 0;

    for (s32 index = 0; index < a.length; ++index) {
        new_string.data[write_index++] = a.data[index];
    }

    for (s32 index = 0; index < b.length; ++index) {
        new_string.data[write_index++] = b.data[index];
    }

    /* cstring friendly... */
    new_string.data[write_index++] = 0;
    return new_string;
}

string string_clone(Memory_Arena* arena, string a) {
    string result = {};
    result.data    = (char*)arena->push_unaligned(a.length+1);
    result.length  = a.length;
    for (s32 index = 0; index < a.length; ++index) {
        result.data[index] = a.data[index];
    }
    result.data[a.length] = 0;
    return result;
}

s32 string_to_s32(string s) {
    char* temporary = format_temp("%.*s", s.length, s.data);
    return atoi(temporary);
}

f32 string_to_f32(string s) {
    char* temporary = format_temp("%.*s", s.length, s.data);
    return atof(temporary);
}

struct string_array string_split(Memory_Arena* arena, string input_string, char separator) {
    struct string_array result = {};

    result.count = 0;
    result.strings = (string*)arena->push_unaligned(0);

    s32 character_index = 0;
    while (character_index < input_string.length) {
        s32 start_of_substring = character_index;
        s32 end_of_substring   = start_of_substring;

        while ((character_index < input_string.length) && input_string.data[character_index] != '+') {
            character_index++;
        }
        end_of_substring = character_index;
        character_index += 1;

        string substr = string_slice(input_string, start_of_substring, end_of_substring);

        arena->push_unaligned(sizeof(string));
        result.strings[result.count++] = substr;
    }

    return result;
}

u64 copy_string_into_cstring(string source, char* destination, u64 destination_length) {
    u64 smaller_length = 0;
    if ((u64)source.length < destination_length) {
        smaller_length = source.length;
    } else {
        smaller_length = destination_length;
    }

    for (u64 character_index = 0; character_index < smaller_length; ++character_index) {
        destination[character_index] = source.data[character_index];
    }

    return smaller_length;
}

bool string_is_substring(string a, string substring) {
    if (a.length < substring.length) {
        return false;
    }

    /* while this isn't obviously stupid, it's also not really that great. Just build small prefix search list */
    /* this is only really possible on small strings, since I don't ask for an arena. (Ideally you shouldn't need one) */
    s32 possible_starting_locations[MAX_START_LOCATION_TABLE_SIZE_FOR_SUBSTRING] = {};
    s32 possible_starting_location_count = 0;

    for (s32 index = 0; index < a.length; ++index) {
        if (a.data[index] == substring.data[0]) {
            if (index+1 < a.length && substring.length > 2) {
                if (a.data[index+1] == substring.data[1]) {
                    possible_starting_locations[possible_starting_location_count++] = index;
                }
            } else {
                possible_starting_locations[possible_starting_location_count++] = index;
            }
        }
    }

    for (s32 starting_location_index = 0; starting_location_index < possible_starting_location_count; ++starting_location_index) {
        for (s32 i = possible_starting_locations[starting_location_index]; i < a.length; ++i) {
            bool success = true;

            for (s32 j = 0; j < substring.length; ++j) {
                if (i+j >= a.length) {
                    success = false;
                    break;
                }
                if (a.data[i+j] != substring.data[j]) {
                    success = false;
                    break;
                }
            }

            if (success) {
                return true;
            }
        }
    }

    return false;
}

string longest_string_in_list(string* strings, s32 length) {
    string best = strings[0];

    for (s32 index = 1; index < length; ++index) {
        auto& s = strings[index];
        if (s.length > best.length) best = s;
    }

    return best;
}

string memory_arena_push_string(Memory_Arena* arena, string to_copy) {
    string result = {};
    result.length = to_copy.length;
    result.data   = (char*)arena->push_unaligned(to_copy.length+1);
    cstring_copy(to_copy.data, result.data, result.length);
    result.data[to_copy.length] = 0;
    return result;
}

string unixify_pathname(Memory_Arena* arena, string pathname) {
    string result = memory_arena_push_string(arena, pathname);
    for (int i = 0; i < result.length; ++i) {
        if (result.data[i] == '\\') {
            result.data[i] = '/';
        }
    }
    return result;
}

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

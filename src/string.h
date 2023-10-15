#ifndef STRING_H
#define STRING_H
#include "common.h"

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

#endif

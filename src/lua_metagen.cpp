/*****
 * Very very simple documentation / lua_register binding generator
 NOTE: Only documents the C++ engine bindings. Does not document the lua defined game apis
 *     which is a bit different.
 *
 * Although there is some pretty bad code! Like really bad. It will break if you
 * cough in it's direction bad!
 *
 *
 * I think there's like a hundred something lua binding functions for the
 * lua api in this engine, and I'm not going to remember most of them even
 * if some of them are primarily getters/setters.
 *
 * It makes it easier for others to make content (okay there is no real custom
 * level support yet),
 *
 * As well as allowing me to more easily figure out what my engine functions are
 * after a few months. Or weeks.

 NOTE: reads text as lines and
 will expect extremely specific in format

GAME_LUA_MODULE(
NAME,
STRING,
STRING
)

GAME_LUA_PROC(
NAME,
STRING,
STRING
)

 Anything else is liable to break this, unless I feel like making this a real lexical analyzer,
 but I'm not likely to because this is a pretty small engine/project, and is basically amateur hour.

 NOTE: will also modify "whitelisted" files, so it's not exactly a good idea to touch this lol,
 might even destroy the source code if I'm not careful.

 That's what git's for!
 *
 *****/

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
#include <string>

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

struct file_buffer {
    u8* buffer;
    u64 length;
};

struct file_buffer OS_read_entire_file(const char* path) {
    file_buffer result;

    if (FILE* f = fopen(path, "rb+")) {
        fseek(f, 0, SEEK_END);
        result.length = ftell(f);
        fseek(f, 0, SEEK_SET);
        result.buffer = (u8*)malloc(result.length+1);
        fread(result.buffer, 1, result.length, f);
        result.buffer[result.length] = 0;
        fclose(f);
    } else {
        
    }
    return result;
}

void file_buffer_free(struct file_buffer* file) {
    file->buffer = 0;
    file->length = 0;
    free(file->buffer);
}


/*
  NOTE: not really a complete lexical analysis solution, and not even really
  that thorough.

  It's not supposed to be. Just needs to do what I want it to do...
*/

// Allocation festival? Gloriously so!
std::vector<std::string> buffer_into_line_buffer(char* buffer, size_t buffer_length) {
    std::vector<std::string> result;

    size_t index = 0;

    while (index < buffer_length) {
        {
            size_t line_start = 0;
            while (index < buffer_length) {
                bool windows_crlf = (buffer[index] == '\r' && (index+1 < buffer_length) && buffer[index+1] == '\n');
                bool unix_lf      = buffer[index] == '\n';
                bool osx_cr       = buffer[index] == '\r';
                if (windows_crlf || unix_lf || osx_cr || (index + 1 >= buffer_length)) {
                    size_t line_end = index;
                    if (windows_crlf) {
                        index += 2;
                    } else {
                        index += 1;
                    }

                    {
                        std::string line;
                        line.assign(buffer + line_start, line_end - line_start);
                        result.push_back(line);
                    }
                    line_start = index;
                } else {
                    index += 1;
                }
            }
        }
    }

    return result;
}

// consumes first string.
std::string consume_string_raw(char* buffer, size_t buffer_length) {
    std::string result;

    size_t index = 0;
    if (buffer[index] == '\"') {
        size_t string_start = index+1;
        while (buffer[index] != '\"') {
            index++;
        }
        size_t string_end = index;
        result.assign(buffer + string_start, (string_end - string_start));

        return result;
    }

    return result;
}

// NOTE: given raw string
std::string escape_string(char* buffer, size_t buffer_length) {
    std::string result;
    size_t index = 0;

    while (index < buffer_length) {
        switch (buffer[index]) {
            case '\\': {
                if ((index+1) < buffer_length) {
                    index++;
                    switch (buffer[index]) {
                        case 'n': {result += '\n'; index++;} break;
                        case 't': {result += '\t'; index++;} break;
                        case 'r': {result += '\r'; index++;} break;
                        default: {
                            result += '\n';
                            index++;
                        } break;
                    }
                } else {
                    result += '\n';
                    index++;
                }
            } break;
            default: {
                result += buffer[index];
                index++;
            } break;
        }
    }

    return result;
}

size_t left_padding(std::string s) {
    size_t result = 0;
    while (result < s.size()) {
        if (!isspace(s[result])) return result;
        result++;
    }

    return result;
}

size_t right_padding(std::string s) {
    size_t result = 0;
    while (result < s.size()) {
        if (!isspace(s[s.size() - (result+1)])) return result;
        result++;
    }

    return result;
}

// Yeah, I was really not kidding with how hacky this is because
// a more proper program would require a real lexer that is significantly larger!
// and I'm not trying to write a tokenizer on a weekend unfortunately :/
std::string read_macro_param(std::vector<std::string>& lines, size_t& i) {
    std::string paramstring;
    {
        bool good = false;
        while (!good) {
            if (lines[i][lines[i].size()-1] == '\\') {
                paramstring += lines[i].substr(0, lines[i].size()-1);
                i++;
            } else {
                // lol
                // slightly buggy
                paramstring += "\n" + lines[i].substr(0, lines[i].size()-2);
                good = true;
                i++;
            }
        }
    }
    return paramstring;
}

std::string strip_extra_padding(std::string original) {
    auto lp = left_padding(original);
    original = original.substr(lp, original.size());
    auto rp = right_padding(original);
    return original.substr(0, original.size() - rp);
}

struct Lua_Game_Proc {
    std::string name;
    std::string param_doc_string;
    std::string short_description;
    std::string long_description;
};

struct Lua_Game_Module {
    std::string name;
    std::string source_outname;
    std::string short_description;
    std::string long_description;

    std::vector<Lua_Game_Proc> procedures;
};

std::vector<Lua_Game_Module> modules;

Lua_Game_Module read_lua_game_module(const char* filename) {
    Lua_Game_Module module;

    auto f = OS_read_entire_file(filename);
    auto lines = buffer_into_line_buffer((char*)f.buffer, f.length);

    size_t i = 0;
    while (i < lines.size()) {
        auto line_one = lines[i];
        if (line_one.substr(0, line_one.size()-1) == "GAME_LUA_MODULE") {
            i++;
            auto name_line = lines[i].substr(0, lines[i].size()-1);
            i++;
            std::string outstring          = strip_extra_padding(read_macro_param(lines, i)).substr(1);
            std::string shortdescription   = strip_extra_padding(read_macro_param(lines, i)).substr(1);
            std::string longestdescription = strip_extra_padding(read_macro_param(lines, i)).substr(1);

            module.name = name_line;
            module.source_outname = outstring;
            module.short_description = shortdescription;
            module.long_description = longestdescription;
        } else if (line_one.substr(0, line_one.size()-1) == "GAME_LUA_PROC") {
            i++;
            Lua_Game_Proc proc;
            auto        name_line          = lines[i].substr(0, lines[i].size()-1);
            i++;
            std::string paramdocstring     = strip_extra_padding(read_macro_param(lines, i)).substr(1);
            std::string shortdescription   = strip_extra_padding(read_macro_param(lines, i)).substr(1);
            std::string longestdescription = strip_extra_padding(read_macro_param(lines, i)).substr(1);

            proc.name = name_line;
            proc.param_doc_string = paramdocstring;
            proc.short_description = shortdescription;
            proc.long_description = longestdescription;

            module.procedures.push_back(proc);
        } else {
            i++;
        }
    }

    file_buffer_free(&f);
    return module;
}

void generate_text_documentation(const char* outloc) {
    FILE* f = fopen(outloc, "wb+");

    fprintf(f, "# Bullet Hell Game Engine API\n");
    fprintf(f, "## Modules\n");
    for (int i = 0; i < modules.size(); ++i) {
        auto& m = modules[i];
        fprintf(f, "%d. %s - %s\n", i+1, m.name.data(), m.short_description.data());
    }
    fprintf(f, "\n\n");
    for (int i = 0; i < modules.size(); ++i) {
        auto& m = modules[i];
        fprintf(f, "## %s - %s\n\n%s\n", m.name.data(), m.short_description.data(), m.long_description.data());
        fprintf(f, "\n### Table of Contents\n");
        for (int j = 0; j < m.procedures.size(); ++j) {
            auto& p = m.procedures[j];
            // TODO: wrong links.
            fprintf(f, "%d. [%s(%s)](#%s_%s__func)\n", j+1, p.name.data(), p.param_doc_string.data(), m.name.data(), p.name.data());
        }
        fprintf(f, "\n\n### Procedure Description\n");
        for (int j = 0; j < m.procedures.size(); ++j) {
            auto& p = m.procedures[j];
            fprintf(f, "#### %s(%s) - %s\n\n%s\n\n", p.name.data(), p.param_doc_string.data(), p.short_description.data(), p.long_description.data());
        }
    }

    fflush(f);
    fclose(f);
}

void generate_code_binding_files(void) {
    for (int i = 0; i < modules.size(); ++i) {
        auto& m = modules[i];
        if (FILE* f = fopen(m.source_outname.data(), "wb+")) {
            fprintf(f, "// Autogenerated file. Do not touch\n");
            fprintf(f, "void bind_%s_lualib(lua_State* L)", m.name.data());
            fprintf(f, "{\n");
            for (int j = 0; j < m.procedures.size(); ++j) {
                auto& p = m.procedures[j];
                fprintf(f, "    lua_register(L, \"%s\", GAME_LUA_BINDING_NAME(%s));\n", p.name.data(), p.name.data());
            }
            fprintf(f, "}\n");
            fclose(f);
        }
    }
}

// Assumed to run in home directory.
const char* scan_for_bindings_in[] = {
    "./src/v2_lua_bindings.cpp",
#if 1
    "./src/particle_system_lua_bindings.cpp",
    "./src/game_lua_bindings.cpp",
    "./src/entity_lua_bindings.cpp"
#endif

    // NOTE: the action mapper is not in the correct documentation format
    // yet.
};

int main(int argc, char** argv) {
    for (size_t index = 0; index < array_count(scan_for_bindings_in); ++index) {
        auto fname = scan_for_bindings_in[index];
        auto module = read_lua_game_module(fname);

        if (module.name.size() == 0) {
            fprintf(stderr, "%s has no modules to scan.\n", fname);
        } else {
            fprintf(stderr, "Found module: %s with (%d procs)\n", module.name.data(), module.procedures.size());
            modules.push_back(module);
        }
    }

    generate_text_documentation("lua_engine_api.md");
    generate_code_binding_files();
    return 0;
}

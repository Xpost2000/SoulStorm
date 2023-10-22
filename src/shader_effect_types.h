#ifndef SHADER_EFFECT_TYPES_H
#define SHADER_EFFECT_TYPES_H

/*
    This is safer for defining game specific shader effects,
    and is probably how I'm intending to utilize the render command system.

    There will be some game-specific rendering commands.
*/
enum Shader_Effect_Type {
    SHADER_EFFECT_TYPE_NONE, // or the identity shader as confirmation...
};

static string shader_effect_type_strings[] = {
    string_literal("SHADER_EFFECT_TYPE_NONE"),
};

#endif

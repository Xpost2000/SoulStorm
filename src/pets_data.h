#ifndef PETS_DATA_H
#define PETS_DATA_H

Gameplay_Data_Pet_Information {
    string_literal("No Pet"),
    string_literal("Flying solo, the standard difficulty setting.\n\nAll vanilla!"),
    DIFFICULTY_NORMAL,
    MAX_BASE_TRIES,
    ATTACK_PATTERN_DEFAULT,
    BOMB_PATTERN_DEFAULT,
    1.0f,
    1.0f
},

Gameplay_Data_Pet_Information {
    string_literal("Cat"),
    string_literal("Nine lives, nine tries. Easiest difficulty setting.\n\nIf you're finding a stage difficult you can puss out!"),
    DIFFICULTY_NORMAL,
    9,
    ATTACK_PATTERN_DEFAULT,
    BOMB_PATTERN_DEFAULT,
    0.35f,
    1.00f
},

Gameplay_Data_Pet_Information {
    string_literal("Dog"),
    string_literal("Fast and energetic! Hard difficulty setting.\n\nMake sure you've had practice before trying this!"),
    DIFFICULTY_HARD,
    3,
    ATTACK_PATTERN_DEFAULT,
    BOMB_PATTERN_DEFAULT,
    1.5f,
    1.25f
},

Gameplay_Data_Pet_Information {
    string_literal("Goldfish"),
    string_literal("Experience the fragility of a fish out of water!\nIf you're really confident about a 1CC go ahead!\n\nMaximal score guaranteed!"),
    DIFFICULTY_HARDEST,
    1,
    ATTACK_PATTERN_DEFAULT,
    BOMB_PATTERN_DEFAULT,
    4.0f,
    0.875f
}

#endif

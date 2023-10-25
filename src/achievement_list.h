#ifndef ACHIEVEMENT_LIST_H
#define ACHIEVEMENT_LIST_H
// Since I'm still designing the game
// it's okay to change often.
// but please avoid changing it once the game
// is finalized!
{
        ACHIEVEMENT_ID_TEST_ACHIEVEMENT0,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_TEST_ACHIEVEMENT0"),
        string_literal("Test 1"),
        string_literal("Does the achievement system even work?"),
},
{
        ACHIEVEMENT_ID_STAGE1,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE1"),
        string_literal("From Nothing"),
        string_literal("Completed \'Limbo\'"),
},
{
        ACHIEVEMENT_ID_STAGE2,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE2"),
        string_literal("From Flames"),
        string_literal("Completed \'Firey Gates\'"),
},
{
        ACHIEVEMENT_ID_STAGE3,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE3"),
        string_literal("Into Flames"),
        string_literal("Completed \'Inferno\'"),
},
{
        ACHIEVEMENT_ID_STAGE4,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        1, 0,
        string_literal("ACHIEVEMENT_ID_STAGE4"),
        string_literal("Endless Ailment"),
        string_literal("Completed \'Insanity\'"),
},
{
        ACHIEVEMENT_ID_KILLER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        0, 0,
        string_literal("ACHIEVEMENT_ID_KILLER"),
        string_literal("Killer"),
        string_literal("Eliminated 1500 enemies cumulatively."),
},
{
        ACHIEVEMENT_ID_MURDERER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        1, 0,
        string_literal("ACHIEVEMENT_ID_MURDERER"),
        string_literal("Murderer"),
        string_literal("Eliminated 5000 enemies cumulatively."),
},
{
        ACHIEVEMENT_ID_SLAYER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        1, 0,
        string_literal("ACHIEVEMENT_ID_SLAYER"),
        string_literal("Slayer"),
        string_literal("Eliminated 10000 enemies cumulatively."),
},
{
        ACHIEVEMENT_ID_PLATINUM,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        1, 0,
        string_literal("ACHIEVEMENT_ID_PLATINUM"),
        string_literal("True Absolution"),
        string_literal("Completed every other achievement. Congratulations."),
}

#endif

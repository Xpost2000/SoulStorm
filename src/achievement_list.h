#ifndef ACHIEVEMENT_LIST_H
#define ACHIEVEMENT_LIST_H
// Since I'm still designing the game
// it's okay to change often.
// but please avoid changing it once the game
// is finalized!
{
        ACHIEVEMENT_ID_STAGE1,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE1"),
        string_literal("Witch-hunter"),
        string_literal("Completed Stage 1"),
},
{
        ACHIEVEMENT_ID_STAGE2,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE2"),
        string_literal("Alpha Male"),
        string_literal("Completed Stage 2"),
},
{
        ACHIEVEMENT_ID_STAGE3,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE3"),
        string_literal("The Biggest Fish"),
        string_literal("Completed Stage 3"),
},

{
        ACHIEVEMENT_ID_STAGE1_FLAWLESS,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE1_FLAWLESS"),
        string_literal("Superstitious"),
        string_literal("Completed all of Stage 1 without dying!"),
},
{
        ACHIEVEMENT_ID_STAGE2_FLAWLESS,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE2_FLAWLESS"),
        string_literal("Evolution"),
        string_literal("Completed all of Stage 2 without dying!"),
},
{
        ACHIEVEMENT_ID_STAGE3_FLAWLESS,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_STAGE3_FLAWLESS"),
        string_literal("Apex Predator"),
        string_literal("Completed all of Stage 3 without dying!"),
},
{
        ACHIEVEMENT_ID_UNTOUCHABLE,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        1, 0,
        string_literal("ACHIEVEMENT_ID_UNTOUCHABLE"),
        string_literal("Untouchable"),
        string_literal("Completed all stages without dying! What a pro!"),
},
{
        ACHIEVEMENT_ID_KILLER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        1, 0,
        string_literal("ACHIEVEMENT_ID_KILLER"),
        string_literal("Fighter"),
        string_literal("Eliminated 1500 enemies cumulatively."),

        {{0, 1500}}
},
{
        ACHIEVEMENT_ID_MURDERER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        1, 0,
        string_literal("ACHIEVEMENT_ID_MURDERER"),
        string_literal("Survivor"),
        string_literal("Eliminated 5000 enemies cumulatively."),

        {{0, 5000}}
},
{
        ACHIEVEMENT_ID_SLAYER,
        ACHIEVEMENT_PROGRESS_TYPE_INT,
        1, 0,
        string_literal("ACHIEVEMENT_ID_SLAYER"),
        string_literal("Battle Hardened"),
        string_literal("Eliminated 10000 enemies cumulatively."),

        {{0, 10000}}
},
{
        ACHIEVEMENT_ID_PLATINUM,
        ACHIEVEMENT_PROGRESS_TYPE_BOOL,
        0, 0,
        string_literal("ACHIEVEMENT_ID_PLATINUM"),
        string_literal("Little Big Hero"),
        string_literal("Completed every achievement. Congratulations!"),
}

#endif

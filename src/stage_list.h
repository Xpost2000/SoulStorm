#ifndef STAGE_LIST_H
#define STAGE_LIST_H
// Stage 1
// easy and slow themed.
    Stage {
        string_literal("Witch's Playpen"),
            string_literal("Magical wonderland! Or not?"),
            1,
            {
                Level {
                    string_literal("Reception Room"),
                        string_literal("Wait in line, just like everyone else!"),
                        0,
                        },
                Level {
                    string_literal("Play Room"),
                        string_literal("Aren't you just entertained?"),
                        0,
                        },
                Level {
                    string_literal("Witch"),
                        string_literal("She's gonna put a spell on you!"),
                        1,
                },
            }
    },
    
    // Stage 2
    // this is "normal"
    Stage {
        string_literal("The Doghouse"),
            string_literal("Where the goodest of boys belong."),
            1,
            {
                Level {
                    string_literal("River Styx"),
                        string_literal("Try not to forget the way out!"),
                        0
                        },
                Level {
                    string_literal("Backyard"),
                        string_literal("Play fetch with bullets!"),
                        0,
                        },
                Level {
                    string_literal("Cerberus"),
                        string_literal("Three heads is better than one!"),
                        1,
                        },
                    }
    },

    // Stage 3
    // this is "hard"
    Stage {
        string_literal("The Deep Sea"),
            string_literal("A big space for big fishies."),
            1,
            {
                Level {
                    string_literal("School"),
                        string_literal("The fish kind."),
                        0
                        },
                Level {
                    string_literal("Aquarium"),
                        string_literal(""),
                        0,
                        },
                Level {
                    string_literal("Apex Predator"),
                        string_literal("The real top of the foodchain."),
                        1,
                        },
                    }
    },

    // Stage 4: Bonus
    // This is going to be a bonus stage that honestly I'm not sure if
    // I can design to be beatable since I'm not a pro LOL.
    // NOTE: all stages are boss stages.
    Stage {
        string_literal("Insanity"),
            string_literal("Battling away for eternity."),
            1,
            {
                Level {
                    string_literal("Pride"),
                        string_literal("Where it begins."),
                        1
                        },
                Level {
                    string_literal("Hubris"),
                        string_literal("Where it grows."),
                        1,
                        },
                Level {
                    string_literal("Arrogance"),
                        string_literal("Where it ends."),
                        1,
                        },
                    }
    },

#endif

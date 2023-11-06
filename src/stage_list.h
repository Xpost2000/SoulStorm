#ifndef STAGE_LIST_H
#define STAGE_LIST_H
// Stage 1
// easy and slow themed.
    Stage {
        string_literal("Limbo"),
            string_literal("The Endless Nothing"),
            1,
            {
                Level {
                    string_literal("Gates of Eternity"),
                        string_literal("The beginnings of escape."),
                        0,
                        },
                Level {
                    string_literal("Empty Sea"),
                        string_literal("The path of wayward souls."),
                        0,
                        },
                Level {
                    string_literal("Reaper's Gate"),
                        string_literal("Judgement awaits."),
                        1,
                },
            }
    },
    
    // Stage 2
    // this is "normal"
    Stage {
        string_literal("Fiery Gates"),
            string_literal("Lost Paradise"),
            1,
            {
                Level {
                    string_literal("River Styx"),
                        string_literal("Washing away with forgetfulness."),
                        0
                        },
                Level {
                    string_literal("Fiery Sojourn"),
                        string_literal("Only embers will remain."),
                        0,
                        },
                Level {
                    string_literal("Cerberus"),
                        string_literal("The Loyal Gatekeeper"),
                        1,
                        },
                    }
    },

    // Stage 3
    // this is "hard"
    Stage {
        string_literal("Inferno"),
            string_literal("A place for the sinful."),
            1,
            {
                Level {
                    string_literal("Greed"),
                        string_literal("For those who want too much."),
                        0
                        },
                Level {
                    string_literal("Heresy"),
                        string_literal("For those who don't believe."),
                        0,
                        },
                Level {
                    string_literal("Treachery"),
                        string_literal("For those who betray themselves and everyone else."),
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

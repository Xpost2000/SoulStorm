#ifndef GAME_UI_H
#define GAME_UI_H

#include "common.h"
#include "render_commands.h"
#include "memory_arena.h"

/*
  This is an IMGUI interface that will be used for the ingame menus.

  Layout is expected to be done manually since there's little to no menus in this
  game.

  I would like there to be polish such as having widgets fade in or something,
  but that's for the future.

  Although for UI it's kind of okay if it just pops in...

  Maybe something nice like getting a little bigger on hover on?

  Should allow for things like
      - Disabling behavior (so that way I can do UI transitions)
      - Transition behavior (this will be hard coded, but it's for polish)
      - A few more widgets (mostly enough to facilitate the settings menu)
*/
namespace GameUI {
    void set_font_selected(font_cache* font);
    void set_font_active(font_cache* font);
    void set_font(font_cache* font);

    void label(V2 where, string text, color32f32 modulation, f32 scale);
    bool button(V2 where, string text, color32f32 modulation, f32 scale);

    void initialize(Memory_Arena* arena);
    void begin_frame(struct render_commands* commands);
    void end_frame();
    void update(f32 dt);
}

#endif

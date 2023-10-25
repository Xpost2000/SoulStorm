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
      - Transition behavior (this will be hard coded, but it's for polish)
      - A few more widgets (mostly enough to facilitate the settings menu,
                            since this game deliberately avoids UI)
*/
enum Widget_Action {
    WIDGET_ACTION_NONE     = 0,
    WIDGET_ACTION_ACTIVATE = 1,
    // like a mouse over
    WIDGET_ACTION_HOT      = 2,
};
namespace GameUI {
    void set_font_selected(font_cache* font);
    void set_font_active(font_cache* font);
    void set_font(font_cache* font);

    void label(V2 where, string text, color32f32 modulation, f32 scale, bool active=true);
    void option_selector(V2 where, string text, color32f32 modulation, f32 scale, string* options, s32 options_count, s32* out_selected, bool active=true);

    // need to enhance this beyond boolean only
    // need to know if I'm "focused"
    s32 button(V2 where, string text, color32f32 modulation, f32 scale, bool active=true);

    // checkboxes will return the value of their "check"
    bool checkbox(V2 where, string text, color32f32 modulation, f32 scale, bool* ptr, bool active=true);
    void f32_slider(V2 where, string text, color32f32 modulation, f32 scale, f32* ptr, f32 min_value, f32 max_value, f32 slider_width_px, bool active=true);

    void initialize(Memory_Arena* arena);
    void begin_frame(struct render_commands* commands);
    void end_frame();
    void update(f32 dt);
}

#endif

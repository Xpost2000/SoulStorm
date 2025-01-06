#ifndef GAME_UI_H
#define GAME_UI_H

#include "common.h"
#include "render_commands.h"
#include "graphics_assets.h"
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

struct GameUI_Ninepatch {
    image_id top_left;
    image_id top_right;
    image_id bottom_left;
    image_id bottom_right;
    image_id left;
    image_id right;
    image_id center;
    image_id bottom;
    image_id top;
    u32 tile_width;
    u32 tile_height; // nothing enforced, but this is basically only going to work if the tiles are square.
};

/*
  Since this is an IMGUI in the simplest sense, I don't store
  previous UI screen state (usually you'd use a stack to do this, but this game
  just doesn't need that advanced of a UI unlike Legends, which just had completely
  hand-coded UI.)
*/

namespace GameUI {
    void set_font_selected(font_cache* font);
    void set_font_active(font_cache* font);
    void set_font(font_cache* font);

    void set_all_visual_alpha(f32 alpha); // does not affect the "active state" of any buttons.
    void reset_alpha(void);
    f32  get_visual_alpha(void);

    void label(V2 where, string text, color32f32 modulation, f32 scale, bool active=true);
    void option_selector(V2 where, string text, color32f32 modulation, f32 scale, string* options, s32 options_count, s32* out_selected, bool active=true);

    // need to enhance this beyond boolean only
    // need to know if I'm "focused"
    s32 button(V2 where, string text, color32f32 modulation, f32 scale, bool active=true);

    // checkboxes will return the value of their "check"
    bool checkbox(V2 where, string text, color32f32 modulation, f32 scale, bool* ptr, bool active=true);
    void f32_slider(V2 where, string text, color32f32 modulation, f32 scale, f32* ptr, f32 min_value, f32 max_value, f32 slider_width_px, bool active=true);

    void initialize(Memory_Arena* arena);
    void begin_frame(struct render_commands* commands, struct graphics_assets* assets);
    void end_frame();
    void update(f32 dt);

    /*
      NOTE: texture atlas is optional
     */
    V2   ninepatch_dimensions(const GameUI_Ninepatch& ninepatch, u32 width, u32 height);
    // NOTE: the four corners are "implicit", so you're really only counting the "centers"
    // so the ninepatch does not support anything "smaller" than (3x3) (width&height == 0)
    // this is fine because I'm not drawing any UI that small since it would be literally unreadable.
    void ninepatch(const GameUI_Ninepatch& ninepatch, V2 where, u32 width, u32 height, color32f32 modulation, f32 scale=1);
    void ninepatch(const Texture_Atlas* texture_atlas, const GameUI_Ninepatch& ninepatch, V2 where, u32 width, u32 height, color32f32 modulation, f32 scale=1);

    /*
      NOTE:
      I'm just going to take a page from Eskil Steenberg's book, and take advantage
      of the fact that string pointers must be unique, so I can just use them as IDs.

      I kind of need them in order to act as a "hash" to figure out if I should reset
      the selected_widget value from a controller.
     */
    void set_ui_id(char* id_string);

    // do I need special logic to go across "multi" element UIs like the option selector which
    // is composed of multiple selectable widgets?
    void move_selected_widget_id(s32 increments);
}

#endif

// NOTE: these should be exclusively render commands in the future. I just don't do it quite yet
// but it would help for rendering in OpenGL, and I could add game specific render commands later...
#include "game.h"

static string menu_font_variation_string_names[] = {
    string_literal("res/fonts/gnsh-bitmapfont-colour1.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour2.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour3.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour4.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour5.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour6.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour7.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour8.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour9.png"),
    string_literal("res/fonts/gnsh-bitmapfont-colour10.png"),
};
/* using GNSH fonts, which are public domain, but credits to open game art, this font looks cool */
enum menu_font_variation {
    MENU_FONT_COLOR_GOLD,

    MENU_FONT_COLOR_ORANGE,
    MENU_FONT_COLOR_LIME,
    MENU_FONT_COLOR_SKYBLUE,
    MENU_FONT_COLOR_PURPLE,
    MENU_FONT_COLOR_BLUE,
    MENU_FONT_COLOR_STEEL,
    MENU_FONT_COLOR_WHITE,
    MENU_FONT_COLOR_YELLOW,
    MENU_FONT_COLOR_BLOODRED,

    /* I want room to have more fonts though, although GNSH fonts are very very nice. */
    MENU_FONT_COUNT,
};

// this is where the actual member variables of the game go
// the class is just a small wrapper for this stuff
struct Game_State {
};


struct Game_Resources {
    graphics_assets graphics_assets;
    font_id         menu_fonts[MENU_FONT_COUNT];

    font_cache* get_font(s32 variation) {
        struct font_cache* font = graphics_assets_get_font_by_id(&graphics_assets, menu_fonts[variation]);
        return font;
    }
};

Game::Game() {
    
}

Game::~Game() {
    
}

void Game::init() {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state     = (Game_State*)arena->push_unaligned(sizeof(*this->state));

    resources->graphics_assets   = graphics_assets_create(arena, 16, 256);
    for (unsigned index = 0; index < array_count(menu_font_variation_string_names); ++index) {
        string current = menu_font_variation_string_names[index];
        resources->menu_fonts[index] = graphics_assets_load_bitmap_font(&resources->graphics_assets, current, 5, 12, 5, 20);
    }
}

void Game::deinit() {
    
}

void Game::update_and_render(software_framebuffer* framebuffer, f32 dt) {
    software_framebuffer_clear_scissor(framebuffer);
    software_framebuffer_clear_buffer(framebuffer, color32u8(255, 255, 255, 255));
    software_framebuffer_draw_quad(framebuffer, rectangle_f32(100, 100, 100, 100), color32u8(0, 255, 0, 255), BLEND_MODE_ALPHA);

    software_framebuffer_draw_text(framebuffer, resources->get_font(MENU_FONT_COLOR_BLOODRED), 2, V2(100, 100), string_literal("I am a brave new world"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
}

// NOTE: these should be exclusively render commands in the future. I just don't do it quite yet
// but it would help for rendering in OpenGL, and I could add game specific render commands later...

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"

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

    state->player.position = V2(320 - 30, 240 - 30);
    state->player.scale    = V2(30, 30);
    state->bullets         = Fixed_Array<Bullet>(arena, 10000);
}

void Game::deinit() {
    
}

/*
 * some testing things
 */

void spawn_bullet_circling_down_homing(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(10, 10);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->player.velocity;
        };

    state->bullets.push(bullet);
}

void spawn_bullet_circling_down_homing2(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(20, 20);

    Timer until_release(2.0f);
    bool triggered = false;
    f32 t_hit = 0;

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) mutable {
            until_release.start();
            if (until_release.triggered()) {
                triggered = true;
                t_hit = self->t_since_spawn;
            }
            until_release.update(dt);

            if (triggered)
                self->velocity = additional + (additional.normalized() * 50 * self->t_since_spawn - t_hit);
            else
                self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->player.velocity;
        };

    state->bullets.push(bullet);
}

void spawn_bullet_circling_down(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(20, 20);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            f32 t_extra = self->t_since_spawn;
            f32 t = (factor + t_extra);
            self->velocity = V2(-sinf(t) * factor2,
                                cos(t) * factor2) + additional;
        };

    state->bullets.push(bullet);
}

void spawn_bullet_linear(Game_State* state, V2 position, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(20, 20);
    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = additional;
        };

    state->bullets.push(bullet);
}

void Game::update_and_render(software_framebuffer* framebuffer, f32 dt) {
    software_framebuffer_clear_scissor(framebuffer);
    software_framebuffer_clear_buffer(framebuffer, color32u8(255, 255, 255, 255));
    software_framebuffer_draw_quad(framebuffer, rectangle_f32(100, 100, 100, 100), color32u8(0, 255, 0, 255), BLEND_MODE_ALPHA);

    software_framebuffer_draw_text(framebuffer, resources->get_font(MENU_FONT_COLOR_BLOODRED), 2, V2(100, 100), string_literal("I am a brave new world"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);

    state->player.update(state, dt);

    if (Input::is_key_pressed(KEY_T)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down(state, position, t, r, V2(0, 60));
        }
    }
    if (Input::is_key_pressed(KEY_U)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down_homing(state, position, t, r, V2(0, 0));
        }
    }
    if (Input::is_key_pressed(KEY_I)) {
        int amount = 10;
        int r = 50;
        for (int i = 0; i < amount; ++i) {
            f32 t = degree_to_radians(i * (360.0f/amount));
            V2 position = V2(cosf(t) * r, sinf(t) * r) + state->player.position;
            spawn_bullet_circling_down_homing2(state, position, t, r, V2(0, 100));
        }
    }
    if (Input::is_key_pressed(KEY_Y)) {
        int amount = 5;
        for (int i = 0; i < amount; ++i) {
            V2 position = V2(i * 40, 0) + state->player.position;
            // spawn_bullet_circling_down(state, position, t, 150, V2(0, 60));
            spawn_bullet_linear(state, position, V2(0, 150));
        }
    }
    if (Input::is_key_pressed(KEY_P)) {
        int amount = 250;
        int r = 500;
        f32 step = r / (amount/2);
        int ll = 0;
        for (int i = 0; i < amount; ++i) {
            if (ll > 0) {
                ll--;
                continue;
            }
            // spawn_bullet_circling_down(state, position, t, 150, V2(0, 60));
            f32 t = degree_to_radians(i * (360.0f/amount));
            f32 ner = r - (step) * i;
            if ((i % 5) == 0) {
                ll = 15;

            }
            if (ner <= 0) continue;
            V2 position = V2(cosf(t) * ner, sinf(t) * ner) + state->player.position;
            spawn_bullet_circling_down_homing(state, position, t, ner, V2(0, 0));
        }
    }

    for (int i = 0; i < (int)state->bullets.size; ++i) {
        auto& b = state->bullets[i];
        b.update(state, dt);
        b.draw(framebuffer);
    }

    state->player.draw(framebuffer);
}

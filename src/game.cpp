// NOTE: these should be exclusively render commands in the future. I just don't do it quite yet
// but it would help for rendering in OpenGL, and I could add game specific render commands later...

// NOTE: game units are in 640x480 pixels now.
#include "game.h"
#include "entity.h"
#include "fixed_array.h"
#include "game_state.h"

Game::Game() {
    
}

Game::~Game() {
    
}

void Game::init() {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state = (Game_State*)arena->push_unaligned(sizeof(*this->state)); (new (this->state) Game_State);

    resources->graphics_assets   = graphics_assets_create(arena, 16, 256);
    for (unsigned index = 0; index < array_count(menu_font_variation_string_names); ++index) {
        string current = menu_font_variation_string_names[index];
        resources->menu_fonts[index] = graphics_assets_load_bitmap_font(&resources->graphics_assets, current, 5, 12, 5, 20);
    }

    resources->circle = graphics_assets_load_image(&resources->graphics_assets, string_literal("res/img/circle256.png"));

    state->player.position   = V2(state->play_area.width / 2, 300);
    state->player.scale      = V2(15, 15);

    state->bullets           = Fixed_Array<Bullet>(arena, 10000);
    state->explosion_hazards = Fixed_Array<Explosion_Hazard>(arena, 256);
    state->laser_hazards     = Fixed_Array<Laser_Hazard>(arena, 128);
}

void Game::deinit() {
    
}

/*
 * some testing things
 */

void spawn_bullet_circling_down_homing(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);

    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = V2(-sinf(self->t_since_spawn + factor) * factor2, cos(self->t_since_spawn + factor) * factor2) + state->player.velocity;
        };

    state->bullets.push(bullet);
}

void spawn_bullet_circling_down_homing2(Game_State* state, V2 position, f32 factor, f32 factor2, V2 additional = V2(0,0)) {
    Bullet bullet;
    bullet.position = position;
    bullet.scale    = V2(5,5);

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
    bullet.scale    = V2(5,5);

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
    bullet.scale    = V2(5,5);
    bullet.velocity_function =
        [=](Bullet* self, Game_State* const state, f32 dt) {
            self->velocity = additional;
        };

    state->bullets.push(bullet);
}

void Game::update_and_render(software_framebuffer* framebuffer, f32 dt) {
    {
        state->play_area.x = framebuffer->width / 2 - state->play_area.width / 2;
        state->play_area.height = framebuffer->height;

        // state->play_area.set_all_edge_behaviors_to(PLAY_AREA_EDGE_WRAPPING);
        state->play_area.set_all_edge_behaviors_to(PLAY_AREA_EDGE_DEADLY);
    }


    software_framebuffer_clear_scissor(framebuffer);
    software_framebuffer_clear_buffer(framebuffer, color32u8(255, 255, 255, 255));

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

    if (Input::is_key_pressed(KEY_X)) {
        Explosion_Hazard h = Explosion_Hazard(state->player.position, 200, 0.5f, 1.0f);
        state->explosion_hazards.push(h);
    }
    if (Input::is_key_pressed(KEY_C)) {
        Laser_Hazard h = Laser_Hazard(state->player.position.y, 30.0f, LASER_HAZARD_DIRECTION_HORIZONTAL, 1.0f, 15.0f);
        state->laser_hazards.push(h);
    }

    if (Input::is_key_pressed(KEY_V)) {
        Laser_Hazard h = Laser_Hazard(state->player.position.x, 30.0f, LASER_HAZARD_DIRECTION_VERTICAL, 1.0f, 15.0f);
        state->laser_hazards.push(h);
    }

    for (int i = 0; i < (int)state->bullets.size; ++i) {
        auto& b = state->bullets[i];
        b.update(state, dt);
        b.draw(state, framebuffer, resources);
    }

    for (int i = 0; i < (int)state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];
        h.update(state, dt);
        h.draw(state, framebuffer, resources);
    }

    for (int i = 0; i < (int)state->laser_hazards.size; ++i) {
        auto& h = state->laser_hazards[i];
        h.update(state, dt);
        h.draw(state, framebuffer, resources);
    }

    state->player.draw(state, framebuffer, resources);

    // draw play area borders / Game UI
    // I'd like to have the UI fade in / animate all fancy like when I can
    {
        auto border_color = color32u8(128, 128, 128, 255);
 
        int play_area_width = state->play_area.width;
        int play_area_x     = state->play_area.x;

        // left border
        software_framebuffer_draw_quad(framebuffer,
                                       rectangle_f32(0,
                                                     0,
                                                     play_area_x,
                                                     framebuffer->height),
                                       border_color, BLEND_MODE_ALPHA);
        // right border
        software_framebuffer_draw_quad(framebuffer,
                                       rectangle_f32(play_area_x + play_area_width,
                                                     0,
                                                     framebuffer->width - play_area_width,
                                                     framebuffer->height),
                                       border_color, BLEND_MODE_ALPHA);
    }

    software_framebuffer_draw_quad(framebuffer, rectangle_f32(100, 100, 100, 100), color32u8(0, 255, 0, 255), BLEND_MODE_ALPHA);
    software_framebuffer_draw_text(framebuffer, resources->get_font(MENU_FONT_COLOR_BLOODRED), 2, V2(100, 100), string_literal("I am a brave new world"), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);

    handle_all_explosions(dt);
    handle_all_lasers(dt);
    handle_all_dead_entities(dt);
}

void Game::handle_all_dead_entities(f32 dt) {
    if (state->player.die) {
        Global_Engine()->die();
    }
}

void Game::handle_all_lasers(f32 dt) {
    for (int i = 0; i < state->laser_hazards.size; ++i) {
        auto& h = state->laser_hazards[i];

        if (h.ready()) {
            auto laser_rect  = h.get_rect(&state->play_area);
            auto player_rect = state->player.get_rect();

            if (rectangle_f32_intersect(player_rect, laser_rect)) {
                _debugprintf("Hi, I died.");
                state->player.die = true;
            }
        }

        if (h.die) {
            _debugprintf("bye bye laser");

            state->laser_hazards.pop_and_swap(i);
        }
    }
}

void Game::handle_all_explosions(f32 dt) {
    for (int i = 0; i < state->explosion_hazards.size; ++i) {
        auto& h = state->explosion_hazards[i];

        if (h.exploded) {
            _debugprintf("biggest boom ever.");
            // check explosion against all entities
            // by all entities, I just mean the player right now.
            {
                auto explosion_circle = circle_f32(h.position.x, h.position.y, h.radius);
                auto player_circle    = circle_f32(state->player.position.x, state->player.position.y, state->player.scale.x);

                if (circle_f32_intersect(explosion_circle, player_circle)) {
                    _debugprintf("Hi, I died.");
                    state->player.die = true;
                }
            }
            state->explosion_hazards.pop_and_swap(i);
        }
    }
}

// Play_Area
bool Play_Area::is_inside_absolute(rectangle_f32 rect) {
    return (rectangle_f32_intersect(rect, rectangle_f32(x, 0, width, height)));
}

bool Play_Area::is_inside_logical(rectangle_f32 rect) {
    return (rectangle_f32_intersect(rect, rectangle_f32(0, 0, width, height)));
}

void Play_Area::set_all_edge_behaviors_to(u8 value) {
    for (int i = 0; i < 4; ++i) {
        edge_behaviors[i] = value;
    }
}

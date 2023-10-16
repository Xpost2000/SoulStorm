#include "game.h"

// this is where the actual member variables of the game go
// the class is just a small wrapper for this stuff
struct Game_State {
    f32 test_timer = 0.0f;
    int color = 0;
};

struct Game_Resources {
    
};

Game::Game() {
    
}

Game::~Game() {
    
}

void Game::init() {
    this->arena     = &Global_Engine()->main_arena;
    this->resources = (Game_Resources*)arena->push_unaligned(sizeof(*this->resources));
    this->state     = (Game_State*)arena->push_unaligned(sizeof(*this->state));
}

void Game::deinit() {
    
}

void Game::update_and_render(software_framebuffer* framebuffer, f32 dt) {
    software_framebuffer_clear_buffer(framebuffer, color32u8(255, 255, 255, 255));
    software_framebuffer_draw_quad(framebuffer, rectangle_f32(0, 0, 100, 100), color32u8(0, 255, 0, 255), BLEND_MODE_ALPHA);
}

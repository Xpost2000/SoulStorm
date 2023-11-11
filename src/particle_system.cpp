#include "particle_system.h"

V2 Particle_Emit_Shape::emit_position(random_state* prng) {
    return V2(0, 0);
}

// Particle Emitter
void Particle_Emitter::reset() {
    emissions = 0;
    emission_timer = 0;
    emission_delay_timer = 0;
}

void Particle_Emitter::update(Particle_Pool* pool, f32 dt) {
    
}

// Particle Pool
Particle_Pool::Particle_Pool(Memory_Arena* arena, s32 amount) {
}

void Particle_Pool::update(f32 dt) {
    
}

void Particle_Pool::draw(struct render_commands* commands, Game_Resources* resources) {
    
}

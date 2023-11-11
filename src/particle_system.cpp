#include "game_state.h"
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
    particles = Fixed_Array<Particle>(arena, amount);
}

void Particle_Pool::update(Game_State* state, f32 dt) {
    for (s32 particle_index = 0; particle_index < particles.size; ++particle_index) {
        auto& particle = particles[particle_index];

        if (particle.lifetime <= 0.0f) {
            particles.pop_and_swap(particle_index);
            continue;
        }

        V2 effective_acceleration = particle.acceleration;

        if (particle.use_attraction_point) {
            V2 direction_to_attraction_point =
                V2_direction(particle.position, particle.attraction_point);

            effective_acceleration += (direction_to_attraction_point * particle.attraction_force);
        }

        particle.sprite.animate(&state->resources->graphics_assets, dt, 0.045);
        particle.velocity += effective_acceleration * dt;
        particle.position += particle.velocity *      dt;
        particle.lifetime -= dt;
    }
}

void Particle_Pool::draw(struct render_commands* commands, Game_Resources* resources) {
    for (s32 particle_index = 0; particle_index < particles.size; ++particle_index) {
        auto& particle = particles[particle_index];

        color32f32 modulation = particle.modulation;
        modulation.a *= (particle.lifetime/particle.lifetime_max);

        auto sprite_object = graphics_get_sprite_by_id(&resources->graphics_assets, particle.sprite.id);
        auto sprite_frame  = sprite_get_frame(sprite_object, particle.sprite.frame);
        auto sprite_img    = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
        V2 sprite_image_size = V2(sprite_img->width, sprite_img->height);
        sprite_image_size.x *= particle.sprite.scale.x; sprite_image_size.y *= particle.sprite.scale.y;

        V2 sprite_position = V2(
            particle.position.x - particle.scale/2 + (particle.sprite.offset.x - sprite_image_size.x/2),
            particle.position.y - particle.scale/2 + (particle.sprite.offset.y - sprite_image_size.y/2)
        );

        render_commands_push_image(
            commands,
            sprite_img,
            rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y),
            sprite_frame->source_rect,
            modulation,
            0,
            BLEND_MODE_ALPHA
        );
    }
}

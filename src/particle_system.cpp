#include "game_state.h"
#include "particle_system.h"
#include "engine.h"

Particle_Emit_Shape::Particle_Emit_Shape() {type = 0;}

Particle_Emit_Shape particle_emit_shape_point(V2 point) {
    Particle_Emit_Shape result;
    result.type = PARTICLE_EMIT_SHAPE_POINT;
    result.point.position = point;
    return result;
}

Particle_Emit_Shape particle_emit_shape_circle(V2 center, f32 radius, bool filled) {
    Particle_Emit_Shape result;
    result.type          = PARTICLE_EMIT_SHAPE_CIRCLE;
    result.circle.center = center;
    result.circle.radius = radius;
    result.filled_shape  = filled;
    return result;
}

Particle_Emit_Shape particle_emit_shape_quad(V2 center, V2 half_lengths, bool filled) {
    Particle_Emit_Shape result;
    result.type                 = PARTICLE_EMIT_SHAPE_QUAD;
    result.quad.center          = center;
    result.quad.half_dimensions = half_lengths;
    result.filled_shape         = filled;
    return result;
}

Particle_Emit_Shape particle_emit_shape_line(V2 start, V2 end) {
    Particle_Emit_Shape result;
    result.type       = PARTICLE_EMIT_SHAPE_LINE;
    result.line.start = start;
    result.line.end   = end;
    return result;
}

V2 Particle_Emit_Shape::emit_position(random_state* prng) {
    switch (type) {
        case PARTICLE_EMIT_SHAPE_POINT: {
            return point.position;
        } break;
        case PARTICLE_EMIT_SHAPE_CIRCLE: {
            // NOTE: apparently this doesn't distribute
            // the points well enough, but I don't particularly mind.
            if (filled_shape) {
                V2 radial_point =
                    V2_direction_from_degree(random_ranged_float(prng, 0.0f, 360.0f));
                return (circle.center + radial_point * random_ranged_float(prng, -circle.radius, circle.radius));
            } else {
                V2 circumference_point =
                    V2_direction_from_degree(random_ranged_float(prng, 0.0f, 360.0f));
                return (circle.center + circumference_point * circle.radius);
            }
        } break;
        case PARTICLE_EMIT_SHAPE_QUAD: {
            if (filled_shape) {
                f32 length_x = random_ranged_float(prng, -quad.half_dimensions.x, quad.half_dimensions.x);
                f32 length_y = random_ranged_float(prng, -quad.half_dimensions.y, quad.half_dimensions.y);

                return quad.center + V2(length_x, length_y);
            } else {
                s32 side = random_ranged_integer(prng, 0, 4);
                f32 length_x = random_ranged_float(prng, -quad.half_dimensions.x, quad.half_dimensions.x);
                f32 length_y = random_ranged_float(prng, -quad.half_dimensions.y, quad.half_dimensions.y);

                switch (side) {
                    case 0: {
                        return quad.center + V2(-quad.half_dimensions.x, length_y);
                    } break;
                    case 1: {
                        return quad.center + V2(quad.half_dimensions.x, length_y);
                    } break;
                    case 2: {
                        return quad.center + V2(length_x, -quad.half_dimensions.x);
                    } break;
                    default:
                    case 3: {
                        return quad.center + V2(length_x,  quad.half_dimensions.x);
                    } break;
                }
            }
        } break;
        case PARTICLE_EMIT_SHAPE_LINE: {
            V2 line_delta = (line.end - line.start);
            f32 t = random_ranged_float(prng, 0.0f, 1.0f);
            return line.start + line_delta * t;
        } break;
    }
    return V2(0, 0);
}

// Particle Emitter
void Particle_Emitter::reset() {
    emissions = 0;
    emission_timer = 0;
}

void Particle_Emitter::update(Particle_Pool* pool, random_state* prng, f32 dt) {
    if (!(flags & PARTICLE_EMITTER_FLAGS_ACTIVE))
        return;

    if (max_emissions != -1 && emissions >= max_emissions) {
        flags &= ~(PARTICLE_EMITTER_FLAGS_ACTIVE);
        return;
    }

    if (emission_timer <= 0.0f) {
        for (s32 index = 0; index < emit_per_emission; ++index) {
            if (pool->particles.size >= pool->particles.capacity) break;
            auto p = pool->particles.alloc_unimportant();

            p->position = shape.emit_position(prng);
            {
                p->sprite       = sprite;
                p->scale        = scale + random_ranged_float(prng, scale_variance.x, scale_variance.y);
                if (flags & PARTICLE_EMITTER_FLAGS_USE_ANGULAR) {
                    V2 direction =
                        V2_direction_from_degree(random_ranged_float(prng, angle_range.x, angle_range.y));
                    p->velocity     =
                        direction * (velocity.x + random_ranged_float(prng, velocity_x_variance.x, velocity_x_variance.y));
                    p->acceleration     =
                        direction * (acceleration.x + random_ranged_float(prng, acceleration_y_variance.x, acceleration_y_variance.y));
                } else {
                    p->velocity     =
                        velocity + V2(
                            random_ranged_float(prng, velocity_x_variance.x, velocity_x_variance.y),
                            random_ranged_float(prng, velocity_y_variance.x, velocity_y_variance.y)
                        );
                    p->acceleration =
                        acceleration + V2(
                            random_ranged_float(prng, acceleration_x_variance.x, acceleration_x_variance.y),
                            random_ranged_float(prng, acceleration_y_variance.x, acceleration_y_variance.y)
                        );
                }
                p->modulation   = color32f32_to_color32u8(modulation);
                p->lifetime     = p->lifetime_max = lifetime + random_ranged_float(prng, lifetime_variance.x, lifetime_variance.y);
                p->blend_mode   = blend_mode;

                if (flags & PARTICLE_EMITTER_FLAGS_USE_ATTRACTION_POINT) {
                    p->flags |= PARTICLE_FLAGS_USE_ATTRACTION_POINT;
                }
                p->attraction_point = attraction_point;
                p->attraction_force = attraction_force;

                if (flags & PARTICLE_EMITTER_FLAGS_USE_COLOR_FADE) {
                    p->target_modulation = color32f32_to_color32u8(target_modulation);
                } else {
                    p->target_modulation = color32f32_to_color32u8(modulation);
                }

                if (flags & PARTICLE_EMITTER_FLAGS_USE_FLAME_MODE) {
                    p->flags |= PARTICLE_FLAGS_USE_FLAME_MODE;
                }
            }
        }

        emissions += 1;
        emission_timer = emission_max_timer;
    } else {
        emission_timer -= dt;
    }
}

// Particle Pool

// NOTE:
// This should be easy way to get some vectorization in...
void Particle_Pool::init(Memory_Arena* arena, s32 amount) {
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

        if (particle.flags & PARTICLE_FLAGS_USE_ATTRACTION_POINT) {
            V2 direction_to_attraction_point =
                V2_direction(particle.position, particle.attraction_point);

            effective_acceleration += (direction_to_attraction_point * particle.attraction_force);
        }

        particle.sprite.animate(&state->resources->graphics_assets, dt, 0.045);

        if (V2_distance_sq(particle.position, particle.attraction_point) < 15 * 15) {
          effective_acceleration = V2(0, 0);
          particle.position = particle.attraction_point;
        } 

        particle.velocity += effective_acceleration * dt;
        particle.position += particle.velocity *      dt;
        particle.lifetime -= dt;
    }
}

/*
  The render_command system was designed for Legends which was 100% software rendered, and
  thus did not suffer from it's lack of batching. The sprite-atlas system was bolted on for
  manual sprite-batching, but it only works on per-image level.

  This is really the only time the game tries to render masses of different blend-modes in
  congregation so manual batching is done here.

  This makes the visual artifacts change a "little", but shouldn't be that bad. Alpha first, additive
  after.
*/
struct Particle_Pool_Draw_Batch {
  rectangle_f32 destination_rectangle;
  rectangle_f32 source_rectangle;
  color32f32    modulation;
};

static void draw_particle_render_batch(struct render_commands* commands, Game_Resources* resources, Fixed_Array<Particle_Pool_Draw_Batch>& batch, int blend_mode) {
  for (s32 index = 0; index < batch.size; ++index) {
    Particle_Pool_Draw_Batch& batchitem = batch[index];
    Texture_Atlas& texture_atlas = resources->gameplay_texture_atlas;
    struct image_buffer* image = graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id);

    render_commands_push_image(
      commands,
      image,
      batchitem.destination_rectangle,
      batchitem.source_rectangle,
      batchitem.modulation,
      0,
      blend_mode
    );
  }
}

void Particle_Pool::draw(struct render_commands* commands, Game_Resources* resources) {
    Fixed_Array<Particle_Pool_Draw_Batch> alpha_batch    = Fixed_Array<Particle_Pool_Draw_Batch>(&Global_Engine()->scratch_arena, particles.size);
    Fixed_Array<Particle_Pool_Draw_Batch> additive_batch = Fixed_Array<Particle_Pool_Draw_Batch>(&Global_Engine()->scratch_arena, particles.size);

    for (s32 particle_index = 0; particle_index < particles.size; ++particle_index) {
        auto& particle = particles[particle_index];

        color32f32 modulation = color32u8_to_color32f32(particle.modulation);
        color32f32 target_modulation = color32u8_to_color32f32(particle.target_modulation);
        f32 effective_t = clamp<f32>(particle.lifetime/particle.lifetime_max, 0.0f, 1.0f);

        {
            modulation.r = lerp_f32(modulation.r, target_modulation.r, 1.0f - effective_t);
            modulation.g = lerp_f32(modulation.g, target_modulation.g, 1.0f - effective_t);
            modulation.b = lerp_f32(modulation.b, target_modulation.b, 1.0f - effective_t);
        }

        modulation.a *= effective_t;

        auto sprite_object = graphics_get_sprite_by_id(&resources->graphics_assets, particle.sprite.id);
        auto sprite_frame  = sprite_get_frame(sprite_object, particle.sprite.frame);
        auto sprite_img    = graphics_assets_get_image_by_id(&resources->graphics_assets, sprite_frame->img);
        V2 sprite_image_size = V2(sprite_img->width, sprite_img->height);
        sprite_image_size.x *= particle.sprite.scale.x * particle.scale; sprite_image_size.y *= particle.sprite.scale.y * particle.scale;

        V2 sprite_position = V2(
            particle.position.x - particle.scale/2 + (particle.sprite.offset.x - sprite_image_size.x/2),
            particle.position.y - particle.scale/2 + (particle.sprite.offset.y - sprite_image_size.y/2)
        );

        {
            Texture_Atlas& texture_atlas = resources->gameplay_texture_atlas;

            struct rectangle_f32 source_rectangle = texture_atlas.get_subrect(sprite_frame->img, sprite_frame->source_rect);
            struct image_buffer* image = graphics_assets_get_image_by_id(&resources->graphics_assets, texture_atlas.atlas_image_id);

            if (particle.flags & PARTICLE_FLAGS_USE_FLAME_MODE) {
                Particle_Pool_Draw_Batch batchitem0;
                Particle_Pool_Draw_Batch batchitem1;

                batchitem0.destination_rectangle = batchitem1.destination_rectangle =
                  rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y);
                batchitem0.source_rectangle = batchitem1.source_rectangle =
                  source_rectangle;
                batchitem0.modulation = batchitem1.modulation = modulation;

                alpha_batch.push(batchitem0);
                additive_batch.push(batchitem1);
            } else {
                Particle_Pool_Draw_Batch batchitem;
                batchitem.destination_rectangle =
                  rectangle_f32(sprite_position.x, sprite_position.y, sprite_image_size.x, sprite_image_size.y);
                batchitem.source_rectangle = source_rectangle;
                batchitem.modulation = modulation;

                if (particle.blend_mode != BLEND_MODE_ADDITIVE) {
                  alpha_batch.push(batchitem);
                } else {
                  additive_batch.push(batchitem);
                }
            }
        }
    }
    
    draw_particle_render_batch(commands, resources, alpha_batch, BLEND_MODE_ALPHA);
    draw_particle_render_batch(commands, resources, additive_batch, BLEND_MODE_ADDITIVE);
}

void Particle_Pool::clear() {
    particles.zero();
}

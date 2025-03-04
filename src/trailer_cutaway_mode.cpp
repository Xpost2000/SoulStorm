#if COMPILE_IN_TRAILER_CLIPPING_CODE == 0
void Game::trailer_cutaway_data_initialize(Graphics_Driver* driver) { }
GAME_SCREEN(update_and_render_game_trailer_cutaway_screen) { }
#else
void Game::trailer_cutaway_data_initialize(Graphics_Driver* driver) {
    auto state = &this->state->titlescreen_data;
    auto resolution = V2(Global_Engine()->virtual_screen_width, Global_Engine()->virtual_screen_height);
    state->main_camera          = camera(V2(resolution.x/2, resolution.y/2), 1.0);
    state->main_camera.centered = true;
    state->main_camera.rng      = &state->prng;
    state->prng                 = random_state();

    // initializing all the stars' positions
    {
        auto& prng = state->prng;

        for (int i = 0; i < array_count(state->star_positions); ++i) {
            state->star_positions[i] = V2(random_ranged_float(&prng, -854, 854),
                                   random_ranged_float(&prng, -480, 480));
        }

        auto& sparkling_stars = state->sparkling_stars;
        for (int i = 0; i < array_count(state->sparkling_stars); ++i) {
            auto& star = sparkling_stars[i];
            star.visibility_delay_timer = star.max_visibility_delay_timer = random_ranged_float(&prng, 3.25f, 7.00f);
            star.anim_timer = 0.0f;
            star.frame_index = 0;
            star.position = V2(random_ranged_float(&prng, -800, 800), random_ranged_float(&prng, -480, 480));
            star.scale = random_ranged_float(&prng, 1.0f, 1.75f);
        }

        state->puppet.prng = &prng;
    }
}

GAME_SCREEN(update_and_render_game_trailer_cutaway_screen) {

}
#endif
#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "fixed_array.h"
#include "audio.h"
#include "input.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "graphics_driver.h"

#include "action_mapper.h"

#include "achievements.h"
#include "serializer.h"

#include "game_preferences.h"

#ifdef PORTABLE_INSTALL
#define PREF_PATH (string_literal("./")
#else
#define PREF_PATH (get_preference_directory(string_literal("xpostgames"), string_literal("solstorm")))
#endif

#define TICKRATE       (60)
#define FIXED_TICKTIME (1.0f / TICKRATE)
#ifdef _WIN32
#define DEFAULT_REPLAY_LOCATION (string_literal("\\replays\\"))
#else
#define DEFAULT_REPLAY_LOCATION (string_literal("/replays/"))
#endif
#define MAX_REPLAYS_PER_PAGE   (6)
local string save_file_name = string_literal("game_save.save");

string prefqpath(string path);
extern bool g_prefpath_portable;

// forward decl some opaque types
struct Game_State;
struct Game_Resources;
struct lua_State;

// since this is strictly animation only + the fact it
// does not affect any actual game state means I can just put this here.
enum Achievement_Notification_Phase {
    ACHIEVEMENT_NOTIFICATION_PHASE_APPEAR,
    ACHIEVEMENT_NOTIFICATION_PHASE_LINGER,
    ACHIEVEMENT_NOTIFICATION_PHASE_BYE,
};
struct Achievement_Notification {
    s32 phase;
    s32 id;
    f32 timer; // NOTE: Timer is in entity.h, but this is a simple thing so I'm not going to use it.
};
struct Achievement_Notification_State {
    Fixed_Array<Achievement_Notification> notifications;   
};

#include "save_data.h"

// TODO:
//
// I'd like to have a way to make more particle systems.
// that are preferably not hardcoded, or at least more flexible.
//
// I may just have a simple text file format through fscanf since it's
// not a big "deal" to user handle things.
//
// or have a very flat tokenizer to handle really really simple formats that
// I would keep in text or something like that.
//
// Since there's a lot of hardcoded data, and while some of it is justified
// something like the particles should obviously not require a re-compile.
//
// Might work on getting more stuff to be data oriented to be less painful.
// 
struct Entity_Loop_Update_Packet {
    Game_State* game_state;
    f32         dt;
};

#define GAME_UI_SCREEN(name) void name(struct render_commands* commands, f32 dt)
#define GAME_SCREEN(name) void name(struct render_commands* game_render_commands, struct render_commands* ui_render_commands, f32 dt)

enum Controls_Menu_Data_Binding_Slot {
    CONTROLS_MENU_DATA_BINDING_SLOT_KEY0,
    CONTROLS_MENU_DATA_BINDING_SLOT_KEY1,
    CONTROLS_MENU_DATA_BINDING_SLOT_GAMEPAD,
    CONTROLS_MENU_DATA_BINDING_SLOT_COUNT,
};

struct Controls_Menu_Data {
    // NOTE: in the case that a controls binding is activated from a key press, I want to
    // eat any key inputs for one frame so that the engine does not automatically take the key that was
    // just pressed as an input.

    // This is primarily because the engine does not have a real input layer system as it doesn't actually
    // need one for the actual game, so this is being done manually.
    s32              trying_to_bind_controls;
    Action_Data      temp_action_map[ACTION_COUNT];

    s32              action_id_to_bind;
    s32              action_id_to_bind_slot;
};

class Game {
public:
    Game();
    ~Game();

    void init(Graphics_Driver* driver);

    // NOTE: all **global/permenant** graphics data initialization is here
    // submodes can feel free to store handles to their own "private" stuff
    // if they want.
    void init_graphics_resources(Graphics_Driver* driver);
    void load_projectile_sprites(Graphics_Driver* driver, lua_State* L);
    void load_entity_sprites(Graphics_Driver* driver, lua_State* L);
    void init_audio_resources();
    void deinit();

    // Submode initialization
    void mainmenu_data_initialize(Graphics_Driver* driver);
    void opening_data_initialize(Graphics_Driver* driver);
    void ending_data_initialize(Graphics_Driver* driver);
    void title_data_initialize(Graphics_Driver* driver);
    void trailer_cutaway_data_initialize(Graphics_Driver* driver);

    void update_and_render(Graphics_Driver* driver, f32 dt);
    void handle_preferences(void);
    void on_resolution_change(int new_width, int new_height);

    Game_Preferences preferences;
    Game_State*     state;
    Game_Resources* resources;

    // used for writing into from the settings before I
    // actually apply them.
    Game_Preferences temp_preferences;
    Controls_Menu_Data control_menu_temp_data;
private: 
    void handle_ui_update_and_render(struct render_commands* commands, f32 dt);

    /*
      NOTE: this game / engine doesn't have a fully fledged UI system.

      It's just an IMGUI that's basically glued together, with these states
      to separate screens.

      This happens to be just fine when working with a controller though because
      you can't use a mouse to go through lots of stuff...
    */
    GAME_UI_SCREEN(update_and_render_replay_save_menu);
    GAME_UI_SCREEN(update_and_render_replay_collection_menu);
    GAME_UI_SCREEN(update_and_render_replay_not_supported_menu);
    GAME_UI_SCREEN(update_and_render_pause_menu);
    GAME_UI_SCREEN(update_and_render_options_menu);
    GAME_UI_SCREEN(update_and_render_renderer_change_disclaimer);
    GAME_UI_SCREEN(update_and_render_controls_menu);
    GAME_UI_SCREEN(update_and_render_review_script_error_menu);
    GAME_UI_SCREEN(update_and_render_stage_pet_select_menu);
    GAME_UI_SCREEN(update_and_render_stage_select_menu);
    GAME_UI_SCREEN(update_and_render_achievements_menu);
    GAME_UI_SCREEN(update_and_render_game_death_maybe_retry_menu);
    GAME_UI_SCREEN(update_and_render_achievement_notifications);
    GAME_UI_SCREEN(update_and_render_confirm_back_to_main_menu);
    GAME_UI_SCREEN(update_and_render_confirm_exit_to_windows);

    void check_and_unlock_perfect_clear_achievement(void);
    void check_and_unlock_platinum_achievement(void);

    void update_dialogue_speaker(f32 dt, s32 speaker_index);
    void render_dialogue_speaker(struct render_commands* commands, f32 dt, s32 speaker_index);

    void update_and_render_dialogue_speaker(struct render_commands* commands, f32 dt, s32 speaker_index);
    void update_and_render_dialogue_ui(struct render_commands* commands, f32 dt);

    // Game Modes
    GAME_SCREEN(update_and_render_game_opening);
    GAME_SCREEN(update_and_render_game_main_menu);
    GAME_SCREEN(update_and_render_game_ingame);
    GAME_SCREEN(update_and_render_game_credits);
    void       title_screen_replay_opening(void);
    GAME_SCREEN(update_and_render_game_title_screen);
    GAME_SCREEN(update_and_render_game_trailer_cutaway_screen);
    GAME_SCREEN(update_and_render_game_ending);

    void ingame_update_introduction_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt);
    void ingame_update_complete_stage_sequence(struct render_commands* commands, Game_Resources* resources, f32 dt);
    void ingame_update_complete_stage_sequence_player_animate_exit(f32 dt);

    // NOTE: most of these should be easy to parallesize.
    void handle_all_bullet_collisions(f32 dt);
    void handle_player_enemy_collisions(f32 dt);
    void handle_player_pickup_collisions(f32 dt);
    void handle_all_explosions(f32 dt);
    void handle_all_lasers(f32 dt);
    void handle_all_dead_entities(f32 dt);
    void cleanup_dead_entities(void);
    void handle_bomb_usage(f32 dt);

    /*
     * NOTE: primarily with the player as context
     *
     * kill_all_* is scoreless. Meant to just easily cleanup a wave.
     */
    void kill_all_bullets();
    void kill_all_enemies();
    void convert_bullets_to_score_pickups(float radius=99999, float value=100);
    void convert_enemies_to_score_pickups(float radius=99999, float value=100);

    void on_player_death();
    bool safely_resurrect_player();
    bool can_resurrect();

    void switch_screen(s32 screen);
    void switch_ui(s32 ui);
    bool can_access_stage(s32 id);

    void setup_stage_start();

    void reset_stage_simulation_state();
    void cleanup_game_simulation();
    void simulate_game_frames_until(int nth_frame);
    void simulate_game_frame(Entity_Loop_Update_Packet* update_params);

    void game_ui_draw_bordered_box(V2 where, s32 width, s32 height, color32f32 main_color = color32f32_DEFAULT_UI_COLOR, color32f32 border_color = color32f32_WHITE);
    void game_ui_draw_achievement_icon(const Achievement& achievement, struct render_commands* commands, V2 where, f32 scale, f32 alpha=1.0f);

    // TODO(jerry):
    // The theming is compiled in/hard-coded
    void game_ui_draw_title_logo(struct render_commands* commands, V2 where, f32 scale, f32 alpha);
    void game_ui_draw_title_part(struct render_commands* commands, V2 where, s8 partid, color32f32 fill_color, f32 scale, f32 alpha);

    // The game will only utilize one save file
    // and auto save so this makes so much way easier.
    // the save file format is not very complicated honestly, since it just
    // needs to basically record what levels you beat, and what score you got.
    bool save_game();
    bool load_game();

    Save_File construct_save_data();
    Save_File serialize_game_state(struct binary_serializer* serializer);
    void update_from_save_data(Save_File* save_data);

    void notify_new_achievement(s32 id);


    Memory_Arena*   arena;
    bool initialized = false;

    Achievement_Notification_State achievement_state;

    f32 total_playtime;
};

#endif

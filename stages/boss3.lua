-- final boss code, meant to be included in stage file

local track1 = load_music("res/snds/music_1_3_bossloop.ogg"); -- todo replace

enable_boss_death_explosion = false;
BOSS_HP = 8000;

boss_teleport_positions = {}; -- forward declaration
boss_state = {
    me,
    last_good_position, -- for if the entity is deleted for any reason
    starting_position,

    -- general boss logic state
    next_rain_attack_until = -9999,

    -- Attacking actions. Attack actions and movements
    -- are scheduled "simulatneously"
    next_think_action_t = -9999,
    next_grid_action_t = -9999,

    -- Not always moving, but sometimes move...
    next_possible_move_action_t = -9999,

    -- phase 1 : shadow play
    -- phase 2 : shadow WITCH
    -- phase 3 : shadow play
    -- phase 4 : shadow bone wings
    -- phase 5 : shadow play
    -- phase 6 : shadow boss 2
    -- phase 7..9 : more erratic
    phase=0,

    -- movement data
    wants_to_move=false,
    moving_to=false,
    move_target,
    move_method='teleport',
    move_speed, -- Velocity if not teleporting
    -- explosion speed is fixed size
};

function _Stage_Boss_MusicPlayer(e)
   while enemy_valid(e) do
      if (music_playing() == false) then
         play_music(track1);
      end
      t_yield();
   end
end

function Game_Spawn_Stage_Boss()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 50);
   enemy_set_hp(e, BOSS_HP); -- TODO for now
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BOSS3_FACE_FRONT);
   enemy_set_burst_gain_value(e, 0.55);
   enemy_show_boss_hp(e, "???");

   -- The boss takes "three threads" of logic.
   -- async_task_lambda(_Stage1_Boss_Logic, e);
   -- async_task_lambda(_Stage1_Boss_Movement_Logic, e);
   -- async_task_lambda(_Stage1_Boss_Maintain_Hexes_Logic, e);
   -- async_task_lambda(_Stage1_Boss_ImmunityToBurstLaser_Logic, e);
   async_task_lambda(_Stage_Boss_MusicPlayer, e);
   
   boss_state.me = e;
   boss_state.starting_position = initial_boss_pos;

   return e;
end
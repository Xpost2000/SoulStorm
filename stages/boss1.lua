-- All code relating to the first boss encounter
-- meant to be included in a stage file.

boss1_state = {

};

hexbind0_state = {

};

hexbind1_state = {

};

-- Per Frame Logic
function _Stage1_Boss_Logic(eid)
    while enemy_valid(eid) do 
        t_yield();
    end
    -- do some explosions and stuff for
    -- a dramatic death.
    --
    -- Stage end.
    --
    -- Bosses, don't really do anything special
    -- when the stage ends other than have a dramatic
    -- explosion (that is dangerous btw, so don't get complicant!)
    --
    -- Flashy, and deadly.
end

function _Stage1_Boss_HexBind0(eid)
end

function _Stage1_Boss_HexBind1(eid)
end

function Game_Spawn_Stage1_Boss()
   local e = enemy_new();
   local initial_boss_pos = v2(play_area_width()/2, 50);
   enemy_set_hp(e, 100);
   enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
   enemy_set_visual(e, ENTITY_SPRITE_BAT_A); -- for now...
   enemy_show_boss_hp(e, "WITCH");
   async_task_lambda(_Stage1_Boss_Logic, e);
   return e;
end

function Game_Spawn_Stage1_Boss_HexBind0()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2 + 50, 50);
    enemy_set_hp(e, 100);
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_visual(e, ENTITY_SPRITE_SKULL_A); -- for now...
    enemy_show_boss_hp(e, "HEX BINDING");
    async_task_lambda(_Stage1_Boss_HexBind0, e);
    return e;
 end
 
 function Game_Spawn_Stage1_Boss_HexBind1()
    local e = enemy_new();
    local initial_boss_pos = v2(play_area_width()/2 - 50, 50);
    enemy_set_hp(e, 100);
    enemy_set_position(e, initial_boss_pos[1], initial_boss_pos[2]);
    enemy_set_visual(e, ENTITY_SPRITE_SKULL_A); -- for now...
    enemy_show_boss_hp(e, "HEX BINDING");
    async_task_lambda(_Stage1_Boss_HexBind1, e);
    return e;
 end
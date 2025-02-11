-- Manifest data file
-- generally this is *additive*.
-- ALSO NOTE: engine has no hotloading support. So any changes require a game
-- restart!
--
--
-- Some game code in C++ relies on the existence of
-- some of these assets so try not to remove anything.

-- This doesn't really make authoring them that much less annoying,
-- since the engine doesn't expose many primitives, but it is better than nothing.

-- NOTE: 

engine_dofile("./stages/constants.lua");

projectiles = {
   -- Blue
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2.png", nil, 0.0}
      }
   },

  -- red 
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_r.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_r.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_r.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_r.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_r.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_r.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_r.png", nil, 0.0}
      }
   },

   -- negative
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_inv.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_inv.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_inv.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_inv.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_inv.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_inv.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_inv.png", nil, 0.0}
      }
   },

   -- hotpink
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_hp.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_hp.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_hp.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_hp.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_hp.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_hp.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_hp.png", nil, 0.0}
      }
   },

   -- green
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_g.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_g.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_g.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_g.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_g.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_g.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_g.png", nil, 0.0}
      }
   },

   -- disks

   {
      true,
      {
         {"res/img/projectiles/projectile1_flat.png", nil, 0.0}
      }
   },
   {
      true,
      {
         {"res/img/projectiles/projectile1_r_flat.png", nil, 0.0}
      }
   },
   {
      true,
      {
         {"res/img/projectiles/projectile1_hp_flat.png", nil, 0.0}
      }
   },

   {
      true,
      {
         {"res/img/projectiles/projectile1_inv_flat.png", nil, 0.0}
      }
   },

   {
      true,
      {
         {"res/img/projectiles/projectile1_g_flat.png", nil, 0.0}
      }
   },

   -- sparkling star
   {
      false,
      {
         {"res/img/projectiles/star_sparkle_1.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_2.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_3.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_4.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_5.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_4.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_3.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_2.png", nil, 0.0},
         {"res/img/projectiles/star_sparkle_1.png", nil, 0.0}
      }
   },

   --- NEW CONTENT HERE THAT ISN'T TECHNICALLY HARD CODED

   --- PURPLE
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_purple.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_purple.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_purple.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_purple.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_purple.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_purple.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_purple.png", nil, 0.0}
      }
   },
   {
      true,
      {
         {"res/img/projectiles/projectile1_flat_purple.png", nil, 0.0}
      }
   },
   --- WRM
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_wrm.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_wrm.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_wrm.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_wrm.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_wrm.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_wrm.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_wrm.png", nil, 0.0}
      }
   },
   {
      true,
      {
         {"res/img/projectiles/projectile1_flat_wrm.png", nil, 0.0}
      }
   },
   --- CAUSTIC
   {
      false, -- no rotation
      -- frames
      {
         {"res/img/projectiles/projectile1_caustic.png", nil, 0.0}
      }
   },
   {
      false,
      {
         {"res/img/projectiles/projectile1_caustic.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_caustic.png", nil, 0.0}
      }
   },

   {
      false,
      {
         {"res/img/projectiles/projectile1_caustic.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_caustic.png", nil, 0.0},
         {"res/img/projectiles/projectile1_1_2_caustic.png", nil, 0.0},
         {"res/img/projectiles/projectile1_2_caustic.png", nil, 0.0}
      }
   },
   {
      true,
      {
         {"res/img/projectiles/projectile1_flat_caustic.png", nil, 0.0}
      }
   },
};

entities = {
   {
      {
         {"res/img/skull/enemy_skull_a_0.png", nil,  0.0},
         {"res/img/skull/enemy_skull_a_1.png", nil,  0.0},
         {"res/img/skull/enemy_skull_a_2.png", nil,  0.0},
      }
   },
   {
      {
         {"res/img/skull/enemy_skull_b_0.png", nil,  0.0},
         {"res/img/skull/enemy_skull_b_1.png", nil,  0.0},
         {"res/img/skull/enemy_skull_b_2.png", nil,  0.0},
      }
   },
   {
      {
         {"res/img/bats/enemy_bat1_0.png", nil,  0.0},
         {"res/img/bats/enemy_bat1_1.png", nil,  0.0},
         {"res/img/bats/enemy_bat1_2.png", nil,  0.0},
         {"res/img/bats/enemy_bat1_3.png", nil,  0.0},
      }
   },
   {
      {
         {"res/img/bats/enemy_bat2_0.png", nil,  0.0},
         {"res/img/bats/enemy_bat2_1.png", nil,  0.0},
         {"res/img/bats/enemy_bat2_2.png", nil,  0.0},
         {"res/img/bats/enemy_bat2_3.png", nil,  0.0},
      }
   },
};

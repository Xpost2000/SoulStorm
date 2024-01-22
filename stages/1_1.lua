dofile("stages/common.lua")

function stage_task()
   dialogue_speaker_set_visibility(0, true);
   dialogue_speaker_set_visibility(1, false);
   dialogue_speaker_set_image(0, "./res/img/dlg/D6_Adell_Bust_1.PNG");
   dialogue_speaker_set_image(1, "./res/img/dlg/D6_Rozalin_Bust_1.PNG");
   dialogue_speaker_set_image_scale(0, 0.5, 0.5);
   dialogue_speaker_set_image_scale(1, 0.5, 0.5);
   dialogue_speaker_set_position_offset(1, -90, 0);
   dialogue_speaker_set_mirrored(0, true);
   dialogue_speaker_set_mirrored(1, false);
   dialogue_speaker_animation_fade_in(0, 0.75);

   -- NOTE: dialogue_start will just start things
   --       but does not initialize much state.
   --       So if you want to setup introduction animations for characters
   --       that needs to happen above.
   dialogue_start();
   -- NOTE: animations are not "required" per say
   -- Animations can be skipped and I will allow them to be skipped
   -- and look "weird", since trying to make them robustly transition
   -- is not really something I'd like to rathole doing...
   -- dialogue_speaker_fade_in(0);
   -- dialogue_speaker_fade_out(0);
   -- dialogue_speaker_slide_in(0);
   -- dialogue_speaker_slide_out(0);
   -- dialogue_speaker_focus_in(0);
   -- dialogue_speaker_focus_out(0);
   -- dialogue_speaker_focus_shake(0);
   -- dialogue_speaker_focus_jump(0);
   -- dialogue_speaker_clear_animations(0);

 -- Yield point. Wait until dialogue advancement is allowed.
   dialogue_say_line("Hello World!");
   dialogue_speaker_set_image(0, "./res/img/dlg/D6_Adell_Bust_3.PNG");
   dialogue_say_line("Wait a second.");
   dialogue_speaker_animation_shake(0, 16, 20, 0.0100);
   dialogue_speaker_set_image(0, "./res/img/dlg/D6_Adell_Bust_4.PNG");
   dialogue_speaker_set_visibility(1, true);
   dialogue_speaker_animation_slide_fade_in(1, 0.75);
   t_wait(0.6);
 -- Yield point. Wait until dialogue advancement is allowed.
   dialogue_say_line("Oh. I pooped myself.");
   dialogue_speaker_animation_slide_fade_out(0, 0.6);
   dialogue_speaker_animation_slide_fade_out(1, 0.6);
   t_wait(0.7);
   dialogue_end();
end

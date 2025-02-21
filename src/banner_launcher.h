#ifndef BANNER_LAUNCHER_H
#define BANNER_LANUCHER_H

/*
  Tiny little piece of polish cause it makes me feel happier
  to see this.

  A little launcher since the game tries to preload everything at start-up
  (since it never has to unload everything), this is just shown so that the
  lack of immediate launch doesn't look sus since there's no asynchronous
  asset loading in this engine.
*/

void start_launcher_banner(void);
void kill_launcher_banner(void);

#endif
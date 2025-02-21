#include "common.h"
#include <SDL2/SDL.h>

static SDL_Window* global_launcher_banner_window;
static SDL_Renderer* global_launcher_banner_window_renderer;
static bool global_can_show_launcher = false;

void start_launcher_banner(void)
{
  if (path_exists(string_literal("./launcher.bmp"))) {
    global_can_show_launcher = true;
  }

  if (global_can_show_launcher) {
    SDL_Surface* surf = SDL_LoadBMP("./launcher.bmp");
    int surf_w = surf->w * 2;
    int surf_h = surf->h * 2;
    global_launcher_banner_window = SDL_CreateWindow("LauncherSolStorm",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      surf_w, surf_h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_ALLOW_HIGHDPI);
    global_launcher_banner_window_renderer = SDL_CreateRenderer(global_launcher_banner_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(global_launcher_banner_window_renderer, surf);
    SDL_RenderCopy(global_launcher_banner_window_renderer, texture, 0, 0);
    SDL_RenderPresent(global_launcher_banner_window_renderer);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surf);
  }
}

void kill_launcher_banner(void)
{
  if (global_can_show_launcher) {
    SDL_DestroyWindow(global_launcher_banner_window);
    SDL_DestroyRenderer(global_launcher_banner_window_renderer);
  }
}
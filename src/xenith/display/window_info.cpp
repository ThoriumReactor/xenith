#include "xenith/display/window_info.h"

#include "SDL3/SDL_video.h"


namespace Xenith::Display
{

  // Width
  int GetWindowWidth(Xenith::Display::Window window)
  {
    int window_width = 0;
    SDL_GetWindowSize(window, &window_width, 0);

    return window_width;
  }


  // Height
  int GetWindowHeight(Xenith::Display::Window window)
  {
    int window_height = 0;
    SDL_GetWindowSize(window, 0, &window_height);

    return window_height;
  }


  // Aspect Ratio
  float GetWindowAspect(Xenith::Display::Window window)
  {
    return (float)GetWindowWidth(window) / (float)GetWindowHeight(window);
  }
}

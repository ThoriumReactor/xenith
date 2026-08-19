#include "xenith/graphics/engine/engine_destroy/display_destroy.h"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

#include "xenith/debug_xn/logging.h"


namespace Xenith::Display 
{
  void DestroyDisplay(Xenith::Display::Window window,
                      Xenith::Debug::LogVerbosity log_verbosity)
  {
    SDL_DestroyWindow(window);

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Window has been destroyed");

    SDL_Quit();
  }
}

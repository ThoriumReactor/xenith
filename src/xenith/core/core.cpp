#include "xenith/core/core.h"

#include <iostream>

#include "SDL3/SDL_mouse.h"


namespace Xenith::Core
{
  void InitializePlatformSubsystems(const SubsystemFlag subsystem_flags)
  {
    if(!SDL_Init(subsystem_flags))
    {
      std::cerr << "Failed to initialize subsystem flags" << SDL_GetError() << std::endl;
    }
  }


  void SetRelativeMouseMode(Xenith::Display::Window window, bool enable)
  {
    SDL_SetWindowRelativeMouseMode(window, enable);
  }
}

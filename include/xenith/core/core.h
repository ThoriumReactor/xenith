#pragma once

#include "SDL3/SDL_init.h"

#include "xenith/display/display.h"


namespace Xenith::Core
{
  using SubsystemFlag = SDL_InitFlags;
  namespace SubsystemFlags
  {
    constexpr SubsystemFlag VIDEO     = SDL_INIT_VIDEO;
    constexpr SubsystemFlag AUDIO     = SDL_INIT_AUDIO;
    constexpr SubsystemFlag EVENTS    = SDL_INIT_EVENTS;
    constexpr SubsystemFlag GAMEPAD   = SDL_INIT_GAMEPAD;
    constexpr SubsystemFlag JOYSTICK  = SDL_INIT_JOYSTICK;
  }


  void InitializePlatformSubsystems(const SubsystemFlag subsystem_flags);


  void SetRelativeMouseMode(Xenith::Display::Window window, bool enable);
}

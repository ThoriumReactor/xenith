#pragma once

#include "SDL3/SDL_video.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/display/display.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Display
{
  using CreateWindowFlag = SDL_WindowFlags;
  namespace CreateWindowFlags 
  {
    constexpr CreateWindowFlag ALWAYS_ON_TOP      = SDL_WINDOW_ALWAYS_ON_TOP;
    constexpr CreateWindowFlag BORDERLESS         = SDL_WINDOW_BORDERLESS;
    constexpr CreateWindowFlag FULLSCREEN         = SDL_WINDOW_FULLSCREEN;
    constexpr CreateWindowFlag RESIZABLE          = SDL_WINDOW_RESIZABLE;
    constexpr CreateWindowFlag HIDDEN             = SDL_WINDOW_HIDDEN;
    constexpr CreateWindowFlag HIGH_PIXEL_DENSITY = SDL_WINDOW_HIGH_PIXEL_DENSITY;
  }


  Xenith::Display::Window CreateWindow(std::string title, int width, int height,
                                       const Xenith::Display::CreateWindowFlag create_window_flags);

  /// Set Swapchain parameters
  /// Select present mode:
  /// 1. Vsync - no tearing, high latency
  /// 2. Mailbox - no tearing, low latency, more gpu load
  /// -- You can have high fps without screen tear
  /// 3. Immediate - can tear, lowest latency
  Xenith::Display::PresentMode ChooseSwapchainPresentMode(Xenith::Graphics::PhysicalDevice physical_device,
                                                          Xenith::Graphics::Surface surface, 
                                                          Xenith::Display::PresentMode preferred_present_mode,
                                                          const Xenith::Debug::LogVerbosity log_verbosity);
} // namespace Xenith::Display

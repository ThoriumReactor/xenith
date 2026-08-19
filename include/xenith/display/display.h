#pragma once

#include "SDL3/SDL_video.h"

#include <vulkan/vulkan_core.h>


namespace Xenith::Display
{
  using Window = SDL_Window*;
  
  
  using PresentMode = VkPresentModeKHR;
  namespace PresentModes
  {
    inline constexpr PresentMode IMMEDIATE = VK_PRESENT_MODE_IMMEDIATE_KHR;
    inline constexpr PresentMode MAILBOX = VK_PRESENT_MODE_MAILBOX_KHR;
    inline constexpr PresentMode VSYNC = VK_PRESENT_MODE_FIFO_KHR;
  }
}

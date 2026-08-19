#include "xenith/graphics/engine/engine_init/display_init.h"
#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"

#include <iostream>


namespace Xenith::Display
{
  Xenith::Display::Window CreateWindow(std::string title, int width, int height,
                                        const Xenith::Display::CreateWindowFlag create_window_flags)
  {
    Xenith::Display::Window window = SDL_CreateWindow(title.c_str(), width, height,
                                                       create_window_flags | SDL_WINDOW_VULKAN);

    if(!window)
    {
      std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    }

    return window;
  }


  Xenith::Display::PresentMode ChooseSwapchainPresentMode(Xenith::Graphics::PhysicalDevice physical_device,
                                                          Xenith::Graphics::Surface surface, 
                                                          Xenith::Display::PresentMode preferred_present_mode,
                                                          const Xenith::Debug::LogVerbosity log_verbosity)
  {
    VkPresentModeKHR present_mode_to_return;

    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, 
                                              &present_modes_count, nullptr);
    
    std::vector<VkPresentModeKHR> present_modes(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_modes_count, 
                                              present_modes.data());


    bool mode_found = false;
    for(const auto &avaible_present_mode : present_modes)
    {
      if(preferred_present_mode == avaible_present_mode)
      {
        present_mode_to_return = preferred_present_mode;
        mode_found = true;

        break;
      }
    }

    if(mode_found)
    {
      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        SDL_Log("Chosen present mode is supported, index: %d", present_mode_to_return);
    }

    else
    {
      present_mode_to_return = Xenith::Display::PresentModes::VSYNC;

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        SDL_Log("Chosen present mode is not supported! Fallback to vsync, index: %d", present_mode_to_return);
    }

    return preferred_present_mode;
  }
} // namespace Xenith::Display

#pragma once

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logs.h"
#include "xenith/display/display.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  /// Gets physical device surface capabilities
  Xenith::Graphics::SurfaceCapabilities GetSurfaceCapabilities(Xenith::Graphics::PhysicalDevice physical_device,
                                                               Xenith::Graphics::Surface surface);

  /// Choose swapchain surface format
  /// If failed, throws an error log and returns SurfaceFormat.format = VK_FORMAT_UNDEFINED
  Xenith::Graphics::SurfaceFormat ChooseSwapchainSurfaceFormat(Xenith::Graphics::PhysicalDevice physical_device,
                                                               Xenith::Graphics::Surface surface, 
                                                               Xenith::Graphics::SwapchainSurfaceFormatConfig &swapchain_surface_format_config,
                                                               const Xenith::Debug::LogVerbosity log_verbosity);

  /// Creates the swapchain
  /// old_swapchain is optional paramter, usually needed when recreating swapchain
  /// If failed, throws an error log and return VK_NULL_HANDLE
  Graphics::Swapchain CreateSwapchain(Xenith::Graphics::Device logical_device,
                                      uint32_t swapchain_width, uint32_t swapchain_height,
                                      Xenith::Graphics::Surface surface,
                                      Xenith::Graphics::SurfaceCapabilities surface_capabilities,
                                      Xenith::Graphics::SurfaceFormat surface_format,
                                      Xenith::Display::PresentMode present_mode,
                                      const Xenith::Debug::LogVerbosity log_verbosity,
                                      Xenith::Graphics::Swapchain old_swapchain = nullptr);

  /// Initializes both images vector and image views vector
  /// If failed, throws an error log
  void InitializeSwapchainImagesAndImageViewsContainers(Xenith::Graphics::Device logical_device,
                                                        Xenith::Graphics::Swapchain swapchain,
                                                        Xenith::Graphics::SurfaceFormat surface_format,
                                                        std::vector<VkImage> &swapchain_images,
                                                        std::vector<VkImageView> &swapchain_image_views,
                                                        const Xenith::Debug::LogVerbosity log_verbosity);
}

#include "xenith/graphics/engine/engine_init/swapchain_init.h"

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logs.h"
#include "xenith/debug_xn/logging.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  Xenith::Graphics::SurfaceCapabilities GetSurfaceCapabilities(Xenith::Graphics::PhysicalDevice physical_device,
                                                               Xenith::Graphics::Surface surface)
  {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                              surface, 
                                              &surface_capabilities);

    return surface_capabilities;
  }
  

  Xenith::Graphics::SurfaceFormat ChooseSwapchainSurfaceFormat(Xenith::Graphics::PhysicalDevice physical_device,
                                                               Xenith::Graphics::Surface surface, 
                                                               Xenith::Graphics::SwapchainSurfaceFormatConfig &swapchain_surface_format_config,
                                                               const Xenith::Debug::LogVerbosity log_verbosity)
  {
    VkSurfaceFormatKHR surface_format_to_return;

    // Surface formats
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                         surface,
                                         &format_count,
                                         nullptr);

    if(format_count == 0)
    {
      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Couldn't find any vulkan surface formats!");

      surface_format_to_return.format = VK_FORMAT_UNDEFINED;
    }
    

    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                         surface,
                                         &format_count,
                                         surface_formats.data());

    bool format_found = false;

    // HDR
    if (swapchain_surface_format_config.color_mode == Xenith::Graphics::SurfaceColorMode::HDR_10_ST2084)
    {
      for (const auto& available_format : surface_formats)
      {
        if (available_format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
            available_format.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
        {
          surface_format_to_return = available_format;
          format_found = true;
          break;
        }
      }
    }

    // SDR
    if (!format_found || swapchain_surface_format_config.color_mode == Xenith::Graphics::SurfaceColorMode::SDR_SRGB)
    {
      for (const auto& available_format : surface_formats)
      {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
          surface_format_to_return = available_format;
          format_found = true;
          break;
        }
      }
    }


    if(!format_found)
    {
      Xenith::Debug::Log("Fallback option was chosen");
      surface_format_to_return = surface_formats[0];
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
    {
      Xenith::Debug::Log("Picked the surface format index: %d", surface_format_to_return.format);
      Xenith::Debug::Log("Picked the surface colorspace index: %d", surface_format_to_return.colorSpace);
    }

    return surface_format_to_return;
  }


  Graphics::Swapchain CreateSwapchain(Xenith::Graphics::Device logical_device,
                                      uint32_t swapchain_width, uint32_t swapchain_height,
                                      Xenith::Graphics::Surface surface,
                                      Xenith::Graphics::SurfaceCapabilities surface_capabilities,
                                      Xenith::Graphics::SurfaceFormat surface_format,
                                      Xenith::Display::PresentMode present_mode,
                                      const Xenith::Debug::LogVerbosity log_verbosity,
                                      Xenith::Graphics::Swapchain old_swapchain)
  {
    VkSwapchainKHR swapchain_to_return = VK_NULL_HANDLE;

    // n of images
    uint32_t requested_image_count = std::max(2u, surface_capabilities.minImageCount);
    if(surface_capabilities.maxImageCount > 0)
    {
      requested_image_count = std::min(requested_image_count, surface_capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,

      .surface = surface,
      .minImageCount = requested_image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent{ .width = swapchain_width, .height = swapchain_height },
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

      .preTransform = surface_capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

      .presentMode = present_mode,

      .oldSwapchain = old_swapchain
    };

    auto swapchain_result = vkCreateSwapchainKHR(logical_device,
                                                           &swapchain_create_info,
                                                           nullptr,
                                                           &swapchain_to_return);
    if(swapchain_result != VK_SUCCESS)
    {
      return VK_NULL_HANDLE;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Swapchain created");

    return swapchain_to_return;
  }


  void InitializeSwapchainImagesAndImageViewsContainers(Xenith::Graphics::Device logical_device,
                                                        Xenith::Graphics::Swapchain swapchain,
                                                        Xenith::Graphics::SurfaceFormat surface_format,
                                                        std::vector<VkImage> &swapchain_images,
                                                        std::vector<VkImageView> &swapchain_image_views,
                                                        const Xenith::Debug::LogVerbosity log_verbosity)
  {
    uint32_t swapchain_image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swapchain, &swapchain_image_count, nullptr);

    swapchain_images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(logical_device, swapchain,
                            &swapchain_image_count,
                            swapchain_images.data());
    swapchain_image_views.resize(swapchain_image_count);

    for(size_t i = 0; i < swapchain_image_views.size(); ++i)
    {
      VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

        .image = swapchain_images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surface_format.format,

        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1
        }
      };

      if(vkCreateImageView(logical_device, &image_view_info,
                           nullptr, &swapchain_image_views[i]) != VK_SUCCESS)
      {
        Xenith::Debug::Log("Failed to create swapchain image view!");
      }
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Swapchain image views container initialized");
  }
}

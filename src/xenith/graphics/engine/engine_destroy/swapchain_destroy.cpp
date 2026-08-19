#include "xenith/graphics/engine/engine_destroy/swapchain_destroy.h"

#include "xenith/graphics/graphics_vk.h"
#include "xenith/debug_xn/logging.h"


namespace Xenith::Graphics
{
  void DestroySwapchainContext(Xenith::EngineECS::SwapchainContext &swapchain_context,
                               Xenith::Graphics::Device logical_device,
                               const Xenith::Debug::LogVerbosity log_verbosity)
  {
    for(auto &image_view : swapchain_context.swapchain_image_views)
    {
      vkDestroyImageView(logical_device, image_view, nullptr);
    }
    swapchain_context.swapchain_image_views.clear();

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Swapchain image views container destroyed");

    if(swapchain_context.swapchain != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(logical_device, swapchain_context.swapchain, nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Swapchain destroyed");
    }
  }

}

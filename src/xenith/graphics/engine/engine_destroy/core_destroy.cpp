#include "xenith/graphics/engine/engine_destroy/core_destroy.h"

#include "xenith/debug_xn/logging.h"


namespace Xenith::Graphics
{
  void DestroyCoreContext(Xenith::EngineECS::CoreContext &core_context,
                          const Xenith::Debug::LogVerbosity log_verbosity)
  {
    // VMA allocator
    if(core_context.allocator)
    {
      vmaDestroyAllocator(core_context.allocator);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Vulkan Memory Allocator destroyed");
    }

    // Logical device
    if(core_context.device != VK_NULL_HANDLE)
    {
      vkDestroyDevice(core_context.device, nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Vulkan logical device destroyed");
    }

    // Surface
    if(core_context.surface != VK_NULL_HANDLE)
    {
      vkDestroySurfaceKHR(core_context.instance, core_context.surface,
                          nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Vulkan window surface destroyed");
    }

    // Instance
    if(core_context.instance != VK_NULL_HANDLE)
    {
      vkDestroyInstance(core_context.instance, nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Vulkan instance destroyed");
    }
  }
}

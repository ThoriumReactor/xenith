#include "xenith/graphics/engine/engine_destroy/render_destroy.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/debug_xn/logging.h"


namespace Xenith::Graphics
{
  void DestroyRenderContext(Xenith::EngineECS::RenderContext &render_context,
                            Xenith::Graphics::Device logical_device,
                            const Xenith::Debug::LogVerbosity log_verbosity)
  {
    // Command resources
    for(auto &single_command_resource : render_context.command_resources)
    {
      vkDestroyCommandPool(logical_device, single_command_resource.command_pool, nullptr);
    }
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Command pools destroyed");

    // Render finished semaphores
    for(auto &render_finished_semaphore : render_context.render_finished_semaphores)
    {
      vkDestroySemaphore(logical_device, render_finished_semaphore, nullptr);
    }
    render_context.render_finished_semaphores.clear();

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Render finished semaphores container destroyed");

    // Image acquired semaphores
    for(auto &single_image_acquired_semaphore : render_context.image_acquired_semaphores)
    {
      vkDestroySemaphore(logical_device, single_image_acquired_semaphore, nullptr);
    }
    render_context.image_acquired_semaphores.clear();

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Image acquired semaphores destroyed");

    // Timeline semaphore
    if(render_context.timeline_semaphore != VK_NULL_HANDLE)
    {
      vkDestroySemaphore(logical_device, render_context.timeline_semaphore, nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Timeline semaphore destroyed");
    }
  }
}

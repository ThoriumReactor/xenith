#include "xenith/graphics/engine/engine_init/render_init.h"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logging.h"
#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"



namespace Xenith::Graphics
{
  Xenith::Graphics::Semaphore CreateTimelineSemaphore(Xenith::Graphics::Device logical_device,
                                                      const uint64_t initial_value,
                                                      const Xenith::Debug::LogVerbosity log_verbosity)
  {
    VkSemaphore timeline_semaphore_to_return = VK_NULL_HANDLE;

    VkSemaphoreTypeCreateInfo timeline_semaphore_type_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,

      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue = initial_value
    };
    VkSemaphoreCreateInfo timeline_semaphore_create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &timeline_semaphore_type_info
    };

    if(vkCreateSemaphore(logical_device, &timeline_semaphore_create_info,
                         nullptr, &timeline_semaphore_to_return) != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create timeline semaphore");

      return VK_NULL_HANDLE;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Timeline semaphore created");

    return timeline_semaphore_to_return;
  }


  void InitializeImageAcquiredSemaphoresContainer(Xenith::Graphics::Device logical_device,
                                                  const uint32_t max_frames_in_flight,
                                                  std::vector<Xenith::Graphics::Semaphore> &image_acquired_semaphores,
                                                  const Xenith::Debug::LogVerbosity log_verbosity)
  {
    image_acquired_semaphores.resize(max_frames_in_flight);

    for(auto &single_image_acquired_semaphore : image_acquired_semaphores)
    {
      VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      };

      if(vkCreateSemaphore(logical_device, &semaphore_create_info,
                           nullptr, &single_image_acquired_semaphore) != VK_SUCCESS)
      {
        Xenith::Debug::Log("Failed to create image acquired semaphore");
      }
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Image acquired semaphores container initialized");
  }


  void InitializeRenderFinishedSemaphoresContainer(Xenith::Graphics::Device logical_device,
                                                   const uint32_t max_frames_in_flight,
                                                   std::vector<Xenith::Graphics::Semaphore> &render_finished_semaphores,
                                                   const Xenith::Debug::LogVerbosity log_verbosity)
  {
    render_finished_semaphores.resize(max_frames_in_flight);

    for(VkSemaphore &semaphore : render_finished_semaphores)
    {
      VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      };

      if(vkCreateSemaphore(logical_device, &semaphore_create_info, 
                           nullptr, &semaphore) != VK_SUCCESS)
      {
        Xenith::Debug::Log("Failed to initialize render finished semaphore");
      }
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Render finished semaphores container initialized");
  }


  void InitializeCommandResourcesCommandPoolsAndAllocateCommandBuffers(Xenith::Graphics::Device logical_device,
                                                                       const uint32_t max_frames_in_flight,
                                                                       std::vector<Xenith::Graphics::CommandResources> &command_resources,
                                                                       uint32_t graphics_family_queue_index,
                                                                       const Xenith::Debug::LogVerbosity log_verbosity)
  {
    command_resources.resize(max_frames_in_flight);

    for(auto &single_command_resource : command_resources)
    {
      VkCommandPoolCreateInfo cmd_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,

        .queueFamilyIndex = graphics_family_queue_index
      };
      if(vkCreateCommandPool(logical_device, &cmd_pool_create_info,
                             nullptr, &single_command_resource.command_pool) != VK_SUCCESS)
      {
        Xenith::Debug::Log("Failed to create command pool");
      }


      VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

        .commandPool = single_command_resource.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
      };

      if(vkAllocateCommandBuffers(logical_device, &cmd_buffer_alloc_info, 
                                  &single_command_resource.command_buffer) != VK_SUCCESS)
      {
        Xenith::Debug::Log("Failed to allocate command buffer");
      }
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
    {
      Xenith::Debug::Log("Command pools initialized");
      Xenith::Debug::Log("Command buffers initialized");
    }
  }
}

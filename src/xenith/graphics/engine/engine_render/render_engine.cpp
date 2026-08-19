#include "xenith/graphics/engine/engine_render/render_engine.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include <vulkan/vulkan_core.h>

#include "xenith/graphics/graphics_vk.h"



namespace Xenith::Graphics
{
  uint32_t CalculateWindowWidthForSwapchainRecreation(int window_width, Xenith::Graphics::SurfaceCapabilities surface_capabilities)
  {
    uint32_t actual_window_width = std::clamp((uint32_t)window_width, surface_capabilities.minImageExtent.width, 
                                              surface_capabilities.maxImageExtent.width);

    return actual_window_width;
  }

  uint32_t CalculateWindowHeightForSwapchainRecreation(int window_height, Xenith::Graphics::SurfaceCapabilities surface_capabilities)
  {
    uint32_t actual_window_height = std::clamp((uint32_t)window_height, surface_capabilities.minImageExtent.height, 
                                               surface_capabilities.maxImageExtent.height);

    return actual_window_height;
  }


  void WaitTimelineSemaphore(Xenith::Graphics::Device logical_device,
                             Xenith::Graphics::Semaphore &timeline_semaphore,
                             const uint64_t wait_value,
                             uint64_t timeout_nanoseconds)
  {
    VkSemaphoreWaitInfo semaphore_wait_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,

      .semaphoreCount = 1,
      .pSemaphores = &timeline_semaphore,
      .pValues = &wait_value
    };

    vkWaitSemaphores(logical_device, &semaphore_wait_info, timeout_nanoseconds);
  }


  void ResetCommandPool(Xenith::Graphics::Device logical_device,
                        Xenith::Graphics::CommandPool command_pool,
                        VkCommandPoolResetFlags reset_flags)
  {
    vkResetCommandPool(logical_device, command_pool, reset_flags);
  }


  SwapchainAcquisitionResult AcquireNextImageFromSwapchain(Xenith::Graphics::Device logical_device,
                                                           Xenith::Graphics::Swapchain swapchain,
                                                           Xenith::Graphics::Semaphore &image_acquired_semaphore,
                                                           uint64_t timeout_nanoseconds)
  {
    SwapchainAcquisitionResult swapchain_acquisition_result_to_return;

    uint32_t image_idx = 0;
    VkResult acquire_result = vkAcquireNextImageKHR(logical_device, swapchain, timeout_nanoseconds, 
                                                    image_acquired_semaphore,
                                                    nullptr, &image_idx);
    swapchain_acquisition_result_to_return = {
      .result = acquire_result,
      .image_index = image_idx
    };

    return swapchain_acquisition_result_to_return;
  }


  void BeginCommandBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                          VkCommandBufferUsageFlags usage_flags,
                          const VkCommandBufferInheritanceInfo* inheritance_info)
  {
    VkCommandBufferBeginInfo cmd_buffer_begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,

      .flags = usage_flags,
      .pInheritanceInfo = inheritance_info
    };

    vkBeginCommandBuffer(command_buffer, &cmd_buffer_begin_info);
  }

  void EndCommandBuffer(Xenith::Graphics::CommandBuffer command_buffer)
  {
    vkEndCommandBuffer(command_buffer);
  }


  Xenith::Graphics::ImageMemoryBarrier CreateSwapchainImageMemoryBarrierForRender(VkImage swapchain_image_handle)
  {
    VkImageMemoryBarrier2 swapchain_image_memory_barrier_render_to_return = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,

      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = swapchain_image_handle,

      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    return swapchain_image_memory_barrier_render_to_return;
  }

  Xenith::Graphics::ImageMemoryBarrier CreateSwapchainImageMemoryBarrierForPresentation(VkImage swapchain_image_handle)
  {
    VkImageMemoryBarrier2 swapchain_image_barrier_present_to_return = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

      .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
      .dstAccessMask = 0,

      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .image = swapchain_image_handle,

      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    return swapchain_image_barrier_present_to_return;
  }

  Xenith::Graphics::ImageMemoryBarrier CreateDepthImageMemoryBarrierForRender(VkImage depth_image_handle)
  {
    VkImageMemoryBarrier2 depth_barrier_render_to_return = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

      .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,

      .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
      .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .image = depth_image_handle,

      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    return depth_barrier_render_to_return;
  }


  void PipelineBarrier(Xenith::Graphics::CommandBuffer command_buffer,
                       const size_t image_memory_barrier_count,
                       const Xenith::Graphics::ImageMemoryBarrier* image_memory_barrier_data)
  {
    VkDependencyInfo dependency_info_render = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,

      .imageMemoryBarrierCount = static_cast<uint32_t>(image_memory_barrier_count), 
      .pImageMemoryBarriers = image_memory_barrier_data
    };

    vkCmdPipelineBarrier2(command_buffer, &dependency_info_render);
  }


  void BeginRendering(Xenith::Graphics::CommandBuffer command_buffer,
                      Xenith::Graphics::RenderingConfig &rendering_config)
  {
    auto &color_attach_config = rendering_config.color_attachment_config_data;
    auto &depth_attach_config = rendering_config.depth_attachment_config;


    constexpr uint32_t MAX_COLOR_ATTACHMENTS = 8;
    uint32_t attachment_count = std::min(rendering_config.color_attachment_count, MAX_COLOR_ATTACHMENTS);

    // Color attachment info
    std::array<VkRenderingAttachmentInfo, MAX_COLOR_ATTACHMENTS> vk_color_attachments{};

    for(uint32_t i = 0; i < attachment_count; ++i)
    {
      const auto& config = color_attach_config[i];

      vk_color_attachments[i] = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,

        .imageView = config.swapchain_image_view,
        .imageLayout = config.image_layout,
        .loadOp = config.load_op,
        .storeOp = config.store_op,

        .clearValue = { 
          .color = { config.clear_color_value.x, config.clear_color_value.y,
                     config.clear_color_value.z, config.clear_color_value.w } 
        } // clear_value
      }; // color attachments[i]
    } // for loop

    // Depth attachment info
    VkClearDepthStencilValue depth_stencil_clear_value = depth_attach_config.clear_depth_stencil_value;
    VkRenderingAttachmentInfo depth_attach_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,

      .imageView = depth_attach_config.image_view,
      .imageLayout = depth_attach_config.image_layout,

      .loadOp = depth_attach_config.load_op,
      .storeOp = depth_attach_config.store_op,

      .clearValue = { .depthStencil = depth_stencil_clear_value }
    };

    VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,

      .renderArea = {
        .offset = { .x = 0, .y = 0 },
        .extent = { .width = rendering_config.render_area_width, .height = rendering_config.render_area_height }
      },
      .layerCount = rendering_config.layer_count,
      .colorAttachmentCount = attachment_count,
      .pColorAttachments = vk_color_attachments.data(),
      .pDepthAttachment = &depth_attach_info
    };

    vkCmdBeginRendering(command_buffer, &rendering_info);
  }

  void EndRendering(Xenith::Graphics::CommandBuffer command_buffer)
  {
    vkCmdEndRendering(command_buffer);
  }


  void SetViewportAndScissor(Xenith::Graphics::CommandBuffer command_buffer,
                             uint32_t width, uint32_t height)
  {
    // Viewport
    VkViewport viewport = {
      .x = 0.0f, .y = 0.0f,

      .width = static_cast<float>(width),
      .height = static_cast<float>(height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f
    };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    // Scissor
    VkRect2D scissor = {
      .offset = VkOffset2D{ .x = 0, .y = 0 },
      .extent = VkExtent2D{ .width = (width),
                            .height =(height) }
    };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
  }


  void BindPipeline(Xenith::Graphics::CommandBuffer command_buffer,
                    Xenith::Graphics::Pipeline pipeline,
                    VkPipelineBindPoint bind_point)
  {
      vkCmdBindPipeline(command_buffer, bind_point, pipeline);
  }


  void PushConstants(Xenith::Graphics::CommandBuffer command_buffer, VkPipelineLayout pipeline_layout_handle,
                     Xenith::Graphics::ShaderTypeFlag shader_type_flags, uint32_t size_of_push_constant,
                     const void* push_constant_data, uint32_t offset)
  {
    vkCmdPushConstants(command_buffer, pipeline_layout_handle,
                       shader_type_flags, offset, size_of_push_constant,
                       push_constant_data);
  }

  
  void BindSingleVertexBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                              const VkBuffer &vertex_buffer_handle,
                              VkDeviceSize offset,
                              uint32_t binding_slot)
  {
    vkCmdBindVertexBuffers(command_buffer, binding_slot, 1,
                           &vertex_buffer_handle, &offset);
  }

  void BindMultipleVertexBuffers(Xenith::Graphics::CommandBuffer command_buffer,
                                 const VkBuffer* raw_vertex_buffers,
                                 const VkDeviceSize* offsets,
                                 uint32_t first_binding,
                                 uint32_t binding_count)
  {
    vkCmdBindVertexBuffers(command_buffer, first_binding, binding_count,
                           raw_vertex_buffers, offsets);
  }


  void BindIndexBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                       VkBuffer index_buffer_handle,
                       VkDeviceSize offset,
                       VkIndexType index_data_type)
  {
    vkCmdBindIndexBuffer(command_buffer, index_buffer_handle, offset, index_data_type);
  }


  void DrawIndexed(Xenith::Graphics::CommandBuffer command_buffer,
                   uint32_t index_count, uint32_t instance_count,
                   uint32_t first_index, uint32_t first_instance,
                   uint32_t vertex_offset)
  {
    vkCmdDrawIndexed(command_buffer, index_count, instance_count, 
                     first_index, vertex_offset, first_instance);
  }


  void SubmitFrameSingleCommandBuffer(Xenith::Graphics::Queue graphics_queue,
                                      Xenith::Graphics::CommandBuffer command_buffer,
                                      const Xenith::Graphics::FrameSubmitConfig &frame_submit_config)
  {
    VkSemaphoreSubmitInfo image_acquire_wait_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,

      .semaphore = frame_submit_config.image_acquired_semaphore,
      .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSemaphoreSubmitInfo render_finished_submit_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,

      .semaphore = frame_submit_config.current_render_finished_semaphore,
      .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
    };
    VkSemaphoreSubmitInfo timeline_submit_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,

      .semaphore = frame_submit_config.timeline_semaphore,
      .value = frame_submit_config.signal_value,
      .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
    };
    std::array<VkSemaphoreSubmitInfo, 2> semaphore_signals = {
      render_finished_submit_info, timeline_submit_info
    };

    VkCommandBufferSubmitInfo cmd_buffer_submit_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,

      .commandBuffer = command_buffer
    };

    VkSubmitInfo2 submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount = 1,
      .pWaitSemaphoreInfos = &image_acquire_wait_info,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmd_buffer_submit_info,
      .signalSemaphoreInfoCount = static_cast<uint32_t>(semaphore_signals.size()),
      .pSignalSemaphoreInfos = semaphore_signals.data()
    };
    vkQueueSubmit2(graphics_queue, 1, &submitInfo, nullptr);
  }

  void SubmitFrameMultipleCommandBuffers(Xenith::Graphics::Queue graphics_queue,
                                         uint32_t command_buffer_count,
                                         const Xenith::Graphics::CommandBuffer* command_buffer_data,
                                         const Xenith::Graphics::FrameSubmitConfig &frame_submit_config)
  {
    // empty for now
  }


  void PresentFrame(Xenith::Graphics::Queue graphics_queue,
                    const Xenith::Graphics::FramePresentConfig &frame_present_config)
  {
    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frame_present_config.current_render_finished_semaphore,
      .swapchainCount = 1,
      .pSwapchains = &frame_present_config.swapchain,
      .pImageIndices = &frame_present_config.swapchain_image_index,

      .pResults = nullptr
    };

    vkQueuePresentKHR(graphics_queue, &present_info);
  }
}

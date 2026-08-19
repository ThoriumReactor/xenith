#pragma once

#include <cstdint>

#include "xenith/graphics/graphics_vk.h"



namespace Xenith::Graphics
{
  /// Clamps window width to hardware allowed surface capabilities
  uint32_t CalculateWindowWidthForSwapchainRecreation(int window_width, Xenith::Graphics::SurfaceCapabilities surface_capabilities);

  /// Clamps window height to hardware allowed surface capabilities
  uint32_t CalculateWindowHeightForSwapchainRecreation(int window_height, Xenith::Graphics::SurfaceCapabilities surface_capabilities);


  /// @brief Holds CPU thread until GPU timeline semaphore reached specified wait value
  /// @param timeout_nanoseconds Determines how long will CPU wait for GPU signal,
  /// The default value for it is UINT64_MAX, which means the CPU is going to wait for GPU as long as its needed
  void WaitTimelineSemaphore(Xenith::Graphics::Device logical_device,
                             Xenith::Graphics::Semaphore &timeline_semaphore,
                             const uint64_t wait_value,
                             uint64_t timeout_nanoseconds = UINT64_MAX);

  /// @brief Resets all of command buffers inside command pool
  /// @param reset_flags is a flag that determines if you want to release resources from command pool back to system
  /// Default value is 0, since usually you want to keep it ready for next frame
  void ResetCommandPool(Xenith::Graphics::Device logical_device,
                        Xenith::Graphics::CommandPool command_pool,
                        VkCommandPoolResetFlags reset_flags = 0);

  /// @brief Acquires next image from swapchain
  /// @param timeout_nanoseconds Determines how long will CPU wait for GPU signal,
  /// The default value for it is UINT64_MAX, which means the CPU is going to wait for GPU as long as its needed
  /// @returns SwapchainAcquisitionResult struct that contains result and image index values
  /// - result variable that you need to check for swapchain recreation(e.g. window resizing)
  /// - image index shows current image index, needed for drawing
  SwapchainAcquisitionResult AcquireNextImageFromSwapchain(Xenith::Graphics::Device logical_device,
                                                           Xenith::Graphics::Swapchain swapchain,
                                                           Xenith::Graphics::Semaphore &image_acquired_semaphore,
                                                           uint64_t timeout_nanoseconds = UINT64_MAX);

  /// @brief Starts recording of command buffer
  /// @param usage_flags variable determines how command buffer will be used.
  /// Default is ONE_TIME_SUBMIT which is for standard frame loop recording
  /// @param inheritance_info needed if you are beginning record of secondary buffer,
  /// default is nullptr for primary buffers
  void BeginCommandBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                          VkCommandBufferUsageFlags usage_flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                          const VkCommandBufferInheritanceInfo* inheritance_info = nullptr);

  /// @brief Ends recording of command buffer
  void EndCommandBuffer(Xenith::Graphics::CommandBuffer command_buffer);


  Xenith::Graphics::ImageMemoryBarrier CreateSwapchainImageMemoryBarrierForRender(VkImage swapchain_image_handle);

  Xenith::Graphics::ImageMemoryBarrier CreateSwapchainImageMemoryBarrierForPresentation(VkImage swapchain_image_handle);

  Xenith::Graphics::ImageMemoryBarrier CreateDepthImageMemoryBarrierForRender(VkImage depth_image_handle);

  void PipelineBarrier(Xenith::Graphics::CommandBuffer command_buffer,
                       const size_t image_memory_barrier_count,
                       const Xenith::Graphics::ImageMemoryBarrier* image_memory_barrier_data);


  void BeginRendering(Xenith::Graphics::CommandBuffer command_buffer,
                      Xenith::Graphics::RenderingConfig &rendering_config);

  void EndRendering(Xenith::Graphics::CommandBuffer command_buffer);


  void SetViewportAndScissor(Xenith::Graphics::CommandBuffer command_buffer,
                             uint32_t width, uint32_t height);


  /// @brief Binds pipeline to command buffer
  /// @param bind_point a variable that specifies binding point of pipeline object, common are:
  /// GRAPHICS, COMPUTE, RAY_TRACING
  void BindPipeline(Xenith::Graphics::CommandBuffer command_buffer,
                    Xenith::Graphics::Pipeline pipeline,
                    VkPipelineBindPoint bind_point);

  void PushConstants(Xenith::Graphics::CommandBuffer command_buffer, VkPipelineLayout pipeline_layout_handle,
                     Xenith::Graphics::ShaderTypeFlag shader_type_flags, uint32_t size_of_push_constant,
                     const void* push_constant_data, uint32_t offset = 0);

  void BindSingleVertexBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                              const VkBuffer &vertex_buffer_handle,
                              VkDeviceSize offset = 0,
                              uint32_t binding_slot = 0);
  
  void BindMultipleVertexBuffers(Xenith::Graphics::CommandBuffer command_buffer,
                                 const VkBuffer* raw_vertex_buffers,
                                 const VkDeviceSize* offsets,
                                 uint32_t first_binding,
                                 uint32_t binding_count);

  void BindIndexBuffer(Xenith::Graphics::CommandBuffer command_buffer,
                       VkBuffer index_buffer_handle,
                       VkDeviceSize offset = 0,
                       VkIndexType index_data_type = VK_INDEX_TYPE_UINT32);

  void DrawIndexed(Xenith::Graphics::CommandBuffer command_buffer,
                   uint32_t index_count, uint32_t instance_count,
                   uint32_t first_index = 0, uint32_t first_instance = 0,
                   uint32_t vertex_offset = 0);

  void SubmitFrameSingleCommandBuffer(Xenith::Graphics::Queue graphics_queue,
                                      Xenith::Graphics::CommandBuffer command_buffer,
                                      const Xenith::Graphics::FrameSubmitConfig &frame_submit_config);

  void SubmitFrameMultipleCommandBuffers(Xenith::Graphics::Queue graphics_queue,
                                         uint32_t command_buffer_count,
                                         const Xenith::Graphics::CommandBuffer* command_buffer_data,
                                         const Xenith::Graphics::FrameSubmitConfig &frame_submit_config);

  void PresentFrame(Xenith::Graphics::Queue graphics_queue,
                    const Xenith::Graphics::FramePresentConfig &frame_present_config);


  inline void QueueWaitIdle(Xenith::Graphics::Queue queue)
  {
    vkQueueWaitIdle(queue);
  }

  inline void DeviceWaitIdle(Xenith::Graphics::Device device)
  {
    vkDeviceWaitIdle(device);
  }
}

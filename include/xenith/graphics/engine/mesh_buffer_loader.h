#pragma once

#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  /// Uploads buffer to device
  Xenith::Graphics::Buffer UploadBufferToVRAMStatic(Xenith::Graphics::Device logical_device,                                                                                                               Xenith::Graphics::Allocator allocator,
                                                    Xenith::Graphics::Queue target_queue,
                                                    Xenith::Graphics::CommandPool command_pool,
                                                    const void* cpu_data,
                                                    const VkDeviceSize buffer_size,
                                                    const VkBufferUsageFlags usage_flags);

  /// Clean Mesh CPU Data
  void CleanMeshCPUData(Xenith::Graphics::Mesh &mesh);

  /// Uploads vertex and index buffers to device, and initializes vertex_and_index_buffers_to_upload variable with handles to buffers
  /// clean mesh's struct index and vertex vectors cpu data by corresponding "clean_cpu_data" bool
  void UploadMeshBuffersToVRAMStaticSingle(Xenith::Graphics::Device logical_device, Xenith::Graphics::Allocator allocator,
                                           Xenith::Graphics::Queue graphics_queue, Xenith::Graphics::CommandPool command_pool,
                                           std::vector<Xenith::Graphics::Vertex3D> &cpu_vertices, std::vector<uint32_t> &cpu_indices,
                                           Xenith::Graphics::VertexAndIndexBuffers &vertex_and_index_buffers_to_initialize,
                                           const bool clean_cpu_data, const Xenith::Debug::LogVerbosity log_verbosity);


  inline void DestroyBuffer(Xenith::Graphics::Allocator allocator, Xenith::Graphics::Buffer &buffer)
  {
    vmaDestroyBuffer(allocator, buffer.handle, buffer.allocation);
  }
} // Xenith::Graphics

#include <xenith/graphics/engine/mesh_buffer_loader.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  Xenith::Graphics::Buffer UploadBufferToVRAMStatic(Xenith::Graphics::Device logical_device,
  					      	                                Xenith::Graphics::Allocator allocator,
                                                    Xenith::Graphics::Queue target_queue,
                                                    Xenith::Graphics::CommandPool command_pool,
                                                    const void* cpu_data,
                                                    const VkDeviceSize buffer_size,
                                                    const VkBufferUsageFlags usage_flags)
  {
    Xenith::Graphics::Buffer buffer_to_return;
    buffer_to_return.size = buffer_size;

    // Buffers
    VkBufferCreateInfo buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,

      .size = buffer_size,
      .usage = usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VmaAllocationCreateInfo allocation_create_info = {
      .usage = VMA_MEMORY_USAGE_AUTO
    };

    vmaCreateBuffer(allocator, &buffer_create_info, 
                    &allocation_create_info, &buffer_to_return.handle,
                    &buffer_to_return.allocation, nullptr);


    VkBuffer staging_buffer;
    VmaAllocation staging_allocation;
    VmaAllocationInfo staging_allocation_result; 

    VkBufferCreateInfo staging_buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,

      .size = buffer_size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VmaAllocationCreateInfo staging_allocation_create_info = {
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO
    };

    vmaCreateBuffer(allocator, &staging_buffer_create_info,
                    &staging_allocation_create_info, &staging_buffer,
                    &staging_allocation, &staging_allocation_result);

    uint8_t* mapped_memory = static_cast<uint8_t*>(staging_allocation_result.pMappedData);
    std::memcpy(mapped_memory, cpu_data, buffer_size);


    VkCommandBufferAllocateInfo transfer_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1
    };
    VkCommandBuffer transfer_buffer;
    vkAllocateCommandBuffers(logical_device, &transfer_allocate_info, &transfer_buffer);

    VkCommandBufferBeginInfo transfer_begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,

      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(transfer_buffer, &transfer_begin_info);

    VkBufferCopy buffer_copy = { .srcOffset = 0, .dstOffset = 0, .size = buffer_size };
    vkCmdCopyBuffer(transfer_buffer, staging_buffer, 
                    buffer_to_return.handle,
                    1, &buffer_copy);
    
    vkEndCommandBuffer(transfer_buffer);


    VkCommandBufferSubmitInfo transfer_buffer_submit_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,

      .commandBuffer = transfer_buffer
    };
    
    VkSubmitInfo2 transfer_submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &transfer_buffer_submit_info,
    };

    vkQueueSubmit2(target_queue, 1, &transfer_submit_info, nullptr);

    vkQueueWaitIdle(target_queue);

    vmaDestroyBuffer(allocator, staging_buffer, staging_allocation);
    vkFreeCommandBuffers(logical_device, command_pool,
                         1, &transfer_buffer);

    return buffer_to_return;
  }


  void CleanMeshCPUData(Xenith::Graphics::Mesh &mesh)
  {
    mesh.cpu_vertices.clear();
    mesh.cpu_vertices.shrink_to_fit();

    mesh.cpu_indices.clear();
    mesh.cpu_indices.shrink_to_fit();
  }


  void UploadMeshBuffersToVRAMStaticSingle(Xenith::Graphics::Device logical_device, Xenith::Graphics::Allocator allocator,
                                           Xenith::Graphics::Queue graphics_queue, Xenith::Graphics::CommandPool command_pool,
                                           std::vector<Xenith::Graphics::Vertex3D> &cpu_vertices, std::vector<uint32_t> &cpu_indices,
                                           Xenith::Graphics::VertexAndIndexBuffers &vertex_and_index_buffers_to_initialize,
                                           const bool clean_cpu_data, const Xenith::Debug::LogVerbosity log_verbosity)
  {
    vertex_and_index_buffers_to_initialize.vertex_buffer = UploadBufferToVRAMStatic(
        logical_device, allocator, 
        graphics_queue,
        command_pool, cpu_vertices.data(),
        cpu_vertices.size() * sizeof(Xenith::Graphics::Vertex3D), 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
    );

    vertex_and_index_buffers_to_initialize.index_buffer = UploadBufferToVRAMStatic(
        logical_device, allocator,
        graphics_queue, 
        command_pool, cpu_indices.data(),
        cpu_indices.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    if(clean_cpu_data)
    {
      cpu_vertices.clear();
      cpu_vertices.shrink_to_fit();

      cpu_indices.clear();
      cpu_indices.shrink_to_fit();


      if(log_verbosity == Xenith::Debug::LogVerbosity::HIGH ||
         log_verbosity == Xenith::Debug::LogVerbosity::MEDIUM)
      {
        SDL_Log("Cleared mesh cpu data");
      }
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      SDL_Log("Mesh buffers successfully uploaded to VRAM");
  }


} // Xenith::Graphics

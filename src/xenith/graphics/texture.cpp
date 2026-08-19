#include "xenith/graphics/texture.h"

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logging.h"
#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"

#include "xenith/graphics/engine/engine_render/render_engine.h"


namespace Xenith::Graphics
{
  ImageData LoadImage(const std::string &path)
  {
    ImageData image{};

    image.pixels = stbi_load(
        path.c_str(),
        &image.width,
        &image.height,
        &image.channels,
        STBI_rgb_alpha
    );
    image.channels = 4;

    if(image.width == 0 || image.height == 0 || image.channels == 0)
    {
      Xenith::Debug::Log("Warning, image could possible be loaded wrong!");
      Xenith::Debug::Log("Path: %s", path.c_str());

      Xenith::Debug::Log("Width: %d", image.width);
      Xenith::Debug::Log("Height: %d", image.height);
      Xenith::Debug::Log("Width: %d", image.channels);
    }


    return image;
  }


  Xenith::Graphics::Texture CreateTexture2D(Xenith::Graphics::Device logical_device,
                                            Xenith::Graphics::Allocator allocator,
                                            const Xenith::Graphics::ImageData &image_data)
  {
    Xenith::Graphics::Texture texture_to_return;

    texture_to_return.format = VK_FORMAT_R8G8B8A8_SRGB;
    texture_to_return.width = static_cast<uint32_t>(image_data.width);
    texture_to_return.height = static_cast<uint32_t>(image_data.height);


    // Image
    VkImageCreateInfo image_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,

      .imageType = VK_IMAGE_TYPE_2D,
      .format = texture_to_return.format,
      .extent = { .width = texture_to_return.width,
                  .height = texture_to_return.height,
                  .depth = 1},

      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo image_allocation_create_info = {
      .usage = VMA_MEMORY_USAGE_AUTO,
      .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

    vmaCreateImage(allocator, &image_create_info, &image_allocation_create_info,
                   &texture_to_return.handle, &texture_to_return.allocation, nullptr);

    // Image view
    VkImageViewCreateInfo image_view_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

      .image = texture_to_return.handle,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,

      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };
    if(vkCreateImageView(logical_device, &image_view_create_info, 
                         nullptr, &texture_to_return.view) != VK_SUCCESS)
      Xenith::Debug::Log("Failed to create texture image view!");


    return texture_to_return;
  }


  void UploadTextureToVRAMStaticSingle(Xenith::Graphics::Device logical_device,
                                       Xenith::Graphics::Allocator allocator,
                                       Xenith::Graphics::CommandPool command_pool,
                                       Xenith::Graphics::Queue graphics_queue,
                                       const Xenith::Graphics::Texture &texture, const void* pixel_data)
  {
    VkDeviceSize image_size = texture.width * texture.height * 4;

    VkBuffer staging_buffer;
    VmaAllocation staging_allocation;

    VkBufferCreateInfo staging_buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,

      .size = image_size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo staging_allocation_create_info = {
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO
    };

    vmaCreateBuffer(allocator, &staging_buffer_create_info, &staging_allocation_create_info,
                    &staging_buffer, &staging_allocation, nullptr);

    void* data;
    vmaMapMemory(allocator, staging_allocation, &data);
    std::memcpy(data, pixel_data, static_cast<size_t>(image_size));
    vmaUnmapMemory(allocator, staging_allocation);

    VkCommandBuffer transfer_command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo transfer_command_buffer_allocation_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(logical_device, &transfer_command_buffer_allocation_info,
                             &transfer_command_buffer);

    Xenith::Graphics::BeginCommandBuffer(transfer_command_buffer);

    VkImageMemoryBarrier2 layout_barrier_to_transfer = {

        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask  = VK_PIPELINE_STAGE_2_COPY_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image         = texture.handle,
        .subresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    Xenith::Graphics::PipelineBarrier(transfer_command_buffer, 1,
                                      &layout_barrier_to_transfer);

    VkBufferImageCopy copy_region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .imageOffset = { 0, 0, 0 },
      .imageExtent = { .width = texture.width, .height = texture.height, .depth = 1 }
    };

    vkCmdCopyBufferToImage(transfer_command_buffer, staging_buffer,
                           texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &copy_region);

    VkImageMemoryBarrier2 layout_barrier_to_shader = layout_barrier_to_transfer;
    layout_barrier_to_shader.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    layout_barrier_to_shader.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    layout_barrier_to_shader.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    layout_barrier_to_shader.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    layout_barrier_to_shader.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    layout_barrier_to_shader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    Xenith::Graphics::PipelineBarrier(transfer_command_buffer, 1,
                                      & layout_barrier_to_shader);

    Xenith::Graphics::EndCommandBuffer(transfer_command_buffer);

    VkCommandBufferSubmitInfo transfer_buffer_submit_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,

      .commandBuffer = transfer_command_buffer
    };

    VkSubmitInfo2 transfer_submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &transfer_buffer_submit_info
    };
    vkQueueSubmit2(graphics_queue, 1, &transfer_submit_info, nullptr);

    vkQueueWaitIdle(graphics_queue);

    vmaDestroyBuffer(allocator, staging_buffer, staging_allocation);
    vkFreeCommandBuffers(logical_device, command_pool,
                         1, &transfer_command_buffer);
  }


  Xenith::Graphics::TextureSampler CreateTextureSampler(Xenith::Graphics::Device logical_device,
                                                        const Xenith::Graphics::TextureSamplerConfig &texture_sampler_config)
  {
    VkSampler texture_sampler_to_return;

    VkSamplerCreateInfo image_texture_sampler_create_info = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

      .magFilter = texture_sampler_config.mag_filter,
      .minFilter = texture_sampler_config.min_filter,

      .mipmapMode = texture_sampler_config.mipmap_mode,

      .addressModeU = texture_sampler_config.address_mode_u,
      .addressModeV = texture_sampler_config.address_mode_v,
      .addressModeW = texture_sampler_config.address_mode_w,
    };
    if(vkCreateSampler(logical_device, &image_texture_sampler_create_info, 
                    nullptr,  &texture_sampler_to_return) != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create texture sampler!");

      return VK_NULL_HANDLE;
    }

    return texture_sampler_to_return;
  }


  Xenith::Graphics::DescriptorSetLayout CreateBindlessDescriptorSetLayout(Xenith::Graphics::Device logical_device,
                                                                          uint32_t max_bindless_textures)
  {
    VkDescriptorSetLayout bindless_descriptor_set_layout_to_return;

    VkDescriptorSetLayoutBinding bindless_descriptor_set_layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = max_bindless_textures,

      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr
    };

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = nullptr,

      .bindingCount = 1,
      .pBindingFlags = &binding_flags
    };

    VkDescriptorSetLayoutCreateInfo bindless_descriptor_set_layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &layout_flags_create_info,

      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = 1,
      .pBindings = &bindless_descriptor_set_layout_binding
    };
    if(vkCreateDescriptorSetLayout(logical_device, &bindless_descriptor_set_layout_create_info, 
                                   nullptr, &bindless_descriptor_set_layout_to_return) != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create descriptor set layout");

      return VK_NULL_HANDLE;
    }

    return bindless_descriptor_set_layout_to_return;
  }


  Xenith::Graphics::DescriptorPool CreateBindlessDescriptorPool(Xenith::Graphics::Device logical_device,
                                                                const uint32_t max_bindless_textures)
  {
    VkDescriptorPool bindless_pool_to_return = VK_NULL_HANDLE;

    VkDescriptorPoolSize bindless_pool_size = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = max_bindless_textures
    };

    VkDescriptorPoolCreateInfo bindless_descriptor_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,

      .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &bindless_pool_size
    };

    if(vkCreateDescriptorPool(logical_device, &bindless_descriptor_pool_create_info, 
                              nullptr, &bindless_pool_to_return) != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create bindless descriptor pool");

      return VK_NULL_HANDLE;
    }

    return bindless_pool_to_return;
  }


  Xenith::Graphics::DescriptorSet CreateBindlessDescriptorSet(Xenith::Graphics::Device logical_device,
                                                              Xenith::Graphics::DescriptorSetLayout bindless_descriptor_set_layout,
                                                              Xenith::Graphics::DescriptorPool bindless_descriptor_pool)
  {
    VkDescriptorSet bindless_descriptor_set_to_return;

    VkDescriptorSetAllocateInfo bindless_descriptor_set_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,

      .descriptorPool = bindless_descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &bindless_descriptor_set_layout
    };
    if(vkAllocateDescriptorSets(logical_device, &bindless_descriptor_set_allocate_info, 
                                &bindless_descriptor_set_to_return) != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create and allocate bindless descriptor set!");

      return VK_NULL_HANDLE;
    }

    return bindless_descriptor_set_to_return;
  }


  void UpdateBindlessDescriptorSetsStatic(Xenith::Graphics::Device logical_device,
                                          Xenith::Graphics::DescriptorSet bindless_descriptor_set, 
                                          Xenith::Graphics::TextureSampler texture_sampler,
                                          const Xenith::Graphics::Texture* textures_data,
                                          const size_t textures_array_size,
                                          const uint32_t starting_bindless_array_id)
  {
    std::vector<VkDescriptorImageInfo> bindless_descriptor_image_infos;
    bindless_descriptor_image_infos.reserve(textures_array_size);

    for(uint32_t i = 0; i < textures_array_size; i++)
    {
      bindless_descriptor_image_infos.push_back({
        .sampler = texture_sampler,
        .imageView = textures_data[i].view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      });
    }

    VkWriteDescriptorSet bindless_write_descriptor_set = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,

      .dstSet = bindless_descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = starting_bindless_array_id,

      .descriptorCount = static_cast<uint32_t>(textures_array_size),
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = bindless_descriptor_image_infos.data()
    };

    vkUpdateDescriptorSets(logical_device, 1, &bindless_write_descriptor_set,
                           0, nullptr);
  }

  void UpdateBindlessDescriptorSetsDynamic();


  void DestroyTexture(Xenith::Graphics::Device logical_device,
                      Xenith::Graphics::Allocator allocator,
                      Xenith::Graphics::Texture &texture_to_destroy, 
                      const Xenith::Debug::LogVerbosity log_verbosity)
  {
    // Depth texture
    // View
    if(texture_to_destroy.view != VK_NULL_HANDLE)
    {
      vkDestroyImageView(logical_device, texture_to_destroy.view,
                         nullptr);
      texture_to_destroy.view = VK_NULL_HANDLE;

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Texture image view destroyed");
    }

    // Handle
    if(texture_to_destroy.handle != VK_NULL_HANDLE)
    {
      vmaDestroyImage(allocator,
                      texture_to_destroy.handle, 
                      texture_to_destroy.allocation);
      texture_to_destroy.handle = VK_NULL_HANDLE;
      texture_to_destroy.allocation = nullptr;

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Texture image destroyed");
    }
  }
}


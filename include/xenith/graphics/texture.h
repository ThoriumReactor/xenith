#pragma once

#include <cstdint>
#include <string>

#include "stbi/stb_image.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  struct ImageData
  {
    uint8_t* pixels;
    int width;
    int height;
    int channels;
  };


  ImageData LoadImage(const std::string &path);

  Xenith::Graphics::Texture CreateTexture2D(Xenith::Graphics::Device logical_device,
                                            Xenith::Graphics::Allocator allocator,
                                            const Xenith::Graphics::ImageData &image_data);

  void UploadTextureToVRAMStaticSingle(Xenith::Graphics::Device logical_device,
                                       Xenith::Graphics::Allocator allocator,
                                       Xenith::Graphics::CommandPool command_pool,
                                       Xenith::Graphics::Queue graphics_queue,
                                       const Xenith::Graphics::Texture &texture, const void* pixel_data);

  /// Creates texture sampler
  /// if failed, throwns an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::TextureSampler CreateTextureSampler(Xenith::Graphics::Device logical_device,
                                                        const Xenith::Graphics::TextureSamplerConfig &texture_sampler_config);

  /// Creates bindless descripto set layout
  /// if failed, throwns an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::DescriptorSetLayout CreateBindlessDescriptorSetLayout(Xenith::Graphics::Device logical_device,
                                                                          uint32_t max_bindless_textures);
  
  /// Creates bindless descriptor pool
  /// if failed, throwns an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::DescriptorPool CreateBindlessDescriptorPool(Xenith::Graphics::Device logical_device,
                                                                const uint32_t max_bindless_textures);

  /// Creates bindless descriptor set
  /// if failed, throwns an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::DescriptorSet CreateBindlessDescriptorSet(Xenith::Graphics::Device logical_device,
                                                              Xenith::Graphics::DescriptorSetLayout bindless_descriptor_set_layout,
                                                              Xenith::Graphics::DescriptorPool bindless_descriptor_pool);

  void UpdateBindlessDescriptorSetsStatic(Xenith::Graphics::Device logical_device,
                                          Xenith::Graphics::DescriptorSet bindless_descriptor_set, 
                                          Xenith::Graphics::TextureSampler texture_sampler,
                                          const Xenith::Graphics::Texture* textures_data,
                                          const size_t textures_array_size,
                                          const uint32_t starting_bindless_array_id);


  void DestroyTexture(Xenith::Graphics::Device logical_device,
                      Xenith::Graphics::Allocator allocator,
                      Xenith::Graphics::Texture &texture_to_destroy, 
                      const Xenith::Debug::LogVerbosity log_verbosity);
}

#pragma once

#include <cstdint>

#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  /// Helper function to load file
  std::vector<char> ReadFile(const std::string &file_path);

  /// Creates depth texture
  /// If failed, throws an error log and returns Texture.handle = VK_NULL_HANDLE
  Xenith::Graphics::Texture CreateDepthTexture(Xenith::Graphics::Device logical_device,
                                               Xenith::Graphics::Allocator allocator,
                                               uint32_t width, uint32_t height,
                                               Xenith::Graphics::DepthFormat depth_format,
                                               const Xenith::Debug::LogVerbosity log_verbosity);

  /// Loads shader
  /// If failed, throws an error log and returns Shader.module = VK_NULL_HANDLE
  Xenith::Graphics::Shader LoadShader(Xenith::Graphics::Device logical_device, 
                                      const std::string &shader_path,
                                      const std::string entrypoint_name,
                                      Xenith::Graphics::ShaderTypeFlag shader_type_flags);

  /// Creates pipeline layout
  /// If failed, throws an error log and returns PipelineLayout.handle = VK_NULL_HANDLE
  Xenith::Graphics::PipelineLayout CreatePipelineLayout(Xenith::Graphics::Device logical_device,
                                                        Xenith::Graphics::ShaderTypeFlag push_constant_shader_type_flags,
                                                        uint32_t push_constant_size,
                                                        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE);

  /// Loads default vertex layout
  Xenith::Graphics::VertexLayout LoadEngineDefaultVertexLayoutTexture();
  Xenith::Graphics::VertexLayout LoadEngineDefaultVertexLayoutColor();

  /// Loads default color blending 
  Xenith::Graphics::ColorBlendConfig LoadEngineDefaultColorBlendConfig();

  /// Loads default depth config
  Xenith::Graphics::DepthConfig LoadEngineDefaultDepthConfig();

  /// Creates graphics pipeline 
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::Pipeline CreateGraphicsPipeline(Xenith::Graphics::Device logical_device,
                                                    Xenith::Graphics::Shader &vertex_shader,
                                                    Xenith::Graphics::Shader &fragment_shader,
                                                    const Xenith::Graphics::PipelineLayout &pipeline_layout,
                                                    const Xenith::Graphics::VertexLayout &vertex_layout,
                                                    const Xenith::Graphics::GraphicsPipelineConfig &graphics_pipeline_config,
                                                    const Xenith::Debug::LogVerbosity log_verbosity);
}

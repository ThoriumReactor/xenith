#include "xenith/graphics/engine/engine_destroy/pipeline_destroy.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/debug_xn/logging.h"
#include "xenith/graphics/graphics_vk.h"
#include "xenith/graphics/texture.h"


namespace Xenith::Graphics
{
  void DestroyGraphicsPipelineContext(Xenith::EngineECS::GraphicsPipelineContext &pipeline_context,
                                      Xenith::Graphics::Device logical_device,
                                      Xenith::Graphics::Allocator allocator,
                                      const Xenith::Debug::LogVerbosity log_verbosity)
  {
    // Graphics pipeline
    if(pipeline_context.graphics_pipeline != VK_NULL_HANDLE) 
    {
      vkDestroyPipeline(logical_device, pipeline_context.graphics_pipeline,
                        nullptr);

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
        Xenith::Debug::Log("Graphics pipeline destroyed");
    }

    // Pipeline layout
    if(pipeline_context.pipeline_layout.handle != VK_NULL_HANDLE)
    {
      vkDestroyPipelineLayout(logical_device, pipeline_context.pipeline_layout.handle,
                              nullptr);

      if(log_verbosity == Xenith::Debug::LogVerbosity::MEDIUM ||
         log_verbosity == Xenith::Debug::LogVerbosity::HIGH)
        Xenith::Debug::Log("Pipeline layout destroyed");
    }

    // Vertex shader module
    if(pipeline_context.vertex_shader.module != VK_NULL_HANDLE)
    {
      vkDestroyShaderModule(logical_device, pipeline_context.vertex_shader.module,
                            nullptr);

      if(log_verbosity == Xenith::Debug::LogVerbosity::HIGH)
        Xenith::Debug::Log("Vertex shader module destroyed");
    }

    // Fragment shader module
    if(pipeline_context.fragment_shader.module != VK_NULL_HANDLE)
    {
      vkDestroyShaderModule(logical_device, pipeline_context.fragment_shader.module,
                            nullptr);

      if(log_verbosity == Xenith::Debug::LogVerbosity::HIGH)
        Xenith::Debug::Log("Fragment shader module destroyed");
    }

    DestroyTexture(logical_device, allocator, pipeline_context.depth_texture, log_verbosity);
  }
}

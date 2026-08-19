#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include "xenith/display/display.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::EngineECS
{
  struct CoreContext
  {
    Xenith::Graphics::Instance instance               = VK_NULL_HANDLE;
    Xenith::Graphics::PhysicalDevice physical_device  = VK_NULL_HANDLE;
    Xenith::Graphics::Surface surface                 = VK_NULL_HANDLE;
    Xenith::Graphics::Device device                   = VK_NULL_HANDLE;
    Xenith::Graphics::Allocator allocator             = VK_NULL_HANDLE;

    Xenith::Display::Window window  = nullptr;
  };

  struct GraphicsContext
  {
    Xenith::Graphics::Queue graphics_queue  = VK_NULL_HANDLE;
    uint32_t graphics_queue_family_index    = 0;
  };

  struct SwapchainContext
  {
    Xenith::Graphics::Swapchain swapchain  = VK_NULL_HANDLE;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    Xenith::Graphics::SurfaceCapabilities surface_capabilities;
    Xenith::Graphics::SurfaceFormat surface_format;

    uint32_t width   = 0;
    uint32_t height  = 0;
  };

  struct GraphicsPipelineContext
  {
    Xenith::Graphics::Texture depth_texture;

    Xenith::Graphics::Shader vertex_shader;
    Xenith::Graphics::Shader fragment_shader;

    Xenith::Graphics::PipelineLayout pipeline_layout{};
    Xenith::Graphics::Pipeline       graphics_pipeline  = VK_NULL_HANDLE;
  };

  struct RenderContext
  {
    Xenith::Graphics::Semaphore timeline_semaphore = VK_NULL_HANDLE;
    std::vector<Xenith::Graphics::CommandResources> command_resources;

    std::vector<Xenith::Graphics::Semaphore> render_finished_semaphores;
    std::vector<Xenith::Graphics::Semaphore> image_acquired_semaphores;

    // render
    uint64_t frame_index        = 0;
    uint64_t next_signal_value;
  };


  struct EngineContext 
  {
    Xenith::EngineECS::CoreContext core_ctx;
    Xenith::EngineECS::GraphicsContext graphics_ctx;
    Xenith::EngineECS::SwapchainContext swap_ctx;
    Xenith::EngineECS::GraphicsPipelineContext pipeline_ctx;
    Xenith::EngineECS::RenderContext render_ctx;
  };
};

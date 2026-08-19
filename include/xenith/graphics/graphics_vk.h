#pragma once

#include <array>
#include <cfloat>
#include <cstdint>
#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"

#include "xenith/math/math.h"

#include "vma/vk_mem_alloc.h"


namespace Xenith::Graphics
{
  // Data types
  using Instance        = VkInstance;
  using PhysicalDevice  = VkPhysicalDevice;
  using Device          = VkDevice;
  using Queue           = VkQueue;

  using Allocator   = VmaAllocator;
  using Allocation  = VmaAllocation;

  using Surface = VkSurfaceKHR;
  using SurfaceCapabilities = VkSurfaceCapabilitiesKHR;
  using SurfaceFormat = VkSurfaceFormatKHR;

  using Swapchain  = VkSwapchainKHR;

  using ImageMemoryBarrier = VkImageMemoryBarrier2;

  using StatusResult = VkResult;
  namespace StatusResults
  {
    constexpr StatusResult SUCCESS  = VK_SUCCESS;

    constexpr StatusResult ERROR_OUT_OF_DATE  = VK_ERROR_OUT_OF_DATE_KHR;
    constexpr StatusResult SUBOPTIMAL         = VK_SUBOPTIMAL_KHR;
  }


  using Pipeline = VkPipeline;
  struct PipelineLayout
  { 
    VkPipelineLayout       handle                 = VK_NULL_HANDLE;
    VkDescriptorSetLayout  descriptor_set_layout  = VK_NULL_HANDLE;
  };


  using Semaphore = VkSemaphore;

  using CommandBuffer = VkCommandBuffer;
  using CommandPool = VkCommandPool;

  struct Buffer
  {
    VkBuffer handle                          = VK_NULL_HANDLE;
    Xenith::Graphics::Allocation allocation  = nullptr;
    VkDeviceSize size                        = 0;
  };


  using TextureSampler = VkSampler;

  struct TextureSamplerConfig
  {
    VkFilter mag_filter;
    VkFilter min_filter;

    VkSamplerMipmapMode mipmap_mode;

    VkSamplerAddressMode address_mode_u;
    VkSamplerAddressMode address_mode_v;
    VkSamplerAddressMode address_mode_w;
  };

  struct Texture
  {
    VkImage handle = VK_NULL_HANDLE;
    Xenith::Graphics::Allocation allocation = nullptr;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;

    uint32_t width = 0;
    uint32_t height = 0;
  };


  using DescriptorSetLayout = VkDescriptorSetLayout;
  using DescriptorPool = VkDescriptorPool;
  using DescriptorSet = VkDescriptorSet;


  using ShaderTypeFlag = VkShaderStageFlagBits;
  namespace ShaderTypeFlags
  {
    constexpr ShaderTypeFlag VERTEX    = VK_SHADER_STAGE_VERTEX_BIT;
    constexpr ShaderTypeFlag FRAGMENT  = VK_SHADER_STAGE_FRAGMENT_BIT;
    constexpr ShaderTypeFlag COMPUTE   = VK_SHADER_STAGE_COMPUTE_BIT;
  }

  struct Shader
  {
    VkShaderModule module;
    VkPipelineShaderStageCreateInfo pipeline_stage_info;
    VkShaderStageFlagBits type;

    char entrypoint_name[64] = {};
  };

  struct Vertex3D
  {
    Xenith::Math::Vec3 position;
    Xenith::Math::Vec3 normals;
    std::array<uint8_t, 4> color;
    Xenith::Math::Vec2 uv;

    uint32_t local_texture_id;
  };


  struct VertexAndIndexBuffers 
  {
    Xenith::Graphics::Buffer vertex_buffer;
    Xenith::Graphics::Buffer index_buffer;
  };

  struct AxisAlignedBoundingBox
  {
    Xenith::Math::Vec3 min;
    Xenith::Math::Vec3 max;
  };

  struct Mesh
  {
    std::vector<Xenith::Graphics::Vertex3D> cpu_vertices;
    std::vector<uint32_t> cpu_indices;
    uint32_t index_count = 0;

    Xenith::Graphics::VertexAndIndexBuffers buffers;

    Xenith::Graphics::AxisAlignedBoundingBox local_aabb;
  };


  struct VertexLayout
  {
    std::vector<VkVertexInputBindingDescription> binding_description;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
  };


  struct CommandResources
  {
    Xenith::Graphics::CommandPool command_pool            = VK_NULL_HANDLE;
    Xenith::Graphics::CommandBuffer command_buffer        = VK_NULL_HANDLE;
  };

  struct SwapchainAcquisitionResult
  {
    Xenith::Graphics::StatusResult result;
    uint32_t image_index;
  };


  struct ApplicationInfo
  {
    const char* application_name;
    uint32_t application_version;
  };

  struct InstanceConfig
  {
    ApplicationInfo application_info;
    
    bool validation_layers_toggle;
  };


  using PhysicalDeviceType = VkPhysicalDeviceType;
  namespace PhysicalDeviceTypes
  {
    constexpr PhysicalDeviceType OTHER           = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    constexpr PhysicalDeviceType INTEGRATED_GPU  = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    constexpr PhysicalDeviceType DISCRETE_GPU    = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    constexpr PhysicalDeviceType VIRTUAL_GPU     = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    constexpr PhysicalDeviceType CPU             = VK_PHYSICAL_DEVICE_TYPE_CPU;
  }

  struct PhysicalDeviceSelectConfig
  {
    Xenith::Graphics::PhysicalDeviceType physical_device_type_priority;
  };

  struct LogicalDeviceConfig
  {
    uint32_t graphics_queue_family_index  = 0xFFFFFFFF;
    uint32_t compute_queue_family_index   = 0xFFFFFFFF;
    uint32_t transfer_queue_family_index  = 0xFFFFFFFF;
  };

  
  enum class SurfaceColorMode
  {
    SDR_SRGB,

    HDR_10_ST2084
  };

  struct SwapchainSurfaceFormatConfig
  {
    SurfaceColorMode color_mode;
  };


  struct PipelineLayoutConfig
  {
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    std::vector<VkPushConstantRange> push_constant_ranges;
  };

  using DepthFormat = VkFormat;
  namespace DepthFormats
  {
    constexpr DepthFormat FLOAT_32                 = VK_FORMAT_D32_SFLOAT;
    constexpr DepthFormat FLOAT_32_STENCIL_UINT_8  = VK_FORMAT_D32_SFLOAT_S8_UINT;

    constexpr DepthFormat INT_24_STENCIL_UINT_8  = VK_FORMAT_D24_UNORM_S8_UINT;
  }
  

  using PrimitiveTopology = VkPrimitiveTopology;
  namespace PrimitiveTopologies
  {
    constexpr PrimitiveTopology POINT_LIST  = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    constexpr PrimitiveTopology LINE_LIST   = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    constexpr PrimitiveTopology LINE_STRIP  = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    constexpr PrimitiveTopology TRIANGLE_LIST   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    constexpr PrimitiveTopology TRIANGLE_STRIP  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    constexpr PrimitiveTopology TRIANGLE_FAN    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
  }

  using FillMode = VkPolygonMode;
  namespace FillModes
  {
    constexpr FillMode FILL   = VK_POLYGON_MODE_FILL;
    constexpr FillMode LINE   = VK_POLYGON_MODE_LINE;
    constexpr FillMode POINT  = VK_POLYGON_MODE_POINT;
  }

  using CullModeFlag = VkCullModeFlags;
  namespace CullModeFlags
  {
    constexpr CullModeFlag NONE   = VK_CULL_MODE_NONE;
    constexpr CullModeFlag FRONT  = VK_CULL_MODE_FRONT_BIT;
    constexpr CullModeFlag BACK   = VK_CULL_MODE_BACK_BIT;
  }

  struct RasterizerConfig
  {
    Xenith::Graphics::PrimitiveTopology primitive_topology;

    Xenith::Graphics::FillMode fill_mode;
    Xenith::Graphics::CullModeFlag cull_mode_flags;
  };

  struct ColorBlendAttachmentState
  {
    VkBool32 enable_blend;
    
    VkBlendFactor src_color_blend_factor;
    VkBlendFactor dst_color_blend_factor;
    VkBlendOp color_blend_op;
    
    VkBlendFactor src_alpha_blend_factor;
    VkBlendFactor dst_alpha_blend_factor;
    VkBlendOp alpha_blend_op;

    VkColorComponentFlags color_write_mask;
  };
  struct ColorBlendConfig
  {
    std::vector<Xenith::Graphics::ColorBlendAttachmentState> attachment_states;
  };

  struct DepthConfig 
  {
    VkCompareOp compare_op;
    VkBool32 enable_depth_test;
    VkBool32 enable_depth_write;
    VkBool32 enable_depth_stencil;
  };

  struct GraphicsPipelineConfig
  {
    VkFormat color_attachment_format;
    VkFormat depth_attachment_format;

    Xenith::Graphics::RasterizerConfig rasterizer_config;
    Xenith::Graphics::ColorBlendConfig color_blend_config;
    Xenith::Graphics::DepthConfig depth_config;
  };



  struct ColorAttachmentConfig
  {
    VkImageView swapchain_image_view;
    VkImageLayout image_layout;

    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;

    Xenith::Math::Vec4 clear_color_value;
  };

  struct DepthAttachmentConfig 
  {
    VkImageView image_view;
    VkImageLayout image_layout;

    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;

    VkClearDepthStencilValue clear_depth_stencil_value;
  };

  struct RenderingConfig
  {
    uint32_t render_area_width   = 0;
    uint32_t render_area_height  = 0;
    uint32_t layer_count         = 1;

    const ColorAttachmentConfig* color_attachment_config_data  = nullptr;
    uint32_t color_attachment_count                            = 1;

    DepthAttachmentConfig depth_attachment_config;
  };

  using PipelineBindPoint = VkPipelineBindPoint;
  namespace PipelineBindPoints
  {
    constexpr PipelineBindPoint GRAPHICS  = VK_PIPELINE_BIND_POINT_GRAPHICS;
    constexpr PipelineBindPoint COMPUTE   = VK_PIPELINE_BIND_POINT_COMPUTE;
  }


  struct FrameSubmitConfig 
  {
    Xenith::Graphics::Semaphore image_acquired_semaphore = VK_NULL_HANDLE;
    Xenith::Graphics::Semaphore current_render_finished_semaphore = VK_NULL_HANDLE;

    Xenith::Graphics::Semaphore timeline_semaphore = VK_NULL_HANDLE;
    uint64_t signal_value = 0;
  };

  struct FramePresentConfig
  {
    Xenith::Graphics::Swapchain swapchain = VK_NULL_HANDLE;
    uint32_t swapchain_image_index;

    Xenith::Graphics::Semaphore current_render_finished_semaphore = VK_NULL_HANDLE;
  };


  struct ModelData
  {
    fastgltf::Asset asset;
    fastgltf::GltfDataBuffer data_buffer;
  };
} // namespace Xenith::Graphics

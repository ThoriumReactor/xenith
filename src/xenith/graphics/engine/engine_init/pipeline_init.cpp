#include "xenith/graphics/engine/engine_init/pipeline_init.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "xenith/debug_xn/logging.h"
#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"



namespace Xenith::Graphics
{
  std::vector<char> ReadFile(const std::string &file_path)
  {
    std::ifstream file{file_path, std::ios::ate | std::ios::binary};

    if(!file.is_open())
      Xenith::Debug::Log("Failed to open file: %s", file_path.c_str());

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
  }


  Xenith::Graphics::Texture CreateDepthTexture(Xenith::Graphics::Device logical_device,
                                               Xenith::Graphics::Allocator allocator,
                                               uint32_t width, uint32_t height,
                                               Xenith::Graphics::DepthFormat depth_format,
                                               const Xenith::Debug::LogVerbosity log_verbosity)
  {
    Xenith::Graphics::Texture texture_to_return;

    VkImage &depth_image = texture_to_return.handle;
    VkImageView &depth_image_view = texture_to_return.view;
    VmaAllocation &depth_image_allocation = texture_to_return.allocation;

    VkImageCreateInfo depth_image_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,

      .imageType = VK_IMAGE_TYPE_2D,
      .format = depth_format,
      .extent = { .width = width, .height = height, .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo depth_allocation_create_info = {
      .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkResult depth_image_result = vmaCreateImage(allocator, &depth_image_create_info,
                                                 &depth_allocation_create_info,
                                                 &depth_image,
                                                 &depth_image_allocation,
                                                 nullptr);
    if(depth_image_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create depth image! Error code: %d", depth_image_result);
      texture_to_return.handle = VK_NULL_HANDLE;
      return texture_to_return;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Depth image created");


    // Depth image view
    VkImageViewCreateInfo depth_image_view_create_info = 
    {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

      .image = depth_image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = depth_format,

      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .levelCount = 1,
        .layerCount = 1
      }
    };

    texture_to_return.format = depth_format;

    VkResult depth_image_view_result = vkCreateImageView(logical_device, &depth_image_view_create_info,
                                                         nullptr, &depth_image_view);
    if(depth_image_view_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create depth image view! Error code: %d", depth_image_view_result);
      texture_to_return.handle = VK_NULL_HANDLE;
      texture_to_return.view = VK_NULL_HANDLE;
      return texture_to_return;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Depth image view created");

    return texture_to_return;
  }


  Xenith::Graphics::Shader LoadShader(Xenith::Graphics::Device logical_device, 
                                      const std::string &shader_path,
                                      const std::string entrypoint_name,
                                      Xenith::Graphics::ShaderTypeFlag shader_type_flags)
  {
    Xenith::Graphics::Shader shader_to_return;

    auto shader_code = ReadFile(shader_path);
    VkShaderModuleCreateInfo vert_shader_module_create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      
      .codeSize = shader_code.size(),
      .pCode = reinterpret_cast<const uint32_t*>(shader_code.data())
    };

    VkResult shader_result = vkCreateShaderModule(logical_device,
                                                  &vert_shader_module_create_info,
                                                  nullptr,
                                                  &shader_to_return.module);
    if(shader_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create shader! Error code: %d", shader_result);
      shader_to_return.module = VK_NULL_HANDLE;
      return shader_to_return;
    }

    // Shader stage
    strncpy(shader_to_return.entrypoint_name, 
            entrypoint_name.c_str(),
            sizeof(shader_to_return.entrypoint_name) - 1);

    shader_to_return.type = shader_type_flags;

    VkPipelineShaderStageCreateInfo shader_pipeline_stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,

      .stage = shader_to_return.type,
      .module = shader_to_return.module,
      .pName = shader_to_return.entrypoint_name
    };
    shader_to_return.pipeline_stage_info = shader_pipeline_stage_info;

    return shader_to_return;
  }


  Xenith::Graphics::PipelineLayout CreatePipelineLayout(Xenith::Graphics::Device logical_device,
                                                        Xenith::Graphics::ShaderTypeFlag push_constant_shader_type_flags,
                                                        uint32_t push_constant_size,
                                                        VkDescriptorSetLayout descriptor_set_layout)
  {
    Xenith::Graphics::PipelineLayout pipeline_layout_to_return;
    VkPipelineLayout pipeline_layout_to_create = VK_NULL_HANDLE;

    uint32_t set_layout_count;
    if(descriptor_set_layout == VK_NULL_HANDLE)
      set_layout_count = 0;
    else
      set_layout_count = 1;


    VkPushConstantRange push_constant_range = {
      .stageFlags = push_constant_shader_type_flags,
      .offset = 0,
      .size = push_constant_size
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

      .setLayoutCount = set_layout_count,
      .pSetLayouts = &descriptor_set_layout,

      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range
    };

    VkResult pipeline_layout_result = vkCreatePipelineLayout(logical_device, 
                                                             &pipeline_layout_create_info,
                                                             nullptr,
                                                             &pipeline_layout_to_create);
    if(pipeline_layout_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create pipeline layout! Error code: %d", pipeline_layout_result);

      pipeline_layout_to_return.handle = VK_NULL_HANDLE;
      return pipeline_layout_to_return;
    }

    pipeline_layout_to_return = {
      .handle = pipeline_layout_to_create,
      .descriptor_set_layout = descriptor_set_layout
    };

    return pipeline_layout_to_return;
  }


  Xenith::Graphics::VertexLayout LoadEngineDefaultVertexLayoutTexture()
  {
    VertexLayout vertex_layout_default;
    
    // Binding Description
    vertex_layout_default.binding_description = {
      {
        .binding = 0, // Buffer slot index
        .stride = sizeof(Xenith::Graphics::Vertex3D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
      }
    }; // Bindind Description

    // Attribute Descriptions
    vertex_layout_default.attribute_descriptions = {
      // Position attribute
      {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, position)
      },

      // Normal attribute
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, normals)
      },

      // Color attribute
      {
        .location = 2,
        .binding = 0,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .offset = offsetof(Xenith::Graphics::Vertex3D, color)
      },

      // UV attribute
      {
        .location = 3,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, uv)
      },

      // Local texture ID
      {
        .location = 4,
        .binding = 0,
        .format = VK_FORMAT_R32_UINT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, local_texture_id)
      }
    }; // Attribute Descriptions

    return vertex_layout_default;
  }

  Xenith::Graphics::VertexLayout LoadEngineDefaultVertexLayoutColor()
  {
    VertexLayout vertex_layout_default;
    
    // Binding Description
    vertex_layout_default.binding_description = {
      {
        .binding = 0, // Buffer slot index
        .stride = sizeof(Xenith::Graphics::Vertex3D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
      }
    }; // Bindind Description

    // Attribute Descriptions
    vertex_layout_default.attribute_descriptions = {
      // Position attribute
      {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, position)
      },

      // Normal attribute
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Xenith::Graphics::Vertex3D, normals)
      },

      // Color attribute
      {
        .location = 2,
        .binding = 0,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .offset = offsetof(Xenith::Graphics::Vertex3D, color)
      },
    }; // Attribute Descriptions

    return vertex_layout_default;
  }


  Xenith::Graphics::ColorBlendConfig LoadEngineDefaultColorBlendConfig()
  {
    ColorBlendConfig color_blend_config_to_return;

    // Color blend state
    Xenith::Graphics::ColorBlendAttachmentState color_blend_attachment_state = {
      .enable_blend = VK_FALSE,

      .src_color_blend_factor = VK_BLEND_FACTOR_ZERO,
      .dst_color_blend_factor = VK_BLEND_FACTOR_ZERO,
      .color_blend_op = VK_BLEND_OP_ADD,

      .src_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
      .dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
      .alpha_blend_op = VK_BLEND_OP_ADD,

      .color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    color_blend_config_to_return.attachment_states.push_back(color_blend_attachment_state);

    return color_blend_config_to_return;
  }


  Xenith::Graphics::DepthConfig LoadEngineDefaultDepthConfig()
  {
    Xenith::Graphics::DepthConfig depth_config_to_return = 
    {
      .compare_op = VK_COMPARE_OP_LESS,
      .enable_depth_test = VK_TRUE,
      .enable_depth_write = VK_TRUE,
      .enable_depth_stencil = VK_FALSE
    };

    return depth_config_to_return;
  }


  Xenith::Graphics::Pipeline CreateGraphicsPipeline(Xenith::Graphics::Device logical_device,
                                                    Xenith::Graphics::Shader &vertex_shader,
                                                    Xenith::Graphics::Shader &fragment_shader,
                                                    const Xenith::Graphics::PipelineLayout &pipeline_layout,
                                                    const Xenith::Graphics::VertexLayout &vertex_layout,
                                                    const Xenith::Graphics::GraphicsPipelineConfig &graphics_pipeline_config,
                                                    const Xenith::Debug::LogVerbosity log_verbosity)
  {
    VkPipeline graphics_pipeline_to_return = VK_NULL_HANDLE;

    // aliases
    const Xenith::Graphics::RasterizerConfig &rasterizer_config = graphics_pipeline_config.rasterizer_config;
    const Xenith::Graphics::DepthConfig &depth_config = graphics_pipeline_config.depth_config;
    const Xenith::Graphics::ColorBlendConfig &color_blend_config = graphics_pipeline_config.color_blend_config;

    // Shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = { 
      vertex_shader.pipeline_stage_info, 
      fragment_shader.pipeline_stage_info
    };

    // Vertex input state
    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

      .vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_layout.binding_description.size()),
      .pVertexBindingDescriptions = vertex_layout.binding_description.data(),
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_layout.attribute_descriptions.size()),
      .pVertexAttributeDescriptions = vertex_layout.attribute_descriptions.data()
    };


    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = 
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,

      .topology = rasterizer_config.primitive_topology
    };

    // Viewport state info
    VkPipelineViewportStateCreateInfo viewport_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,

      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr
    };

    // Rasterization state info
    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    
      .polygonMode = rasterizer_config.fill_mode,
      .cullMode = rasterizer_config.cull_mode_flags,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth = 1.0f
    };

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisample_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,

      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    // Depth stencil info
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,

      .depthTestEnable = depth_config.enable_depth_test,
      .depthWriteEnable = depth_config.enable_depth_write,
      .depthCompareOp = depth_config.compare_op,
      .stencilTestEnable = depth_config.enable_depth_stencil
    };

    // Color blend state
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states;
    color_blend_attachment_states.reserve(color_blend_config.attachment_states.size());

    std::vector<VkFormat> vk_color_formats;
    vk_color_formats.reserve(color_blend_config.attachment_states.size());

    for (const auto& attachment : color_blend_config.attachment_states) 
    {
      color_blend_attachment_states.push_back({
          .blendEnable         = attachment.enable_blend,
          .srcColorBlendFactor = attachment.src_color_blend_factor,
          .dstColorBlendFactor = attachment.dst_color_blend_factor,
          .colorBlendOp        = attachment.color_blend_op,
          .srcAlphaBlendFactor = attachment.src_alpha_blend_factor,
          .dstAlphaBlendFactor = attachment.dst_alpha_blend_factor,
          .alphaBlendOp        = attachment.alpha_blend_op,
          .colorWriteMask      = attachment.color_write_mask
      });

      vk_color_formats.push_back(graphics_pipeline_config.color_attachment_format);
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

      .attachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size()),
      .pAttachments = color_blend_attachment_states.data()
    };

    // Dynamic State
    std::array<VkDynamicState, 2> dynamic_states{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,

      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()
    };

    
    // Graphics Pipeline creation
    VkPipelineRenderingCreateInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext = nullptr,

      .colorAttachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size()),
      .pColorAttachmentFormats = vk_color_formats.data(),
      .depthAttachmentFormat = graphics_pipeline_config.depth_attachment_format
    };

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &render_info,

      .stageCount = static_cast<uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),

      .pVertexInputState = &vertex_input_state_info,
      .pInputAssemblyState = &input_assembly_state_info,
      .pViewportState = &viewport_state_info,
      .pRasterizationState = &rasterization_state_info,
      .pMultisampleState = &multisample_state_info,
      .pDepthStencilState = &depth_stencil_state_info,
      .pColorBlendState = &color_blend_state_info,
      .pDynamicState = &dynamic_state_info,
      .layout = pipeline_layout.handle,
    };

    VkResult pipeline_result = vkCreateGraphicsPipelines(logical_device, nullptr,
                                                         1,
                                                         &graphics_pipeline_create_info,
                                                         nullptr, &graphics_pipeline_to_return);
    if(pipeline_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create graphics pipeline! Error code: %d", pipeline_result);
      return VK_NULL_HANDLE;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Graphics pipeline created");

    return graphics_pipeline_to_return;
  }
} // namespace Xenith::Graphics

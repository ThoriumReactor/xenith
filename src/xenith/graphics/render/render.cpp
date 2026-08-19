#include "xenith/graphics/render/render.h"

#include <array>

#include <vulkan/vulkan_core.h>

#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  Xenith::Graphics::ColorAttachmentConfig CreateEngineDefaultColorAttachmentConfig(VkImageView swapchain_image_view,
                                                                                   const Xenith::Math::Vec4 &clear_color_value)
  {
    Xenith::Graphics::ColorAttachmentConfig color_attachment_config_to_return;

    color_attachment_config_to_return = {
      .swapchain_image_view = swapchain_image_view,
      .image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,

      .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .store_op = VK_ATTACHMENT_STORE_OP_STORE,

      .clear_color_value = clear_color_value
    };

    return color_attachment_config_to_return;
  }


  Xenith::Graphics::DepthAttachmentConfig CreateEngineDefaultDepthAttachmentConfig(VkImageView depth_image_view)
  {
    Xenith::Graphics::DepthAttachmentConfig depth_attachment_config_to_return;

    depth_attachment_config_to_return = {
      .image_view = depth_image_view,
      .image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,

      .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,

      .clear_depth_stencil_value = {1.0f, 0}
    };

    return depth_attachment_config_to_return;
  }


  Xenith::Graphics::Mesh GenerateBoxMesh(Xenith::Math::Vec3 extent, std::array<uint8_t, 4> color)
  {
    Mesh mesh = {};
    
    // Half extent
    float hx = extent.x * 0.5f;
    float hy = extent.y * 0.5f;
    float hz = extent.z * 0.5f;

    mesh.local_aabb.min = { -hx, -hy, -hz };
    mesh.local_aabb.max = {  hx,  hy,  hz };

    mesh.cpu_vertices = {
      { { -hx, -hy, hz}, { 0, 0, 1 },
            {  color }, { 0.0f, 1.0f }, 0 },
      { { hx, -hy, hz}, { 0, 0, 1 },
            {  color }, { 0.0f, 1.0f }, 0 },

    };

    // 24 vertices
    mesh.cpu_vertices = {
        // Front (+Z)
        { {-hx, -hy,  hz}, { 0,  0,  1}, color,{0.0f, 0.0f},  0 },
        { { hx, -hy,  hz}, { 0,  0,  1}, color,{1.0f, 0.0f},  0 },
        { { hx,  hy,  hz}, { 0,  0,  1}, color,{1.0f, 1.0f},  0 },
        { {-hx,  hy,  hz}, { 0,  0,  1}, color,{0.0f, 1.0f},  0 },
                                                                            
        // Back (-Z)                                                        
        { { hx, -hy, -hz}, { 0,  0, -1}, color,{0.0f, 0.0f},  0 },
        { {-hx, -hy, -hz}, { 0,  0, -1}, color,{1.0f, 0.0f},  0 },
        { {-hx,  hy, -hz}, { 0,  0, -1}, color,{1.0f, 1.0f},  0 },
        { { hx,  hy, -hz}, { 0,  0, -1}, color,{0.0f, 1.0f},  0 },
                                                                            
        // Top (+Y)                                                         
        { {-hx,  hy,  hz}, { 0,  1,  0}, color,{0.0f, 0.0f},  0 },
        { { hx,  hy,  hz}, { 0,  1,  0}, color,{1.0f, 0.0f},  0 },
       { { hx,  hy, -hz}, { 0,  1,  0}, color,{1.0f, 1.0f},  0 },
       { {-hx,  hy, -hz}, { 0,  1,  0}, color,{0.0f, 1.0f},  0 },
                                                                            
        // Bottom (-Y)                                                      
       { {-hx, -hy, -hz}, { 0, -1,  0}, color,{0.0f, 0.0f},  0 },
       { { hx, -hy, -hz}, { 0, -1,  0}, color,{1.0f, 0.0f},  0 },
       { { hx, -hy,  hz}, { 0, -1,  0}, color,{1.0f, 1.0f},  0 },
       { {-hx, -hy,  hz}, { 0, -1,  0}, color,{0.0f, 1.0f},  0 },
                                                                           
       // Right (+X)                                                       
       { { hx, -hy,  hz}, { 1,  0,  0}, color,{0.0f, 0.0f},  0 },
       { { hx, -hy, -hz}, { 1,  0,  0}, color,{1.0f, 0.0f},  0 },
       { { hx,  hy, -hz}, { 1,  0,  0}, color,{1.0f, 1.0f},  0 },
       { { hx,  hy,  hz}, { 1,  0,  0}, color,{0.0f, 1.0f},  0 },
                                                                           
       // Left (-X)                                                        
       { {-hx, -hy, -hz}, {-1,  0,  0}, color,{0.0f, 0.0f},  0 },
       { {-hx, -hy,  hz}, {-1,  0,  0}, color,{1.0f, 0.0f},  0 },
       { {-hx,  hy,  hz}, {-1,  0,  0}, color,{1.0f, 1.0f},  0 },
       { {-hx,  hy, -hz}, {-1,  0,  0}, color,{0.0f, 1.0f},  0 }
    };

    // Indices for 6 quads
    mesh.cpu_indices = {
         0,  1,  2,   0,  2,  3,        // Front
         4,  5,  6,   4,  6,  7,      // Back
         8,  9, 10,   8, 10, 11,  // Top
        12, 13, 14,  12, 14, 15,  // Bottom
        16, 17, 18,  16, 18, 19,  // Right
        20, 21, 22,  20, 22, 23   // Left
    };

    mesh.index_count = static_cast<uint32_t>(mesh.cpu_indices.size());
    return mesh;
  }

  Xenith::Graphics::Mesh GenerateWireframeBoxMesh(Xenith::Math::Vec3 extent) 
  {
      Xenith::Graphics::Mesh mesh = {};
      
      float hx = extent.x * 0.5f;
      float hy = extent.y * 0.5f;
      float hz = extent.z * 0.5f;

      mesh.local_aabb.min = { -hx, -hy, -hz };
      mesh.local_aabb.max = {  hx,  hy,  hz };

      // Exactly 8 corner vertices (no duplicates for normals/UVs needed)
      mesh.cpu_vertices = {
          // Bottom / Back face (-Z)
          { { -hx, -hy, -hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 0.0f, 0.0f }, 0 }, // 0: Bottom-Left-Back
          { {  hx, -hy, -hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 1.0f, 0.0f }, 0 }, // 1: Bottom-Right-Back
          { {  hx,  hy, -hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 1.0f, 1.0f }, 0 }, // 2: Top-Right-Back
          { { -hx,  hy, -hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 0.0f, 1.0f }, 0 }, // 3: Top-Left-Back

          // Top / Front face (+Z)
          { { -hx, -hy,  hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 0.0f, 0.0f }, 0 }, // 4: Bottom-Left-Front
          { {  hx, -hy,  hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 1.0f, 0.0f }, 0 }, // 5: Bottom-Right-Front
          { {  hx,  hy,  hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 1.0f, 1.0f }, 0 }, // 6: Top-Right-Front
          { { -hx,  hy,  hz }, { 0, 0, 1 }, { 255, 255, 255, 255 }, { 0.0f, 1.0f }, 0 }  // 7: Top-Left-Front
      };

      // 12 Line segments (24 indices total) — pair by pair
      mesh.cpu_indices = {
          // Back face loop (-Z)
          0, 1,   1, 2,   2, 3,   3, 0,
          // Front face loop (+Z)
          4, 5,   5, 6,   6, 7,   7, 4,
          // Connecting edges between Back and Front
          0, 4,   1, 5,   2, 6,   3, 7
      };

      mesh.index_count = static_cast<uint32_t>(mesh.cpu_indices.size()); // 24
      return mesh;
  }
} // namespace Xenith::Graphics

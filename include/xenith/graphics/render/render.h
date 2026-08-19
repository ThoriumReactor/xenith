#pragma once

#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  Xenith::Graphics::ColorAttachmentConfig CreateEngineDefaultColorAttachmentConfig(VkImageView swapchain_image_view,
                                                                                   const Xenith::Math::Vec4 &clear_color_value);

  Xenith::Graphics::DepthAttachmentConfig CreateEngineDefaultDepthAttachmentConfig(VkImageView depth_image_view);


  Xenith::Graphics::Mesh GenerateBoxMesh(Xenith::Math::Vec3 extent, std::array<uint8_t, 4> color);
  Xenith::Graphics::Mesh GenerateWireframeBoxMesh(Xenith::Math::Vec3 extent);
}

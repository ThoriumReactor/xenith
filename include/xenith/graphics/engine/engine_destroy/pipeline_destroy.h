#pragma once

#include "xenith/engine_ecs/engine_components.h"
#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics
{
  void DestroyGraphicsPipelineContext(Xenith::EngineECS::GraphicsPipelineContext &pipeline_context,
                                      Xenith::Graphics::Device logical_device,
                                      Xenith::Graphics::Allocator allocator,
                                      const Xenith::Debug::LogVerbosity log_verbosity);
}

#pragma once

#include "xenith/engine_ecs/engine_components.h"
#include "xenith/debug_xn/logs.h"


namespace Xenith::Graphics
{
  void DestroySwapchainContext(Xenith::EngineECS::SwapchainContext &swapchain_context,
                               Xenith::Graphics::Device logical_device,
                               const Xenith::Debug::LogVerbosity log_verbosity);
}

#pragma once

#include "xenith/engine_ecs/engine_components.h"
#include "xenith/debug_xn/logs.h"


namespace Xenith::Graphics
{
  // Destroys core context
  void DestroyCoreContext(Xenith::EngineECS::CoreContext &core_context,
                          const Xenith::Debug::LogVerbosity log_verbosity);
}

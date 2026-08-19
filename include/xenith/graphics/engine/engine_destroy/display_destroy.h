#pragma once

#include "xenith/debug_xn/logs.h"
#include "xenith/display/display.h"


namespace Xenith::Display
{
  /// as for now it is just window
  void DestroyDisplay(Xenith::Display::Window window,
                      Xenith::Debug::LogVerbosity log_verbosity);
}

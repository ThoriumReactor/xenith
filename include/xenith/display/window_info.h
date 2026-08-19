#pragma once

#include "xenith/display/display.h"


namespace Xenith::Display
{
  // Width
  int GetWindowWidth(Xenith::Display::Window window);

  // Height
  int GetWindowHeight(Xenith::Display::Window window);


  // Aspect Ratio
  float GetWindowAspect(Xenith::Display::Window window);
}

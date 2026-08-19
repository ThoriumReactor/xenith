#pragma once

#include "xenith/math/math.h"


struct PushConstant
{
  alignas(16) Xenith::Math::Matrix4 model;
  alignas(16) Xenith::Math::Matrix4 view;
  alignas(16) Xenith::Math::Matrix4 projection;
};

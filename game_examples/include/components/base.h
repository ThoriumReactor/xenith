#pragma once

#include "xenith/math/math.h"

// !! This is local ecs, not engines!!
// meaning that this ecs is made by user in this example
namespace ecs
{
  struct WindowContext
  {
    bool is_resizing = false;
    float resize_timer = 0.0f;
    float resize_time_cooldown;
  };


  struct Transform
  {
    Xenith::Math::Vec3 position;
    Xenith::Math::Vec3 scale;
  };

  struct Camera
  {
    Xenith::Math::Matrix4 projection_matrix{ 1.0f };
    Xenith::Math::Matrix4 view_matrix{ 1.0f };

    Xenith::Math::Vec3 position;

    Xenith::Math::Vec3 forward;
    Xenith::Math::Vec3 right;
    Xenith::Math::Vec3 up;

    float yaw    = 0.0f;
    float pitch  = 0.0f;
  };
}

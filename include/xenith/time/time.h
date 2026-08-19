#pragma once


namespace Xenith::Time
{
  /// Needed for framerate limit, pass desired FPS
  void SetupLimitFramerate(float target_fps);

  /// Put this in start of update game loop
  void UpdateTime();

  /// Limits framerate, do this after initializing SetupLimitFramerate
  void LimitFramerate();

  float CalculateDeltaTimeFloat();
  double CalculateDeltaTimeDouble();
  float CalculateFrameTimeMS();
} // Xenith::Time

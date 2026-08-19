#pragma once

#include "xenith/math/math.h"

#include <cstdint>


namespace Xenith::Input
{
  static const uint8_t MOUSE_BUTTONS_COUNT = 5;
  enum MouseButtons : uint8_t
  {
    LEFT,
    RIGHT,
    MIDDLE,
    M4,
    M5
  };

  void PresentUpdateMouse();
  void UpdateMousePosition(float x, float y);
  void UpdateMouseButtonState(uint8_t button_index, bool is_pressed);


  float GetMousePositionX();
  float GetMousePositionY();
  Xenith::Math::Vec2 GetMousePositionXY();


  bool IsMouseButtonPressed(Xenith::Input::MouseButtons mouse_button);
  bool IsMouseButtonHeld(Xenith::Input::MouseButtons mouse_button);
  bool IsMouseButtonReleased(Xenith::Input::MouseButtons mouse_button);


  /// Calculates delta x and y
  void CalculateMouseDeltas(float x_relative, float y_relative);

  /// Resets mouse delta between frames
  void ZeroMouseDeltas();

  /// Returns mouse delta x
  float GetMouseDeltaX();

  /// Returns mouse delta y
  float GetMouseDeltaY();

  /// Returns mouse delta for both x and y in vector 2 shape
  Xenith::Math::Vec2 GetMouseDeltaXY();

} // Xenith::Input

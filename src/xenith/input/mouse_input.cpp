#include "xenith/input/mouse_input.h"

#include "SDL3/SDL_mouse.h"
#include "xenith/math/math.h"

#include <cstring>


namespace Xenith::Input
{
  static float mouse_dx = 0.0f;
  static float mouse_dy = 0.0f;

  static float mouse_x = 0.0f;
  static float mouse_y = 0.0f;

  static bool current_buttons[MOUSE_BUTTONS_COUNT]   = { false };
  static bool previous_buttons[MOUSE_BUTTONS_COUNT]  = { false };

  // Calculator
  void CalculateMouseDeltas(float x_relative, float y_relative)
  {
    mouse_dx += x_relative;
    mouse_dy += y_relative;
  }


  void PresentUpdateMouse()
  {
    std::memcpy(previous_buttons, current_buttons, sizeof(current_buttons));
  }


  void UpdateMousePosition(float x, float y)
  {
    mouse_x = x;
    mouse_y = y;
  }


  void UpdateMouseButtonState(uint8_t button_index, bool is_pressed)
  {
    if(button_index == SDL_BUTTON_LEFT)
      current_buttons[Xenith::Input::MouseButtons::LEFT] = is_pressed;

    if(button_index == SDL_BUTTON_RIGHT)
      current_buttons[Xenith::Input::MouseButtons::RIGHT] = is_pressed;

    if(button_index == SDL_BUTTON_MIDDLE)
      current_buttons[Xenith::Input::MouseButtons::MIDDLE] = is_pressed;

    if(button_index == SDL_BUTTON_X1)
      current_buttons[Xenith::Input::MouseButtons::M4] = is_pressed;

    if(button_index == SDL_BUTTON_X2)
      current_buttons[Xenith::Input::MouseButtons::M5] = is_pressed;
  }


  float GetMousePositionX()
  {
    return mouse_x;
  }

  float GetMousePositionY()
  {
    return mouse_y;
  }

  Xenith::Math::Vec2 GetMousePositionXY()
  {
    Xenith::Math::Vec2 mouse_position_xy_to_return = { mouse_x, mouse_y };

    return mouse_position_xy_to_return;
  }


  bool IsMouseButtonPressed(Xenith::Input::MouseButtons mouse_button)
  {
    return (current_buttons[mouse_button] && !previous_buttons[mouse_button]);
  }

  bool IsMouseButtonHeld(Xenith::Input::MouseButtons mouse_button)
  {
    return current_buttons[mouse_button];
  }

  bool IsMouseButtonReleased(Xenith::Input::MouseButtons mouse_button)
  {
    return (previous_buttons[mouse_button] && !current_buttons[mouse_button]);
  }


  // zero
  void ZeroMouseDeltas()
  {
    mouse_dx = 0.0f;
    mouse_dy = 0.0f;
  }
  

  // Getters
  float GetMouseDeltaX()
  {
    return mouse_dx;
  }

  float GetMouseDeltaY()
  {
    return mouse_dy;
  }

  Xenith::Math::Vec2 GetMouseDeltaXY()
  {
    return Xenith::Math::Vec2(mouse_dx, mouse_dy);
  }
} // Xenith::Input

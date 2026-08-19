#include "xenith/input/keyboard_input.h"

#include <cstring>

#include "SDL3/SDL_keyboard.h"




namespace Xenith::Input
{
  static bool current_states[512]  = { false };
  static bool previous_states[512] = { false };

  void PresentUpdateKeyboard()
  {
    std::memcpy(previous_states, current_states, sizeof(current_states));

    int n_keys = 0;
    const bool* sdl_states = SDL_GetKeyboardState(&n_keys);

    if(n_keys > 512) n_keys = 512;

    std::memcpy(current_states, sdl_states, n_keys * sizeof(bool));
  }

  bool IsKeyPressed(Xenith::Input::KeyCode key)
  {
    return (current_states[key] && !previous_states[key]);
  }

  bool IsKeyHeld(Xenith::Input::KeyCode key)
  {
    return current_states[key];
  }

  bool IsKeyReleased(Xenith::Input::KeyCode key)
  {
    return (previous_states[key] && !current_states[key]);
  }
} // Xenith::Input

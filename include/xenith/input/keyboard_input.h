#pragma once

#include "SDL3/SDL_scancode.h"


namespace Xenith::Input
{
  using KeyCode = SDL_Scancode;
  namespace Keys
  {
    inline constexpr KeyCode ESCAPE = SDL_SCANCODE_ESCAPE;
    inline constexpr KeyCode F1     = SDL_SCANCODE_F1;
    inline constexpr KeyCode F2     = SDL_SCANCODE_F2;
    inline constexpr KeyCode F3     = SDL_SCANCODE_F3;
    inline constexpr KeyCode F4     = SDL_SCANCODE_F4;
    inline constexpr KeyCode F5     = SDL_SCANCODE_F5;
    inline constexpr KeyCode F6     = SDL_SCANCODE_F6;
    inline constexpr KeyCode F7     = SDL_SCANCODE_F7;
    inline constexpr KeyCode F8     = SDL_SCANCODE_F8;
    inline constexpr KeyCode F9     = SDL_SCANCODE_F9;
    inline constexpr KeyCode F10    = SDL_SCANCODE_F10;
    inline constexpr KeyCode F11    = SDL_SCANCODE_F11;
    inline constexpr KeyCode F12    = SDL_SCANCODE_F12;


    inline constexpr KeyCode TILDE     = SDL_SCANCODE_GRAVE;
    inline constexpr KeyCode ONE       = SDL_SCANCODE_1;
    inline constexpr KeyCode TWO       = SDL_SCANCODE_2;
    inline constexpr KeyCode THREE     = SDL_SCANCODE_3;
    inline constexpr KeyCode FOUR      = SDL_SCANCODE_4;
    inline constexpr KeyCode FIVE      = SDL_SCANCODE_5;
    inline constexpr KeyCode SIX       = SDL_SCANCODE_6;
    inline constexpr KeyCode SEVEN     = SDL_SCANCODE_7;
    inline constexpr KeyCode EIGHT     = SDL_SCANCODE_8;
    inline constexpr KeyCode NINE      = SDL_SCANCODE_9;
    inline constexpr KeyCode ZERO      = SDL_SCANCODE_0;
    inline constexpr KeyCode MINUS     = SDL_SCANCODE_MINUS;
    inline constexpr KeyCode PLUS      = SDL_SCANCODE_EQUALS;
    inline constexpr KeyCode EQUAL     = SDL_SCANCODE_EQUALS;
    inline constexpr KeyCode BACKSPACE = SDL_SCANCODE_BACKSPACE;


    inline constexpr KeyCode TAB = SDL_SCANCODE_TAB;
    inline constexpr KeyCode Q   = SDL_SCANCODE_Q;
    inline constexpr KeyCode W   = SDL_SCANCODE_W;
    inline constexpr KeyCode E   = SDL_SCANCODE_E;
    inline constexpr KeyCode R   = SDL_SCANCODE_R;
    inline constexpr KeyCode T   = SDL_SCANCODE_T;
    inline constexpr KeyCode Y   = SDL_SCANCODE_Y;
    inline constexpr KeyCode U   = SDL_SCANCODE_U;
    inline constexpr KeyCode I   = SDL_SCANCODE_I;
    inline constexpr KeyCode O   = SDL_SCANCODE_O;
    inline constexpr KeyCode P   = SDL_SCANCODE_P;
    inline constexpr KeyCode LEFT_BRACKET        = SDL_SCANCODE_LEFTBRACKET;
    inline constexpr KeyCode LEFT_CURLY_BRACKET  = SDL_SCANCODE_LEFTBRACKET;
    inline constexpr KeyCode RIGHT_BRACKET       = SDL_SCANCODE_RIGHTBRACKET;
    inline constexpr KeyCode RIGHT_CURLY_BRACKET = SDL_SCANCODE_RIGHTBRACKET;
    inline constexpr KeyCode BACKSLASH           = SDL_SCANCODE_BACKSLASH;
    inline constexpr KeyCode VERTICAL_LINE       = SDL_SCANCODE_BACKSLASH;


    inline constexpr KeyCode CAPS_LOCK = SDL_SCANCODE_CAPSLOCK;
    inline constexpr KeyCode A         = SDL_SCANCODE_A;
    inline constexpr KeyCode S         = SDL_SCANCODE_S;
    inline constexpr KeyCode D         = SDL_SCANCODE_D;
    inline constexpr KeyCode F         = SDL_SCANCODE_F;
    inline constexpr KeyCode G         = SDL_SCANCODE_G;
    inline constexpr KeyCode H         = SDL_SCANCODE_H;
    inline constexpr KeyCode J         = SDL_SCANCODE_J;
    inline constexpr KeyCode K         = SDL_SCANCODE_K;
    inline constexpr KeyCode L         = SDL_SCANCODE_L;
    inline constexpr KeyCode SEMICOLON = SDL_SCANCODE_SEMICOLON;
    inline constexpr KeyCode COLON     = SDL_SCANCODE_SEMICOLON;
    inline constexpr KeyCode QUOTES    = SDL_SCANCODE_APOSTROPHE;


    inline constexpr KeyCode LEFT_SHIFT = SDL_SCANCODE_LSHIFT;
    inline constexpr KeyCode Z          = SDL_SCANCODE_Z;
    inline constexpr KeyCode X          = SDL_SCANCODE_X;
    inline constexpr KeyCode C          = SDL_SCANCODE_C;
    inline constexpr KeyCode V          = SDL_SCANCODE_V;
    inline constexpr KeyCode B          = SDL_SCANCODE_B;
    inline constexpr KeyCode N          = SDL_SCANCODE_N;
    inline constexpr KeyCode M          = SDL_SCANCODE_M;

    inline constexpr KeyCode COMMA             = SDL_SCANCODE_COMMA;
    inline constexpr KeyCode LESS_THAN_SIGN    = SDL_SCANCODE_COMMA;
    inline constexpr KeyCode DOT               = SDL_SCANCODE_PERIOD;
    inline constexpr KeyCode PERIOD            = SDL_SCANCODE_PERIOD;
    inline constexpr KeyCode GREATER_THAN_SIGN = SDL_SCANCODE_PERIOD;
    inline constexpr KeyCode FORWARD_SLASH     = SDL_SCANCODE_SLASH;
    inline constexpr KeyCode QUESTION_MARK     = SDL_SCANCODE_SLASH;


    inline constexpr KeyCode LEFT_CONTROL  = SDL_SCANCODE_LCTRL;
    inline constexpr KeyCode LEFT_SUPER    = SDL_SCANCODE_LGUI;
    inline constexpr KeyCode LEFT_ALT      = SDL_SCANCODE_LALT;
    inline constexpr KeyCode SPACEBAR      = SDL_SCANCODE_SPACE;
    inline constexpr KeyCode RIGHT_ALT     = SDL_SCANCODE_RALT;
    inline constexpr KeyCode RIGHT_SUPER   = SDL_SCANCODE_RGUI;
    inline constexpr KeyCode RIGHT_CONTROL = SDL_SCANCODE_RCTRL;


    inline constexpr KeyCode UP_ARROW = SDL_SCANCODE_UP;
    inline constexpr KeyCode DOWN_ARROW = SDL_SCANCODE_DOWN;
    inline constexpr KeyCode LEFT_ARROW = SDL_SCANCODE_LEFT;
    inline constexpr KeyCode RIGHT_ARROW = SDL_SCANCODE_RIGHT;
  }


  void PresentUpdateKeyboard();

  bool IsKeyPressed(Xenith::Input::KeyCode key);
  bool IsKeyHeld(Xenith::Input::KeyCode key);
  bool IsKeyReleased(Xenith::Input::KeyCode key);
} // Xenith::Input

#pragma once

#include "sdl3/include/SDL3/SDL_events.h"

#include <cstdint>

#include "xenith/input/mouse_input.h"


namespace Xenith::Core
{
  using EventType = SDL_EventType;
  namespace EventTypes
  {
    constexpr EventType NONE = static_cast<EventType>(0);

    constexpr EventType WINDOW_RESIZED             = SDL_EVENT_WINDOW_RESIZED;
    constexpr EventType WINDOW_PIXEL_SIZE_CHANGED  = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;

    constexpr EventType MOUSE_MOTION       = SDL_EVENT_MOUSE_MOTION;
    constexpr EventType MOUSE_BUTTON_DOWN  = SDL_EVENT_MOUSE_BUTTON_DOWN;
    constexpr EventType MOUSE_BUTTON_UP    = SDL_EVENT_MOUSE_BUTTON_UP;

    constexpr EventType QUIT  = SDL_EVENT_QUIT;
  }

  struct WindowEvent
  {
    Xenith::Core::EventType type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    int32_t width;
    int32_t height;
  };

  struct MouseMotionEvent
  {
    Xenith::Core::EventType type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t mouse_id;
    Xenith::Input::MouseButtons state;

    float x;
    float y;
    float x_relative;
    float y_relative;
  };

  struct MouseButtonEvent
  {
    Xenith::Core::EventType type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t mouse_id;

    uint8_t button_index;
    bool is_down;
    uint8_t clicks; // 1 for single click, 2 for double click, etc
    uint8_t padding;
    float x;
    float y;
  };


  struct Event
  {
    union
    {
      Xenith::Core::EventType type;
      Xenith::Core::WindowEvent window;
      Xenith::Core::MouseMotionEvent mouse_motion;
      Xenith::Core::MouseButtonEvent mouse_button;
    };
  };

  inline bool PollEvent(Xenith::Core::Event* event)
  {
    SDL_Event sdl_event;
    if(!SDL_PollEvent(&sdl_event))
      return false;

    switch(sdl_event.type)
    {
      case SDL_EVENT_QUIT:
        event->type = Xenith::Core::EventTypes::QUIT;
        break;

      case SDL_EVENT_MOUSE_MOTION:
        event->type = Xenith::Core::EventTypes::MOUSE_MOTION;
        
        event->mouse_motion.x = sdl_event.motion.x;
        event->mouse_motion.y = sdl_event.motion.y;
        event->mouse_motion.x_relative = sdl_event.motion.xrel;
        event->mouse_motion.y_relative = sdl_event.motion.yrel;
        break;

      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        event->type = Xenith::Core::EventTypes::WINDOW_PIXEL_SIZE_CHANGED;

        event->window.width = sdl_event.window.data1;
        event->window.height = sdl_event.window.data2;
        break;

      default:
        event->type = Xenith::Core::EventTypes::NONE;
        break;
    }


    return true;
  }
}

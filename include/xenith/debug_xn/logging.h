#pragma once

#include "xenith/math/math.h"

#include <cstdio>


namespace Xenith::Debug
{
  void LogInternal(const char* c_string);

  inline void Log(const char* c_string)
  {
    LogInternal(c_string);
  }

  template<typename... Args>
  void Log(const char* c_string, Args... args)
  {
    char buffer[1024];

    std::snprintf(buffer, sizeof(buffer), c_string, args...);

    LogInternal(buffer);
  }

  void LogVec3(const char* text_at_start, Xenith::Math::Vec3 vector_to_log);
}

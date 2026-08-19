#include "xenith/debug_xn/logging.h"

#include "SDL3/SDL_log.h"


namespace Xenith::Debug
{
  void LogInternal(const char* c_string)
  {
    SDL_Log("%s", c_string);
  }

  void LogVec3(const char* text_at_start, Xenith::Math::Vec3 vector_to_log)
  {
    Xenith::Debug::Log("%s %f, %f, %f,",
                       text_at_start,
                       vector_to_log.x, vector_to_log.y, vector_to_log.z);
  }
}

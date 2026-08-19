#include "xenith/time/time.h"

#include <cstdint>

#include "SDL3/SDL_timer.h"


namespace Xenith::Time
{
  struct InternalTimePacer 
  {
    uint64_t target_ns = 0;
    uint64_t last_time_ns = 0;
    uint64_t frame_start_ns = 0;
    double dt_double = 0.0;
  };

  static InternalTimePacer time_pacer;


  void SetupLimitFramerate(float target_fps)
  {
    time_pacer.target_ns = (uint64_t)(1000000000.0 / target_fps);

    time_pacer.last_time_ns = SDL_GetTicksNS();
    time_pacer.dt_double = 0.0;
  }


  void UpdateTime()
  {
    time_pacer.frame_start_ns = SDL_GetTicksNS();

    time_pacer.dt_double = (double)(time_pacer.frame_start_ns - time_pacer.last_time_ns) / 1e9;

    time_pacer.last_time_ns = time_pacer.frame_start_ns;
  }


  void LimitFramerate()
  {
    if (time_pacer.target_ns == 0) return;

    while(true)
    {
      Uint64 current = SDL_GetTicksNS();
      Uint64 elapsed = current - time_pacer.frame_start_ns;

      if(elapsed >= time_pacer.target_ns)
        break;

      Uint64 remaining = time_pacer.target_ns - elapsed;
      if(remaining > 2000000)
      {
        SDL_Delay(1);
      }
    }
  }


  float CalculateDeltaTimeFloat()
  {
    return (float)time_pacer.dt_double;
  }

  double CalculateDeltaTimeDouble()
  {
    return time_pacer.dt_double;
  }

  float CalculateFrameTimeMS()
  {
    uint64_t frame_end_ns = SDL_GetTicksNS();
    return (float)((double)(frame_end_ns - time_pacer.frame_start_ns) / 1000000.0);
  }

} // Xenith::Time

#pragma once

#include "xenith/debug_xn/logs.h"
#include "xenith/graphics/graphics_vk.h"

#include <vector>


namespace Xenith::Graphics
{
  /// Creates timeline semaphore
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::Semaphore CreateTimelineSemaphore(Xenith::Graphics::Device logical_device,
                                                      const uint64_t initial_value,
                                                      const Xenith::Debug::LogVerbosity log_verbosity);

  /// Initializes image acquired semaphores container
  void InitializeImageAcquiredSemaphoresContainer(Xenith::Graphics::Device logical_device,
                                                  const uint32_t max_frames_in_flight,
                                                  std::vector<Xenith::Graphics::Semaphore> &image_acquired_semaphores,
                                                  const Xenith::Debug::LogVerbosity log_verbosity);

  /// Initializes render finished semaphores container
  void InitializeRenderFinishedSemaphoresContainer(Xenith::Graphics::Device logical_device,
                                                   const uint32_t max_frames_in_flight,
                                                   std::vector<Xenith::Graphics::Semaphore> &render_finished_semaphores,
                                                   const Xenith::Debug::LogVerbosity log_verbosity);

  ///
  void InitializeCommandResourcesCommandPoolsAndAllocateCommandBuffers(Xenith::Graphics::Device logical_device,
                                                                       const uint32_t max_frames_in_flight,
                                                                       std::vector<Xenith::Graphics::CommandResources> &command_resources,
                                                                       uint32_t graphics_family_queue_index,
                                                                       const Xenith::Debug::LogVerbosity log_verbosity);
}

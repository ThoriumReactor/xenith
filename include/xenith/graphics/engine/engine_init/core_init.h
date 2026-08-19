#pragma once

#include "xenith/debug_xn/logs.h"

#include "xenith/display/display.h"

#include "xenith/graphics/graphics_vk.h"


namespace Xenith::Graphics 
{
  /// Creates vulkan instance and returns it. 
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Graphics::Instance CreateVulkanInstance(const Xenith::Graphics::InstanceConfig &instance_config,
                                          const Xenith::Debug::LogVerbosity log_verbosity);
                                          
  /// Creates vulkan window surface
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Xenith::Graphics::Surface CreateSurface(Xenith::Display::Window window, 
                                          Xenith::Graphics::Instance instance,
                                          const Xenith::Debug::LogVerbosity log_verbosity);


  /// Searches and returns physical device, 
  /// you can manage priority of physical device type by changing physical_device_type_priority variable. 
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Graphics::PhysicalDevice FindAndSelectPhysicalDevice(Xenith::Graphics::Instance instance,
                                                       Xenith::Graphics::Surface surface,
                                                       const Xenith::Graphics::PhysicalDeviceSelectConfig &select_config,
                                                       const Xenith::Debug::LogVerbosity log_verbosity);

  /// Searches and returns graphics family queue index.
  /// If failed, throws an error log and returns 0xFFFFFFFF
  uint32_t FindGraphicsFamilyQueueIndex(Xenith::Graphics::PhysicalDevice physical_device,
                                        Xenith::Graphics::Surface surface,
                                        Xenith::Debug::LogVerbosity log_verbosity);

  /// Creates logical device
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Graphics::Device CreateLogicalDevice(Xenith::Graphics::PhysicalDevice physical_device, 
                                       const Xenith::Graphics::LogicalDeviceConfig &logical_device_config,
                                       const Xenith::Debug::LogVerbosity log_verbosity);

  /// Fetches and returns queue, by corresponding queue family index
  Xenith::Graphics::Queue GetDeviceQueue(Xenith::Graphics::Device logical_device, 
                                         uint32_t queue_family_index);

  /// Creates Vulkan Memory Allocator
  /// If failed, throws an error log and returns VK_NULL_HANDLE
  Graphics::Allocator CreateVulkanMemoryAllocator(Xenith::Graphics::Instance instance,
                                      Xenith::Graphics::PhysicalDevice physical_device,
                                      Xenith::Graphics::Device logical_device,
                                      Xenith::Debug::LogVerbosity log_verbosity);
}

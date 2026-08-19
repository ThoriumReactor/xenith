#include "xenith/graphics/engine/engine_init/core_init.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/debug_xn/logging.h"

#include "xenith/graphics/graphics_vk.h"

#include "SDL3/SDL_vulkan.h"

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vma/vk_mem_alloc.h"


namespace Xenith::Graphics
{
  Graphics::Instance CreateVulkanInstance(const Xenith::Graphics::InstanceConfig &instance_config,
                                          const Xenith::Debug::LogVerbosity log_verbosity)
  {
    if(log_verbosity != Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Vulkan instance ------");

    VkInstance instance_to_return = VK_NULL_HANDLE;

    VkApplicationInfo application_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = instance_config.application_info.application_name,
      .applicationVersion = instance_config.application_info.application_version,

      .pEngineName = "Xenith", 
      .engineVersion = VK_MAKE_VERSION(0, 0, 1), // as for now version of engine is temporarily hardcoded

      .apiVersion = VK_API_VERSION_1_3
    };

    // Extensions
    uint32_t extension_count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);


    // Validation layers
    std::vector<const char*> requested_layers;

    if(instance_config.validation_layers_toggle)
    {
      requested_layers.push_back("VK_LAYER_KHRONOS_validation");
    }


    // VK Instance
    VkInstanceCreateInfo instance_create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

      .pApplicationInfo = &application_info,

      .enabledLayerCount = static_cast<uint32_t>(requested_layers.size()),
      .ppEnabledLayerNames = requested_layers.data(),

      .enabledExtensionCount = extension_count,
      .ppEnabledExtensionNames = extensions
    };

    VkResult result = vkCreateInstance(&instance_create_info, 
                                       nullptr, &instance_to_return);

    if(result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create vulkan instance! Error code: %d", result);
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Vulkan instance created");

    return instance_to_return;
  }


  Xenith::Graphics::Surface CreateSurface(Xenith::Display::Window window, 
                                          Xenith::Graphics::Instance instance,
                                          const Xenith::Debug::LogVerbosity log_verbosity)
  {
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Vulkan Surface ------");

    VkSurfaceKHR surface = VK_NULL_HANDLE;


    if(!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    {
      Xenith::Debug::Log("Failed to create Vulkan window surface!");

      return VK_NULL_HANDLE;
    }
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Vulkan window surface created");

    return surface;
  }


  Graphics::PhysicalDevice FindAndSelectPhysicalDevice(Xenith::Graphics::Instance instance,
                                                       Xenith::Graphics::Surface surface,
                                                       const Xenith::Graphics::PhysicalDeviceSelectConfig &select_config,
                                                       const Xenith::Debug::LogVerbosity log_verbosity)
  {
    VkPhysicalDevice physical_device_to_return = VK_NULL_HANDLE;

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Physical Device ------");

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    
    if(device_count == 0)
    {
      Xenith::Debug::Log("No physical devices(GPUs) found with Vulkan support!");
      return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, 
                               devices.data());

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Found %d physical devices supporting Vulkan", device_count);


    // Picking best physical device
    for(const auto& device : devices)
    {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(device, &properties);

      // Check surface presentation support
      uint32_t queue_family_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, 
                                               &queue_family_count, nullptr);
      std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
      vkGetPhysicalDeviceQueueFamilyProperties(device,
                                               &queue_family_count, 
                                               queue_family_properties.data());
      bool supports_presentation = false;
      for(uint32_t i = 0; i < queue_family_count; ++i)
      {
        VkBool32 present_support_vk = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, 
                                             surface, &present_support_vk);

        if(present_support_vk)
        {
          supports_presentation = true;
          break;
        }
      }

      // If the physical device can't support presentation, we will just skip it
      if(!supports_presentation)
        continue;
      

      if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      {
        Xenith::Debug::Log(">----- Physical Device properties ------");
        Xenith::Debug::Log("name: %s", properties.deviceName);

        if(log_verbosity == Xenith::Debug::LogVerbosity::MEDIUM ||
           log_verbosity == Xenith::Debug::LogVerbosity::HIGH)
          Xenith::Debug::Log("physical device type: %d", properties.deviceType);

        if(log_verbosity == Xenith::Debug::LogVerbosity::HIGH)
        {
          Xenith::Debug::Log("api version: %d.%d.%d", 
              VK_API_VERSION_MAJOR(properties.apiVersion),
              VK_API_VERSION_MINOR(properties.apiVersion),
              VK_API_VERSION_PATCH(properties.apiVersion)
          );
        }
      }

      if(properties.deviceType == select_config.physical_device_type_priority)
      {
        physical_device_to_return = device;
        if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
          Xenith::Debug::Log("GPU Picked as active physical device: %s", properties.deviceName);

        break;
      }
    }

    if(physical_device_to_return == VK_NULL_HANDLE)
    {
      physical_device_to_return = devices[0];
      VkPhysicalDeviceProperties fallback_properties;
      vkGetPhysicalDeviceProperties(physical_device_to_return, &fallback_properties);

      Xenith::Debug::Log("No discrete GPU found! fallback to: %s", fallback_properties.deviceName);
    }

    return physical_device_to_return;
  }


  uint32_t FindGraphicsFamilyQueueIndex(Xenith::Graphics::PhysicalDevice physical_device,
                                        Xenith::Graphics::Surface surface,
                                        Xenith::Debug::LogVerbosity log_verbosity)
  {
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Graphics queue family ------");

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, 
                                             &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties2> queue_family_properties(
        queue_family_count,
        {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2}
    );
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, 
                                              &queue_family_count,
                                              queue_family_properties.data());

    uint32_t graphics_queue_family_index = 0xFFFFFFFF;

    for(uint32_t i = 0; i < queue_family_properties.size(); i++)
    {
      VkBool32 has_present_support = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, 
                                           surface, &has_present_support);

      const auto &queue_property = queue_family_properties[i];
      if(queue_property.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && has_present_support)
      {
        graphics_queue_family_index = i;

        if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
          Xenith::Debug::Log("Found graphics queue index: %d", graphics_queue_family_index);

        break;
      }
    }

    if(graphics_queue_family_index == 0xFFFFFFFF)
    {
      Xenith::Debug::Log("Failed to find graphics queue family index!");

      return 0xFFFFFFFF;
    }

    return graphics_queue_family_index;
  }


  Graphics::Device CreateLogicalDevice(Xenith::Graphics::PhysicalDevice physical_device, 
                                       const Xenith::Graphics::LogicalDeviceConfig &logical_device_config,
                                       const Xenith::Debug::LogVerbosity log_verbosity)
  {
    // Logical Device
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Logical Device ------");

    VkDevice logical_device_to_return = VK_NULL_HANDLE;


    // Vulkan features
    VkPhysicalDeviceVulkan13Features features_1_3 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = nullptr,

      .synchronization2 = VK_TRUE,
      .dynamicRendering = VK_TRUE
    };

    VkPhysicalDeviceVulkan12Features features_1_2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &features_1_3,

      .shaderSampledImageArrayNonUniformIndexing     = VK_TRUE,
      .descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE,
      .descriptorBindingPartiallyBound               = VK_TRUE,
      .runtimeDescriptorArray                        = VK_TRUE,

      .timelineSemaphore = VK_TRUE,

      .bufferDeviceAddress = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features features_1_1 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
      .pNext = &features_1_2,

      .shaderDrawParameters = VK_TRUE
    };

    VkPhysicalDeviceFeatures2 features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &features_1_1,

      .features = {
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE
      }
    };


    // Queue
    std::vector<float> queue_priorities = {1.0f};

    // Device queue create info
    VkDeviceQueueCreateInfo graphics_queue_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,

      .queueFamilyIndex = logical_device_config.graphics_queue_family_index,
      .queueCount = 1,
      .pQueuePriorities = queue_priorities.data()
    };


    // Extensions
    const std::vector<const char*> device_extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    
    // Device create info
    VkDeviceCreateInfo device_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &features,

      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &graphics_queue_info,

      .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),

      .pEnabledFeatures = nullptr
    };

    VkResult device_result = vkCreateDevice(physical_device, &device_create_info,
                                            nullptr, &logical_device_to_return);
    if(device_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create logical device! Error code: %d", device_result);

      return VK_NULL_HANDLE;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("Logical device created");

    return logical_device_to_return;
  }


  Xenith::Graphics::Queue GetDeviceQueue(Xenith::Graphics::Device logical_device, 
                                         uint32_t queue_family_index)
  {
    VkQueue queue_to_return = VK_NULL_HANDLE;

    vkGetDeviceQueue(logical_device, queue_family_index, 
                     0, &queue_to_return);

    return queue_to_return;
  }


  Graphics::Allocator CreateVulkanMemoryAllocator(Xenith::Graphics::Instance instance,
                                      Xenith::Graphics::PhysicalDevice physical_device,
                                      Xenith::Graphics::Device logical_device,
                                      Xenith::Debug::LogVerbosity log_verbosity)
  {
    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("------ Vulkan Memory Allocator ------");

    VmaAllocator allocator_to_return = VK_NULL_HANDLE;

    VmaVulkanFunctions vma_func_info{};
    VmaAllocatorCreateInfo vma_allocator_info = {
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,

      .physicalDevice = physical_device,
      .device = logical_device,
      .pVulkanFunctions = nullptr,

      .instance = instance,
      .vulkanApiVersion = VK_API_VERSION_1_3
    };

    VkResult vma_result = vmaCreateAllocator(&vma_allocator_info, &allocator_to_return);
    if(vma_result != VK_SUCCESS)
    {
      Xenith::Debug::Log("Failed to create VMA");

      return VK_NULL_HANDLE;
    }

    if(log_verbosity != Xenith::Debug::LogVerbosity::OFF)
      Xenith::Debug::Log("VMA created");

    return allocator_to_return;
  }



  // void Destroy
} // Xenith::Graphics

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "xenith/core/core.h"
#include "xenith/core/event.h"

#include "xenith/ecs_xn/ecs_xn.h"
#include "xenith/engine_ecs/engine_components.h"

#include "xenith/debug_xn/logs.h"
#include "xenith/debug_xn/logging.h"

#include "xenith/display/display.h"
#include "xenith/display/window_info.h"

#include "xenith/graphics/engine/engine_init/core_init.h"
#include "xenith/graphics/engine/engine_init/display_init.h"
#include "xenith/graphics/engine/engine_init/swapchain_init.h"
#include "xenith/graphics/engine/engine_init/pipeline_init.h"
#include "xenith/graphics/engine/engine_init/render_init.h"

#include "xenith/graphics/engine/engine_destroy/core_destroy.h"
#include "xenith/graphics/engine/engine_destroy/display_destroy.h"
#include "xenith/graphics/engine/engine_destroy/swapchain_destroy.h"
#include "xenith/graphics/engine/engine_destroy/pipeline_destroy.h"
#include "xenith/graphics/engine/engine_destroy/render_destroy.h"

#include "xenith/graphics/engine/engine_render/render_engine.h"
#include "xenith/graphics/engine/mesh_buffer_loader.h"
#include "xenith/graphics/graphics_vk.h"

#include "xenith/graphics/render/render.h"
#include "xenith/graphics/texture.h"
#include "xenith/input/keyboard_input.h"
#include "xenith/input/mouse_input.h"
#include "xenith/math/math.h"
#include "xenith/time/time.h"

#include "xenith/physics/physics.h"


#include "systems/init/graphics/data.h"
#include "components/base.h"



// namespace aliases
namespace XntCore = Xenith::Core;
namespace XntEngineECS = Xenith::EngineECS;
namespace XntDisplay = Xenith::Display;
namespace XntGraphics = Xenith::Graphics;
namespace XntMath = Xenith::Math;
namespace XntInput = Xenith::Input;
namespace XntTime = Xenith::Time;
namespace XntPhysics = Xenith::Physics;
namespace XntDebug = Xenith::Debug;


XntDebug::LogVerbosity log_verbosity = XntDebug::LogVerbosity::HIGH;
XntDebug::LogVerbosity log_verbosity_off = XntDebug::LogVerbosity::OFF;

// ----- Physics
namespace Layers
{
  constexpr XntPhysics::ObjectLayer NON_MOVING  = 0;
  constexpr XntPhysics::ObjectLayer MOVING      = 1;
  constexpr XntPhysics::ObjectLayer PLAYER      = 2;
}

XntPhysics::LayerMatrix CreateLayerMatrix()
{
  XntPhysics::LayerMatrix layer_matrix;

  // Map objects layers to broadphase layers
  layer_matrix.object_to_broadphase[Layers::NON_MOVING]  = XntPhysics::BroadPhase::STATIC;
  layer_matrix.object_to_broadphase[Layers::MOVING]      = XntPhysics::BroadPhase::DYNAMIC;
  layer_matrix.object_to_broadphase[Layers::PLAYER]      = XntPhysics::BroadPhase::DYNAMIC;

  // Broadphase layer masks
  layer_matrix.broadphase_masks[XntPhysics::BroadPhase::STATIC]   = (1 << XntPhysics::BroadPhase::DYNAMIC);

  layer_matrix.broadphase_masks[XntPhysics::BroadPhase::DYNAMIC]  = (1 << XntPhysics::BroadPhase::STATIC |
                                                                     1 << XntPhysics::BroadPhase::DYNAMIC);

  // Object collision masks
  layer_matrix.collision_masks[Layers::NON_MOVING]  = (1ULL << Layers::MOVING) |
                                                      (1ULL << Layers::PLAYER);

  layer_matrix.collision_masks[Layers::MOVING]      = (1ULL << Layers::NON_MOVING) |
                                                      (1ULL << Layers::MOVING) |
                                                      (1ULL << Layers::PLAYER);

  layer_matrix.collision_masks[Layers::PLAYER]      = (1ULL << Layers::NON_MOVING) |
                                                      (1ULL << Layers::MOVING);

  return layer_matrix;
}


void DestroyEngine(ecsxn::Registry &registry)
{
  auto &core_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::CoreContext>(registry);
  auto &swapchain_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::SwapchainContext>(registry);
  auto &graphics_pipeline_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::GraphicsPipelineContext>(registry);
  auto &render_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::RenderContext>(registry);

  XntDebug::Log("------ Engine Destruction ------");

  XntDebug::Log(">----- Render objects ------");
  XntGraphics::DestroyRenderContext(render_context, core_context.device, log_verbosity);

  XntDebug::Log(">----- Graphics Pipeline context ------");
  XntGraphics::DestroyGraphicsPipelineContext(graphics_pipeline_context, core_context.device, 
                                              core_context.allocator, log_verbosity);

  XntDebug::Log(">----- Swapchain context ------");
  XntGraphics::DestroySwapchainContext(swapchain_context, core_context.device, log_verbosity);

  XntDebug::Log(">----- Core context ------");
  XntGraphics::DestroyCoreContext(core_context, log_verbosity);

  XntDebug::Log(">----- Display ------");
  XntDisplay::DestroyDisplay(core_context.window, log_verbosity);
}



const char* application_name = "Cube Platformer 01";
uint32_t application_version = VK_MAKE_VERSION(1, 0, 0);

int window_width   = 1280;
int window_height  = 720;


int main()
{
  // ----- Intialization
  // ---- Subsystems
  XntCore::InitializePlatformSubsystems(Xenith::Core::SubsystemFlags::VIDEO);

  // ---- ECS Registry
  ecsxn::Registry registry;

  // ---- Contexts
  XntEngineECS::CoreContext &core_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::CoreContext>(registry);
  XntEngineECS::GraphicsContext &graphics_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::GraphicsContext>(registry);
  XntEngineECS::SwapchainContext &swapchain_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::SwapchainContext>(registry);
  XntEngineECS::GraphicsPipelineContext &graphics_pipeline_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::GraphicsPipelineContext>(registry);
  XntEngineECS::RenderContext &render_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::RenderContext>(registry);

  auto layer_matrix = CreateLayerMatrix();
  XntPhysics::PhysicsContext &physics_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntPhysics::PhysicsContext>(registry, layer_matrix);


  // aliases
  auto &window = core_context.window;
  auto &instance = core_context.instance;
  auto &surface = core_context.surface;
  auto &physical_device = core_context.physical_device;
  auto &device = core_context.device;
  auto &allocator =  core_context.allocator;

  auto &graphics_queue_index = graphics_context.graphics_queue_family_index;
  auto &graphics_queue = graphics_context.graphics_queue;

  auto &surface_capabilities = swapchain_context.surface_capabilities;
  auto &surface_format = swapchain_context.surface_format;
  auto &swapchain_width = swapchain_context.width;
  auto &swapchain_height = swapchain_context.height;
  auto &swapchain = swapchain_context.swapchain;
  auto &swapchain_images = swapchain_context.swapchain_images;
  auto &swapchain_image_views = swapchain_context.swapchain_image_views;

  auto &vertex_shader = graphics_pipeline_context.vertex_shader;
  auto &fragment_shader = graphics_pipeline_context.fragment_shader;
  auto &pipeline_layout = graphics_pipeline_context.pipeline_layout;
  auto &depth_texture = graphics_pipeline_context.depth_texture;
  auto &graphics_pipeline = graphics_pipeline_context.graphics_pipeline;

  auto &timeline_semaphore = render_context.timeline_semaphore;
  auto &image_acquired_semaphores = render_context.image_acquired_semaphores;
  auto &render_finished_semaphores = render_context.render_finished_semaphores;
  auto &command_resources = render_context.command_resources;
  auto &frame_index = render_context.frame_index;
  auto &next_signal_value = render_context.next_signal_value;

  uint32_t max_frames_in_flight = 3;


  // ---- Window
  window = XntDisplay::CreateWindow(application_name, window_width, window_height,
                                    XntDisplay::CreateWindowFlags::RESIZABLE);

  // ---- Vulkan
  // --- Instance
  XntGraphics::InstanceConfig instance_config = {
    .application_info = {
      .application_name     = application_name,
      .application_version  = application_version
    },

    .validation_layers_toggle = true
  };
  instance = XntGraphics::CreateVulkanInstance(instance_config, log_verbosity);

  if(instance == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // --- Surface
  surface = XntGraphics::CreateSurface(window, instance, log_verbosity);

  if(surface == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // --- Physical Device fetch
  XntGraphics::PhysicalDeviceSelectConfig select_config = {
    .physical_device_type_priority = XntGraphics::PhysicalDeviceTypes::DISCRETE_GPU
  };
  physical_device = XntGraphics::FindAndSelectPhysicalDevice(instance, surface, select_config, log_verbosity);

  if(physical_device == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // --- Graphics queue family index fetch
  graphics_queue_index = XntGraphics::FindGraphicsFamilyQueueIndex(physical_device, surface, log_verbosity);

  if(graphics_queue_index == 0xFFFFFFFF)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // --- Logical Device
  XntGraphics::LogicalDeviceConfig logical_device_config = {
    .graphics_queue_family_index = graphics_queue_index
  };
  device = XntGraphics::CreateLogicalDevice(physical_device, logical_device_config, log_verbosity);

  if(device == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // --- Graphics queue
  graphics_queue = XntGraphics::GetDeviceQueue(device, graphics_queue_index);

  // --- Vulkan Memory Allocator
  allocator = XntGraphics::CreateVulkanMemoryAllocator(instance, physical_device,
                                                       device, log_verbosity);

  // --- Swapchain
  XntDebug::Log("------ Swapchain ------");

  // -- Surface properties
  surface_capabilities = XntGraphics::GetSurfaceCapabilities(physical_device, surface);
  
  XntGraphics::SwapchainSurfaceFormatConfig surface_format_config = {
    .color_mode = XntGraphics::SurfaceColorMode::SDR_SRGB
  };
  surface_format = XntGraphics::ChooseSwapchainSurfaceFormat(physical_device, surface,
                                                             surface_format_config,
                                                             log_verbosity);

  if(surface_format.format == VK_FORMAT_UNDEFINED)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // -- Present mode
  XntDisplay::PresentMode chosen_present_mode = XntDisplay::ChooseSwapchainPresentMode(physical_device, surface, 
                                                                                       XntDisplay::PresentModes::IMMEDIATE,
                                                                                       log_verbosity);

  // -- Swapchain creation
  swapchain_width   = window_width;
  swapchain_height  = window_height;

  swapchain = XntGraphics::CreateSwapchain(device, swapchain_width, swapchain_height, surface,
                                           surface_capabilities, surface_format, chosen_present_mode, log_verbosity);

  if(swapchain == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // -- Images and image views
  XntGraphics::InitializeSwapchainImagesAndImageViewsContainers(device, swapchain, surface_format, 
                                                                swapchain_images, swapchain_image_views,
                                                                log_verbosity);


  // --- Graphics Pipeline
  XntDebug::Log("------ Graphics Pipeline ------");

  // -- Shaders
  vertex_shader = XntGraphics::LoadShader(device, "./default_shaders/default_color/col_vert.spv",
                                          "main", XntGraphics::ShaderTypeFlags::VERTEX);

  fragment_shader = XntGraphics::LoadShader(device, "./default_shaders/default_color/col_frag.spv",
                                            "main", XntGraphics::ShaderTypeFlags::FRAGMENT);

  // -- Pipeline Layout
  pipeline_layout = XntGraphics::CreatePipelineLayout(device, 
                                                      XntGraphics::ShaderTypeFlags::VERTEX,
                                                      sizeof(PushConstant));

  if(pipeline_layout.handle == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // -- Vertex Layout
  XntGraphics::VertexLayout vertex_layout = XntGraphics::LoadEngineDefaultVertexLayoutColor();

  // -- Depth Texture
  depth_texture = XntGraphics::CreateDepthTexture(device, allocator, window_width, window_height,
                                                  XntGraphics::DepthFormats::FLOAT_32, log_verbosity);

  if(depth_texture.handle == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // -- Graphics Pipeline creation
  XntGraphics::RasterizerConfig rasterizer_config = {
    .primitive_topology  = XntGraphics::PrimitiveTopologies::TRIANGLE_LIST,
    .fill_mode           = XntGraphics::FillModes::FILL,
    .cull_mode_flags     = XntGraphics::CullModeFlags::NONE
  };

  XntGraphics::ColorBlendConfig color_blend_config = XntGraphics::LoadEngineDefaultColorBlendConfig();
  XntGraphics::DepthConfig depth_config = XntGraphics::LoadEngineDefaultDepthConfig();

  XntGraphics::GraphicsPipelineConfig graphics_pipeline_config = {
    .color_attachment_format  = surface_format.format,
    .depth_attachment_format  = depth_texture.format,

    .rasterizer_config   = rasterizer_config,
    .color_blend_config  = color_blend_config,
    .depth_config        = depth_config
  };

  graphics_pipeline = XntGraphics::CreateGraphicsPipeline(device, vertex_shader, fragment_shader,
                                                          pipeline_layout, vertex_layout,
                                                          graphics_pipeline_config, log_verbosity);

  if(graphics_pipeline == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }


  // --- Synchronization
  XntDebug::Log("------ Synchronization ------");

  // -- Timeline semaphore
  timeline_semaphore = XntGraphics::CreateTimelineSemaphore(device, max_frames_in_flight, log_verbosity);

  if(timeline_semaphore == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

  // -- Binary semaphores
  XntGraphics::InitializeImageAcquiredSemaphoresContainer(device, max_frames_in_flight,
                                                          image_acquired_semaphores, log_verbosity);

  XntGraphics::InitializeRenderFinishedSemaphoresContainer(device, max_frames_in_flight, 
                                                           render_finished_semaphores, log_verbosity);

  // -- Command resources
  XntGraphics::InitializeCommandResourcesCommandPoolsAndAllocateCommandBuffers(device,
                                                                               max_frames_in_flight,
                                                                               command_resources,
                                                                               graphics_queue_index,
                                                                               log_verbosity);
  next_signal_value = max_frames_in_flight + 1;

  // ---- Physics system
  XntPhysics::PhysicsSystemInitializationConfig physics_system_init_config = XntPhysics::LoadEngineDefaultPhysicsSystemInitializationConfig();
  XntPhysics::InitializePhysicsSystem(physics_context, physics_system_init_config);


  // ---- Main game loop
  bool is_running = true;
  bool is_focused = true;
  XntCore::SetRelativeMouseMode(window, is_focused);

  XntCore::Event event;

  float fps = 240.0f;
  XntTime::SetupLimitFramerate(fps);

  ecs::WindowContext window_context = {
    .is_resizing = false,
    .resize_timer = 0.0f,
    .resize_time_cooldown = 0.001f
  };


  // --- Camera
  float camera_fov = 90.0f;
  XntMath::Vec3 camera_start_position = { 1.0f, 1.0f, 2.0f };
  XntMath::Vec3 camera_start_target = { 0.0f, 0.0f, -1.0f };
  XntMath::Vec3 camera_up = XntMath::Vec3{ 0.0f, 1.0f, 0.0f };

  float camera_sensitivity = 4.0f / 1000.0f;

  ecs::Camera camera = {
    .projection_matrix = XntMath::PerspectiveProjection(camera_fov, 
                                                        XntDisplay::GetWindowAspect(window), 
                                                        0.1f, 1000.0f,
                                                        XntMath::CoordinateSystem::RIGHT_HANDED),
    .view_matrix = XntMath::LookAt(camera_start_position, camera_start_target, camera_up),

    .position = camera_start_position,

    .forward = { 0.0f, 0.0f, -1.0f },
    .right = { 1.0f, 0.0f, 0.0f },
    .up = camera_up
  };

  // --- Models
  XntDebug::Log("------ Models ------");

  XntPhysics::PhysicsSystem &physics_system = physics_context.physics_system;
  XntPhysics::BodyInterface &body_interface = XntPhysics::GetBodyInterfaceFromPhysicsSystem(physics_system);

  // const std::array<uint8_t, 4> CYAN   = { 0,   255, 255, 255 };
  // const std::array<uint8_t, 4> PINK   = { 255, 105, 180, 255 };
  // const std::array<uint8_t, 4> RED    = { 255, 15,  15,  255 };
  // const std::array<uint8_t, 4> GREEN  = { 20,  250, 20,  255 };
  // const std::array<uint8_t, 4> WHITE  = { 200, 200, 200, 255 };
  // const std::array<uint8_t, 4> BLUE   = { 0,   0,   255, 255 };
  // const std::array<uint8_t, 4> BLACK  = { 3,   3,   3,   255 };

  // Vibrant Modern Synthwave / Arcade Palette
  const std::array<uint8_t, 4> CYAN   = {  32, 210, 244, 255 };
  const std::array<uint8_t, 4> PINK   = { 255,  85, 160, 255 };
  const std::array<uint8_t, 4> RED    = { 240,  65,  75, 255 };
  const std::array<uint8_t, 4> GREEN  = {  46, 213, 115, 255 };
  const std::array<uint8_t, 4> WHITE  = { 220, 225, 235, 255 };
  const std::array<uint8_t, 4> BLUE   = {  83,  82, 237, 255 };
  const std::array<uint8_t, 4> BLACK  = {  4,  6,  6, 255 };

  std::vector<XntGraphics::Mesh> box_meshes;

  auto create_colored_box_mesh = [&](std::array<uint8_t, 4> color)
  {
    XntGraphics::Mesh mesh_to_return = XntGraphics::GenerateBoxMesh({ 1.0f, 1.0f, 1.0f }, color);

    XntGraphics::UploadMeshBuffersToVRAMStaticSingle(device, allocator, graphics_queue,
                                                     command_resources[0].command_pool,
                                                     mesh_to_return.cpu_vertices, mesh_to_return.cpu_indices,
                                                     mesh_to_return.buffers,
                                                     true, log_verbosity); 

    return mesh_to_return;
  };

  box_meshes.push_back(create_colored_box_mesh(CYAN));
  box_meshes.push_back(create_colored_box_mesh(PINK));
  box_meshes.push_back(create_colored_box_mesh(RED));
  box_meshes.push_back(create_colored_box_mesh(GREEN));
  box_meshes.push_back(create_colored_box_mesh(WHITE));
  box_meshes.push_back(create_colored_box_mesh(BLUE));
  box_meshes.push_back(create_colored_box_mesh(BLACK));

  size_t cyan_index   = 0;
  size_t pink_index   = 1;
  size_t red_index    = 2;
  size_t green_index  = 3;
  size_t white_index  = 4;
  size_t blue_index   = 5;
  size_t black_index  = 6;


  // -- Player cube
  ecsxn::Entity player_entity = ecsxn::CreateEntityInRegistry(registry);

  ecs::Transform player_transform = {
    .position  = { 0.0f, 0.0f, -1.0f },
    .scale     = { 1.0f, 1.0f, 1.0f }
  };

  XntPhysics::BoxShapeSettings player_box_shape_settings = {
    .extent = player_transform.scale
  };
  XntPhysics::ShapeReference player_box_shape = XntPhysics::CreateBoxShapeReference(player_box_shape_settings);

  XntPhysics::RigidbodySettings player_box_rigidbody_settings = {
    .shape_reference = player_box_shape,
    .position = player_transform.position,
    .rotation_quaternion = XntMath::QuaternionIdentity(),
    .motion_type = XntPhysics::MotionTypes::DYNAMIC,
    .object_layer = Layers::PLAYER,
    .mass = 5.0f,
    .allowed_degrees_of_freedom_flags = XntPhysics::AllowedDegreesOfFreedoms::TRANSLATION_XYZ
  };
  XntPhysics::Rigidbody player_rigidbody = XntPhysics::CreateRigidbody(body_interface, player_box_rigidbody_settings,
                                                                       player_entity, true,
                                                                       XntPhysics::ActivationModes::ACTIVATE); 

  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, player_entity, player_transform);
  ecsxn::EmplaceEntityComponentToRegistry<XntPhysics::Rigidbody>(registry, player_entity, player_rigidbody);
  ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, player_entity, &box_meshes[black_index]);

  // - Movement
  float player_cube_speed = 4.5f;
  float player_cube_jump_velocity = 7.7f;

  bool is_grounded = false;
  
  float coyote_timer = 0.0f;
  float coyote_timer_limit = 0.25f;


  // -- Floor 
  ecsxn::Entity floor_entity = ecsxn::CreateEntityInRegistry(registry);

  ecs::Transform floor_transform = {
    .position  = { 0.0f, -3.0f, 0.0f },
    .scale     = { 16.0f, 1.0f, 16.0f }
  };

  XntPhysics::BoxShapeSettings floor_box_shape_settings = {
    .extent = floor_transform.scale
  };
  XntPhysics::ShapeReference floor_box_shape = XntPhysics::CreateBoxShapeReference(floor_box_shape_settings);

  XntPhysics::RigidbodySettings floor_box_rigidbody_settings = {
    .shape_reference = floor_box_shape,
    .position = floor_transform.position,
    .rotation_quaternion = XntMath::QuaternionIdentity(),
    .motion_type = XntPhysics::MotionTypes::STATIC,
    .object_layer = Layers::NON_MOVING,
  };
  XntPhysics::Rigidbody floor_box_rigidbody = XntPhysics::CreateRigidbody(body_interface, floor_box_rigidbody_settings,
                                                                          floor_entity, true,
                                                                          XntPhysics::ActivationModes::DONT_ACTIVATE);

  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, floor_entity, floor_transform);
  ecsxn::EmplaceEntityComponentToRegistry<XntPhysics::Rigidbody>(registry, floor_entity, floor_box_rigidbody);
  ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, floor_entity, &box_meshes[blue_index]);


  // -- Obstacles / Walls / Blocks boxes
  auto create_static_object = [&](ecs::Transform transform, size_t color_index)
  {
    ecsxn::Entity static_object_entity = ecsxn::CreateEntityInRegistry(registry);

    ecs::Transform static_object_transform = transform;

    XntPhysics::BoxShapeSettings static_box_shape_settings = {
      .extent = static_object_transform.scale
    };
    XntPhysics::ShapeReference static_box_shape = XntPhysics::CreateBoxShapeReference(static_box_shape_settings);

    XntPhysics::RigidbodySettings static_box_rigidbody_settings = {
      .shape_reference = static_box_shape,
      .position = static_object_transform.position,
      .rotation_quaternion = XntMath::QuaternionIdentity(),
      .motion_type = XntPhysics::MotionTypes::STATIC,
      .object_layer = Layers::NON_MOVING,
    };
    XntPhysics::Rigidbody static_box_rigidbody = XntPhysics::CreateRigidbody(body_interface, static_box_rigidbody_settings,
                                                                             static_object_entity, true,
                                                                             XntPhysics::ActivationModes::DONT_ACTIVATE);

    ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, static_object_entity, static_object_transform);
    ecsxn::EmplaceEntityComponentToRegistry<XntPhysics::Rigidbody>(registry, static_object_entity, static_box_rigidbody);
    ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, static_object_entity, &box_meshes[color_index]);
  };

  ecs::Transform block_1 = {
    .position  = XntMath::Vec3 { 0.9f, -1.8f, 9.9f },
    .scale     = XntMath::Vec3 {1.3f, 0.5f, 1.3f}
  };
  create_static_object(block_1, red_index);

  ecs::Transform block_2 = {
    .position  = XntMath::Vec3 { 0.9f, -1.8f, 13.9f },
    .scale     = XntMath::Vec3 { 1.5f, 0.5f, 1.5f }
  };
  create_static_object(block_2, pink_index);

  ecs::Transform wall_1 = {
    .position  = XntMath::Vec3 { 0.9f, -1.0f, 15.1f },
    .scale     = XntMath::Vec3 { 1.3f, 15.5f, 0.5f }
  };
  create_static_object(wall_1, white_index);

  ecs::Transform block_3 = {
    .position  = XntMath::Vec3 { 0.9f, -1.8f, 17.3f },
    .scale     = XntMath::Vec3 { 1.5f, 0.5f, 1.5f }
  };
  create_static_object(block_3, green_index);

  ecs::Transform block_4 = {
    .position  = XntMath::Vec3 { 0.9f, -0.5f, 21.3f },
    .scale     = XntMath::Vec3 { 1.5f, 0.3f, 1.5f }
  };
  create_static_object(block_4, cyan_index);

  // -- Dummy
  ecsxn::Entity dummy_entity = ecsxn::CreateEntityInRegistry(registry);

  ecs::Transform dummy_transform = {
    .position  = XntMath::Vec3{ 4.5f, -2.0f, 1.0f },
    .scale     = XntMath::Vec3{ 0.8f, 1.0f, 0.8f }
  };

  XntPhysics::BoxShapeSettings dummy_box_shape_settings = {
    .extent = dummy_transform.scale
  };
  XntPhysics::ShapeReference dummy_box_shape_reference = XntPhysics::CreateBoxShapeReference(dummy_box_shape_settings);

  XntPhysics::RigidbodySettings dummy_box_rigidbody_settings = {
    .shape_reference = dummy_box_shape_reference,
    .position = dummy_transform.position,
    .rotation_quaternion = XntMath::QuaternionIdentity(),
    .motion_type = XntPhysics::MotionTypes::DYNAMIC,
    .object_layer = Layers::MOVING,

    .mass = 600.0f,
    .allowed_degrees_of_freedom_flags = XntPhysics::AllowedDegreesOfFreedoms::TRANSLATION_XYZ,
  };
  XntPhysics::Rigidbody dummy_rigidbody = XntPhysics::CreateRigidbody(body_interface, dummy_box_rigidbody_settings,
                                                                      dummy_entity, true,
                                                                      XntPhysics::ActivationModes::ACTIVATE);

  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, dummy_entity, dummy_transform);
  ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, dummy_entity, &box_meshes[green_index]);
  ecsxn::EmplaceEntityComponentToRegistry<XntPhysics::Rigidbody>(registry, dummy_entity, dummy_rigidbody);


  while(is_running)
  {
    // ----- Update
    XntTime::UpdateTime();

    float dt = XntTime::CalculateDeltaTimeFloat();

    XntPhysics::UpdatePhysicsSystemStep(physics_context, dt);
    
    // ---- Handle events
    XntInput::PresentUpdateKeyboard();

    XntInput::ZeroMouseDeltas();

    while(XntCore::PollEvent(&event))
    {
      // --- Handle quitting
      if(event.type == XntCore::EventTypes::QUIT)
        is_running = false;

      // --- Handle resizing
      if(event.type == XntCore::EventTypes::WINDOW_PIXEL_SIZE_CHANGED)
      {
        window_width   = event.window.width;
        window_height  = event.window.height;

        window_context.is_resizing = true;
      }

      // --- Handle mouse input
      // -- Mouse motion
      if(event.type == XntCore::EventTypes::MOUSE_MOTION)
      {
        XntInput::CalculateMouseDeltas(event.mouse_motion.x_relative, event.mouse_motion.y_relative);

        XntInput::UpdateMousePosition(event.mouse_motion.x, event.mouse_motion.y);
      }
    }

    if(XntInput::IsKeyPressed(XntInput::Keys::ESCAPE))
      is_running = false;

    if(XntInput::IsKeyPressed(XntInput::Keys::I) && !is_focused)
    {
      XntCore::SetRelativeMouseMode(window, true);
      is_focused = true;
    }
    else if(XntInput::IsKeyPressed(XntInput::Keys::O) && is_focused)
    {
      XntCore::SetRelativeMouseMode(window, false);
      is_focused = false;
    }


    // ---- Resize timer
    if(window_context.is_resizing)
      window_context.resize_timer += dt;

    // ---- Player
    auto &player_transform_got = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, player_entity);
    auto &player_rigidbody_id = ecsxn::GetEntityComponentFromRegistry<XntPhysics::Rigidbody>(
        registry, player_entity
    ).body_id;
    player_transform_got.position = XntPhysics::GetRigidbodyPositionFromBodyInterface(body_interface, player_rigidbody_id);

    // --- Dummy sync
    auto &dummy_transform_got = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, dummy_entity);
    auto &dummy_rigidbody_id = ecsxn::GetEntityComponentFromRegistry<XntPhysics::Rigidbody>(registry, dummy_entity).body_id;
    dummy_transform_got.position = XntPhysics::GetRigidbodyPositionFromBodyInterface(body_interface, dummy_rigidbody_id);



    // --- Camera 1
    XntMath::Vec2 mouse_delta_xy = XntInput::GetMouseDeltaXY();
    if(!is_focused)
      mouse_delta_xy = { 0.0f, 0.0f };

    camera.yaw    += mouse_delta_xy.x * camera_sensitivity;
    camera.pitch  -= mouse_delta_xy.y * camera_sensitivity;
    camera.pitch  = XntMath::ClampValueReturn(camera.pitch, -1.56f, 1.56f);

    camera.forward.x = cosf(camera.pitch) * cosf(camera.yaw);
    camera.forward.y = sinf(camera.pitch);
    camera.forward.z = cosf(camera.pitch) * sinf(camera.yaw);
    camera.forward = XntMath::NormalizeReturn(camera.forward);

    XntMath::Vec3 camera_forward_flat = camera.forward;
    camera_forward_flat.y = 0.0f;
    camera_forward_flat = XntMath::NormalizeReturn(camera_forward_flat);

    camera.right = XntMath::CrossProductReturn(camera.forward, camera.up);
    camera.right = XntMath::NormalizeReturn(camera.right);

    // --- Movement
    // -- Input
    XntMath::Vec3 move_input_direction{0, 0, 0};

    move_input_direction.x = float(XntInput::IsKeyHeld(XntInput::Keys::D))
                           - float(XntInput::IsKeyHeld(XntInput::Keys::A));

    move_input_direction.z = float(XntInput::IsKeyHeld(XntInput::Keys::W))
                           - float(XntInput::IsKeyHeld(XntInput::Keys::S));

    XntMath::Vec3 desired_move_direction = (camera.right * move_input_direction.x) +
                                           0.0f +
                                           (camera_forward_flat * move_input_direction.z);

    if(XntMath::LengthSquared(desired_move_direction) > 0.01f)
    {
      desired_move_direction = XntMath::NormalizeReturn(desired_move_direction);
    }

    // -- Physics
    XntMath::Vec3 player_current_position = XntPhysics::GetRigidbodyPositionFromBodyInterface(body_interface, player_rigidbody_id);
    XntMath::Vec3 player_current_velocity = XntPhysics::GetRigidbodyLinearVelocityFromBodyInterface(body_interface, player_rigidbody_id);

    float player_new_velocity_y = player_current_velocity.y - 0.05f;


    // Grounded check
    float cube_half_height = player_transform_got.scale.y * 0.5f;
    XntMath::Vec3 cube_ray_origin = player_current_position - XntMath::Vec3{ 0.0f, cube_half_height, 0.0f };
    XntMath::Vec3 cube_ray_direction = { 0.0f, -0.025f, 0.0f };

    is_grounded = XntPhysics::CastRayNarrowPhaseQueryWithObjectLayerFilter(physics_system, 
                                                                           cube_ray_origin, cube_ray_direction,
                                                                           (1ULL << Layers::NON_MOVING) | 
                                                                           (1ULL << Layers::MOVING));

    if(is_grounded)
      coyote_timer = coyote_timer_limit;
    else
      coyote_timer -= dt;

    // Jumping
    if(XntInput::IsKeyHeld(XntInput::Keys::SPACEBAR) && coyote_timer > 0.0f)
    {
      player_new_velocity_y = player_cube_jump_velocity;
      is_grounded = false;

      coyote_timer = 0.0f;
    }

    // Apply player velocity
    XntMath::Vec3 player_cube_velocity_vector = {
      desired_move_direction.x * player_cube_speed,
      player_new_velocity_y,
      desired_move_direction.z * player_cube_speed
    };

    XntPhysics::SetRigidbodyLinearVelocityInBodyInterface(body_interface, player_rigidbody_id, 
                                                          player_cube_velocity_vector);


    // --- Camera 2
    float follow_distance = 2.7f;
    XntMath::Vec3 camera_target = player_transform_got.position;
    camera.position = camera_target - (camera.forward * follow_distance);

    camera.view_matrix = XntMath::LookAt(camera.position, camera_target, camera.up);

    // ----- Render
    // ---- Handle window resize
    if(window_width != swapchain_width || window_height != swapchain_height)
    {
      if(window_context.resize_timer > window_context.resize_time_cooldown)
      {
        XntGraphics::DeviceWaitIdle(device);

        if(window_width > 0 && window_height > 0)
        {
          XntGraphics::SurfaceCapabilities resize_surface_capabilities = XntGraphics::GetSurfaceCapabilities(physical_device, surface);

          uint32_t actual_width   = XntGraphics::CalculateWindowWidthForSwapchainRecreation(window_width, resize_surface_capabilities);
          uint32_t actual_height  = XntGraphics::CalculateWindowHeightForSwapchainRecreation(window_height, resize_surface_capabilities);

          XntGraphics::Swapchain old_swapchain = swapchain_context.swapchain;
          XntGraphics::Swapchain new_swapchain = XntGraphics::CreateSwapchain(
              device,
              actual_width, actual_height,
              surface, resize_surface_capabilities,
              swapchain_context.surface_format, 
              chosen_present_mode,
              log_verbosity_off,
              old_swapchain
          );

          XntGraphics::DestroySwapchainContext(swapchain_context, device, log_verbosity_off);
          swapchain_context.swapchain = new_swapchain;

          XntGraphics::InitializeSwapchainImagesAndImageViewsContainers(device, new_swapchain,
                                                                        swapchain_context.surface_format,
                                                                        swapchain_images, swapchain_image_views,
                                                                        log_verbosity_off);


          XntGraphics::DestroyTexture(device, allocator, depth_texture,
                                      log_verbosity_off);
          depth_texture = XntGraphics::CreateDepthTexture(device, allocator,
                                                          actual_width, actual_height,
                                                          XntGraphics::DepthFormats::FLOAT_32,
                                                          log_verbosity_off);

          swapchain_context.width = actual_width;
          swapchain_context.height = actual_height;

          window_context.is_resizing = false;
          window_context.resize_timer = 0.0f;
        }
      }
    }

    if(window_context.is_resizing && window_context.resize_timer < window_context.resize_time_cooldown)
    {
      XntTime::LimitFramerate();
      continue;
    }


    const uint32_t frame_resource_index = frame_index % max_frames_in_flight;
    const uint32_t signal_value = next_signal_value;
    const uint64_t wait_value = signal_value - max_frames_in_flight;

    XntGraphics::WaitTimelineSemaphore(device, timeline_semaphore, wait_value);

    XntGraphics::Semaphore current_image_acquired_semaphore   = image_acquired_semaphores[frame_resource_index];
    XntGraphics::Semaphore current_render_finished_semaphore  = render_finished_semaphores[frame_resource_index];
    XntGraphics::CommandResources &current_command_resources  = command_resources[frame_resource_index];

    XntGraphics::ResetCommandPool(device, current_command_resources.command_pool);

    XntGraphics::SwapchainAcquisitionResult acquisition = XntGraphics::AcquireNextImageFromSwapchain(
        device, swapchain, current_image_acquired_semaphore 
    );

    const XntGraphics::StatusResult acquire_result = acquisition.result;
    const uint32_t current_image_index = acquisition.image_index;

    // --- Handle swapchain recreation checks
    if(acquire_result == XntGraphics::StatusResults::ERROR_OUT_OF_DATE ||
       acquire_result == XntGraphics::StatusResults::SUBOPTIMAL)
    {
      window_context.is_resizing = true;
      window_context.resize_timer = window_context.resize_time_cooldown + 0.1f;
      XntTime::LimitFramerate();

      continue;
    }
    else if(acquire_result != XntGraphics::StatusResults::SUCCESS)
    {
      XntDebug::Log("Failed to acquire swapchain image! Error code: %d", acquire_result);
      is_running = false;
      break;
    }


    // ---- Start command buffer record
    XntGraphics::BeginCommandBuffer(current_command_resources.command_buffer);

    XntGraphics::ImageMemoryBarrier swapchain_image_memory_barrier_render = XntGraphics::CreateSwapchainImageMemoryBarrierForRender(
        swapchain_images[current_image_index]
    );

    XntGraphics::ImageMemoryBarrier depth_image_memory_barrier_render = XntGraphics::CreateDepthImageMemoryBarrierForRender(
        depth_texture.handle
    );

    std::array<XntGraphics::ImageMemoryBarrier, 2> render_image_memory_barriers = {
      swapchain_image_memory_barrier_render,
      depth_image_memory_barrier_render
    };

    XntGraphics::PipelineBarrier(current_command_resources.command_buffer, 
                                 render_image_memory_barriers.size(),
                                 render_image_memory_barriers.data());

    // ---- Render config
    XntMath::Vec4 clear_color = { 0.06f, 0.07f, 0.16f, 1.0f };
    XntGraphics::ColorAttachmentConfig color_attachment_config = XntGraphics::CreateEngineDefaultColorAttachmentConfig(
        swapchain_image_views[current_image_index],
        clear_color
    );

    XntGraphics::DepthAttachmentConfig depth_attachment_config = XntGraphics::CreateEngineDefaultDepthAttachmentConfig(
        depth_texture.view
    );

    XntGraphics::RenderingConfig rendering_config = {
      .render_area_width   = swapchain_width,
      .render_area_height  = swapchain_height,

      .color_attachment_config_data = &color_attachment_config,
      .color_attachment_count = 1,

      .depth_attachment_config = depth_attachment_config
    };

    XntGraphics::BeginRendering(current_command_resources.command_buffer, rendering_config);
    {
      XntGraphics::SetViewportAndScissor(current_command_resources.command_buffer, 
                                         swapchain_width, swapchain_height);

      XntGraphics::BindPipeline(current_command_resources.command_buffer, graphics_pipeline,
                                XntGraphics::PipelineBindPoints::GRAPHICS);

      // -- render objects
      auto models_view = ecsxn::CreateFilteredEntityViewFromRegistryWithoutExclusion<ecs::Transform, XntGraphics::Mesh*>(registry);

      for(auto model_entity : models_view)
      {
        auto &model_transform = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, model_entity);
        auto &model_mesh = ecsxn::GetEntityComponentFromRegistry<XntGraphics::Mesh*>(registry, model_entity);

        XntMath::Matrix4 model_matrix{ 1.0f };

        XntMath::TranslateReference(model_matrix, model_transform.position);
        XntMath::ScaleVectorReference(model_matrix, model_transform.scale);

        PushConstant push_constant = {
          .model       = model_matrix,
          .view        = camera.view_matrix,
          .projection  = camera.projection_matrix
        };

        XntGraphics::PushConstants(current_command_resources.command_buffer, pipeline_layout.handle, 
                                   XntGraphics::ShaderTypeFlags::VERTEX, sizeof(push_constant),
                                   &push_constant);

        XntGraphics::BindSingleVertexBuffer(current_command_resources.command_buffer, model_mesh->buffers.vertex_buffer.handle);
        XntGraphics::BindIndexBuffer(current_command_resources.command_buffer, model_mesh->buffers.index_buffer.handle);

        XntGraphics::DrawIndexed(current_command_resources.command_buffer, model_mesh->index_count, 1);
      }
    }
    XntGraphics::EndRendering(current_command_resources.command_buffer);

    XntGraphics::ImageMemoryBarrier swapchain_image_memory_barrier_present = XntGraphics::CreateSwapchainImageMemoryBarrierForPresentation(
        swapchain_images[current_image_index]
    );
    XntGraphics::PipelineBarrier(current_command_resources.command_buffer, 
                                 1,
                                 &swapchain_image_memory_barrier_present);

    XntGraphics::EndCommandBuffer(current_command_resources.command_buffer);

    // ---- Frame submit
    XntGraphics::FrameSubmitConfig frame_submit_config = {
      .image_acquired_semaphore = current_image_acquired_semaphore,
      .current_render_finished_semaphore = current_render_finished_semaphore,
      .timeline_semaphore = timeline_semaphore,
      .signal_value = signal_value
    };
    XntGraphics::SubmitFrameSingleCommandBuffer(graphics_queue, current_command_resources.command_buffer, frame_submit_config);

    // ---- Frame Present
    XntGraphics::FramePresentConfig frame_present_config = {
      .swapchain = swapchain,
      .swapchain_image_index = current_image_index,
      .current_render_finished_semaphore = current_render_finished_semaphore
    };
    XntGraphics::PresentFrame(graphics_queue, frame_present_config);


    // after render loop
    frame_index++;
    next_signal_value++;

    XntTime::LimitFramerate();
    // XntDebug::Log("Frame time: %f", XntTime::CalculateFrameTimeMS());
  } // while(is_running)

  // ----- End
  XntGraphics::DeviceWaitIdle(device);

  for(auto &mesh : box_meshes)
  {
    XntGraphics::DestroyBuffer(allocator, mesh.buffers.vertex_buffer);
    XntGraphics::DestroyBuffer(allocator, mesh.buffers.index_buffer);
  }
  
  DestroyEngine(registry);

  return 0;
}

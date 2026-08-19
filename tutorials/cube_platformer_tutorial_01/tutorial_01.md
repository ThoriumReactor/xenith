# Tutorial 01: Learn and build your first game: "cube platformer"

Welcome to the first and official tutorial of Xenith. In this tutorial we will learn how to go from a simple window to a functional cube platformer game.

## What will you learn:
- Main simple graphics pipeline: essential graphic variables creation and why are they needed, rendering stage
- ECS, What are entities, components, making filtered view for entities and getting components from entities(e.g. getting Transform component from entity). What are contexts, why are they needed
- Handling events(e.g. How we can make input function in our main function, how we take mouse motion)
- Creating own ECS component structs e.g. Transform
- Initialization, update, for physics engine, how to create physics layer matrix, how you can make collision layers and make them functional(e.g. player layer can collide with moving and non moving layers, but can't collide with itself by just bitwise operations). Learn how to create rigidbodies and change their data(such as setting position/velocity)
- Learn how to create single instance where we combine physics rigidbody, mesh and ECS(e.g. creating player mesh, uploading mesh to VRAM, creating rigidbody for player and in the end add components to registry that player has)
- How to make resizing window functional

## Prerequisites 
First of all you have to download the library itself, you can do so at project github page, click on "Releases" and from there you download the xenith_release_(version)(e.g. xenith_release_0_0_1), in this zipped file you have most important stuff you need for this tutorial such as default shaders(Needed later for making our graphics pipeline), helper scripts(such as generating build, building), include, src and CMakeLists.txt that are in directory "xenith/".

Note: You can ignore helper_scripts directory, if you have your own scripts.

After this make sure your structure is right, so your default shaders and helper scripts are inside your main project directory, so it looks like this:
```
ls tutorial_cube_platformer_01

assets  external_libraries                         include
build   compile_commands.json  helper_scripts      src

```
Note: I moved default_shaders directory to assets/, so their location is assets/default_shaders/

Now as for Xenith library source code, you will have at your release unarchived zip a directory named "xenith/", it contains it's own include, src, and CMakeLists.txt for itself, just move that directory to your external_libraries directory, so it looks like external_libraries/xenith/

You have to make sure you meet requirements and link Xenith to your project, guide for this is located at main directory and named REQUIREMENTS_AND_LINKING.md

After linking, make sure your compilation works, you can do something simple as:
```
#include "xenith/debug_xn/logging.h"



int main()
{
  Xenith::Debug::Log("Hello world!");

  return 0;
}

```

If it works, then everything is great, and we can move on to making first steps for making our game.


## Part 1, A simple, empty window application
Before starting, if you have some troubles with code, you can always look at full code located at Xenith github page, in directory "game_examples/cube_platformer_tutorial_01" there will be the full code of this tutorial project, we're making.

Also, if namespaces are very long for you, you can always do alias for them, e.g.
```
namespace xntdisplay = Xenith::Display;
```
As for this project I will be using XntModule style, e.g. XntGraphics = Xenith::Graphics


### First lines of code
For creating window, first of all we need to initialize subsystem with video flag

```
XntCore::InitializePlatformSubsystems(XntCore::SubsystemFlags::VIDEO);
```

Then we need to create our window

```
XntDisplay::Window window = XntDisplay::CreateWindow(application_name, window_width, window_height, 
						     XntDisplay::CreateWindowFlags::RESIZABLE);
```
Note: in this example parameters for window were initialized on top of main:  

```
const char* application_name = "Cube Platformer 01";

int window_width   = 1280;
int window_height  = 720;

int main()
{
  ...
}
```

### ECS Contexts
Before we continue, we should put window on core context. 

#### What is even a ECS context and why should we use it? 
ECS Context is a very neat way to handle global variables, where you can request them from every function, by just handing ECS registry, as for why, it's a very easy way to, for example, destroy variables in our application, when application fails or ends, we need to free our resources e.g. XntGraphics::DestroyCoreContext(core_ctx), contexts eliminate our need to pass thousands of parameters in single function(Like DestroyEngine(Window, Device, Swaphain, ...), and accessing those contexts from different module points by just handing registry. 

#### But what is a registry? 
Registry, is a central unit that handles and contains contexts, components, entity ids. For example, with registry we can append or get components, for example,
``` 
EmplaceEntityComponentToRegistry<ecs::Transform>(registry, entity, transform)
``` 
Or get data: 
```
GetEntityComponentFromRegistry<ecs::Transform>(registry, entity)
```

### Remaking previous initialization

```
  // ----- Intialization
  // ---- Subsystems
  XntCore::InitializePlatformSubsystems(Xenith::Core::SubsystemFlags::VIDEO);

  // ---- ECS Registry
  ecsxn::Registry registry;

  // ---- Contexts
  XntEngineECS::CoreContext &core_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::CoreContext>(registry);
```

Just like that we created registry and emplaced core context, now we can initialize window in the core context

```
  core_context.window = XntDisplay::CreateWindow(application_name, window_width, window_height,
                                                 XntDisplay::CreateWindowFlags::RESIZABLE);

```
Although in our example it's pretty much tedious to write "module_context.object = ..." every time, in this tutorial I will use aliases for less tedious writing
```
// ---- Contexts
...

// aliases
auto &window = core_context.window;


// ---- Window
window = XntDisplay::CreateWindow(application_name, window_width, window_height,
                                  XntDisplay::CreateWindowFlags::RESIZABLE);
```
Now that we created window, we need to make it at least appear until we close it, for this we need to make our own while loop and poll events.
```
  // ---- Main game loop
  bool is_running = true;
  XntCore::Event event;

  while(is_running)
  {
    // ----- Update
    
    // ---- Handle events
    while(XntCore::PollEvent(&event))
    {
      if(event.type == XntCore::EventTypes::QUIT)
        is_running = false;
    }
  }
```
Now that way your window is going to be empty, and going to exist until you press window cross button.

## Part 2, Actually displaying something in window
Now that we have functioning empty window, we need to display something in our window(as for simplicity for now it will be just a clear color background), for this we need entire rendering pipeline: swapchain, graphics pipeline and synchronization

Let's learn what these terms mean
- **Swapchain** is an essential part of Vulkan rendering, it is a queue of presentable images(functioning as abstracted ring buffer). Instead of drawing directly on screen(which would give screen tears due to being unsynchronized). Swapchain solves this by way, where we acquire avaible image from swapchain, draw on them, submit and finally present.

- **Graphics pipeline** is an essential part for rendering, as it defines how geometry is processed and drawn on screen

- **Synchronization** is very important, as it keeps CPU with GPU working in step. Without it, it would've been terrible experience, for example the GPU might draw onto an image before it's even done being acquired, or the CPU could submit new frame, while GPU is still busy with rendering previous one.

### Main Vulkan variables
Now that we learned what we need for render pipeline, let's start with the first and most important variable for Vulkan to work.
- **Vulkan Instance** It is a bridge between our application code and Vulkan library driver.

First of all, in this part I will follow a code zwriting pattern for initialization that is a reverse (or down to top), meaning, first of all we will write a function
```
XntGraphics::CreateVulkanInstance(const Xenith::Graphics::InstanceConfig &instance_config, const Xenith::Debug::LogVerbosity log_verbosity)
```
And now, we see that two variables are missing, we will step upper from this line, creating essential variables we need for this function, e.g. in our current case we will create InstanceConfig and LogVerbosity.

Continuing onto Vulkan Instance, We need an instance config which defines what name our application has, application version, and validation layers toggle. Although validation layers are very useful when doing raw vulkan, it will still be very useful even in Xenith, since it can catch some undefined behaviour like out of sync image acquiring or vulkan-related memory leaks, so it's still very useful.

Now let's create those variables, as for InstanceConfig, we have variables above function, same for log verbosity
```
XntDebug::LogVerbosity log_verbosity = XntDebug::LogVerbosity::HIGH;


const char* application_name = "Cube Platformer 01";
uint32_t application_version = VK_MAKE_VERSION(1, 0, 0);

int window_width   = 1280;
int window_height  = 720;
```
That way, we can now initialize the InstanceConfig struct and create Vulkan instance:
```
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
```
Also if, instance or any other variable that we will initialize in future fail, they log message and return null handle value or similar, you can look that by hovering over function to see it's comment about which variable it returns in case of fail.

Now let's handle resource freeing, don't forget that we will also need to put our destroy function in the end aswell.

Since we are using contexts containers, it is very easy to free resources
```
void DestroyEngine(ecsxn::Registry &registry)
{
  auto &core_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::CoreContext>(registry);

  XntDebug::Log("------ Engine Destruction ------");

  XntDebug::Log(">----- Core context ------");
  XntGraphics::DestroyCoreContext(core_context, log_verbosity);
}
```
And now, instance creation:
```
  instance = XntGraphics::CreateVulkanInstance(instance_config, log_verbosity);

  if(instance == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }
```
And don't forget to put in the end,
```
  // ----- End
  DestroyEngine(registry);

  return 0;
```
Now that we've created Vulkan instance variable, we have also several other important variables, they are:
- **Surface** Is a bridge between Vulkan and our OS window system.
- **Physical Device** Is a variable showing our physical GPU's capabilities, that we need to fetch
- **Logical Device** Is our application's interface to physical GPU, that controls queues, features and memory allocations, basically a command unit for your physical GPU that tells what exactly your physical GPU should execute

Now let's code:
Note: It is very important to first create surface rather than physical device, since you can't know if physical device is actually capable of rendering your window until you have surface
```
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
```

If you tried to call CreateLogicalDevice, you could see that we have new dependency -- graphics_queue_family_index

```
  // --- Logical Device
  XntGraphics::LogicalDeviceConfig logical_device_config = {
    .graphics_queue_family_index = 
  }
  device = XntGraphics::CreateLogicalDevice(physical_device, logical_device_config, log_verbosity);
```
Let's learn what this new variable means:

**First of all what is even an queue family?**

When rendering or computing on GPU, your GPU doesn't do that on one monolithic channel, instead it has grouping for each task, there's different types for them:
- **Graphics Queue** Supports doing geometry, rasterization and rendering pipelines, basically it's an queue needed for rendering

   **IMPORTANT NOTE:** It's very common for primary graphics queue having support for compute and transfer operation on this same queue.

- **Compute Queue** Needed for compute shaders(Like a physics simulation calculated on GPU, particle simulation)
- **Transfer Queue** Optimized for data transfer, such as texture and buffer data going from RAM to VRAM

Now coming back to creating Logical Device, we need to fetch graphics family queue index, done like this:
```
  // --- Physical Device fetch
  ...
  
  // --- Graphics queue family index fetch
  graphics_queue_index = XntGraphics::FindGraphicsFamilyQueueIndex(physical_device, surface, log_verbosity);

  if(graphics_queue_index == 0xFFFFFFFF)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }
  
  // --- Logical Device
  ...
``` 
Note: Don't forget to add graphics context
```
  // ---- Contexts
  XntEngineECS::CoreContext &core_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::CoreContext>(registry);
  XntEngineECS::GraphicsContext &graphics_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::GraphicsContext>(registry);

  // aliases
  auto &window = core_context.window;
  auto &instance = core_context.instance;
  auto &surface = core_context.surface;
  auto &physical_device = core_context.physical_device;
  auto &device = core_context.device;

  auto &graphics_queue_index = graphics_context.graphics_queue_family_index;
```

Now, we can finish creating logical device:  
```
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
```

### Swapchain
Now let's make Swapchain

```
  XntGraphics::CreateSwapchain(logical_device, swapchain_width, swapchain_height, surface, surface_capabilities, surface_format, present_mode, log_verbosity)
```
There's several new additional dependencies for creating swapchain, which are:
surface capabilities, surface format and present mode, Let's learn what every of this mean:

- **Surface capabilities** As we learned before surface is bridge between Vulkan and our OS window system, surface capabilities show us, what our surface is capable of, for example it's maximum extent, it's minimum extent, how much swapchain images it can contain(min and max)
- **Surface format** Defines how colors are packed in memory(8 bit RGBA, 10 bit RGB) and how colors are interpreted by monitor(SDR / sRGB / HDR10)
- **Present mode** Controls how images are swapped to screen by swapchain, basically your swapchain synchronization strategy(e.g. VSYNC, IMMEDIATE(Which is the same as uncapped fps))

Let's create our variables, but first don't forget to add new context:
```
  XntEngineECS::SwapchainContext &swapchain_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::SwapchainContext>(registry);

```

Now let's start with first variables needed for swapchain creation
```
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

```

And present mode, but first of all let's learn what most popular present modes there are:  
- **VSYNC** or Vertical Synchronization, synchronizes frame presentation with your monitor's refresh rate, which doesn't have any screen tears, basically high input latency and zero tearing.
- **IMMEDIATE** or uncapped, unlike VSYNC, immediate present mode doesn't wait for your monitor's refresh cycle, but introduces screen tears, basically low input latency but tearing is sometimes present
- **MAILBOX** is a technique that may be not supported by all physical GPU or monitor devices, but it's purpose is to being uncapped while having zero screen tears, although it has more GPU power consumption. Basically low input latency and zero tearing(at cost of more GPU power consumption)

Now let's continue onto code
```
  // -- Present mode
  XntDisplay::PresentMode chosen_present_mode = XntDisplay::ChooseSwapchainPresentMode(physical_device, surface, 
                                                                                       XntDisplay::PresentModes::VSYNC,
                                                                                       log_verbosity);
```
I personally chose VSYNC present mode, but later after we can draw objects, I will set IMMEDIATE mode, because I want to render game at 240 fps, although you can choose whichever you want or need.

And finally after creating every variable needed for swapchain, let's create swapchain itself:
```
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
```
Also if you tried running the code at this point and your validation layers are on, you definitely got message:  
```
Validation Error:
vkDestroyDevice(): Object Tracking - For VkDevice, VkSwapchainKHR has not been destroyed.
```
This means we forgot to free swapchain resources after closing quitting while loop, let's fix this:
```
void DestroyEngine(ecsxn::Registry &registry)
{
  auto &core_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::CoreContext>(registry);
  auto &swapchain_context = ecsxn::GetContextComponentFromRegistry<XntEngineECS::SwapchainContext>(registry);

  XntDebug::Log("------ Engine Destruction ------");

  XntDebug::Log(">----- Swapchain context ------");
  XntGraphics::DestroySwapchainContext(swapchain_context, core_context.device, log_verbosity);

  XntDebug::Log(">----- Core context ------");
  XntGraphics::DestroyCoreContext(core_context, log_verbosity);
}
```
Also very important reminder is to free resources in reverse order, since you need to free resources first that you made earlier and then going up freeing the core resources

Before we go further, we also need swapchain images and image views. We would need those later in render loop

- **Image (`VkImage`):** is a raw handle to the image GPU memory allocated.
- **Image view (`VkImageView`)** is a view or mask that describes how we want to access that image and which part of it to use
```
  // -- Images and image views
  XntGraphics::InitializeSwapchainImagesAndImageViewsContainers(device, swapchain, surface_format, 
                                                                swapchain_images, swapchain_image_views,
                                                                log_verbosity);
```

### Graphics Pipeline
Now let's make a variable that actually defines how our application is rendered.

```
  XntGraphics::CreateGraphicsPipeline(logical_device, vertex_shader, fragment_shader, pipeline_layout, vertex_layout, graphics_pipeline_config, log_verbosity)
```
Let's learn all of the new dependencies:

- **Graphics Pipeline Shaders**
	+ Vertex Shader: essentially a shader that defines where our vertices should located on our 2D screen
	+ Fragment Shader: Figures out what color each pixel should have, whether it's calculating lighting, applying texture or just picking solid color
	
- **Pipeline Layout** Defines interface between your shaders and memory resources. It's needed for push constants and descriptor sets(we will learn about them in other tutorials, but basically it's essential concept needed for textures)
- **Vertex Layout** Describes your vertex data memory format (data types, location, input rate). Although in current tutorial we won't go deep into this, we will just load engine default vertex layout corresponding to vertex shader. But generally it's mandatory to fill in, if you aren't using engine default shaders.
- **Graphics pipeline configuration** defines how we want our vertex points assemble, how we want to rasterize them, configure blending, how depth testing should be calculated, but we will learn more precisely in a moment.

Now let's start by creating first variables we need for graphics pipeline
```
  // ---- Contexts
  -- core
  -- graphics
  -- swapchain
  XntEngineECS::GraphicsPipelineContext &graphics_pipeline_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::GraphicsPipelineContext>(registry);

```

Let's load shaders that you imported into your project from xenith release zip,
we loaded default shaders specifically for color.

We need shader location filepath, entrypoint name and shader type that we need to specify in creation. In every default shaders provided. 

Important note: There will be a README.md that specifies what entrypoints it has and what it expects in push constants.
```
  // --- Graphics Pipeline
  XntDebug::Log("------ Graphics Pipeline ------");

  // -- Shaders
  vertex_shader = XntGraphics::LoadShader(device, "./assets/default_shaders/default_color/col_vert.spv",
                                          "main", XntGraphics::ShaderTypeFlags::VERTEX);

  fragment_shader = XntGraphics::LoadShader(device, "./assets/default_shaders/default_color/col_frag.spv",
                                            "main", XntGraphics::ShaderTypeFlags::FRAGMENT);
```
Now pipeline layout
```
  // -- Pipeline Layout
  pipeline_layout = XntGraphics::CreatePipelineLayout(logical_device, push_constant_shader_type_flags, push_constant_size);
```
If you read the readme.md from shaders directory, you saw that we need a push constant with 3 matrices model, view and projection, this means we need to create our own push constant for this one:

Note: I created it at project_directory/include/systems/init/graphics/data.h
```
#pragma once

#include "xenith/math/math.h"


struct PushConstant
{
  alignas(16) Xenith::Math::Matrix4 model;
  alignas(16) Xenith::Math::Matrix4 view;
  alignas(16) Xenith::Math::Matrix4 projection;
};
```
Quick note on model, view, projection -- those are essential matrices for camera to put 3D perspective view onto 2D monitor:
- **Model** defines how we want to scale, rotate or wherever we want our object to be located at(Translation)
- **View** represents your camera position and orientation
- **Projection** depending on projection type, distorts camera view, adding depth, for example perspective projection which is pretty standard for 3D applications, requires fov(Field of view) so it adds depth, or orthographic projection which is standard for 2D games, where it doesn't need any depth added.

And now we can create pipeline layout:
```
  // -- Pipeline Layout
  pipeline_layout = XntGraphics::CreatePipelineLayout(device, 
                                                      XntGraphics::ShaderTypeFlags::VERTEX,
                                                      sizeof(PushConstant));

  if(pipeline_layout.handle == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }

```
Note: Push constant is a small amount of data that we can send directly to our shaders, in our case we want to send MVP data to vertex shader, so there it would calculate the final 2D screen positions for all of our 3D vertices.

As for vertex layout, in our case where we use default shaders, we can just load them right away:
```
  // -- Vertex Layout
  XntGraphics::VertexLayout vertex_layout = XntGraphics::LoadEngineDefaultVertexLayoutColor();

```

Now let's move onto graphics pipeline configuration and look what we need there:
```
  XntGraphics::GraphicsPipelineConfig graphics_pipeline_config = {
    // .color_attachment_format  = 
    // .depth_attachment_format  =
    //
    // .rasterizer_config   = 
    // .color_blend_config  =
    // .depth_config        =
  }
```
As for color attachment format, it's just our surface format's format(as we learned before it's how our pixel values are packed in memory), as for depth attachment format, this means we need to create depth texture.
```
  // -- Depth Texture
  depth_texture = XntGraphics::CreateDepthTexture(device, allocator, width, height, depth_format, log_verbosity);
```
We can see that we got new dependencies such as allocator and depth format

- **Allocator** Is a video memory allocator, basically needed for allocating meshes, textures and buffers
- **Depth format** Is our depth texture's format, the popular choice for modern hardware is FLOAT_32(for depth only) and FLOAT_32_STENCIL_UINT_8(for depth and stencil)

First we need to create allocator:
```
  // --- Logical Device
  ...

  // --- Vulkan Memory Allocator
  allocator = XntGraphics::CreateVulkanMemoryAllocator(instance, physical_device,
                                                      device, log_verbosity);

  // --- Swapchain
  ...

```
And now, we can make depth texture
```
  // -- Depth Texture
  depth_texture = XntGraphics::CreateDepthTexture(device, allocator, window_width, window_height,
                                                  XntGraphics::DepthFormats::FLOAT_32, log_verbosity);

  if(depth_texture.handle == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }
```


**Rasterization configuration:** 
- **Primitive Topology**
Usually meshes consist of what's called primitives, those are groups of vertex points assembled, For example a 3D cube is typically assembled from a group of triangles. In graphics pipeline you can choose how your primitives are assembled(e.g. triangle, line).

- **Fill Mode**
Except for ways we can assemble our primitives, we also can change how they are filled, e.g. a full filled triangle with color, or a line wireframe(For example it's a useful fill mode for making debug shapes, like for example hitbox box, or hitbox sphere) where our primitives visually consist of just lines. 

- **Cull mode**
Cull mode is a fundamental optimization technique where GPU doesn't render specified face(front or back). Usually you will see back face culling as default in most graphics pipelines, as it's a helpful technique that doesn't render the triangles camera doesn't see.

**Color Blend configuration:** We won't really discuss in details this one, but basically color blend configuration is usually needed for deciding transparency of objects and alpha blending, in this tutorial we will just load the default one.

**Depth configuration** Also an advanced topic, but basically it decides whether we should have depth test/depth write, what is the compare operator when deciding which one object should cover other. We will also load this one.

Now let's fill those configurations
```
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
```
I chose triangle list because it's pretty much standard for assembling, fill mode is full fill, and cull mode is none, because it's very important for us to first get working object, and then, after we have stable renderer with ability to render meshes, we can choose back culling.

Now we can create graphics pipeline:
```
  graphics_pipeline = XntGraphics::CreateGraphicsPipeline(device, vertex_shader, fragment_shader,
                                                          pipeline_layout, vertex_layout,
                                                          graphics_pipeline_config, log_verbosity);

  if(graphics_pipeline == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }
```


### Synchronization
We need to create and/or initialize essential variables for synchronizing:

- **Frames in flight** Concept that lets CPU work on recording for upcoming frames, while GPU actively executes work for previous ones, this lets us maximize the work of both CPU and GPU. popular choices are 2 and 3.
- **Timeline semaphore:** A Vulkan 1.2 object that acts as monotonically increasing counter(0, 1, 2, 3, ... n). The GPU increments counter upon completing a frame's workload. CPU checks and waits on this value before even beginning to acquire image from swapchain.
- **Image acquired semaphores** is a binary semaphore, we will use for synchronizing when we acquire swapchain image, making sure GPU waits until the swapchain image is ready for drawing.
- **Render finished semaphores** a binary semaphore, we will use for synchronizing after finishing rendering, letting present know that it is safe to display the rendered image on screen.

Important note: Since we will be using multi frames in flight, we need vector containers for both image acquire and render finished semaphores.

- **Command resources** In our rendering phase, in order to even begin rendering we need to begin recording commands into a command buffer and after we rendered our mesh objects, we have to submit the command buffer to graphics queue. But for command buffers we need command pools, they are important since they handle allocating command buffers.

Important note: Since we will be using multi frames in flight, we need a vector container of CommandResources which will contain command pools and command buffer, the size of container will be frames in flight value.


Now let's move onto code:
```
  XntEngineECS::RenderContext &render_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntEngineECS::RenderContext>(registry);
  
  ...

  uint32_t max_frames_in_flight = 3;
```
Timeline semaphore:
```
  // --- Synchronization
  XntDebug::Log("------ Synchronization ------");

  // -- Timeline semaphore
  timeline_semaphore = XntGraphics::CreateTimelineSemaphore(device, max_frames_in_flight, log_verbosity);

  if(timeline_semaphore == VK_NULL_HANDLE)
  {
    DestroyEngine(registry);
    return EXIT_FAILURE;
  }
```

Binary semaphores creation:

Note: Binary semaphores mean that they have on and off signals, not that there's two of them.
```
  // -- Binary semaphores
  XntGraphics::InitializeImageAcquiredSemaphoresContainer(device, max_frames_in_flight,
                                                          image_acquired_semaphores, log_verbosity);

  XntGraphics::InitializeRenderFinishedSemaphoresContainer(device, max_frames_in_flight, 
                                                           render_finished_semaphores, log_verbosity);

```
And command resources:
```
  // -- Command resources
  XntGraphics::InitializeCommandResourcesCommandPoolsAndAllocateCommandBuffers(device,
                                                                               max_frames_in_flight,
                                                                               command_resources,
                                                                               graphics_queue_index,
                                                                               log_verbosity);
```


### Rendering loop
Now that we finished initializing variables, we can now start making render loop.

First of all before we even start our render loop pipeline, we first need to synchronize with our timeline semaphore:
```
    XntGraphics::WaitTimelineSemaphore(device, timeline_semaphore, wait_value);
```
we need a wait value, it is a value that we calculate by signal value and max frames in flight difference.

- **But what is a signal value?** - It is essentially a value that GPU will set timeline semaphore counter to upon completing frame.
- **But why is wait value calculated as "signal_value - max_frames_in_flight"** It is used because we want to prevent CPU(or faster GPU queue) from overwriting frame resources that are still actively being processed by older frame.

So, our render loop looks like this:
```
    // ----- Update
    ...
    
    // ----- Render
    const uint32_t signal_value = next_signal_value;
    const uint64_t wait_value = signal_value - max_frames_in_flight;

    XntGraphics::WaitTimelineSemaphore(device, timeline_semaphore, wait_value);
    
    // after render loop
    next_signal_value++;
```
But as you can notice if we have signal value as 0 from start, we would have our wait value as (0 - 3) = -3, which is wrong(because wait_value is a unsigned integer 64, this underflows to 2^64 - 3), so, we have to give our next_signal_value a starting value as:
```
  // --- Synchronization
  ....
  
  // -- Timeline semaphore
  ...

  // -- Binary semaphores
  ...

  // -- Command resources
  ...
  
  next_signal_value = max_frames_in_flight + 1;
```

Our first step would be to reset command pools and to acquire images, though for accessing command resources and other frame resources, we need an index that goes from 0 to 1 to 2 and again 0 and such on(with 3 max frames in flight). We can calculate it that way
```
    const uint32_t frame_resource_index = frame_index % max_frames_in_flight;
```
And, after loop:
```
    frame_index++;
```
So, we get:
```
    // ----- Render
    const uint32_t frame_resource_index = frame_index % max_frames_in_flight;
    const uint32_t signal_value = next_signal_value;
    const uint64_t wait_value = signal_value - max_frames_in_flight;

    XntGraphics::WaitTimelineSemaphore(device, timeline_semaphore, wait_value);
    
    // after render loop
    frame_index++;
    next_signal_value++;
```

Now let's clear command pools before acquiring and starting recording command buffer, so every time in render loop we start with clear command buffers.
```
    XntGraphics::WaitTimelineSemaphore(...);


    XntGraphics::ResetCommandPool(device, command_resources[frame_resource_index].command_pool);
```
Note: I will use aliases for less tedious write(e.g. current_command_resources instead of command_resources[frame_resource_index])
```
    XntGraphics::Semaphore current_image_acquired_semaphore   = image_acquired_semaphores[frame_resource_index];
    XntGraphics::Semaphore current_render_finished_semaphore  = render_finished_semaphores[frame_resource_index];
    XntGraphics::CommandResources &current_command_resources  = command_resources[frame_resource_index];
    
    XntGraphics::ResetCommandPool(device, current_command_resources.command_pool);
```

And let's continue by acquiring image:
```
    XntGraphics::SwapchainAcquisitionResult acquisition = XntGraphics::AcquireNextImageFromSwapchain(
        device, swapchain, current_image_acquired_semaphore 
    );

    const XntGraphics::StatusResult acquire_result = acquisition.result;
    const uint32_t current_image_index = acquisition.image_index;
    
    // ---- Handle swapchain recreation checks
    - We will do this in future!
```
Don't worry about acquire_result yet, it's needed for window resizing and we will do that later.

Now we can start recording commands to our command buffer. Before we start drawing, we first need to transition both of our swapchain and depth image layouts to drawing optimal, we can do so with pipeline barriers, done like this:
```
    // ---- Start command buffer record
    XntGraphics::BeginCommandBuffer(current_command_resources.command_buffer);

    XntGraphics::ImageMemoryBarrier swapchain_image_memory_barrier_render = XntGraphics::CreateSwapchainImageMemoryBarrierForRender(
        swapchain_images[current_image_index]
    );

    XntGraphics::ImageMemoryBarrier depth_image_memory_barrier_render = XntGraphics::CreateDepthImageMemoryBarrierForRender(
        depth_texture.handle
    );
```
```
    std::array<XntGraphics::ImageMemoryBarrier, 2> render_image_memory_barriers = {
      swapchain_image_memory_barrier_render,
      depth_image_memory_barrier_render
    };

    XntGraphics::PipelineBarrier(current_command_resources.command_buffer, 
                                 render_image_memory_barriers.size(),
                                 render_image_memory_barriers.data());
```

As you can see to start rendering, we need rendering config:
```
XntGraphics::BeginRendering(current_command_buffer, rendering_config);
```

```
    XntGraphics::RenderingConfig rendering_config = {
      .render_area_width = 
      .render_area_height = 

      .color_attachment_config_data = 
      .color_attachment_count = 

      .depth_attachment_config = 
    };
```

- **Color attachment configuration** a config that defines our target image view, background clear color value(RGBA), loading and storing operator(How we want to load or store our image to render, for example if we want to clear pixels before drawing or preserve rendered pixels for presentation)
- **Depth attachment configuration** defines our depth texture view, initial depth clear value(usually 1.0), load and store operations.


We won't go into depth about those concepts for now, we will create structs that have most parameters preconfigured.

```
    // ---- Render config
    XntMath::Vec4 clear_color = { 0.0f, 0.3f, 0.0f, 1.0f };
    XntGraphics::ColorAttachmentConfig color_attachment_config = XntGraphics::CreateEngineDefaultColorAttachmentConfig(
        swapchain_image_views[current_image_index],
        clear_color
    );

    XntGraphics::DepthAttachmentConfig depth_attachment_config = XntGraphics::CreateEngineDefaultDepthAttachmentConfig(
        depth_texture.view
    );
```
```
    XntGraphics::RenderingConfig rendering_config = {
      .render_area_width   = swapchain_width,
      .render_area_height  = swapchain_height,

      .color_attachment_config_data = &color_attachment_config,
      .color_attachment_count = 1,

      .depth_attachment_config = depth_attachment_config
    };
```

Now we can start rendering:
```
    XntGraphics::BeginRendering(current_command_resources.command_buffer, rendering_config);
    {
      ...
    }
    XntGraphics::EndRendering(current_command_resources.command_buffer);
```
First thing we need to do after beginning rendering is to set viewport and scissors, and bind pipeline to our command buffer.

- **Viewport** defines which part of the screen we want to render. For example, if we want to draw only half of screen and reserve the other half for other pipeline to draw on (like in 2 player screen mode)
- **Scissors** define where our drawing should cut off, so for example random particle can't spill over. For example, if we have 2 player screen mode, we don't want random particle that belongs to 1st player screen side to flicker on 2nd player screen side.

In our example, we will use just single full screen available
```
      XntGraphics::SetViewportAndScissor(current_command_resources.command_buffer, 
                                         swapchain_width, swapchain_height);
```

As for binding pipeline, we need to bind pipeline to command buffer at specific point, whether our pipeline is graphics or compute.
```
      XntGraphics::BindPipeline(current_command_resources.command_buffer, graphics_pipeline,
                                XntGraphics::PipelineBindPoints::GRAPHICS);
```

Later on, we will create triangle and display it, but for now let's get atleast some visual feedback as just clear color background displayed:

```
    XntGraphics::BeginRendering(current_command_resources.command_buffer, rendering_config);
    {
      XntGraphics::SetViewportAndScissor(current_command_resources.command_buffer, 
                                         swapchain_width, swapchain_height);

      XntGraphics::BindPipeline(current_command_resources.command_buffer, graphics_pipeline,
                                XntGraphics::PipelineBindPoints::GRAPHICS);

      // -- render objects
    }
    XntGraphics::EndRendering(current_command_resources.command_buffer);

```
Now after we finished rendering, we need to submit the command buffer and finally present the image we rendered. Although first of all before ending command buffer, we need to transition our swapchain image layout to presentation optimal:
```
    }
    XntGraphics::EndRendering(current_command_resources.command_buffer);

    XntGraphics::ImageMemoryBarrier swapchain_image_memory_barrier_present = XntGraphics::CreateSwapchainImageMemoryBarrierForPresentation(
        swapchain_images[current_image_index]
    );
    XntGraphics::PipelineBarrier(current_command_resources.command_buffer, 
                                 1,
                                 &swapchain_image_memory_barrier_present);

    XntGraphics::EndCommandBuffer(current_command_resources.command_buffer);

```
Note: We don't need to transition depth image layout to presentation optimal, since we usually need depth image for calculating depth during rasterization phase and not for displaying it.

As you can see engine provides 2 frame submit functions
```
    XntGraphics::SubmitFrameSingleCommandBuffer(...)
```
and
```
    XntGraphics::SubmitFrameMultipleCommandBuffers(...)
```

SubmitFrameMultipleCommandBuffers() usually needed when we have multi threaded command recording, where each command buffer takes its own chunk of recording and then during submitting phase they are sent all at once, this is highly performant(since utilizes several or more cores) but advanced. We will use one simple monolithic command buffer for sake of simplicity.

```
    // ---- Frame submit
    XntGraphics::SubmitFrameSingleCommandBuffer(graphics_queue, command_buffer, frame_submit_config);
```
To send command buffers to GPU, we first need a queue itself:
```
  // --- Logical Device
  ...

  // --- Graphics queue
  graphics_queue = XntGraphics::GetDeviceQueue(device, graphics_queue_index);

  // --- Vulkan Memory Allocator
  ...
```
```
    // ---- Frame submit
    XntGraphics::FrameSubmitConfig frame_submit_config = {
      .image_acquired_semaphore = current_image_acquired_semaphore,
      .current_render_finished_semaphore = current_render_finished_semaphore,
      .timeline_semaphore = timeline_semaphore,
      .signal_value = signal_value
    };
    XntGraphics::SubmitFrameSingleCommandBuffer(graphics_queue, current_command_resources.command_buffer, frame_submit_config);
```

And now we can finally present the frame we rendered:
```
    // ---- Frame Present
    XntGraphics::FramePresentConfig frame_present_config = {
      .swapchain = swapchain,
      .swapchain_image_index = current_image_index,
      .current_render_finished_semaphore = current_render_finished_semaphore
    };
    XntGraphics::PresentFrame(graphics_queue, frame_present_config);

```
Important note!!!

It's not over yet, after we quit our application we also need to wait for queue, because Vulkan is still asynchronously completing the commands:
```
  // ----- End
  XntGraphics::QueueWaitIdle(graphics_queue);
  
  DestroyEngine(registry);
```
And just like that, we finally have a background clear color display, you just successfully built an entire Vulkan render pipeline from scratch! Even though Xenith uses less code for Vulkan render pipeline setup, it saves nearly entire Vulkan setup logic.


## Part 3, Drawing objects

### Drawing triangle
Now that we finished setup, lets draw at least triangle. Vulkan uses coordination system (-X left, +X right), (-Y top, +Y down), (-Z points to us, +Z points from us). In order to display the triangle, we need to first create it's mesh, fill vertices and indices vector containers and upload to VRAM.

You can try for yourself first visualizing the triangle in paint software/tool you are using and then try and fill "```std::vector<Vertex> vertices```" and "```std::vector<uint_32t> indices```"

```
  // ---- Main game loop
  ...

  // --- Models
  XntDebug::Log("------ Models ------");

  std::vector<XntGraphics::Vertex3D> triangle_vertices = {
    //      position       normals       color         uv, texture_id
    { { -0.5f,  0.5f, 0.0f }, {}, {255, 0,   0,   255}, {}, {} },
    { {  0.0f, -0.5f, 0.0f }, {}, {0,   255, 0,   255}, {}, {} },
    { {  0.5f,  0.5f, 0.0f }, {}, {0,   0,   255, 255}, {}, {}}
  };
```
As for now, ignore every parameter except for position and color
```
  std::vector<uint32_t> triangle_indices = {
    0, 1, 2
  };
  uint32_t triangle_indices_size = static_cast<uint32_t>(triangle_indices.size());
```
Note: We indices count, because in future when we will draw indexed we need to know exactly how much indices we have in our mesh.

Now we can create mesh struct:
```
  XntGraphics::Mesh triangle_mesh = {
    .cpu_vertices = triangle_vertices,
    .cpu_indices = triangle_indices,
    .index_count = triangle_indices_size
  };
```
But before we can draw, we have to send our mesh to VRAM, so our GPU can draw this mesh.

To do so, we need ```UploadMeshBuffersToVRAMStaticSingle()```, this function basically sends our mesh cpu data to VRAM.

This functions creates staging buffer(which is why you need allocator as argument), maps CPU data to staging memory, then we create transfer command buffer(which is why you need a command pool in arguments), start transfer command buffer, copy data from staging buffer to buffer we want to initialize with handle, end transfer command buffer, submit to queue, and wait for queue(for synchronization) and then free buffers(staging and transfer).

The "Static" in the name means, that it's most optimal to upload during initialization phase, in future we will learn how to dynamically upload mesh to VRAM.

The "Single" means that it's most optimal for single mesh upload, although this solution is alright for simple games with small amount of meshes(or data size), it's not viable to use for full projects, since if you have a 100 high poly meshes, it's going to be very slow(Since for every 100 models, not only it causes Vulkan driver overhead because of submitting to queue every mesh, but also in the end of this function there's call for QueueWaitIdle, which can also slow down alot since it's called for every mesh). 
And the best way in this situation is to upload mesh data during initialization phase is to upload batch of meshes. Although for this simple example and tutorial, we will use the simplest way.

Now lets upload the mesh:
```
  XntGraphics::UploadMeshBuffersToVRAMStaticSingle(device, allocator, graphics_queue,
                                                   command_resources[0].command_pool,
                                                   triangle_mesh.cpu_vertices, triangle_mesh.cpu_indices,
                                                   triangle_mesh.buffers,
                                                   true, log_verbosity);
```
Now, we need to draw, in order to draw the triangle, we need to first bind vertex and index buffers to command buffer
```
    XntGraphics::BeginRendering(current_command_resources.command_buffer, rendering_config);
    {
      XntGraphics::SetViewportAndScissor();
      XntGraphics::BindPipeline();
	  
      // -- render objects
      XntGraphics::BindSingleVertexBuffer(current_command_resources.command_buffer, triangle_mesh.buffers.vertex_buffer.handle);
      XntGraphics::BindIndexBuffer(current_command_resources.command_buffer, triangle_mesh.buffers.index_buffer.handle);
```
And now we can draw indexed:
```
      XntGraphics::DrawIndexed(current_command_resources.command_buffer, triangle_mesh.index_count, 1);
```
But before this, if your pipeline layout expects push constant and you don't push one, you would get Vulkan validation layers error. To solve this error, we can simply push dummy push constant:
```
      // -- render objects
      PushConstant dummy_push_constant = {
        .model       = XntMath::Matrix4{1.0f},
        .view        = XntMath::Matrix4{1.0f},
        .projection  = XntMath::Matrix4{1.0f}
      };

      XntGraphics::PushConstants(current_command_resources.command_buffer, pipeline_layout.handle, 
                                 XntGraphics::ShaderTypeFlags::VERTEX, sizeof(dummy_push_constant),
                                 &dummy_push_constant);

      XntGraphics::BindSingleVertexBuffer(current_command_resources.command_buffer, triangle_mesh.buffers.vertex_buffer.handle);

```
Also don't forget to free mesh buffers memory:
```
  // ----- End
  XntGraphics::QueueWaitIdle(graphics_queue);

  XntGraphics::DestroyBuffer(allocator, triangle_mesh.buffers.vertex_buffer);
  XntGraphics::DestroyBuffer(allocator, triangle_mesh.buffers.index_buffer);
  
  DestroyEngine(registry);

  return 0;
}
```

### Keyboard Input handling
Now that our triangle shows up, lets learn how to handle keyboard input.

To handle keyboard input, engine tracks two states: current_states and previous_states.

- **Current keys** keys held down in current frame
- **Previous keys** keys held down frame earlier.

By comparing those two arrays we can check if current keys differ from previous ones. Although in engine it's a hidden state where you present one update function, thus making functions "Xenith::Input::IsKeyPressed" work.
```
XntInput::PresentUpdateKeyboard();
```

Now, we can try to test how our input works:
```
    // ---- Handle events
    XntInput::PresentUpdateKeyboard();

    while(XntCore::PollEvent(&event))
    {
      if(event.type == XntCore::EventTypes::QUIT)
        is_running = false;
    }

    if(XntInput::IsKeyPressed(XntInput::Keys::SPACEBAR))
    {
      XntDebug::Log("Spacebar Hello world!");
    }
```
In your console you should get this log. You also can test IsKeyHeld and IsKeyReleased functions, where IsKeyHeld checks if key is held and IsKeyReleased(opposite to pressed) checks if key was released.

While we aren't really building full gameplay mechanics just yet, but we can make one simple handy feature that let's you quit on key press(Escape in my case)
```
    if(XntInput::IsKeyPressed(XntInput::Keys::ESCAPE))
      is_running = false;
```

### Drawing cube
Now let's draw cube, in order for this we need a real camera view and projection matrices values instead of identity matrices, first we need to create camera struct/component(In this example it's located in include/components/base.h)
```
  struct Camera
  {
    Xenith::Math::Matrix4 projection_matrix{ 1.0f };
    Xenith::Math::Matrix4 view_matrix{ 1.0f };
  };
```
Important note: The reason why camera isn't a struct/component out of box (just like any other components such as Transform) -- is because you never know which camera exactly you want, a first person? Third person? Maybe you want to have rotation inside your camera? About transform, what if instead of Euler vector rotation you want a quaternion one(and the other way around)? In all of these cases you have to fight with engine to get components with right variables you need.

And now let's configure the camera, for perspective projection you need FOV degree, near and far z clipping(distance of camera rendering) and coordination system: "Left handed"(-Y up, +Y down) or "Right handed"(-Y down, +Y up), I personally use right handed, since it's more intuitive that way.

Your view matrix is basically where your camera will look at, I defined start position and start view target

```
  // ---- Main game loop
  ...

  // --- Camera
  float camera_fov = 90.0f;
  XntMath::Vec3 camera_start_position = { 1.0f, 1.0f, 2.0f };
  XntMath::Vec3 camera_start_target = { 0.0f, 0.0f, -1.0f };

  XntMath::Vec3 camera_up = XntMath::Vec3{ 0.0f, 1.0f, 0.0f };

  ecs::Camera camera = {
    .projection_matrix = XntMath::PerspectiveProjection(camera_fov, 
                                                        XntDisplay::GetWindowAspect(window), 
                                                        0.1f, 1000.0f,
                                                        XntMath::CoordinateSystem::RIGHT_HANDED),
    .view_matrix = XntMath::LookAt(camera_start_position, camera_start_target, camera_up)
  };
  
  // --- Models
  ...
```

Now let's create cube, to create cube you can either assemble your own with your primitive assembling way or you can call GenerateBoxMesh function, we will stick to second option since assembling cube from triangles is a tedious and long task, although if you want to learn you can freely try and make your own cube like we did triangle before.

```
  // --- Models
  XntDebug::Log("------ Models ------");

  XntGraphics::Mesh cube_mesh = XntGraphics::GenerateBoxMesh({1.0f, 1.0f, 1.0f}, {100, 2, 2, 255});
  XntGraphics::UploadMeshBuffersToVRAMStaticSingle(device, allocator, graphics_queue,
                                                   command_resources[0].command_pool,
                                                   cube_mesh.cpu_vertices, cube_mesh.cpu_indices,
                                                   cube_mesh.buffers,
                                                   true, log_verbosity);
```
Now our push constant isn't a dummy variable but actual camera projection data

```
      // -- render objects
      PushConstant push_constant = {
        .model       = XntMath::Matrix4{1.0f},
        .view        = camera.view_matrix,
        .projection  = camera.projection_matrix
      };

      XntGraphics::PushConstants(current_command_resources.command_buffer, pipeline_layout.handle, 
                                 XntGraphics::ShaderTypeFlags::VERTEX, sizeof(push_constant),
                                 &push_constant);
```

### Moving, Rotating, Scaling the cube
To move, rotate or scale any 3D object, we have to use model matrix. 

Lets try moving our cube
```
      // -- render objects
      XntMath::Matrix4 cube_model_matrix = { 1.0f };
      XntMath::TranslateReference(cube_model_matrix, XntMath::Vec3{ 1.0f, 1.0f, 1.0f });

      PushConstant push_constant = {
        .model       = cube_model_matrix,
        .view        = camera.view_matrix,
        .projection  = camera.projection_matrix
      };
```

Scaling:
```
      XntMath::ScaleScalarReference(cube_model_matrix, 0.5f);
```

Rotating:
```
      XntMath::RotateReference(cube_model_matrix, XntMath::Vec3{ 1.0f, 0.0f, 0.0f }, 65);
```

Although, be aware that correct way of applying those operations is:
1. Translate
2. Rotate
3. Scale

### Window resizing
Currently our window is not resizable, to fix this we need to get window size data from events and recreate our swapchain and depth texture, if we're resizing

First step is to make window resizing struct,
```
namespace ecs
{
  struct WindowContext
  {
    bool is_resizing = false;
    float resize_timer = 0.0f;
    float resize_time_cooldown;
  };
  
  struct Camera 
  {
    ...
  };
}
```
and create in main loop:
```
  // ---- Main game loop
  
  ...
  
  ecs::WindowContext window_context = {
    .is_resizing = false,
    .resize_timer = 0.0f,
    .resize_time_cooldown = 1.0f
  };
```

Now, in order to check if our window size data is off, we first have to handle this in events:
```
    // ---- Handle events
    ...

    while(XntCore::PollEvent(&event))
    {
      // --- Handle quitting
      ...

      // --- Handle resizing
      if(event.type == XntCore::EventTypes::WINDOW_PIXEL_SIZE_CHANGED)
      {
        window_width   = event.window.width;
        window_height  = event.window.height;

        window_context.is_resizing = true;
      }
    }

``` 
Now, we need to start timer of resize_timer when we are in resizing mode
```
    // ----- Update
    XntTime::UpdateTime();

    float dt = XntTime::CalculateDeltaTimeFloat();
	
    // ---- Handle events
    ...
    
    // ---- Resize timer
    if(window_context.is_resizing)
      window_context.resize_timer += dt;
```
In this part, we have to check for resizing calculate actual width and height, recreate swapchain and recreate depth texture.

And now before starting to acquire image from swapchain, we first need to check if we are resizing window:
```
    // ----- Render
    // ---- Handle window resize
    if(window_width != swapchain_width || window_height != swapchain_height)
    {
      
    }
    
    const uint32_t frame_resource_index = ...
    ...
```
In this part we are going to destroy and create swapchain, so we have to wait for device, since Vulkan driver is asynchronous
```
    if(window_width != swapchain_width || window_height != swapchain_height)
    {
      if(window_context.resize_timer > window_context.resize_time_cooldown)
      {
        XntGraphics::DeviceWaitIdle(device);
      }
    }
```
Before recreating variables, we first have to check if we are not in minimized mode:
```
      if(window_context.resize_timer > window_context.resize_time_cooldown)
      {
        XntGraphics::DeviceWaitIdle(device);

        if(window_width > 0 && window_height > 0)
        {
          
        }
      }
```
```
    if(window_width != swapchain_width || window_height != swapchain_height)
    {
      if(window_context.resize_timer > window_context.resize_time_cooldown)
      {
        XntGraphics::DeviceWaitIdle(graphics_queue);

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
```
And also, if we are not finished we should not proceed to acquire image and render:
```
    // ---- Handle window resize
    if(window_width != swapchain_width || window_height != swapchain_height)
    {
      ...
    }

    if(window_context.is_resizing && window_context.resize_timer < window_context.resize_time_cooldown)
    {
      XntTime::LimitFramerate();
      continue;
    }
```
Also incase if our acquire result is suboptimal, we should recreate aswell:
```
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
```


## Part 4, Going from static cube to playable platformer game

### Camera movement
Since we want a third person camera, we need to move it, to make functions like GetMouseDeltaXY() functional, we first need to handle mouse motion events
```
    // ---- Handle events
    XntInput::PresentUpdateKeyboard();

    XntInput::ZeroMouseDeltas();

    while(XntCore::PollEvent(&event))
    {
      // --- Handle quitting
      ...
      
      // --- Handle resizing
      ...

      // --- Handle mouse input
      // -- Mouse motion
      if(event.type == XntCore::EventTypes::MOUSE_MOTION)
      {
        XntInput::CalculateMouseDeltas(event.mouse_motion.x_relative, event.mouse_motion.y_relative);

        XntInput::UpdateMousePosition(event.mouse_motion.x, event.mouse_motion.y);
      }
    }
```

And now validate, that getting mouse motion information works:
```
    XntDebug::Log("mouse delta x: %f", XntInput::GetMouseDeltaX());
    XntDebug::Log("mouse delta y: %f", XntInput::GetMouseDeltaY());
```
Or
```
    XntDebug::Log("mouse position x: %f", XntInput::GetMousePositionX());
    XntDebug::Log("mouse position y: %f", XntInput::GetMousePositionY());
```

Before doing camera movement calculations lets add some new fields:
```
  struct Camera
  {
    Xenith::Math::Matrix4 projection_matrix{ 1.0f };
    Xenith::Math::Matrix4 view_matrix{ 1.0f };

    Xenith::Math::Vec3 position;

    Xenith::Math::Vec3 forward;
    Xenith::Math::Vec3 right;
    Xenith::Math::Vec3 up;

    float yaw    = 0.0f;
    float pitch  = 0.0f;
  };
```
Now when creating camera:
```
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
```

But before update function, lets first limit framerate, so framerate is constant and smooth!

Setup
```
  // ---- Main game loop
  bool is_running = true;
  XntCore::Event event;

  float fps = 240.0f;
  XntTime::SetupLimitFramerate(fps);
```
Then in update beginning, don't forget UpdateTime function, so framerate limit and getting frametime or delta time can function:
```
    // ----- Update
    XntTime::UpdateTime();
```
And in the end of render we can limit framerate:
```
    // after render loop
    frame_index++;
    next_signal_value++;

    XntTime::LimitFramerate();
    
    XntDebug::Log("Frame time: %f", XntTime::CalculateFrameTimeMS());
    // or
    XntDebug::Log("Fps: %f", 1000.0f / XntTime::CalculateFrameTimeMS());
```

Now in our update function we can calculate the camera movement by:
```
    // ----- Update
    ...
    
    // ---- Resize timer
    ...
    
    // ---- Player
    // --- Camera 1
    XntMath::Vec2 mouse_delta_xy = XntInput::GetMouseDeltaXY();

    camera.yaw    += mouse_delta_xy.x * camera_sensitivity;
    camera.pitch  -= mouse_delta_xy.y * camera_sensitivity;

    camera.forward.x = cosf(camera.pitch) * cosf(camera.yaw);
    camera.forward.y = sinf(camera.pitch);
    camera.forward.z = cosf(camera.pitch) * sinf(camera.yaw);
    camera.forward = XntMath::NormalizeReturn(camera.forward);

    XntMath::Vec3 camera_target = camera.forward + camera.position;
    camera.view_matrix = XntMath::LookAt(camera.position, camera_target, camera.up);
```
And just like that we have simple first person view! However in our game, we need a third person camera:
```
    float follow_distance = 2.0f;
    XntMath::Vec3 camera_target = { 0.0f, 0.0f, 0.0f }; // Assume that this is player's position.
    camera.position = camera_target - (camera.forward * follow_distance);

    camera.view_matrix = XntMath::LookAt(camera.position, camera_target, camera.up);
```

And just like that we have functioning third person orbit camera. Although if you played around, you could notice that if you move your mouse too up or too down, the movement on y axis completely breaks and flips. We can avoid this by simple value clamping:

```
    camera.yaw    += mouse_delta_xy.x * camera_sensitivity;
    camera.pitch  -= mouse_delta_xy.y * camera_sensitivity;
    camera.pitch  = XntMath::ClampValueReturn(camera.pitch, -1.56f, 1.56f);
```

#### Window focus
You can also make focused option to move camera only when you are focused in window, but also hide cursor:
```
// ---- Main game loop
  bool is_running = true;
  bool is_focused = true;
  XntCore::SetRelativeMouseMode(window, is_focused);  
  ...
```
Update
```
    if(XntInput::IsKeyPressed(XntInput::Keys::ESCAPE))
      ...

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
    
    // Resize timer
    ...
```
And camera movement calculations:
```
    XntMath::Vec2 mouse_delta_xy = XntInput::GetMouseDeltaXY();
    if(!is_focused)
      mouse_delta_xy = { 0.0f, 0.0f };
```

### Drawing multiple objects
Now lets try to draw several cubes, player cube and second cube, but first of all lets create transform component, so both player and second cube entities can have position and scale

```
namespace ecs
{
  struct Transform
  {
    Xenith::Math::Vec3 position;
    Xenith::Math::Vec3 scale;
  };

  ...
}
```
And now we need to create our first entity, to do so we have to call function ecsxn::CreateEntityInRegistry()
```
  // -- Player cube
  ecsxn::Entity player_entity = ecsxn::CreateEntityInRegistry(registry);
```
Now we can emplace players component we made to registry

```
  ecs::Transform player_transform = {
    .position  = { 1.0f, -2.0f, -2.0f },
    .scale    = { 2.0f, 1.0f, 1.0f }
  };

  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, player_entity, player_transform);
```
Same for second cube:
```
  // -- Second cube
  ecsxn::Entity second_cube_entity = ecsxn::CreateEntityInRegistry(registry);

  ecs::Transform second_cube_transform = {
    .position  = { 2.0f, 1.0f, 0.0f },
    .scale    = { 1.0f, 0.5f, 1.0f }
  };

  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(registry, second_cube_entity, second_cube_transform);
```

And now, in our render loop we need to query for entities that have component we need(transform in our case)
```
      XntGraphics::SetViewportAndScissor(...);
      XntGraphics::BindPipeline(...);

      // -- render objects
      auto models_view = ecsxn::CreateFilteredEntityViewFromRegistryWithoutExclusion<ecs::Transform>(registry);
```
Model view is just a container of entities, we need to get entities components from registry:

```
      for(auto model_entity : models_view)
      {
        auto &model_transform = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, model_entity);
```

Just like that, we can easily translate both of objects
```
        XntMath::TranslateReference(model_matrix, model_transform.position);
```
This way it's really simple to add new object.

```
      // -- render objects
      auto models_view = ecsxn::CreateFilteredEntityViewFromRegistryWithoutExclusion<ecs::Transform>(registry);

      for(auto model_entity : models_view)
      {
        auto &model_transform = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, model_entity);

        XntMath::Matrix4 model_matrix{ 1.0f };

        XntMath::TranslateReference(model_matrix, model_transform.position);
        XntMath::ScaleVectorReference(model_matrix, model_transform.scale);

        ...
      }
```

Very important note: Currently what are we doing is we iterate over every entity in the view and draw one by one, but this is bad for perfomance, since you call DrawIndexed for absolutely every entity, imagine if you had thousand entities you need to render, this would cause CPU Vulkan driver overhead because you're constantly pushing, binding and drawing entity one by one. In future we will learn about Multi Draw Indirect, a technique that would help us draw everything in just large buffers instead of drawing everything one by one, draw group of objects for single call. Although it's not tragic at all for simple games, and pretty much fine for our cubic platformer example.


### Introduction to physics
Now that we have orbit camera and we can draw player cube, we need player movement, in order for this we need to create rigidbody for player's cube, but first lets create physics system and context itself

For physics system we need to create layer matrix which defines how we want our objects to collide.

First lets create our first layers:
```
...

// ----- Physics
namespace Layers
{
  constexpr XntPhysics::ObjectLayer NON_MOVING  = 0;
  constexpr XntPhysics::ObjectLayer MOVING      = 1;
  constexpr XntPhysics::ObjectLayer PLAYER      = 2;
}

...

int main()
{
  ...
}

```

Now we can create local function for layer matrix creation
```
XntPhysics::LayerMatrix CreateLayerMatrix()
{
  XntPhysics::LayerMatrix layer_matrix;
}
```


Collision is pretty expensive if every object tests every shape, so in physics engines it is usually split in two phases:
- **Broadphase** -- a fast bounding box search that determines if objects should even collide(e.g. if they are far away or both static)
- **Narrowphase** -- detailed, exact geometric intersection test performed only on objects that survived broadphase.

First we need to map objects to broadphase, whether our object is static in broadphase or dynamic:


```
  // Map objects layers to broadphase layers
  layer_matrix.object_to_broadphase[Layers::NON_MOVING]  = XntPhysics::BroadPhase::STATIC;
  layer_matrix.object_to_broadphase[Layers::MOVING]      = XntPhysics::BroadPhase::DYNAMIC;
  layer_matrix.object_to_broadphase[Layers::PLAYER]      = XntPhysics::BroadPhase::DYNAMIC;
```

Now we need to assign how broadphase layers should interact with each other, static broadphase should interact only with dynamic layer, since it's pretty much useless to check for static broadphase layer.
```
  layer_matrix.broadphase_masks[XntPhysics::BroadPhase::STATIC]   = (1 << XntPhysics::BroadPhase::DYNAMIC);

  layer_matrix.broadphase_masks[XntPhysics::BroadPhase::DYNAMIC]  = (1 << XntPhysics::BroadPhase::STATIC |
                                                                     1 << XntPhysics::BroadPhase::DYNAMIC);
```

Now, for collision masks we determine which layers should interact with each other, in this example:
```
  // Object collision masks
  layer_matrix.collision_masks[Layers::NON_MOVING]  = (1ULL << Layers::MOVING) |
                                                      (1ULL << Layers::PLAYER);

  layer_matrix.collision_masks[Layers::MOVING]      = (1ULL << Layers::NON_MOVING) |
                                                      (1ULL << Layers::MOVING) |
                                                      (1ULL << Layers::PLAYER);

  layer_matrix.collision_masks[Layers::PLAYER]      = (1ULL << Layers::NON_MOVING) |
                                                      (1ULL << Layers::MOVING);

  return layer_matrix;
```
Note: in this example player doesn't collide with itself, because we are making single player game.

Now, we can create physics context:
```
    // ---- Contexts
    XntEngineECS::RenderContext &render_context = ...

    auto layer_matrix = CreateLayerMatrix();
    XntPhysics::PhysicsContext &physics_context = ecsxn::EmplaceContextComponentToRegistryAndGetComponentData<XntPhysics::PhysicsContext>(registry, layer_matrix);
```

And initialize physics system, you can set your own parameters in physics system initialization config, but for simplicity we will use default one
```
  ...
  
  // ---- Physics system
  XntPhysics::PhysicsSystemInitializationConfig physics_system_init_config = XntPhysics::LoadEngineDefaultPhysicsSystemInitializationConfig();
  XntPhysics::InitializePhysicsSystem(physics_context, physics_system_init_config);


  // ---- Main game loop
  ...
```

Now, to create player body, we first need to create shape,
```
  // -- Player cube
  ...
  ecs::Transform player_transform = ...
  
  XntPhysics::BoxShapeSettings player_box_shape_settings = {
    .extent = player_transform.scale
  };
  XntPhysics::ShapeReference player_box_shape = XntPhysics::CreateBoxShapeReference(player_box_shape_settings);
```
And to create rigidbody, we also need a body interface, it is a system that manages bodies, with this system we can get or set rigidbodies positions, velocities, rotations, and so on. 
```
  XntPhysics::PhysicsSystem &physics_system = physics_context.physics_system;
  XntPhysics::BodyInterface &body_interface = XntPhysics::GetBodyInterfaceFromPhysicsSystem(physics_system);
```

```
  XntPhysics::RigidbodySettings player_box_rigidbody_settings = {
    .shape_reference = player_box_shape,
    .position = player_transform.position,
    .rotation_quaternion = XntMath::QuaternionIdentity(),
    .motion_type = XntPhysics::MotionTypes::DYNAMIC,
    .object_layer = Layers::PLAYER,
    .mass = 10.0f,
    .allowed_degrees_of_freedom_flags = XntPhysics::AllowedDegreesOfFreedoms::TRANSLATION_XYZ
  };
  XntPhysics::Rigidbody player_rigidbody = XntPhysics::CreateRigidbody(body_interface, player_box_rigidbody_settings,
                                                                       player_entity, true,
                                                                       XntPhysics::ActivationModes::ACTIVATE); 
```
Allowed degrees of freedom is very helpful field when we need our body to move into one way, whether we want it to only rotate, or only move, in my case I don't want the cube to rotate.

Dont forget to emplace component
```
  ecsxn::EmplaceEntityComponentToRegistry<ecs::Transform>(...);
  ecsxn::EmplaceEntityComponentToRegistry<XntPhysics::Rigidbody>(registry, player_entity, player_rigidbody);
```

And now update the physics system:
```
    // ----- Update
    XntTime::UpdateTime();

    float dt = XntTime::CalculateDeltaTimeFloat();

    XntPhysics::UpdatePhysicsSystemStep(physics_context, dt);
    
    ...
```
Although if you could see, the cube didn't move. That is because physics transform calculations are done in separate, meaning we have to synchronize physics with render transform. But first lets validate if physics even works:
```
    auto player_position = XntPhysics::GetRigidbodyPositionFromBodyInterface(body_interface, player_rigidbody.body_id);
    XntDebug::Log("player y: %f", player_position.y);
```
You should see that y coordinate is decreasing, this is because when dynamic body added, it automatically gets gravity acceleration.
```
    // ---- Player
    auto &player_transform_got = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, player_entity);
    auto &player_rigidbody_id = ecsxn::GetEntityComponentFromRegistry<XntPhysics::Rigidbody>(
        registry, player_entity
    ).body_id;

    // --- Camera 1
    camera.forward.x = ...
    camera.forward.y = ...
    camera.forward.z = ...
    camera.forward = ...
    ...
```
Now you can see that cube is falling downwards. Now lets make floor so our player doesn't fall into void:
```
  // -- Player
  ...
  
  // -- Floor 
  ecsxn::Entity floor_entity = ecsxn::CreateEntityInRegistry(registry);

  ecs::Transform floor_transform = {
    .position  = { 0.0f, -6.0f, 0.0f },
    .scale     = { 12.0f, 1.0f, 13.0f }
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

```
Although currently everything looks in same color, lets make different meshes with their own color.
Note: This isn't a great choice for memory to create every mesh because of color difference, but for simple cube platformer this is fine.

```
  const std::array<uint8_t, 4> CYAN   = { 0,   255, 255, 255 };
  const std::array<uint8_t, 4> PINK   = { 255, 105, 180, 255 };
  const std::array<uint8_t, 4> RED    = { 255, 20,  20,  255 };
  const std::array<uint8_t, 4> GREEN  = { 50,  250, 50,  255 };

  std::vector<XntGraphics::Mesh> box_meshes;
```
```
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

  size_t cyan_index   = 0;
  size_t pink_index   = 1;
  size_t red_index    = 2;
  size_t green_index  = 3;
```
Now we can assign new component for both player and floor
```
  ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, player_entity, &box_meshes[red_index]);

  ...
  
  ecsxn::EmplaceEntityComponentToRegistry<XntGraphics::Mesh*>(registry, floor_entity, &box_meshes[green_index]);

```
And in render loop:
```
      auto models_view = ecsxn::CreateFilteredEntityViewFromRegistryWithoutExclusion<ecs::Transform, XntGraphics::Mesh*>(registry);

      for(auto model_entity : models_view)
      {
        auto &model_transform = ecsxn::GetEntityComponentFromRegistry<ecs::Transform>(registry, model_entity);
        auto &model_mesh = ecsxn::GetEntityComponentFromRegistry<XntGraphics::Mesh*>(registry, model_entity);
        
        ...
```
And don't forget about binding:
```
        XntGraphics::BindSingleVertexBuffer(current_command_resources.command_buffer, model_mesh->buffers.vertex_buffer.handle);
        XntGraphics::BindIndexBuffer(current_command_resources.command_buffer, model_mesh->buffers.index_buffer.handle);
```

Now our cube should fall down on floor, though the camera doesn't follow player, to fix this we have to change one line:
```
    XntMath::Vec3 camera_target = player_transform_got.position;
```

And don't forget to destroy mesh buffers that we allocated
```
  // ----- End
  XntGraphics::DeviceWaitIdle(device);

  for(auto &mesh : box_meshes)
  {
    XntGraphics::DestroyBuffer(allocator, mesh.buffers.vertex_buffer);
    XntGraphics::DestroyBuffer(allocator, mesh.buffers.index_buffer);
  }
  
  ...
```

### Player movement
Now that we implemented 2 rigidbodies, lets make player able to move and jump around

#### Horizontal movement
To make player move we need to first read input:
```
    // --- Camera 1
    ...
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
```
Now, to physics part:
```
  // ----- Initialization
  ...
  
  // -- Player cube
  ...
  
  // - Movement
  float player_cube_speed = 4.0f;
```


```
    // --- Movement
    // -- Input
    ...

    // -- Physics
    XntMath::Vec3 player_current_velocity = XntPhysics::GetRigidbodyLinearVelocityFromBodyInterface(body_interface, player_rigidbody_id);

    float player_new_velocity_y = player_current_velocity.y;

    // Apply player velocity
    XntMath::Vec3 player_cube_velocity_vector = {
      desired_move_direction.x * player_cube_speed,
      player_new_velocity_y,
      desired_move_direction.z * player_cube_speed
    };

    XntPhysics::SetRigidbodyLinearVelocityInBodyInterface(body_interface, player_rigidbody_id, 
                                                          player_cube_velocity_vector);
```


#### Jumping
Now that player can move horizontally, now we need to handle jumping, in initialization:
```
  // - Movement
  float player_cube_speed = ...
  float player_cube_jump_velocity = 4.0f;
```

And to jump:
```
    float player_new_velocity_y = player_current_velocity.y;

    if(XntInput::IsKeyHeld(XntInput::Keys::SPACEBAR))
    {
      player_new_velocity_y = player_cube_jump_velocity;
    }
```
Although, we have a problem where player can jump infinitely, to fix this we need to check if player cube is grounded, we can do that with simple raycast check:
```
  // - Movement
  float player_cube_speed = ...
  float player_cube_jump_velocity = ...

  bool is_grounded = false;
```
and in update:
```
    // -- Physics
    XntMath::Vec3 player_current_position = XntPhysics::GetRigidbodyPositionFromBodyInterface(body_interface, player_rigidbody_id);
    XntMath::Vec3 player_current_velocity = XntPhysics::GetRigidbodyLinearVelocityFromBodyInterface(body_interface, player_rigidbody_id);

    float player_new_velocity_y = player_current_velocity.y;


    // Grounded check
    float cube_half_height = player_transform_got.scale.y * 0.5f;
    XntMath::Vec3 cube_ray_origin = player_current_position - XntMath::Vec3{ 0.0f, cube_half_height, 0.0f };
    XntMath::Vec3 cube_ray_direction = { 0.0f, -0.025f, 0.0f };

    is_grounded = XntPhysics::CastRayNarrowPhaseQueryWithObjectLayerFilter(physics_system, 
                                                                           cube_ray_origin, cube_ray_direction,
                                                                           (1ULL << Layers::NON_MOVING) | 
                                                                           (1ULL << Layers::MOVING));
    
    // Jumping
    if(Xenith::Input::IsKeyHeld(XntInput::Keys::SPACEBAR) && is_grounded)
    {
      player_new_velocity_y = player_jump_velocity;
      is_grounded = false;
    }

  // Apply player velocity
  ...
```

And just like that we have made cube platformer game! You can look into full code for coyote jumping and a small map with obstacles, blocks and dynamic dummy cube.
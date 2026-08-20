Xenith is a lightweight, data-oriented 3D game framework designed for game developers that value control and performance.

## Motivation
Currently there's no such framework that provides graphics low level control, such as explicit VRAM allocation and custom Vulkan pipeline, while at the same time giving essential features for creating games, such as physics, model loading, input handling, ECS all out of box. 

## Core architecture & design
Xenith is designed as thin wrapper. It eliminates just enough repetitive code, such as making own image data loader, model loader, creating textures, linking and trying to make them work together, while leaving you with rest, such as making own Vulkan pipeline, structuring game loop.

- **Explicit ownership** Xenith gives you full access to handles such as VkInstance, VkDevice and Jolt physics interfaces.
- **Data-oriented** Strictly data-oriented both in syntax and actual code.
- **Determinstic** Most functions are determinstic and precise with explaining their task.
- **Modern Vulkan** Xenith encourages you to use and learn modern Vulkan(1.3) features without hiding API driver behind opaque abstraction.
- **Modular architecture** gives you freedom to take or remove what you don't need or want.

## Requirements for using and linking
For system dependencies, build instructions and CMake setup, check REQUIREMENTS_AND_LINKING.md


## Third-Party Libraries license

* **EnTT** MIT
* **fastgltf** MIT, "include/third_party_libraries/fastgltf/LICENSE.md"
* **glm** The Happy Bunny License (Modified MIT License), "include/third_party_libraries/manual.md"
* **Jolt** MIT
* **SDL3** Zlib
* **stb_image** ALTERNATIVE A - MIT License, ALTERNATIVE B - Public Domain (www.unlicense.org)
* **VulkanMemoryAllocator(vma)** MIT

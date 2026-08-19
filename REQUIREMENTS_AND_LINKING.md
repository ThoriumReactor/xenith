# REQUIREMENTS

## Linux

### Debian/Ubuntu based:

```bash
sudo apt install build-essential cmake ccache pkg-config libvulkan-dev vulkan-validationlayers-dev slang
```


X11
```bash
sudo apt install libx11-dev libxrandr-dev libxkbcommon-dev
```
		
Wayland
```bash
sudo apt install libwayland-dev
```

# LINKING 

Linking example:
```cmake
cmake_minimum_required(VERSION 3.22)

project("Your_Project_Name" VERSION 1.2.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# - Including Xenith
add_subdirectory(${PROJECT_SOURCE_DIR}/Your_External_Libraries/xenith)


# - Your Executable
file(GLOB_RECURSE GAME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(main ${GAME_SOURCES})

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# - Linking Xenith
target_link_libraries(main PRIVATE
  xenith_engine
)
```
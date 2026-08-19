cmake_minimum_required(VERSION 3.22)

project(Xenith VERSION 0.01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# ------------------------------------------------
# CCACHE
# - Makes recompiling faster by caching previous compilations
# ------------------------------------------------

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()



# ------------------------------------------------
# VULKAN SDK
# ------------------------------------------------
find_package(Vulkan QUIET)

if(Vulkan_FOUND)
  message(STATUS "Found Vulkan SDK version: ${Vulkan_VERSION}")
else()
  message(FATAL_ERROR "Vulkan SDK not found on your system!\n")
endif()



# ------------------------------------------------
# DEPENDENCIES (SDL3 and fastgltf)
# ------------------------------------------------
add_subdirectory(${PROJECT_SOURCE_DIR}/include/third_party_libraries/sdl3)

add_subdirectory(${PROJECT_SOURCE_DIR}/include/third_party_libraries/fastgltf)



# ------------------------------------------------
# JOLT PHYSICS
# ------------------------------------------------

file(GLOB_RECURSE JOLT_SOURCES
    ${PROJECT_SOURCE_DIR}/include/third_party_libraries/Jolt/*.cpp
)

add_library(Jolt STATIC ${JOLT_SOURCES})

target_include_directories(Jolt PUBLIC
    ${PROJECT_SOURCE_DIR}/include/third_party_libraries
)


# ------------------------------------------------
# XENITH ENGINE STATIC LIBRARY
# ------------------------------------------------
file(GLOB_RECURSE ENGINE_HEADERS
  "${PROJECT_SOURCE_DIR}/include/xenith/*.h"
)

file(GLOB_RECURSE ENGINE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/xenith/*.cpp"
  "${PROJECT_SOURCE_DIR}/src/third_party_libraries/*.cpp"
)

add_library(xenith_engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

set_target_properties(xenith_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Xenith's include
target_include_directories(xenith_engine PUBLIC
  ${PROJECT_SOURCE_DIR}/include)


# Libraries includes
target_include_directories(xenith_engine SYSTEM PUBLIC
  ${Vulkan_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/vma
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/glm
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/fastgltf
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/entt
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/stbi
)

target_include_directories(xenith_engine PRIVATE
  ${PROJECT_SOURCE_DIR}/include/third_party_libraries/sdl3/include
)

# glm 
target_compile_definitions(xenith_engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)


# Include directiories and linking
target_link_libraries(xenith_engine PUBLIC
  SDL3::SDL3-shared
  Vulkan::Vulkan
  fastgltf
  Jolt
)


# Precompiled header
target_precompile_headers(xenith_engine PRIVATE
    ${PROJECT_SOURCE_DIR}/include/xenith/pch/pch.h
)



# ------------------------------------------------
# BUILD OPTIONS
# ------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(XENITH_IS_TOP_LEVEL ON)
else()
  set(XENITH_IS_TOP_LEVEL OFF)
endif()

option(XENITH_BUILD_EXAMPLES "Build Xenith game examples" ${XENITH_IS_TOP_LEVEL})



# ------------------------------------------------
# GAME EXAMPLE
# ------------------------------------------------
if(XENITH_BUILD_EXAMPLES)
  add_subdirectory(game_examples)
endif()

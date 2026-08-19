project(Cube_Platformer_Example_01 VERSION 0.01)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE EXAMPLE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(cube_platformer ${EXAMPLE_SOURCES})

target_include_directories(cube_platformer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


target_link_libraries(cube_platformer PRIVATE
  xenith_engine
)

cmake_minimum_required(VERSION 3.30)

project(opengl2-compute)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(vendor)

find_package(OpenGL)

add_executable (
  opengl2-compute 
  src/App.cpp
  src/App.h
  src/Shader.cpp
  src/Shader.h
  src/main.cpp
)

target_include_directories(opengl2-compute PRIVATE)
target_link_libraries(opengl2-compute PRIVATE spdlog::spdlog libglew_static OpenGL::GL SDL3::SDL3 $<$<BOOL:${MINGW}>:ws2_32>)


cmake_minimum_required(VERSION 3.5)
project(BallSimulator LANGUAGES CXX)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)

find_package(OpenGL REQUIRED)

add_library(BallSimulator
    src/config.h
    src/vec2.hpp
    src/rectangle.hpp
    src/quadtree.hpp
    src/ball.cpp src/ball.hpp
    src/world.cpp src/world.hpp
    src/simulator.cpp src/simulator.hpp)
set_property(TARGET BallSimulator PROPERTY CXX_STANDARD 14)

add_executable(BallSimulatorGl
    src/renderer.cpp src/renderer.hpp
    src/application.cpp src/application.hpp
    src/ballsimulatorgl.cpp src/ballsimulatorgl.hpp
    src/main.cpp)
set_property(TARGET BallSimulatorGl PROPERTY CXX_STANDARD 14)
target_link_libraries(BallSimulatorGl BallSimulator glfw OpenGL::GL)
target_compile_definitions(BallSimulatorGl PRIVATE $<$<PLATFORM_ID:Darwin>:GL_SILENCE_DEPRECATION>)

add_executable(BallSimulatorCli src/main_cli.cpp)
set_property(TARGET BallSimulatorCli PROPERTY CXX_STANDARD 14)
target_link_libraries(BallSimulatorCli BallSimulator)

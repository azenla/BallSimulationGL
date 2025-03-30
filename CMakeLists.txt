cmake_minimum_required(VERSION 3.10)
project(BallSimulator LANGUAGES CXX)

find_package(SDL3 REQUIRED CONFIG)
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

add_executable(BallSimulatorGl MACOSX_BUNDLE WIN32
    src/renderer.cpp src/renderer.hpp
    src/application.cpp src/application.hpp
    src/ballsimulatorgl.cpp src/ballsimulatorgl.hpp
    src/main.cpp)
set_property(TARGET BallSimulatorGl PROPERTY CXX_STANDARD 20)
target_link_libraries(BallSimulatorGl BallSimulator SDL3::SDL3 OpenGL::GL)
target_compile_definitions(BallSimulatorGl PRIVATE $<$<PLATFORM_ID:Darwin>:GL_SILENCE_DEPRECATION>)

add_executable(BallSimulatorCli src/main_cli.cpp)
set_property(TARGET BallSimulatorCli PROPERTY CXX_STANDARD 20)
target_link_libraries(BallSimulatorCli BallSimulator)

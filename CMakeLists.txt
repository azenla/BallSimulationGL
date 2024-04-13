cmake_minimum_required(VERSION 3.5)
project(BallSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)

find_package(OpenGL REQUIRED)

set(LIB_FILES src/quadtree.h src/simulator.cpp src/simulator.h src/vec2.h)
set(SIM_FILES src/main.cpp ${LIB_FILES} src/config.h)
add_executable(BallSimulator ${SIM_FILES})
add_executable(BallSimulatorCli src/main_cli.cpp ${LIB_FILES})
target_link_libraries(BallSimulator glfw ${OPENGL_LIBRARIES})
target_compile_definitions(BallSimulator PRIVATE $<$<PLATFORM_ID:Darwin>:GL_SILENCE_DEPRECATION>)

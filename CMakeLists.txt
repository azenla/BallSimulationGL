cmake_minimum_required(VERSION 3.5)
project(BallSimulator)

set(CMAKE_CXX_STANDARD 11)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)

find_package(OpenGL REQUIRED)

set(LIB_FILES src/quadtree.h src/Simulator.cpp src/Simulator.h src/vec2.h)
set(SIM_FILES src/main.cpp ${LIB_FILES})
add_executable(BallSimulator ${SIM_FILES})
add_executable(BallSimulatorCli src/main_cli.cpp ${LIB_FILES})
target_link_libraries(BallSimulator glfw ${OPENGL_LIBRARIES})

cmake_minimum_required(VERSION 3.8)
project(BallSimulator)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

set(SOURCE_FILES src/main.cpp src/Simulator.cpp src/Simulator.h src/vec2.h)
add_executable(BallSimulator ${SOURCE_FILES})
target_link_libraries(BallSimulator ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

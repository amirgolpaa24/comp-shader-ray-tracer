cmake_minimum_required(VERSION 3.10)
project(ComputeShaderTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add executable and link libraries
add_executable(ComputeShaderTest src/main.cpp src/glad.c)
target_link_libraries(ComputeShaderTest ${OPENGL_LIBRARIES} glfw GLEW)
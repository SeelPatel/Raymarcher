add_subdirectory(deps)

cmake_minimum_required(VERSION 3.26)
project(Raymarcher)
set(CMAKE_CXX_STANDARD 23)

# Setup raymarcher core library
file(GLOB_RECURSE RAYMARCHER_SOURCE CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE RAYMARCHER_INCLUDE CONFIGURE_DEPENDS "include/*.h")

add_library(core STATIC ${RAYMARCHER_SOURCE} ${RAYMARCHER_INCLUDE})
target_link_libraries(core glfw glm glad imgui)

# Create executable
add_executable(Raymarcher main.cpp)

# Link libraries
# todo create include dir vars in deps. ex GLFW_INCLUDE_DIRECTORIES
include_directories(
        "include"
        "${CMAKE_SOURCE_DIR}/deps/glfw/include"
        "${CMAKE_SOURCE_DIR}/deps/glm/glm"
        "${CMAKE_SOURCE_DIR}/deps/glad/include"
)

target_link_libraries(Raymarcher PUBLIC core)

# Copy assets to binary directory
file(COPY raymarching_shader.glsl DESTINATION ${CMAKE_BINARY_DIR})
file(COPY imgui.ini DESTINATION ${CMAKE_BINARY_DIR})

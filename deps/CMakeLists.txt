add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(glad)

cmake_minimum_required(VERSION 3.2.6)
project(ImGui LANGUAGES CXX)

# Setup imgui library
add_library(imgui STATIC
        "imgui/imgui.h"
        "imgui/imgui.cpp"
        "imgui/imconfig.h"
        "imgui/imgui_demo.cpp"
        "imgui/imgui_draw.cpp"
        "imgui/imgui_internal.h"
        "imgui/imgui_tables.cpp"
        "imgui/imgui_widgets.cpp"
        "imgui/imstb_rectpack.h"
        "imgui/imstb_textedit.h"
        "imgui/imstb_truetype.h"
        "imgui/backends/imgui_impl_glfw.cpp"
        "imgui/backends/imgui_impl_glfw.h"
        "imgui/backends/imgui_impl_opengl3.h"
        "imgui/backends/imgui_impl_opengl3.cpp"
        "imgui/backends/imgui_impl_opengl3_loader.h"
        "imgui/misc/cpp/imgui_stdlib.h"
        "imgui/misc/cpp/imgui_stdlib.cpp"
)

target_include_directories(imgui PUBLIC "imgui" "imgui/backends")
target_link_libraries(imgui glfw)


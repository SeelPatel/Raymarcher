#include <iostream>

#include <compute/compute.h>
#include <engine/scene.h>
#include <engine/image_renderer.h>
#include <editor/viewport.h>
#include <editor/scene_editor.h>
#include <editor/editor_data.h>
#include <editor/imgui_utils.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool input_state_initialized = false;
editor::InputState inputs;

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, const int width, const int height) {
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow *window, const double xpos, const double ypos) {
    if (!input_state_initialized) {
        inputs.prev_mouse_pos = {xpos, ypos};
        inputs.mouse_delta = {0, 0};
        input_state_initialized = true;
        return;
    }

    const glm::vec2 curr_pos(xpos, ypos);
    inputs.mouse_delta = inputs.prev_mouse_pos - curr_pos;
    inputs.prev_mouse_pos = curr_pos;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1600, 900, "Raymarcher", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup inputs and callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    editor::setup_theme();

    // Setup Raymarching shader and rendering
    ImageRenderer renderer(1280, 720);
    compute::ComputeBuffer object_buffer(1024);
    compute::ComputeShader raymarcher{};

    Err err;
    if ((err = renderer.init()) || (err = object_buffer.init()) ||
        (err = raymarcher.init(std::filesystem::path("..\\raymarching_shader.glsl")))) {
        err.print();
        return -1;
    }


    // Setup scene
    Scene scene;

    // Load scene from file.
    if (std::filesystem::exists("test.scene")) {
        Buffer buffer;
        if (!(err = buffer.read_from_file("test.scene"))) {
            if ((err = scene.read_from_buffer(buffer))) {
                scene = Scene();
            }
        }
    }

    // Setup editor
    editor::Viewport viewport;
    editor::SceneEditor scene_editor;

    // Render loop
    float last_frame_time = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        const float current_frame_time = static_cast<float>(glfwGetTime());
        const float delta_time = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // process events and update scene
        inputs.mouse_delta = {0, 0};
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Run raymarcher
        raymarcher.activate();
        scene.setup_raymarcher(raymarcher, object_buffer, renderer);

        constexpr GLuint GROUP_SIZE = 32;
        raymarcher.execute(ceil_divide(renderer.image_width(), GROUP_SIZE),
                           ceil_divide(renderer.image_height(), GROUP_SIZE), 1);

        // Make sure writing to image has finished before rendering
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Update editor.
        editor::EditorData editor_data{window, delta_time, scene, inputs, renderer};
        viewport.update(editor_data);
        scene_editor.update(editor_data);

        // Render ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Save scene
    Buffer buffer;
    scene.write_to_buffer(buffer);
    buffer.write_to_file("test.scene");

    return 0;
}

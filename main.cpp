#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <compute/compute.h>
#include <engine/scene.h>
#include <engine/image_renderer.h>

bool input_state_initialized = false;
struct InputState {
    glm::vec2 prev_mouse_pos;
    glm::vec2 mouse_delta;
} inputs;

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
    // Setup GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Raymarcher", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup inputs and callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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
    scene.root.children.emplace_back(
            Object(Object::ObjectType::Box, {10, 0, 0}, {50, 0.2f, 50}, {1.0f, 1.0f, 1.0f}, {}));

    scene.root.children.emplace_back(
            Object(Object::ObjectType::Box, {20, 3, 20}, {6, 6, 6}, {1.0f, 1.0f, 1.0f}));
    scene.root.children.emplace_back(
            Object(Object::ObjectType::Sphere, {10, -1.5, 0}, {6, 1, 1}, {1.0f, 1.0f, 1.0f}));

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
        scene.process_inputs(window, inputs.mouse_delta, delta_time);

        // Run raymarcher
        raymarcher.activate();
        scene.setup_raymarcher(raymarcher, object_buffer, renderer);

        constexpr GLuint GROUP_SIZE = 32;
        raymarcher.execute(ceil_divide(renderer.image_width(), GROUP_SIZE),
                           ceil_divide(renderer.image_height(), GROUP_SIZE), 1);

        // Make sure writing to image has finished before rendering
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Render scene
        renderer.draw();

        // Swap buffers
        glfwSwapBuffers(window);
    }


    return 0;
}

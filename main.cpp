#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utils/buf.h>

#include <compute/compute.h>
#include <engine/scene.h>
#include <engine/image_renderer.h>

GLFWwindow *window;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1280, 720, "Raymarcher", NULL, NULL);

    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ImageRenderer renderer(1280, 720);
    if (Err err = renderer.init()) {
        err.print();
        return -1;
    }

    compute::ComputeBuffer object_buffer(1024);
    if (Err err = object_buffer.init()) {
        err.print();
        return -1;
    }

    compute::ComputeShader raymarcher{};
    if (Err err = raymarcher.init(std::filesystem::path("..\\raymarching_shader.glsl"))) {
        err.print();
        return -1;
    }

    Scene scene;
    scene.objects.emplace_back(Object(Object::Type::InfiniteSpheres, 5, 0, 0, 6, 6, 6, 1.0f, 0.0f, 1.0f));
    scene.camera.translate({0, 0, -5.0f});

    while (!glfwWindowShouldClose(window)) {
        // Process events and setup next frame
        glfwPollEvents();

        // Clear screen
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        raymarcher.activate();

        scene.setup_compute_shader(raymarcher, object_buffer, renderer);
        object_buffer.transfer_to_gpu();
        raymarcher.bind_buffer(object_buffer, 2);

        constexpr int GROUP_SIZE = 32;
        const auto num_hori_groups = (GLuint) std::ceil(((float) renderer.image_width()) / GROUP_SIZE);
        const auto num_vert_groups = (GLuint) std::ceil(((float) renderer.image_height()) / GROUP_SIZE);
        raymarcher.execute(num_hori_groups, num_vert_groups, 1);

        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        renderer.draw();

        // Swap buffers
        glfwSwapBuffers(window);
    }


    return 0;
}

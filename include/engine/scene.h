#ifndef RAYMARCHER_SCENE_H
#define RAYMARCHER_SCENE_H

#include <engine/object.h>
#include <engine/camera.h>
#include <compute/buffer.h>
#include <compute/compute.h>
#include <engine/image_renderer.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Scene {
    Camera camera;
    Object root{"Root", ObjectType::Empty, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}};

    // Parameters
    float fov = 75.0f;
    float fog_distance = 100;

    glm::vec3 sky_bottom_color = glm::vec3(242, 231, 255) / 255.0f;
    glm::vec3 sky_top_color = glm::vec3(120, 128, 170) / 255.0f;

    float shadow_intensity = 0.0;
    bool visualize_distances = false;

    glm::vec3 light_dir = glm::normalize(glm::vec3(-1, -1, 0));
    glm::vec3 light_pos = {30, 30, 0};
    glm::vec3 light_color = glm::vec3(255, 237, 227) / 255.0f;

    Err setup_raymarcher(compute::ComputeShader &raymarcher, compute::ComputeBuffer &object_buffer,
                         const ImageRenderer &image_renderer) const;

    void process_inputs(GLFWwindow *const window, const glm::vec2 &mouse_delta, float delta_time);

    Err write_to_buffer(Buffer &buffer) const;

    Err read_from_buffer(Buffer &buffer);

private:

};

#endif //RAYMARCHER_SCENE_H

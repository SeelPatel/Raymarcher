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
    std::vector<Object> objects;

    Err setup_raymarcher(compute::ComputeShader &raymarcher, compute::ComputeBuffer &object_buffer,
                         const ImageRenderer &image_renderer) const;

    void process_inputs(GLFWwindow *const window, const glm::vec2 &mouse_delta, float delta_time);

private:

};

#endif //RAYMARCHER_SCENE_H

#ifndef RAYMARCHER_SCENE_H
#define RAYMARCHER_SCENE_H

#include <engine/object.h>
#include <engine/camera.h>
#include <compute/buffer.h>
#include <compute/compute.h>
#include <engine/image_renderer.h>

struct Scene {
    Camera camera;
    std::vector<Object> objects;

    Err setup_compute_shader(compute::ComputeShader &raymarcher, compute::ComputeBuffer &object_buffer,
                             const ImageRenderer &image_renderer) const;
};

#endif //RAYMARCHER_SCENE_H

#include <engine/scene.h>


Err Scene::setup_raymarcher(compute::ComputeShader &raymarcher, compute::ComputeBuffer &object_buffer,
                            const ImageRenderer &image_renderer) const {
    // Fill object buffer
    object_buffer.reset();
    const std::expected<size_t, Err> objects_write_result = root.write_to_compute_buffer(object_buffer);

    if (!objects_write_result) return objects_write_result.error();

    object_buffer.transfer_to_gpu();
    raymarcher.bind_buffer(object_buffer, 1);

    const glm::mat4 view = camera.view_matrix();
    const glm::mat4 proj = glm::perspective(glm::radians(fov), 16.0f / 9.0f, 0.1f, 100.0f);
    const glm::mat4 proj_inverse = glm::inverse(proj);
    const glm::mat4 view_inverse = glm::inverse(view);

    raymarcher.activate();
    raymarcher.bind("view", view_inverse);
    raymarcher.bind("inv_proj", proj_inverse);
    raymarcher.bind("image_width", image_renderer.image_width());
    raymarcher.bind("image_height", image_renderer.image_height());

    raymarcher.bind("num_objects", (GLuint) objects_write_result.value());

    raymarcher.bind("sky_top_color", sky_top_color);
    raymarcher.bind("sky_bottom_color", sky_bottom_color);

    raymarcher.bind("fog_dist", fog_distance);
    raymarcher.bind("shadow_intensity", shadow_intensity);

    raymarcher.bind("visualize_distances", visualize_distances);

    // TEMP: light. use buffer of lights in the future

    raymarcher.bind("light_direction", light_dir);
    raymarcher.bind("light_pos", light_pos);
    raymarcher.bind("light_color", light_color);

    return {};
}

void Scene::process_inputs(GLFWwindow *const window, const glm::vec2 &mouse_delta, float delta_time) {

    glm::vec3 translation(0);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        translation.z += 1;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        translation.z -= 1;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        translation.x -= 1;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        translation.x += 1;

    camera.translate(delta_time * translation);
    camera.rotate(-mouse_delta.x, mouse_delta.y);
}

Err Scene::write_to_buffer(Buffer &buffer) const {
    Err err;
    if ((err = buffer.write(fov, fog_distance, sky_bottom_color, sky_top_color, shadow_intensity,
                            visualize_distances, light_dir, light_pos, light_color)))
        return err;
    if ((err = root.write_to_buffer(buffer))) return err;
    return err;
}

Err Scene::read_from_buffer(Buffer &buffer) {
    Err err;
    if ((err = buffer.read(fov, fog_distance, sky_bottom_color, sky_top_color, shadow_intensity,
                           visualize_distances, light_dir, light_pos, light_color)))
        return err;
    if ((err = root.read_from_buffer(buffer))) return err;
    return err;
}

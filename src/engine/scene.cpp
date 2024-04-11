#include <engine/scene.h>


Err Scene::setup_raymarcher(compute::ComputeShader &raymarcher, compute::ComputeBuffer &object_buffer,
                            const ImageRenderer &image_renderer) const {
    // Fill object buffer
    object_buffer.reset();
    const std::expected<size_t, Err> objects_write_result = root.write_to_buffer(object_buffer);

    if (!objects_write_result) return objects_write_result.error();

    object_buffer.transfer_to_gpu();
    raymarcher.bind_buffer(object_buffer, 1);

    // todo make these configurable
    bool render_shadows = true;
    bool render_lighting = true;
    bool render_links;
    float fov = 75.0f;
    float fog_distance = 100;
    glm::vec3 sky_bottom = glm::vec3(242, 231, 255) / 255.0f;
    glm::vec3 sky_top = glm::vec3(120, 128, 170) / 255.0f;
    float shadow_intensity = 0.7;
    bool visualize_distances = false;

    // Set shader uniforms

    // todo double check the math here
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

    raymarcher.bind("sky_top_color", sky_top);
    raymarcher.bind("sky_bottom_color", sky_bottom);

    raymarcher.bind("fog_dist", fog_distance);
    raymarcher.bind("shadow_intensity", shadow_intensity);

    raymarcher.bind("visualize_distances", visualize_distances);

    // TEMP: light. use buffer of lights in the future
    const glm::vec3 light_dir = glm::normalize(glm::vec3(-1, -1, 0));
    constexpr glm::vec3 light_pos(-25, 25, 0);
    constexpr glm::vec3 light_color = glm::vec3(255, 237, 227) / 255.0f;

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
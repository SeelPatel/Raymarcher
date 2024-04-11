#include <engine/object.h>

std::expected<size_t, Err> Object::write_to_buffer(compute::ComputeBuffer &buf) const {
    return write_to_buffer_impl(buf, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
}

std::expected<size_t, Err> Object::write_to_buffer_impl(compute::ComputeBuffer &buf, const glm::vec3 &parent_pos,
                                                        const glm::vec3 &parent_scale) const {
    Err err;

    const glm::vec3 global_pos = parent_pos + pos;
    const glm::vec3 global_scale = parent_scale * scale;

    const GLuint num_children = children.size();
    if ((err = buf.write(obj_type, global_pos, global_scale, color, link_type, num_children))) return err;

    size_t num_total_objects = 1;
    for (const Object &child: children) {
        std::expected<size_t, Err> child_result = child.write_to_buffer_impl(buf, global_pos, global_scale);
        if (!child_result) return child_result.error();

        num_total_objects += child_result.value();
    }

    return num_total_objects;
}

#include <engine/object.h>

std::expected<size_t, Err> Object::write_to_compute_buffer(compute::ComputeBuffer &buf) const {
    return write_to_compute_buffer_impl(buf, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
}

std::expected<size_t, Err>
Object::write_to_compute_buffer_impl(compute::ComputeBuffer &buf, const glm::vec3 &parent_pos,
                                     const glm::vec3 &parent_scale) const {
    Err err;
    // todo refactor.
    const glm::vec3 global_pos = pos;
    const glm::vec3 global_scale = scale;

    const GLuint num_children = children.size();
    if ((err = buf.write(obj_type, global_pos, global_scale, color, link_type, num_children))) return err;

    size_t num_total_objects = 1;
    for (const Object &child: children) {
        std::expected<size_t, Err> child_result = child.write_to_compute_buffer_impl(buf, global_pos, global_scale);
        if (!child_result) return child_result.error();

        num_total_objects += child_result.value();
    }

    return num_total_objects;
}

Object::Object(const std::string &name, ObjectType objType, const glm::vec3 &pos, const glm::vec3 &scale,
               const glm::vec3 &color) : name(name),
                                         obj_type(
                                                 objType),
                                         pos(pos),
                                         scale(scale),
                                         color(color) {
}


Err Object::write_to_buffer(Buffer &buffer) const {
    Err err;
    if ((err = buffer.write(name, obj_type, pos, scale, color, link_type))) return err;

    const uint16_t num_children = children.size();
    if ((err = buffer.write(num_children))) return err;

    for (const Object &child: children) {
        if ((err = child.write_to_buffer(buffer))) return err;
    }

    return err;
}

Err Object::read_from_buffer(Buffer &buffer) {
    Err err;

    if ((err = buffer.read(name, obj_type, pos, scale, color, link_type))) return err;

    uint16_t num_children;
    if ((err = buffer.read(num_children))) return err;

    children.reserve(num_children);
    for (uint16_t i = 0; i < num_children; ++i) {
        Object object;
        if ((err = object.read_from_buffer(buffer))) return err;

        children.emplace_back(std::move(object));
    }

    return err;
}


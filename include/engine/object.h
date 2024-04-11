#ifndef RAYMARCHER_OBJECT_H
#define RAYMARCHER_OBJECT_H

#include <compute/buffer.h>

#include <glm/glm.hpp>

#include <expected>

struct Object {
    enum class ObjectType : uint32_t {
        Placeholder, Sphere, Box, Torus, InfiniteSpheres
    } obj_type;

    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 color;

    enum class LinkType : uint32_t {
        Default, SoftUnion, Subtraction, Intersection
    } link_type;

    std::vector<Object> children;

    std::expected<size_t, Err> write_to_buffer(compute::ComputeBuffer &buf) const;

private:
    std::expected<size_t, Err>
    write_to_buffer_impl(compute::ComputeBuffer &buf, const glm::vec3 &parent_pos, const glm::vec3 &parent_scale) const;
};

#endif //RAYMARCHER_OBJECT_H

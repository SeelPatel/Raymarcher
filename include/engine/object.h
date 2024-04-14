#ifndef RAYMARCHER_OBJECT_H
#define RAYMARCHER_OBJECT_H

#include <compute/buffer.h>

#include <glm/glm.hpp>

#include <expected>

enum class ObjectType : uint32_t {
    Empty, Sphere, Box, Torus, InfiniteSpheres, RoundBox, Octohedron, HexPrism, GridPlane
};

enum class LinkType : uint32_t {
    Default, SoftUnion, Subtraction, Intersection
};

struct Object {
    Object() = default;

    Object(const std::string &name, ObjectType objType, const glm::vec3 &pos, const glm::vec3 &scale,
           const glm::vec3 &color);

    std::string name;

    // Rest of these are sent to OpenGL
    ObjectType obj_type = ObjectType::Box;
    glm::vec3 pos{};
    glm::vec3 scale{};
    glm::vec3 color{};

    float diffuse = 1.0f;
    float specular = 48.0f;

    LinkType link_type = LinkType::Default;

    std::vector<Object> children;

    std::expected<size_t, Err> write_to_compute_buffer(compute::ComputeBuffer &buf) const;

    Err write_to_buffer(Buffer &buffer) const;

    Err read_from_buffer(Buffer &buffer);

    constexpr uint32_t uuid() const { return id; };

private:
    uint32_t id = rand();

    std::expected<size_t, Err>
    write_to_compute_buffer_impl(compute::ComputeBuffer &buf, const glm::vec3 &parent_pos,
                                 const glm::vec3 &parent_scale) const;
};

#endif //RAYMARCHER_OBJECT_H

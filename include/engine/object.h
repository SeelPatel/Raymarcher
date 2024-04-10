#ifndef RAYMARCHER_OBJECT_H
#define RAYMARCHER_OBJECT_H

#include <glm/glm.hpp>

struct Object {
    enum class Type : uint32_t {
        Sphere, Box, Torus, InfiniteSpheres
    } type;

    float x, y, z;
    float sx, sy, sz;
    float r, g, b;
};

#endif //RAYMARCHER_OBJECT_H

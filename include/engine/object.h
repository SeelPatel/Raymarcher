#ifndef RAYMARCHER_OBJECT_H
#define RAYMARCHER_OBJECT_H

#include <glm/glm.hpp>

struct Object {
    enum class Type : int {
        Sphere, Box, Torus, InfiniteSpheres
    } type;

    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 color;
};

#endif //RAYMARCHER_OBJECT_H

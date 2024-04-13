#ifndef RAYMARCHER_CAMERA_H
#define RAYMARCHER_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr glm::vec3 world_up = glm::vec3(0, 1, 0);

struct Camera {
    glm::vec3 pos{0, 0, 0};
    glm::vec3 front{0, 0, 1};
    glm::vec3 up{0, 1, 0};
    glm::vec3 right{};

    float yaw = 0;
    float pitch = 0;

    float speed = 30;
    float sens = 0.5f;

    Camera() {
        update_vectors();
    }

    void translate(const glm::vec3 &xyz);

    void rotate(float x_offset, float y_offset);

    [[nodiscard]] constexpr glm::mat4x4 view_matrix() const {
        return glm::lookAt(pos, pos + front, up);
    }

private:
    void update_vectors();
};

#endif //RAYMARCHER_CAMERA_H

#include <engine/camera.h>

void Camera::translate(const glm::vec3 &xyz) {
    pos = pos + (xyz.x * right + xyz.y * up + xyz.z * front) * speed;
}

void Camera::rotate(const float x_offset, const float y_offset) {
    yaw += x_offset * sens;
    pitch += y_offset * sens;
    update_vectors();
}

void Camera::update_vectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}

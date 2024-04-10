#ifndef RAYMARCHER_COMPUTE_H
#define RAYMARCHER_COMPUTE_H

#include <utils/err.h>
#include <compute/buffer.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <filesystem>

namespace compute {
    class ComputeShader {
        GLuint shader_id;
        GLuint program_id;

    public:
        Err init(const std::filesystem::path &shader_path);

        Err init(const std::string &code);

        void activate() const;

        [[nodiscard]] bool is_active() const;

        Err bind_buffer(const ComputeBuffer &buf, GLuint index) const;

        Err execute(GLuint nx, GLuint ny, GLuint nz) const;

        Err bind(const std::string_view &id, const GLint value) const;

        Err bind(const std::string_view &id, const GLuint value) const;

        Err bind(const std::string_view &id, const GLfloat value) const;

        Err bind(const std::string_view &id, const GLboolean value) const;

        Err bind(const std::string_view &id, const glm::vec3 &value) const;

        Err bind(const std::string_view &id, const glm::mat4x4 &value) const;

    };
}

#endif //RAYMARCHER_COMPUTE_H

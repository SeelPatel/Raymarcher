#include <compute/compute.h>

#include <array>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>

namespace compute {
    Err ComputeShader::init(const std::filesystem::path &shader_path) {
        std::ifstream file;
        file.open(shader_path);

        if (!file.is_open() || file.fail()) return Err("Failed to open shader file: {}", shader_path.string());

        std::stringstream str_stream;
        str_stream << file.rdbuf();

        if (file.fail()) return Err("Failed to read shader file: {}", shader_path.string());

        return init(str_stream.str());
    }

    Err ComputeShader::init(const std::string &code) {
        // Create shader and link source code
        shader_id = glCreateShader(GL_COMPUTE_SHADER);
        if (!shader_id) return Err("Failed to create compute shader.");

        const char *const code_c_str = code.c_str();
        glShaderSource(shader_id, 1, &code_c_str, nullptr);

        // Compile compute shader
        glCompileShader(shader_id);

        GLint success;
        std::array<char, 1024> error_info{};
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader_id, error_info.size(), nullptr, error_info.data());
            return Err("Error compiling shader source code. \n{}", error_info.data());
        }

        // Create program and link shader
        program_id = glCreateProgram();
        if (!program_id) return Err("Failed to create compute shader program.");

        glAttachShader(program_id, shader_id);
        glLinkProgram(program_id);

        glGetProgramiv(program_id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program_id, error_info.size(), nullptr, error_info.data());
            return Err("Error linking shader program. \n{}", error_info.data());
        }

        return {};
    }

    Err ComputeShader::bind_buffer(const ComputeBuffer &buf, GLuint index) const {
        if (!is_active()) return Err("Attempting to attach buffer to inactive program.");

        buf.bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf.id());
        return {};
    }

    Err ComputeShader::execute(GLuint nx, GLuint ny, GLuint nz) const {
        if (!is_active()) return Err("Attempting execute inactive program.");

        glDispatchCompute(nx, ny, nz);
        return {};
    }

    void ComputeShader::activate() const {
        glUseProgram(program_id);
    }

    bool ComputeShader::is_active() const {
        GLint active_prog = -1;
        glGetIntegerv(GL_CURRENT_PROGRAM, &active_prog);

        return active_prog == program_id;
    }

    Err ComputeShader::bind(const std::string_view &id, const GLint value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniform1i(attr_id, value);
        return {};
    }

    Err ComputeShader::bind(const std::string_view &id, const GLuint value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniform1ui(attr_id, value);
        return {};
    }

    Err ComputeShader::bind(const std::string_view &id, const GLfloat value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniform1f(attr_id, value);
        return {};
    }

    Err ComputeShader::bind(const std::string_view &id, const GLboolean value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniform1i(attr_id, value);
        return {};
    }

    Err ComputeShader::bind(const std::string_view &id, const glm::vec3 &value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniform3f(attr_id, value[0], value[1], value[2]);
        return {};
    }

    Err ComputeShader::bind(const std::string_view &id, const glm::mat4x4 &value) const {
        const GLint attr_id = glGetUniformLocation(program_id, id.data());
        if (attr_id < 0) return Err("Failed to bind uniform {}. Cannot find uniform location.", id);
        glUniformMatrix4fv(attr_id, 1, GL_FALSE, glm::value_ptr(value));
        return {};
    }
}
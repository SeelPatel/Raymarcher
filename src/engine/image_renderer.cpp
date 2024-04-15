#include <engine/image_renderer.h>

#include <cstring>
#include <array>

const char *vertex_shader_code = "#version 460\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "layout (location = 2) in vec2 aTexCoord;\n"
                                 "\n"
                                 "out vec3 ourColor;\n"
                                 "out vec2 TexCoord;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "\tgl_Position = vec4(aPos, 1.0);\n"
                                 "\tourColor = aColor;\n"
                                 "\tTexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
                                 "}\0";

const char *frag_shader_code = "#version 460\n"
                               "out vec4 FragColor;\n"
                               "\n"
                               "in vec3 ourColor;\n"
                               "in vec2 TexCoord;\n"
                               "\n"
                               "// texture samplers\n"
                               "uniform sampler2D texture1;\n"
                               "\n"
                               "void main()\n"
                               "{\n"
                               "    FragColor = texture(texture1, TexCoord);\n"
                               "}\0";

ImageRenderer::ImageRenderer(const GLuint width, const GLuint height) : width(width), height(height) {

}

Err ImageRenderer::init() {
    GLint success;
    std::array<char, 1024> error_info{};

    // Compile vertex shader
    GLuint vertex_shader_id;

    vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
    glCompileShader(vertex_shader_id);

    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader_id, error_info.size(), nullptr, error_info.data());
        return Err("Error compiling vertex shader source code. \n{}", error_info.data());
    }

    // Compile fragment shader
    GLuint fragment_shader_id;

    fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &frag_shader_code, nullptr);
    glCompileShader(fragment_shader_id);

    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader_id, error_info.size(), nullptr, error_info.data());
        return Err("Error compiling fragment shader source code. \n{}", error_info.data());
    }

    // Create shader program
    program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_id, error_info.size(), nullptr, error_info.data());
        return Err("Error linking shader program. \n{}", error_info.data());
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Generate and populate vertex buffer
    constexpr float vertices[] = {
            // positions          // colors           // texture coords
            1.f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
            1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // bottom left
            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f    // top left
    };
    constexpr uint32_t indices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Create the OpenGL Texture
    glGenTextures(1, &texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glUseProgram(program_id);
    glUniform1i(glGetUniformLocation(program_id, "texture1"), 0);
    glBindImageTexture(0, texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return {};
}

void ImageRenderer::draw() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glUseProgram(program_id);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}



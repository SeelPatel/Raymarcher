#ifndef RAYMARCHER_IMAGE_RENDERER_H
#define RAYMARCHER_IMAGE_RENDERER_H

#include <utils/err.h>

#include <glad/glad.h>

#include <cstdint>


class ImageRenderer {
    GLuint width, height;
    GLuint texture_id;

    GLuint vbo, vao, ebo;

    GLuint program_id;


public:
    ImageRenderer(GLuint width, GLuint height);

    Err init();

    void draw() const;

    constexpr GLuint image_width() const { return width; };

    constexpr GLuint image_height() const { return height; }

    [[nodiscard]] constexpr GLuint texture() const { return texture_id; }
};

#endif //RAYMARCHER_IMAGE_RENDERER_H

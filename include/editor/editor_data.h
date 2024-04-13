#ifndef RAYMARCHER_EDITOR_DATA_H
#define RAYMARCHER_EDITOR_DATA_H

#include <engine/scene.h>
#include <engine/image_renderer.h>

namespace editor {
    struct InputState {
        glm::vec2 prev_mouse_pos;
        glm::vec2 mouse_delta;
    };

    struct EditorData {
        GLFWwindow *window;
        float delta_time;
        Scene &scene;
        InputState &inputs;
        ImageRenderer &renderer;
    };
}

#endif //RAYMARCHER_EDITOR_DATA_H

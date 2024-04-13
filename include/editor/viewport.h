#ifndef RAYMARCHER_VIEWPORT_H
#define RAYMARCHER_VIEWPORT_H

#include <engine/scene.h>
#include <engine/image_renderer.h>
#include <editor/editor_data.h>

namespace editor {
    class Viewport {


    public:
        void update(EditorData &state);

    };
}

#endif //RAYMARCHER_VIEWPORT_H

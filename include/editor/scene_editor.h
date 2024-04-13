#ifndef RAYMARCHER_SCENE_EDITOR_H
#define RAYMARCHER_SCENE_EDITOR_H

#include <engine/scene.h>
#include <editor/editor_data.h>

namespace editor {
    class SceneEditor {

        Object *selected_object = nullptr;

        enum class Action {
            NONE, DELETE_OBJ
        };

        Action object_hierarchy(Object &object, const size_t level);

        void scene_hierarchy(EditorData &state);

        void object_editor(EditorData &state);

        void scene_editor(EditorData &state);

        const std::unordered_map<ObjectType, std::string_view> obj_type_mapping = {
                {ObjectType::Empty,           "Empty"},
                {ObjectType::Box,             "Box"},
                {ObjectType::Sphere,          "Sphere"},
                {ObjectType::InfiniteSpheres, "Infinite Spheres"},
                {ObjectType::Torus,           "Torus"},
        };
    public:
        void update(EditorData &state);

    };
}

#endif //RAYMARCHER_SCENE_EDITOR_H
#include <editor/scene_editor.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ranges>

namespace editor {

    void SceneEditor::update(EditorData &state) {
        scene_hierarchy(state);
        object_editor(state);
        scene_editor(state);
    }

    void SceneEditor::scene_hierarchy(EditorData &state) {
        ImGui::Begin("Hierarchy");

        Object &root = state.scene.root;
        object_hierarchy(root, 0);

        ImGui::End();
    }

    void SceneEditor::scene_editor(EditorData &state) {
        ImGui::Begin("Scene Settings");

        Scene &scene = state.scene;

        ImGui::SeparatorText("Sky Settings");
        ImGui::SliderFloat("Fog Distance", &scene.fog_distance, 10, 100);
        ImGui::ColorEdit3("Sky Top", (float *) &scene.sky_top_color);
        ImGui::ColorEdit3("Sky Bottom", (float *) &scene.sky_bottom_color);

        ImGui::SeparatorText("Light Settings");
        ImGui::DragFloat3("Light Position", (float *) &scene.light_pos, 0.125f);
        ImGui::ColorEdit3("Light Color", (float *) &scene.light_color);


        ImGui::SeparatorText("Misc.");
        ImGui::SliderFloat("FOV", &scene.fov, 10, 120);
        ImGui::SliderFloat("Shadow Intensity", &scene.shadow_intensity, 0, 1);
        ImGui::Checkbox("Visualize Distances", &scene.visualize_distances);

        ImGui::End();
    }

    void SceneEditor::object_editor(EditorData &state) {
        ImGui::Begin("Object Editor");
        if (!selected_object) {
            ImGui::End();
            return;
        }
        Object &object = *selected_object;

        // Modify name
        ImGui::InputText("Name", &object.name);

        // Modify Type
        if (ImGui::BeginCombo("Type", obj_type_mapping.at(object.obj_type).data())) {
            for (const auto &[obj_type, type_string]: obj_type_mapping) {
                if (ImGui::Selectable(obj_type_mapping.at(obj_type).data(), obj_type == object.obj_type))
                    object.obj_type = obj_type;
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Position, scale color
        ImGui::DragFloat3("Position", (float *) &object.pos, 0.125f);
        ImGui::DragFloat3("Scale", (float *) &object.scale, 0.125f);
        ImGui::ColorEdit3("Color", (float *) &object.color);

        ImGui::Separator();

        // Modify link type
        if (ImGui::BeginCombo("Link Type", link_type_mapping.at(object.link_type).data())) {
            for (const auto &[link_type, type_string]: link_type_mapping) {
                if (ImGui::Selectable(link_type_mapping.at(link_type).data(), link_type == object.link_type))
                    object.link_type = link_type;
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }

    SceneEditor::Action SceneEditor::object_hierarchy(Object &object, const size_t level) {
        const float offset_amount = ImGui::GetStyle().FramePadding.x * level * 4;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_amount);

        SceneEditor::Action result = Action::NONE;

        // Button to add child
        if (level < 2) {
            if (ImGui::Button(std::format("+##{}", object.uuid()).c_str())) {
                object.children.emplace_back(
                        Object(std::format("{} child", object.name), ObjectType::Box, {0, 0, 0}, {1, 1, 1}, {1, 1, 1}));
            }
            ImGui::SameLine();
        }

        // Button to delete object
        if (level != 0) {
            if (ImGui::Button(std::format("x##{}", object.uuid()).c_str())) {
                result = Action::DELETE_OBJ; // Signal parent to delete this child
                selected_object = nullptr; // todo fix this.
            }
            ImGui::SameLine();
        }

        // Display name and selectable
        if (ImGui::Selectable(std::format("{}##{}", object.name, object.uuid()).c_str(), &object == selected_object,
                              ImGuiSelectableFlags_AllowOverlap))
            selected_object = &object;

        // Draw children hierarchy
        int delete_idx = -1;
        for (auto [i, child]: std::ranges::views::enumerate(object.children)) {
            const Action child_result = object_hierarchy(child, level + 1);
            if (child_result == Action::DELETE_OBJ) delete_idx = i;
        }

        if (delete_idx >= 0) object.children.erase(object.children.begin() + delete_idx);

        return result;
    }

}

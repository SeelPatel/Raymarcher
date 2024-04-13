#include <editor/viewport.h>
#include <imgui.h>

namespace editor {
    void Viewport::update(EditorData &state) {

        ImGui::Begin("Viewport");

        // Fill viewport with current image
        const float vp_width = ImGui::GetWindowWidth();
        const float vp_height = ImGui::GetWindowHeight();
        const float img_width = state.renderer.image_width();
        const float img_height = state.renderer.image_height();

        const float horizontal_scale = vp_width / img_width;
        const float vertical_scale = vp_height / img_height;
        const float scale = std::min(horizontal_scale, vertical_scale);
        const ImVec2 offset_xy = {
                (vp_width - img_width * scale) / 2,
                (vp_height - img_height * scale) / 2,
        };

        ImGui::SetCursorPos(offset_xy);
        ImGui::Image((ImTextureID) state.renderer.texture(), ImVec2(img_width * scale, img_height * scale),
                     ImVec2(0, 1),
                     ImVec2(1, 0));

        // Control scene camera
        if (ImGui::IsWindowFocused() && ImGui::IsAnyMouseDown()) {
            state.scene.process_inputs(state.window, state.inputs.mouse_delta, state.delta_time);
            glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        ImGui::End();
    }
}
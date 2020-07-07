#pragma once

#include "sge.hh"
#include "sge_data.hh"
#include "imgui_ext.hh"

// OVERLAY
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class overlay : public runtime::view {
public:
    overlay (const runtime::api& z)
        : runtime::view (z, "Overlay", CUSTOM_DEBUG_UI) {
            utils::set_flag_at_mask (runtime_state, runtime_flags::CUSTOM_DEBUG_UI_ACTIVE, true);
        }
    

    virtual void custom_debug_ui () override {
        ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0));
        ImGui::Begin ("BACKGROUND", NULL,
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
        
        ImGui::SetWindowPos (ImVec2 (0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize (ImGui::GetIO ().DisplaySize);
        ImGui::SetWindowCollapsed (false);
        {
            int y = 20 + imgui::ext::guess_main_menu_bar_height();
            int* py = &y;
            const int line_spacing = 14;
            const std::function<void ()> next_line = [py]() {
                int yy = *py + line_spacing;
                *py = yy;
            };

            char text[64];
            sprintf (text, "SGE v%s", sge.system__get_state_string (sge::runtime::system_string_state::engine_version));
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text);
            next_line ();

            sprintf (text, "%d FPS @ %dx%d",
                sge.timer__get_fps (),
                sge.system__get_state_int (sge::runtime::system_int_state::compute_width),
                sge.system__get_state_int (sge::runtime::system_int_state::compute_height));
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text);
            next_line ();

            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), sge.system__get_state_string (sge::runtime::system_string_state::gpu_name));
            next_line ();
        }
        ImGui::End ();

        ImGui::PopStyleColor ();
        

    }
};

}



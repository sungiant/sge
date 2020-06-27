#if SGE_EXTENSIONS_ENABLED

#pragma once

#include "sge.hh"

#include <imgui/imgui.h>

// OVERLAY
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class overlay : public runtime::view {
public:
    overlay (const runtime::api& z) : runtime::view (z) {}

    virtual void debug_ui () override {
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
            int y = 20;
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
                sge.system__get_state_int (sge::runtime::system_int_state::screenwidth),
                sge.system__get_state_int (sge::runtime::system_int_state::screenheight));
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text);
            next_line ();

            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), sge.system__get_state_string (sge::runtime::system_string_state::gpu_name));
            next_line ();
        }
        ImGui::End ();

        ImGui::PopStyleColor ();

        /*
        ImGui::Begin ("Tools");
        {
            if (ImGui::Button ("Toggle Fullscreen")) {
                sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
            }


            if (ImGui::Button ("Set 800x600")) {
                sge.system__set_state_int (sge::runtime::system_int_state::screenwidth, 800);
                sge.system__set_state_int (sge::runtime::system_int_state::screenheight, 600);
            }
        }
        ImGui::End ();
        */
    }
};

}

#endif



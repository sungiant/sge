#pragma once

#include "sge.hh"
#include "sge_data.hh"
#include "imgui_ext.hh"

// OVERLAY
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class overlay : public runtime::view {
private:
    std::vector<data::vertex_pos_norm> toy_obj_verts;
    std::vector<uint32_t> toy_obj_indices;
    std::vector<data::vertex_pos_col> toy_toy_obj_verts_temp;
public:
    overlay (const runtime::api& z)
        : runtime::view (z)
    {
        data::get_teapot (toy_obj_verts, toy_obj_indices);
        
        for (auto& v : toy_obj_verts)
            toy_toy_obj_verts_temp.emplace_back(data::vertex_pos_col {v.position, 0xFFFFFFFF} );
    }

    virtual void debug_ui () override {
        static float toy_cam_zn = -0.1f, toy_cam_zf = -300.0f, toy_cam_fov = 45.0f;
        static math::vector3 toy_cam_pos = math::vector3 { 0, 0, 5 };
        static math::quaternion toy_cam_orientation = math::quaternion::identity;
        static math::rect toy_container { { 100, 100 }, { 320, 180 }};
        static math::vector3 toy_obj_pos = math::vector3 { 0, 0.0f, -1 };
        static math::quaternion toy_obj_orientation = math::quaternion::identity;
        static float toy_obj_t = 0.0f, toy_obj_speed = 0.3f;
        
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
            toy_obj_t += sge.timer__get_delta() * toy_obj_speed;
            toy_obj_orientation = math::quaternion().set_from_yaw_pitch_roll(toy_obj_t, 2.0 * toy_obj_t, -toy_obj_t);
            
            imgui::ext::draw_user_triangles (
                toy_container,
                toy_cam_pos,
                toy_cam_orientation,
                toy_cam_fov * math::DEG2RAD,
                toy_cam_zn,
                toy_cam_zf,
                toy_toy_obj_verts_temp,
                toy_obj_indices,
                toy_obj_pos,
                toy_obj_orientation,
                true);
            
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
        
        ImGui::Begin ("Tools"); {
            ImGui::SliderFloat("speed", &toy_obj_speed, 0.0f, 2.0f);
            ImGui::SliderInt2("container_offset", &toy_container.location.x, 0, 1000);
            ImGui::SliderInt2("container_extent", &toy_container.extent.x, 0, 1000);
            /*
            if (ImGui::Button ("Toggle Fullscreen")) {
                sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
            }


            if (ImGui::Button ("Set 800x600")) {
                sge.system__set_state_int (sge::runtime::system_int_state::screenwidth, 800);
                sge.system__set_state_int (sge::runtime::system_int_state::screenheight, 600);
            }
            */
        }
        ImGui::End ();
    }
};

}



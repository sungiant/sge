#pragma once

#include "sge.hh"
#include "sge_data.hh"
#include "imgui_ext.hh"

// OVERLAY
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class overlay : public runtime::view {
private:
    std::vector<data::vertex_pos_norm> obj_verts;
    std::vector<uint32_t> obj_indices;
    std::vector<data::vertex_pos_col> obj_verts_temp;
public:
    overlay (const runtime::api& z)
        : runtime::view (z)
    {
        data::get_torus (obj_verts, obj_indices);
        
        for (auto& v : obj_verts)
            obj_verts_temp.emplace_back(data::vertex_pos_col {v.position, 0xFFFFFFFF} );
    }

    virtual void debug_ui () override {
        static float gizmo_cam_zn = -0.1f, gizmo_cam_zf = -300.0f, gizmo_cam_fov = 45.0f;
        static math::vector3 gizmo_cam_pos = math::vector3 { 0, 0, 5 };
        static math::quaternion gizmo_cam_orientation = math::quaternion::identity;
        static math::rect gizmo_container { { 100, 100 }, { 320, 180 }};
        static math::vector3 gizmo_obj_pos = math::vector3 { 0, 0.0f, -1 };
        static math::quaternion gizmo_obj_orientation = math::quaternion::identity;
        static float gizmo_obj_t = 0.0f, gizmo_obj_speed = 0.3f;
        const std::span<sge::data::vertex_pos_col> gizmo_vertices = sge::data::unit_cube;
        
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
            gizmo_obj_t += sge.timer__get_delta() * gizmo_obj_speed;
            gizmo_obj_orientation = math::quaternion().set_from_yaw_pitch_roll(gizmo_obj_t, 2.0 * gizmo_obj_t, -gizmo_obj_t);
            std::vector<uint32_t> gizmo_indices (gizmo_vertices.size());
            std::iota (std::begin(gizmo_indices), std::end(gizmo_indices), 0); // on the stack?! todo

            
            imgui::ext::draw_user_triangles (
                gizmo_container,
                gizmo_cam_pos,
                gizmo_cam_orientation,
                gizmo_cam_fov * math::DEG2RAD,
                gizmo_cam_zn,
                gizmo_cam_zf,
                //gizmo_vertices,
                //gizmo_indices,
                obj_verts_temp,
                obj_indices,
                gizmo_obj_pos,
                gizmo_obj_orientation,
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
            ImGui::SliderFloat("speed", &gizmo_obj_speed, 0.0f, 2.0f);
            ImGui::SliderInt2("container_offset", &gizmo_container.location.x, 0, 1000);
            ImGui::SliderInt2("container_extent", &gizmo_container.extent.x, 0, 1000);
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



#pragma once

#include "sge.hh"
#include "sge_data.hh"
#include "sge_runtime.hh"
#include "sge_ext_freecam.hh"
#include "imgui_ext.hh"


// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class gizmo : public runtime::view {

    std::vector<sge::data::vertex_pos_col> gizmo_vertices;
    std::vector<uint32_t> gizmo_indices;;
    
    const int gizmo_size = 64;

    float gizmo_cam_zn = -0.1f, gizmo_cam_zf = -300.0f, gizmo_cam_fov = 45.0f;
    math::vector3 gizmo_cam_pos = math::vector3 { 0, 0, 4.5 };
    math::quaternion gizmo_cam_orientation = math::quaternion::identity;
    math::vector3 gizmo_obj_pos = math::vector3 { 0, 0.0f, -1 };
    math::quaternion gizmo_obj_orientation = math::quaternion::identity;
    
    ext::freecam* freecam;
        
public:
    gizmo (const runtime::api& z) : runtime::view (z) {
        if (1) {
            sge::data::get_unit_cube(gizmo_vertices);
            gizmo_indices.resize(gizmo_vertices.size());
            std::iota (std::begin(gizmo_indices), std::end(gizmo_indices), 0); // on the stack?! todo
        } else {
            sge::data::get_colourful_cube(gizmo_vertices, gizmo_indices);
        }
        
        freecam = static_cast <ext::freecam*> (sge.extension_get (runtime::type_id<ext::freecam>()));
        
    }
    virtual void update () override {
        if (!freecam || !freecam->is_enabled()) return;
        
        gizmo_obj_orientation = freecam->orientation;
    }

    virtual void debug_ui () override {
        if (!freecam || !freecam->is_enabled()) return;
        
        int screen_w = sge.system__get_state_int(runtime::system_int_state::screenwidth);

        ImGui::Begin ("Gizmo Debugger");
        ImGui::Text("cam position (x:%.2f, y:%.2f, z:%.2f)", gizmo_cam_pos.x, gizmo_cam_pos.y, gizmo_cam_pos.z);
        ImGui::Text("cam orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", gizmo_cam_orientation.i, gizmo_cam_orientation.j, gizmo_cam_orientation.k, gizmo_cam_orientation.u);
        ImGui::Text("obj position (x:%.2f, y:%.2f, z:%.2f)", gizmo_obj_pos.x, gizmo_obj_pos.y, gizmo_obj_pos.z);
        ImGui::Text("obj orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", gizmo_obj_orientation.i, gizmo_obj_orientation.j, gizmo_obj_orientation.k, gizmo_obj_orientation.u);
        ImGui::SliderFloat("near", &gizmo_cam_zn, -50.0f, 50.0f);
        ImGui::SliderFloat("far", &gizmo_cam_zf, -50.0f, 50.0f);
        ImGui::SliderFloat("fov", &gizmo_cam_fov, 0.0f, 180.0f);
        ImGui::End ();

        ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0));
        ImGui::Begin ("Gizmo", NULL,
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::SetWindowPos (ImVec2 (0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize (ImGui::GetIO ().DisplaySize);
        ImGui::SetWindowCollapsed (false);
        {
            math::rect gizmo_container = { { screen_w - gizmo_size, 0 }, { gizmo_size, gizmo_size }};

            imgui::ext::draw_user_triangles (
                gizmo_container,
                false,
                gizmo_cam_pos,
                gizmo_cam_orientation,
                gizmo_cam_fov * math::DEG2RAD,
                gizmo_cam_zn,
                gizmo_cam_zf,
                gizmo_vertices,
                gizmo_indices,
                gizmo_obj_pos,
                gizmo_obj_orientation);
        }
        ImGui::End ();
        ImGui::PopStyleColor ();
    }
};

}

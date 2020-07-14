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
    gizmo (const runtime::api& z) : runtime::view (z, "Gizmo", MANAGED_DEBUG_UI | CUSTOM_DEBUG_UI) {
        utils::set_flag_at_mask (runtime_state, runtime_flags::CUSTOM_DEBUG_UI_ACTIVE, true);
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
        if (!freecam || !freecam->is_active()) {
            set_active(false);
            return;
        }
        
        gizmo_obj_orientation = freecam->orientation;
    }

    virtual void managed_debug_ui () override {
        if (!freecam || !freecam->is_active()) return;
        ImGui::Text("cam position (x:%.2f, y:%.2f, z:%.2f)", gizmo_cam_pos.x, gizmo_cam_pos.y, gizmo_cam_pos.z);
        ImGui::Text("cam orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", gizmo_cam_orientation.i, gizmo_cam_orientation.j, gizmo_cam_orientation.k, gizmo_cam_orientation.u);
        ImGui::Text("obj position (x:%.2f, y:%.2f, z:%.2f)", gizmo_obj_pos.x, gizmo_obj_pos.y, gizmo_obj_pos.z);
        ImGui::Text("obj orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", gizmo_obj_orientation.i, gizmo_obj_orientation.j, gizmo_obj_orientation.k, gizmo_obj_orientation.u);
        ImGui::SliderFloat("near", &gizmo_cam_zn, -50.0f, 0.0f);
        ImGui::SliderFloat("far", &gizmo_cam_zf, -50.0f, 0.0f);
        ImGui::SliderFloat("fov", &gizmo_cam_fov, 0.0f, 180.0f);
        
        const math::vector3 point_of_interest = math::vector3::zero;
        const float current_distance_from_camera = freecam->position.distance (point_of_interest);

#define SET_ORI(y, p, r) freecam->orientation = math::quaternion().set_from_yaw_pitch_roll (y, p, r)
#define SET_POS freecam->position = point_of_interest + math::matrix33().set_from_orientation (freecam->orientation).backward () * current_distance_from_camera
      
        ImGui::Columns (3);
        
        
        if (ImGui::Button ("+X____")) { SET_ORI (math::HALF_PI, 0, 0);           SET_POS; }
        if (ImGui::Button ("-X____")) { SET_ORI (math::HALF_PI * 3.0f, 0, 0);    SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("__+Y__")) {
            // todo: this doesn't work right now as camera only has an orientation (it doesn't have an up vector - all of the orientations below are the same)
            // const math::vector3 b = math::matrix33().set_from_orientation (freecam->orientation).backward ();
            // if (abs (b.x) > abs (b.z)) {
            //     if (b.x >= 0)  SET_ORI (math::HALF_PI, -math::HALF_PI, 0);SET_POS;
            //     else           SET_ORI (3.0f * math::HALF_PI, -math::HALF_PI, 0);SET_POS;
            // }
            // else {
            //     if (b.z >= 0)  SET_ORI (0, -math::HALF_PI, 0);SET_POS;
            //     else           SET_ORI (math::PI, -math::HALF_PI, 0);SET_POS;
            // }
            SET_ORI (0, -math::HALF_PI, 0);SET_POS; // this is the best we can do for now without changing the freecam
        }
        if (ImGui::Button ("__-Y__")) {
            // todo: this doesn't work right now as camera only has an orientation (it doesn't have an up vector - all of the orientations below are the same)
            // const math::vector3 b = math::matrix33().set_from_orientation (freecam->orientation).backward ();
            // if (abs (b.x) > abs (b.z)) {
            //     if (b.x >= 0)  SET_ORI (math::HALF_PI, math::HALF_PI, 0);SET_POS;
            //     else           SET_ORI (3.0f * math::HALF_PI, math::HALF_PI, 0);SET_POS;
            // }
            // else {
            //     if (b.z >= 0)  SET_ORI (0, math::HALF_PI, 0);SET_POS;
            //     else           SET_ORI (math::PI, math::HALF_PI, 0);SET_POS;
            // }
            SET_ORI (0, math::HALF_PI, 0);SET_POS; // this is the best we can do for now without changing the freecam
        }
        ImGui::NextColumn ();
        if (ImGui::Button ("____+Z")) { freecam->orientation = math::quaternion::identity;    SET_POS; }
        if (ImGui::Button ("____-Z")) { SET_ORI (math::PI, 0, 0);                SET_POS; }
        ImGui::NextColumn ();
        ImGui::Separator ();
        if (ImGui::Button ("+X__+Z")) { SET_ORI (math::HALF_PI * 0.5f, 0, 0);   SET_POS; }
        if (ImGui::Button ("+X__-Z")) { SET_ORI (math::HALF_PI * 1.5f, 0, 0);   SET_POS; }
        if (ImGui::Button ("-X__-Z")) { SET_ORI (math::HALF_PI * 2.5f, 0, 0);   SET_POS; }
        if (ImGui::Button ("-X__+Z")) { SET_ORI (math::HALF_PI * 3.5f, 0, 0);   SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("__+Y+Z")) { SET_ORI (0, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("__-Y+Z")) { SET_ORI (0, math::HALF_PI * 0.5f,0);   SET_POS; }
        if (ImGui::Button ("__+Y-Z")) { SET_ORI (math::PI, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("__-Y-Z")) { SET_ORI (math::PI, math::HALF_PI * 0.5f,0);   SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("+X+Y__")) { SET_ORI (math::HALF_PI, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("+X-Y__")) { SET_ORI (math::HALF_PI, math::HALF_PI * 0.5f,0);   SET_POS; }
        if (ImGui::Button ("-X-Y__")) { SET_ORI (math::HALF_PI * 3.0f, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("-X+Y__")) { SET_ORI (math::HALF_PI * 3.0f, math::HALF_PI * 0.5f,0);   SET_POS; }
        ImGui::NextColumn ();
        ImGui::Columns (1);
        ImGui::Separator ();
        ImGui::Columns (4);
        if (ImGui::Button ("+X+Y+Z")) { SET_ORI (math::HALF_PI * 0.5f, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("+X+Y-Z")) { SET_ORI (math::HALF_PI * 1.5f, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("+X-Y+Z")) { SET_ORI (math::HALF_PI * 0.5f, math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("+X-Y-Z")) { SET_ORI (math::HALF_PI * 1.5f, math::HALF_PI * 0.5f, 0);   SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("-X+Y+Z")) { SET_ORI (-math::HALF_PI * 0.5f, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("-X+Y-Z")) { SET_ORI (-math::HALF_PI * 1.5f, -math::HALF_PI * 0.5f, 0);   SET_POS; }
        ImGui::NextColumn ();
        if (ImGui::Button ("-X-Y+Z")) { SET_ORI (-math::HALF_PI * 0.5f, math::HALF_PI * 0.5f, 0);   SET_POS; }
        if (ImGui::Button ("-X-Y-Z")) { SET_ORI (-math::HALF_PI * 1.5f, math::HALF_PI * 0.5f, 0);   SET_POS; }
        ImGui::NextColumn ();
        ImGui::Columns (1);
#undef SET_POS
#undef SET_ORI
    }
    
    virtual void custom_debug_ui () override {
        if (!freecam || !freecam->is_active()) return;
        ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0));
        ImGui::Begin ("Gizmo 3D", NULL,
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            
        ImGui::SetWindowPos (ImVec2 (0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize (ImGui::GetIO ().DisplaySize);
        ImGui::SetWindowCollapsed (false);
        {
            const int canvas_x = sge.system__get_state_int (runtime::system_int_state::canvas_offset_x);
            const int canvas_y = sge.system__get_state_int (runtime::system_int_state::canvas_offset_y);
            const int canvas_w = sge.system__get_state_int (runtime::system_int_state::canvas_width);
            math::rect gizmo_container = { { canvas_x + canvas_w - gizmo_size, canvas_y }, { gizmo_size, gizmo_size }};

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

#pragma once

#include "sge.hh"
#include "sge_runtime.hh"
#include "sge_data.hh"
#include "imgui_ext.hh"

namespace sge::ext {

class toy : public runtime::view {

    std::vector<data::vertex_pos_col> toy_toy_obj_verts;
    std::vector<uint32_t> toy_obj_indices;
    
    float toy_cam_zn = -0.1f, toy_cam_zf = -300.0f, toy_cam_fov = 45.0f;
    math::vector3 toy_cam_pos = math::vector3 { 0, 0, 5 };
    math::quaternion toy_cam_orientation = math::quaternion::identity;
    math::rect toy_container { { 0, 0 }, { 230, 180 }};
    math::vector3 toy_obj_pos = math::vector3 { 0, 0.0f, -1 };
    math::quaternion toy_obj_orientation = math::quaternion::identity;
    float toy_obj_t = 0.0f, toy_obj_speed = 0.3f;
    
public:
    toy (const runtime::api& z) : runtime::view (z) {

        std::vector<data::vertex_pos_norm> verts;
        data::get_teapot (verts, toy_obj_indices, 1.0f, 4);
        
        for (auto& v : verts)
            toy_toy_obj_verts.emplace_back(data::vertex_pos_col {v.position, 0xFFFFFFFF} );
    }

    virtual void debug_ui () override {
        ImGui::Begin ("Toy");
        {
            ImGui::SliderInt2("container_offset", &toy_container.location.x, 0, 1000);
            ImGui::SliderInt2("container_extent", &toy_container.extent.x, 0, 1000);
            
            toy_obj_t += sge.timer__get_delta() * toy_obj_speed;
            toy_obj_orientation = math::quaternion().set_from_yaw_pitch_roll(toy_obj_t, 2.0 * toy_obj_t, -toy_obj_t);
            
            imgui::ext::draw_user_triangles (
                toy_container,
                true,
                toy_cam_pos,
                toy_cam_orientation,
                toy_cam_fov * math::DEG2RAD,
                toy_cam_zn,
                toy_cam_zf,
                toy_toy_obj_verts,
                toy_obj_indices,
                toy_obj_pos,
                toy_obj_orientation,
                true);
                ImGui::SliderFloat("speed", &toy_obj_speed, 0.0f, 2.0f);
        }
        ImGui::End ();
    }
};

}

#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_runtime.hh"
#include "sge_data.hh"

namespace imgui::ext {
    
    using namespace sge::math;
    
    
    inline uint32_t average_colour (uint32_t v0, uint32_t v1, uint32_t v2) {
        const ImColor col0 = ImColor (v0);
        const ImColor col1 = ImColor (v1);
        const ImColor col2 = ImColor (v2);
        const ImU32 av_col = IM_COL32 (
            (col0.Value.x + col1.Value.x + col2.Value.x) / 3.0f *255.0f,
            (col0.Value.y + col1.Value.y + col2.Value.y) / 3.0f *255.0f,
            (col0.Value.z + col1.Value.z + col2.Value.z) / 3.0f *255.0f,
            (col0.Value.w + col1.Value.w + col2.Value.w) / 3.0f *255.0f);
        return av_col;
    }
    
    inline ImVec2 ndc_to_container_coordinates (const vector3& ndc, const rect& container) {
        return ImVec2 (
            (((ndc.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
            (float) container.extent.y - (((ndc.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
    }
    inline bool is_within_ndc (vector3 v) { return v.x >= -1 && v.x <= 1 && v.y >= -1 && v.y <= 1 && v.z >= -1 && v.z <= 1; }
    
    struct tri {
        vector3 p0, p1, p2;
        
        vector3 normal () const { return (p1-p0) ^ (p2-p0); } // anti-clockwise winding
        vector3 centroid () const { return p0 + ((((p1+p2)/2.0f) - p0) * 2.0f / 3.0f); }
    };
    
    struct tri_index_group {
        uint32_t i0, i1, i2;
        void get_tri (const std::span<sge::data::vertex_pos_col> vertices, const matrix44& transform, tri& result) const {
            result.p0 = vertices[i0].position % transform;
            result.p1 = vertices[i1].position % transform;
            result.p2 = vertices[i2].position % transform;
        }
        uint32_t get_col (const std::span<sge::data::vertex_pos_col> vertices) const {
            return average_colour (vertices[i0].colour, vertices[i1].colour, vertices[i2].colour);
        }
    };

    inline void draw_user_triangles (
        const rect& container,
        const vector3& camera_position,
        const quaternion& camera_orientation,
        const float camera_fov,
        const float camera_near,
        const float camera_far,
        const std::span<sge::data::vertex_pos_col> vertices,
        std::span<uint32_t> indices,
        const vector3& obj_pos,
        const quaternion& model_orientation,
        const std::optional<float> wireframe_thickness = std::nullopt) {

        ImDrawList& drawList = *ImGui::GetWindowDrawList();
        
        const ImVec2 im_min = ImVec2(container.location.x, container.location.y);
        const ImVec2 im_max = ImVec2(container.location.x + container.extent.x, container.location.y + container.extent.y);
        
        drawList.AddRectFilled(im_min, im_max, 0xCC333333);
        drawList.AddRect(im_min, im_max,0xFF00FF00);
        drawList.PushClipRect (im_min, im_max);
        
        const float aspect = (float) container.extent.x / (float) container.extent.y;
        
        const matrix44 world = matrix44()
            .set_translation_component(obj_pos)
            .set_rotation_component(model_orientation);
        
        const matrix44 viewF = matrix44()
            .set_translation_component(camera_position)
            .set_rotation_component(camera_orientation);
        
        matrix44 view = viewF;
        view.inverse();
        
        const matrix44 view2 = matrix44()
            .set_as_view_transform_from_look_at_target(camera_position, vector3::zero, vector3::up);
        
        const matrix44 proj = matrix44()
            .set_as_perspective_fov_rh (camera_fov, aspect, camera_near, camera_far);
        
        const matrix44 wv = world * view;
        
        assert (indices.size() % 3 == 0);
        
        const int num_tris = indices.size() / 3;
        
        tri_index_group* tri_indices = reinterpret_cast<tri_index_group*>(indices.data());
        
        const auto f = [&](const tri_index_group& l, const tri_index_group& r){
            tri tri_l; l.get_tri (vertices, wv, tri_l);
            tri tri_r; r.get_tri (vertices, wv, tri_r);
            const float zl = tri_l.centroid().z;
            const float zr = tri_r.centroid().z;
            return zl < zr;
        };
        
        // sort groups of indicies in-place by Z
        std::sort(tri_indices, tri_indices + num_tris, f);
        
        for (int i = 0; i < num_tris; ++i) {
            
            const uint32_t av_col = tri_indices[i].get_col (vertices);
            
            tri tri_vs; tri_indices[i].get_tri (vertices, wv, tri_vs);

            if ((tri_vs.normal() | -tri_vs.centroid()) < -0.0f) // https://chortle.ccsu.edu/VectorLessons/vch09/vch09_6.html
                continue; // cull backfaces. https://www.youtube.com/watch?v=zGyfiOqiR4s

            const vector3 ndc0 = tri_vs.p0 % proj;
            const vector3 ndc1 = tri_vs.p1 % proj;
            const vector3 ndc2 = tri_vs.p2 % proj;
            
            if (!is_within_ndc (ndc0) || !is_within_ndc (ndc1) || !is_within_ndc (ndc2))
                continue;
                
            const ImVec2 p0 = ndc_to_container_coordinates (ndc0, container);
            const ImVec2 p1 = ndc_to_container_coordinates (ndc1, container);
            const ImVec2 p2 = ndc_to_container_coordinates (ndc2, container);
            if (wireframe_thickness.has_value ())
                drawList.AddTriangle (p0, p1, p2, av_col, wireframe_thickness.value());
            else
                drawList.AddTriangleFilled (p0, p1, p2, av_col);
        }
        
        drawList.PopClipRect();
    }
}

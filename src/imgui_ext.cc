#include "imgui_ext.hh"

namespace imgui::ext {
    
using namespace sge::math;

inline ImVec2 ndc_to_container_coordinates (const vector3& ndc, const rect& container) {
    return ImVec2 (
        (((ndc.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
        (float) container.extent.y - (((ndc.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
}

inline bool is_within_ndc (vector3 v) { return v.x >= -1 && v.x <= 1 && v.y >= -1 && v.y <= 1 && v.z >= -1 && v.z <= 1; }

struct tri_positions {
    vector3 p0, p1, p2;
    vector3 normal () const { return (p1-p0) ^ (p2-p0); } // anti-clockwise winding
    vector3 centroid () const { return p0 + ((((p1+p2)/2.0f) - p0) * 2.0f / 3.0f); }
};

struct tri_index_group {
    uint32_t i0, i1, i2;
    inline void get_tri_positions (const std::span<sge::data::vertex_pos_col> vertices, const matrix44& transform, tri_positions& result) const {
        result.p0 = vertices[i0].position % transform;
        result.p1 = vertices[i1].position % transform;
        result.p2 = vertices[i2].position % transform;
    }
    inline uint32_t get_tri_average_colour (const std::span<sge::data::vertex_pos_col> vertices) const {
        const ImColor col0 = ImColor (vertices[i0].colour);
        const ImColor col1 = ImColor (vertices[i1].colour);
        const ImColor col2 = ImColor (vertices[i2].colour);
        const ImU32 av_col = IM_COL32 (
            (col0.Value.x + col1.Value.x + col2.Value.x) / 3.0f *255.0f,
            (col0.Value.y + col1.Value.y + col2.Value.y) / 3.0f *255.0f,
            (col0.Value.z + col1.Value.z + col2.Value.z) / 3.0f *255.0f,
            (col0.Value.w + col1.Value.w + col2.Value.w) / 3.0f *255.0f);
        return av_col;
    }
    inline void get_tri_colours (const std::span<sge::data::vertex_pos_col> vertices, uint32_t& c0, uint32_t& c1, uint32_t& c2) const {
        c0 = vertices[i0].colour; c1 = vertices[i1].colour; c2 = vertices[i2].colour;
    }
    inline bool is_tri_multicoloured (const std::span<sge::data::vertex_pos_col> vertices) const {
        return !((vertices[i0].colour == vertices[i1].colour) && (vertices[i1].colour == vertices[i2].colour));
    }
};

void draw_user_triangles (
    const rect& user_container,
    const bool user_container_relative_to_window,
    const vector3& camera_position,
    const quaternion& camera_orientation,
    const float camera_fov,
    const float camera_near,
    const float camera_far,
    const std::span<sge::data::vertex_pos_col> vertices,
    std::span<uint32_t> indices,
    const vector3& obj_pos,
    const quaternion& model_orientation,
    const bool lighting)
{
    assert (indices.size() % 3 == 0);
    
    if (ImGui::IsWindowCollapsed())
        return; // early out as doing all of this on the cpu isn't cheap.
    
    rect container = user_container;
    if (user_container_relative_to_window) {
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const ImVec2 region_max = ImGui::GetContentRegionMax();
        const float aspect = (float)user_container.extent.x / (float)user_container.extent.y;
        if (user_container.location.x + container.extent.x > region_max.x) {
            container.extent.x = std::max (16.0f, region_max.x - container.location.x);
            container.extent.y = container.extent.x / aspect;
        }
        container.location.x += p.x;
        container.location.y += p.y;
        ImGui::SetCursorScreenPos(ImVec2 { p.x, (float)container.location.y + (float)container.extent.y });
    }
    
    ImDrawList& drawList = *ImGui::GetWindowDrawList();
    
    const ImVec2 im_min = ImVec2(container.location.x, container.location.y);
    const ImVec2 im_max = ImVec2(container.location.x + container.extent.x, container.location.y + container.extent.y);
    
    drawList.AddRectFilled(im_min, im_max, 0x22000000);
    //drawList.AddRect(im_min, im_max,0xFF00FF00);
    
    const float aspect = (float) container.extent.x / (float) container.extent.y;
    const matrix44 world = matrix44().set_translation_component(obj_pos).set_rotation_component(model_orientation);
    const matrix44 view_frame = matrix44().set_translation_component(camera_position).set_rotation_component(camera_orientation);
    const matrix44 view = view_frame.inverse();
    const matrix44 proj = matrix44().set_as_perspective_fov_rh (camera_fov, aspect, camera_near, camera_far);
    const matrix44 wv = world * view;
    const int num_tris = indices.size() / 3;
    tri_index_group * const tri_indices = reinterpret_cast<tri_index_group*>(indices.data());
    
    const auto f = [&](const tri_index_group& l, const tri_index_group& r){
        tri_positions tri_l; l.get_tri_positions (vertices, wv, tri_l);
        tri_positions tri_r; r.get_tri_positions (vertices, wv, tri_r);
        const float zl = tri_l.centroid().z;
        const float zr = tri_r.centroid().z;
        return zl < zr;
    };
    
    // sort groups of indicies in-place by Z
    std::sort(tri_indices, tri_indices + num_tris, f);
    
    for (int i = 0; i < num_tris; ++i) {
        const uint32_t av_col = tri_indices[i].get_tri_average_colour (vertices);
        tri_positions tri_vs; tri_indices[i].get_tri_positions (vertices, wv, tri_vs);
        if ((tri_vs.normal() | -tri_vs.centroid()) < -0.0f) // https://chortle.ccsu.edu/VectorLessons/vch09/vch09_6.html
            // right handed direction: https://www.youtube.com/watch?v=zGyfiOqiR4s
            continue; // cull backfaces.
        
        uint32_t tri_col = av_col;
        if (lighting) {
            const vector3 light_dir = -vector3::one;
            const vector3 light_col = vector3 { 0.855f, 0.647f, 0.125f };
            const vector3 ambient_col = vector3 { 0.5f, 0.5f, 0.5f };
            const ImColor c = ImColor (av_col);
            const vector3 tri_albedo = { c.Value.x, c.Value.y, c.Value.z };
            const vector3 N = ~(tri_vs.normal());
            const vector3 L = ~(light_dir);
            const vector3 lighting = light_col * tri_albedo * (N | L) + ambient_col * tri_albedo;
            const vector3 lighting2 = vector3 {std::clamp (lighting.x, 0.0f, 1.0f), std::clamp (lighting.y, 0.0f, 1.0f), std::clamp (lighting.z, 0.0f, 1.0f)};
            const ImColor c2 = ImColor (lighting2.x, lighting2.y, lighting2.z);
            tri_col = c2;
        }
        
        const vector3 ndc0 = tri_vs.p0 % proj;
        const vector3 ndc1 = tri_vs.p1 % proj;
        const vector3 ndc2 = tri_vs.p2 % proj;
        
        if (!is_within_ndc (ndc0) || !is_within_ndc (ndc1) || !is_within_ndc (ndc2))
            continue;
            
        const ImVec2 p0 = ndc_to_container_coordinates (ndc0, container);
        const ImVec2 p1 = ndc_to_container_coordinates (ndc1, container);
        const ImVec2 p2 = ndc_to_container_coordinates (ndc2, container);

        if (tri_indices[i].is_tri_multicoloured (vertices)) {
            uint32_t c0, c1, c2; tri_indices[i].get_tri_colours (vertices, c0, c1, c2);
            drawList.AddTriangleFilledMultiColor (p0, p1, p2, c0, c1, c2);
        } else {
            drawList.AddTriangleFilled (p0, p1, p2, tri_col);
        }
    }
}
}

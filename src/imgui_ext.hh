#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_runtime.hh"

namespace imgui::ext {
    
    using namespace sge::math;

    struct vert { vector3 position; uint32_t colour; };
    static std::vector<vert> vs = { //ABGR
        {{ -1.0f, -1.0f, -1.0f }, 0xFF000000},
        {{  1.0f, -1.0f, -1.0f }, 0xFFFF0000},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        {{  1.0f,  1.0f, -1.0f }, 0xFFFFFF00},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f,  1.0f }, 0xFFFF00FF},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF00FFFF},
        {{  1.0f,  1.0f,  1.0f }, 0xFFFFFFFF},
    };
    static std::vector<uint32_t> vi = {
        0, 2, 3, 0, 3, 1, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5,
        0, 4, 6, 0, 6, 2, 2, 6, 7, 2, 7, 3, 0, 1, 5, 0, 5, 4, };
    
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
    
    inline void draw_user_triangles (
        const rect& container,
        const vector3& camera_position,
        const quaternion& camera_orientation,
        const float camera_fov,
        const float camera_near,
        const float camera_far,
        const std::span<vert> vertices,
        const std::span<uint32_t> indices,
        const matrix44& world,
        const std::optional<float> wireframe_thickness = std::nullopt) {

        ImDrawList& drawList = *ImGui::GetWindowDrawList();
        
        drawList.AddRectFilled(ImVec2(container.location.x, container.location.y), ImVec2 (container.extent.x, container.extent.y), 0xCC333333);
        
        //drawList.AddRect(ImVec2(container.location.x, container.location.y), ImVec2 (container.extent.x, container.extent.y), 0xFF00FF00);
        
        drawList.PushClipRect (ImVec2(container.location.x, container.location.y), ImVec2 (container.extent.x, container.extent.y));

        const matrix44 view = matrix44()
            .set_as_view_transform_from_look_at_target(camera_position, vector3::zero, vector3::unit_y);
        
        
        const float aspect = (float) container.extent.x / (float) container.extent.y;
        
        const matrix44 proj = matrix44()
            .set_as_perspective_from_field_of_view (camera_fov, aspect, camera_near, camera_far);
        
        const matrix44 vp = view * proj;
        
        
        for (int i = 0; i < indices.size(); i+=3) {
            const vert& vert0 = vertices[indices[i]];
            const vert& vert1 = vertices[indices[i+1]];
            const vert& vert2 = vertices[indices[i+2]];
            
            const ImU32 av_col = average_colour (vert0.colour, vert1.colour, vert2.colour);
            
            // to world space.
            const vector3 w0 = vert0.position * world;
            const vector3 w1 = vert1.position * world;
            const vector3 w2 = vert2.position * world;
            /*
            {
                // face normal
                const vector3 normal = (w1-w0) ^ (w2-w0);
                // face center of mass
                const vector3 centroid = w0 + ((((w1+w2)/2.0f) - w0) * 2.0f / 3.0f);
                const vector3 centroid_to_camera = camera_position - centroid;
                if ((normal | centroid_to_camera) > 0.0f)
                    continue; // cull backfaces.
            }
             */
            // now to view space.
            const vector3 v0 = w0 * view;
            const vector3 v1 = w1 * view;
            const vector3 v2 = w2 * view;
            const vector3 cam_in_view = camera_position * view;
            {
                // face normal
                const vector3 normal = (v1-v0) ^ (v2-v0);
                // face center of mass
                const vector3 centroid = v0 + ((((v1+v2)/2.0f) - v0) * 2.0f / 3.0f);
                const vector3 centroid_to_camera = cam_in_view - centroid;
                if ((normal | centroid_to_camera) > 0.0f)
                    continue; // cull backfaces.
            }
            
            // now to projection space. hack as the proj mat is not the right one for us
            const vector3 temp0 = w0 * vp / 10.0f;
            const vector3 temp1 = w1 * vp / 10.0f;
            const vector3 temp2 = w2 * vp / 10.0f;
            const ImVec2 p0 = ImVec2 (
                (((temp0.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (((temp0.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            const ImVec2 p1 = ImVec2 (
                (((temp1.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (((temp1.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            const ImVec2 p2 = ImVec2 (
                (((temp2.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (((temp2.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            
            // todo - z order sorting.
            if (wireframe_thickness.has_value ())
                drawList.AddTriangle (p0, p1, p2, av_col, wireframe_thickness.value());
            else
                drawList.AddTriangleFilled (p0, p1, p2, av_col);
        }
        
        drawList.PopClipRect();
    }
    
    inline static void test (const sge::runtime::api& sge) {
        const rect container { { 100, 100 }, { 320, 240 }};
        const float time = sge.timer__get_time();
        const quaternion model_orientation = quaternion().set_from_yaw_pitch_roll(time, 2.0 * time, -time);
        const matrix44 world = matrix44().set_rotation_component(model_orientation);
        draw_user_triangles (
            container,
            vector3 {0, 0, 15},
            quaternion::identity,
            57.5f, 0.01f, 100.0f, vs, vi, world);
    }
}

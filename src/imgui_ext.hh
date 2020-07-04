#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_runtime.hh"

#include "ext_freecam.hh"

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
    
    static std::vector<vert> vs2 = { //ABGR
        
        {{  1.0f,  1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f, -1.0f }, 0xFF0000FF},
        {{  1.0f,  1.0f, -1.0f }, 0xFF0000FF},
        {{  1.0f,  1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f, -1.0f }, 0xFF0000FF},

        {{  1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{  1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{  1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        
        {{  1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        {{  1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        {{  1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        
        {{ -1.0f, -1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF000033},
        
        {{ -1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f,  1.0f }, 0xFF003300},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF003300},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f,  1.0f }, 0xFF003300},
        
        {{ -1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{  1.0f,  1.0f, -1.0f }, 0xFF330000},
        {{  1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF330000},
        {{  1.0f,  1.0f, -1.0f }, 0xFF330000},
         
    };
    
    static std::vector<uint32_t> vi2 (vs2.size());
    
    
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
    
    inline bool is_within_ndc (vector3 v) { return v.x >= -1 && v.x <= 1 && v.y >= -1 && v.y <= 1 && v.z >= -1 && v.z <= 1; }
    
    inline void draw_user_triangles (
        const rect& container,
        const vector3& camera_position,
        const quaternion& camera_orientation,
        const float camera_fov,
        const float camera_near,
        const float camera_far,
        const std::span<vert> vertices,
        const std::span<uint32_t> indices,
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
            .set_position_component(obj_pos)
            .set_rotation_component(model_orientation);
        
        const matrix44 view = matrix44()
            .set_as_view_transform_from_look_at_target(camera_position, obj_pos, vector3::up);
        
        const matrix44 proj = matrix44()
            .set_as_perspective_fov_rh (camera_fov, aspect, camera_near, camera_far);
            //.set_as_orthographic_off_center (-16.0f, 16.0f, -9.0f, 9.0f, 1000, -1000);
            //.set_as_perspective_rh (1.6f, 0.9f, camera_near, camera_far);
        //const matrix44 vp = view * proj;
        
        const matrix44 wv = world * view;
        const matrix44 wvp = world * view * proj;
        
        
        for (int i = 0; i < indices.size(); i+=3) {
            const vert& vert0 = vertices[indices[i]];
            const vert& vert1 = vertices[indices[i+1]];
            const vert& vert2 = vertices[indices[i+2]];
            
            const ImU32 av_col = average_colour (vert0.colour, vert1.colour, vert2.colour);
            
            // view space.
            const vector3 v0 = vert0.position % wv;
            const vector3 v1 = vert1.position % wv;
            const vector3 v2 = vert2.position % wv;
            const vector3 cam_in_view = camera_position % view;
            {
                // face normal
                const vector3 normal = (v1-v0) ^ (v2-v0);
                // face center of mass
                const vector3 centroid = v0 + ((((v1+v2)/2.0f) - v0) * 2.0f / 3.0f);
                const vector3 centroid_to_camera = cam_in_view - centroid;
                //if ((normal | centroid_to_camera) < -0.5f) // https://chortle.ccsu.edu/VectorLessons/vch09/vch09_6.html
                //    continue; // cull backfaces.
            }
            
            vector3 temp0 = v0 % proj;
            vector3 temp1 = v1 % proj;
            vector3 temp2 = v2 % proj;
            
            //if (!is_within_ndc (temp0) || !is_within_ndc (temp1) || !is_within_ndc (temp2))
            //    continue;
                
            
            const ImVec2 p0 = ImVec2 (
                (((temp0.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (float) container.extent.y - (((temp0.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            const ImVec2 p1 = ImVec2 (
                (((temp1.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (float) container.extent.y - (((temp1.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            const ImVec2 p2 = ImVec2 (
                (((temp2.x + 1.0f) / 2.0f) * (float) container.extent.x) + (float) container.location.x,
                (float) container.extent.y - (((temp2.y + 1.0f) / 2.0f) * (float) container.extent.y) + (float) container.location.y);
            
            // todo - z order sorting.
            if (wireframe_thickness.has_value ())
                drawList.AddTriangle (p0, p1, p2, av_col, wireframe_thickness.value());
            else
                drawList.AddTriangleFilled (p0, p1, p2, av_col);
        }
        
        drawList.PopClipRect();
    }

    inline static void test (const sge::runtime::api& sge) {

        static bool flag = false;
        static rect container { { 100, 100 }, { 320, 180 }};
        static vector3 cam_pos = vector3 { 0, 0, 15 };
        static vector3 obj_pos = vector3 { 0, 0.5f, -10 };
        static float t = 0.0f, zn = -1.0f, zf = -300.0f, fov = 45.0f, speed = 0.0f;
        
        
        const auto freecam = static_cast <sge::ext::freecam*> (sge.extension_get (sge::runtime::type_id<sge::ext::freecam>()));
        
        if (flag == false) {
            flag = true;
            if (freecam)
                freecam->position = cam_pos;
        }
        
        if (freecam) {
            //cam_pos = freecam->position;
        }
        ImGui::Begin ("Gizmo");
        ImGui::SliderFloat3("cam_pos", &cam_pos.x, 0.0f, 250.0f);
        ImGui::SliderFloat3("obj_pos", &obj_pos.x, -50.0f, 50.0f);
        ImGui::SliderInt2("container_offset", &container.location.x, 0, 300);
        ImGui::SliderInt2("container_extent", &container.extent.x, 0, 600);
        ImGui::SliderFloat("near", &zn, -50.0f, 50.0f);
        ImGui::SliderFloat("far", &zf, -500.0f, 500.0f);
        ImGui::SliderFloat("fov", &fov, 0.0f, 180.0f);
        ImGui::SliderFloat("speed", &speed, 0.0f, 2.0f);
        ImGui::End ();
        
        
        
        t += sge.timer__get_delta() * speed;
        const quaternion model_orientation = quaternion().set_from_yaw_pitch_roll(t, 2.0 * t, -t);

        std::iota (std::begin(vi2), std::end(vi2), 0);
        
        
        draw_user_triangles (
            container,
            cam_pos,
            quaternion::identity,
            fov * DEG2RAD, zn, zf, vs2, vi2, obj_pos, model_orientation);
    }
}

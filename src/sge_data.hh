#pragma once

#include "sge.hh"
#include "sge_math.hh"

namespace sge::data {

    struct vertex_pos_col { math::vector3 position; uint32_t colour; }; // ABGR
    struct vertex_pos_norm { math::vector3 position; math::vector3 normal; };

    static std::vector<vertex_pos_col> unit_cube = { // anti-clockwise winding
        // +X: red
        {{  1.0f,  1.0f,  1.0f }, 0xFF6666AA},
        {{  1.0f, -1.0f, -1.0f }, 0xFF6666AA},
        {{  1.0f,  1.0f, -1.0f }, 0xFF6666AA},
        {{  1.0f,  1.0f,  1.0f }, 0xFF6666AA},
        {{  1.0f, -1.0f,  1.0f }, 0xFF6666AA},
        {{  1.0f, -1.0f, -1.0f }, 0xFF6666AA},
        // +Y: green
        {{  1.0f,  1.0f,  1.0f }, 0xFF66AA66},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF66AA66},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF66AA66},
        {{  1.0f,  1.0f,  1.0f }, 0xFF66AA66},
        {{  1.0f,  1.0f, -1.0f }, 0xFF66AA66},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF66AA66},
        // +Z: blue
        //{{  1.0f,  1.0f,  1.1f }, 0xFF6A0DAD},
        //{{ -1.0f, -1.0f,  1.1f }, 0xFF6A0DAD},
        //{{  1.0f, -1.0f,  1.1f }, 0xFF6A0DAD},
        {{  1.0f,  1.0f,  1.0f }, 0xFFAA6666},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFAA6666},
        {{  1.0f, -1.0f,  1.0f }, 0xFFAA6666},
        {{  1.0f,  1.0f,  1.0f }, 0xFFAA6666},
        {{ -1.0f,  1.0f,  1.0f }, 0xFFAA6666},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFAA6666},
        // -X: dark red
        {{ -1.0f, -1.0f, -1.0f }, 0xFF444466},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF444466},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF444466},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF444466},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF444466},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF444466},
        // -Y: dark green
        {{ -1.0f, -1.0f, -1.0f }, 0xFF446644},
        {{  1.0f, -1.0f,  1.0f }, 0xFF446644},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF446644},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF446644},
        {{  1.0f, -1.0f, -1.0f }, 0xFF446644},
        {{  1.0f, -1.0f,  1.0f }, 0xFF446644},
        // -Z: dark blue
        {{ -1.0f, -1.0f, -1.0f }, 0xFF664444},
        {{  1.0f,  1.0f, -1.0f }, 0xFF664444},
        {{  1.0f, -1.0f, -1.0f }, 0xFF664444},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF664444},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF664444},
        {{  1.0f,  1.0f, -1.0f }, 0xFF664444},
    };

    inline static void get_torus (std::vector<vertex_pos_norm>& verts, std::vector<uint32_t>& indices, const int tessellation = 32, const float thickness = 0.333f, const float diameter = 1.0f) {
        assert(tessellation >= 3);
        for (int i = 0; i < tessellation; i++) {
            const float outerAngle = i * math::TAU / tessellation;
            const math::matrix44 translation = math::matrix44().set_translation_component (diameter / 2, 0, 0);
            const math::matrix44 rotationY = math::matrix44().set_rotation_component(math::matrix33().set_from_y_axis_angle (outerAngle));
            const math::matrix44 transform = translation * rotationY;
            for (int j = 0; j < tessellation; j++) {
                const float innerAngle = j * math::TAU / tessellation;
                const float dx = cos(innerAngle);
                const float dy = sin(innerAngle);
                const math::vector3 normalIn = math::vector3 (dx, dy, 0);
                const math::vector3 positionIn = normalIn * thickness / 2;
                const math::vector3 position = positionIn % transform;
                const math::vector3 normal = normalIn % transform;
                verts.emplace_back(vertex_pos_norm{position, normal});
                const int nextI = (i + 1) % tessellation;
                const int nextJ = (j + 1) % tessellation;
                indices.emplace_back(i * tessellation + j);
                indices.emplace_back(nextI * tessellation + j);
                indices.emplace_back(i * tessellation + nextJ);
                
                indices.emplace_back(i * tessellation + nextJ);
                indices.emplace_back(nextI * tessellation + j);
                indices.emplace_back(nextI * tessellation + nextJ);
            }
        }
    }

    inline static void get_teapot (std::vector<vertex_pos_norm>& verts, std::vector<uint32_t>& indices) {
        
    }
}

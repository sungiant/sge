#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_utils.hh"

namespace sge::data { // anti-clockwise winding

    struct vertex_pos_col { math::vector3 position; uint32_t colour; }; // ABGR
    struct vertex_pos_norm { math::vector3 position; math::vector3 normal; };

    inline static void get_unit_cube (std::vector<vertex_pos_col>& verts) {
        assert (verts.size() == 0);
        verts = {
            // +X: red
            {{  1.0f,  1.0f,  1.0f }, 0xFF6666AA }, {{  1.0f, -1.0f, -1.0f }, 0xFF6666AA }, {{  1.0f,  1.0f, -1.0f }, 0xFF6666AA },
            {{  1.0f,  1.0f,  1.0f }, 0xFF6666AA }, {{  1.0f, -1.0f,  1.0f }, 0xFF6666AA }, {{  1.0f, -1.0f, -1.0f }, 0xFF6666AA },
            // +Y: green
            {{  1.0f,  1.0f,  1.0f }, 0xFF66AA66 }, {{ -1.0f,  1.0f, -1.0f }, 0xFF66AA66 }, {{ -1.0f,  1.0f,  1.0f }, 0xFF66AA66 },
            {{  1.0f,  1.0f,  1.0f }, 0xFF66AA66 }, {{  1.0f,  1.0f, -1.0f }, 0xFF66AA66 }, {{ -1.0f,  1.0f, -1.0f }, 0xFF66AA66 },
            // +Z: blue
            {{  1.0f,  1.0f,  1.0f }, 0xFFAA6666 }, {{ -1.0f, -1.0f,  1.0f }, 0xFFAA6666 }, {{  1.0f, -1.0f,  1.0f }, 0xFFAA6666 },
            {{  1.0f,  1.0f,  1.0f }, 0xFFAA6666 }, {{ -1.0f,  1.0f,  1.0f }, 0xFFAA6666 }, {{ -1.0f, -1.0f,  1.0f }, 0xFFAA6666 },
            // -X: dark red
            {{ -1.0f, -1.0f, -1.0f }, 0xFF444466 }, {{ -1.0f,  1.0f,  1.0f }, 0xFF444466 }, {{ -1.0f,  1.0f, -1.0f }, 0xFF444466 },
            {{ -1.0f, -1.0f, -1.0f }, 0xFF444466 }, {{ -1.0f, -1.0f,  1.0f }, 0xFF444466 }, {{ -1.0f,  1.0f,  1.0f }, 0xFF444466 },
            // -Y: dark green
            {{ -1.0f, -1.0f, -1.0f }, 0xFF446644 }, {{  1.0f, -1.0f,  1.0f }, 0xFF446644 }, {{ -1.0f, -1.0f,  1.0f }, 0xFF446644 },
            {{ -1.0f, -1.0f, -1.0f }, 0xFF446644 }, {{  1.0f, -1.0f, -1.0f }, 0xFF446644 }, {{  1.0f, -1.0f,  1.0f }, 0xFF446644 },
            // -Z: dark blue
            {{ -1.0f, -1.0f, -1.0f }, 0xFF664444 }, {{  1.0f,  1.0f, -1.0f }, 0xFF664444 }, {{  1.0f, -1.0f, -1.0f }, 0xFF664444 },
            {{ -1.0f, -1.0f, -1.0f }, 0xFF664444 }, {{ -1.0f,  1.0f, -1.0f }, 0xFF664444 }, {{  1.0f,  1.0f, -1.0f }, 0xFF664444 },
        };
    }

    inline static void get_torus (std::vector<vertex_pos_norm>& verts, std::vector<uint32_t>& indices, const int tessellation = 32, const float thickness = 0.333f, const float diameter = 1.0f) {
        assert (verts.size() == 0);
        assert (indices.size() == 0);
        assert (tessellation >= 3);
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

    inline static void get_teapot (std::vector<vertex_pos_norm>& verts, std::vector<uint32_t>& indices, const float size = 1.0f, const int tessellation = 8) {
        assert (verts.size() == 0);
        assert (indices.size() == 0);
        const std::vector<std::pair<bool, const std::vector<int>>> patches = {
            { true, { 102, 103, 104, 105, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }},
            { true, { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 }},
            { true, { 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 }},
            { true, { 96, 96, 96, 96, 97, 98, 99, 100, 101, 101, 101, 101, 0, 1, 2, 3 }},
            { true, { 0, 1, 2, 3, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117 }},
            { false, { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56 }},
            { false, { 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 28, 65, 66, 67 }},
            { false, { 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83 }},
            { false, { 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 }},
            { true, { 118, 118, 118, 118, 124, 122, 119, 121, 123, 126, 125, 120, 40, 39, 38, 37 }},
        };
        const std::vector<math::vector3> control_points = {
            { 0.0f, 0.345f, -0.05f }, { -0.028f, 0.345f, -0.05f }, { -0.05f, 0.345f, -0.028f },
            { -0.05f, 0.345f, 0.0f }, { 0.0f, 0.3028125f, -0.334375f }, { -0.18725f, 0.3028125f, -0.334375f },
            { -0.334375f, 0.3028125f, -0.18725f }, { -0.334375f, 0.3028125f, 0.0f }, { 0.0f, 0.3028125f, -0.359375f },
            { -0.20125f, 0.3028125f, -0.359375f }, { -0.359375f, 0.3028125f, -0.20125f },
            { -0.359375f, 0.3028125f, 0.0f }, { 0.0f, 0.27f, -0.375f }, { -0.21f, 0.27f, -0.375f },
            { -0.375f, 0.27f, -0.21f }, { -0.375f, 0.27f, 0.0f }, { 0.0f, 0.13875f, -0.4375f },
            { -0.245f, 0.13875f, -0.4375f }, { -0.4375f, 0.13875f, -0.245f }, { -0.4375f, 0.13875f, 0.0f },
            { 0.0f, 0.007499993f, -0.5f }, { -0.28f, 0.007499993f, -0.5f }, { -0.5f, 0.007499993f, -0.28f },
            { -0.5f, 0.007499993f, 0.0f }, { 0.0f, -0.105f, -0.5f }, { -0.28f, -0.105f, -0.5f },
            { -0.5f, -0.105f, -0.28f }, { -0.5f, -0.105f, 0.0f }, { 0.0f, -0.105f, 0.5f },
            { 0.0f, -0.2175f, -0.5f }, { -0.28f, -0.2175f, -0.5f }, { -0.5f, -0.2175f, -0.28f },
            { -0.5f, -0.2175f, 0.0f }, { 0.0f, -0.27375f, -0.375f }, { -0.21f, -0.27375f, -0.375f },
            { -0.375f, -0.27375f, -0.21f }, { -0.375f, -0.27375f, 0.0f }, { 0.0f, -0.2925f, -0.375f },
            { -0.21f, -0.2925f, -0.375f }, { -0.375f, -0.2925f, -0.21f }, { -0.375f, -0.2925f, 0.0f },
            { 0.0f, 0.17625f, 0.4f }, { -0.075f, 0.17625f, 0.4f }, { -0.075f, 0.2325f, 0.375f },
            { 0.0f, 0.2325f, 0.375f }, { 0.0f, 0.17625f, 0.575f }, { -0.075f, 0.17625f, 0.575f },
            { -0.075f, 0.2325f, 0.625f }, { 0.0f, 0.2325f, 0.625f }, { 0.0f, 0.17625f, 0.675f },
            { -0.075f, 0.17625f, 0.675f }, { -0.075f, 0.2325f, 0.75f }, { 0.0f, 0.2325f, 0.75f },
            { 0.0f, 0.12f, 0.675f }, { -0.075f, 0.12f, 0.675f }, { -0.075f, 0.12f, 0.75f },
            { 0.0f, 0.12f, 0.75f }, { 0.0f, 0.06375f, 0.675f }, { -0.075f, 0.06375f, 0.675f },
            { -0.075f, 0.007499993f, 0.75f }, { 0.0f, 0.007499993f, 0.75f }, { 0.0f, -0.04875001f, 0.625f },
            { -0.075f, -0.04875001f, 0.625f }, { -0.075f, -0.09562501f, 0.6625f }, { 0.0f, -0.09562501f, 0.6625f },
            { -0.075f, -0.105f, 0.5f }, { -0.075f, -0.18f, 0.475f }, { 0.0f, -0.18f, 0.475f },
            { 0.0f, 0.02624997f, -0.425f }, { -0.165f, 0.02624997f, -0.425f }, { -0.165f, -0.18f, -0.425f },
            { 0.0f, -0.18f, -0.425f }, { 0.0f, 0.02624997f, -0.65f }, { -0.165f, 0.02624997f, -0.65f },
            { -0.165f, -0.12375f, -0.775f }, { 0.0f, -0.12375f, -0.775f }, { 0.0f, 0.195f, -0.575f },
            { -0.0625f, 0.195f, -0.575f }, { -0.0625f, 0.17625f, -0.6f }, { 0.0f, 0.17625f, -0.6f },
            { 0.0f, 0.27f, -0.675f }, { -0.0625f, 0.27f, -0.675f }, { -0.0625f, 0.27f, -0.825f },
            { 0.0f, 0.27f, -0.825f }, { 0.0f, 0.28875f, -0.7f }, { -0.0625f, 0.28875f, -0.7f },
            { -0.0625f, 0.2934375f, -0.88125f }, { 0.0f, 0.2934375f, -0.88125f }, { 0.0f, 0.28875f, -0.725f },
            { -0.0375f, 0.28875f, -0.725f }, { -0.0375f, 0.298125f, -0.8625f }, { 0.0f, 0.298125f, -0.8625f },
            { 0.0f, 0.27f, -0.7f }, { -0.0375f, 0.27f, -0.7f }, { -0.0375f, 0.27f, -0.8f },
            { 0.0f, 0.27f, -0.8f }, { 0.0f, 0.4575f, 0.0f }, { 0.0f, 0.4575f, -0.2f },
            { -0.1125f, 0.4575f, -0.2f }, { -0.2f, 0.4575f, -0.1125f }, { -0.2f, 0.4575f, 0.0f },
            { 0.0f, 0.3825f, 0.0f }, { 0.0f, 0.27f, -0.35f }, { -0.196f, 0.27f, -0.35f },
            { -0.35f, 0.27f, -0.196f }, { -0.35f, 0.27f, 0.0f }, { 0.0f, 0.3075f, -0.1f },
            { -0.056f, 0.3075f, -0.1f }, { -0.1f, 0.3075f, -0.056f }, { -0.1f, 0.3075f, 0.0f },
            { 0.0f, 0.3075f, -0.325f }, { -0.182f, 0.3075f, -0.325f }, { -0.325f, 0.3075f, -0.182f },
            { -0.325f, 0.3075f, 0.0f }, { 0.0f, 0.27f, -0.325f }, { -0.182f, 0.27f, -0.325f },
            { -0.325f, 0.27f, -0.182f }, { -0.325f, 0.27f, 0.0f }, { 0.0f, -0.33f, 0.0f },
            { -0.1995f, -0.33f, -0.35625f }, { 0.0f, -0.31125f, -0.375f }, { 0.0f, -0.33f, -0.35625f },
            { -0.35625f, -0.33f, -0.1995f }, { -0.375f, -0.31125f, 0.0f }, { -0.35625f, -0.33f, 0.0f },
            { -0.21f, -0.31125f, -0.375f }, { -0.375f, -0.31125f, -0.21f },
        };
        assert(tessellation >= 1);
        const auto bezier_f = [](float p1, float p2, float p3, float p4, float t) {
            return p1 * (1 - t) * (1 - t) * (1 - t) +
                   p2 * 3 * t * (1 - t) * (1 - t) +
                   p3 * 3 * t * t * (1 - t) +
                   p4 * t * t * t;
        };
        const auto bezier_v3 = [&](math::vector3 p1, math::vector3 p2, math::vector3 p3, math::vector3 p4, float t) {
            return math::vector3 {
                bezier_f(p1.x, p2.x, p3.x, p4.x, t),
                bezier_f(p1.y, p2.y, p3.y, p4.y, t),
                bezier_f(p1.z, p2.z, p3.z, p4.z, t),
            };
        };
        const auto bezier_tangent_f = [](float p1, float p2, float p3, float p4, float t) {
            return p1 * (-1 + 2 * t - t * t) +
                   p2 * (1 - 4 * t + 3 * t * t) +
                   p3 * (2 * t - 3 * t * t) +
                   p4 * (t * t);
        };
        const auto bezier_tangent_v3 = [&](math::vector3 p1, math::vector3 p2, math::vector3 p3, math::vector3 p4, float t) {
            return math::vector3 {
                bezier_tangent_f(p1.x, p2.x, p3.x, p4.x, t),
                bezier_tangent_f(p1.y, p2.y, p3.y, p4.y, t),
                bezier_tangent_f(p1.z, p2.z, p3.z, p4.z, t),
            }.normalise ();
        };
        const auto create_patch_indices = [&](bool mirror) {
            int stride = tessellation + 1;
            for (int i = 0; i < tessellation; i++) {
                for (int j = 0; j < tessellation; j++) {
                    int index_arr[] = {
                        i * stride + j,
                        (i + 1) * stride + j + 1,
                        (i + 1) * stride + j,
                        i * stride + j,
                        i * stride + j + 1,
                        (i + 1) * stride + j + 1,
                    };
                    if (mirror)
                        std::reverse(std::begin(index_arr), std::end(index_arr));
                    for (int index : index_arr)
                        indices.emplace_back (verts.size() + index);
                }
            }
        };
        const auto create_patch_verticies = [&](std::array<math::vector3, 16>& patch, bool mirror) {
            for (int i = 0; i <= tessellation; i++) {
                const float ti = (float)i / tessellation;
                for (int j = 0; j <= tessellation; j++) {
                    const float tj = (float)j / tessellation;
                    const math::vector3 p1 = bezier_v3(patch[0], patch[1], patch[2], patch[3], ti);
                    const math::vector3 p2 = bezier_v3(patch[4], patch[5], patch[6], patch[7], ti);
                    const math::vector3 p3 = bezier_v3(patch[8], patch[9], patch[10], patch[11], ti);
                    const math::vector3 p4 = bezier_v3(patch[12], patch[13], patch[14], patch[15], ti);
                    const math::vector3 position = bezier_v3(p1, p2, p3, p4, tj);
                    const math::vector3 q1 = bezier_v3(patch[0], patch[4], patch[8], patch[12], tj);
                    const math::vector3 q2 = bezier_v3(patch[1], patch[5], patch[9], patch[13], tj);
                    const math::vector3 q3 = bezier_v3(patch[2], patch[6], patch[10], patch[14], tj);
                    const math::vector3 q4 = bezier_v3(patch[3], patch[7], patch[11], patch[15], tj);
                    const math::vector3 tangentA = bezier_tangent_v3(p1, p2, p3, p4, tj);
                    const math::vector3 tangentB = bezier_tangent_v3(q1, q2, q3, q4, ti);
                    math::vector3 normal = tangentA ^ tangentB;
                    if (normal.length() > 0.0001f) {
                        normal.normalise();
                        if (mirror)
                            normal = -normal;
                    }
                    else { // fix classic teapot's degenerate geometry
                        if (position.y > 0) normal = math::vector3::up;
                        else normal = math::vector3::down;
                    }
                    verts.emplace_back(vertex_pos_norm{position, normal});
                }
            }
        };
        const auto tessellate_patch = [&](const std::vector<int>& indices, const math::vector3& scale) {
            std::array<math::vector3, 16> cp_arr;
            for (int i = 0; i < cp_arr.size(); i++)
                cp_arr[i] = control_points[indices[i]] * scale;
            const bool mirror = sge::utils::sign (scale.x) != sge::utils::sign (scale.z);
            create_patch_indices (mirror);
            create_patch_verticies (cp_arr, mirror);
        };
        for (auto& patch : patches) {
            tessellate_patch (patch.second, math::vector3 { size, size, size });
            tessellate_patch (patch.second, math::vector3 { -size, size, size });
            if (patch.first) {
                tessellate_patch (patch.second, math::vector3 { size, size, -size });
                tessellate_patch (patch.second, math::vector3 { -size, size, -size });
            }
        }
    }
}

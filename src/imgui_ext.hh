#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_data.hh"

namespace imgui::ext {

void draw_user_triangles (
    const sge::math::rect& user_container,
    const bool user_container_relative_to_window,
    const sge::math::vector3& camera_position,
    const sge::math::quaternion& camera_orientation,
    const float camera_fov,
    const float camera_near,
    const float camera_far,
#if USE_SPAN
    const std::span<sge::data::vertex_pos_col> vertices,
    std::span<uint32_t> indices, // not const, these indices will be sorted!  allocate and pass in a copy if you don't want this.
#else
    const std::vector<sge::data::vertex_pos_col>& vertices,
    std::vector<uint32_t>& indices,
#endif
    const sge::math::vector3& obj_pos,
    const sge::math::quaternion& model_orientation,
    const bool lighting = false);

}

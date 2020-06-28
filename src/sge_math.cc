#include "sge_math.hh"

namespace sge::math {

const point2 point2::zero = point2 (0, 0);
const point2 point2::one = point2 (1.0f, 1);
const point2 point2::unit_x = point2 (1, 0);
const point2 point2::unit_y = point2 (0, 1);
    
const vector2 vector2::zero = vector2 (0.0f, 0.0f);
const vector2 vector2::one = vector2 (1.0f, 1.0f);
const vector2 vector2::unit_x = vector2 (1.0f, 0.0f);
const vector2 vector2::unit_y = vector2 (0.0f, 1.0f);

const vector3 vector3::zero = vector3 (0.0f, 0.0f, 0.0f);
const vector3 vector3::one = vector3 (1.0f, 1.0f, 1.0f);
const vector3 vector3::unit_x = vector3 (1.0f, 0.0f, 0.0f);
const vector3 vector3::unit_y = vector3 (0.0f, 1.0f, 0.0f);
const vector3 vector3::unit_z = vector3 (0.0f, 0.0f, 1.0f);
const vector3 vector3::right = vector3 (1.0f, 0.0f, 0.0f);
const vector3 vector3::up = vector3 (0.0f, 1.0f, 0.0f);
const vector3 vector3::backward = vector3 (0.0f, 0.0f, 1.0f);
const vector3 vector3::left = vector3 (-1.0f, 0.0f, 0.0f);
const vector3 vector3::down = vector3 (0.0f, -1.0f, 0.0f);
const vector3 vector3::forward = vector3 (0.0f, 0.0f, -1.0f);
    
const vector4 vector4::zero = vector4 (0.0f, 0.0f, 0.0f, 0.0f);
const vector4 vector4::one = vector4 (1.0f, 1.0f, 1.0f, 1.0f);
const vector4 vector4::unit_x = vector4 (1.0f, 0.0f, 0.0f, 0.0f);
const vector4 vector4::unit_y = vector4 (0.0f, 1.0f, 0.0f, 0.0f);
const vector4 vector4::unit_z = vector4 (0.0f, 0.0f, 1.0f, 0.0f);
const vector4 vector4::unit_w = vector4 (0.0f, 0.0f, 0.0f, 1.0f);
    
const quaternion quaternion::zero = quaternion (0.0f, 0.0f, 0.0f, 0.0f);
const quaternion quaternion::identity = quaternion (0.0f, 0.0f, 0.0f, 1.0f);

}

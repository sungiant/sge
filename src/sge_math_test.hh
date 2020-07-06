#pragma once

#include "sge_math.hh"

namespace sge::math::test {

struct framework {
    
framework () {
    
    { // cross product (right handed)
#define FN(a, b, ex) { assert ((a ^ b) == ex); }
        FN(vector3::up,       vector3::forward, vector3::left);  FN(vector3::up,       vector3::left, vector3::backward); FN(vector3::up,       vector3::backward, vector3::right); FN(vector3::up,       vector3::right, vector3::forward);
        FN(vector3::down,     vector3::forward, vector3::right); FN(vector3::down,     vector3::left, vector3::forward);  FN(vector3::down,     vector3::backward, vector3::left);  FN(vector3::down,     vector3::right, vector3::backward);
        FN(vector3::right,    vector3::forward, vector3::up);    FN(vector3::right,    vector3::down, vector3::forward);  FN(vector3::right,    vector3::backward, vector3::down);  FN(vector3::right,    vector3::up,    vector3::backward);
        FN(vector3::left,     vector3::forward, vector3::down);  FN(vector3::left,     vector3::down, vector3::backward); FN(vector3::left,     vector3::backward, vector3::up);    FN(vector3::left,     vector3::up,    vector3::forward);
        FN(vector3::backward, vector3::left,    vector3::down);  FN(vector3::backward, vector3::down, vector3::right);    FN(vector3::backward, vector3::right,    vector3::up);    FN(vector3::backward, vector3::up,    vector3::left);
        FN(vector3::forward,  vector3::left,    vector3::up);    FN(vector3::forward,  vector3::down, vector3::left);     FN(vector3::forward,  vector3::right,    vector3::down);  FN(vector3::forward,  vector3::up,    vector3::right);
#undef FN
    }
    { // fluent chains
         matrix44 view0 = matrix44()
            .set_rotation_component(quaternion::identity)
            .set_translation_component({2, 3, 4});

        matrix44 expected = matrix44::identity;
        expected.r3c0 = 2;
        expected.r3c1 = 3;
        expected.r3c2 = 4;
        assert (view0 == expected);
    }
    { // quaternion vector transformations
#define FN(ax, y, p, r, ex) { assert (ax * quaternion().set_from_yaw_pitch_roll(y, p, r) == ex); }
        FN (vector3::right, -TAU,          0, 0, vector3::right);
        FN (vector3::right, -3.0f*HALF_PI, 0, 0, vector3::forward);
        FN (vector3::right, -PI,           0, 0, vector3::left);
        FN (vector3::right, -HALF_PI,      0, 0, vector3::backward);
        FN (vector3::right, 0,             0, 0, vector3::right);
        FN (vector3::right, HALF_PI,       0, 0, vector3::forward);
        FN (vector3::right, PI,            0, 0, vector3::left);
        FN (vector3::right, 3.0f*HALF_PI,  0, 0, vector3::backward);
        FN (vector3::right, TAU,           0, 0, vector3::right);
        
        FN (vector3::right, 0, -TAU,          0, vector3::right);
        FN (vector3::right, 0, -3.0f*HALF_PI, 0, vector3::right);
        FN (vector3::right, 0, -PI,           0, vector3::right);
        FN (vector3::right, 0, -HALF_PI,      0, vector3::right);
        FN (vector3::right, 0, 0,             0, vector3::right);
        FN (vector3::right, 0, HALF_PI,       0, vector3::right);
        FN (vector3::right, 0, PI,            0, vector3::right);
        FN (vector3::right, 0, 3.0f*HALF_PI,  0, vector3::right);
        FN (vector3::right, 0, TAU,           0, vector3::right);
        
        FN (vector3::right, 0, 0, -TAU         , vector3::right);
        FN (vector3::right, 0, 0, -3.0f*HALF_PI, vector3::up);
        FN (vector3::right, 0, 0, -PI          , vector3::left);
        FN (vector3::right, 0, 0, -HALF_PI     , vector3::down);
        FN (vector3::right, 0, 0, 0            , vector3::right);
        FN (vector3::right, 0, 0, HALF_PI      , vector3::up);
        FN (vector3::right, 0, 0, PI           , vector3::left);
        FN (vector3::right, 0, 0, 3.0f*HALF_PI , vector3::down);
        FN (vector3::right, 0, 0, TAU          , vector3::right);
        
        FN (vector3::up, -TAU,          0, 0, vector3::up);
        FN (vector3::up, -3.0f*HALF_PI, 0, 0, vector3::up);
        FN (vector3::up, -PI,           0, 0, vector3::up);
        FN (vector3::up, -HALF_PI,      0, 0, vector3::up);
        FN (vector3::up, 0,             0, 0, vector3::up);
        FN (vector3::up, HALF_PI,       0, 0, vector3::up);
        FN (vector3::up, PI,            0, 0, vector3::up);
        FN (vector3::up, 3.0f*HALF_PI,  0, 0, vector3::up);
        FN (vector3::up, TAU,           0, 0, vector3::up);
        
        FN (vector3::up, 0, -TAU,          0, vector3::up);
        FN (vector3::up, 0, -3.0f*HALF_PI, 0, vector3::backward);
        FN (vector3::up, 0, -PI,           0, vector3::down);
        FN (vector3::up, 0, -HALF_PI,      0, vector3::forward);
        FN (vector3::up, 0, 0,             0, vector3::up);
        FN (vector3::up, 0, HALF_PI,       0, vector3::backward);
        FN (vector3::up, 0, PI,            0, vector3::down);
        FN (vector3::up, 0, 3.0f*HALF_PI,  0, vector3::forward);
        FN (vector3::up, 0, TAU,           0, vector3::up);
        
        FN (vector3::up, 0, 0, -TAU         , vector3::up);
        FN (vector3::up, 0, 0, -3.0f*HALF_PI, vector3::left);
        FN (vector3::up, 0, 0, -PI          , vector3::down);
        FN (vector3::up, 0, 0, -HALF_PI     , vector3::right);
        FN (vector3::up, 0, 0, 0            , vector3::up);
        FN (vector3::up, 0, 0, HALF_PI      , vector3::left);
        FN (vector3::up, 0, 0, PI           , vector3::down);
        FN (vector3::up, 0, 0, 3.0f*HALF_PI , vector3::right);
        FN (vector3::up, 0, 0, TAU          , vector3::up);
        
        FN (vector3::backward, -TAU,          0, 0, vector3::backward);
        FN (vector3::backward, -3.0f*HALF_PI, 0, 0, vector3::right);
        FN (vector3::backward, -PI,           0, 0, vector3::forward);
        FN (vector3::backward, -HALF_PI,      0, 0, vector3::left);
        FN (vector3::backward, 0,             0, 0, vector3::backward);
        FN (vector3::backward, HALF_PI,       0, 0, vector3::right);
        FN (vector3::backward, PI,            0, 0, vector3::forward);
        FN (vector3::backward, 3.0f*HALF_PI,  0, 0, vector3::left);
        FN (vector3::backward, TAU,           0, 0, vector3::backward);
        
        FN (vector3::backward, 0, -TAU,          0, vector3::backward);
        FN (vector3::backward, 0, -3.0f*HALF_PI, 0, vector3::down);
        FN (vector3::backward, 0, -PI,           0, vector3::forward);
        FN (vector3::backward, 0, -HALF_PI,      0, vector3::up);
        FN (vector3::backward, 0, 0,             0, vector3::backward);
        FN (vector3::backward, 0, HALF_PI,       0, vector3::down);
        FN (vector3::backward, 0, PI,            0, vector3::forward);
        FN (vector3::backward, 0, 3.0f*HALF_PI,  0, vector3::up);
        FN (vector3::backward, 0, TAU,           0, vector3::backward);
        
        FN (vector3::backward, 0, 0, -TAU         , vector3::backward);
        FN (vector3::backward, 0, 0, -3.0f*HALF_PI, vector3::backward);
        FN (vector3::backward, 0, 0, -PI          , vector3::backward);
        FN (vector3::backward, 0, 0, -HALF_PI     , vector3::backward);
        FN (vector3::backward, 0, 0, 0            , vector3::backward);
        FN (vector3::backward, 0, 0, HALF_PI      , vector3::backward);
        FN (vector3::backward, 0, 0, PI           , vector3::backward);
        FN (vector3::backward, 0, 0, 3.0f*HALF_PI , vector3::backward);
        FN (vector3::backward, 0, 0, TAU          , vector3::backward);
#undef FN
    }
    { // matrix33 vector transformations
        assert (vector3::right * matrix33().set_from_y_axis_angle(-TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_y_axis_angle(-3.0f*HALF_PI) == vector3::forward);
        assert (vector3::right * matrix33().set_from_y_axis_angle(-PI          ) == vector3::left);
        assert (vector3::right * matrix33().set_from_y_axis_angle(-HALF_PI     ) == vector3::backward);
        assert (vector3::right * matrix33().set_from_y_axis_angle(0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_y_axis_angle(HALF_PI      ) == vector3::forward);
        assert (vector3::right * matrix33().set_from_y_axis_angle(PI           ) == vector3::left);
        assert (vector3::right * matrix33().set_from_y_axis_angle(3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::right * matrix33().set_from_y_axis_angle(TAU          ) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_x_axis_angle(-TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(-3.0f*HALF_PI) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(-PI          ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(-HALF_PI     ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(HALF_PI      ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(PI           ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(3.0f*HALF_PI ) == vector3::right);
        assert (vector3::right * matrix33().set_from_x_axis_angle(TAU          ) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_z_axis_angle(-TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_z_axis_angle(-3.0f*HALF_PI) == vector3::up);
        assert (vector3::right * matrix33().set_from_z_axis_angle(-PI          ) == vector3::left);
        assert (vector3::right * matrix33().set_from_z_axis_angle(-HALF_PI     ) == vector3::down);
        assert (vector3::right * matrix33().set_from_z_axis_angle(0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_z_axis_angle(HALF_PI      ) == vector3::up);
        assert (vector3::right * matrix33().set_from_z_axis_angle(PI           ) == vector3::left);
        assert (vector3::right * matrix33().set_from_z_axis_angle(3.0f*HALF_PI ) == vector3::down);
        assert (vector3::right * matrix33().set_from_z_axis_angle(TAU          ) == vector3::right);
        
        assert (vector3::up * matrix33().set_from_y_axis_angle(-TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(-3.0f*HALF_PI) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(-PI          ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(-HALF_PI     ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(HALF_PI      ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(PI           ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(3.0f*HALF_PI ) == vector3::up);
        assert (vector3::up * matrix33().set_from_y_axis_angle(TAU          ) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_x_axis_angle(-TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_x_axis_angle(-3.0f*HALF_PI) == vector3::backward);
        assert (vector3::up * matrix33().set_from_x_axis_angle(-PI          ) == vector3::down);
        assert (vector3::up * matrix33().set_from_x_axis_angle(-HALF_PI     ) == vector3::forward);
        assert (vector3::up * matrix33().set_from_x_axis_angle(0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_x_axis_angle(HALF_PI      ) == vector3::backward);
        assert (vector3::up * matrix33().set_from_x_axis_angle(PI           ) == vector3::down);
        assert (vector3::up * matrix33().set_from_x_axis_angle(3.0f*HALF_PI ) == vector3::forward);
        assert (vector3::up * matrix33().set_from_x_axis_angle(TAU          ) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_z_axis_angle(-TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_z_axis_angle(-3.0f*HALF_PI) == vector3::left);
        assert (vector3::up * matrix33().set_from_z_axis_angle(-PI          ) == vector3::down);
        assert (vector3::up * matrix33().set_from_z_axis_angle(-HALF_PI     ) == vector3::right);
        assert (vector3::up * matrix33().set_from_z_axis_angle(0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_z_axis_angle(HALF_PI      ) == vector3::left);
        assert (vector3::up * matrix33().set_from_z_axis_angle(PI           ) == vector3::down);
        assert (vector3::up * matrix33().set_from_z_axis_angle(3.0f*HALF_PI ) == vector3::right);
        assert (vector3::up * matrix33().set_from_z_axis_angle(TAU          ) == vector3::up);
        
        assert (vector3::backward * matrix33().set_from_y_axis_angle(-TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(-3.0f*HALF_PI) == vector3::right);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(-PI          ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(-HALF_PI     ) == vector3::left);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(HALF_PI      ) == vector3::right);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(PI           ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(3.0f*HALF_PI ) == vector3::left);
        assert (vector3::backward * matrix33().set_from_y_axis_angle(TAU          ) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_x_axis_angle(-TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(-3.0f*HALF_PI) == vector3::down);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(-PI          ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(-HALF_PI     ) == vector3::up);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(HALF_PI      ) == vector3::down);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(PI           ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(3.0f*HALF_PI ) == vector3::up);
        assert (vector3::backward * matrix33().set_from_x_axis_angle(TAU          ) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_z_axis_angle(-TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(-3.0f*HALF_PI) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(-PI          ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(-HALF_PI     ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(HALF_PI      ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(PI           ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_z_axis_angle(TAU          ) == vector3::backward);
    }
    { // matrix33 vector transformations
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::forward);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::left);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::backward);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0,             0, 0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::forward);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::left);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::backward);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0,             0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, PI,            0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::up);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::left);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::down);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::up);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::left);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::down);
        assert (vector3::right * matrix33().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::right);
        
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0,             0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::backward);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::down);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::forward);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0,             0) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::backward);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, PI,            0) == vector3::down);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::forward);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::left);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::down);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::right);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::left);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::down);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::right);
        assert (vector3::up * matrix33().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::up);
        
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::right);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::left);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0,             0, 0) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::right);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::left);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::down);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::up);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0,             0) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::down);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, PI,            0) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::up);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::backward);
    }
    { // matrix33 vector transformations
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, -TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, -3.0f*HALF_PI) == vector3::forward);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, -PI          ) == vector3::left);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, -HALF_PI     ) == vector3::backward);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, 0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, HALF_PI      ) == vector3::forward);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, PI           ) == vector3::left);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, 3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::up, TAU          ) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, -TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, -3.0f*HALF_PI) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, -PI          ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, -HALF_PI     ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, 0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, HALF_PI      ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, PI           ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, 3.0f*HALF_PI ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::right, TAU          ) == vector3::right);
        
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, -TAU         ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, -3.0f*HALF_PI) == vector3::up);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, -PI          ) == vector3::left);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, -HALF_PI     ) == vector3::down);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, 0            ) == vector3::right);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, HALF_PI      ) == vector3::up);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, PI           ) == vector3::left);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, 3.0f*HALF_PI ) == vector3::down);
        assert (vector3::right * matrix33().set_from_axis_angle(vector3::backward, TAU          ) == vector3::right);
        
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, -TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, -3.0f*HALF_PI) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, -PI          ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, -HALF_PI     ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, 0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, HALF_PI      ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, PI           ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, 3.0f*HALF_PI ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::up, TAU          ) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, -TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, -3.0f*HALF_PI) == vector3::backward);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, -PI          ) == vector3::down);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, -HALF_PI     ) == vector3::forward);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, 0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, HALF_PI      ) == vector3::backward);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, PI           ) == vector3::down);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, 3.0f*HALF_PI ) == vector3::forward);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::right, TAU          ) == vector3::up);
        
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, -TAU         ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, -3.0f*HALF_PI) == vector3::left);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, -PI          ) == vector3::down);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, -HALF_PI     ) == vector3::right);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, 0            ) == vector3::up);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, HALF_PI      ) == vector3::left);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, PI           ) == vector3::down);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, 3.0f*HALF_PI ) == vector3::right);
        assert (vector3::up * matrix33().set_from_axis_angle(vector3::backward, TAU          ) == vector3::up);
        
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, -TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, -3.0f*HALF_PI) == vector3::right);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, -PI          ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, -HALF_PI     ) == vector3::left);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, 0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, HALF_PI      ) == vector3::right);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, PI           ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, 3.0f*HALF_PI ) == vector3::left);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::up, TAU          ) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, -TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, -3.0f*HALF_PI) == vector3::down);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, -PI          ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, -HALF_PI     ) == vector3::up);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, 0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, HALF_PI      ) == vector3::down);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, PI           ) == vector3::forward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, 3.0f*HALF_PI ) == vector3::up);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::right, TAU          ) == vector3::backward);
        
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, -TAU         ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, -3.0f*HALF_PI) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, -PI          ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, -HALF_PI     ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, 0            ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, HALF_PI      ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, PI           ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, 3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::backward * matrix33().set_from_axis_angle(vector3::backward, TAU          ) == vector3::backward);
    }
    {
        matrix43 m = matrix43().set_translation_component(vector3(10, -10, 4));
        vector4 v = { 1, 3, 4, 1 };
        vector3 res = v * m;
        assert (res == vector3(11, -7, 8));
    }
    {
        matrix43 m = matrix43().set_scale_component(vector3(10, -10, 4));
        vector4 v = { 1, 3, 4, 1 };
        vector3 res = v * m;
        assert (res == vector3(10, -30, 16));
    }
    {
        const auto q = matrix33().set_from_orientation(quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0));
        const auto m = matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0);
        assert (m == q);
    }
    {
        const matrix44 a = {-27,36,9,-54,36,3,9,9,9,9,-36,6,-24,9,36,-12};
        const matrix44 b = {3402,-1269,-2187,2484,-999,1467,351,-1971,-387,81,1674,-693,1584,-621,-1863,1737};
        assert (a * a == b);
    }
    {
        const vector3 cam_pos = { -7.0f, 0.0f, 9.0f };
        const vector3 point0 = { 2.0f, 0.0f, 0.0f };
        const vector3 point1 = { 8.0f, 0.0f, 0.0f };
        const vector3 point2 = { 3.0f, 0.0f, -5.0f };
        const vector3 point3 = { 3.0f, 0.0f, 7.0f };
        const vector3 point4 = { 15.0f, 0.0f, -13.0f };
        
        const matrix44 viewF =
            matrix44().set_rotation_component(matrix33().set_from_axis_angle(vector3::unit_y, -PI / 4.0f)) *
            matrix44().set_translation_component(cam_pos);
        
        const matrix44 viewM = inverse(viewF);

        const vector3 cam_pos_vs = viewM % cam_pos;

        const vector3 vs0 = viewM % point0;
        const vector3 vs1 = viewM % point1;
        const vector3 vs2 = viewM % point2;
        const vector3 vs3 = viewM % point3;
        const vector3 vs4 = viewM % point4;

        assert (cam_pos_vs == vector3::zero);
        assert (vs0 == vector3 (  0.0f,        0.0f, -12.7279234f));
        assert (vs1 == vector3 (  4.24264145f, 0.0f, -16.9705639f));
        assert (vs2 == vector3 (-2.82842779f,  0.0f, -16.9705639f));
        assert (vs3 == vector3 (  5.65685558f, 0.0f, - 8.48528289f));
        assert (vs4 == vector3 (  0.0f,        0.0f, -31.1127033f));
        /*
        assert (vs0 == vector3 (  0.0f,        0.0f, -12.7279253f));
        assert (vs1 == vector3 (  4.24264193f, 0.0f, -16.9705658f));
        assert (vs2 == vector3 (-2.82842779f,  0.0f, -16.9705658f));
        assert (vs3 == vector3 (  5.65685558f, 0.0f, - 8.48528289f));
        assert (vs4 == vector3 (  0.0f,        0.0f, -31.1127071f));
        */
        const matrix44 proj = matrix44().set_as_perspective_fov_rh(45.0f * DEG2RAD, 1.0f, -3.0f, -22.0f);
        
        const vector3 cam_pos_ndc = proj % cam_pos;

        const vector3 ndc0 = proj % vs0;
        const vector3 ndc1 = proj % vs1;
        const vector3 ndc2 = proj % vs2;
        const vector3 ndc3 = proj % vs3;
        const vector3 ndc4 = proj % vs4;
        
        //assert (cam_pos_ndc == vector3 (0.0f, 0.0f, 0.0f));
        //assert (r2 == expected2);
    }
    {
        const vector3 point = { 1.5f, 1.0f, 1.5f };
        const matrix44 m = matrix44().set_translation_component(point);
        const matrix44 rx = matrix44().set_rotation_component (matrix33().set_from_axis_angle(vector3::unit_x, PI));
        const matrix44 ry = matrix44().set_rotation_component (matrix33().set_from_axis_angle(vector3::unit_y, HALF_PI));
        const matrix44 expected = { 0, 0, 1, 0, 0, -1, 0, 0, 1, 0, 0, 0, 1.5f, 1.0f, 1.5f, 1 };
        const matrix44 res = ry * rx * m;
        assert (res == expected);
    }
    { // view matrix from look at
        const vector3 obj_pos = { 0, 0, 0 };
        const vector3 cam_pos = { 0, 0, 10 };

        const matrix44 viewF =
            matrix44().set_rotation_component(quaternion::identity) *
            matrix44().set_translation_component(cam_pos);
        
        const matrix44 viewM = inverse(viewF);
        
        const vector3 obj_pos_vs = viewM % obj_pos;
        const vector3 expected = vector3 { 0, 0, -10 };
        assert (obj_pos_vs == expected);
        
        const vector3 cam_pos_vs = viewM % cam_pos;
        const vector3 expected2 = vector3 { 0, 0, 0 };
        assert (cam_pos_vs == expected2);
    }
    { // view matrix
        const vector3 obj_pos = { 1, 2, 3 };
        const vector3 cam_pos = { 0, 0, 10 };
        const matrix44 view2world = matrix44().set_as_view_frame_from_look_at_target (cam_pos, obj_pos, vector3::up);
        const matrix44 world2view = matrix44().set_as_view_transform_from_look_at_target (cam_pos, obj_pos, vector3::up);
        assert (view2world != world2view);
        matrix44 view2world2 = world2view;
        matrix44 world2view2 = view2world;
        view2world2.inverse();
        world2view2.inverse();
        //assert (view2world == view2world2);
        //assert (world2view == world2view2);
    }
}
};

}

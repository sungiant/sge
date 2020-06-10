#pragma once

// SGE-MATH
// -------------------------------------
// Stand alone mathematics library.
// -------------------------------------

#include <cmath>

namespace sge::math {

const float PI = std::atan (1.0f) * 4.0f;
const float TAU = 2.0f * PI;
const float EPISILON = 0.000001f;

static bool is_zero (float f) { return (f >= -EPISILON) && (f <= EPISILON); }

struct point2 {
    int x = 0, y = 0;
    bool operator == (const point2& p) const { return x == p.x && y == p.y; }
    bool operator != (const point2& p) const { return !(*this == p); }
};

struct rect {
    point2 location;
    point2 extent;
    bool operator == (const rect& r) const { return location == r.location && extent == r.extent; }
    bool operator != (const rect& r) const { return !(*this == r); }
};

struct vector2 {
    float x, y = 0.0f;
    bool operator == (const vector2& v) const { return is_zero (x - v.x) && is_zero (y - v.y); }
    bool operator != (const vector2& v) const { return !(*this == v); }
};

struct vector3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    bool operator == (const vector3&v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z); }
    bool operator != (const vector3& v) const { return !(*this == v); }
};

struct vector4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    bool operator == (const vector4& v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z) && is_zero (w - v.w); }
    bool operator != (const vector4& v) const { return !(*this == v); }
};

struct quaternion {
    float i = 0.0f, j = 0.0f, k = 0.0f, u = 0.0f;
    bool operator == (const quaternion& v) const { return is_zero (i - v.i) && is_zero (j - v.j) && is_zero (k - v.k) && is_zero (u - v.u); }
    bool operator != (const quaternion& v) const { return !(*this == v); }

    static quaternion from_yaw_pitch_roll (float yaw, float pitch, float roll) {
        float y = yaw * 0.5f;
        float p = pitch * 0.5f;
        float r = roll * 0.5f;

        float sy = std::sin (y);
        float cy = std::cos (y);
        float sp = std::sin (p);
        float cp = std::cos (p);
        float sr = std::sin (r);
        float cr = std::cos (r);

        return {
            (cy * sp * cr) + (sy * cp * sr),//y
            (sy * cp * cr) - (cy * sp * sr),//z
            (cy * cp * sr) - (sy * sp * cr),//x
            (cy * cp * cr) + (sy * sp * sr) };//w
    }

    static vector3 to_yaw_pitch_roll (quaternion q) {
        vector3 angles;

        // roll (x-axis rotation)
        float sinr_cosp = 2 * (q.u * q.k + q.i * q.j);
        float cosr_cosp = 1 - 2 * (q.k * q.k + q.i * q.i);
        angles.z = std::atan2 (sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        float sinp = 2 * (q.u * q.i - q.j * q.k);
        if (std::abs (sinp) >= 1)
            angles.y = std::copysign (PI / 2, sinp); // use 90 degrees if out of range
        else
            angles.y = std::asin (sinp);

        // yaw (z-axis rotation)
        float siny_cosp = 2 * (q.u * q.j + q.k * q.i);
        float cosy_cosp = 1 - 2 * (q.i * q.i + q.j * q.j);
        angles.x = std::atan2 (siny_cosp, cosy_cosp);

        return angles;
    }
};

}

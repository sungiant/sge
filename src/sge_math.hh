// SGE-MATH - A.J.Pook
// ---------------------------------- //
// Stand alone mathematics library.
// ---------------------------------- //

#pragma once

#include "sge.hh"

namespace sge::math {

const float PI = atan (1.0f) * 4.0f;
const float TAU = 2.0f * PI;
const float EPSILON = 0.000001f;

inline bool is_zero (const float f) { return (f >= -EPSILON) && (f <= EPSILON); }
    
inline int max (int l, int r) { if (l > r) return l; else return r; }
inline float max (float l, float r) { if (l > r) return l; else return r; }
inline int min (int l, int r) { if (l < r) return l; else return r; }
inline float min (float l, float r) { if (l < r) return l; else return r; }

struct point2; struct rect; struct vector2; struct vector3; struct vector4; struct quaternion;

struct point2 {
    int x = 0, y = 0;
    
    point2 () = default;
    point2 (const int z_x, const int z_y) : x (z_x), y (z_y) {}
    
    bool operator == (const point2& p) const { return x == p.x && y == p.y; }
    bool operator != (const point2& p) const { return !(*this == p); }
    
    point2& operator+= (const point2& v) { x+=v.x; y+=v.y; return *this; }
    point2& operator+= (const int i) { x+=i; y+=i; return *this; }
    point2& operator-= (const point2& v) { x-=v.x; y-=v.y; return *this; }
    point2& operator-= (const int i) { x-=i; y-=i; return *this; }
    point2& operator*= (const point2& v) { x*=v.x; y*=v.y; return *this; }
    point2& operator*= (const int i) { x*=i; y*=i; return *this; }
    point2& operator/= (const point2& v) { x/=v.x; y/=v.y;; return *this; }
    point2& operator/= (const int i) { x/=i; y/=i; return *this; }
    
    static const point2 zero, one, unit_x, unit_y;
};

struct rect {
    point2 location;
    point2 extent;
    bool operator == (const rect& r) const { return location == r.location && extent == r.extent; }
    bool operator != (const rect& r) const { return !(*this == r); }
};

struct vector2 {
    float x, y = 0.0f;
    
    vector2 () = default;
    vector2 (const float z_x, const float z_y) : x (z_x), y (z_y) {}
    vector2 (const point2& v) : x (v.x), y (v.y) {}
    
    bool operator == (const vector2& v) const { return is_zero (x - v.x) && is_zero (y - v.y); }
    bool operator != (const vector2& v) const { return !(*this == v); }
    
    vector2& operator+= (const vector2& v) { x+=v.x; y+=v.y; return *this; }
    vector2& operator+= (const float f) { x+=f; y+=f; return *this; }
    vector2& operator-= (const vector2& v) { x-=v.x; y-=v.y; return *this; }
    vector2& operator-= (const float f) { x-=f; y-=f; return *this; }
    vector2& operator*= (const vector2& v) { x*=v.x; y*=v.y; return *this; }
    vector2& operator*= (const float f) { x*=f; y*=f; return *this; }
    vector2& operator/= (const vector2& v) { x/=v.x; y/=v.y;; return *this; }
    vector2& operator/= (const float f) { x/=f; y/=f; return *this; }
    
    float length () const { return sqrt (x*x + y*y); }
    float length_sq () const { return x*x + y*y; }
    vector2& normalise () { const float l = length (); x /= l; y /= l; return *this; }
    
    static const vector2 zero, one, unit_x, unit_y;
};

struct vector3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    vector3 () = default;
    vector3 (const float z_x, const float z_y, const float z_z) : x (z_x), y (z_y), z (z_z) {}
    
    bool operator== (const vector3&v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z); }
    bool operator!= (const vector3& v) const { return !(*this == v); }
    
    vector3& operator+= (const vector3& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    vector3& operator+= (const float f) { x+=f; y+=f; z+=f; return *this; }
    vector3& operator-= (const vector3& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
    vector3& operator-= (const float f) { x-=f; y-=f; z-=f; return *this; }
    vector3& operator*= (const vector3& v) { x*=v.x; y*=v.y; z*=v.z; return *this; }
    vector3& operator*= (const float f) { x*=f; y*=f; z*=f; return *this; }
    vector3& operator*= (const quaternion& q);
    vector3& operator/= (const vector3& v) { x/=v.x; y/=v.y; z/=v.z; return *this; }
    vector3& operator/= (const float f) { x/=f; y/=f; z/=f; return *this; }
    vector3& operator^= (const vector3& v) { // cross
        float const xx = y*v.z - z*v.y;
        float const yx = z*v.x - x*v.z;
        float const zx = x*v.y - y*v.x;
        x = xx; y = yx; z = zx; return *this;
    }
        
    float length () const { return sqrt (x*x + y*y + z*z); }
    float length_sq () const { return x*x + y*y + z*z; }
    vector3& normalise () { const float l = length (); x /= l; y /= l; z /= l; return *this; }
    
    static const vector3 zero, one, unit_x, unit_y, unit_z, unit_w, right, up, backward, left, down, forward;
};
    
struct vector4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    
    vector4 () = default;
    vector4 (const float z_x, const float z_y, const float z_z, const float z_w) : x (z_x), y (z_y), z (z_z), w (z_w) {}
    
    bool operator == (const vector4& v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z) && is_zero (w - v.w); }
    bool operator != (const vector4& v) const { return !(*this == v); }
    
    vector4& operator+= (const vector4& v) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
    vector4& operator+= (const float f) { x+=f; y+=f; z+=f; w+=f; return *this; }
    vector4& operator-= (const vector4& v) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; }
    vector4& operator-= (const float f) { x-=f; y-=f; z-=f; w-=f; return *this; }
    vector4& operator*= (const vector4& v) { x*=v.x; y*=v.y; z*=v.z; w*=v.w; return *this; }
    vector4& operator*= (const float f) { x*=f; y*=f; z*=f; w*=f; return *this; }
    vector4& operator/= (const vector4& v) { x/=v.x; y/=v.y; w/=v.w; return *this; }
    vector4& operator/= (const float f) { x/=f; y/=f; z/=f; w/=f; return *this; }
        
    float length () const { return sqrt (x*x + y*y + z*z + w*w); }
    float length_sq () const { return x*x + y*y + z*z + w*w; }
    vector4& normalise () { const float l = length (); x /= l; y /= l; z /= l; w /= l; return *this; }
    
    static const vector4 zero, one, unit_x, unit_y, unit_z, unit_w;
};

struct quaternion {
    float i = 0.0f, j = 0.0f, k = 0.0f; // vector
    float u = 0.0f; // scalar
    
    quaternion () = default;
    quaternion (const float z_i, const float z_j, const float z_k, const float z_u) : i (z_i), j (z_j), k (z_k), u (z_u) {}
    
    bool operator == (const quaternion& v) const { return is_zero (i - v.i) && is_zero (j - v.j) && is_zero (k - v.k) && is_zero (u - v.u); }
    bool operator != (const quaternion& v) const { return !(*this == v); }
    
    quaternion& operator+= (const quaternion& v) { i+=v.i; j+=v.j; k+=v.k; u+=v.u; return *this; }
    quaternion& operator+= (const float f) { i+=f; j+=f; k+=f; u+=f; return *this; }
    quaternion& operator-= (const quaternion& v) { i-=v.i; j-=v.j; k-=v.k; u-=v.u; return *this; }
    quaternion& operator-= (const float f) { i-=f; j-=f; k-=f; u-=f; return *this; }
    quaternion& operator*= (const float f) { i*=f; j*=f; k*=f; u*=f; return *this; }
    quaternion& operator/= (const float f) { i/=f; j/=f; k/=f; u/=f; return *this; }
    quaternion& operator&= (const quaternion& v) { // concatenate (chain rotations)
        const float ix = u*v.i + i*v.u + k*v.j - j*v.k;
        const float jx = u*v.j + j*v.u + i*v.k - k*v.i;
        const float kx = u*v.k + k*v.u + j*v.i - i*v.j;
        const float ux = u*v.u - k*v.k - i*v.i + j*v.j;
        i = ix; j = jx; k = kx; u = ux; return *this;
    }
    
    float length () const { return sqrt (i*i + j*j + k*k + u*u); }
    float length_sq () const { return i*i + j*j + k*k + u*u; }
    quaternion& normalise () { const float l = length (); i /= l; j /= l; k /= l; u /= l; return *this; }
    void get_axis_angle (vector3& axis, float& angle) const;
    void get_yaw_pitch_roll (vector3& angles) const;
    quaternion& set_axis_angle (const vector3& axis, const float angle);
    quaternion& set_yaw_pitch_roll (const float yaw, const float pitch, const float roll);
    
    static quaternion const zero, identity;
    static quaternion create_from_axis_angle (const vector3& axis, const float angle) { return quaternion ().set_axis_angle (axis, angle); }
    static quaternion create_from_yaw_pitch_roll (const float yaw, const float pitch, const float roll) { return quaternion ().set_yaw_pitch_roll (yaw, pitch, roll); }
};

// Point 2 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline point2 operator+(const point2& l, const point2& r) { return point2 { l.x + r.x, l.y + r.y }; }
inline point2 operator-(const point2& p) { return point2 { - p.x, - p.y }; }
inline point2 operator-(const point2& l, const point2& r) { return point2 { l.x - r.x, l.y - r.y }; }
inline point2 operator*(const point2& l, const point2& r) { return point2 { l.x * r.x, l.y * r.y }; }
inline point2 operator*(const point2& p, const int i) { return point2 { p.x * i, p.y * i }; }
inline vector2 operator*(const point2& p, const float f) { return vector2 { (float) p.x * f, (float) p.y * f }; }
inline point2 operator/(const point2& l, const point2& r) { return point2 { l.x / r.x, l.y / r.y }; }
inline point2 operator/(const point2& p, const int i) { return point2 { p.x / i, p.y / i }; }
inline vector2 operator/(const point2& p, const float f) { return vector2 { (float) p.x / f, (float) p.y / f }; }

inline float distance (const point2& l, const point2& r) { return sqrt ((float)((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y))); }
inline int manhattan_distance (const point2& l, const point2& r) { return (l.x-r.x) + (l.y-r.y); }
inline int chebyshev_distance (const point2& l, const point2& r) { return max (l.x-r.x, l.y-r.y); }

// Vector 2 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline vector2 operator+(const vector2& l, const vector2& r) { return vector2 { l.x + r.x, l.y + r.y }; }
inline vector2 operator-(const vector2& v) { return vector2 { - v.x, - v.y }; }
inline vector2 operator-(const vector2& l, const vector2& r) { return vector2 { l.x - r.x, l.y - r.y }; }
inline vector2 operator*(const vector2& l, const vector2& r) { return vector2 { l.x * r.x, l.y * r.y }; }
inline vector2 operator*(const vector2& v, const float f) { return vector2 { v.x * f, v.y * f }; }
inline vector2 operator/(const vector2& l, const vector2& r) { return vector2 { l.x / r.x, l.y / r.y }; }
inline vector2 operator/(const vector2& v, const float f) { return v * (1.0f / f); }
inline float operator|(const vector2& l, const vector2& r) { return l.x * r.x + l.y * r.y; } // dot

inline vector2 normalise (const vector2& v) { return v / v.length (); }
inline float distance (const vector2& l, const vector2& r) { return sqrt ((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y)); }

// Vector 3 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline vector3& vector3::operator*= (const quaternion& q) {
    float const xx = x - (2.0f * x * (q.j*q.j + q.k*q.k)) + (2.0f * y * (q.i*q.j - q.u*q.k)) + (2.0f * z * (q.i*q.k + q.u*q.j));
    float const yx = y + (2.0f * x * (q.i*q.j + q.u*q.k)) - (2.0f * y * (q.i*q.i + q.k*q.k)) + (2.0f * z * (q.j*q.k - q.u*q.i));
    float const zx = z + (2.0f * x * (q.i*q.k - q.u*q.j)) + (2.0f * y * (q.j*q.k + q.u*q.i)) - (2.0f * z * (q.i*q.i + q.j*q.j));
    x = xx; y = yx; z = zx; return *this;
}

inline vector3 operator+(const vector3& l, const vector3& r) { return vector3 { l.x + r.x, l.y + r.y, l.z + r.z }; }
inline vector3 operator-(const vector3& v) { return vector3 { - v.x, - v.y, - v.z }; }
inline vector3 operator-(const vector3& l, const vector3& r) { return vector3 { l.x - r.x, l.y - r.y, l.z - r.z }; }
inline vector3 operator*(const vector3& l, const vector3& r) { return vector3 { l.x * r.x, l.y * r.y, l.z * r.z }; }
inline vector3 operator*(const vector3& v, const float f) { return vector3 { v.x * f, v.y * f, v.z * f }; }
inline vector3 operator*(const vector3& v, const quaternion& q) { return vector3 {
    v.x - (2.0f * v.x * (q.j*q.j + q.k*q.k)) + (2.0f * v.y * (q.i*q.j - q.u*q.k)) + (2.0f * v.z * (q.i*q.k + q.u*q.j)),
    v.y + (2.0f * v.x * (q.i*q.j + q.u*q.k)) - (2.0f * v.y * (q.i*q.i + q.k*q.k)) + (2.0f * v.z * (q.j*q.k - q.u*q.i)),
    v.z + (2.0f * v.x * (q.i*q.k - q.u*q.j)) + (2.0f * v.y * (q.j*q.k + q.u*q.i)) - (2.0f * v.z * (q.i*q.i + q.j*q.j)) };
}
inline vector3 operator/(const vector3& l, const vector3& r) { return vector3 { l.x / r.x, l.y / r.y, l.z / r.z }; }
inline vector3 operator/(const vector3& v, const float f) { return v * (1.0f / f); }
inline float operator|(const vector3& l, const vector3& r) { return l.x * r.x + l.y * r.y + l.z * r.z; } // dot
inline vector3 operator^(const vector3& l, const vector3& r) { return vector3 { l.y*r.z - l.z*r.y, l.z*r.x - l.x*r.z, l.x*r.y - l.y*r.x }; } // cross

inline vector3 normalise (const vector3& v) { return v / v.length (); }
inline float distance (const vector3& l, const vector3& r) { return sqrt ((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y) + (l.z-r.z)*(l.z-r.z)); }

// Vector 4 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline vector4 operator+(const vector4& l, const vector4& r) { return vector4 { l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w }; }
inline vector4 operator-(const vector4& v) { return vector4 { - v.x, - v.y, - v.z, -v.w }; }
inline vector4 operator-(const vector4& l, const vector4& r) { return vector4 { l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w }; }
inline vector4 operator*(const vector4& l, const vector4& r) { return vector4 { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
inline vector4 operator*(const vector4& v, const float f) { return vector4 { v.x * f, v.y * f, v.z * f, v.w * f }; }
inline vector4 operator/(const vector4& l, const vector4& r) { return vector4 { l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w }; }
inline vector4 operator/(const vector4& v, const float f) { return v * (1.0f / f); }
inline float operator|(const vector4& l, const vector4& r) { return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w; } // dot

inline vector4 normalise (const vector4& v) { return v / v.length (); }
inline float distance (const vector4& l, const vector4& r) { return sqrt ((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y) + (l.z-r.z)*(l.z-r.z) + (l.w-r.w)*(l.w-r.w)); }

// Quaternion inline
// ------------------------------------------------------------------------------------------------------------------ //
inline quaternion operator~(const quaternion& v) { return quaternion { - v.i, - v.j, - v.k, v.u }; } // conjugate (inverse)
inline quaternion operator&(const quaternion& l, const quaternion& r) { return quaternion { // concatenate (chain rotations)
    l.u*r.i + l.i*r.u + l.k*r.j - l.j*r.k, l.u*r.j + l.j*r.u + l.i*r.k - l.k*r.i,
    l.u*r.k + l.k*r.u + l.j*r.i - l.i*r.j, l.u*r.u - l.k*r.k - l.i*r.i + l.j*r.j };
}
inline quaternion operator+(const quaternion& l, const quaternion& r) { return quaternion { l.i + r.i, l.j + r.j, l.k + r.k, l.u + r.u }; }
inline quaternion operator-(const quaternion& v) { return quaternion { - v.i, - v.j, - v.k, -v.u }; }
inline quaternion operator-(const quaternion& l, const quaternion& r) { return quaternion { l.i - r.i, l.j - r.j, l.k - r.k, l.u - r.u }; }
inline quaternion operator*(const quaternion& q, const float f) { return quaternion { q.i * f, q.j * f, q.k * f, q.u * f }; }
inline quaternion operator/(const quaternion& q, const float f) { return q * (1.0f / f); }
inline float operator|(const quaternion& l, const quaternion& r) { return l.i * r.i + l.j * r.j + l.k * r.k + l.u * r.u; } // dot

inline quaternion normalise (const quaternion& v) { return v / v.length (); }

inline void quaternion::get_axis_angle(vector3& axis, float& angle) const {
    const float cos_angle = u;
    angle = acos(cos_angle);
    float sin_angle = sqrt(1.0f - cos_angle * cos_angle);
    if (fabs(sin_angle) < EPSILON)
        sin_angle = 1.0f;
    axis.x = i / sin_angle;
    axis.y = j / sin_angle;
    axis.z = i / sin_angle;
}

inline void quaternion::get_yaw_pitch_roll (vector3& angles) const {
    // roll (x-axis rotation)
    const float sinr_cosp = 2 * (u * k + i * j);
    const float cosr_cosp = 1 - 2 * (k * k + i * i);
    angles.z = atan2 (sinr_cosp, cosr_cosp);
    // pitch (y-axis rotation)
    const float sinp = 2 * (u * i - j * k);
    if (abs (sinp) >= 1)
        angles.y = copysign (PI / 2, sinp); // use 90 degrees if out of range
    else
        angles.y = asin (sinp);
    // yaw (z-axis rotation)
    const float siny_cosp = 2 * (u * j + k * i);
    const float cosy_cosp = 1 - 2 * (i * i + j * j);
    angles.x = atan2 (siny_cosp, cosy_cosp);
}

inline quaternion& quaternion::set_axis_angle(const vector3& axis, const float angle) {
    const float half_angle = angle / 2.0f;
    const float s = sin(half_angle);
    const float c = cos(half_angle);
    i = s * axis.x;
    j = s * axis.y;
    k = s * axis.z;
    u = c;
    return *this;
}

inline quaternion& quaternion::set_yaw_pitch_roll (const float yaw, const float pitch, const float roll) {
    const float y = yaw * 0.5f;
    const float p = pitch * 0.5f;
    const float r = roll * 0.5f;
    const float sy = sin (y);
    const float cy = cos (y);
    const float sp = sin (p);
    const float cp = cos (p);
    const float sr = sin (r);
    const float cr = cos (r);
    i = (cy * sp * cr) + (sy * cp * sr);
    j = (sy * cp * cr) - (cy * sp * sr);
    k = (cy * cp * sr) - (sy * sp * cr);
    u = (cy * cp * cr) + (sy * sp * sr);
    return *this;
}

}

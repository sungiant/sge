// ===================
//  |@@@@-----@|--@@|
//  |@@@-----@@|--@@|
//  |@@-----@@@|--@@|
//  |@-----@@@@|--@@|
//  |@@@@@-----|@--@|
//  |@@@@-----@|@--@|
// ===================
// SGE-MATH - A.J.Pook
// ---------------------------------- //
// Stand alone mathematics library.
//
//    +Y      based on a right handed
//     |           coordinate system:
//     |
//     |______ +X     * X: right
//    /               * Y: up
//   /                * Z: backwards
// +Z
// ---------------------------------- //
// Angles of rotation are measured
// anti-clockwise when viewed from the
// rotation axis (positive side) toward
// the origin.  Right handed rule for
// rotations - thumb pointing along the
// axis of rotation, away from the
// origin.
// ---------------------------------- //
// Cameras are aligned with the main
// coordinate system and look in the
// direction of their negative Z axis
// (just like OpenGL).
// ---------------------------------- //
// Near and far planes defined as
// their Z positions in view-space, not
// as their distance from the camera
// in view space.  This means they will
// be negative in the general case.
// ---------------------------------- //
// Normalised device coordinates are
// inline with the main coordinate
// system and range from -1 to 1 on
// all axes.
// ---------------------------------- //
#pragma once

#include "sge.hh" // (dependencies: math.h, assert.h)

namespace sge::math {
    
const float HALF_PI = 1.5707963267948966192313216916398f;
const float PI      = 3.1415926535897932384626433832795f;
const float TAU     = 6.2831853071795864769252867665590f;
const float EPSILON = 1.0e-6f; // FLT_EPSILON;
const float RAD2DEG = 180.f / PI;
const float DEG2RAD = PI / 180.f;

inline bool is_zero (const float f) { return (f >= -EPSILON) && (f <= EPSILON); }
    
inline int max (int l, int r) { if (l > r) return l; else return r; }
inline float max (float l, float r) { if (l > r) return l; else return r; }
inline int min (int l, int r) { if (l < r) return l; else return r; }
inline float min (float l, float r) { if (l < r) return l; else return r; }

struct point2; struct rect; struct vector2; struct vector3; struct vector4; struct quaternion; struct matrix33; struct matrix43; struct matrix44;

// ------------------------------------------------------------------------------------------------------------------ //
struct point2 {
    int x = 0, y = 0;
    
    point2 () = default;
    point2 (const int z_x, const int z_y) : x (z_x), y (z_y) {}
    
    bool operator == (const point2& p) const { return x == p.x && y == p.y; }
    bool operator != (const point2& p) const { return !(*this == p); }

          int& operator[] (const int i)       { assert (i >=0 && i <= 1); return (&x)[i]; }
    const int& operator[] (const int i) const { assert (i >=0 && i <= 1); return (&x)[i]; }
    
    point2& operator+= (const point2& v) { x+=v.x; y+=v.y; return *this; }
    point2& operator+= (const int i)     { x+=i;   y+=i;   return *this; }
    point2& operator-= (const point2& v) { x-=v.x; y-=v.y; return *this; }
    point2& operator-= (const int i)     { x-=i;   y-=i;   return *this; }
    point2& operator*= (const point2& v) { x*=v.x; y*=v.y; return *this; }
    point2& operator*= (const int i)     { x*=i;   y*=i;   return *this; }
    point2& operator/= (const point2& v) { x/=v.x; y/=v.y; return *this; }
    point2& operator/= (const int i)     { x/=i;   y/=i;   return *this; }
    
    float  length       () const { return sqrt (x*x + y*y); }
    float  length_sq    () const { return x*x + y*y; }
    point2& negate      ()       { x=-x;y=-y; return *this; }
    
    float  distance     (const point2& v) const { return sqrt ((x-v.x)*(x-v.x) + (y-v.y)*(y-v.y)); }
    float  distance_sq  (const point2& v) const { return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y); }
    
    int manhattan_distance (const point2& v) const { return (x-v.x) + (y-v.y); }
    int chebyshev_distance (const point2& v) const { return max (x-v.x, y-v.y); }
    
    static const point2 zero, one, unit_x, unit_y;
};

// ------------------------------------------------------------------------------------------------------------------ //
struct rect {
    point2 location;
    point2 extent;
    bool operator == (const rect& r) const { return location == r.location && extent == r.extent; }
    bool operator != (const rect& r) const { return !(*this == r); }
};

// ------------------------------------------------------------------------------------------------------------------ //
struct vector2 {
    float x, y = 0.0f;
    
    vector2 () = default;
    vector2 (const float z_x, const float z_y) : x (z_x), y (z_y) {}
    vector2 (const point2& v) : x (v.x), y (v.y) {}
    
    bool operator == (const vector2& v) const { return is_zero (x - v.x) && is_zero (y - v.y); }
    bool operator != (const vector2& v) const { return !(*this == v); }
    
          float& operator[] (const int i)       { assert (i >=0 && i <= 1); return (&x)[i]; }
    const float& operator[] (const int i) const { assert (i >=0 && i <= 1); return (&x)[i]; }
    
    vector2& operator+= (const vector2& v) { x+=v.x; y+=v.y; return *this; }
    vector2& operator+= (const float f)    { x+=f;   y+=f;   return *this; }
    vector2& operator-= (const vector2& v) { x-=v.x; y-=v.y; return *this; }
    vector2& operator-= (const float f)    { x-=f;   y-=f;   return *this; }
    vector2& operator*= (const vector2& v) { x*=v.x; y*=v.y; return *this; }
    vector2& operator*= (const float f)    { x*=f;   y*=f;   return *this; }
    vector2& operator/= (const vector2& v) { x/=v.x; y/=v.y; return *this; }
    vector2& operator/= (const float f)    { x/=f;   y/=f;   return *this; }
    
    bool   is_unit      () const { return is_zero (1.0f - x*x - y*y); }
    float  length       () const { return sqrt (x*x + y*y); }
    float  length_sq    () const { return x*x + y*y; }
    
    float  distance     (const vector2& v) const { return sqrt ((x-v.x)*(x-v.x) + (y-v.y)*(y-v.y)); }
    float  distance_sq  (const vector2& v) const { return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y); }
    float  dot          (const vector2& v) const { return x * v.x + y * v.y; }
    
    vector2& normalise  ()       { const float l = length (); x /= l; y /= l; return *this; }
    vector2& negate     ()       { x=-x;y=-y; return *this; }
    
    static const vector2 zero, one, unit_x, unit_y;
};

// ------------------------------------------------------------------------------------------------------------------ //
struct vector3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    vector3 () = default;
    vector3 (const float z_x, const float z_y, const float z_z) : x (z_x), y (z_y), z (z_z) {}
    vector3 (const vector2& z_v, const float z_z) : x (z_v.x), y (z_v.y), z (z_z){}
    
    bool operator== (const vector3& v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z); }
    bool operator!= (const vector3& v) const { return !(*this == v); }

          float& operator[] (const int i)       { assert (i >=0 && i <= 2); return (&x)[i]; }
    const float& operator[] (const int i) const { assert (i >=0 && i <= 2); return (&x)[i]; }
    
    vector3& operator+= (const vector3& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    vector3& operator+= (const float f)    { x+=f;   y+=f;   z+=f;   return *this; }
    vector3& operator-= (const vector3& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
    vector3& operator-= (const float f)    { x-=f;   y-=f;   z-=f;   return *this; }
    vector3& operator*= (const vector3& v) { x*=v.x; y*=v.y; z*=v.z; return *this; }
    vector3& operator*= (const float f)    { x*=f;   y*=f;   z*=f;   return *this; }
    vector3& operator*= (const quaternion&);
    vector3& operator*= (const matrix33&);
    vector3& operator/= (const vector3& v) { x/=v.x; y/=v.y; z/=v.z; return *this; }
    vector3& operator/= (const float f)    { x/=f;   y/=f;   z/=f;   return *this; }
    
    bool   is_unit      () const { return is_zero (1.0f - x*x - y*y - z*z); }
    float  length       () const { return sqrt (x*x + y*y + z*z); }
    float  length_sq    () const { return x*x + y*y + z*z; }
    
    float  distance     (const vector3& v) const { return sqrt ((x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z)); }
    float  distance_sq  (const vector3& v) const { return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z); }
    float  dot          (const vector3& v) const { return x * v.x + y * v.y + z * v.z; }

    vector3& normalise  ()       { const float l = length (); x /= l; y /= l; z /= l; return *this; }
    vector3& negate     ()       { x=-x;y=-y;z=-z; return *this; }
    vector3& cross      (const vector3& v) { const vector3 t = *this; x=t.y*v.z-t.z*v.y;y=t.z*v.x-t.x*v.z;z=t.x*v.y-t.y*v.x; return *this; }
    
    static const vector3 zero, one, unit_x, unit_y, unit_z, unit_w, right, up, backward, left, down, forward;
};
    
// ------------------------------------------------------------------------------------------------------------------ //
struct vector4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    
    vector4 () = default;
    vector4 (const float z_x, const float z_y, const float z_z, const float z_w) : x (z_x), y (z_y), z (z_z), w (z_w) {}
    vector4 (const vector3& z_v, const float z_w) : x (z_v.x), y (z_v.y), z (z_v.z), w (z_w) {}
    
    bool operator == (const vector4& v) const { return is_zero (x - v.x) && is_zero (y - v.y) && is_zero (z - v.z) && is_zero (w - v.w); }
    bool operator != (const vector4& v) const { return !(*this == v); }

          float& operator[] (const int i)       { assert (i >=0 && i <= 3); return (&x)[i]; }
    const float& operator[] (const int i) const { assert (i >=0 && i <= 3); return (&x)[i]; }
    
    const vector3& xyz () const { return *reinterpret_cast<const vector3*>(this); }
    
    vector4& operator+= (const vector4& v) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
    vector4& operator+= (const float f)    { x+=f;   y+=f;   z+=f;   w+=f;   return *this; }
    vector4& operator-= (const vector4& v) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; }
    vector4& operator-= (const float f)    { x-=f;   y-=f;   z-=f;   w-=f;   return *this; }
    vector4& operator*= (const vector4& v) { x*=v.x; y*=v.y; z*=v.z; w*=v.w; return *this; }
    vector4& operator*= (const float f)    { x*=f;   y*=f;   z*=f;   w*=f;   return *this; }
    vector4& operator*= (const matrix44&);
    vector4& operator/= (const vector4& v) { x/=v.x; y/=v.y; z/=v.z; w/=v.w; return *this; }
    vector4& operator/= (const float f)    { x/=f;   y/=f;   z/=f;   w/=f;   return *this; }
    
    bool   is_unit      () const { return is_zero (1.0f - x*x - y*y - z*z - w*w); }
    float  length       () const { return sqrt (x*x + y*y + z*z + w*w); }
    float  length_sq    () const { return x*x + y*y + z*z + w*w; }
    
    vector4& normalise  ()       { const float l = length (); x /= l; y /= l; z /= l; w /= l; return *this; }
    vector4& negate     ()       { x=-x;y=-y;z=-z;w=-w; return *this; }
    
    float    dot        (const vector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    
    static const vector4 zero, one, unit_x, unit_y, unit_z, unit_w;
};

// ------------------------------------------------------------------------------------------------------------------ //
struct quaternion {
    float i = 0.0f, j = 0.0f, k = 0.0f; // vector
    float u = 1.0f; // scalar
    
    quaternion () = default;
    quaternion (const float z_i, const float z_j, const float z_k, const float z_u) : i (z_i), j (z_j), k (z_k), u (z_u) {}
    
    bool operator == (const quaternion& v) const { return is_zero (i - v.i) && is_zero (j - v.j) && is_zero (k - v.k) && is_zero (u - v.u); }
    bool operator != (const quaternion& v) const { return !(*this == v); }
    
    quaternion& operator+= (const quaternion& v) { i+=v.i; j+=v.j; k+=v.k; u+=v.u; return *this; }
    quaternion& operator+= (const float f)       { i+=f;   j+=f;   k+=f;   u+=f;   return *this; }
    quaternion& operator-= (const quaternion& v) { i-=v.i; j-=v.j; k-=v.k; u-=v.u; return *this; }
    quaternion& operator-= (const float f)       { i-=f;   j-=f;   k-=f;   u-=f;   return *this; }
    quaternion& operator*= (const float f)       { i*=f;   j*=f;   k*=f;   u*=f;   return *this; }
    quaternion& operator/= (const float f)       { i/=f;   j/=f;   k/=f;   u/=f;   return *this; }
    
    bool        is_unit   () const { return is_zero (1.0f - i*i - j*j - k*k - u*u); }
    float       length    () const { return sqrt (i*i + j*j + k*k + u*u); }
    float       length_sq () const { return i*i + j*j + k*k + u*u; }

    quaternion& normalise ()       { const float l = length (); i /= l; j /= l; k /= l; u /= l; return *this; }
    quaternion& negate    ()       { i=-i;j=-j;k=-k;u=-u; return *this; }
    quaternion& conjugate ()       { i=-i;j=-j;k=-k; return *this; } // inverse
    quaternion& concatenate (const quaternion&);
    
    quaternion  normalise   () const { quaternion cp = *this; return cp.normalise(); }
    quaternion  negate      () const { quaternion cp = *this; return cp.negate(); }
    quaternion  conjugate   () const { quaternion cp = *this; return cp.conjugate(); }
    
    vector3&    rotate (vector3&) const;
    vector3     rotate (const vector3& v) const { vector3 res = v; rotate (res); return res; }
    
    void        get_axis_angle (vector3&, float&) const;
    void        get_yaw_pitch_roll (vector3&) const;
    
    quaternion& set_from_axis_angle (const vector3&, const float);
    quaternion& set_from_yaw_pitch_roll (const float, const float, const float);
    quaternion& set_from_rotation (const matrix33&);
    
    static quaternion const zero, identity;
    
};

// ------------------------------------------------------------------------------------------------------------------ //
struct matrix33 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f,
          r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f,
          r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f;
    matrix33 () = default;
    matrix33 (float z_r0c0, float z_r0c1, float z_r0c2, float z_r1c0, float z_r1c1, float z_r1c2, float z_r2c0, float z_r2c1, float z_r2c2) : r0c0(z_r0c0), r0c1(z_r0c1), r0c2(z_r0c2), r1c0(z_r1c0), r1c1(z_r1c1), r1c2(z_r1c2), r2c0(z_r2c0), r2c1(z_r2c1), r2c2(z_r2c2) {}
    matrix33 (vector3 z_r0, vector3 z_r1, vector3 z_r2, vector3 z_r3) : r0c0(z_r0[0]), r0c1(z_r0[1]), r0c2(z_r0[2]), r1c0(z_r1[0]), r1c1(z_r1[1]), r1c2(z_r1[2]), r2c0(z_r2[0]), r2c1(z_r2[1]), r2c2(z_r2[2]) {}
    
    bool operator == (const matrix33& m) const { return (*this)[0] == m[0] && (*this)[1] == m[1] && (*this)[2] == m[2]; }
    bool operator != (const matrix33& m) const { return !(*this == m); }

          vector3& operator[] (const int i)       { assert (i >=0 && i <= 2); return reinterpret_cast<vector3*>(&r0c0)[i]; }
    const vector3& operator[] (const int i) const { assert (i >=0 && i <= 2); return reinterpret_cast<const vector3*>(&r0c0)[i]; }
    
          vector3& right   ()       { return (*this)[0]; }
          vector3& up      ()       { return (*this)[1]; }
          vector3& backward()       { return (*this)[2]; }
    const vector3& right   () const { return (*this)[0]; }
    const vector3& up      () const { return (*this)[1]; }
    const vector3& backward() const { return (*this)[2]; }
          vector3  left    () const { return vector3 (-r0c0, -r0c1, -r0c2); }
          vector3  down    () const { return vector3 (-r1c0, -r1c1, -r1c2); }
          vector3  forward () const { return vector3 (-r2c0, -r2c1, -r2c2); }
    
    matrix33& operator+= (const matrix33& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; return *this; }
    matrix33& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    return *this; }
    matrix33& operator-= (const matrix33& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; return *this; }
    matrix33& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    return *this; }
    matrix33& operator*= (const float f)     { (*this)[0]*=f;    (*this)[1]*=f;    (*this)[2]*=f;    return *this; }
    matrix33& operator*= (const matrix33& v) { product (*this, v, *this); return *this;  }
    matrix33& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    matrix33& orthonormalise ();
    matrix33& transpose      ();
    matrix33& negate         ()       { (*this)[0]=(*this)[0].negate(); (*this)[1]=(*this)[1].negate(); (*this)[2]=(*this)[2].negate(); return *this; }
    
    bool      is_orthonormal () const;
    float     determinant    () const { return r0c0*(r1c1*r2c2 - r1c2*r2c1) - r0c1*(r1c0*r2c2 - r1c2*r2c0) + r0c2*(r1c0*r2c1 - r1c1*r2c0); }
    matrix33  orthonormalise () const { matrix33 cp = *this; return cp.orthonormalise(); }
    matrix33  transpose      () const { matrix33 cp = *this; return cp.transpose(); }
    matrix33  negate         () const { matrix33 cp = *this; return cp.negate(); }
    
    matrix33& set_from_scale_factors (const vector3&);
    matrix33& set_from_x_axis_angle (const float);
    matrix33& set_from_y_axis_angle (const float);
    matrix33& set_from_z_axis_angle (const float);
    matrix33& set_from_yaw_pitch_roll (const float, const float, const float);
    matrix33& set_from_axis_angle (const vector3&, const float);
    matrix33& set_from_orientation (const quaternion&);
    matrix33& set_from_transform (const matrix43&);
    matrix33& set_from_transform (const matrix44&);
    
    static matrix33 const zero, identity;
    
    static void product (const matrix33&, const matrix33&, matrix33&); // [3x3] * [3x3] => [3x3]
    static void product (const vector3& , const matrix33&, vector3& ); // [1x3] * [3x3] => [1x3]
    static void product (const matrix33&, const vector3& , vector3& ); // [3x3] * [3x1] => [3x1]
};

// ------------------------------------------------------------------------------------------------------------------ //
struct matrix43 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f,
          r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f,
          r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f,
          r3c0 = 0.0f, r3c1 = 0.0f, r3c2 = 0.0f;
    matrix43 () = default;
    matrix43 (float z_r0c0, float z_r0c1, float z_r0c2, float z_r1c0, float z_r1c1, float z_r1c2, float z_r2c0, float z_r2c1, float z_r2c2, float z_r3c0, float z_r3c1, float z_r3c2) : r0c0(z_r0c0), r0c1(z_r0c1), r0c2(z_r0c2), r1c0(z_r1c0), r1c1(z_r1c1), r1c2(z_r1c2), r2c0(z_r2c0), r2c1(z_r2c1), r2c2(z_r2c2), r3c0(z_r3c0), r3c1(z_r3c1), r3c2(z_r3c2) {}
    matrix43 (vector3 z_r0, vector3 z_r1, vector3 z_r2, vector3 z_r3) : r0c0(z_r0[0]), r0c1(z_r0[1]), r0c2(z_r0[2]), r1c0(z_r1[0]), r1c1(z_r1[1]), r1c2(z_r1[2]), r2c0(z_r2[0]), r2c1(z_r2[1]), r2c2(z_r2[2]), r3c0(z_r3[0]), r3c1(z_r3[1]), r3c2(z_r3[2]) {}
    
    bool operator == (const matrix43& m) const { return (*this)[0] == m[0] && (*this)[1] == m[1] && (*this)[2] == m[2] && (*this)[3] == m[3]; }
    bool operator != (const matrix43& m) const { return !(*this == m); }

          vector3& operator[] (const int i)       { assert (i >=0 && i <= 3); return reinterpret_cast<vector3*>(&r0c0)[i]; }
    const vector3& operator[] (const int i) const { assert (i >=0 && i <= 3); return reinterpret_cast<const vector3*>(&r0c0)[i]; }
    
          vector3& right   ()       { return (*this)[0]; }
          vector3& up      ()       { return (*this)[1]; }
          vector3& backward()       { return (*this)[2]; }
    const vector3& right   () const { return (*this)[0]; }
    const vector3& up      () const { return (*this)[1]; }
    const vector3& backward() const { return (*this)[2]; }
          vector3  left    () const { return vector3 (-r0c0, -r0c1, -r0c2); }
          vector3  down    () const { return vector3 (-r1c0, -r1c1, -r1c2); }
          vector3  forward () const { return vector3 (-r2c0, -r2c1, -r2c2); }
    
    matrix43& operator+= (const matrix43& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; (*this)[3]+=v[3]; return *this; }
    matrix43& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    (*this)[3]+=f;    return *this; }
    matrix43& operator-= (const matrix43& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; (*this)[3]-=v[3]; return *this; }
    matrix43& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    (*this)[3]-=f;    return *this; }
    matrix43& operator*= (const float f)     { (*this)[0]*=f;    (*this)[1]*=f;    (*this)[2]*=f;    (*this)[3]*=f;    return *this; }
    matrix43& operator*= (const matrix33& v) { product(*this, v, *this); return *this; }
    matrix43& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    matrix43& negate () { (*this)[0]=(*this)[0].negate(); (*this)[1]=(*this)[1].negate(); (*this)[2]=(*this)[2].negate(); (*this)[3]=(*this)[3].negate(); return *this; }
    
    matrix43  negate () const { matrix43 cp = *this; return cp.negate(); }
    
    void get_rotation_component (matrix33& m) const { m[0] = (*this)[0]; m[1] = (*this)[1]; m[2] = (*this)[2]; }
    void get_translation_component (vector3& v) const { v.x = r3c0; v.y = r3c1; v.z = r3c2; };
    
    matrix33 get_rotation_component () const { matrix33 r; get_rotation_component (r); return r; }
    vector3 get_translation_component () const { vector3 r; get_translation_component (r); return r; }
    
    matrix43& set_translation_component (const float zx, const float zy, const float zz) { r3c0 = zx; r3c1 = zy; r3c2 = zz; return *this; }
    matrix43& set_translation_component (const vector3& v) { r3c0 = v.x; r3c1 = v.y; r3c2 = v.z; return *this; }
    matrix43& set_rotation_component (const matrix33& m) { (*this)[0] = m[0]; (*this)[1] = m[1]; (*this)[2] = m[2]; return *this; }
    matrix43& set_rotation_component (const quaternion&);
    matrix43& set_scale_component (const vector3& v) { r0c0 = v.x; r1c1 = v.y; r2c2 = v.z; return *this; }
    
    matrix43& set_from_transform (const matrix44&);

    static matrix43 const zero, identity;

    static void product (const matrix43&, const matrix33&, matrix43&); // [4x3] * [3x3] => [4x3]
    static void product (const matrix43&, const vector3& , vector4& ); // [4*3] * [3x1] => [4x1]
    static void product (const vector4& , const matrix43&, vector3& ); // [1x4] * [4*3] => [1x3]
};
    
// ------------------------------------------------------------------------------------------------------------------ //
struct matrix44 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f, r0c3 = 0.0f,
          r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f, r1c3 = 0.0f,
          r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f, r2c3 = 0.0f,
          r3c0 = 0.0f, r3c1 = 0.0f, r3c2 = 0.0f, r3c3 = 1.0f;
    matrix44 () = default;
    matrix44 (float z_r0c0, float z_r0c1, float z_r0c2, float z_r0c3, float z_r1c0, float z_r1c1, float z_r1c2, float z_r1c3, float z_r2c0, float z_r2c1, float z_r2c2, float z_r2c3, float z_r3c0, float z_r3c1, float z_r3c2, float z_r3c3) : r0c0(z_r0c0), r0c1(z_r0c1), r0c2(z_r0c2), r0c3(z_r0c3), r1c0(z_r1c0), r1c1(z_r1c1), r1c2(z_r1c2), r1c3(z_r1c3), r2c0(z_r2c0), r2c1(z_r2c1), r2c2(z_r2c2), r2c3(z_r2c3), r3c0(z_r3c0), r3c1(z_r3c1), r3c2(z_r3c2), r3c3(z_r3c3) {}
    matrix44 (vector4 z_r0, vector4 z_r1, vector4 z_r2, vector4 z_r3) : r0c0(z_r0[0]), r0c1(z_r0[1]), r0c2(z_r0[2]), r0c3(z_r0[3]), r1c0(z_r1[0]), r1c1(z_r1[1]), r1c2(z_r1[2]), r1c3(z_r1[3]), r2c0(z_r2[0]), r2c1(z_r2[1]), r2c2(z_r2[2]), r2c3(z_r2[3]), r3c0(z_r3[0]), r3c1(z_r3[1]), r3c2(z_r3[2]), r3c3(z_r3[3]) {}

    bool operator == (const matrix44& m) const { return (*this)[0] == m[0] && (*this)[1] == m[1] && (*this)[2] == m[2] && (*this)[3] == m[3]; }
    bool operator != (const matrix44& m) const { return !(*this == m); }

          vector4& operator[] (const int i)       { assert (i >=0 && i <= 3); return reinterpret_cast<vector4*>(&r0c0)[i]; }
    const vector4& operator[] (const int i) const { assert (i >=0 && i <= 3); return reinterpret_cast<const vector4*>(&r0c0)[i]; }

          vector3& right   ()       { vector3* p = reinterpret_cast<vector3*>(&r0c0); return *p; }
          vector3& up      ()       { vector3* p = reinterpret_cast<vector3*>(&r1c0); return *p; }
          vector3& backward()       { vector3* p = reinterpret_cast<vector3*>(&r2c0); return *p; }
    const vector3& right   () const { const vector3* p = reinterpret_cast<const vector3*>(&r0c0); return *p; }
    const vector3& up      () const { const vector3* p = reinterpret_cast<const vector3*>(&r1c0); return *p; }
    const vector3& backward() const { const vector3* p = reinterpret_cast<const vector3*>(&r2c0); return *p; }
          vector3  left    () const { return vector3 (-r0c0, -r0c1, -r0c2); }
          vector3  down    () const { return vector3 (-r1c0, -r1c1, -r1c2); }
          vector3  forward () const { return vector3 (-r2c0, -r2c1, -r2c2); }
    
    matrix44& operator+= (const matrix44& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; (*this)[3]+=v[3]; return *this; }
    matrix44& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    (*this)[3]+=f;    return *this; }
    matrix44& operator-= (const matrix44& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; (*this)[3]-=v[3]; return *this; }
    matrix44& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    (*this)[3]-=f;    return *this; }
    matrix44& operator*= (const matrix44& v) { product (*this, v, *this); return *this; }
    matrix44& operator*= (const float f)     { (*this)[0]*=f; (*this)[1]*=f; (*this)[2]*=f; (*this)[3]*=f; return *this; }
    matrix44& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    matrix44& negate () { (*this)[0]=(*this)[0].negate(); (*this)[1]=(*this)[1].negate(); (*this)[2]=(*this)[2].negate(); (*this)[3]=(*this)[3].negate(); return *this; }
    matrix44& transpose ();
    matrix44& inverse ();
    matrix44& affine_inverse ();
    matrix44& decompose ();
    
    float     determinant    () const;
    matrix44  negate         () const { matrix44 cp = *this; return cp.negate(); }
    matrix44  transpose      () const { matrix44 cp = *this; return cp.transpose(); }
    matrix44  inverse        () const { matrix44 cp = *this; return cp.inverse(); }
    matrix44  affine_inverse () const { matrix44 cp = *this; return cp.affine_inverse(); }
    matrix44  decompose      () const { matrix44 cp = *this; return cp.decompose(); }
    
    vector3&    transform (vector3&v) const { vector4 v4 = { v.x, v.y, v.z, 1.0f }; product (v4, *this, v4); v.x = v4.x / v4.w; v.y = v4.y / v4.w; v.z = v4.z / v4.w; return v; } // row vector * matrix
    vector3     transform (const vector3& v) const { vector3 res = v; transform (res); return res; }
    
    vector3&    rotate (vector3&v) const { vector4 v4 = { v.x, v.y, v.z, 0.0f }; product (*this, v4, v4); v.x = v4.x; v.y = v4.y; v.z = v4.z; return v; } // row vector * matrix
    vector3     rotate (const vector3& v) const { vector3 res = v; transform (res); return res; }
    
    void get_rotation_component (matrix33& m) const { m[0] = (*this)[0].xyz(); m[1] = (*this)[1].xyz(); m[2] = (*this)[2].xyz(); }
    void get_translation_component (vector3& v) const { v.x = r3c0; v.y = r3c1; v.z = r3c2; };
    
    matrix33 get_rotation_component () const { matrix33 r; get_rotation_component (r); return r; }
    vector3 get_translation_component () const { vector3 r; get_translation_component (r); return r; }
    
    matrix44& set_translation_component (const float zx, const float zy, const float zz) { r3c0 = zx; r3c1 = zy; r3c2 = zz; return *this; }
    matrix44& set_translation_component (const vector3& v) { r3c0=v.x;r3c1=v.y;r3c2=v.z; return *this; }
    matrix44& set_rotation_component (const matrix33& m) { r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;return *this; }
    matrix44& set_rotation_component (const quaternion&);
    matrix44& set_scale_component (const vector3& v) { r0c0 = v.x; r1c1 = v.y; r2c2 = v.z; return *this; }
    
    matrix44& set_as_view_transform_from_look_at_target (vector3, vector3, vector3); // view direction is negative Z (positive Z is the back of the camera)
    matrix44& set_as_view_frame_from_look_at_target (vector3, vector3, vector3); // view direction is negative Z (positive Z is the back of the camera)
    
    matrix44& set_as_perspective_rh (const float, const float, const float, const float);
    matrix44& set_as_perspective_fov_rh (const float, const float, const float, const float);
    matrix44& set_as_orthographic_off_center (const float, const float, const float, const float, const float, const float);
    
    static matrix44 const zero, identity;

    static void product (const matrix44&, const matrix44&, matrix44&); // [4x4] * [4x4] => [4x4]
    static void product (const matrix44&, const matrix43&, matrix43&); // [4x4] * [4x3] => [4x3]
    static void product (const matrix44&, const vector4& , vector4& ); // [4*4] * [4x1] => [4x1] (column vector * matrix)
    static void product (const vector4& , const matrix44&, vector4& ); // [1x4] * [4*4] => [1x4] (row vector * matrix)
};
    
    
    

    
    

/*          _            _.,----,
 __  _.-._ / '-.        -  ,._  \) 
|  `-)_   '-.   \       / < _ )/" }
/__    '-.   \   '-, ___(c-(6)=(6)
 , `'.    `._ '.  _,'   >\    "  )
 :;;,,'-._   '---' (  ( "/`. -='/
;:;;:;;,  '..__    ,`-.`)'- '--'
;';:;;;;;'-._ /'._|   Y/   _/' \
      '''"._ F    |  _/ _.'._   `\
             L    \   \/     '._  \
      .-,-,_ |     `.  `'---,  \_ _|
      //    'L    /  \,   ("--',=`)7
     | `._       : _,  \  /'`-._L,_'-._
     '--' '-.\__/ _L   .`'         './/
                 [ (  /
                  ) `{
                  \__)*/
// ------------------------------------------------------------------------------------------------------------------ //
// Point 2 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline point2  operator-(const point2& v)                       { auto cp = v; return cp.negate(); }
inline point2  operator-(const point2& l, const point2& r)      { auto cp = l; return cp-=r; }
inline point2  operator+(const point2& l, const point2& r)      { auto cp = l; return cp+=r; }
inline point2  operator*(const point2& v, const int f)          { auto cp = v; return cp*=f; }
inline point2  operator*(const point2& l, const point2& r)      { auto cp = l; return cp*=r; }
inline point2  operator/(const point2& v, const int f)          { auto cp = v; return cp/=f; }
inline point2  operator/(const point2& l, const point2& r)      { auto cp = l; return cp/=r; }

inline float   distance (const point2& l, const point2& r)           { return l.distance(r); }
inline float   distance_sq (const point2& l, const point2& r)        { return l.distance_sq(r); }
inline int     manhattan_distance (const point2& l, const point2& r) { return l.manhattan_distance(r); }
inline int     chebyshev_distance (const point2& l, const point2& r) { return l.chebyshev_distance(r); }


// ------------------------------------------------------------------------------------------------------------------ //
// Vector 2 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline vector2 operator-(const vector2& v)                      { auto cp = v; return cp.negate(); }
inline vector2 operator-(const vector2& l, const vector2& r)    { auto cp = l; return cp-=r; }
inline vector2 operator+(const vector2& l, const vector2& r)    { auto cp = l; return cp+=r; }
inline vector2 operator*(const vector2& v, const float f)       { auto cp = v; return cp*=f; }
inline vector2 operator*(const vector2& l, const vector2& r)    { auto cp = l; return cp*=r; }
inline vector2 operator/(const vector2& v, const float f)       { auto cp = v; return cp/=f; }
inline vector2 operator/(const vector2& l, const vector2& r)    { auto cp = l; return cp/=r; }
inline float   operator|(const vector2& l, const vector2& r)    { return l.dot(r); }
inline vector2 operator~(const vector2& v)                      { auto cp = v; return cp.normalise(); }

inline float   dot         (const vector2& l, const vector2& r) { return l.dot(r); }
inline vector2 normalise   (const vector2& v)                   { auto cp = v; return cp.normalise(); }
inline float   distance    (const vector2& l, const vector2& r) { return l.distance(r); }
inline float   distance_sq (const vector2& l, const vector2& r) { return l.distance_sq(r); }


// ------------------------------------------------------------------------------------------------------------------ //
// Vector 3 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline vector3 operator-(const vector3& v)                      { auto cp = v; return cp.negate(); }
inline vector3 operator-(const vector3& l, const vector3& r)    { auto cp = l; return cp-=r; }
inline vector3 operator+(const vector3& l, const vector3& r)    { auto cp = l; return cp+=r; }
inline vector3 operator*(const vector3& v, const float f)       { auto cp = v; return cp*=f; }
inline vector3 operator*(const vector3& l, const vector3& r)    { auto cp = l; return cp*=r; }
inline vector3 operator*(const vector3& l, const quaternion& r) { auto cp = l; return cp*=r; }
inline vector3 operator*(const vector3& l, const matrix33& r)   { auto cp = l; return cp*=r; }
inline vector3 operator%(const vector3& l, const matrix44& r)   { return r.transform (l); }
inline vector3 operator/(const vector3& v, const float f)       { auto cp = v; return cp/=f; }
inline vector3 operator/(const vector3& l, const vector3& r)    { auto cp = l; return cp/=r; }
inline float   operator|(const vector3& l, const vector3& r)    { return l.dot(r); }
inline vector3 operator^(const vector3& l, const vector3& r)    { auto cp = l; return cp.cross(r); }
inline vector3 operator~(const vector3& v)                      { auto cp = v; return cp.normalise(); }

inline float   dot         (const vector3& l, const vector3& r) { return l.dot(r); }
inline vector3 cross       (const vector3& l, const vector3& r) { auto cp = l; return cp.cross(r); }
inline vector3 normalise   (const vector3& v)                   { auto cp = v; return cp.normalise(); }
inline float   distance    (const vector3& l, const vector3& r) { return l.distance(r); }
inline float   distance_sq (const vector3& l, const vector3& r) { return l.distance_sq(r); }

// ------------------------------------------------------------------------------------------------------------------ //
// Vector 4 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline vector4 operator-(const vector4& v)                   { auto cp = v; return cp.negate(); }
inline vector4 operator-(const vector4& l, const vector4& r) { auto cp = l; return cp-=r; }
inline vector4 operator+(const vector4& l, const vector4& r) { auto cp = l; return cp+=r; }
inline vector4 operator*(const vector4& v, const float f)    { auto cp = v; return cp*=f; }
inline vector4 operator*(const vector4& l, const vector4& r) { auto cp = l; return cp*=r; }
inline vector4 operator*(const vector4& l, const matrix44& r){ vector4 res; matrix44::product (l, r, res); return res; }
inline vector3 operator*(const vector4& l, const matrix43& r){ vector3 res; matrix43::product (l, r, res); return res; }
inline vector4 operator/(const vector4& v, const float f)    { auto cp = v; return cp/=f; }
inline vector4 operator/(const vector4& l, const vector4& r) { auto cp = l; return cp/=r; }
inline float   operator|(const vector4& l, const vector4& r) { return l.dot(r); }
inline vector4 operator~(const vector4& v)                   { auto cp = v; return cp.normalise(); }

inline vector4 normalise (const vector4& v)                  { auto cp = v; return cp.normalise(); }

// ------------------------------------------------------------------------------------------------------------------ //
// Quaternion extras
// ------------------------------------------------------------------------------------------------------------------ //
inline quaternion operator-(const quaternion& v)                      { auto cp = v; return cp.negate(); }
inline quaternion operator-(const quaternion& l, const quaternion& r) { auto cp = l; return cp-=r; }
inline quaternion operator+(const quaternion& l, const quaternion& r) { auto cp = l; return cp+=r; }
inline quaternion operator*(const quaternion& v, const float f)       { auto cp = v; return cp*=f; }
inline quaternion operator/(const quaternion& v, const float f)       { auto cp = v; return cp/=f; }
inline quaternion operator&(const quaternion& l, const quaternion& r) { auto cp = l; return cp.concatenate(r); }
inline quaternion operator~(const quaternion& v)                      { auto cp = v; return cp.normalise(); }
inline quaternion operator!(const quaternion& v)                      { auto cp = v; return cp.conjugate(); }
inline vector3    operator*(const quaternion& l, const vector3& r)    { return l.rotate (r); }

inline quaternion concatenate(const quaternion& l, const quaternion& r) { auto cp = l; return cp.concatenate(r); }
inline quaternion normalise  (const quaternion& v)                      { auto cp = v; return cp.normalise(); }
inline quaternion conjugate  (const quaternion& v)                      { auto cp = v; return cp.conjugate(); }

// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 3x3 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline matrix33 operator-(const matrix33& v)                    { auto cp = v; return cp.negate(); }
inline matrix33 operator-(const matrix33& l, const matrix33& r) { auto cp = l; return cp-=r; }
inline matrix33 operator+(const matrix33& l, const matrix33& r) { auto cp = l; return cp+=r; }
inline matrix33 operator*(const matrix33& v, const float f)     { auto cp = v; return cp*=f; }
inline matrix33 operator*(const matrix33& l, const matrix33& r) { auto cp = l; return cp*=r; }
inline vector3  operator*(const matrix33& l, const vector3& r)  { vector3 res; matrix33::product (l, r, res); return res; }
inline matrix33 operator/(const matrix33& v, const float f)     { auto cp = v; return cp/=f; }

inline matrix33 orthonormalise(const matrix33& v) { auto cp = v; return cp.orthonormalise(); }
inline matrix33 negate(const matrix33& v) { auto cp = v; return cp.negate(); }

// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 4x3 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline matrix43 operator-(const matrix43& v)                    { auto cp = v; return cp.negate(); }
inline matrix43 operator-(const matrix43& l, const matrix43& r) { auto cp = l; return cp-=r; }
inline matrix43 operator+(const matrix43& l, const matrix43& r) { auto cp = l; return cp+=r; }
inline matrix43 operator*(const matrix43& v, const float f)     { auto cp = v; return cp*=f; }
inline matrix43 operator/(const matrix43& v, const float f)     { auto cp = v; return cp/=f; }

inline matrix43 operator*(const matrix43& l, const matrix33& r) { matrix43 res; matrix43::product (l, r, res); return res; }
inline vector4  operator*(const matrix43& l, const vector3& r)  { vector4 res; matrix43::product (l, r, res); return res; }

inline matrix43 negate    (const matrix43& v) { auto cp = v; return cp.negate(); }

// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 4x4 extras
// ------------------------------------------------------------------------------------------------------------------ //
inline matrix44 operator-(const matrix44& v)                    { auto cp = v; return cp.negate(); }
inline matrix44 operator-(const matrix44& l, const matrix44& r) { auto cp = l; return cp-=r; }
inline matrix44 operator+(const matrix44& l, const matrix44& r) { auto cp = l; return cp+=r; }
inline matrix44 operator*(const matrix44& v, const float f)     { auto cp = v; return cp*=f; }
inline matrix44 operator*(const matrix44& l, const matrix44& r) { auto cp = l; return cp*=r; }
inline vector3  operator%(const matrix44& l, const vector3& r)  { return l.transform(r); }
inline matrix44 operator/(const matrix44& v, const float f)     { auto cp = v; return cp/=f; }

inline matrix43 operator*(const matrix44& l, const matrix43& r) { matrix43 res; matrix44::product (l, r, res); return res; }
inline vector4  operator*(const matrix44& l, const vector4& r)  { vector4 res; matrix44::product (l, r, res); return res; }

inline matrix44 negate(const matrix44& v) { auto cp = v; return cp.negate(); }
inline matrix44 transpose(const matrix44& v) { auto cp = v; return cp.transpose(); }
inline matrix44 inverse(const matrix44& v) { auto cp = v; return cp.inverse(); }
inline matrix44 affine_inverse(const matrix44& v) { auto cp = v; return cp.affine_inverse(); }
inline matrix44 decompose(const matrix44& v) { auto cp = v; return cp.decompose(); }

}

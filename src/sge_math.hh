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
#pragma once

#include "sge.hh" // (dependencies: math.h, assert.h)

namespace sge::math {

const float PI = atan (1.0f) * 4.0f;
const float TAU = 2.0f * PI;
const float EPSILON = 0.000001f;

inline bool is_zero (const float f) { return (f >= -EPSILON) && (f <= EPSILON); }
    
inline int max (int l, int r) { if (l > r) return l; else return r; }
inline float max (float l, float r) { if (l > r) return l; else return r; }
inline int min (int l, int r) { if (l < r) return l; else return r; }
inline float min (float l, float r) { if (l < r) return l; else return r; }

struct point2; struct rect; struct vector2; struct vector3; struct vector4; struct quaternion; struct matrix33; struct matrix43; struct matrix44;

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
    
    bool     is_unit   () const { return is_zero (1.0f - x*x - y*y); }
    float    length    () const { return sqrt (x*x + y*y); }
    float    length_sq () const { return x*x + y*y; }
    vector2& normalise ()       { const float l = length (); x /= l; y /= l; return *this; }
    
    static const vector2 zero, one, unit_x, unit_y;
};

struct vector3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    vector3 () = default;
    vector3 (const float z_x, const float z_y, const float z_z) : x (z_x), y (z_y), z (z_z) {}
    
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
    vector3& operator^= (const vector3& v) { // cross
        float const xx = y*v.z - z*v.y;
        float const yx = z*v.x - x*v.z;
        float const zx = x*v.y - y*v.x;
        x = xx; y = yx; z = zx; return *this;
    }
    
    bool    is_unit    () const { return is_zero (1.0f - x*x - y*y - z*z); }
    float    length    () const { return sqrt (x*x + y*y + z*z); }
    float    length_sq () const { return x*x + y*y + z*z; }
    vector3& normalise ()       { const float l = length (); x /= l; y /= l; z /= l; return *this; }
    
    static const vector3 zero, one, unit_x, unit_y, unit_z, unit_w, right, up, backward, left, down, forward;
};
    
struct vector4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
    
    vector4 () = default;
    vector4 (const float z_x, const float z_y, const float z_z, const float z_w) : x (z_x), y (z_y), z (z_z), w (z_w) {}
    
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
    vector4& operator/= (const vector4& v) { x/=v.x; y/=v.y; z/=v.z; w/=v.w; return *this; }
    vector4& operator/= (const float f)    { x/=f;   y/=f;   z/=f;   w/=f;   return *this; }
    
    bool     is_unit   () const { return is_zero (1.0f - x*x - y*y - z*z - w*w); }
    float    length    () const { return sqrt (x*x + y*y + z*z + w*w); }
    float    length_sq () const { return x*x + y*y + z*z + w*w; }
    vector4& normalise ()       { const float l = length (); x /= l; y /= l; z /= l; w /= l; return *this; }
    
    static const vector4 zero, one, unit_x, unit_y, unit_z, unit_w;
};

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
    quaternion& operator&= (const quaternion& v) { // concatenate (chain rotations)
        const float ix = u*v.i + i*v.u + k*v.j - j*v.k;
        const float jx = u*v.j + j*v.u + i*v.k - k*v.i;
        const float kx = u*v.k + k*v.u + j*v.i - i*v.j;
        const float ux = u*v.u - k*v.k - i*v.i + j*v.j;
        i = ix; j = jx; k = kx; u = ux; return *this;
    }
    
    bool        is_unit   () const { return is_zero (1.0f - i*i - j*j - k*k - u*u); }
    float       length    () const { return sqrt (i*i + j*j + k*k + u*u); }
    float       length_sq () const { return i*i + j*j + k*k + u*u; }
    quaternion& normalise ()       { const float l = length (); i /= l; j /= l; k /= l; u /= l; return *this; }

    void        get_axis_angle (vector3&, float&) const;
    void        get_yaw_pitch_roll (vector3&) const;
    
    quaternion& set_from_axis_angle (const vector3&, const float);
    quaternion& set_from_yaw_pitch_roll (const float, const float, const float);
    quaternion& set_from_rotation (const matrix33&);
    
    static quaternion create_from_axis_angle (const vector3& axis, const float angle) { return quaternion ().set_from_axis_angle (axis, angle); }
    static quaternion create_from_yaw_pitch_roll (const float yaw, const float pitch, const float roll) { return quaternion ().set_from_yaw_pitch_roll (yaw, pitch, roll); }
    static quaternion create_from_rotation (const matrix33& m) { return quaternion ().set_from_rotation (m); }
    
    static quaternion const zero, identity;
};

struct matrix33 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f, r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f, r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f;
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
          vector3  left    ()       { return vector3 (-r0c0, -r0c1, -r0c2); }
          vector3  down    ()       { return vector3 (-r1c0, -r1c1, -r1c2); }
          vector3  forward ()       { return vector3 (-r2c0, -r2c1, -r2c2); }
    
    matrix33& operator+= (const matrix33& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; return *this; }
    matrix33& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    return *this; }
    matrix33& operator-= (const matrix33& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; return *this; }
    matrix33& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    return *this; }
    matrix33& operator*= (const matrix33& v) { assert (false); return *this; } // todo
    matrix33& operator*= (const float f)     { (*this)[0]*=f;    (*this)[1]*=f;    (*this)[2]*=f;    return *this; }
    matrix33& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    bool is_orthonormal () const;
    matrix33& orthonormalise ();
    
    matrix33& set_from_orientation (const quaternion&);
    matrix33& set_from_transform (const matrix43& m);
    matrix33& set_from_transform (const matrix44& m);
    
    static matrix33 create_from_orientation (const quaternion& q) { return matrix33 ().set_from_orientation (q); }
    static matrix33 create_from_transform (const matrix43& m) { return matrix33 ().set_from_transform (m); }
    static matrix33 create_from_transform (const matrix44& m) { return matrix33 ().set_from_transform (m); }
    
    static matrix33 const zero, identity;
};

struct matrix43 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f, r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f, r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f, r3c0 = 0.0f, r3c1 = 0.0f, r3c2 = 0.0f;
    matrix43 () = default;
    matrix43 (float z_r0c0, float z_r0c1, float z_r0c2, float z_r1c0, float z_r1c1, float z_r1c2, float z_r2c0, float z_r2c1, float z_r2c2, float z_r3c0, float z_r3c1, float z_r3c2) : r0c0(z_r0c0), r0c1(z_r0c1), r0c2(z_r0c2), r1c0(z_r1c0), r1c1(z_r1c1), r1c2(z_r1c2), r2c0(z_r2c0), r2c1(z_r2c1), r2c2(z_r2c2), r3c0(z_r3c0), r3c1(z_r3c1), r3c2(z_r3c2) {}
    matrix43 (vector3 z_r0, vector3 z_r1, vector3 z_r2, vector3 z_r3) : r0c0(z_r0[0]), r0c1(z_r0[1]), r0c2(z_r0[2]), r1c0(z_r1[0]), r1c1(z_r1[1]), r1c2(z_r1[2]), r2c0(z_r2[0]), r2c1(z_r2[1]), r2c2(z_r2[2]), r3c0(z_r3[0]), r3c1(z_r3[1]), r3c2(z_r3[2]) {}
    
    bool operator == (const matrix43& m) const { return (*this)[0] == m[0] && (*this)[1] == m[1] && (*this)[2] == m[2] && (*this)[3] == m[3]; }
    bool operator != (const matrix43& m) const { return !(*this == m); }

          vector3& operator[] (const int i)       { assert (i >=0 && i <= 3); return reinterpret_cast<vector3*>(&r0c0)[i]; }
    const vector3& operator[] (const int i) const { assert (i >=0 && i <= 3); return reinterpret_cast<const vector3*>(&r0c0)[i]; }
    
    matrix43& operator+= (const matrix43& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; (*this)[3]+=v[3]; return *this; }
    matrix43& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    (*this)[3]+=f;    return *this; }
    matrix43& operator-= (const matrix43& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; (*this)[3]-=v[3]; return *this; }
    matrix43& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    (*this)[3]-=f;    return *this; }
    matrix43& operator*= (const matrix43& v) { assert (false); return *this; } // todo
    matrix43& operator*= (const matrix33& v) { assert (false); return *this; } // todo
    matrix43& operator*= (const float f)     { (*this)[0]*=f;    (*this)[1]*=f;    (*this)[2]*=f;    (*this)[3]*=f;    return *this; }
    matrix43& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    void get_rotation_component (matrix33& m) const { m[0] = (*this)[0]; m[1] = (*this)[1]; m[2] = (*this)[2]; }
    
    matrix43& set_rotation_component (const matrix33& m) { (*this)[0] = m[0]; (*this)[1] = m[1]; (*this)[2] = m[2]; return *this; }
    matrix43& set_rotation_component (const quaternion&);
    
    matrix43& set_from_transform (const matrix44&);
    
    static matrix43 create_from_rotation (const matrix33& m) { return matrix43 ().set_rotation_component (m); }
    static matrix43 create_from_orientation (const quaternion& q) { return matrix43 ().set_rotation_component (q); }
    static matrix43 create_from_transform (const matrix44& m) { return matrix43 ().set_from_transform (m); }
    
    static matrix43 const zero, identity;
};
    
struct matrix44 {
    float r0c0 = 1.0f, r0c1 = 0.0f, r0c2 = 0.0f, r0c3 = 0.0f, r1c0 = 0.0f, r1c1 = 1.0f, r1c2 = 0.0f, r1c3 = 0.0f, r2c0 = 0.0f, r2c1 = 0.0f, r2c2 = 1.0f, r2c3 = 0.0f, r3c0 = 0.0f, r3c1 = 0.0f, r3c2 = 0.0f, r3c3 = 1.0f;
    matrix44 () = default;
    matrix44 (float z_r0c0, float z_r0c1, float z_r0c2, float z_r0c3, float z_r1c0, float z_r1c1, float z_r1c2, float z_r1c3, float z_r2c0, float z_r2c1, float z_r2c2, float z_r2c3, float z_r3c0, float z_r3c1, float z_r3c2, float z_r3c3) : r0c0(z_r0c0), r0c1(z_r0c1), r0c2(z_r0c2), r0c3(z_r0c3), r1c0(z_r1c0), r1c1(z_r1c1), r1c2(z_r1c2), r1c3(z_r1c3), r2c0(z_r2c0), r2c1(z_r2c1), r2c2(z_r2c2), r2c3(z_r2c3), r3c0(z_r3c0), r3c1(z_r3c1), r3c2(z_r3c2), r3c3(z_r3c3) {}
    matrix44 (vector4 z_r0, vector4 z_r1, vector4 z_r2, vector4 z_r3) : r0c0(z_r0[0]), r0c1(z_r0[1]), r0c2(z_r0[2]), r0c3(z_r0[3]), r1c0(z_r1[0]), r1c1(z_r1[1]), r1c2(z_r1[2]), r1c3(z_r1[3]), r2c0(z_r2[0]), r2c1(z_r2[1]), r2c2(z_r2[2]), r2c3(z_r2[3]), r3c0(z_r3[0]), r3c1(z_r3[1]), r3c2(z_r3[2]), r3c3(z_r3[3]) {}

    bool operator == (const matrix44& m) const { return (*this)[0] == m[0] && (*this)[1] == m[1] && (*this)[2] == m[2] && (*this)[3] == m[3]; }
    bool operator != (const matrix44& m) const { return !(*this == m); }

          vector4& operator[] (const int i)       { assert (i >=0 && i <= 3); return reinterpret_cast<vector4*>(&r0c0)[i]; }
    const vector4& operator[] (const int i) const { assert (i >=0 && i <= 3); return reinterpret_cast<const vector4*>(&r0c0)[i]; }
    
    matrix44& operator+= (const matrix44& v) { (*this)[0]+=v[0]; (*this)[1]+=v[1]; (*this)[2]+=v[2]; (*this)[3]+=v[3]; return *this; }
    matrix44& operator+= (const float f)     { (*this)[0]+=f;    (*this)[1]+=f;    (*this)[2]+=f;    (*this)[3]+=f;    return *this; }
    matrix44& operator-= (const matrix44& v) { (*this)[0]-=v[0]; (*this)[1]-=v[1]; (*this)[2]-=v[2]; (*this)[3]-=v[3]; return *this; }
    matrix44& operator-= (const float f)     { (*this)[0]-=f;    (*this)[1]-=f;    (*this)[2]-=f;    (*this)[3]-=f;    return *this; }
    matrix44& operator*= (const matrix44& v) {
        r0c0 = r0c0*v.r0c0 + r0c1*v.r1c0 + r0c2*v.r2c0 + r0c3*v.r3c0;
        r0c1 = r0c0*v.r0c1 + r0c1*v.r1c1 + r0c2*v.r2c1 + r0c3*v.r3c1;
        r0c2 = r0c0*v.r0c2 + r0c1*v.r1c2 + r0c2*v.r2c2 + r0c3*v.r3c2;
        r0c3 = r0c0*v.r0c3 + r0c1*v.r1c3 + r0c2*v.r2c3 + r0c3*v.r3c3;
        r1c0 = r1c0*v.r0c0 + r1c1*v.r1c0 + r1c2*v.r2c0 + r1c3*v.r3c0;
        r1c1 = r1c0*v.r0c1 + r1c1*v.r1c1 + r1c2*v.r2c1 + r1c3*v.r3c1;
        r1c2 = r1c0*v.r0c2 + r1c1*v.r1c2 + r1c2*v.r2c2 + r1c3*v.r3c2;
        r1c3 = r1c0*v.r0c3 + r1c1*v.r1c3 + r1c2*v.r2c3 + r1c3*v.r3c3;
        r2c0 = r2c0*v.r0c0 + r2c1*v.r1c0 + r2c2*v.r2c0 + r2c3*v.r3c0;
        r2c1 = r2c0*v.r0c1 + r2c1*v.r1c1 + r2c2*v.r2c1 + r2c3*v.r3c1;
        r2c2 = r2c0*v.r0c2 + r2c1*v.r1c2 + r2c2*v.r2c2 + r2c3*v.r3c2;
        r2c3 = r2c0*v.r0c3 + r2c1*v.r1c3 + r2c2*v.r2c3 + r2c3*v.r3c3;
        r3c0 = r3c0*v.r0c0 + r3c1*v.r1c0 + r3c2*v.r2c0 + r3c3*v.r3c0;
        r3c1 = r3c0*v.r0c1 + r3c1*v.r1c1 + r3c2*v.r2c1 + r3c3*v.r3c1;
        r3c2 = r3c0*v.r0c2 + r3c1*v.r1c2 + r3c2*v.r2c2 + r3c3*v.r3c2;
        r3c3 = r3c0*v.r0c3 + r3c1*v.r1c3 + r3c2*v.r2c3 + r3c3*v.r3c3;
        return *this;
    }
    matrix44& operator*= (const matrix43& v) { assert (false); return *this; } // todo
    matrix44& operator*= (const matrix33& v) { assert (false); return *this; } // todo
    matrix44& operator*= (const float f)     { (*this)[0]*=f; (*this)[1]*=f; (*this)[2]*=f; (*this)[3]*=f; return *this; }
    matrix44& operator/= (const float f)     { return (*this) *= (1.0f / f); }
    
    void get_rotation_component (matrix33& m) const { m[0] = (*this)[0].xyz(); m[1] = (*this)[1].xyz(); m[2] = (*this)[2].xyz(); }
    
    matrix44& set_rotation_component (const matrix33& m) { r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;return *this; }
    matrix44& set_rotation_component (const quaternion&);
    
    static matrix44 create_from_rotation (const matrix33& m) { return matrix44 ().set_rotation_component (m); }
    static matrix44 create_from_orientation (const quaternion& q) { return matrix44 ().set_rotation_component (q); }
    
    static matrix44 const zero, identity;
};

// Point 2 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline point2  operator+(const point2& l, const point2& r) { return point2 (l.x + r.x, l.y + r.y); }
inline point2  operator-(const point2& p) { return point2 (-p.x, -p.y); }
inline point2  operator-(const point2& l, const point2& r) { return point2 (l.x - r.x, l.y - r.y); }
inline point2  operator*(const point2& l, const point2& r) { return point2 (l.x * r.x, l.y * r.y); }
inline point2  operator*(const point2& p, const int i) { return point2 (p.x * i, p.y * i); }
inline vector2 operator*(const point2& p, const float f) { return vector2 ((float) p.x * f, (float) p.y * f); }
inline point2  operator/(const point2& l, const point2& r) { return point2 (l.x / r.x, l.y / r.y); }
inline point2  operator/(const point2& p, const int i) { return point2 (p.x / i, p.y / i); }
inline vector2 operator/(const point2& p, const float f) { return vector2 ((float) p.x / f, (float) p.y / f); }

inline float distance (const point2& l, const point2& r) { return sqrt ((float)((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y))); }
inline int manhattan_distance (const point2& l, const point2& r) { return (l.x-r.x) + (l.y-r.y); }
inline int chebyshev_distance (const point2& l, const point2& r) { return max (l.x-r.x, l.y-r.y); }

// Vector 2 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline vector2 operator+(const vector2& l, const vector2& r) { return vector2 (l.x + r.x, l.y + r.y); }
inline vector2 operator-(const vector2& v) { return vector2 (-v.x, -v.y); }
inline vector2 operator-(const vector2& l, const vector2& r) { return vector2 (l.x - r.x, l.y - r.y); }
inline vector2 operator*(const vector2& l, const vector2& r) { return vector2 (l.x * r.x, l.y * r.y); }
inline vector2 operator*(const vector2& v, const float f) { return vector2 (v.x * f, v.y * f); }
inline vector2 operator/(const vector2& l, const vector2& r) { return vector2 (l.x / r.x, l.y / r.y); }
inline vector2 operator/(const vector2& v, const float f) { return v * (1.0f / f); }
inline float   operator|(const vector2& l, const vector2& r) { return l.x * r.x + l.y * r.y; } // dot

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
inline vector3& vector3::operator*= (const matrix33& m) {
    float const xx = x*m[0][0] + y*m[1][0] + z*m[2][0];
    float const yx = x*m[0][1] + y*m[1][1] + z*m[2][1];
    float const zx = x*m[0][2] + y*m[1][2] + z*m[2][2];
    x = xx; y = yx; z = zx; return *this;
}
inline vector3 operator+(const vector3& l, const vector3& r) { return vector3 (l.x + r.x, l.y + r.y, l.z + r.z); }
inline vector3 operator-(const vector3& v) { return vector3 (-v.x, -v.y, -v.z); }
inline vector3 operator-(const vector3& l, const vector3& r) { return vector3 (l.x - r.x, l.y - r.y, l.z - r.z); }
inline vector3 operator*(const vector3& l, const vector3& r) { return vector3 (l.x * r.x, l.y * r.y, l.z * r.z); }
inline vector3 operator*(const vector3& v, const float f) { return vector3 (v.x * f, v.y * f, v.z * f); }
inline vector3 operator*(const vector3& v, const quaternion& q) { return vector3 (
    v.x - (2.0f * v.x * (q.j*q.j + q.k*q.k)) + (2.0f * v.y * (q.i*q.j - q.u*q.k)) + (2.0f * v.z * (q.i*q.k + q.u*q.j)),
    v.y + (2.0f * v.x * (q.i*q.j + q.u*q.k)) - (2.0f * v.y * (q.i*q.i + q.k*q.k)) + (2.0f * v.z * (q.j*q.k - q.u*q.i)),
    v.z + (2.0f * v.x * (q.i*q.k - q.u*q.j)) + (2.0f * v.y * (q.j*q.k + q.u*q.i)) - (2.0f * v.z * (q.i*q.i + q.j*q.j)));
}
inline vector3 operator*(const vector3& v, const matrix33& m) { return vector3 {
    v.x*m[0][0] + v.y*m[1][0] + v.z*m[2][0],
    v.x*m[0][1] + v.y*m[1][1] + v.z*m[2][1],
    v.x*m[0][2] + v.y*m[1][2] + v.z*m[2][2] };
}
inline vector3 operator/(const vector3& l, const vector3& r) { return vector3 (l.x / r.x, l.y / r.y, l.z / r.z); }
inline vector3 operator/(const vector3& v, const float f) { return v * (1.0f / f); }
inline float   operator|(const vector3& l, const vector3& r) { return l.x * r.x + l.y * r.y + l.z * r.z; } // dot
inline vector3 operator^(const vector3& l, const vector3& r) { return vector3 (l.y*r.z - l.z*r.y, l.z*r.x - l.x*r.z, l.x*r.y - l.y*r.x); } // cross

inline vector3 normalise (const vector3& v) { return v / v.length (); }
inline float   distance (const vector3& l, const vector3& r) { return sqrt ((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y) + (l.z-r.z)*(l.z-r.z)); }

// Vector 4 inline
// ------------------------------------------------------------------------------------------------------------------ //
inline vector4 operator+(const vector4& l, const vector4& r) { return vector4 (l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w); }
inline vector4 operator-(const vector4& v) { return vector4 (-v.x, -v.y, -v.z, -v.w); }
inline vector4 operator-(const vector4& l, const vector4& r) { return vector4 (l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w); }
inline vector4 operator*(const vector4& l, const vector4& r) { return vector4 (l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w); }
inline vector4 operator*(const vector4& v, const float f) { return vector4 (v.x * f, v.y * f, v.z * f, v.w * f); }
inline vector4 operator/(const vector4& l, const vector4& r) { return vector4 (l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w); }
inline vector4 operator/(const vector4& v, const float f) { return v * (1.0f / f); }
inline float   operator|(const vector4& l, const vector4& r) { return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w; } // dot

inline vector4 normalise (const vector4& v) { return v / v.length (); }
inline float   distance (const vector4& l, const vector4& r) { return sqrt ((l.x-r.x)*(l.x-r.x) + (l.y-r.y)*(l.y-r.y) + (l.z-r.z)*(l.z-r.z) + (l.w-r.w)*(l.w-r.w)); }

// Quaternion inline
// ------------------------------------------------------------------------------------------------------------------ //
inline quaternion operator~(const quaternion& v) { return quaternion { - v.i, - v.j, - v.k, v.u }; } // conjugate (inverse)
inline quaternion operator&(const quaternion& l, const quaternion& r) { return quaternion { // concatenate (chain rotations)
    l.u*r.i + l.i*r.u + l.k*r.j - l.j*r.k, l.u*r.j + l.j*r.u + l.i*r.k - l.k*r.i,
    l.u*r.k + l.k*r.u + l.j*r.i - l.i*r.j, l.u*r.u - l.k*r.k - l.i*r.i + l.j*r.j };
}
inline quaternion operator+(const quaternion& l, const quaternion& r) { return quaternion (l.i + r.i, l.j + r.j, l.k + r.k, l.u + r.u); }
inline quaternion operator-(const quaternion& v) { return quaternion (-v.i, -v.j, -v.k, -v.u); }
inline quaternion operator-(const quaternion& l, const quaternion& r) { return quaternion (l.i - r.i, l.j - r.j, l.k - r.k, l.u - r.u); }
inline quaternion operator*(const quaternion& v, const float f) { return quaternion (v.i * f, v.j * f, v.k * f, v.u * f); }
inline quaternion operator/(const quaternion& v, const float f) { return v * (1.0f / f); }
inline float      operator|(const quaternion& l, const quaternion& r) { return l.i * r.i + l.j * r.j + l.k * r.k + l.u * r.u; } // dot

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

inline quaternion& quaternion::set_from_axis_angle (const vector3& axis, const float angle) {
    const float half_angle = angle / 2.0f;
    const float s = sin(half_angle);
    const float c = cos(half_angle);
    i = s * axis.x;
    j = s * axis.y;
    k = s * axis.z;
    u = c;
    assert (is_unit());
    return *this;
}

inline quaternion& quaternion::set_from_yaw_pitch_roll (const float yaw, const float pitch, const float roll) {
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
    assert (is_unit());
    return *this;
}

inline quaternion& quaternion::set_from_rotation (const matrix33& m) {
    assert (m.is_orthonormal());
    const float tr = m.r0c0 + m.r1c1 + m.r2c2;
    if (tr > 0.0f) {
        const float s = sqrt (tr + 1.0f) * 2.0f;
        u = 0.25f * s; i = (m.r1c2 - m.r2c1) / s; j = (m.r2c0 - m.r0c2) / s; k = (m.r0c1 - m.r1c0) / s;
    }
    else if ((m.r0c0 >= m.r1c1) && (m.r0c0 >= m.r2c2)) {
        const float s = sqrt (1.0f + m.r0c0 - m.r1c1 - m.r2c2) * 2.0f;
        u = (m.r1c2 - m.r2c1) / s; i = 0.25f * s; j = (m.r0c1 + m.r1c0) / s; k = (m.r0c2 + m.r2c0) / s;
    }
    else if (m.r1c1 > m.r2c2) {
        const float s = sqrt (1.0f + m.r1c1 - m.r0c0 - m.r2c2) * 2.0f;
        u = (m.r2c0 - m.r0c2) / s; i = (m.r1c0 + m.r0c1) / s; j = 0.25f * s; k = (m.r2c1 + m.r1c2) / s;
    }
    else {
        const float s = sqrt (1.0f + m.r2c2 - m.r0c0 - m.r1c1) * 2.0f;
        u = (m.r0c1 - m.r1c0) / s; i = (m.r2c0 + m.r0c2) / s; j = (m.r2c1 + m.r1c2) / s; k = 0.25f * s;
    }
    
    assert (is_unit());
    return *this;
}

// Matrix 3x3 inline
// ------------------------------------------------------------------------------------------------------------------ //

inline matrix33 operator+(const matrix33& l, const matrix33& r) { return matrix33 (l[0] + r[0], l[1] + r[1], l[2] + r[2], l[3] + r[3]); }
inline matrix33 operator-(const matrix33& v) { return matrix33 (-v[0], -v[1], -v[2], -v[3]); }
inline matrix33 operator-(const matrix33& l, const matrix33& r) { return matrix33 (l[0] - r[0], l[1] - r[1], l[2] - r[2], l[3] - r[3]); }
inline matrix33 operator*(const matrix33& v, const float f) { return matrix33 (v[0] * f, v[1] * f, v[2] * f, v[3] * f); }
inline matrix33 operator/(const matrix33& v, const float f) { return v * (1.0f / f); }

inline matrix33& matrix33::orthonormalise() {
    up() = normalise (up());
    right() = normalise (up() ^ backward());
    backward() = right() ^ up();
    assert (backward().is_unit());
    assert (is_orthonormal());
    return *this;
}
    
inline bool matrix33::is_orthonormal () const {
    for (int i = 0; i < 3; ++i) {
        if (!(*this)[i].is_unit()) return false;
        if (((*this)[i] | (*this)[(i+1) % 3]) > EPSILON) return false;
    } return true;
}

inline matrix33& matrix33::set_from_transform (const matrix43& m) { r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2; return *this; }
inline matrix33& matrix33::set_from_transform (const matrix44& m) { r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2; return *this; }
    
inline matrix33& matrix33::set_from_orientation (const quaternion& q) { // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
    assert (q.is_unit());
    (*this)[0][0] = 1.0f - 2.0f* q.j*q.j - 2.0f* q.k*q.k;
    (*this)[1][0] = 2.0f* q.i*q.j - 2.0f* q.u*q.k;
    (*this)[2][0] = 2.0f* q.i*q.k + 2.0f* q.u*q.j;
    (*this)[0][1] = 2.0f* q.i*q.j + 2.0f* q.u*q.k;
    (*this)[1][1] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.k*q.k;
    (*this)[2][1] = 2.0f* q.j*q.k - 2.0f* q.u*q.i;
    (*this)[0][2] = 2.0f* q.i*q.k - 2.0f* q.u*q.j;
    (*this)[1][2] = 2.0f* q.j*q.k + 2.0f* q.u*q.i;
    (*this)[2][2] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.j*q.j;
    orthonormalise (); // would prefer to just assert here, but I keep hitting it :/
    return *this;
}
    
// Matrix 4x3 inline
// ------------------------------------------------------------------------------------------------------------------ //

inline matrix43 operator+(const matrix43& l, const matrix43& r) { return matrix43 (l[0] + r[0], l[1] + r[1], l[2] + r[2], l[3] + r[3]); }
inline matrix43 operator-(const matrix43& v) { return matrix43 (-v[0], -v[1], -v[2], -v[3]); }
inline matrix43 operator-(const matrix43& l, const matrix43& r) { return matrix43 (l[0] - r[0], l[1] - r[1], l[2] - r[2], l[3] - r[3]); }
inline matrix43 operator*(const matrix43& v, const float f) { return matrix43 (v[0] * f, v[1] * f, v[2] * f, v[3] * f); }
inline matrix43 operator/(const matrix43& v, const float f) { return v * (1.0f / f); }

inline matrix43& matrix43::set_from_transform (const matrix44& m) { r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;r3c0=m.r3c0;r3c1=m.r3c1;r3c2=m.r3c2; return *this; }
    
inline matrix43& matrix43::set_rotation_component (const quaternion& q) {
    assert (q.is_unit());
    (*this)[0][0] = 1.0f - 2.0f* q.j*q.j - 2.0f* q.k*q.k;
    (*this)[1][0] = 2.0f* q.i*q.j - 2.0f* q.u*q.k;
    (*this)[2][0] = 2.0f* q.i*q.k + 2.0f* q.u*q.j;
    (*this)[3][0] = 0.0f;
    (*this)[0][1] = 2.0f* q.i*q.j + 2.0f* q.u*q.k;
    (*this)[1][1] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.k*q.k;
    (*this)[2][1] = 2.0f* q.j*q.k - 2.0f* q.u*q.i;
    (*this)[3][1] = 0.0f;
    (*this)[0][2] = 2.0f* q.i*q.k - 2.0f* q.u*q.j;
    (*this)[1][2] = 2.0f* q.j*q.k + 2.0f* q.u*q.i;
    (*this)[2][2] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.j*q.j;
    (*this)[3][2] = 0.0f;
    return *this;
}
    
// Matrix 4x4 inline
// ------------------------------------------------------------------------------------------------------------------ //

inline matrix44 operator+(const matrix44& l, const matrix44& r) { return matrix44 (l[0] + r[0], l[1] + r[1], l[2] + r[2], l[3] + r[3]); }
inline matrix44 operator-(const matrix44& v) { return matrix44 (-v[0], -v[1], -v[2], -v[3]); }
inline matrix44 operator-(const matrix44& l, const matrix44& r) { return matrix44 (l[0] - r[0], l[1] - r[1], l[2] - r[2], l[3] - r[3]); }
inline matrix44 operator*(const matrix44& v, const float f) { return matrix44 (v[0] * f, v[1] * f, v[2] * f, v[3] * f); }
inline matrix44 operator/(const matrix44& v, const float f) { return v * (1.0f / f); }
inline matrix44 operator*(const matrix44& l, const matrix44& r) { return matrix44 (
    l.r0c0*r.r0c0 + l.r0c1*r.r1c0 + l.r0c2*r.r2c0 + l.r0c3*r.r3c0,
    l.r0c0*r.r0c1 + l.r0c1*r.r1c1 + l.r0c2*r.r2c1 + l.r0c3*r.r3c1,
    l.r0c0*r.r0c2 + l.r0c1*r.r1c2 + l.r0c2*r.r2c2 + l.r0c3*r.r3c2,
    l.r0c0*r.r0c3 + l.r0c1*r.r1c3 + l.r0c2*r.r2c3 + l.r0c3*r.r3c3,
    l.r1c0*r.r0c0 + l.r1c1*r.r1c0 + l.r1c2*r.r2c0 + l.r1c3*r.r3c0,
    l.r1c0*r.r0c1 + l.r1c1*r.r1c1 + l.r1c2*r.r2c1 + l.r1c3*r.r3c1,
    l.r1c0*r.r0c2 + l.r1c1*r.r1c2 + l.r1c2*r.r2c2 + l.r1c3*r.r3c2,
    l.r1c0*r.r0c3 + l.r1c1*r.r1c3 + l.r1c2*r.r2c3 + l.r1c3*r.r3c3,
    l.r2c0*r.r0c0 + l.r2c1*r.r1c0 + l.r2c2*r.r2c0 + l.r2c3*r.r3c0,
    l.r2c0*r.r0c1 + l.r2c1*r.r1c1 + l.r2c2*r.r2c1 + l.r2c3*r.r3c1,
    l.r2c0*r.r0c2 + l.r2c1*r.r1c2 + l.r2c2*r.r2c2 + l.r2c3*r.r3c2,
    l.r2c0*r.r0c3 + l.r2c1*r.r1c3 + l.r2c2*r.r2c3 + l.r2c3*r.r3c3,
    l.r3c0*r.r0c0 + l.r3c1*r.r1c0 + l.r3c2*r.r2c0 + l.r3c3*r.r3c0,
    l.r3c0*r.r0c1 + l.r3c1*r.r1c1 + l.r3c2*r.r2c1 + l.r3c3*r.r3c1,
    l.r3c0*r.r0c2 + l.r3c1*r.r1c2 + l.r3c2*r.r2c2 + l.r3c3*r.r3c2,
    l.r3c0*r.r0c3 + l.r3c1*r.r1c3 + l.r3c2*r.r2c3 + l.r3c3*r.r3c3);
}

inline matrix44& matrix44::set_rotation_component (const quaternion& q) {
    assert (q.is_unit());
    (*this)[0][0] = 1.0f - 2.0f* q.j*q.j - 2.0f* q.k*q.k;
    (*this)[1][0] = 2.0f* q.i*q.j - 2.0f* q.u*q.k;
    (*this)[2][0] = 2.0f* q.i*q.k + 2.0f* q.u*q.j;
    (*this)[3][0] = 0.0f;
    (*this)[0][1] = 2.0f* q.i*q.j + 2.0f* q.u*q.k;
    (*this)[1][1] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.k*q.k;
    (*this)[2][1] = 2.0f* q.j*q.k - 2.0f* q.u*q.i;
    (*this)[3][1] = 0.0f;
    (*this)[0][2] = 2.0f* q.i*q.k - 2.0f* q.u*q.j;
    (*this)[1][2] = 2.0f* q.j*q.k + 2.0f* q.u*q.i;
    (*this)[2][2] = 1.0f - 2.0f* q.i*q.i - 2.0f* q.j*q.j;
    (*this)[3][2] = 0.0f;
    (*this)[0][3] = 0.0f;
    (*this)[1][3] = 0.0f;
    (*this)[2][3] = 0.0f;
    (*this)[3][3] = 1.0f;
    return *this;
}

}

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
const quaternion quaternion::identity = quaternion { 0.0f, 0.0f, 0.0f, 1.0f };

const matrix33 matrix33::zero = matrix33 (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const matrix33 matrix33::identity = matrix33 (1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

const matrix43 matrix43::zero = matrix43 (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const matrix43 matrix43::identity = matrix43 (1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

const matrix44 matrix44::zero = matrix44 (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const matrix44 matrix44::identity = matrix44 (1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    

    


/*_                                                                       d
 ##_                                                                     d#
 NN#p                                                                  j0NN
 40NNh_                                                              _gN#B0
 4JF@NNp_                                                          _g0WNNL@
 JLE5@WRNp_                                                      _g@NNNF3_L
 _F`@q4WBN@Np_                                                _gNN@ZL#p"Fj_
 "0^#-LJ_9"NNNMp__                                         _gN#@#"R_#g@q^9"
 a0,3_j_j_9FN@N@0NMp__                                __ggNZNrNM"P_f_f_E,0a
  j  L 6 9""Q"#^q@NDNNNMpg____                ____gggNNW#W4p^p@jF"P"]"j  F
 rNrr4r*pr4r@grNr@q@Ng@q@N0@N#@NNMpmggggmqgNN@NN@#@4p*@M@p4qp@w@m@Mq@r#rq@r
   F Jp 9__b__M,Juw*w*^#^9#""EED*dP_@EZ@^E@*#EjP"5M"gM@p*Ww&,jL_J__f  F j
 -r#^^0""E" 6  q  q__hg-@4""*,_Z*q_"^pwr""p*C__@""0N-qdL_p" p  J" 3""5^^0r-
   t  J  __,Jb--N""",  *_s0M`""q_a@NW__JP^u_p"""p4a,p" _F""V--wL,_F_ F  #
 _,Jp*^#""9   L  5_a*N"""q__INr" "q_e^"*,p^""qME_ y"""p6u,f  j'  f "N^--LL_
    L  ]   k,w@#"""_  "_a*^E   ba-" ^qj-""^pe"  J^-u_f  _f "q@w,j   f  jL
    #_,J@^""p  `_ _jp-""q  _Dw^" ^cj*""*,j^  "p#_  y""^wE_ _F   F"^qN,_j
 w*^0   4   9__sAF" `L  _Dr"  m__m""q__a^"m__*  "qA_  j" ""Au__f   J   0^--
    ]   J_,x-E   3_  jN^" `u _w^*_  _RR_  _J^w_ j"  "pL_  f   7^-L_F   #
    jLs*^6   `_  _&*"  q  _,NF   "wp"  "*g"   _NL_  p  "-d_   F   ]"*u_F
 ,x-"F   ]    Ax^" q    hp"  `u jM""u  a^ ^, j"  "*g_   p  ^mg_   */


// ------------------------------------------------------------------------------------------------------------------ //
// Vector 3 inline definitions
// ------------------------------------------------------------------------------------------------------------------ //
vector3& vector3::operator*= (const quaternion& q) {
    float const xx = x - (2.0f * x * (q.j*q.j + q.k*q.k)) + (2.0f * y * (q.i*q.j - q.u*q.k)) + (2.0f * z * (q.i*q.k + q.u*q.j));
    float const yx = y + (2.0f * x * (q.i*q.j + q.u*q.k)) - (2.0f * y * (q.i*q.i + q.k*q.k)) + (2.0f * z * (q.j*q.k - q.u*q.i));
    float const zx = z + (2.0f * x * (q.i*q.k - q.u*q.j)) + (2.0f * y * (q.j*q.k + q.u*q.i)) - (2.0f * z * (q.i*q.i + q.j*q.j));
    x = xx; y = yx; z = zx; return *this;
}
vector3& vector3::operator*= (const matrix33& m) {
    float const xx = x*m.r0c0 + y*m.r1c0 + z*m.r2c0;
    float const yx = x*m.r0c1 + y*m.r1c1 + z*m.r2c1;
    float const zx = x*m.r0c2 + y*m.r1c2 + z*m.r2c2;
    x = xx; y = yx; z = zx; return *this;
}
vector3& vector3::operator*= (const matrix43& m) {
    float const xx = x*m.r0c0 + y*m.r1c0 + z*m.r2c0 + m.r3c0;
    float const yx = x*m.r0c1 + y*m.r1c1 + z*m.r2c1 + m.r3c1;
    float const zx = x*m.r0c2 + y*m.r1c2 + z*m.r2c2 + m.r3c2;
    x = xx; y = yx; z = zx; return *this;
}
vector3& vector3::operator*= (const matrix44& m) {
    float const xx = x*m.r0c0 + y*m.r1c0 + z*m.r2c0 + m.r3c0;
    float const yx = x*m.r0c1 + y*m.r1c1 + z*m.r2c1 + m.r3c1;
    float const zx = x*m.r0c2 + y*m.r1c2 + z*m.r2c2 + m.r3c2;
    x = xx; y = yx; z = zx; return *this;
}
    

// ------------------------------------------------------------------------------------------------------------------ //
// Quaternion inline definitions
// ------------------------------------------------------------------------------------------------------------------ //

quaternion& quaternion::concatenate(const quaternion& v) { // radians
    const float ix = u*v.i + i*v.u + k*v.j - j*v.k;
    const float jx = u*v.j + j*v.u + i*v.k - k*v.i;
    const float kx = u*v.k + k*v.u + j*v.i - i*v.j;
    const float ux = u*v.u - k*v.k - i*v.i + j*v.j;
    i = ix; j = jx; k = kx; u = ux; return *this;
}
void quaternion::get_axis_angle(vector3& axis, float& angle) const { // radians
    assert (is_unit());
    const float cos_angle = u;
    angle = 2.0f * acos(cos_angle); // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/
    float sin_angle = sqrt(1.0f - cos_angle * cos_angle);
    if (is_zero (sin_angle)) {
        sin_angle = 1.0f;
        axis = vector3::up;
    }
    else {
        axis.x = i / sin_angle;
        axis.y = j / sin_angle;
        axis.z = k / sin_angle;
        axis.normalise ();
        assert (axis.is_unit ());
    }
}
void quaternion::get_yaw_pitch_roll (vector3& angles) const { // radians
    assert (is_unit());
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
quaternion& quaternion::set_from_axis_angle (const vector3& axis, const float angle) {
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
quaternion& quaternion::set_from_yaw_pitch_roll (const float yaw, const float pitch, const float roll) { // radians
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
quaternion& quaternion::set_from_rotation (const matrix33& m) {
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

// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 33 inline definitions
// ------------------------------------------------------------------------------------------------------------------ //
matrix33& matrix33::operator*= (const matrix33& v) {
    assert (false);
    return *this;
}

matrix33& matrix33::orthonormalise() {
    up() = ~up();
    right() = ~(up() ^ backward());
    backward() = right() ^ up();
    assert (backward().is_unit());
    assert (is_orthonormal());
    return *this;
}
bool matrix33::is_orthonormal () const {
    for (int i = 0; i < 3; ++i) {
        if (!(*this)[i].is_unit()) return false;
        if (((*this)[i] | (*this)[(i+1) % 3]) > EPSILON) return false;
    } return true;
}
matrix33& matrix33::set_from_scale_factors (const vector3& f) {
    r0c0 = f.x;  r0c1 = 0.0f; r0c2 = 0.0f;
    r1c0 = 0.0f; r1c1 = f.y;  r1c2 = 0.0f;
    r2c0 = 0.0f; r2c1 = 0.0f; r2c2 = f.z;
    return *this;
}
matrix33& matrix33::set_from_x_axis_angle (const float theta) {
    const float s = sin (-theta);
    const float c = cos (-theta);
    r0c0 = 1.0f; r0c1 = 0.0f; r0c2 = 0.0f;
    r1c0 = 0.0f; r1c1 = c;    r1c2 = -s;
    r2c0 = 0.0f; r2c1 = s;    r2c2 =  c;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_y_axis_angle (const float theta) {
    const float s = sin (theta);
    const float c = cos (theta);
    r0c0 =  c;   r0c1 = 0.0f; r0c2 = s;
    r1c0 = 0.0f; r1c1 = 1.0f; r1c2 = 0.0f;
    r2c0 = -s;   r2c1 = 0.0f; r2c2 = c;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_z_axis_angle (const float theta) {
    const float s = sin (theta);
    const float c = cos (theta);
    r0c0 = c;    r0c1 = -s;   r0c2 = 0.0f;
    r1c0 = s;    r1c1 =  c;   r1c2 = 0.0f;
    r2c0 = 0.0f; r2c1 = 0.0f; r2c2 = 1.0f;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_yaw_pitch_roll (const float yaw, const float pitch, const float roll) {
    const float cx = cos (pitch);
    const float sx = sin (pitch);
    const float cy = cos (yaw);
    const float sy = sin (yaw);
    const float cz = cos (roll);
    const float sz = sin (roll);
    r0c0 =  (cx * cz) + (sx * sy * sz);
    r0c1 =  (cz * sx * sy) - (cx * sz);
    r0c2 =  (cy * sx);
    r1c0 =  (cy * sz);
    r1c1 =  (cy * cz);
    r1c2 =  -sy;
    r2c0 =  (cx * sy * sz) - (cz * sx);
    r2c1 =  (cx * cz * sy) + (sx * sz);
    r2c2 =  (cx * cy);
    assert (is_orthonormal());
    return *this;

}
matrix33& matrix33::set_from_axis_angle (const vector3& axis, const float angle) {
    assert (axis.is_unit());
    const float c = cos (angle);
    const float s = sin (angle);
    const float t = 1.0f - c;
    const float tx  = t  * axis.x; const float ty  = t  * axis.y; const float tz  = t  * axis.z;
    const float sx  = s  * axis.x; const float sy  = s  * axis.y; const float sz  = s  * axis.z;
    const float txy = tx * axis.y; const float tyz = tx * axis.z; const float txz = tx * axis.z;
    r0c0 = tx * axis.x + c;
    r0c1 = txy - sz;
    r0c2 = txz + sy;
    r1c0 = txy + sz;
    r1c1 = ty * axis.y + c;
    r1c2 = tyz - sx;
    r2c0 = txz - sy;
    r2c1 = tyz + sx;
    r2c2 = tz * axis.z + c;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_transform (const matrix43& m) {
    r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_transform (const matrix44& m) {
    r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;
    assert (is_orthonormal());
    return *this;
}
matrix33& matrix33::set_from_orientation (const quaternion& q) { // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
    assert (q.is_unit());
    r0c0 = 1.0f - 2.0f * (q.j*q.j + q.k*q.k);
    r0c1 =        2.0f * (q.i*q.j + q.u*q.k);
    r0c2 =        2.0f * (q.i*q.k - q.u*q.j);
    r1c0 =        2.0f * (q.i*q.j - q.u*q.k);
    r1c1 = 1.0f - 2.0f * (q.i*q.i + q.k*q.k);
    r1c2 =        2.0f * (q.j*q.k + q.u*q.i);
    r2c0 =        2.0f * (q.i*q.k + q.u*q.j);
    r2c1 =        2.0f * (q.j*q.k - q.u*q.i);
    r2c2 = 1.0f - 2.0f * (q.i*q.i + q.j*q.j);
    assert (is_orthonormal());
    //orthonormalise (); // would prefer to just assert here, but I keep hitting it :/
    return *this;
}

// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 43 inline definitions
// ------------------------------------------------------------------------------------------------------------------ //
    
matrix43& matrix43::operator*= (const matrix33& v) {
    assert (false);
    return *this;
}
matrix43& matrix43::operator*= (const matrix43& v) {
    assert (false);
    return *this;
}
matrix43& matrix43::set_from_transform (const matrix44& m) {
    r0c0=m.r0c0;r0c1=m.r0c1;r0c2=m.r0c2;r1c0=m.r1c0;r1c1=m.r1c1;r1c2=m.r1c2;
    r2c0=m.r2c0;r2c1=m.r2c1;r2c2=m.r2c2;r3c0=m.r3c0;r3c1=m.r3c1;r3c2=m.r3c2;
    return *this;
}
matrix43& matrix43::set_rotation_component (const quaternion& q) {
    assert (q.is_unit());
    r0c0 = 1.0f - 2.0f * (q.j*q.j + q.k*q.k);
    r0c1 =        2.0f * (q.i*q.j + q.u*q.k);
    r0c2 =        2.0f * (q.i*q.k - q.u*q.j);
    r1c0 =        2.0f * (q.i*q.j - q.u*q.k);
    r1c1 = 1.0f - 2.0f * (q.i*q.i + q.k*q.k);
    r1c2 =        2.0f * (q.j*q.k + q.u*q.i);
    r2c0 =        2.0f * (q.i*q.k + q.u*q.j);
    r2c1 =        2.0f * (q.j*q.k - q.u*q.i);
    r2c2 = 1.0f - 2.0f * (q.i*q.i + q.j*q.j);
    r3c0 = 0.0f;
    r3c1 = 0.0f;
    r3c2 = 0.0f;
    return *this;
}
// ------------------------------------------------------------------------------------------------------------------ //
// Matrix 44 inline definitions
// ------------------------------------------------------------------------------------------------------------------ //

matrix44& matrix44::operator*= (const matrix33& v) {
    assert (false);
    return *this;
}
matrix44& matrix44::operator*= (const matrix43& v) {
    assert (false);
    return *this;
}
matrix44& matrix44::operator*= (const matrix44& v) {
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
matrix44& matrix44::transpose () {
    float t = r0c1;
    r0c1 = r1c0; r1c0 = t;
    t = r0c2;
    r0c2 = r2c0; r2c0 = t;
    t = r0c3;
    r0c3 = r3c0; r3c0 = t;
    t = r1c2;
    r1c2 = r2c1; r2c1 = t;
    t = r1c3;
    r1c3 = r3c1; r3c1 = t;
    t =  r2c3;
    r2c3 = r3c2; r3c2 = t;
    return *this;
}
matrix44& matrix44::inverse () {
    assert (false);
    return *this;
}
matrix44& matrix44::affine_inverse () {
    const matrix44 cp = *this;
    const float i = cp.r1c1*cp.r2c2 - cp.r2c1*cp.r1c2;
    const float j = cp.r2c0*cp.r1c2 - cp.r1c0*cp.r2c2;
    const float k = cp.r1c0*cp.r2c1 - cp.r2c0*cp.r1c1;
    const float det = cp.r0c0*i + cp.r0c1*j + cp.r0c2*k;
    assert (!is_zero (det));
    const float invDet = 1.0f / det;
    r0c0 = invDet*i;
    r1c0 = invDet*j;
    r2c0 = invDet*k;
    r0c1 = invDet*(cp.r2c1*cp.r0c2 - cp.r0c1*cp.r2c2);
    r1c1 = invDet*(cp.r0c0*cp.r2c2 - cp.r2c0*cp.r0c2);
    r2c1 = invDet*(cp.r2c0*cp.r0c1 - cp.r0c0*cp.r2c1);
    r0c2 = invDet*(cp.r0c1*cp.r1c2 - cp.r1c1*cp.r0c2);
    r1c2 = invDet*(cp.r1c0*cp.r0c2 - cp.r0c0*cp.r1c2);
    r2c2 = invDet*(cp.r0c0*cp.r1c1 - cp.r1c0*cp.r0c1);
    r0c3 = -r0c0*cp.r0c3 - r0c1*cp.r1c3 - r0c2*cp.r2c3;
    r1c3 = -r1c0*cp.r0c3 - r1c1*cp.r1c3 - r1c2*cp.r2c3;
    r2c3 = -r2c0*cp.r0c3 - r2c1*cp.r1c3 - r2c2*cp.r2c3;
    return *this;
}
matrix44& matrix44::decompose () {
    assert (false);
    return *this;
}
float matrix44::determinant () const { return
      r0c3 * r1c2 * r2c1 * r3c0
    - r0c2 * r1c3 * r2c1 * r3c0
    - r0c3 * r1c1 * r2c2 * r3c0
    + r0c1 * r1c3 * r2c2 * r3c0
    + r0c2 * r1c1 * r2c3 * r3c0
    - r0c1 * r1c2 * r2c3 * r3c0
    - r0c3 * r1c2 * r2c0 * r3c1
    + r0c2 * r1c3 * r2c0 * r3c1
    + r0c3 * r1c0 * r2c2 * r3c1
    - r0c0 * r1c3 * r2c2 * r3c1
    - r0c2 * r1c0 * r2c3 * r3c1
    + r0c0 * r1c2 * r2c3 * r3c1
    + r0c3 * r1c1 * r2c0 * r3c2
    - r0c1 * r1c3 * r2c0 * r3c2
    - r0c3 * r1c0 * r2c1 * r3c2
    + r0c0 * r1c3 * r2c1 * r3c2
    + r0c1 * r1c0 * r2c3 * r3c2
    - r0c0 * r1c1 * r2c3 * r3c2
    - r0c2 * r1c1 * r2c0 * r3c3
    + r0c1 * r1c2 * r2c0 * r3c3
    + r0c2 * r1c0 * r2c1 * r3c3
    - r0c0 * r1c2 * r2c1 * r3c3
    - r0c1 * r1c0 * r2c2 * r3c3
    + r0c0 * r1c1 * r2c2 * r3c3;
}
matrix44& matrix44::set_rotation_component (const quaternion& q) {
    assert (q.is_unit());
    r0c0 = 1.0f - 2.0f * (q.j*q.j + q.k*q.k);
    r0c1 =        2.0f * (q.i*q.j + q.u*q.k);
    r0c2 =        2.0f * (q.i*q.k - q.u*q.j);
    r0c3 = 0.0f;
    r1c0 =        2.0f * (q.i*q.j - q.u*q.k);
    r1c1 = 1.0f - 2.0f * (q.i*q.i + q.k*q.k);
    r1c2 =        2.0f * (q.j*q.k + q.u*q.i);
    r1c3 = 0.0f;
    r2c0 =        2.0f * (q.i*q.k + q.u*q.j);
    r2c1 =        2.0f * (q.j*q.k - q.u*q.i);
    r2c2 = 1.0f - 2.0f * (q.i*q.i + q.j*q.j);
    r2c3 = 0.0f;
    r3c0 = 0.0f;
    r3c1 = 0.0f;
    r3c2 = 0.0f;
    r3c3 = 1.0f;
    return *this;
}
// Builds a perspective projection matrix based on a field of view. After the projection transformation, visible content has x- and y-coordinates ranging from −1 to 1, and a z-coordinate ranging from 0 to 1.
matrix44& matrix44::set_as_perspective_from_field_of_view (const float fov, const float aspect, const float near, const float far) {
    // http://msdn.microsoft.com/en-us/library/bb205351(v=vs.85).aspx
    // xScale     0          0              0
    // 0        yScale       0              0
    // 0        0        zf/(zn-zf)        -1
    // 0        0        zn*zf/(zn-zf)      0
    // where:
    // yScale = cot(fovY/2)
    // xScale = yScale / aspect ratio
    
    const float y_scale = 1.0f / tan (fov * 0.5f);
    const float x_scale = y_scale / aspect;

    r0c0 = x_scale; r0c1 = 0.0f;    r0c2 = 0.0f;                  r0c3 = 0.0f;
    r1c0 = 0.0f;    r1c1 = y_scale; r1c2 = 0.0f;                  r1c3 = 0.0f;
    r2c0 = 0.0f;    r2c1 = 0.0f;    r2c2 = far/(near-far);        r2c3 = -1.0f;
    r3c0 = 0.0f;    r3c1 = 0.0f;    r3c2 = (near*far)/(near-far); r3c3 = 0.0f;
    return *this;
}
matrix44& matrix44::set_as_view_transform_from_look_at_target (vector3 cam_pos, vector3 cam_target, vector3 cam_up) {
    set_as_view_frame_from_look_at_target (cam_pos, cam_target, cam_up);
    affine_inverse();
    return *this;
}
matrix44& matrix44::set_as_view_frame_from_look_at_target (vector3 cam_pos, vector3 cam_target, vector3 cam_up) {
    const vector3 view_dir = ~(cam_target - cam_pos); // vector: eye -> target (viewing down the negative Z axis).
    const vector3 view_right = ~(cam_up ^ -view_dir);
    const vector3 view_up = ~(view_dir ^ -view_right);
    // view frame is right handed like OpenGL (non-standard) which means the camera looks down the negative Z axis.
    // we invert the view direction as per pg.206 of essential mathematics for games.
    r0c0 = view_right.x; r0c1 = view_up.x; r0c2 = -view_dir.x; r0c3 = 0.0f;
    r1c0 = view_right.y; r1c1 = view_up.y; r1c2 = -view_dir.y; r1c3 = 0.0f;
    r2c0 = view_right.z; r2c1 = view_up.z; r2c2 = -view_dir.z; r2c3 = 0.0f;
    r3c0 = cam_pos.x;    r3c1 = cam_pos.y; r3c2 = cam_pos.z;   r3c3 = 1.0f;
    return *this;
}


        
        
        
        
        
        
    

    
// ------------------------------------------------------------------------------------------------------------------ //
#if 1
tests::tests() {
    {   // cross product (right handed)
        assert ((vector3::up ^ vector3::forward) == vector3::left);
        assert ((vector3::up ^ vector3::left) == vector3::backward);
        assert ((vector3::up ^ vector3::backward) == vector3::right);
        assert ((vector3::up ^ vector3::right) == vector3::forward);
        
        assert ((vector3::down ^ vector3::forward) == vector3::right);
        assert ((vector3::down ^ vector3::left) == vector3::forward);
        assert ((vector3::down ^ vector3::backward) == vector3::left);
        assert ((vector3::down ^ vector3::right) == vector3::backward);
        
        assert ((vector3::right ^ vector3::forward) == vector3::up);
        assert ((vector3::right ^ vector3::down) == vector3::forward);
        assert ((vector3::right ^ vector3::backward) == vector3::down);
        assert ((vector3::right ^ vector3::up) == vector3::backward);
        
        assert ((vector3::left ^ vector3::forward) == vector3::down);
        assert ((vector3::left ^ vector3::down) == vector3::backward);
        assert ((vector3::left ^ vector3::backward) == vector3::up);
        assert ((vector3::left ^ vector3::up) == vector3::forward);
        
        assert ((vector3::backward ^ vector3::left) == vector3::down);
        assert ((vector3::backward ^ vector3::down) == vector3::right);
        assert ((vector3::backward ^ vector3::right) == vector3::up);
        assert ((vector3::backward ^ vector3::up) == vector3::left);
        
        assert ((vector3::forward ^ vector3::left) == vector3::up);
        assert ((vector3::forward ^ vector3::down) == vector3::left);
        assert ((vector3::forward ^ vector3::right) == vector3::down);
        assert ((vector3::forward ^ vector3::up) == vector3::right);
    }
    { // fluent chains
         matrix44 view0 = matrix44()
            .set_rotation_component(quaternion::identity)
            .set_position_component({2, 3, 4});

        matrix44 expected = matrix44::identity;
        expected.r3c0 = 2;
        expected.r3c1 = 3;
        expected.r3c2 = 4;
        assert (view0 == expected);
    }
    { // view matrix
        const vector3 obj_pos = { 1, 2, 3 };
        const vector3 cam_pos = { 0, 0, 10 };
        const matrix44 view2world = matrix44().set_as_view_frame_from_look_at_target (cam_pos, obj_pos, vector3::up);
        const matrix44 world2view = matrix44().set_as_view_transform_from_look_at_target (cam_pos, obj_pos, vector3::up);
        assert (view2world != world2view);
        matrix44 view2world2 = world2view;
        matrix44 world2view2 = view2world;
        view2world2.affine_inverse();
        world2view2.affine_inverse();
        assert (view2world == view2world2);
        assert (world2view == world2view2);
    }
    { // quaternion vector transformations
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::forward);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::left);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::backward);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0,             0, 0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::forward);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::left);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::backward);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::right);
        
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0,             0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, PI,            0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::right);
        
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::up);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::left);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::down);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::right);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::up);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::left);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::down);
        assert (vector3::right * quaternion().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::right);
        
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0,             0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::up);
        
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::backward);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::down);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::forward);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0,             0) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::backward);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, PI,            0) == vector3::down);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::forward);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::up);
        
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::left);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::down);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::right);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::up);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::left);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::down);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::right);
        assert (vector3::up * quaternion().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::up);
        
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(-TAU,          0, 0) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0) == vector3::right);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(-PI,           0, 0) == vector3::forward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(-HALF_PI,      0, 0) == vector3::left);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0,             0, 0) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(HALF_PI,       0, 0) == vector3::right);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(PI,            0, 0) == vector3::forward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(3.0f*HALF_PI,  0, 0) == vector3::left);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(TAU,           0, 0) == vector3::backward);
        
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, -TAU,          0) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, -3.0f*HALF_PI, 0) == vector3::down);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, -PI,           0) == vector3::forward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, -HALF_PI,      0) == vector3::up);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0,             0) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, HALF_PI,       0) == vector3::down);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, PI,            0) == vector3::forward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 3.0f*HALF_PI,  0) == vector3::up);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, TAU,           0) == vector3::backward);
        
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, -TAU         ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, -3.0f*HALF_PI) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, -PI          ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, -HALF_PI     ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, 0            ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, HALF_PI      ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, PI           ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, 3.0f*HALF_PI ) == vector3::backward);
        assert (vector3::backward * quaternion().set_from_yaw_pitch_roll(0, 0, TAU          ) == vector3::backward);
        
    }
    { // matrix33 vector transformations
        assert (vector3::right * matrix33().set_from_y_axis_angle(-TAU         ) == vector3::right);
        auto t0 = matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0);
        auto t1 = matrix33().set_from_orientation(quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0));
        auto t0x = vector3::right * t0;
        auto t1x = vector3::right * t1;
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
    {
        const auto q = matrix33().set_from_orientation(quaternion().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0));
        const auto m = matrix33().set_from_yaw_pitch_roll(-3.0f*HALF_PI, 0, 0);
        assert (m == q);
    }
    { // view matrix from look at
        const vector3 obj_pos = { 0, 0, 0 };
        const vector3 cam_pos = { 0, 0, 10 };
        const matrix44 view = matrix44().set_as_view_transform_from_look_at_target (cam_pos, obj_pos, vector3::up);
        
        matrix44 view0 = matrix44::identity;
        view0.r3c0 = 0;
        view0.r3c1 = 0;
        view0.r3c2 = -10;
        
        const vector3 obj_pos_vs = obj_pos * view;
        const vector3 expected = vector3 { 0, 0, -10 };
        assert (obj_pos_vs == expected);
        
        
        const vector3 cam_pos_vs = cam_pos * view;
        const vector3 expected2 = vector3 { 0, 0, 0 };
        assert (cam_pos_vs == expected2);
    }
}

tests::~tests() {}
    
tests run;


#endif
}

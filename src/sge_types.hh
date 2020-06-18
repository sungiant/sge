#pragma once

// SGE-TYPES
// -------------------------------------
// Common types accessible to all areas
// of the engine, e.g. host, app,
// runtime, core.
// -------------------------------------
// todo: Review need for this - perhaps it'd be better to avoid having concrete types spanning areas of the engine.

#include <variant>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>
#include <algorithm>

#include "sge_math.hh"

namespace sge {

struct dataspan {
    void* address;
    size_t size;
    bool operator == (const dataspan& s) const { return address == s.address && size == s.size; }
    bool operator != (const dataspan& s) const { return !(*this == s); }
};

template <typename T>
bool contains_value (std::vector<std::optional<T>> xs)
{
    return std::find_if (xs.begin (), xs.end (), [](std::optional<T> x) { return x.has_value ();  }) != xs.end ();
}

enum class input_control_identifier {
    // keyboard binary controls (virtual)
    kb_escape, kb_enter, kb_spacebar, kb_shift, kb_control, kb_alt, kb_backspace, kb_tab,                                                                           // Control
    kb_ins, kb_del, kb_home, kb_end, kb_page_up, kb_page_down, kb_right_click, kb_prt_sc, kb_pause,                                                                 // Navigation
    kb_up, kb_down, kb_left, kb_right,                                                                                                                              // Arrows
    kb_a, kb_b, kb_c, kb_d, kb_e, kb_f, kb_g, kb_h, kb_i, kb_j, kb_k, kb_l, kb_m, kb_n, kb_o, kb_p, kb_q, kb_r, kb_s, kb_t, kb_u, kb_v, kb_w, kb_x, kb_y, kb_z,     // Alphabet
    kb_0, kb_1, kb_2, kb_3, kb_4, kb_5, kb_6, kb_7, kb_8, kb_9,                                                                                                     // Numbers
    kb_plus, kb_minus, kb_comma, kb_period,                                                                                                                         // Operators
    kb_windows, kb_cmd,                                                                                                                                             // OS specific
    kb_f1, kb_f2, kb_f3, kb_f4, kb_f5, kb_f6, kb_f7, kb_f8, kb_f9, kb_f10, kb_f11, kb_f12,                                                                          // Functions
    kb_numpad_0, kb_numpad_1, kb_numpad_2, kb_numpad_3, kb_numpad_4, kb_numpad_5, kb_numpad_6, kb_numpad_7, kb_numpad_8, kb_numpad_9,                               // Numpad numbers
    kb_numpad_decimal, kb_numpad_divide, kb_numpad_multiply, kb_numpad_subtract, kb_numpad_add, kb_numpad_enter, kb_numpad_equals,                                  // Numpad operators
    // keyboard character controls - only support maximum of ten keys pressed at once
    kc_0, kc_1, kc_2, kc_3, kc_4, kc_5, kc_6, kc_7, kc_8, kc_9,
    // keyboard quaternary controls
    kq_caps_lk, kq_scr_lk, kq_num_lk,
    // mouse binary controls
    mb_left, mb_middle, mb_right,
    // mouse point controls
    mp_position,
    // mouse digital controls
    md_scrollwheel,
    // gamepad binary controls
    gb_dpad_up, gb_dpad_down, gb_dpad_left, gb_dpad_right, gb_start, gb_back, gb_left_thumb, gb_right_thumb, gb_left_shoulder, gb_right_shoulder, gb_a, gb_b, gb_x, gb_y,
    // gamepad analog controls
    ga_left_stick_x, ga_left_stick_y, ga_left_trigger, ga_right_stick_x, ga_right_stick_y, ga_right_trigger,
    // touch pad digital controls - only support maximum of ten touches at once
    tp_0, tp_1, tp_2, tp_3, tp_4, tp_5, tp_6, tp_7, tp_8, tp_9,

    COUNT,
    INVALID
};

// Devices: [k] keyboard, [g] gamepad, [m] mouse, [t] touchpad
// Input types: [u] unary, [b] binary, [q] quaternary, [c] character, [d] digital, [p] point, [a] analog

// Used for normal buttons/keys
// - if (!present) key either doesn't exist or is not pressed
// - if (present && true) key is pressed
// - if (present && false) key is not pressed
typedef bool                        input_binary_control;

// Used for locking buttons/keys, i.e. caps lock
// - if (!present) key either doesn't exist or is not locked or is not pressed
// - if (present && first) key is locked (light is on)
// - if (present && !first) key is not locked (light is off)
// - if (present && second) key is pressed
// - if (present && !second) key is not pressed
typedef std::pair<bool, bool>       input_quaternary_control;

typedef wchar_t                     input_character_control;
typedef int                         input_digital_control;
typedef sge::math::point2           input_point_control;
typedef float                       input_analog_control;

typedef std::variant<input_binary_control, input_quaternary_control, input_character_control, input_digital_control, input_point_control, input_analog_control> input_control_value;

// todo: use std::common_type to pack variants into a uint64_t
typedef std::unordered_map<input_control_identifier, input_control_value> input_state;

}

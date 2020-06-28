#pragma once

#include "sge.hh"

#include "sge_math.hh"

// SGE-RUNTIME
// ---------------------------------- //
// Definitions for SGE's low level
// runtime interface.
// ---------------------------------- //
// * A thin layer over sge::core to avoid the need for direct user access.
// * SGE systems/views are built on top of this API.

namespace sge::runtime {

enum class keyboard_key {
    escape, enter, spacebar, shift, control, alt, backspace, tab,
    ins, del, home, end, page_up, page_down, right_click, prt_sc, pause,
    up, down, left, right,
    a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
    zero, one, two, three, four, five, six, seven, eight, nine,
    plus, minus, comma, period,
    windows, cmd,
    f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
    numpad_0, numpad_1, numpad_2, numpad_3, numpad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
    numpad_decimal, numpad_divide, numpad_multiply, numpad_subtract, numpad_add, numpad_enter, numpad_equals,
    COUNT
};

enum class keyboard_character   { COUNT = 10 }; // todo: this is a hack - the information is in core.hh (num keyboard character controls)
enum class keyboard_lock        { caps_lk, scr_lk, num_lk, COUNT };

enum class mouse_button         { left, middle, right, COUNT };
    
enum class gamepad_button       { dpad_up, dpad_down, dpad_left, dpad_right, back, center, start, left_thumb, right_thumb, left_shoulder, right_shoulder, a, b, x, y, COUNT };
enum class gamepad_axis         { left_trigger, right_trigger, left_stick_vertical, left_stick_horizontal, right_stick_vertical, right_stick_horizontal, COUNT };
    
enum class system_bool_state    { fullscreen, imgui, COUNT };
enum class system_int_state     { /* renderable area */ screenwidth, screenheight, /* fullscreen area */ displaywidth, displayheight, COUNT };
enum class system_string_state  { title, gpu_name, engine_version, COUNT };

class extension;

// the runtime api is a low level interface for interacting with SGE at runtime.
struct api {
    virtual ~api () {};
    
    // ----------------------------------
    
    virtual bool                    system__get_state_bool              (system_bool_state) const = 0;
    virtual int                     system__get_state_int               (system_int_state) const = 0;
    virtual const char*             system__get_state_string            (system_string_state) const = 0;
    virtual bool                    system__did_container_just_change   () const = 0;

    virtual uint32_t                timer__get_fps                      () const = 0;
    virtual float                   timer__get_delta                    () const = 0;
    virtual float                   timer__get_time                     () const = 0;

    virtual void                    input__keyboard_pressed_characters  (uint32_t*, wchar_t*) const = 0;
    virtual void                    input__keyboard_pressed_keys        (uint32_t*, keyboard_key*) const = 0;
    virtual void                    input__keyboard_pressed_locks       (uint32_t*, keyboard_lock*) const = 0;
    virtual void                    input__keyboard_locked_locks        (uint32_t*, keyboard_lock*) const = 0;
    virtual void                    input__mouse_pressed_buttons        (uint32_t*, mouse_button*) const = 0;
    virtual void                    input__mouse_position               (int*, int*) const = 0;
    virtual void                    input__mouse_scrollwheel            (int*) const = 0;
    virtual void                    input__gamepad_pressed_buttons      (uint32_t*, gamepad_button*) const = 0;
    virtual void                    input__gamepad_analogue_axes        (uint32_t*, gamepad_axis*, float*) const = 0;
    virtual void                    input__touches                      (uint32_t*, uint32_t*, int*, int*) const = 0;
    
    virtual extension*              extension_get                       (size_t) const = 0; // needs a better home...

    // The intention here is that every non-const function call to this api will be captured and enqueued for later processing.
    // ----------------------------------
    virtual void                    system__request_shutdown            () = 0;
    virtual void                    system__toggle_state_bool           (system_bool_state) = 0;
    virtual void                    system__set_state_bool              (system_bool_state, bool) = 0;
    virtual void                    system__set_state_int               (system_int_state, int) = 0;
    virtual void                    system__set_state_string            (system_string_state, const char*) = 0;
};


// todo: move extensions away from here.

template<typename T> struct type { static void id() {} };
template<typename T> size_t type_id() { return reinterpret_cast<size_t>(&type<T>::id); }

class extension {
public:
    virtual void update () {};
    virtual void debug_ui () {};
    virtual ~extension () {};
protected:
    extension () {};
};

// runtime views have readonly (const) access to the runtime api.
class view : public extension {
protected:
    const api& sge;
    view (const api& z) : sge (z) {}
public:
    virtual ~view () {};
};

// runtime systems have full access to the runtime api.
class system : public extension {
protected:
    api& sge;
    system (api& z) : sge (z) {}
public:
    virtual ~system () {};
};


}




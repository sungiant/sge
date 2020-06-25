// ---------------------------------- //
//   _________ ___________________    //
//  /   _____//  _____/\_   _____/    //
//  \_____  \/   \  ___ |    __)_     //
//  /        \    \_\  \|        \    //
// /_______  /\______  /_______  /    //
//         \/        \/  0.1.0 \/     //
// ---------------------------------- //
// Sungiant's game engine             //
// Copyright (C) 2020, A.J.Pook       //
// https://github.com/sungiant/sge    //
// ---------------------------------- //

#pragma once

#include <cmath>
#include <cassert>

#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <utility>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <functional>

#include <iostream>
#include <fstream>

#include "sge_math.hh"
#include "sge_utils.hh"

// ---------------------------------- //

#define SGE_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define SGE_VERSION SGE_MAKE_VERSION(0, 1, 0)
#define SGE_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define SGE_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define SGE_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

namespace sge {

struct dataspan {
    void* address;
    size_t size;
    bool operator == (const dataspan& s) const { return address == s.address && size == s.size; }
    bool operator != (const dataspan& s) const { return !(*this == s); }
};

template<typename T> struct type { static void id() {} };
template<typename T> size_t type_id() { return reinterpret_cast<size_t>(&type<T>::id); }

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

// SGE-RUNTIME
// ---------------------------------- //
// SGE's low level runtime interface.
// ---------------------------------- //
// * A thin layer over sge::core to avoid the need for direct user access.
// * SGE systems/views are built on top of this API.

namespace sge::runtime {

    
enum class system_bool_state {
    fullscreen,                         // run in fullscreen mode?
    imgui,                              // enable imgui
};

enum class system_int_state {
    screenwidth,                        // screen width
    screenheight,                       // screen height
};

enum class system_string_state {
    title,                              // window title
    gpu_name,
    engine_version,
};

class extension;

// the runtime api is a low level interface for interacting with SGE at runtime.
struct api {
    virtual ~api () {};
    
    virtual bool                    system__get_state_bool              (system_bool_state) const = 0;
    virtual int                     system__get_state_int               (system_int_state) const = 0;
    virtual const char*             system__get_state_string            (system_string_state) const = 0;
    virtual bool                    system__did_container_just_change   () const = 0;

    virtual uint32_t                timer__get_fps                      () const = 0;
    virtual float                   timer__get_delta                    () const = 0;
    virtual float                   timer__get_time                     () const = 0;

    virtual void                    input__get_state                    (input_state&) const = 0;

    // The intention is that every non-const function call to this api will be captured
    // and enqueued for later processing.
    virtual void                    system__request_shutdown            () = 0;
    virtual void                    system__toggle_state_bool           (system_bool_state) = 0;
    virtual void                    system__set_state_bool              (system_bool_state, bool) = 0;
    virtual void                    system__set_state_int               (system_int_state, int) = 0;
    virtual void                    system__set_state_string            (system_string_state, const char*) = 0;
    
    virtual extension*              extension_get                       (size_t) const = 0;

};

// runtime extensions are managed by the core engine and facilitiate the provision of high level
// functionality based on top of the runtime api.
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


// SGE-APP
// ---------------------------------- //
// The user interface into the engine.
// ---------------------------------- //
// ! Contains unimplemented functions which must be implemented by the user's application.

// Indicates that the function, implemented in user application space, will be called from SGE code.
#define SGE_CALL

namespace sge::app {

// An SGE app can be customised on initialisation with the settings in this stucture.
struct configuration {
    std::string app_name = "SGE";
    int app_width = 640;
    int app_height = 360;
    bool enable_console = false;
    bool ignore_os_dpi_scaling = false;
};

// The content of an SGE app is a computation defined by and app and managed by the engine.
// This structure is a collection of the information needed by the engine in order to managed an
// app's computation.
// Compute shaders must be of the form:
//  |#version 450
//  |layout (local_size_x = 16, local_size_y = 16) in;
//  |layout (binding = 0, rgba8) uniform writeonly image2D output;
//  |void main () {
//  |    imageStore (output, ivec2 (gl_GlobalInvocationID.xy), vec4 (0, 0, 0, 1));
//  |}
// Where the output format is currently fixed in the engine to be rgba8 - more flexibility
// to be added in due course, i.e. optional depth buffer output and alternative formats.
struct content {
    std::string shader_path;
    std::optional<dataspan> push_constants;
    std::vector<dataspan> uniforms;
    std::vector<dataspan> blobs; // todo: change to pair<dataspan, size_t> and make it possible to know the maximum size a blob could be over the full course of the app so we can allocate it on the gpu upfront.  right now when the user changes blob size at runtime the whole sbo is deallocated and reallocated to accomodate.
};
    
struct extensions {
    typedef std::function<runtime::view*(const runtime::api&)> create_view_fn;
    typedef std::function<runtime::system*(runtime::api&)> create_system_fn;
    
    std::vector<std::pair<size_t, create_view_fn>> views;
    std::vector<std::pair<size_t, create_system_fn>> systems;
};
    
struct api { // used by an SGE app to interact with SGE
    
    api (runtime::api& z_api) : runtime (z_api) {}
    
    runtime::api& runtime; // direct access to full low level runtime api (not really needed).

    // direct access to full low level runtime api (not really needed).
    template<typename T>
    T& ext () const {
        static_assert(std::is_base_of<runtime::extension, T>::value, "T must inherit from extension");
        size_t id = type_id <T>();
        runtime::extension* ext = runtime.extension_get (id);
        assert (ext);
        T* ext_t = static_cast <T*> (ext);
        assert (ext_t);
        return *ext_t;
    }
};


// TODO: Replace - see notes in sge_runtime.hh
struct response {
    bool request_shutdown;
    bool push_constants_changed;
    std::vector<bool> uniform_changes;
    std::vector<std::optional<dataspan>> blob_changes;
    response (int usz, int bsz): uniform_changes (usz), blob_changes (bsz) {}
};

// Called by the engine at ealiest point possible - use this to allocate.
SGE_CALL void initialise ();

// An app must be able to provide the engine with its desired configuration.
SGE_CALL configuration& get_configuration ();

// An app must be able to provide the engine with the details of its content.
SGE_CALL content& get_content ();

// An app must provide the any with the details needed to create any extensions it uses.
SGE_CALL extensions& get_extensions ();

// Called by the engine once it has been set up, and once only.
SGE_CALL void start (const api&);

// Called by the engine each frame.
// - The user_response structure enabled interaction with the running engine.
// - The user_interface structure allows readonly access to the engine.
SGE_CALL void update (response&, const api&);

// Called by the engine each frame - use this for ImGUI.
SGE_CALL void debug_ui (response&, const api&);

// Called by the engine once it has been shut down, and once only.
SGE_CALL void stop (const api&);

// Called by the engine at latest point possible - use this to deallocate.
SGE_CALL void terminate ();

}



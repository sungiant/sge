// SGE-CORE
// ---------------------------------- //
// The internals of the engine, not
// intended for broad direct access.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk.hh"

namespace sge::core {

// ! WARNING - ORDER HERE IS IMPORTANT
// ! Review the implementation of the input functions of the runtime api before making changes here.
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
    gb_dpad_up_0, gb_dpad_down_0, gb_dpad_left_0, gb_dpad_right_0, gb_start_0, gb_center_0, gb_back_0, gb_left_thumb_0, gb_right_thumb_0, gb_left_shoulder_0, gb_right_shoulder_0, gb_a_0, gb_b_0, gb_x_0, gb_y_0,
    gb_dpad_up_1, gb_dpad_down_1, gb_dpad_left_1, gb_dpad_right_1, gb_start_1, gb_center_1, gb_back_1, gb_left_thumb_1, gb_right_thumb_1, gb_left_shoulder_1, gb_right_shoulder_1, gb_a_1, gb_b_1, gb_x_1, gb_y_1,
    gb_dpad_up_2, gb_dpad_down_2, gb_dpad_left_2, gb_dpad_right_2, gb_start_2, gb_center_2, gb_back_2, gb_left_thumb_2, gb_right_thumb_2, gb_left_shoulder_2, gb_right_shoulder_2, gb_a_2, gb_b_2, gb_x_2, gb_y_2,
    gb_dpad_up_3, gb_dpad_down_3, gb_dpad_left_3, gb_dpad_right_3, gb_start_3, gb_center_3, gb_back_3, gb_left_thumb_3, gb_right_thumb_3, gb_left_shoulder_3, gb_right_shoulder_3, gb_a_3, gb_b_3, gb_x_3, gb_y_3,
    // gamepad analogue controls
    ga_left_stick_x_0, ga_left_stick_y_0, ga_left_trigger_0, ga_right_stick_x_0, ga_right_stick_y_0, ga_right_trigger_0,
    ga_left_stick_x_1, ga_left_stick_y_1, ga_left_trigger_1, ga_right_stick_x_1, ga_right_stick_y_1, ga_right_trigger_1,
    ga_left_stick_x_2, ga_left_stick_y_2, ga_left_trigger_2, ga_right_stick_x_2, ga_right_stick_y_2, ga_right_trigger_2,
    ga_left_stick_x_3, ga_left_stick_y_3, ga_left_trigger_3, ga_right_stick_x_3, ga_right_stick_y_3, ga_right_trigger_3,
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
typedef float                       input_analogue_control;

typedef std::variant<input_binary_control, input_quaternary_control, input_character_control, input_digital_control, input_point_control, input_analogue_control> input_control_value;

// todo: use std::common_type to pack variants into a uint64_t
typedef std::unordered_map<input_control_identifier, input_control_value> input_state;

struct container_state {
    bool is_resizing;
    int current_width;
    int current_height;
};

typedef std::function <void (void)>         void_fn;
typedef std::function <void (const char*)>  string_fn;
typedef std::function <void (bool)>         bool_fn;
typedef std::function <void (int, int)>     point_fn;

// collection of copided information about the host
// (the host is the authority on this data).
struct host_state {

    std::optional<string_fn>                set_window_title_fn;
    std::optional<bool_fn>                  set_window_fullscreen_fn;
    std::optional<point_fn>                 set_window_size_fn;
    std::optional<void_fn>                  shutdown_request_fn;

    std::string                             window_title;
    bool                                    is_fullscreen;
    bool                                    container_just_changed;
};

struct configuration_state {
    const float defaultClearColor [4] = { 0.025f, 0.025f, 0.025f, 1.0f };
};

struct platform_state {
#if TARGET_WIN32
    HINSTANCE hinst;
    HWND hwnd;
#elif TARGET_MACOSX
    void* view;
#elif TARGET_LINUX
    xcb_connection_t* connection;
    xcb_window_t window;
#endif
};

struct instrumentation_state {
    float frameTimer = 1.0f;
    float totalTimer = 0.0f;
    uint32_t frameCounter = 0;
    uint32_t lastFPS = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
};


typedef struct vk::vk graphics_state;

struct engine_state {

    std::string version;
    
    // engine input - replaced each frame by copied provided by the platform layer
    container_state container;
    input_state input;

    // engine constant data
    configuration_state configuration;
    platform_state platform;

    // engine state
    host_state host;
    instrumentation_state instrumentation;
    graphics_state graphics;

};

// Stuff for the engine to do when it gets round to it.
// Every user interaction with the engine at runtime should be here.
struct engine_tasks {
    std::optional<bool>                 change_imgui_enabled;
    std::optional<bool>                 change_fullscreen_enabled;
    std::optional<std::string>          change_window_title;
    std::optional<int>                  change_screen_width;
    std::optional<int>                  change_screen_height;
    std::optional<std::monostate>       shutdown_request;
};


//----------------------------------------------------------------------------------------------------------------//
    

// the runtime api is a low level interface for interacting with SGE at runtime.
class api_impl : public runtime::api {
    const core::engine_state& engine_state;
    core::engine_tasks& engine_tasks;
#if SGE_EXTENSIONS_ENABLED
    std::unordered_map<size_t, std::unique_ptr<runtime::extension>>& engine_extensions;
#endif
public:

    api_impl (const core::engine_state&, core::engine_tasks&
#if SGE_EXTENSIONS_ENABLED
        , std::unordered_map<size_t, std::unique_ptr<runtime::extension>>&
#endif
    );

    bool                    system__get_state_bool              (runtime::system_bool_state) const;
    int                     system__get_state_int               (runtime::system_int_state) const;
    const char*             system__get_state_string            (runtime::system_string_state) const;
    bool                    system__did_container_just_change   () const;

    uint32_t                timer__get_fps                      () const;
    float                   timer__get_delta                    () const;
    float                   timer__get_time                     () const;
    
    void                    input__keyboard_pressed_characters  (uint32_t*, wchar_t*) const;
    void                    input__keyboard_pressed_keys        (uint32_t*, runtime::keyboard_key*) const;
    void                    input__keyboard_pressed_locks       (uint32_t*, runtime::keyboard_lock*) const;
    void                    input__keyboard_locked_locks        (uint32_t*, runtime::keyboard_lock*) const;
    void                    input__mouse_pressed_buttons        (uint32_t*, runtime::mouse_button*) const;
    void                    input__mouse_position               (int*, int*) const;
    void                    input__mouse_scrollwheel            (int*) const;
    void                    input__gamepad_pressed_buttons      (uint32_t*, runtime::gamepad_button*) const;
    void                    input__gamepad_analogue_axes        (uint32_t*, runtime::gamepad_axis*, float*) const;
    void                    input__touches                      (uint32_t*, uint32_t*, int*, int*) const;
    

    // The intention is that every non-const function call to this api will be captured
    // and enqueued for later processing.
    void                    system__request_shutdown            ();
    void                    system__toggle_state_bool           (runtime::system_bool_state);
    void                    system__set_state_bool              (runtime::system_bool_state, bool);
    void                    system__set_state_int               (runtime::system_int_state, int);
    void                    system__set_state_string            (runtime::system_string_state, const char*);

#if SGE_EXTENSIONS_ENABLED
    runtime::extension*     extension_get                       (size_t) const;
#endif
    
    
};
    
//----------------------------------------------------------------------------------------------------------------//

class engine {

    std::unique_ptr<engine_state>                       engine_state;
    std::unique_ptr<engine_tasks>                       engine_tasks;
    std::unique_ptr<api_impl>                           engine_api;
#if SGE_EXTENSIONS_ENABLED
    std::unordered_map<size_t, std::unique_ptr<runtime::extension>> engine_extensions = {};
#endif
    std::unique_ptr<app::response>                      user_response;
    std::unique_ptr<app::api>                           user_api;
    std::vector<std::function<void ()>>                 debug_fns = {};

public:
    engine ();
    ~engine ();

    void register_set_window_title_callback (const string_fn& z) { engine_state->host.set_window_title_fn = z; }
    void register_set_window_fullscreen_callback (const bool_fn& z) { engine_state->host.set_window_fullscreen_fn = z; }
    void register_set_window_size_callback (const point_fn& z) { engine_state->host.set_window_size_fn = z; }
    void register_shutdown_request_callback (const void_fn& z) { engine_state->host.shutdown_request_fn = z; }

    void setup (
#if TARGET_WIN32
        HINSTANCE,
        HWND
#elif TARGET_MACOSX
        void*
#elif TARGET_LINUX
        xcb_connection_t*,
        xcb_window_t
#endif
    );

    void start ();
    void update (container_state&, input_state&);
    void stop ();

    void shutdown ();
};

}

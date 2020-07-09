#pragma once

#include "sge.hh"

#include "sge_math.hh"
#include "sge_utils.hh"

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
    
enum class system_bool_state    { fullscreen, imgui, hide_cusor, COUNT };
enum class system_int_state     {
    max_canvas_width, max_canvas_height,                    // maximum extent of the user renderable viewport on current monitor in the current scaling mode (not always the native resolution of the monitor), i.e. in fullscreen and full viewport mode (if the if the application moves to a different monitor of a different resolution/orientation this value will change).  this information can be useful to the user as it can help determine the velocity of mouse movements in proportion to context hosting the application.
    canvas_width, canvas_height,                            // current user renderable viewport extent
    canvas_offset_x, canvas_offset_y,                       // current user renderable viewport offset within parent container (ideally this wouldn't be here as the user shouldn't have to know about this detail - the user viewport is not always the full frame as the engine may instantiate UI around it, which is all fine, however, right now ImGui for the user is not aware of this).
    COUNT };

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
    //virtual void                    system__set_cursor                  (int, int, int, int, bool*, bool*) const = 0;

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
    virtual ~extension () {};
    
    void invoke_update () { update (); };
    
    void invoke_debug_menu () {
        if (utils::get_flag_at_mask (configuration_state, config_flags::MANAGED_DEBUG_UI)) {
            if (utils::get_flag_at_mask (runtime_state, runtime_flags::ACTIVE)) {
                if (ImGui::MenuItem (display_name.c_str(), NULL, utils::get_flag_at_mask (runtime_state, runtime_flags::MANAGED_DEBUG_UI_ACTIVE))) {
                    utils::toggle_flag_at_mask (runtime_state, runtime_flags::MANAGED_DEBUG_UI_ACTIVE);
                }
            }
        }
    }
    
    void invoke_debug_ui () {
        
        if (utils::get_flag_at_mask (configuration_state, config_flags::MANAGED_DEBUG_UI)) {
            const bool active = utils::get_flag_at_mask (runtime_state, runtime_flags::MANAGED_DEBUG_UI_ACTIVE);
            if (active) {
                bool open = active;
                ImGui::SetNextWindowPos(ImVec2 (100, 130), ImGuiCond_Once);
                ImGui::Begin (display_name.c_str(), &open, ImGuiWindowFlags_NoCollapse);
                managed_debug_ui ();
                ImGui::End();
                
                if (open != active)
                    utils::set_flag_at_mask (runtime_state, runtime_flags::MANAGED_DEBUG_UI_ACTIVE, open);
            }
        }
        
        if (utils::get_flag_at_mask (configuration_state, config_flags::CUSTOM_DEBUG_UI)) {
            if (utils::get_flag_at_mask (runtime_state, runtime_flags::CUSTOM_DEBUG_UI_ACTIVE))
                custom_debug_ui ();
        }
    };
    
    bool is_active () const { return utils::get_flag_at_mask (runtime_state, runtime_flags::ACTIVE); }
    void set_active (const bool v) { utils::set_flag_at_mask (runtime_state, runtime_flags::ACTIVE, v); }

    const std::string& get_display_name () const { return display_name; }
protected:
    
    extension (const uint32_t z_configuration, const std::string_view z_display_name)
        : configuration_state (z_configuration)
        , runtime_state (default_initial_runtime)
        , display_name (z_display_name) {}
    
    virtual void update () {};
    virtual void managed_debug_ui () {};
    virtual void custom_debug_ui () {};
    
    enum config_flags : uint8_t {
        MANAGED_DEBUG_UI = 1 << 0, // does the extension have a managed debug_ui? i.e.
        CUSTOM_DEBUG_UI  = 1 << 1, // does the extension have a custom debug_ui?
    };
    
    enum runtime_flags : uint8_t {
        ACTIVE                  = 1 << 0,  // is the extension active (some extensions are inactive by default - i.e. the freecam).
        MANAGED_DEBUG_UI_ACTIVE = 1 << 1,
        CUSTOM_DEBUG_UI_ACTIVE  = 1 << 2, /*FOO = 1 << 2, BAR = 1 << 3, FOO1 = 1 << 4, BAR1 = 1 << 5*/
    };
    
    const uint32_t configuration_state;
    uint32_t runtime_state;
    const std::string display_name;
    
    static const uint32_t default_configuration = MANAGED_DEBUG_UI;
    static const uint32_t default_initial_runtime = ACTIVE | CUSTOM_DEBUG_UI_ACTIVE;
};

// runtime views have readonly (const) access to the runtime api.
class view : public extension {
protected:
    const api& sge;
    view (const api& z, const std::string_view z_display_name) : extension (default_configuration, z_display_name), sge (z) {}
    view (const api& z, const std::string_view z_display_name, const uint32_t z_configuration) : extension (z_configuration, z_display_name), sge (z) {}
public:
    virtual ~view () {};
};

// runtime systems have full access to the runtime api.
class system : public extension {
protected:
    api& sge;
    system (api& z, const std::string_view z_display_name) : extension (default_configuration, z_display_name), sge (z) {}
    system (api& z, const std::string_view z_display_name, const uint32_t z_configuration)  : extension (z_configuration, z_display_name), sge (z) {}
public:
    virtual ~system () {};
};

}




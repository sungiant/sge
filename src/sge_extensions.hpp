#pragma once

// SGE-EXTENSIONS
// -------------------------------------
// Helper systems built on top of the
// SGE Runtime API.
// -------------------------------------
// * These systems know nothing about the core engine.

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <utility>
#include <optional>
#include <algorithm>
#include <functional>
#include <cctype>
#include <imgui/imgui.h>

#include "sge_version.hpp"
#include "sge_types.hpp"
#include "sge_runtime.hpp"
#include "sge_math.hpp"


// OVERLAY
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::overlay {

class system : public runtime::system {
    std::string sge_version;
public:
    system (runtime::api& z) : runtime::system (z), sge_version (sge::version ()) {}

    virtual void debug_ui () override {
        ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0));
        ImGui::Begin ("BACKGROUND", NULL,
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::SetWindowPos (ImVec2 (0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize (ImGui::GetIO ().DisplaySize);
        ImGui::SetWindowCollapsed (false);
        {
            int y = 20;
            int* py = &y;
            const int line_spacing = 14;
            std::function<void ()> next_line = [py]() {
                int yy = *py + line_spacing;
                *py = yy;
            };

            char text[128];
            sprintf (text, "SGE v%s", sge_version.c_str());
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            sprintf (text, "%d FPS", sge.timer__get_fps ());
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            sprintf (text, "Screen size: %d x %d", sge.system__get_state_int (sge::runtime::system_int_state::screenwidth), sge.system__get_state_int (sge::runtime::system_int_state::screenheight));
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            sprintf (text, "GPU: %s", sge.system__get_state_string (sge::runtime::system_string_state::gpu_name));
            ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            //sprintf (text, "Fullscreen: %s", sge.system__get_state_bool (sge::runtime::system_bool_state::fullscreen) ? "enabled" : "disabled");
            //ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            //sprintf (text, "Window title: %s", sge.system__get_state_string (sge::runtime::system_string_state::title));
            //ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

            //sprintf (text, "Did container just change: %s", sge.system__did_container_just_change () ? "true" : "false");
            //ImGui::GetWindowDrawList ()->AddText (ImVec2 (20, y), ImColor (0.0f, 1.0f, 0.0f, 1.0f), text); next_line ();

        }
        ImGui::End ();

        ImGui::PopStyleColor ();

        /*
        ImGui::Begin ("Tools");
        {
            if (ImGui::Button ("Toggle Fullscreen")) {
                sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
            }


            if (ImGui::Button ("Set 800x600")) {
                sge.system__set_state_int (sge::runtime::system_int_state::screenwidth, 800);
                sge.system__set_state_int (sge::runtime::system_int_state::screenheight, 600);
            }
        }
        ImGui::End ();
        */
    }
};

}


// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::instrumentation {

class view : public runtime::view {

public:
    view (const runtime::api& z) : runtime::view (z) {}

    uint32_t fps () const { return sge.timer__get_fps (); }
    float dt () const { return sge.timer__get_delta (); }
    float timer () const { return sge.timer__get_time (); }
};

}



// INPUT
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::input {

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
    enum class keyboard_lock {
        caps_lk, scr_lk, num_lk, COUNT
    };

    typedef wchar_t keyboard_character;

    enum class mouse_button { left, middle, right, COUNT };

    enum class gamepad_button { dpad_up, dpad_down, dpad_left, dpad_right, start, back, left_thumb, right_thumb, left_shoulder, right_shoulder, a, b, x, y, COUNT };

//--------------------------------------------------------------------------------------------------------------------//


    class keyboard {
        const input_state& current;
        const input_state& previous;

    public:
        keyboard (const input_state& z_current, const input_state& z_previous) : current (z_current), previous (z_previous) {}

        bool is_character_down          (wchar_t z)                 const { return get_character (z); }
        bool is_character_up            (wchar_t z)                 const { return !get_character (z); }
        bool was_character_down         (wchar_t z)                 const { return get_previous_character (z); }
        bool was_character_up           (wchar_t z)                 const { return !get_previous_character (z); }
        bool character_just_pressed     (wchar_t z)                 const { return is_character_down (z) && was_character_up (z); }
        bool character_just_released    (wchar_t z)                 const { return is_character_up (z) && was_character_down (z); }

        bool is_key_down                (keyboard_key z)            const { return current.find (convert (z)) != current.end (); }
        bool is_key_up                  (keyboard_key z)            const { return current.find (convert (z)) == current.end (); }
        bool was_key_down               (keyboard_key z)            const { return previous.find (convert (z)) != previous.end (); }
        bool was_key_up                 (keyboard_key z)            const { return previous.find (convert (z)) == previous.end (); }
        bool key_just_pressed           (keyboard_key z)            const { return is_key_down (z) && was_key_up (z); }
        bool key_just_released          (keyboard_key z)            const { return is_key_up (z) && was_key_down (z); }

        bool is_lock_locked             (keyboard_lock z)           const { return (current.find (convert (z)) != current.end ()) ? std::get<input_quaternary_control> (current.at (convert (z))).first : false; }
        bool is_lock_down               (keyboard_lock z)           const { return (current.find (convert (z)) != current.end ()) ? std::get<input_quaternary_control> (current.at (convert (z))).second : false; }

    private:

        input_control_identifier convert (keyboard_key kk) const {

#define SGE_X1(x) { case sge::input::keyboard_key::x: return input_control_identifier::kb_ ## x; }
#define SGE_X2(x, y) { case sge::input::keyboard_key::x: return input_control_identifier::kb_ ## y; }
            switch (kk) {
                SGE_X1 (escape); SGE_X1 (enter); SGE_X1 (spacebar); SGE_X1 (shift); SGE_X1 (control); SGE_X1 (alt); SGE_X1 (backspace); SGE_X1 (tab);
                SGE_X1 (ins); SGE_X1 (del); SGE_X1 (home); SGE_X1 (end); SGE_X1 (page_up); SGE_X1 (page_down); SGE_X1 (right_click); SGE_X1 (prt_sc); SGE_X1 (pause);
                SGE_X1 (up); SGE_X1 (down); SGE_X1 (left); SGE_X1 (right);
                SGE_X1 (a); SGE_X1 (b); SGE_X1 (c); SGE_X1 (d); SGE_X1 (e); SGE_X1 (f); SGE_X1 (g); SGE_X1 (h); SGE_X1 (i); SGE_X1 (j); SGE_X1 (k); SGE_X1 (l); SGE_X1 (m);
                SGE_X1 (n); SGE_X1 (o); SGE_X1 (p); SGE_X1 (q); SGE_X1 (r); SGE_X1 (s); SGE_X1 (t); SGE_X1 (u); SGE_X1 (v); SGE_X1 (w); SGE_X1 (x); SGE_X1 (y); SGE_X1 (z);
                SGE_X2 (zero, 0); SGE_X2 (one, 1); SGE_X2 (two, 2); SGE_X2 (three, 3); SGE_X2 (four, 4); SGE_X2 (five, 5); SGE_X2 (six, 6); SGE_X2 (seven, 7); SGE_X2 (eight, 8); SGE_X2 (nine, 9);
                SGE_X1 (plus); SGE_X1 (minus); SGE_X1 (comma); SGE_X1 (period);
                SGE_X1 (windows); SGE_X1 (cmd);
                SGE_X1 (f1); SGE_X1 (f2); SGE_X1 (f3); SGE_X1 (f4); SGE_X1 (f5); SGE_X1 (f6); SGE_X1 (f7); SGE_X1 (f8); SGE_X1 (f9); SGE_X1 (f10); SGE_X1 (f11); SGE_X1 (f12);
                SGE_X1 (numpad_0); SGE_X1 (numpad_1); SGE_X1 (numpad_2); SGE_X1 (numpad_3); SGE_X1 (numpad_4); SGE_X1 (numpad_5); SGE_X1 (numpad_6); SGE_X1 (numpad_7); SGE_X1 (numpad_8); SGE_X1 (numpad_9);
                SGE_X1 (numpad_decimal); SGE_X1 (numpad_divide); SGE_X1 (numpad_multiply); SGE_X1 (numpad_subtract); SGE_X1 (numpad_add); SGE_X1 (numpad_enter); SGE_X1 (numpad_equals);
                default: assert (false); return input_control_identifier::INVALID;
            }
        }
#undef SGE_X2
#undef SGE_X1

        input_control_identifier convert (keyboard_lock z) const {
            switch (z) {
                case sge::input::keyboard_lock::caps_lk: return input_control_identifier::kq_caps_lk;
                case sge::input::keyboard_lock::scr_lk: return input_control_identifier::kq_scr_lk;
                case sge::input::keyboard_lock::num_lk: return input_control_identifier::kq_num_lk;

                default: assert (false); return input_control_identifier::INVALID;
            }
        }

        std::optional<input_character_control> get_character_control (input_control_identifier z, bool z_previous) const {
            const input_state& is = z_previous ? previous : current;
            return (is.find (z) != is.end ())
                ? std::optional<input_character_control>(std::get<input_character_control> (is.at (z)))
                : std::nullopt;
        }

        void append_character_controls (std::vector<input_character_control>& z, bool z_previous) const {
            int first = static_cast<int>(input_control_identifier::kc_0);
            int last = static_cast<int>(input_control_identifier::kc_9);
            for (int i = first; i <= last; ++i) {
                auto id = static_cast<input_control_identifier> (i);
                auto cc = get_character_control (id, z_previous);
                if (cc.has_value ()) {
                    z.emplace_back (cc.value ());
                }
            }
        }

        bool get_character (wchar_t z, bool z_previous = false) const {
            std::vector<input_character_control> xs;
            append_character_controls (xs, z_previous);
            return std::find (xs.begin (), xs.end (), z) != xs.end ();
        }
        bool get_previous_character (wchar_t z) const { return get_character (z, true); }
    };

    //----------------------------------------------------------------------------------------------------------------//

    class mouse {
        const input_state& current;
        const input_state& previous;

    public:
        mouse (const input_state& z_current, const input_state& z_previous) : current (z_current), previous (z_previous) {}

        bool is_button_down             (mouse_button z)    const { return current.find (convert (z)) != current.end (); }
        bool is_button_up               (mouse_button z)    const { return current.find (convert (z)) == current.end (); }
        bool was_button_down            (mouse_button z)    const { return previous.find (convert (z)) != previous.end (); }
        bool was_button_up              (mouse_button z)    const { return previous.find (convert (z)) == previous.end (); }
        bool is_button_just_pressed     (mouse_button z)    const { return is_button_down (z) && was_button_up (z); }
        bool is_button_just_released    (mouse_button z)    const { return is_button_up (z) && was_button_down (z); }
        math::point2 position           ()                  const { return get_point_control (input_control_identifier::mp_position); }
        int scrollwheel                 ()                  const { return get_digital_control (input_control_identifier::md_scrollwheel); }
        int scrollwheel_delta           ()                  const { return get_digital_control_delta (input_control_identifier::md_scrollwheel); }

    private:

        input_control_identifier convert (mouse_button z) const {
            switch (z) {
                case sge::input::mouse_button::left: return input_control_identifier::mb_left;
                case sge::input::mouse_button::middle: return input_control_identifier::mb_middle;
                case sge::input::mouse_button::right: return input_control_identifier::mb_right;
                default: assert (false); return input_control_identifier::INVALID;
            }
        }

        input_digital_control get_digital_control (input_control_identifier z) const {
            return (current.find (z) != current.end ())
                ? std::get<input_digital_control> (current.at (z))
                : 0;
        }

        sge::math::point2 get_point_control (input_control_identifier z) const {
            return (current.find (z) != current.end ())
                ? std::get<input_point_control> (current.at (z))
                : sge::math::point2 {0, 0 };
        }

        input_digital_control get_digital_control_delta (input_control_identifier z) const {
            return (current.find (z) != current.end ())
                ? previous.find (z) != previous.end ()
                    ? std::get<input_digital_control> (current.at (z)) - std::get<input_digital_control> (previous.at (z))
                    : std::get<input_digital_control> (current.at (z))
                : 0;
        }
    };

    //----------------------------------------------------------------------------------------------------------------//

    class gamepad {
        const input_state& current;
        const input_state& previous;

    public:
        gamepad (const input_state& z_current, const input_state& z_previous) : current (z_current), previous (z_previous) {}

        bool is_button_down (gamepad_button z)              const { return current.find (convert (z)) != current.end (); }
        bool is_button_up (gamepad_button z)                const { return current.find (convert (z)) == current.end (); }
        bool was_button_down (gamepad_button z)             const { return previous.find (convert (z)) != previous.end (); }
        bool was_button_up (gamepad_button z)               const { return previous.find (convert (z)) == previous.end (); }
        bool is_button_just_pressed (gamepad_button z)      const { return is_button_down (z) && was_button_up (z); }
        bool is_button_just_released (gamepad_button z)     const { return is_button_up (z) && was_button_down (z); }
        math::vector2 left_stick ()                         const { auto x = get_analog_control (input_control_identifier::ga_left_stick_x), y = get_analog_control (input_control_identifier::ga_left_stick_y); return math::vector2{ x, y }; }
        math::vector2 right_stick ()                        const { auto x = get_analog_control (input_control_identifier::ga_right_stick_x), y = get_analog_control (input_control_identifier::ga_right_stick_y); return math::vector2{ x, y }; }
        float left_trigger ()                               const { return get_analog_control (input_control_identifier::ga_left_trigger); }
        float right_trigger ()                              const { return get_analog_control (input_control_identifier::ga_right_trigger); }

    private:

        input_control_identifier convert (gamepad_button z) const {
            switch (z) {
                case sge::input::gamepad_button::dpad_up: return input_control_identifier::gb_dpad_up;
                case sge::input::gamepad_button::dpad_down: return input_control_identifier::gb_dpad_down;
                case sge::input::gamepad_button::dpad_left: return input_control_identifier::gb_dpad_left;
                case sge::input::gamepad_button::dpad_right: return input_control_identifier::gb_dpad_right;
                case sge::input::gamepad_button::start: return input_control_identifier::gb_start;
                case sge::input::gamepad_button::back: return input_control_identifier::gb_back;
                case sge::input::gamepad_button::left_thumb: return input_control_identifier::gb_left_thumb;
                case sge::input::gamepad_button::right_thumb: return input_control_identifier::gb_right_thumb;
                case sge::input::gamepad_button::left_shoulder: return input_control_identifier::gb_left_shoulder;
                case sge::input::gamepad_button::right_shoulder: return input_control_identifier::gb_right_shoulder;
                case sge::input::gamepad_button::a: return input_control_identifier::gb_a;
                case sge::input::gamepad_button::b: return input_control_identifier::gb_b;
                case sge::input::gamepad_button::x: return input_control_identifier::gb_x;
                case sge::input::gamepad_button::y: return input_control_identifier::gb_y;
                default: assert (false); return input_control_identifier::INVALID;
            }
        }

        input_analog_control get_analog_control (input_control_identifier z) const {
            return (current.find (z) != current.end ())
                ? std::get<input_analog_control> (current.at (z))
                : 0;
        }
    };

    //----------------------------------------------------------------------------------------------------------------//

    class view : public runtime::view {
    private:
        input_state input_state_current;
        input_state input_state_previous;

    public:
        keyboard keyboard;
        mouse mouse;
        gamepad gamepad;

        view (const runtime::api& z)
            : runtime::view (z)
            , keyboard (input_state_current, input_state_previous)
            , mouse (input_state_current, input_state_previous)
            , gamepad (input_state_current, input_state_previous)
        {}

        virtual void update () override {
            input_state_previous = input_state_current; // copy

            sge.input__get_state (input_state_current);
        }

        virtual void debug_ui () override {

            ImGui::Begin ("SGE Input System");
            {
                ImGui::Text ("Keyboard");

                ImGui::Columns (3);
                int first = static_cast<int>(input_control_identifier::kc_0);
                int last = static_cast<int>(input_control_identifier::kc_9);
                int counter = 0;
                for (int i = first; i <= last; ++i) {
                    auto id = static_cast<input_control_identifier> (i);
                    if (input_state_current.find (id) != input_state_current.end ()) {
                        wchar_t c = std::get<input_character_control> (input_state_current.at (id));
                        ImGui::TextColored (ImGui::GetStyleColorVec4 (ImGuiCol_CheckMark), "character_%d: [%c]", counter++, static_cast<char>(c));
                    }
                    else {
                        ImGui::Text ("character_%d: ", counter++);
                    }
                    ImGui::NextColumn ();
                }


                ImGui::Columns (1);
                ImGui::Separator ();
                ImGui::Columns (3);

#define SGE_X(x) { bool key_ ## x = keyboard.is_key_down (keyboard_key::x); ImGui::Checkbox (#x, &key_ ## x); ImGui::NextColumn (); }
                SGE_X (escape); SGE_X (enter); SGE_X (spacebar); SGE_X (shift); SGE_X (control); SGE_X (alt); SGE_X (backspace); SGE_X (tab);
                SGE_X (ins); SGE_X (del); SGE_X (home); SGE_X (end); SGE_X (page_up); SGE_X (page_down); SGE_X (right_click); SGE_X (prt_sc); SGE_X (pause);
                SGE_X (up); SGE_X (down); SGE_X (left); SGE_X (right);
                SGE_X (a); SGE_X (b); SGE_X (c); SGE_X (d); SGE_X (e); SGE_X (f); SGE_X (g); SGE_X (h); SGE_X (i); SGE_X (j); SGE_X (k); SGE_X (l); SGE_X (m);
                SGE_X (n); SGE_X (o); SGE_X (p); SGE_X (q); SGE_X (r); SGE_X (s); SGE_X (t); SGE_X (u); SGE_X (v); SGE_X (w); SGE_X (x); SGE_X (y); SGE_X (z);
                SGE_X (zero); SGE_X (one); SGE_X (two); SGE_X (three); SGE_X (four); SGE_X (five); SGE_X (six); SGE_X (seven); SGE_X (eight); SGE_X (nine);
                SGE_X (plus); SGE_X (minus); SGE_X (comma); SGE_X (period);
                SGE_X (windows); SGE_X (cmd);
                SGE_X (f1); SGE_X (f2); SGE_X (f3); SGE_X (f4); SGE_X (f5); SGE_X (f6); SGE_X (f7); SGE_X (f8); SGE_X (f9); SGE_X (f10); SGE_X (f11); SGE_X (f12);
                SGE_X (numpad_0); SGE_X (numpad_1); SGE_X (numpad_2); SGE_X (numpad_3); SGE_X (numpad_4); SGE_X (numpad_5); SGE_X (numpad_6); SGE_X (numpad_7); SGE_X (numpad_8); SGE_X (numpad_9);
                SGE_X (numpad_decimal); SGE_X (numpad_divide); SGE_X (numpad_multiply); SGE_X (numpad_subtract); SGE_X (numpad_add); SGE_X (numpad_enter); SGE_X (numpad_equals);
#undef SGE_X

                ImGui::Columns (1);
                ImGui::Separator ();
                ImGui::Columns (3);

                ImGui::Text ("caps_lk"); ImGui::NextColumn ();
                bool key_caps_lk_pressed = keyboard.is_lock_down (keyboard_lock::caps_lk); ImGui::Checkbox ("pressed", &key_caps_lk_pressed); ImGui::NextColumn ();
                bool key_caps_lk_locked = keyboard.is_lock_locked (keyboard_lock::caps_lk); ImGui::Checkbox ("locked", &key_caps_lk_locked); ImGui::NextColumn ();

                ImGui::Text ("scr_lk"); ImGui::NextColumn ();
                bool key_scr_lk_pressed = keyboard.is_lock_down (keyboard_lock::scr_lk); ImGui::Checkbox ("pressed", &key_scr_lk_pressed); ImGui::NextColumn ();
                bool key_scr_lk_locked = keyboard.is_lock_locked (keyboard_lock::scr_lk); ImGui::Checkbox ("locked", &key_scr_lk_locked); ImGui::NextColumn ();

                ImGui::Text ("num_lk"); ImGui::NextColumn ();
                bool key_num_lk_pressed = keyboard.is_lock_down (keyboard_lock::num_lk); ImGui::Checkbox ("pressed", &key_num_lk_pressed); ImGui::NextColumn ();
                bool key_num_lk_locked = keyboard.is_lock_locked (keyboard_lock::num_lk); ImGui::Checkbox ("locked", &key_num_lk_locked); ImGui::NextColumn ();

                ImGui::Columns (1);

            }

            ImGui::Separator ();
            {
                ImGui::Text ("Mouse");

                ImGui::Columns (3);
                bool button_left = mouse.is_button_down (mouse_button::left); ImGui::Checkbox ("Left", &button_left);
                ImGui::NextColumn ();
                bool button_middle = mouse.is_button_down (mouse_button::middle); ImGui::Checkbox ("Middle", &button_middle);
                ImGui::NextColumn ();
                bool button_right = mouse.is_button_down (mouse_button::right); ImGui::Checkbox ("Right", &button_right);
                ImGui::Columns (1);
                int mouse_x = mouse.position ().x; ImGui::SliderInt ("Position X", &mouse_x, 0, 4096);
                int mouse_y = mouse.position ().y; ImGui::SliderInt ("Position Y", &mouse_y, 0, 4096);

                int mouse_scroll = mouse.scrollwheel (); ImGui::SliderInt ("Scroll Wheel", &mouse_scroll, -16384, 16384);
            }

            ImGui::Separator ();
            {
                ImGui::Text ("Gamepad");

                float left_thumbstick_x = gamepad.left_stick ().x; ImGui::SliderFloat ("Left Thumbstick X", &left_thumbstick_x, -1.0f, 1.0f);
                float left_thumbstick_y = gamepad.left_stick ().y; ImGui::SliderFloat ("Left Thumbstick Y", &left_thumbstick_y, -1.0f, 1.0f);

                float right_thumbstick_x = gamepad.right_stick ().x; ImGui::SliderFloat ("Right Thumbstick X", &right_thumbstick_x, -1.0f, 1.0f);
                float right_thumbstick_y = gamepad.right_stick ().y; ImGui::SliderFloat ("Right Thumbstick Y", &right_thumbstick_y, -1.0f, 1.0f);

                float left_trigger = gamepad.left_trigger (); ImGui::SliderFloat ("Left Trigger", &left_trigger, 0.0f, 1.0f);
                float right_trigger = gamepad.right_trigger (); ImGui::SliderFloat ("Right Trigger", &right_trigger, 0.0f, 1.0f);


                ImGui::Columns (4);

                bool button_a = gamepad.is_button_down (gamepad_button::a); ImGui::Checkbox ("A", &button_a);
                bool button_x = gamepad.is_button_down (gamepad_button::x); ImGui::Checkbox ("X", &button_x);
                bool button_y = gamepad.is_button_down (gamepad_button::y); ImGui::Checkbox ("Y", &button_y);
                bool button_b = gamepad.is_button_down (gamepad_button::b); ImGui::Checkbox ("B", &button_b);

                ImGui::NextColumn ();

                bool button_down = gamepad.is_button_down (gamepad_button::dpad_down);  ImGui::Checkbox ("DOWN", &button_down);
                bool button_left = gamepad.is_button_down (gamepad_button::dpad_left);  ImGui::Checkbox ("LEFT", &button_left);
                bool button_up = gamepad.is_button_down (gamepad_button::dpad_up);    ImGui::Checkbox ("Up", &button_up);
                bool button_right = gamepad.is_button_down (gamepad_button::dpad_right); ImGui::Checkbox ("RIGHT", &button_right);

                ImGui::NextColumn ();

                bool button_start = gamepad.is_button_down (gamepad_button::start); ImGui::Checkbox ("Start", &button_start);
                bool button_back = gamepad.is_button_down (gamepad_button::back);  ImGui::Checkbox ("Back", &button_back);

                ImGui::NextColumn ();

                bool button_lshoulder = gamepad.is_button_down (gamepad_button::left_shoulder);  ImGui::Checkbox ("L1", &button_lshoulder);
                bool button_rshoulder = gamepad.is_button_down (gamepad_button::right_shoulder); ImGui::Checkbox ("R1", &button_rshoulder);
                bool button_lthumb = gamepad.is_button_down (gamepad_button::left_thumb);     ImGui::Checkbox ("L3", &button_lthumb);
                bool button_rthumb = gamepad.is_button_down (gamepad_button::right_thumb);    ImGui::Checkbox ("R3", &button_rthumb);

                ImGui::Columns (1);
            }
            ImGui::End ();
        }
    };
}

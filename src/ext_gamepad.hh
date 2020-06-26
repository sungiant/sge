#if SGE_EXTENSIONS_ENABLED

#pragma once

#include "sge.hh"

namespace sge::ext {
    
class gamepad : public runtime::view {
    
public:
    
    bool is_button_down (runtime::gamepad_button z)             const { return buttons_current.find (z) != buttons_current.end (); }
    bool is_button_up (runtime::gamepad_button z)               const { return buttons_current.find (z) == buttons_current.end (); }
    bool was_button_down (runtime::gamepad_button z)            const { return buttons_previous.find (z) != buttons_previous.end (); }
    bool was_button_up (runtime::gamepad_button z)              const { return buttons_previous.find (z) == buttons_previous.end (); }
    bool is_button_just_pressed (runtime::gamepad_button z)     const { return is_button_down (z) && was_button_up (z); }
    bool is_button_just_released (runtime::gamepad_button z)    const { return is_button_up (z) && was_button_down (z); }
    math::vector2 left_stick ()                                 const { auto x = get_analog_control (runtime::gamepad_axis::left_stick_horizontal), y = get_analog_control (runtime::gamepad_axis::left_stick_vertical); return math::vector2{ x, y }; }
    math::vector2 right_stick ()                                const { auto x = get_analog_control (runtime::gamepad_axis::right_stick_horizontal), y = get_analog_control (runtime::gamepad_axis::right_stick_vertical); return math::vector2{ x, y }; }
    float left_trigger ()                                       const { return get_analog_control (runtime::gamepad_axis::left_trigger); }
    float right_trigger ()                                      const { return get_analog_control (runtime::gamepad_axis::right_trigger); }

private:
    std::unordered_set<runtime::gamepad_button> buttons_current;
    std::unordered_set<runtime::gamepad_button> buttons_previous;
    std::unordered_map<runtime::gamepad_axis, float> axes_current;

    float get_analog_control (runtime::gamepad_axis z) const {
        return (axes_current.find (z) != axes_current.end ())
            ? axes_current.at (z)
            : 0.0f;
    }

public:
    
    gamepad (const runtime::api& z) : runtime::view (z) {
        buttons_current.reserve((size_t) runtime::gamepad_button::COUNT);
        buttons_previous.reserve((size_t) runtime::gamepad_button::COUNT);
    }

    virtual void update () override {
        uint32_t sz = 0;
        
        { // buttons
            static std::array<runtime::gamepad_button, (size_t) runtime::gamepad_button::COUNT> buttons_arr;
            
            buttons_previous = buttons_current;
            buttons_current.clear();
            
            sge.input__gamepad_pressed_buttons (&sz, nullptr);
            assert (sz <= buttons_arr.size ());
            sge.input__gamepad_pressed_buttons (&sz, buttons_arr.data());
            
            for (uint32_t i = 0; i < sz; ++i)
                buttons_current.insert(buttons_arr[i]);
        }
        
        { // axes
            static std::array<runtime::gamepad_axis, (size_t) runtime::gamepad_axis::COUNT> axes_arr_keys;
            static std::array<float, (size_t) runtime::gamepad_axis::COUNT> axes_arr_values;
            
            sge.input__gamepad_analogue_axes (&sz, nullptr, nullptr);
            assert (sz <= axes_arr_keys.size () && sz < axes_arr_values.size ());
            sge.input__gamepad_analogue_axes (&sz, axes_arr_keys.data(), axes_arr_values.data());
            
            for (uint32_t i = 0; i < sz; ++i)
                axes_current[axes_arr_keys[i]] = axes_arr_values[i];
        }
    }

    virtual void debug_ui () override {
        ImGui::Begin ("Gamepad");
        
        float left_thumbstick_x = left_stick ().x; ImGui::SliderFloat ("Left Thumbstick X", &left_thumbstick_x, -1.0f, 1.0f);
        float left_thumbstick_y = left_stick ().y; ImGui::SliderFloat ("Left Thumbstick Y", &left_thumbstick_y, -1.0f, 1.0f);

        float right_thumbstick_x = right_stick ().x; ImGui::SliderFloat ("Right Thumbstick X", &right_thumbstick_x, -1.0f, 1.0f);
        float right_thumbstick_y = right_stick ().y; ImGui::SliderFloat ("Right Thumbstick Y", &right_thumbstick_y, -1.0f, 1.0f);

        float ltrigger = left_trigger (); ImGui::SliderFloat ("Left Trigger", &ltrigger, 0.0f, 1.0f);
        float rtrigger = right_trigger (); ImGui::SliderFloat ("Right Trigger", &rtrigger, 0.0f, 1.0f);

        ImGui::Columns (4);

        bool button_a = is_button_down (runtime::gamepad_button::a); ImGui::Checkbox ("A", &button_a);
        bool button_x = is_button_down (runtime::gamepad_button::x); ImGui::Checkbox ("X", &button_x);
        bool button_y = is_button_down (runtime::gamepad_button::y); ImGui::Checkbox ("Y", &button_y);
        bool button_b = is_button_down (runtime::gamepad_button::b); ImGui::Checkbox ("B", &button_b);

        ImGui::NextColumn ();

        bool button_down = is_button_down (runtime::gamepad_button::dpad_down); ImGui::Checkbox ("DOWN", &button_down);
        bool button_left = is_button_down (runtime::gamepad_button::dpad_left); ImGui::Checkbox ("LEFT", &button_left);
        bool button_up = is_button_down (runtime::gamepad_button::dpad_up); ImGui::Checkbox ("Up", &button_up);
        bool button_right = is_button_down (runtime::gamepad_button::dpad_right); ImGui::Checkbox ("RIGHT", &button_right);

        ImGui::NextColumn ();

        bool button_start = is_button_down (runtime::gamepad_button::start); ImGui::Checkbox ("Start", &button_start);
        bool button_back = is_button_down (runtime::gamepad_button::back); ImGui::Checkbox ("Back", &button_back);

        ImGui::NextColumn ();

        bool button_lshoulder = is_button_down (runtime::gamepad_button::left_shoulder); ImGui::Checkbox ("L1", &button_lshoulder);
        bool button_rshoulder = is_button_down (runtime::gamepad_button::right_shoulder); ImGui::Checkbox ("R1", &button_rshoulder);
        bool button_lthumb = is_button_down (runtime::gamepad_button::left_thumb); ImGui::Checkbox ("L3", &button_lthumb);
        bool button_rthumb = is_button_down (runtime::gamepad_button::right_thumb); ImGui::Checkbox ("R3", &button_rthumb);

        ImGui::Columns (1);
        
        ImGui::End ();
    }

};

}

#endif

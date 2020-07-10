#pragma once

#include "sge.hh"

namespace sge::ext {

class mouse : public runtime::view {

public:

    enum class proportion { screensize, displaysize };

    bool is_button_down             (runtime::mouse_button z)   const { return buttons_current.find (z) != buttons_current.end (); }
    bool is_button_up               (runtime::mouse_button z)   const { return buttons_current.find (z) == buttons_current.end (); }
    bool was_button_down            (runtime::mouse_button z)   const { return buttons_previous.find (z) != buttons_previous.end (); }
    bool was_button_up              (runtime::mouse_button z)   const { return buttons_previous.find (z) == buttons_previous.end (); }
    bool is_button_just_pressed     (runtime::mouse_button z)   const { return is_button_down (z) && was_button_up (z); }
    bool is_button_just_released    (runtime::mouse_button z)   const { return is_button_up (z) && was_button_down (z); }
    math::point2 position           ()                          const { return position_current; }
    math::point2 position_delta     ()                          const { return position_current - position_previous; }
    math::vector2 velocity          ()                          const { return math::vector2(position_delta ()) / current_dt; }

    math::vector2 position          (proportion p)              const { math::point2 d = position (); return sge::math::vector2 { (float) d.x / current_screenwidth, (float)d.y / current_screenheight }; }
    math::vector2 position_delta    (proportion p)              const { math::point2 d = position_delta (); return sge::math::vector2 { (float) d.x / current_screenwidth, (float)d.y / current_screenheight }; }
    math::vector2 velocity          (proportion p)              const { math::vector2 d = position_delta (p); return sge::math::vector2 { d.x / current_dt, d.y / current_dt }; }

    int scrollwheel                 ()                          const { return scrollwheel_current; }
    int scrollwheel_delta           ()                          const { return scrollwheel_current - scrollwheel_previous; }

private:
    std::unordered_set<runtime::mouse_button> buttons_current;
    std::unordered_set<runtime::mouse_button> buttons_previous;

    bool imgui_wants_mouse_current;
    bool imgui_wants_mouse_previous;

    math::point2 position_current = math::point2 { 0, 0 };
    math::point2 position_previous = math::point2 { 0, 0 };

    int scrollwheel_current = 0;
    int scrollwheel_previous = 0;

    int current_screenwidth;
    int current_screenheight;

    float current_dt;

public:

    mouse (const runtime::api& z) : runtime::view (z, "Mouse") {
        buttons_current.reserve((size_t) runtime::mouse_button::COUNT);
        buttons_previous.reserve((size_t) runtime::mouse_button::COUNT);
    }

    virtual void update () override {

        current_screenwidth = sge.system__get_state_int (sge::runtime::system_int_state::canvas_width);
        current_screenheight = sge.system__get_state_int (sge::runtime::system_int_state::canvas_height);
        current_dt = sge.timer__get_delta();

        imgui_wants_mouse_previous = imgui_wants_mouse_current;
        imgui_wants_mouse_current = ImGui::GetIO().WantCaptureMouse;

        { // buttons
            uint32_t sz = 0;
            static std::array<runtime::mouse_button, (size_t) runtime::mouse_button::COUNT> buttons_arr;

            buttons_previous = buttons_current;
            buttons_current.clear();

            if (!imgui_wants_mouse_current) { // only track pressed mouse buttons if imgui doesn't want to take the mouse input.

                sge.input__mouse_pressed_buttons (&sz, nullptr);
                assert (sz <= buttons_arr.size ());
                sge.input__mouse_pressed_buttons (&sz, buttons_arr.data());

                for (uint32_t i = 0; i < sz; ++i)
                    buttons_current.insert(buttons_arr[i]);
            }
        }

        { // position
            position_previous = position_current;
            sge.input__mouse_position(&position_current.x, &position_current.y);
        }

        { // scroll wheel
            if (!imgui_wants_mouse_current) { // only track the scroll wheel if imgui doesn't want to take the mouse input.
                scrollwheel_previous = scrollwheel_current;
                sge.input__mouse_scrollwheel(&scrollwheel_current);

                if (imgui_wants_mouse_previous) // avoid lurches in scroll wheel delta if imgui took the mouse input previously.
                    scrollwheel_previous = scrollwheel_current;
            }
        }
    }

    virtual void managed_debug_ui () override {
        ImGui::Columns (3);
        bool button_left = is_button_down (runtime::mouse_button::left); ImGui::Checkbox ("Left", &button_left);
        ImGui::NextColumn ();
        bool button_middle = is_button_down (runtime::mouse_button::middle); ImGui::Checkbox ("Middle", &button_middle);
        ImGui::NextColumn ();
        bool button_right = is_button_down (runtime::mouse_button::right); ImGui::Checkbox ("Right", &button_right);
        ImGui::Columns (1);
        int mouse_x = position ().x; ImGui::SliderInt ("Position X", &mouse_x, -4096, 4096);
        int mouse_y = position ().y; ImGui::SliderInt ("Position Y", &mouse_y, -4096, 4096);
        int mouse_scroll = scrollwheel (); ImGui::SliderInt ("Scroll Wheel", &mouse_scroll, -16384, 16384);
    }
};

}

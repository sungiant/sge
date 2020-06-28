#pragma once

#include <cmath>
#include <algorithm>

#include <sge_math.hh>
#include <sge_app.hh>

struct free_camera {
    free_camera () {
        float y = 4.0f;
        float zx = 6.0f;
        position = { zx, y, zx };
        orientation = sge::math::quaternion::create_from_yaw_pitch_roll(5.0f * 3.1415962f / 4.0f, (3.1415962f / 2.0f) - (atan(sqrt(zx * zx * 2.0f) / y)), 0.0f);
    }

    void update (const float dt, const sge::app::api::input_container& input) {

        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_left)) { fov -= dt * FOV_DEBUG_RATE; }
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_right)) {  fov += dt * FOV_DEBUG_RATE; }
        fov = std::clamp (fov, FOV_MIN, FOV_MAX);

        const float sp = input.keyboard.is_key_down(sge::runtime::keyboard_key::spacebar) ? 0.8f : 0.0f;
        const float r1 = input.gamepad.is_button_down(sge::runtime::gamepad_button::right_shoulder) ? 0.5f : 0.0f;
        const float r2 = input.gamepad.right_trigger();

        const float fast_factor = std::max (sp, std::max (r1, r2));
        const float traverse_rate = std::max(TRAVERSE_RATE, fast_factor * FAST_TRAVERSE_RATE) * (1.0f / traverse_sensitivity);
        const float look_rate = std::max (LOOK_RATE, fast_factor * FAST_LOOK_RATE) * (1.0f / look_sensitivity);

        
        // AXIS UP/DOWN
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_down)) { position.y -= traverse_rate * dt; }
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_up)) { position.y += traverse_rate * dt; }

        sge::math::vector3 eulerAngles;
        orientation.get_yaw_pitch_roll(eulerAngles);

        // YAW
        float rxx = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::left)) { rxx = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::right)) { rxx = +1.0f; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::right)) { rxx += input.mouse.velocity (sge::ext::mouse::proportion::displaysize).x * MOUSE_F; }
        if (abs(input.gamepad.right_stick().x) > abs (rxx)) { rxx = input.gamepad.right_stick().x; }
        if (!sge::math::is_zero (rxx)) {
            eulerAngles.x -=dt * look_rate * rxx;
        }

        // PITCH
        float rxy = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::down)) { rxy = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::up)) { rxy = +1.0f; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::right)) { rxy += input.mouse.velocity (sge::ext::mouse::proportion::displaysize).y * MOUSE_F; } // not inverting here as it feels wrong with the visible mouse cursor.
        if (abs(input.gamepad.right_stick().y) > abs (rxy)) { rxy = input.gamepad.right_stick().y; }
        if (!sge::math::is_zero (rxy)) {
            eulerAngles.y += dt * look_rate * rxy;
        }

        orientation = sge::math::quaternion::create_from_yaw_pitch_roll(eulerAngles.x, eulerAngles.y, eulerAngles.z);

        // LEFT/RIGHT
        {
        float f = 0.0f;
            if (input.keyboard.is_character_down('a')) { f = -1.0f; }
            if (input.keyboard.is_character_down('d')) { f = +1.0f; }
            if (input.mouse.is_button_down (sge::runtime::mouse_button::middle)) { f += input.mouse.velocity (sge::ext::mouse::proportion::displaysize).x * MOUSE_F; }
            if (abs(input.gamepad.left_stick().x) > abs (f)) { f = input.gamepad.left_stick().x; }
            if (!sge::math::is_zero (f)) {
                position -= sge::math::vector3::right * orientation * traverse_rate * f * dt;
            }
        }

        // FORWARD/BACKWARD
        {
            float f = 0.0f;
            if (input.keyboard.is_character_down('s')) { f = -1.0f; }
            if (input.keyboard.is_character_down('w')) { f = +1.0f; }
            int scroll = input.mouse.scrollwheel_delta ();
            if (scroll > 0) { f = +50.0f; }
            if (scroll < 0) { f = -50.0f; }
            if (abs(input.gamepad.left_stick().y) > abs (f)) { f = input.gamepad.left_stick().y; }
            if (!sge::math::is_zero (f)) {
                position -= sge::math::vector3::forward * orientation * traverse_rate * f * dt;
            }
        }
        // UP/DOWN
        {
            float f = 0.0f;
            if (input.mouse.is_button_down (sge::runtime::mouse_button::middle)) { f += input.mouse.velocity (sge::ext::mouse::proportion::displaysize).y * MOUSE_F; }
            if (!sge::math::is_zero (f)) {
                position -= sge::math::vector3::up * orientation * traverse_rate * f * dt;
            }
        }
    }

    sge::math::vector3         position;
    sge::math::quaternion      orientation;

    float traverse_sensitivity = 1.0f;
    float look_sensitivity = 1.0f;

    float fov = 57.5;
    float near = 0.1f;
    float far = 100.0f;

    constexpr static float FOV_MIN = 30.0f;
    constexpr static float FOV_MAX = 90.0f;
    constexpr static float FOV_DEBUG_RATE = 20.0f;

    constexpr static float TRAVERSE_RATE = 5.0f;
    constexpr static float FAST_TRAVERSE_RATE = 30.0f;
    constexpr static float LOOK_RATE = 1.20f;
    constexpr static float FAST_LOOK_RATE = 1.70f;
    constexpr static float MOUSE_F = 1.0f / 0.40f; // moving 40% of screen space per second is equivalent to holding a joystick on full

};

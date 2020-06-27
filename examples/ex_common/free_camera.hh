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
        orientation = sge::math::quaternion::from_yaw_pitch_roll(5.0f * 3.1415962f / 4.0f, (3.1415962f / 2.0f) - (atan(sqrt(zx * zx * 2.0f) / y)), 0.0f);
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


        // UP/DOWN
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_down)) { position.y -= traverse_rate * dt; }
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_up)) { position.y += traverse_rate * dt; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::middle)) { position.y -= MOUSE_RATE * input.mouse.position_delta_proportional ().y * traverse_rate * dt; }


        sge::math::vector3 eulerAngles = sge::math::quaternion::to_yaw_pitch_roll(orientation);

        // YAW
        float rxx = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::left)) { rxx = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::right)) { rxx = +1.0f; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::right)) { rxx += MOUSE_RATE * input.mouse.position_delta_proportional ().x; }
        if (abs(input.gamepad.right_stick().x) > abs (rxx)) { rxx = input.gamepad.right_stick().x; }
        if (!sge::math::is_zero (rxx)) {
            eulerAngles.x -=dt * look_rate * rxx;
        }

        // PITCH
        float rxy = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::down)) { rxy = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::up)) { rxy = +1.0f; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::right)) { rxy += MOUSE_RATE * input.mouse.position_delta_proportional ().y; } // not inverting here as it feels wrong with the visible mouse cursor.
        if (abs(input.gamepad.right_stick().y) > abs (rxy)) { rxy = input.gamepad.right_stick().y; }
        if (!sge::math::is_zero (rxy)) {
            eulerAngles.y += dt * look_rate * rxy;
        }

        orientation = sge::math::quaternion::from_yaw_pitch_roll(eulerAngles.x, eulerAngles.y, eulerAngles.z);
        float x_comp = sin(eulerAngles.x);
        float z_comp = cos(eulerAngles.x);
        float y_comp = sin(eulerAngles.y);

        // LEFT/RIGHT
        float lxx = 0.0f;
        if (input.keyboard.is_character_down('a')) { lxx = -1.0f; }
        if (input.keyboard.is_character_down('d')) { lxx = +1.0f; }
        if (input.mouse.is_button_down (sge::runtime::mouse_button::middle)) { lxx += MOUSE_RATE * input.mouse.position_delta_proportional().x; }
        if (abs(input.gamepad.left_stick().x) > abs (lxx)) { lxx = input.gamepad.left_stick().x; }
        if (!sge::math::is_zero (lxx)) {
            position.x += -z_comp * traverse_rate * lxx * dt;
            position.z += x_comp * traverse_rate * lxx * dt;
        }

        // FORWARD/BACKWARD
        float lxy = 0.0f;
        if (input.keyboard.is_character_down('s')) { lxy = -1.0f; }
        if (input.keyboard.is_character_down('w')) { lxy = +1.0f; }
        int scroll = input.mouse.scrollwheel_delta ();
        if (scroll > 0) { lxy = +50.0f; }
        if (scroll < 0) { lxy = -50.0f; }
        if (abs(input.gamepad.left_stick().y) > abs (lxy)) { lxy = input.gamepad.left_stick().y; }
        if (!sge::math::is_zero (lxy)) {
            position.x += x_comp * traverse_rate * lxy * dt;
            position.z += z_comp * traverse_rate * lxy * dt;
            position.y -= y_comp * traverse_rate * lxy * dt;
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
    constexpr static float MOUSE_RATE = 300.0f;

};

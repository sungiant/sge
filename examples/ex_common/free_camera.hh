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
        
        float sp = input.keyboard.is_key_down(sge::runtime::keyboard_key::spacebar) ? 0.8f : 0.0f;
        float r1 = input.gamepad.is_button_down(sge::runtime::gamepad_button::right_shoulder) ? 0.5f : 0.0f;
        float r2 = input.gamepad.right_trigger();

        float fast_factor = std::max (sp, std::max (r1, r2));
        float traverse_rate = std::max(TRAVERSE_RATE, fast_factor * FAST_TRAVERSE_RATE);
        float look_rate = std::max (LOOK_RATE, fast_factor * FAST_LOOK_RATE);
        
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_down)) { position.y -= traverse_rate * dt; }
        if (input.gamepad.is_button_down(sge::runtime::gamepad_button::dpad_up)) { position.y += traverse_rate * dt; }

        sge::math::vector3 eulerAngles = sge::math::quaternion::to_yaw_pitch_roll(orientation);
        
        float rxx = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::left)) { rxx = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::right)) { rxx = +1.0f; }
        if (std::abs(input.gamepad.right_stick().x) > std::abs (rxx)) { rxx = input.gamepad.right_stick().x; }
        if (!sge::math::is_zero (rxx)) {
            eulerAngles.x -=dt * look_rate * rxx;
        }
        
        float rxy = 0.0f;
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::down)) { rxy = -1.0f; }
        if (input.keyboard.is_key_down(sge::runtime::keyboard_key::up)) { rxy = +1.0f; }
        if (std::abs(input.gamepad.right_stick().y) > std::abs (rxy)) { rxy = input.gamepad.right_stick().y; }
        if (!sge::math::is_zero (rxy)) {
            eulerAngles.y += dt * look_rate * rxy;
        }
        
        orientation = sge::math::quaternion::from_yaw_pitch_roll(eulerAngles.x, eulerAngles.y, eulerAngles.z);
        float x_comp = std::sin(eulerAngles.x);
        float z_comp = std::cos(eulerAngles.x);
        float y_comp = std::sin(eulerAngles.y);

        float lxx = 0.0f;
        if (input.keyboard.is_character_down('a')) { lxx = -1.0f; }
        if (input.keyboard.is_character_down('d')) { lxx = +1.0f; }
        if (std::abs(input.gamepad.left_stick().x) > std::abs (lxx)) { lxx = input.gamepad.left_stick().x; }
        if (!sge::math::is_zero (lxx)) {
            position.x += -z_comp * traverse_rate * lxx * dt;
            position.z += x_comp * traverse_rate * lxx * dt;
        }
        
        float lxy = 0.0f;
        if (input.keyboard.is_character_down('s')) { lxy = -1.0f; }
        if (input.keyboard.is_character_down('w')) { lxy = +1.0f; }
        if (std::abs(input.gamepad.left_stick().y) > std::abs (lxy)) { lxy = input.gamepad.left_stick().y; }
        if (!sge::math::is_zero (lxy)) {
            position.x += x_comp * traverse_rate * lxy * dt;
            position.z += z_comp * traverse_rate * lxy * dt;
            position.y -= y_comp * traverse_rate * lxy * dt;
        }
    }

    sge::math::vector3         position;
    sge::math::quaternion      orientation;

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

};

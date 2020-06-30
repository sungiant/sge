#pragma once

#include <cmath>
#include <algorithm>

#include <sge.hh>
#include <sge_runtime.hh>
#include <sge_app.hh>
#include <imguizmo/ImGuizmo.h>

struct free_camera : public sge::runtime::view {
    free_camera (const sge::runtime::api& z) : sge::runtime::view (z) {
        reset();
    }
    void reset () {
        float y = 4.0f;
        float zx = 6.0f;
        position = { zx, y, zx };
        orientation = sge::math::quaternion::create_from_yaw_pitch_roll(5.0f * 3.1415962f / 4.0f, (3.1415962f / 2.0f) - (atan(sqrt(zx * zx * 2.0f) / y)), 0.0f);
    }
    
    virtual void update () override {
        
        const float dt = sge.timer__get_delta ();
        
        const auto& keyboard = *static_cast <sge::ext::keyboard*> (sge.extension_get (sge::runtime::type_id<sge::ext::keyboard>()));
        const auto& mouse = *static_cast <sge::ext::mouse*> (sge.extension_get (sge::runtime::type_id<sge::ext::mouse>()));
        const auto& gamepad = *static_cast <sge::ext::gamepad*> (sge.extension_get (sge::runtime::type_id<sge::ext::gamepad>()));
        
        if (gamepad.is_button_down(sge::runtime::gamepad_button::dpad_left)) { fov -= dt * FOV_DEBUG_RATE; }
        if (gamepad.is_button_down(sge::runtime::gamepad_button::dpad_right)) {  fov += dt * FOV_DEBUG_RATE; }
        fov = std::clamp (fov, FOV_MIN, FOV_MAX);

        const float sp = keyboard.is_key_down(sge::runtime::keyboard_key::spacebar) ? 0.8f : 0.0f;
        const float r1 = gamepad.is_button_down(sge::runtime::gamepad_button::right_shoulder) ? 0.5f : 0.0f;
        const float r2 = gamepad.right_trigger();

        const float fast_factor = std::max (sp, std::max (r1, r2));
        const float traverse_rate = std::max(TRAVERSE_RATE, fast_factor * FAST_TRAVERSE_RATE) * (1.0f / traverse_sensitivity);
        const float look_rate = std::max (LOOK_RATE, fast_factor * FAST_LOOK_RATE) * (1.0f / look_sensitivity);

        // AXIS UP/DOWN
        if (gamepad.is_button_down(sge::runtime::gamepad_button::dpad_down)) { position.y -= traverse_rate * dt; }
        if (gamepad.is_button_down(sge::runtime::gamepad_button::dpad_up)) { position.y += traverse_rate * dt; }

        sge::math::vector3 eulerAngles;
        orientation.get_yaw_pitch_roll(eulerAngles);

        // YAW
        float rxx = 0.0f;
        if (keyboard.is_key_down(sge::runtime::keyboard_key::left)) { rxx = -1.0f; }
        if (keyboard.is_key_down(sge::runtime::keyboard_key::right)) { rxx = +1.0f; }
        if (mouse.is_button_down (sge::runtime::mouse_button::right)) { rxx += mouse.velocity (sge::ext::mouse::proportion::displaysize).x * MOUSE_F; }
        if (abs(gamepad.right_stick().x) > abs (rxx)) { rxx = gamepad.right_stick().x; }
        if (!sge::math::is_zero (rxx)) {
            eulerAngles.x -=dt * look_rate * rxx;
        }

        // PITCH
        float rxy = 0.0f;
        if (keyboard.is_key_down(sge::runtime::keyboard_key::down)) { rxy = -1.0f; }
        if (keyboard.is_key_down(sge::runtime::keyboard_key::up)) { rxy = +1.0f; }
        if (mouse.is_button_down (sge::runtime::mouse_button::right)) { rxy += mouse.velocity (sge::ext::mouse::proportion::displaysize).y * MOUSE_F; } // not inverting here as it feels wrong with the visible mouse cursor.
        if (abs(gamepad.right_stick().y) > abs (rxy)) { rxy = gamepad.right_stick().y; }
        if (!sge::math::is_zero (rxy)) {
            eulerAngles.y += dt * look_rate * rxy;
        }

        // ROLL
        float rz = 0.0f;
        if (keyboard.is_key_down(sge::runtime::keyboard_key::q)) { rz = -1.0f; }
        if (keyboard.is_key_down(sge::runtime::keyboard_key::e)) { rz = +1.0f; }
        if (gamepad.is_button_down(sge::runtime::gamepad_button::left_shoulder)) { rz = -1.0f; }
        if (gamepad.is_button_down(sge::runtime::gamepad_button::right_shoulder)) { rz = +1.0f; }
        if (!sge::math::is_zero (rz)) {
            eulerAngles.z += dt * look_rate * rz;
        }

        orientation = sge::math::quaternion::create_from_yaw_pitch_roll(eulerAngles.x, eulerAngles.y, eulerAngles.z);

        // LEFT/RIGHT
        {
        float f = 0.0f;
            if (keyboard.is_character_down('a')) { f = +1.0f; }
            if (keyboard.is_character_down('d')) { f = -1.0f; }
            if (mouse.is_button_down (sge::runtime::mouse_button::middle)) { f -= mouse.velocity (sge::ext::mouse::proportion::displaysize).x * MOUSE_F; }
            if (abs(gamepad.left_stick().x) > abs (f)) { f = -gamepad.left_stick().x; }
            if (!sge::math::is_zero (f)) {
                position += sge::math::vector3::right * orientation * traverse_rate * f * dt;
            }
        }

        // FORWARD/BACKWARD
        {
            float f = 0.0f;
            if (keyboard.is_character_down('s')) { f = +1.0f; }
            if (keyboard.is_character_down('w')) { f = -1.0f; }
            int scroll = mouse.scrollwheel_delta ();
            if (scroll > 0) { f = -3.0f; }
            if (scroll < 0) { f = +3.0f; }
            if (abs(gamepad.left_stick().y) > abs (f)) { f = -gamepad.left_stick().y; }
            if (!sge::math::is_zero (f)) {
                position += sge::math::vector3::forward * orientation * traverse_rate * f * dt;
            }
        }
        
        // UP/DOWN
        {
            float f = 0.0f;
            if (mouse.is_button_down (sge::runtime::mouse_button::middle)) { f -= mouse.velocity (sge::ext::mouse::proportion::displaysize).y * MOUSE_F; }
            if (!sge::math::is_zero (f)) {
                position += sge::math::vector3::up * orientation * traverse_rate * f * dt;
            }
        }
    }
    
    virtual void debug_ui () override {

        ImGuizmo::BeginFrame ();
        ImGuizmo::SetOrthographic(false);

        float screen_w = sge.system__get_state_int(sge::runtime::system_int_state::screenwidth);
        
        if (!orientation.is_unit())
            orientation.normalise();
        
        sge::math::matrix33 cameraRotationLH = sge::math::matrix33::create_from_orientation(orientation);
        // imguizmo looks to be using a left handed coordinate system.
        cameraRotationLH.r2c0 = -cameraRotationLH.r2c0;
        cameraRotationLH.r2c1 = -cameraRotationLH.r2c1;
        cameraRotationLH.r2c2 = -cameraRotationLH.r2c2;
        cameraRotationLH.orthonormalise();
        
        const sge::math::matrix44 gizmoView = sge::math::matrix44::create_from_rotation(cameraRotationLH);
        
        auto gizmoViewDelta = gizmoView;
               
        const int gizmo_size = 64;
        ImGuizmo::ViewManipulate(&gizmoViewDelta[0][0], position.length(), ImVec2(screen_w - gizmo_size, 0), ImVec2(gizmo_size, gizmo_size), 0x10101010);
        
        if (gizmoViewDelta != gizmoView) {
            sge::math::matrix33 new_rotation;
            gizmoViewDelta.get_rotation_component (new_rotation);

            // and back to right handed!
            new_rotation.r2c0 = -new_rotation.r2c0;
            new_rotation.r2c1 = -new_rotation.r2c1;
            new_rotation.r2c2 = -new_rotation.r2c2;
            
            new_rotation.orthonormalise();
            orientation = sge::math::quaternion::create_from_rotation(new_rotation);
        }
    }

    sge::math::vector3 position;
    sge::math::quaternion orientation;

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
    constexpr static float MOUSE_F = 1.0f / 1.40f; // moving 140% of screen space per second is equivalent to holding a joystick on full

};

#pragma once

#include "sge.hh"
#include "sge_runtime.hh"
#include "sge_ext_keyboard.hh"
#include "sge_ext_mouse.hh"
#include "sge_ext_gamepad.hh"

namespace sge::ext {

struct freecam : public runtime::view {

    math::vector3 position;
    math::quaternion orientation;
    
    
    math::vector3 default_position;
    math::quaternion default_orientation;

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
    
    freecam (const runtime::api& z) : runtime::view (z, "Freecam") {
        float y = 4.0f;
        float zx = 6.0f;
        default_position = { zx, y, zx };
        default_orientation.set_from_yaw_pitch_roll(1.0f * 3.1415962f / 4.0f, -(3.1415962f / 2.0f) + (atan(sqrt(zx * zx * 2.0f) / y)), 0.0f);
        position = default_position;
        orientation = default_orientation;
        set_active(false);
    }
    
    virtual void update () override {
        static bool flag = false;
        
        if (!flag) {
            flag = true;
            position = default_position;
            orientation = default_orientation;
        }
        
        const float dt = sge.timer__get_delta ();

        const auto& keyboard = *static_cast <ext::keyboard*> (sge.extension_get (runtime::type_id<ext::keyboard>()));
        const auto& mouse = *static_cast <ext::mouse*> (sge.extension_get (runtime::type_id<ext::mouse>()));
        const auto& gamepad = *static_cast <ext::gamepad*> (sge.extension_get (runtime::type_id<ext::gamepad>()));
        
        if (gamepad.is_button_down(runtime::gamepad_button::dpad_left)) { fov -= dt * FOV_DEBUG_RATE; }
        if (gamepad.is_button_down(runtime::gamepad_button::dpad_right)) {  fov += dt * FOV_DEBUG_RATE; }
        fov = std::clamp (fov, FOV_MIN, FOV_MAX);

        const float sp = keyboard.is_key_down(runtime::keyboard_key::spacebar) ? 0.8f : 0.0f;
        const float r1 = gamepad.is_button_down(runtime::gamepad_button::right_shoulder) ? 0.5f : 0.0f;
        const float r2 = gamepad.right_trigger();

        const float fast_factor = std::max (sp, std::max (r1, r2));
        const float traverse_rate = std::max(TRAVERSE_RATE, fast_factor * FAST_TRAVERSE_RATE) * (1.0f / traverse_sensitivity);
        const float look_rate = std::max (LOOK_RATE, fast_factor * FAST_LOOK_RATE) * (1.0f / look_sensitivity);

        // AXIS UP/DOWN
        if (gamepad.is_button_down(runtime::gamepad_button::dpad_down)) { position.y -= traverse_rate * dt; }
        if (gamepad.is_button_down(runtime::gamepad_button::dpad_up)) { position.y += traverse_rate * dt; }

        // LEFT/RIGHT
        {
            float f = 0.0f;
            if (keyboard.is_character_down('a')) { f = -1.0f; }
            if (keyboard.is_character_down('d')) { f = +1.0f; }
            if (mouse.is_button_down (runtime::mouse_button::middle)) { f += mouse.velocity (mouse::proportion::displaysize).x * MOUSE_F; }
            if (abs(gamepad.left_stick().x) > abs (f)) { f = gamepad.left_stick().x; }
            if (!math::is_zero (f)) { position += math::vector3::right * orientation * traverse_rate * f * dt; }
        }
        // FORWARD/BACKWARD
        {
            float f = 0.0f;
            if (keyboard.is_character_down('s')) { f = -1.0f; }
            if (keyboard.is_character_down('w')) { f = +1.0f; }
            const int scroll = mouse.scrollwheel_delta ();
            if (scroll < 0) { f = -3.0f; }
            if (scroll > 0) { f = +3.0f; }
            if (abs(gamepad.left_stick().y) > abs (f)) { f = gamepad.left_stick().y; }
            if (!math::is_zero (f)) { position += math::vector3::forward * orientation * traverse_rate * f * dt; }
        }
        // UP/DOWN
        {
            float f = 0.0f;
            if (mouse.is_button_down (runtime::mouse_button::middle)) { f -= mouse.velocity (mouse::proportion::displaysize).y * MOUSE_F; }
            if (!math::is_zero (f)) { position += math::vector3::up * orientation * traverse_rate * f * dt; }
        }
        
        math::vector3 eulerAngles;
        orientation.get_yaw_pitch_roll(eulerAngles);

        // YAW
        {
            float rxx = 0.0f;
            if (keyboard.is_key_down(runtime::keyboard_key::left)) { rxx = +1.0f; }
            if (keyboard.is_key_down(runtime::keyboard_key::right)) { rxx = -1.0f; }
            if (mouse.is_button_down (runtime::mouse_button::right)) { rxx -= mouse.velocity (mouse::proportion::displaysize).x * MOUSE_F; }
            if (abs(gamepad.right_stick().x) > abs (rxx)) { rxx = -gamepad.right_stick().x; }
            if (!math::is_zero (rxx)) { eulerAngles.x +=dt * look_rate * rxx; }
        }
        // PITCH
        {
            float rxy = 0.0f;
            if (keyboard.is_key_down(runtime::keyboard_key::down)) { rxy = -1.0f; }
            if (keyboard.is_key_down(runtime::keyboard_key::up)) { rxy = +1.0f; }
            if (mouse.is_button_down (runtime::mouse_button::right)) { rxy -= mouse.velocity (mouse::proportion::displaysize).y * MOUSE_F; } // not inverting here as it feels wrong with the visible mouse cursor.
            if (abs(gamepad.right_stick().y) > abs (rxy)) { rxy = -gamepad.right_stick().y; }
            if (!math::is_zero (rxy)) { eulerAngles.y += dt * look_rate * rxy; }
        }
        // ROLL
        {
            float rz = 0.0f;
            if (keyboard.is_key_down(runtime::keyboard_key::e)) { rz = -1.0f; }
            if (keyboard.is_key_down(runtime::keyboard_key::q)) { rz = +1.0f; }
            if (gamepad.is_button_down(runtime::gamepad_button::left_shoulder)) { rz = +1.0f; }
            if (gamepad.is_button_down(runtime::gamepad_button::right_shoulder)) { rz = -1.0f; }
            if (!math::is_zero (rz)) { eulerAngles.z += dt * look_rate * rz; }
        }

        orientation.set_from_yaw_pitch_roll(eulerAngles.x, eulerAngles.y, eulerAngles.z);
    }
    
    virtual void managed_debug_ui () override {
        
        ImGui::Text("position (x:%.2f, y:%.2f, z:%.2f)", position.x, position.y, position.z);
        ImGui::Text("orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", orientation.i, orientation.j, orientation.k, orientation.u);
        {
            const math::matrix33 camera_rotation = math::matrix33().set_from_orientation(orientation);
            ImGui::Text("right (x:%.2f, y:%.2f, z:%.2f)", camera_rotation.r0c0, camera_rotation.r0c1, camera_rotation.r0c2);
            ImGui::Text("up (x:%.2f, y:%.2f, z:%.2f)", camera_rotation.r1c0, camera_rotation.r1c1, camera_rotation.r1c2);
            ImGui::Text("backward (x:%.2f, y:%.2f, z:%.2f)", camera_rotation.r2c0, camera_rotation.r2c1, camera_rotation.r2c2);
        }
        {
            math::vector3 camera_rotation_axis;
            float camera_rotation_angle;
            orientation.get_axis_angle(camera_rotation_axis, camera_rotation_angle);
            ImGui::Text("axis angle (x:%.2f, y:%.2f, z:%.2f, a:%.2f)", camera_rotation_axis.x, camera_rotation_axis.y, camera_rotation_axis.z, camera_rotation_angle * math::RAD2DEG);
        }
        {
            math::vector3 camera_rotation_euler_angles;
            orientation.get_yaw_pitch_roll(camera_rotation_euler_angles);
            ImGui::Text("euler angles (yaw:%.2f, pitch:%.2f, roll:%.2f)", camera_rotation_euler_angles.x * math::RAD2DEG, camera_rotation_euler_angles.y * math::RAD2DEG, camera_rotation_euler_angles.z * math::RAD2DEG);
        }
        
        if (ImGui::Button("reset")){
            position = default_position;
            orientation = default_orientation;
        }
        
        if (ImGui::Button("look at origin"))
            orientation.set_from_rotation(math::matrix44().set_as_view_transform_from_look_at_target (position, math::vector3::zero, math::vector3::up).get_rotation_component());
        
        ImGui::SliderFloat("fov", &fov, freecam::FOV_MIN, freecam::FOV_MAX);
        ImGui::SliderFloat("near", &near, 0.0f, 1000.0f);
        ImGui::SliderFloat("far", &far, 0.0f, 1000.0f);
        ImGui::SliderFloat ("sensitivity", &traverse_sensitivity, 1, 1000); // todo: automatically adjusted this based on proximity to surface - need info back from the compute shader for this.
    
    }
};

}

#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.hh>

#include "../ex_common/free_camera.hh"

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;

struct PUSH { float time = 0.0f; } push;

struct UBO {
    sge::math::vector3          position;
    float                       gamma = 1.2f;
    sge::math::quaternion       orientation;
    int                         marching_step_limit = 256;
    int                         fractal_step_limit = 16;
    float                       anim_speed = 0.15f;
    float                       anim_range = 0.25f;
    sge::math::vector3          d = sge::math::vector3 { 0.0f, 1.0f, 0.2f };
    float                       fractal_power = 4.1f;
    sge::math::vector3          e = sge::math::vector3 { 0.7f, -1.9f, 0.0f };
    float                       brightness_amount = 0.6f;
    sge::math::vector3          glow_colour = { 0.454f, 0.0f, 0.672f };
    float                       glow_amount = 1.2f;

    bool operator == (const UBO& ubo) const {
        return position == ubo.position
            && gamma == ubo.gamma
            && orientation == ubo.orientation
            && marching_step_limit == ubo.marching_step_limit
            && fractal_step_limit == ubo.fractal_step_limit
            && anim_speed == ubo.anim_speed
            && anim_range == ubo.anim_range
            && d == ubo.d
            && fractal_power == ubo.fractal_power
            && e == ubo.e
            && brightness_amount == ubo.brightness_amount
            && glow_colour == ubo.glow_colour
            && glow_amount == ubo.glow_amount;
    }
    bool operator != (const UBO& ubo) const { return !(*this == ubo); }
} ubo;

free_camera camera;

void initialise () {
    config = std::make_unique<sge::app::configuration> ();
    config->app_name = "mandlebulb";
    config->app_width = 1280;
    config->app_height = 720;
    config->enable_console = true;

    computation = std::make_unique<sge::app::content>(sge::app::content {
        "mandlebulb.comp.spv",
        std::optional<sge::dataspan> ({ &push, sizeof (PUSH) }),
        {
            sge::dataspan { &ubo, sizeof (UBO) },
        }
    });

    camera.position = { 0.62f, 0.53f, -2.65f };
    camera.orientation = { 0.09f, -0.10f, -0.01f, 0.99f };
}

void terminate () { config.reset (); }

void update (sge::app::response& r, const sge::app::api& sge) {
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    UBO u = ubo;

    camera.update (sge.instrumentation.dt(), sge.input);

    u.position = camera.position;
    u.orientation = camera.orientation;
    if (u != ubo) {
        ubo = u;
        r.uniform_changes[0] = true;
    }

    push.time = sge.instrumentation.timer();
    r.push_constants_changed = true;
}

void debug_ui (sge::app::response& r, const sge::app::api& sge) {
    UBO u = ubo;
    ImGui::Begin ("Mandlebulb");
    {
        ImGui::Text("camera position (x:%.2f, y:%.2f, z:%.2f)", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("camera orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", camera.orientation.i, camera.orientation.j, camera.orientation.k, camera.orientation.u);
        ImGui::SliderFloat("gamma", &u.gamma, 0, 4.0f);
        ImGui::SliderInt("marching step limit", &u.marching_step_limit, 1, 1024);
        ImGui::SliderInt("fractal step limit", &u.fractal_step_limit, 1, 32);
        ImGui::SliderFloat("fractal power", &u.fractal_power, 0, 10.0f);
        ImGui::SliderFloat("anim speed", &u.anim_speed, 0.0, 5.0f);
        ImGui::SliderFloat("anim range", &u.anim_range, 0.0, 2.0f);
        ImGui::SliderFloat("brightness amount", &u.brightness_amount, 0.0, 1.0f);
        ImGui::Text ("float t = iterations / iteration_limit");
        ImGui::Text ("vec3 col = d + e * t");
        ImGui::DragFloat3 ("d", &u.d.x, 0.1f, -5.0f, 5.0f);
        ImGui::DragFloat3 ("e", &u.e.x, 0.1f, -5.0f, 5.0f);
        ImGui::SliderFloat("glow amount", &u.glow_amount, 0.0, 2.0f);
        ImGui::ColorEdit3("glow colour", &u.glow_colour.x);
    }
    ImGui::End();

    if (u != ubo) {
        ubo = u;
        r.uniform_changes[0] = true;
    }
}



//--------------------------------------------------------------------------------------------------------------------//
namespace sge::app { // HOOK UP TO SGE

void               initialise          ()                              { ::initialise (); }
configuration&     get_configuration   ()                              { return *::config; }
content&           get_content         ()                              { return *::computation; }
void               start               (const api& sge)                {}
void               update              (response& r, const api& sge)   { ::update (r, sge); }
void               debug_ui            (response& r, const api& sge)   { ::debug_ui (r, sge); }
void               stop                (const api& sge)                {}
void               terminate           ()                              { ::terminate (); }

}

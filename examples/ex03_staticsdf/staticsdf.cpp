#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.h>

#include "../ex_common/free_camera.hpp"

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;

struct PUSH {
    float time;
    bool no_change;

    bool operator == (const PUSH& push) const {
        return sge::math::is_zero(time - push.time)
            && no_change == push.no_change;
    }
    bool operator != (const PUSH& push) const { return !(*this == push); }
} push;

struct UBO_CAMERA { // https://www.reddit.com/r/vulkan/comments/9spolm/help_with_compute_shader_misaligned_write/
    sge::math::vector3          position;
    int                         dummy;
    sge::math::quaternion       orientation;
    float                       fov;
    float                       aspect;
    float                       near;
    float                       far;

    bool operator == (const UBO_CAMERA& ubo) const {
        return position == ubo.position
            //&& dummy == ubo.dummy
            && orientation == ubo.orientation
            && sge::math::is_zero(fov - ubo.fov)
            && sge::math::is_zero(aspect - ubo.aspect)
            && sge::math::is_zero(near - ubo.near)
            && sge::math::is_zero(far - ubo.far);
    }
    bool operator != (const UBO_CAMERA& ubo) const { return !(*this == ubo); }
} ubo_camera;

struct UBO_SETTINGS {
    enum Flags {
        CalculateAO = 0x01,
        CalculateShadows = 0x03,
        CalculateLighting = 0x07,
        EnableLazyness = 0xf,
        EnablebufferDownsizing = 0x10,
    };

    int         display_mode = 0;
    float       gamma = 2.2f;
    int         iterations = 64;
    uint32_t    flags =  CalculateAO | CalculateShadows | CalculateLighting | EnableLazyness | EnablebufferDownsizing;
    float       soft_shadow_factor = 10.0f;

    bool operator == (const UBO_SETTINGS& ubo) const {
        return display_mode == ubo.display_mode
            && sge::math::is_zero(gamma - ubo.gamma)
            && iterations == ubo.iterations
            && flags == ubo.flags
            && sge::math::is_zero(soft_shadow_factor - ubo.soft_shadow_factor);
    }
    bool operator != (const UBO_SETTINGS& ubo) const { return !(*this == ubo); }
} ubo_settings;


free_camera camera;

void initialise () {
    config = std::make_unique<sge::app::configuration> ();
    config->app_name = "SDF (static)";
    config->app_width = 960;
    config->app_height = 540;
    config->enable_console = true;

    // initial ubo settings
    ubo_camera.aspect = (float)config->app_width / (float)config->app_height;
    ubo_settings.iterations = 64;
    ubo_settings.display_mode = 0;

    computation = std::make_unique<sge::app::content>(sge::app::content {
        "staticsdf.comp.spv",
        std::optional<sge::dataspan> ({ &push, sizeof (PUSH) }),
        {
            sge::dataspan { &ubo_camera, sizeof (UBO_CAMERA) },
            sge::dataspan { &ubo_settings, sizeof (UBO_SETTINGS) },
        }
    });


    push.time = 0.0f;
    push.no_change = false;
}

void terminate () { config.reset (); }


void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    camera.update (sge.instrumentation.dt(), sge.input);
    int res_x = sge.runtime.system__get_state_int(sge::runtime::system_int_state::screenwidth);
    int res_y = sge.runtime.system__get_state_int (sge::runtime::system_int_state::screenheight);

    UBO_CAMERA uc = ubo_camera;
    uc.position = camera.position;
    uc.orientation = camera.orientation;
    uc.fov = camera.fov;
    uc.near = camera.near;
    uc.far = camera.far;
    uc.aspect = (float) res_x / (float)res_y;
    if (uc != ubo_camera) {
        ubo_camera = uc;
        r.uniform_changes[0] = true;
    }

    if (!r.uniform_changes[0] && !r.uniform_changes[1] && !push.no_change) {
        push.no_change = true;
        r.push_constants_changed = true;
    }

    if ((r.uniform_changes[0] || r.uniform_changes[1] || sge.runtime.system__did_container_just_change ()) && push.no_change) {
        push.no_change = false;
        r.push_constants_changed = true;
    }

}

const char* display_mode_text[] {
    "composition",
    "depth",
    "iterations",
    "normals",
    "minimum cone ratio",
    "hard shadows",
    "soft shadows",
    "ambient occlusion",
    "lighting",
};

void debug_ui (sge::app::response& r, const sge::app::api& sge) {
    ImGui::Begin ("SDF (static)");
    {
        UBO_SETTINGS us = ubo_settings;

        ImGui::Text("displaying: %s", display_mode_text[us.display_mode]);
        ImGui::Text("position (x:%.2f, y:%.2f, z:%.2f)", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("orientation (i:%.2f, j:%.2f, k:%.2f, u:%.2f)", camera.orientation.i, camera.orientation.j, camera.orientation.k, camera.orientation.u);

        ImGui::SliderFloat("fov", &camera.fov, free_camera::FOV_MIN, free_camera::FOV_MAX);
        ImGui::SliderFloat("near", &camera.near, 0.0f, 1000.0f);
        ImGui::SliderFloat("far", &camera.far, 0.0f, 1000.0f);

        ImGui::SliderInt("display mode", &us.display_mode, 0, 7);
        ImGui::SliderFloat("gamma", &us.gamma, 0, 4.0f);
        ImGui::SliderInt("iterations", &us.iterations, 1, 256);

        ImGui::Text("FLAGS: %d", us.flags);
        bool ao = (us.flags >> 0) & 1u;
        bool shadows = (us.flags >> 1) & 1u;
        bool lighting = (us.flags >> 2) & 1u;
        bool lazy = (us.flags >> 3) & 1u;
        if (ImGui::RadioButton("enable AO", ao)) { us.flags ^= (1u << 0); }
        if (ImGui::RadioButton("enable shadows", shadows)) { us.flags ^= (1u << 1); }
        if (ImGui::RadioButton("enable lighting", lighting)) { us.flags ^= (1u << 2); }
        if (ImGui::RadioButton("enable lazyness", lazy)) { us.flags ^= (1u << 3); }

        ImGui::SliderFloat("shadow_factor", &us.soft_shadow_factor, 1.0f, 100.0f);

        if (us != ubo_settings) {
            ubo_settings = us;
            r.uniform_changes[1] = true;
        }
    }
    ImGui::End();
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

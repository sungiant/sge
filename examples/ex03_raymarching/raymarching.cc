#include <sge_app.hh>

sge::app::configuration config = {};
sge::app::content computation = {};
sge::app::extensions extensions = {};

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
        CalculateAO            = (1 << 0),
        CalculateShadows       = (1 << 1),
        CalculateLighting      = (1 << 2),
        EnableLazyness         = (1 << 3),
        RenderLazynessMarkers  = (1 << 4),
        DebugAxes              = (1 << 5)
    };

    int         display_mode = 0;
    float       gamma = 2.2f;
    int         iterations = 64;
    uint32_t    flags =  CalculateAO | CalculateShadows | CalculateLighting | EnableLazyness;
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

void initialise () {
    config.app_name = "Raymarching (static SDF)";
    config.app_width = 960;
    config.app_height = 540;
    config.enable_console = true;

    // initial ubo settings
    ubo_camera.aspect = (float)config.app_width / (float)config.app_height;
    ubo_settings.iterations = 64;
    ubo_settings.display_mode = 0;

    computation.shader_path = "raymarching.comp.spv";
    computation.push_constants = std::optional<sge::dataspan> ({ &push, sizeof (PUSH) });
    computation.uniforms = {
        sge::dataspan { &ubo_camera, sizeof (UBO_CAMERA) },
        sge::dataspan { &ubo_settings, sizeof (UBO_SETTINGS) },
    };

    push.time = 0.0f;
    push.no_change = false;
}

void terminate () {}

void start (const sge::app::api& sge) {
    sge.freecam.set_active (true);
}

void stop (const sge::app::api& sge) {}

void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }
    
    int res_x = sge.runtime.system__get_state_int(sge::runtime::system_int_state::canvas_width);
    int res_y = sge.runtime.system__get_state_int (sge::runtime::system_int_state::canvas_height);

    UBO_CAMERA uc = ubo_camera;
    uc.position = sge.freecam.position;
    uc.orientation = sge.freecam.orientation;
    uc.fov = sge.freecam.fov;
    uc.near = sge.freecam.near;
    uc.far = sge.freecam.far;
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
        ImGui::SliderInt("display mode", &us.display_mode, 0, 7);
        ImGui::SliderFloat("gamma", &us.gamma, 0, 4.0f);
        ImGui::SliderInt("iterations", &us.iterations, 1, 256);

        ImGui::Text("FLAGS: %d", us.flags);
        bool ao = sge::utils::get_flag_at_index (us.flags, 0);
        bool shadows = sge::utils::get_flag_at_index (us.flags, 1);
        bool lighting = sge::utils::get_flag_at_index (us.flags, 2);
        bool lazy = sge::utils::get_flag_at_index (us.flags, 3);
        bool lazy_markers = sge::utils::get_flag_at_index (us.flags, 4);
        bool debug_axes = sge::utils::get_flag_at_index (us.flags, 5);
        if (ImGui::RadioButton("enable AO", ao)) { sge::utils::toggle_flag_at_index (us.flags, 0); }
        if (ImGui::RadioButton("enable shadows", shadows)) {sge::utils::toggle_flag_at_index (us.flags, 1); }
        if (ImGui::RadioButton("enable lighting", lighting)) { sge::utils::toggle_flag_at_index (us.flags, 2); }
        if (ImGui::RadioButton("enable lazyness", lazy)) { sge::utils::toggle_flag_at_index (us.flags, 3); }
        if (ImGui::RadioButton("render lazyness markers", lazy_markers)) { sge::utils::toggle_flag_at_index (us.flags, 4); }
        if (ImGui::RadioButton("show debug axes", debug_axes)) { sge::utils::toggle_flag_at_index (us.flags, 5); }

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
configuration&     get_configuration   ()                              { return ::config; }
content&           get_content         ()                              { return ::computation; }
extensions&        get_extensions      ()                              { return ::extensions; }
void               start               (const api& sge)                { ::start (sge); }
void               update              (response& r, const api& sge)   { ::update (r, sge); }
void               debug_ui            (response& r, const api& sge)   { ::debug_ui (r, sge); }
void               stop                (const api& sge)                { ::stop (sge); }
void               terminate           ()                              { ::terminate (); }

}

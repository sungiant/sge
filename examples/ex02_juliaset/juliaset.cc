#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.hh>

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;

struct PUSH {
    float                   time            = 0.0f;
} push;

struct UBO {
    sge::math::vector2      complex         = sge::math::vector2 { 0.0f, 0.0f };
    sge::math::vector2      pan             = sge::math::vector2 { 0.0f, 0.0f };

    int                     flags           = 0;
    int                     iterations      = 64;
    float                   zoom            = 0.770f;

    bool operator == (const UBO& ubo) const {
        return complex == ubo.complex
            && pan == ubo.pan
            && flags == ubo.flags
            && iterations == ubo.iterations
            && zoom == ubo.zoom;
    }
    bool operator != (const UBO& ubo) const { return !(*this == ubo); }
} ubo;

struct COLOUR {
    sge::math::vector4 d = sge::math::vector4 { 0.3f, 0.3f, 0.5f, 1.0f };
    sge::math::vector4 e = sge::math::vector4 { -0.2f, -0.3f ,-0.5f, 1.0f };
    sge::math::vector4 f = sge::math::vector4 { 2.1f, 2.0f, 3.0f, 1.0f };
    sge::math::vector4 g = sge::math::vector4 { 0.0f, 0.1f, 0.0f, 1.0f };

    bool operator == (const COLOUR& col) const { return d == col.d && e == col.e && f == col.f && g == col.g; }
    bool operator != (const COLOUR& col) const { return !(*this == col); }
} ubo_colour;

void initialise () {

    config = std::make_unique<sge::app::configuration> ();
    config->app_name = "Julia Set";
    config->app_width = 960;
    config->app_height = 540;
    config->enable_console = false;

    computation = std::make_unique<sge::app::content>(sge::app::content {
        "juliaset.comp.spv",
        std::optional<sge::dataspan> ({ &push, sizeof (PUSH) }),
        {
            sge::dataspan { &ubo, sizeof (UBO) },
            sge::dataspan { &ubo_colour, sizeof (COLOUR) }
        }
    });
}
void terminate () {
    config.reset ();
}

void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::input::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }


    UBO u = ubo;

    { // zoom
        float zoom = sge.input.gamepad.right_trigger() - sge.input.gamepad.left_trigger();
        if (sge.input.keyboard.is_character_down('+') || sge.input.keyboard.is_character_down('e')) zoom = 0.5f;
        if (sge.input.keyboard.is_character_down('-') || sge.input.keyboard.is_character_down('q')) zoom = -0.5f;

        const float speed = 1.5f * u.zoom;
        if (!sge::math::is_zero (zoom)) { u.zoom += speed * sge.instrumentation.dt() * zoom; }
    }

    { // pan
        float pan_x = sge.input.gamepad.right_stick().x;
        if (sge.input.keyboard.is_key_down(sge::input::keyboard_key::right)) pan_x = 0.5f;
        if (sge.input.keyboard.is_key_down(sge::input::keyboard_key::left)) pan_x = -0.5f;

        float pan_y = sge.input.gamepad.right_stick().y;
        if (sge.input.keyboard.is_key_down(sge::input::keyboard_key::up)) pan_y = 0.5f;
        if (sge.input.keyboard.is_key_down(sge::input::keyboard_key::down)) pan_y = -0.5f;

        const float speed = 1.5f / u.zoom;
        if (!sge::math::is_zero (pan_x)) { u.pan.x -= speed * sge.instrumentation.dt() * pan_x; }
        if (!sge::math::is_zero (pan_y)) { u.pan.y -= speed * sge.instrumentation.dt() * pan_y; }
    }

    { // complex
        float complex_x = sge.input.gamepad.left_stick().x;
        if (sge.input.keyboard.is_character_down('d')) complex_x = 0.5f;
        if (sge.input.keyboard.is_character_down('a')) complex_x = -0.5f;

        float complex_y = sge.input.gamepad.left_stick().y;
        if (sge.input.keyboard.is_character_down('w')) complex_y = 0.5f;
        if (sge.input.keyboard.is_character_down('s')) complex_y = -0.5f;

        const float speed = 0.5f * u.zoom;
        if (!sge::math::is_zero (complex_x)) { u.complex.x -= speed * sge.instrumentation.dt() * complex_x; }
        if (!sge::math::is_zero (complex_y)) { u.complex.y -= speed * sge.instrumentation.dt() * complex_y; }
    }

    if (u != ubo) {
        ubo = u;
        r.uniform_changes[0] = true;
    }

    if (ubo.flags != 0) { // if we are animating update the timer
        push.time = sge.instrumentation.timer();
        r.push_constants_changed = true;
    }
}

void debug_ui (sge::app::response& r, const sge::app::api& sge) {
    ImGui::Begin ("Julia Set");
    {
        UBO u = ubo;

        bool animate_real = (u.flags >> 0) & 1u;
        if (ImGui::RadioButton("Animate real", animate_real)) { u.flags ^= (1u << 0); }

        bool animate_imaginary = (u.flags >> 1) & 1u;
        if (ImGui::RadioButton("Animate imaginary", animate_imaginary)) { u.flags ^= (1u << 1); }

        ImGui::SliderFloat("Real component", &u.complex.x, -2.0f, 2.0f);
        ImGui::SliderFloat("Imaginary component", &u.complex.y, -2.0f, 2.0f);

        ImGui::SliderInt("Iteration limit", &u.iterations, 1, 128);

        ImGui::Text("Zoom: %.6f", u.zoom);
        ImGui::Text("Pan X:%.30f", u.pan.x);
        ImGui::Text("Pan Y:%.30f", u.pan.y);


        if (u != ubo) {
            ubo = u;
            r.uniform_changes[0] = true;
        }

        COLOUR uc = ubo_colour;

        ImGui::Text ("float t = iterations / iteration_limit");
        ImGui::Text ("vec3 col = d + e * cos (2 * pi * (f * t + g))");
        ImGui::DragFloat3 ("d", &uc.d.x, 0.1f, -5.0f, 5.0f);
        ImGui::DragFloat3 ("e", &uc.e.x, 0.1f, -5.0f, 5.0f);
        ImGui::DragFloat3 ("f", &uc.f.x, 0.1f, -5.0f, 5.0f);
        ImGui::DragFloat3 ("g", &uc.g.x, 0.1f, -5.0f, 5.0f);

        if (uc != ubo_colour) {
            ubo_colour = uc;
            r.uniform_changes[1] = true;
        }

    }
    ImGui::End ();
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



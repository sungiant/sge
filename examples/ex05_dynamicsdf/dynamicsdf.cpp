#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.h>

#include "../ex_common/free_camera.hpp"


enum TreeElement : uint32_t { TREE_NODE, TREE_LEAF };
enum ShapeType : uint32_t { SDF_SPHERE, SDF_CUBE, SDF_CUBOID };
enum CSG_OP : uint32_t { CSG_UNION, CSG_INTERSECTION, CSG_DIFFERENCE };

struct Material { sge::math::vector3 colour; float shininess; };

struct PointLight {
    sge::math::vector3 position;
    float range; // for now linear fall off over range
    sge::math::vector3 colour;
    float shadow_factor;
};

struct Shape { sge::math::vector3 position; uint32_t type; sge::math::vector4 data; };

struct Tree { uint32_t type; uint32_t data; };


struct PUSH {
    float time;
    bool no_change;

    bool operator == (const PUSH& push) const {
        return sge::math::is_zero(time - push.time)
            && no_change == push.no_change;
    }
    bool operator != (const PUSH& push) const { return !(*this == push); }
};

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
};

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
};

typedef std::vector<Material> SBO_MATERIALS;
typedef std::vector<PointLight> SBO_LIGHTS;

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;
free_camera camera;

PUSH push;
UBO_CAMERA ubo_camera;
UBO_SETTINGS ubo_settings;

SBO_MATERIALS sbo_materials;
SBO_LIGHTS sbo_lights;

bool light_to_add = false;
std::optional <int> light_to_delete = std::nullopt;

//std::vector<Shape> scene_shapes;
//std::vector<Tree> scene_tree;


void initialise () {
    config = std::make_unique<sge::app::configuration> ();
    config->app_name = "SDF (dynamic)";
    config->app_width = 960;
    config->app_height = 540;
    config->enable_console = true;

    // initial ubo settings
    ubo_camera.aspect = (float)config->app_width / (float)config->app_height;
    ubo_settings.iterations = 64;
    ubo_settings.display_mode = 0;

    const auto white        = sge::math::vector3 { 1.00, 1.00, 1.00 };
    const auto black        = sge::math::vector3 { 0.00, 0.00, 0.00 };
    const auto magenta      = sge::math::vector3 { 1.00, 0.00, 1.00 };
    const auto coolBlack    = sge::math::vector3 { 0.00, 0.18, 0.39 };
    const auto warmBlack    = sge::math::vector3 { 0.00, 0.26, 0.26 };
    const auto persimmon    = sge::math::vector3 { 0.93, 0.35, 0.00 };
    const auto vermillion   = sge::math::vector3 { 0.35, 0.10, 0.08 };
    const auto jade         = sge::math::vector3 { 0.00, 0.66, 0.42 };

    sbo_materials.emplace_back (Material { magenta, 0 });
    sbo_materials.emplace_back (Material { warmBlack, 1 });
    sbo_materials.emplace_back (Material { coolBlack, 256 });
    sbo_materials.emplace_back (Material { jade, 2 });
    sbo_materials.emplace_back (Material { persimmon, 128 });
    sbo_materials.emplace_back (Material { persimmon, 8 });
    sbo_materials.emplace_back (Material { vermillion, 32 });

    sbo_lights.emplace_back (PointLight { sge::math::vector3 {6, 8, 2}, 18.0f, sge::math::vector3 {1, 0.71, 0}, 1 });
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {-5.7, 1.2, 5}, 16.0f, sge::math::vector3 {0.28, 0.12, 0.40}, 1 });
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {0, 0, 0}, 2.3f, sge::math::vector3 {0.32, 0.32, 0.32}, 0.7f });
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {3, 3, 0}, 12.0f, sge::math::vector3 {1, 1, 1}, 1.0f });


    computation = std::make_unique<sge::app::content>(sge::app::content {
        "dynamicsdf.comp.spv",
        std::optional<sge::app::content::span> ({ &push, sizeof (PUSH) }),
        {
            sge::app::content::span { &ubo_camera,              sizeof (UBO_CAMERA) },
            sge::app::content::span { &ubo_settings,            sizeof (UBO_SETTINGS) },
        },
        {
            sge::app::content::span { sbo_materials.data(),     sbo_materials.size() * sizeof (Material) },
         //   sge::app::content::span { sbo_lights.data(),        sbo_lights.size() },
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

    // ---------------------------------
    ImGui::Begin ("SDF (dynamic)");
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

    // ---------------------------------
    ImGui::Begin("Material editor");   
    {
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 40.0f);
        ImGui::SetColumnWidth(1, 200.0f);

        ImGui::NextColumn();
        ImGui::NextColumn();
        ImGui::Text("Shininess");
        ImGui::NextColumn();


        for (int i = 0; i < sbo_materials.size(); ++i)
        {
            char id[64];

            sprintf(&id[0], "#%d", i);
            ImGui::Text(id);
            ImGui::NextColumn();

            //ImGui::ColorButton("Colour", c);
            sprintf(&id[0], "##mat:%d-colour", i);

            ImGui::PushItemWidth(180.0f);
            if (ImGui::ColorEdit3(id, &sbo_materials[i].colour.x))
            {
                r.blob_changes[0] = true;
            }
            ImGui::PopItemWidth();

            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            sprintf(&id[0], "##mat:%d-shininess", i);
            if (ImGui::SliderFloat(id, &(sbo_materials[i].shininess), 0.0f, 256.0f))
            {
                r.blob_changes[0] = true;
            }
            ImGui::PopItemWidth();
            ImGui::NextColumn();
        }
    }
    ImGui::End();

    // ---------------------------------
    ImGui::Begin("Lighting Editor");
    {
        ImGui::Columns(6);
        ImGui::SetColumnWidth(0, 40.0f);
        ImGui::SetColumnWidth(1, 240.0f);
        ImGui::SetColumnWidth(2, 200.0f);
        ImGui::SetColumnWidth(3, 240.0f);
        ImGui::SetColumnWidth(4, 140.0f);
        ImGui::SetColumnWidth(5, 40.0f);

        ImGui::NextColumn();
        ImGui::Text("Position");
        ImGui::NextColumn();
        ImGui::Text("Colour");
        ImGui::NextColumn();
        ImGui::Text("Range");
        ImGui::NextColumn();
        ImGui::Text("Shadows");
        ImGui::NextColumn();
        ImGui::Text("Remove");
        ImGui::NextColumn();

        for (int i = 0; i < sbo_lights.size(); ++i)
        {
            char id[64];
            sprintf(&id[0], "#%d", i);
            ImGui::Text(id);
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-pos", i);
            ImGui::PushItemWidth(220.0f);
            if (ImGui::SliderFloat3(id, &sbo_lights[i].position.x, -20.0f, 20.0f)) { r.blob_changes[1] = true; }
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::PushItemWidth(180.0f);
            if (ImGui::ColorEdit3(id, &sbo_lights[i].colour.x)) { r.blob_changes[1] = true; }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-range", i);
            ImGui::PushItemWidth(-1);
            if (ImGui::SliderFloat(id, &sbo_lights[i].range, 0.5f, 50.0f)) { r.blob_changes[1] = true; }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-casts", i);
            ImGui::PushItemWidth(-1);
            if (ImGui::SliderFloat(id, &sbo_lights[i].shadow_factor, 0.0f, 1.0f)) { r.blob_changes[1] = true; }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            if (sbo_lights.size() > 1) {
                sprintf(&id[0], "Delete:%d", i);
                if (ImGui::Button(id)) {
                    light_to_delete = i;
                }
            }
            ImGui::NextColumn();

        }

        if (ImGui::Button("Add light")) {
            light_to_add = true;
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

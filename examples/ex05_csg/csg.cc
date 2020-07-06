#include <sge_app.hh>

#ifdef TARGET_MACOSX
#define USE_SBO_MATERIALS 1
#define USE_SBO_LIGHTS 0
#define USE_SBO_SCENE 0
#else
#define USE_SBO_MATERIALS 1
#define USE_SBO_LIGHTS 1
#define USE_SBO_SCENE 1
#endif

#define UPDATE_STORAGE_BUFFER_DELAY 0.2f

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




sge::app::configuration config = {};
sge::app::content computation = {};
sge::app::extensions extensions = {};

PUSH push;
UBO_CAMERA ubo_camera;
UBO_SETTINGS ubo_settings;

std::vector<std::optional<sge::dataspan>> blobs_changed;
float last_blob_update_time = 0.0f;


#if (USE_SBO_MATERIALS == 1)
struct Material { sge::math::vector3 colour; float shininess; };
typedef std::vector<Material> SBO_MATERIALS;
SBO_MATERIALS sbo_materials;
sge::dataspan current_materials_dataspan () { return sge::dataspan{ sbo_materials.data (), sbo_materials.size () * sizeof (Material) }; }
#endif

#if (USE_SBO_LIGHTS == 1)
struct PointLight {
    sge::math::vector3 position;
    float range; // for now linear fall off over range
    sge::math::vector3 colour;
    float shadow_factor;
};
typedef std::vector<PointLight> SBO_LIGHTS;
SBO_LIGHTS sbo_lights;
sge::dataspan current_lights_dataspan () { return sge::dataspan{ sbo_lights.data (), sbo_lights.size () * sizeof (PointLight) }; }
#endif

#if (USE_SBO_SCENE == 1)
enum TreeElement : uint32_t { TREE_NODE, TREE_LEAF };
enum ShapeType : uint32_t { SDF_SPHERE, SDF_CUBE, SDF_CUBOID };
enum CSG_OP : uint32_t { CSG_UNION, CSG_INTERSECTION, CSG_DIFFERENCE };
struct Shape { sge::math::vector3 position; uint32_t type; sge::math::vector4 data; };
struct Tree { uint32_t type; uint32_t data; };
typedef std::vector<Shape> SBO_SHAPES;
typedef std::vector<Tree> SBO_TREE;
SBO_SHAPES sbo_shapes;
SBO_TREE sbo_tree;
sge::dataspan current_shapes_dataspan () { return sge::dataspan{ sbo_shapes.data (), sbo_shapes.size () * sizeof (Shape) }; }
sge::dataspan current_tree_dataspan () { return sge::dataspan{ sbo_tree.data (), sbo_tree.size () * sizeof (Tree) }; }
#endif

void initialise () {
    config.app_name = "Dynamic CSG evaluation";
    config.app_width = 960;
    config.app_height = 540;
    config.enable_console = true;

    // initial ubo settings
    ubo_camera.aspect = (float)config.app_width / (float)config.app_height;
    ubo_settings.iterations = 64;
    ubo_settings.display_mode = 0;
#if (USE_SBO_MATERIALS == 1)
    const auto magenta      = sge::math::vector3 { 1.00, 0.00, 1.00 };
    const auto my_black     = sge::math::vector3 { 0.03, 0.04, 0.08 };
    const auto yellow       = sge::math::vector3 { 1.00, 0.90, 0.00 };
    const auto white        = sge::math::vector3 { 1.00, 1.00, 1.00 };
    const auto blue         = sge::math::vector3 { 0.00, 0.00, 1.00 };
    const auto green        = sge::math::vector3 { 0.00, 1.00, 0.00 };
    const auto red          = sge::math::vector3 { 1.00, 0.00, 0.00 };

    sbo_materials.emplace_back (Material { magenta, 0 });
    sbo_materials.emplace_back (Material { my_black, 1 });
    sbo_materials.emplace_back (Material { yellow, 51 });
    sbo_materials.emplace_back (Material { white, 3 });
    sbo_materials.emplace_back (Material { blue, 54 });
    sbo_materials.emplace_back (Material { green, 54 });
    sbo_materials.emplace_back (Material { red, 54 });
#endif

#if (USE_SBO_LIGHTS == 1)
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {2.143f, 4.286f, -2.069f}, 20.395f, sge::math::vector3 {0.91f, 0, 1}, 0.266f });
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {-1.429, 0.714f, 4.138f}, 19.684f, sge::math::vector3 {0.96f, 0.32f, 0.12f}, 0.459f });
    sbo_lights.emplace_back (PointLight { sge::math::vector3 {0, 0, 0}, 3.105f, sge::math::vector3 {0.16f, 0.65f, 0}, 0.422f });
#endif

#if (USE_SBO_SCENE == 1)
    sbo_shapes.emplace_back (Shape{ sge::math::vector3 {0, -5.8, 0},  ShapeType::SDF_CUBE, sge::math::vector4 {10, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-3.5, 0.7, -3.5}, ShapeType::SDF_CUBE, sge::math::vector4 {3, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{2, 0.2, -4.5}, ShapeType::SDF_CUBOID, sge::math::vector4 {1, 2.4, 1, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-4, 0.2, 0}, ShapeType::SDF_CUBE, sge::math::vector4 {2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-4.5, 0.2, 2}, ShapeType::SDF_CUBOID, sge::math::vector4 {1, 2.8, 1, 0} });

    sbo_shapes.emplace_back (Shape{ sge::math::vector3{0, 0, 0}, ShapeType::SDF_CUBE, sge::math::vector4 {1.9, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{0, 0, 0}, ShapeType::SDF_SPHERE, sge::math::vector4 {1.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{0, 0, 0}, ShapeType::SDF_CUBOID, sge::math::vector4 {2.2, 0.7, 0.7, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{0, 0, 0}, ShapeType::SDF_CUBOID, sge::math::vector4 {0.7, 2.2, 0.7, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{0, 0, 0}, ShapeType::SDF_CUBOID, sge::math::vector4 {0.7, 0.7, 2.2, 0} });

    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-1.35, -0.6, 1.65}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-1.45, -0.6, 2.35}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-1.75, -0.6, 1.09}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-2.70, -0.6, 2.05}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-2.00, -0.6, 3.25}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{-3.20, -0.6, 3.60}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{2.50, -0.6, 0.40}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{2.50, -0.6, 1.40}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{1.50, -0.6, 1.40}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{2.50, -0.6, -2.85}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{1.70, -0.6, -2.15}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });
    sbo_shapes.emplace_back (Shape{ sge::math::vector3{2.30, -0.6, -3.55}, ShapeType::SDF_SPHERE, sge::math::vector4 {0.2, 0, 0, 0} });

    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (1u << 24) | 0u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (1u << 24) | 1u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (1u << 24) | 2u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (1u << 24) | 3u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (1u << 24) | 4u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });

    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (3u << 24) | 5u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (3u << 24) | 6u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_INTERSECTION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (3u << 24) | 7u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (3u << 24) | 8u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (3u << 24) | 9u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_DIFFERENCE });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });

    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (4u << 24) | 10u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (4u << 24) | 11u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (4u << 24) | 12u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (5u << 24) | 13u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (5u << 24) | 14u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (5u << 24) | 15u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (6u << 24) | 16u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (6u << 24) | 17u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (6u << 24) | 18u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (2u << 24) | 19u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (2u << 24) | 20u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_LEAF, (2u << 24) | 21u });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });
    sbo_tree.emplace_back (Tree{ TreeElement::TREE_NODE, CSG_OP::CSG_UNION });

#endif

    computation.shader_path = "csg.comp.spv";
    computation.push_constants = std::optional<sge::dataspan> ({ &push, sizeof (PUSH) });
    computation.uniforms = {
        sge::dataspan { &ubo_camera, sizeof (UBO_CAMERA) },
        sge::dataspan { &ubo_settings, sizeof (UBO_SETTINGS) },
    };
    computation.blobs = {
#if (USE_SBO_MATERIALS == 1)
        current_materials_dataspan (),
#endif
#if (USE_SBO_LIGHTS == 1)
        current_lights_dataspan (),
#endif
#if (USE_SBO_SCENE == 1)
        current_shapes_dataspan (),
        current_tree_dataspan (),
#endif
    };

    blobs_changed.resize (computation.blobs.size ());

    push.time = 0.0f;
    push.no_change = false;
}

void terminate () {}

void start (const sge::app::api& sge) {
    sge.freecam.set_active (true);
}

void stop (const sge::app::api& sge) {}

bool has_local_blob_change = false;

void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    int res_x = sge.runtime.system__get_state_int(sge::runtime::system_int_state::screenwidth);
    int res_y = sge.runtime.system__get_state_int (sge::runtime::system_int_state::screenheight);

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

    const bool blob_update_needed = std::find_if (blobs_changed.begin (), blobs_changed.end (), [](std::optional<sge::dataspan> x) { return x.has_value ();  }) != blobs_changed.end ();

    if (blob_update_needed && last_blob_update_time + UPDATE_STORAGE_BUFFER_DELAY < sge.instrumentation.timer ()) {
        // no need to update blobs every frames when user is just changing colours
        push.no_change = false;
        r.push_constants_changed = true;
        r.blob_changes = blobs_changed;
        last_blob_update_time = sge.instrumentation.timer ();
        blobs_changed.clear ();
        blobs_changed.resize (computation.blobs.size ());
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
#if (USE_SBO_MATERIALS == 1)
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

            ImGui::Text("%s", id);
            ImGui::NextColumn();

            sprintf(&id[0], "##mat:%d-colour", i);

            ImGui::PushItemWidth(180.0f);
            if (ImGui::ColorEdit3 (id, &sbo_materials[i].colour.x)) { blobs_changed[0] = current_materials_dataspan (); }
            ImGui::PopItemWidth();

            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            sprintf(&id[0], "##mat:%d-shininess", i);
            if (ImGui::SliderFloat(id, &(sbo_materials[i].shininess), 0.0f, 256.0f)) { blobs_changed[0] = current_materials_dataspan ();  }
            ImGui::PopItemWidth();
            ImGui::NextColumn();
        }
    }
    ImGui::End();
#endif

    // ---------------------------------
#if (USE_SBO_LIGHTS == 1)
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
            ImGui::Text("%s", id);
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-pos", i);
            ImGui::PushItemWidth(220.0f);
            if (ImGui::SliderFloat3(id, &sbo_lights[i].position.x, -20.0f, 20.0f)) { blobs_changed[1] = current_lights_dataspan (); }
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::PushItemWidth(180.0f);
            if (ImGui::ColorEdit3(id, &sbo_lights[i].colour.x)) { blobs_changed[1] = current_lights_dataspan (); }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-range", i);
            ImGui::PushItemWidth(-1);
            if (ImGui::SliderFloat(id, &sbo_lights[i].range, 0.5f, 50.0f)) { blobs_changed[1] = current_lights_dataspan (); }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            sprintf(&id[0], "##light:%d-casts", i);
            ImGui::PushItemWidth(-1);
            if (ImGui::SliderFloat(id, &sbo_lights[i].shadow_factor, 0.0f, 1.0f)) { blobs_changed[1] = current_lights_dataspan (); }
            ImGui::PopItemWidth();
            ImGui::NextColumn();

            if (sbo_lights.size() > 1) {
                sprintf(&id[0], "Delete:%d", i);
                if (ImGui::Button(id)) {
                    sbo_lights.erase (sbo_lights.begin () + i);
                    blobs_changed[1] = current_lights_dataspan ();
                }
            }
            ImGui::NextColumn();

        }

        if (ImGui::Button("Add light")) {
            sbo_lights.emplace_back (PointLight{ sge::math::vector3 {0, 5, 5}, 18.0f, sge::math::vector3 {0.5, 0.21, 0.9}, 0 });
            blobs_changed[1] = current_lights_dataspan ();
        }
    }
    ImGui::End();
#endif

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

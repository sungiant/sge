#include <sge_app.hh>

//todo: this demo is placeholder and is almost entirely a clone of sascha's compute raytracer.
//      i've added it for now to try and help illustrate and debug a molten vk issue relating to
//      more than a single SBO in a compute shader.

sge::app::configuration config = {};
sge::app::content computation = {};
sge::app::extensions extensions = {};

struct PUSH { float time = 0.0f; } push;

struct UBO {
    sge::math::vector3          position        = { 0.0f, 0.0f, 4.0f };
    float                       gamma           = 1.2f;
    sge::math::quaternion       orientation     = { 0.0f, 1.0f, 0.0f, 0.0f };
    sge::math::vector3          fog_colour      = { 0.3f, 0.1f, 0.3f };
    float                       fov             = 100.0f;

    bool operator == (const UBO& ubo) const {
        return position == ubo.position
            && gamma == ubo.gamma
            && orientation == ubo.orientation
            && fog_colour == ubo.fog_colour
            && fov == ubo.fov;
    }
    bool operator != (const UBO& ubo) const { return !(*this == ubo); }
} ubo;

struct Sphere {
    sge::math::vector3 pos;
    float radius;
    sge::math::vector3 diffuse;
    float specular;
    uint32_t id;
    sge::math::vector3 padding;
};

std::vector<Sphere> sbo_spheres;

struct Plane {
    sge::math::vector3 normal;
    float distance;
    sge::math::vector3 diffuse;
    float specular;
    uint32_t id;
    sge::math::vector3 padding;
};

std::vector<Plane> sbo_planes;

int currentId = 0;

Sphere newSphere(sge::math::vector3 pos, float radius, sge::math::vector3 diffuse, float specular) {
    Sphere sphere;
    sphere.id = currentId++;
    sphere.pos = pos;
    sphere.radius = radius;
    sphere.diffuse = diffuse;
    sphere.specular = specular;
    return sphere;
}

Plane newPlane(sge::math::vector3 normal, float distance, sge::math::vector3 diffuse, float specular) {
    Plane plane;
    plane.id = currentId++;
    plane.normal = normal;
    plane.distance = distance;
    plane.diffuse = diffuse;
    plane.specular = specular;
    return plane;
}

void initialise () {
    config.app_name = "Raytracing";
    config.app_width = 1280;
    config.app_height = 720;
    config.enable_console = true;

    sbo_spheres.push_back(newSphere(sge::math::vector3 { 1.75f, -0.5f, 0.0f }, 1.0f, sge::math::vector3 { 0.0f, 1.0f, 0.0f }, 32.0f));
    sbo_spheres.push_back(newSphere(sge::math::vector3 { 0.0f, 1.0f, -0.5f }, 1.0f, sge::math::vector3 { 0.65f, 0.77f, 0.97f }, 32.0f));
    sbo_spheres.push_back(newSphere(sge::math::vector3 { -1.75f, -0.75f, -0.5f }, 1.25f, sge::math::vector3 { 0.9f, 0.76f, 0.46f }, 32.0f));

    const float roomDim = 4.0f;
    sbo_planes.push_back(newPlane(sge::math::vector3 { 0.0f, 1.0f, 0.0f }, roomDim, sge::math::vector3 { 1.0f, 1.0f, 1.0f }, 32.0f));
    sbo_planes.push_back(newPlane(sge::math::vector3 { 0.0f, -1.0f, 0.0f }, roomDim, sge::math::vector3 { 1.0f, 1.0f, 1.0f }, 32.0f));
    sbo_planes.push_back(newPlane(sge::math::vector3 { 0.0f, 0.0f, 1.0f }, roomDim, sge::math::vector3 { 1.0f, 1.0f, 1.0f }, 32.0f));
    sbo_planes.push_back(newPlane(sge::math::vector3 { 0.0f, 0.0f, -1.0f }, roomDim, sge::math::vector3 { 0.0f, 0.0f, 0.0f }, 32.0f));
    sbo_planes.push_back(newPlane(sge::math::vector3 { -1.0f, 0.0f, 0.0f}, roomDim, sge::math::vector3 { 1.0f, 0.0f, 0.0f }, 32.0f));
    sbo_planes.push_back(newPlane(sge::math::vector3 { 1.0f, 0.0f, 0.0f }, roomDim, sge::math::vector3 { 0.0f, 1.0f, 0.0f }, 32.0f));

    computation.shader_path = "raytracing.comp.spv";
    computation.push_constants = std::optional<sge::dataspan> ({ &push, sizeof (PUSH) });
    computation.uniforms = { sge::dataspan { &ubo, sizeof (UBO) }, };
    computation.blobs = {
        sge::dataspan { sbo_spheres.data(), sbo_spheres.size() * sizeof(Sphere) },
        sge::dataspan { sbo_planes.data(), sbo_planes.size() * sizeof(Plane) },
    };
}

void terminate () {}

void start (const sge::app::api& sge) {
    sge.freecam.set_enabled (true);
    sge.freecam.position = ubo.position;
    sge.freecam.orientation = ubo.orientation;
}

void stop (const sge::app::api& sge) {}

void update (sge::app::response& r, const sge::app::api& sge) {
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    UBO u = ubo;

    u.position = sge.freecam.position;
    u.orientation = sge.freecam.orientation;
    u.fov = sge.freecam.fov;
    if (u != ubo) {
        ubo = u;
        r.uniform_changes[0] = true;
    }

    push.time = sge.instrumentation.timer();
    r.push_constants_changed = true;
}

void debug_ui (sge::app::response& r, const sge::app::api& sge) {
    UBO u = ubo;
    ImGui::Begin ("Raytracing");
    {
        ImGui::SliderFloat("gamma", &u.gamma, 0, 4.0f);
        ImGui::ColorEdit3("fog colour", &u.fog_colour.x);
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
configuration&     get_configuration   ()                              { return ::config; }
content&           get_content         ()                              { return ::computation; }
extensions&        get_extensions      ()                              { return ::extensions; }
void               start               (const api& sge)                { ::start (sge); }
void               update              (response& r, const api& sge)   { ::update (r, sge); }
void               debug_ui            (response& r, const api& sge)   { ::debug_ui (r, sge); }
void               stop                (const api& sge)                { ::stop (sge); }
void               terminate           ()                              { ::terminate (); }

}

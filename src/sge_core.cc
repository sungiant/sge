#include "sge_core.hh"

#include <sstream>
#include <functional>
#include <utility>

namespace sge::core {

void internal_update (sge::app::response& user_response, engine_state& engine_state, engine_tasks& engine_tasks) {
    auto tStart = std::chrono::high_resolution_clock::now ();

    // USER TASKS
    {
        if (engine_tasks.change_imgui_enabled.has_value ()) {
            engine_state.graphics.imgui_on = engine_tasks.change_imgui_enabled.value ();
            engine_tasks.change_imgui_enabled.reset ();
        }

        if (engine_tasks.change_fullscreen_enabled.has_value () && engine_state.host.set_window_fullscreen_fn.has_value ()) {
            const bool v = engine_tasks.change_fullscreen_enabled.value ();
            engine_state.host.set_window_fullscreen_fn.value() (v);
            engine_state.host.is_fullscreen = v;
            engine_tasks.change_fullscreen_enabled.reset ();
            engine_state.host.container_just_changed = true;
        }

        if (engine_tasks.change_window_title.has_value () && engine_state.host.set_window_title_fn.has_value ()) {
            const std::string& v = engine_tasks.change_window_title.value ();
            engine_state.host.set_window_title_fn.value () (v.c_str ());
            engine_state.host.window_title = v;
            engine_tasks.change_window_title.reset ();
        }

        if ((engine_tasks.change_screen_width.has_value () || engine_tasks.change_screen_height.has_value ()) && engine_state.host.set_window_size_fn.has_value ()) {
            const int vw = engine_tasks.change_screen_width.has_value () ? engine_tasks.change_screen_width.value () : engine_state.container.current_width;
            const int vh = engine_tasks.change_screen_height.has_value() ? engine_tasks.change_screen_height.value () : engine_state.container.current_height;
            engine_state.host.set_window_size_fn.value () (vw, vh);
            engine_tasks.change_screen_width.reset ();
            engine_tasks.change_screen_height.reset ();
            engine_state.host.container_just_changed = true;
        }

        if (engine_tasks.shutdown_request.has_value ()) {
            engine_state.host.shutdown_request_fn.value() ();
            engine_tasks.shutdown_request.reset ();
        }
    }

    // IMGUI
    {
        ImGuiIO& io = ImGui::GetIO ();
        auto& input = engine_state.input;

        auto p = (input.find (input_control_identifier::mp_position) != input.end ())
            ? std::get<input_point_control> (input.at (input_control_identifier::mp_position))
        : sge::math::point2 { 0, 0 };

        io.MousePos = ImVec2 (p.x, p.y);
        io.MouseDown[0] = input.find (input_control_identifier::mb_left) != input.end () && std::get<input_binary_control>(input.at(input_control_identifier::mb_left));
        io.MouseDown[1] = input.find (input_control_identifier::mb_middle) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::mb_middle));
        io.MouseDown[2] = input.find (input_control_identifier::mb_right) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::mb_right));
        io.DeltaTime = engine_state.instrumentation.frameTimer;
        io.MouseWheel = (input.find (input_control_identifier::md_scrollwheel) != input.end ())
            ? (float) std::get<input_digital_control> (input.at (input_control_identifier::md_scrollwheel))
            : 0.0f;

        io.KeyCtrl = input.find (input_control_identifier::kb_control) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_control));
        io.KeyShift = input.find (input_control_identifier::kb_shift) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_shift));
        io.KeyAlt = input.find (input_control_identifier::kb_alt) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_alt));
        io.KeySuper = input.find (input_control_identifier::kb_cmd) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_cmd));

    }

    // VULKAN
    engine_state.graphics.update (
        user_response.push_constants_changed,
        user_response.uniform_changes,
        user_response.blob_changes,
        engine_state.instrumentation.frameTimer);

    // INSTRUMENTATION
    {
        engine_state.instrumentation.frameCounter++;
        auto tEnd = std::chrono::high_resolution_clock::now ();
        auto tDiff = std::chrono::duration<double, std::milli> (tEnd - tStart).count ();
        engine_state.instrumentation.frameTimer = (float)tDiff / 1000.0f;
        engine_state.instrumentation.totalTimer += engine_state.instrumentation.frameTimer;
        float fpsTimer = (float)(std::chrono::duration<double, std::milli> (tEnd - engine_state.instrumentation.lastTimestamp).count ());
        if (fpsTimer > 1000.0f) {
            engine_state.instrumentation.lastFPS = static_cast<uint32_t>((float) engine_state.instrumentation.frameCounter * (1000.0f / fpsTimer));

            engine_state.instrumentation.frameCounter = 0;
            engine_state.instrumentation.lastTimestamp = tEnd;
        }
    }
}
    

    
    
//--------------------------------------------------------------------------------------------------------------------//

api_impl::api_impl (const core::engine_state& z_state, core::engine_tasks& z_tasks, std::unordered_map<size_t, std::unique_ptr<runtime::extension>>& z_exts)
    : engine_state (z_state)
    , engine_tasks (z_tasks)
    , engine_extensions (z_exts)
{}

bool api_impl::system__get_state_bool (runtime::system_bool_state z) const {
    switch (z){
        
        case runtime::system_bool_state::fullscreen: return engine_state.host.is_fullscreen;
        case runtime::system_bool_state::imgui: return engine_state.graphics.imgui_on;
    }
}
int api_impl::system__get_state_int (runtime::system_int_state z) const {
    switch (z) {
        case runtime::system_int_state::screenwidth: return engine_state.container.current_width;
        case runtime::system_int_state::screenheight: return engine_state.container.current_height;
    }
}
const char* api_impl::system__get_state_string (runtime::system_string_state z) const {
    switch (z) {
        case runtime::system_string_state::title: return engine_state.host.window_title.c_str ();
        case runtime::system_string_state::gpu_name: return engine_state.graphics.kernel->get_physical_device_name();
        case runtime::system_string_state::engine_version: {
            return engine_state.version.c_str ();
        }
    }
}

bool api_impl::system__did_container_just_change () const { return engine_state.host.container_just_changed; }

uint32_t api_impl::timer__get_fps () const { return engine_state.instrumentation.lastFPS; }
float api_impl::timer__get_delta () const { return engine_state.instrumentation.frameTimer; }
float api_impl::timer__get_time () const { return engine_state.instrumentation.totalTimer; }

void api_impl::input__get_state (input_state& z) const { z = engine_state.input; }


//--------------------------------------------------------------------------------------------------------------------//
// Non const functions - these don't do anything directly, the just add stuff to the engine task queue.
//--------------------------------------------------------------------------------------------------------------------//

void api_impl::system__request_shutdown () {
    engine_tasks.shutdown_request = std::monostate {};
}

void api_impl::system__toggle_state_bool (runtime::system_bool_state z) {
    bool v = system__get_state_bool (z);
    system__set_state_bool (z, !v);
}
void api_impl::system__set_state_bool (runtime::system_bool_state z, bool v) {
    switch (z){
        case runtime::system_bool_state::fullscreen: engine_tasks.change_fullscreen_enabled = v; break;
        case runtime::system_bool_state::imgui: engine_tasks.change_imgui_enabled = v; break;
        default: break;
    }
}
void api_impl::system__set_state_int (runtime::system_int_state z, int v) {
    switch (z){
        case runtime::system_int_state::screenwidth: engine_tasks.change_screen_width = v; break;
        case runtime::system_int_state::screenheight: engine_tasks.change_screen_height = v; break;
        default: break;
    }
}
void api_impl::system__set_state_string (runtime::system_string_state z, const char* v) {
    switch (z){
        case runtime::system_string_state::title: engine_tasks.change_window_title = std::string (v); break;
        default: break;
    }
}

runtime::extension* api_impl::extension_get  (size_t id) const {
    assert (engine_extensions.find(id) != engine_extensions.end());
    runtime::extension* ext = engine_extensions.at(id).get();
    assert (ext);
    return ext;
};








    
//--------------------------------------------------------------------------------------------------------------------//
engine::engine () {
    
    app::initialise ();
}

engine::~engine () {
    app::terminate ();
}

void engine::setup (
#if VARIANT_HEADLESS
#elif TARGET_WIN32
    HINSTANCE z_hinst,
    HWND z_hwnd
#elif TARGET_MACOSX
    void* z_view
#elif TARGET_LINUX
    xcb_connection_t* z_connection;
    xcb_window_t* z_window;
#endif
) {
    std::cout <<
        "\n"
        "   _________ ___________________\n"
        "  /   _____//  _____/\\_   _____/\n"
        "  \\_____  \\/   \\  ___ |    __)_\n"
        "  /        \\    \\_\\  \\|        \\\n"
        " /_______  /\\______  /_______  /\n"
        "         \\/        \\/  0.1.0 \\/\n"
        "\n\n";

    engine_state = std::make_unique<struct engine_state> ();
    
    std::stringstream ss;
    ss << SGE_VERSION_MAJOR (SGE_VERSION) << "." << SGE_VERSION_MINOR (SGE_VERSION) << "." << SGE_VERSION_PATCH (SGE_VERSION);
    engine_state->version = ss.str();
    
    engine_tasks = std::make_unique<struct engine_tasks> ();

    auto configuration = sge::app::get_configuration ();

    engine_tasks->change_window_title = configuration.app_name;

#if VARIANT_HEADLESS
    engine_state->graphics.create (engine_state->container.current_width, engine_state->container.current_height);
#elif TARGET_WIN32
    engine_state->platform.hinst = z_hinst;
    engine_state->platform.hwnd = z_hwnd;
    engine_state->graphics.create (z_hinst, z_hwnd, engine_state->container.current_width, engine_state->container.current_height);
#elif TARGET_MACOSX
    engine_state->platform.view = z_view;
    engine_state->graphics.create (z_view, engine_state->container.current_width, engine_state->container.current_height);
#elif TARGET_LINUX
    engine_state->graphics.create (z_connection, z_window, engine_state->container.current_width, engine_state->container.current_height);
#else
#endif

    engine_api = std::make_unique<api_impl> (*engine_state, *engine_tasks, engine_extensions);

    auto& user_extensions = sge::app::get_extensions ();

    // I am certain that this can be done in a much better way with some template wizardary.
    for (int i = 0; i < user_extensions.views.size(); ++i) {
        size_t id = user_extensions.views[i].first;
        auto& new_fn = user_extensions.views[i].second;
        runtime::view* view = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (view);
    }
    
 
    for (int i = 0; i < user_extensions.systems.size(); ++i) {
        size_t id = user_extensions.systems[i].first;
        auto& new_fn = user_extensions.systems[i].second;
        runtime::system* system = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (system);
    }
    

    user_response = std::make_unique<struct app::response> (app::get_content ().uniforms.size (), app::get_content ().blobs.size ());
    user_api = std::make_unique<app::api> (*engine_api);

    for (auto& kvp : engine_extensions) {
        const size_t id = kvp.first;
        debug_fns.emplace_back ([this, id]() { engine_extensions[id]->debug_ui (); });
    }

    debug_fns.emplace_back ([this]() { sge::app::debug_ui (*user_response, *user_api); });

}

void engine::start () {

    sge::app::start (*user_api);
    engine_state->graphics.create_systems (debug_fns);
}

void engine::update (container_state& z_container, input_state& z_input) {

    engine_state->host.container_just_changed = false;

    if (z_container.current_width != engine_state->container.current_width
        || z_container.current_height != engine_state->container.current_height
        || z_container.is_resizing) {
        engine_state->host.container_just_changed = true;
    }

    // copy new state provided by the host
    engine_state->container = z_container;
    engine_state->input = z_input;


    // update the engine - not dependent on engine_extensions here as they've not been updated.
    internal_update (*user_response, *engine_state, *engine_tasks);

    // update all registered extensions
    for (auto& kvp : engine_extensions) {
        kvp.second->update ();
    }
    // update the user's app
    sge::app::update (*user_response, *user_api);


}
void engine::stop () {
    sge::app::stop (*user_api);
}

void engine::shutdown () {
    user_api.reset ();
    user_response.reset ();
    engine_extensions.clear ();
    engine_state->graphics.destroy ();
    engine_tasks.reset ();
    engine_state.reset ();
}

}

#include "sge_core.hh"

#include "sge_extensions.hh" // TODO: core shouldn't know about specific runtime extensions...

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

engine::engine () {
    app::initialise ();
}

engine::~engine () {
    app::terminate ();
}

void engine::setup (
#if TARGET_WIN32
    HINSTANCE z_hinst,
    HWND z_hwnd
#elif TARGET_MACOSX
    void* z_view
#elif TARGET_LINUX
    xcb_connection_t* z_connection,
    xcb_window_t* z_window
#else
#error
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
    engine_tasks = std::make_unique<struct engine_tasks> ();

    auto configuration = sge::app::get_configuration ();

    engine_tasks->change_window_title = configuration.app_name;

#if TARGET_WIN32
    engine_state->platform.hinst = z_hinst;
    engine_state->platform.hwnd = z_hwnd;
    engine_state->graphics.create (z_hinst, z_hwnd, engine_state->container.current_width, engine_state->container.current_height);
#elif TARGET_MACOSX
    engine_state->platform.view = z_view;
    engine_state->graphics.create (z_view, engine_state->container.current_width, engine_state->container.current_height);
#elif TARGET_LINUX
    engine_state->graphics.create (z_connection, z_window, engine_state->container.current_width, engine_state->container.current_height);
#else
#error
#endif

    engine_api = std::make_unique<runtime::api> (*engine_state, *engine_tasks);

    auto up_input = std::unique_ptr<runtime::extension> (new input::view (*engine_api));
    auto up_instrumentation = std::unique_ptr<runtime::extension> (new instrumentation::view (*engine_api));
    auto up_overlay = std::unique_ptr<runtime::extension> (new overlay::system (*engine_api));

    engine_extensions.emplace_back (std::move (up_input));
    engine_extensions.emplace_back (std::move (up_instrumentation));
    engine_extensions.emplace_back (std::move (up_overlay));

    user_response = std::make_unique<struct app::response> (app::get_content ().uniforms.size (), app::get_content ().blobs.size ());
    user_api = std::make_unique<app::api> (
        *engine_api,
        // yuck
        static_cast<input::view&>(*engine_extensions[0]),
        static_cast<instrumentation::view&>(*engine_extensions[1]),
        static_cast<overlay::system&>(*engine_extensions[2]));

    // yuck
    debug_fns.emplace_back ([this]() { engine_extensions[2]->debug_ui (); });

    //engine_state->graphics.kernel->append_debug_fns (debug_fns);
    //for (int i = 0; i < engine_extensions.size (); ++i) {
    //    debug_fns.emplace_back ([this, i]() { engine_extensions[i]->debug_ui (); });
    //}

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
    for (auto& e : engine_extensions) {
        e->update ();
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

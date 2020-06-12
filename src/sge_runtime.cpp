#include "sge_runtime.hpp"

#include "sge_core.hpp"
#include "sge_app.hpp"

namespace sge::runtime {

api::api (const core::engine_state& z_state, core::engine_tasks& z_tasks)
    : engine_state (z_state)
    , engine_tasks (z_tasks)
{}

bool api::system__get_state_bool (system_bool_state z) const {
    switch (z){
        
        case system_bool_state::fullscreen: return engine_state.host.is_fullscreen;
        case system_bool_state::imgui: return engine_state.graphics.imgui_on;
    }
}
int api::system__get_state_int (system_int_state z) const {
    switch (z) {
        case system_int_state::screenwidth: return engine_state.container.current_width;
        case system_int_state::screenheight: return engine_state.container.current_height;
    }
}
const char* api::system__get_state_string (system_string_state z) const {
    switch (z) {
        case system_string_state::title: return engine_state.host.window_title.c_str ();
        case system_string_state::gpu_name: return engine_state.graphics.kernel->get_physical_device_name();
    }
}

bool api::system__did_container_just_change () const { return engine_state.host.container_just_changed; }

uint32_t api::timer__get_fps () const { return engine_state.instrumentation.lastFPS; }
float api::timer__get_delta () const { return engine_state.instrumentation.frameTimer; }
float api::timer__get_time () const { return engine_state.instrumentation.totalTimer; }

void api::input__get_state (input_state& z) const { z = engine_state.input; }


//--------------------------------------------------------------------------------------------------------------------//
// Non const functions - these don't do anything directly, the just add stuff to the engine task queue.
//--------------------------------------------------------------------------------------------------------------------//

void api::system__request_shutdown () {
    engine_tasks.shutdown_request = std::monostate {};
}

void api::system__toggle_state_bool (system_bool_state z) {
    bool v = system__get_state_bool (z);
    system__set_state_bool (z, !v);
}
void api::system__set_state_bool (system_bool_state z, bool v) {
    switch (z){
        case system_bool_state::fullscreen: engine_tasks.change_fullscreen_enabled = v; break;
        case system_bool_state::imgui: engine_tasks.change_imgui_enabled = v; break;
        default: break;
    }
}
void api::system__set_state_int (system_int_state z, int v) {
    switch (z){
        case system_int_state::screenwidth: engine_tasks.change_screen_width = v; break;
        case system_int_state::screenheight: engine_tasks.change_screen_height = v; break;
        default: break;
    }
}
void api::system__set_state_string (system_string_state z, const char* v) {
    switch (z){
        case system_string_state::title: engine_tasks.change_window_title = std::string (v); break;
        default: break;
    }
}

}


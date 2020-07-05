#include <sge_app.hh>

sge::app::configuration config = {};
sge::app::content computation = {};
sge::app::extensions extensions = {};

struct PUSH { float time = 0.0f; } push;

void initialise () {
    config.app_name = "sinewaves";
    config.app_width = 1280;
    config.app_height = 720;
    config.enable_console = true;
    
    computation.shader_path = "sinewaves.comp.spv";
    computation.push_constants = std::optional<sge::dataspan> ({ &push, sizeof (PUSH) });
}

void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.input.keyboard.key_just_pressed (sge::runtime::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    char temp[32];
    sprintf (temp, "%s (%d fps)", config.app_name.c_str (), sge.runtime.timer__get_fps ());

    sge.runtime.system__set_state_string (sge::runtime::system_string_state::title, temp);
 
    push.time = sge.instrumentation.timer();
    r.push_constants_changed = true;
}


//--------------------------------------------------------------------------------------------------------------------//
namespace sge::app { // HOOK UP TO SGE

void               initialise          ()                              { ::initialise (); }
configuration&     get_configuration   ()                              { return ::config; }
content&           get_content         ()                              { return ::computation; }
extensions&        get_extensions      ()                              { return ::extensions; }
void               start               (const api& sge)                { ; }
void               update              (response& r, const api& sge)   { ::update (r, sge); }
void               debug_ui            (response& r, const api& sge)   { ; }
void               stop                (const api& sge)                { ; }
void               terminate           ()                              { ; }

}

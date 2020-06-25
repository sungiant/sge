#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.hh>
#include <ext_overlay.hh>
#include <ext_input.hh>
#include <ext_instrumentation.hh>

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;
std::unique_ptr<sge::app::extensions> extensions;

struct PUSH { float time = 0.0f; } push;

void initialise () {
    config = std::make_unique<sge::app::configuration> ();
    config->app_name = "sinewaves";
    config->app_width = 640;
    config->app_height = 360;
    config->enable_console = true;

    computation = std::make_unique<sge::app::content>(sge::app::content {
        "sinewaves.comp.spv",
        std::optional<sge::dataspan> ({ &push, sizeof (PUSH) }),
        {}
    });
    
    extensions = std::make_unique<sge::app::extensions>();
    
    extensions->views = {
        { sge::type_id<sge::overlay::view>(), [] (const sge::runtime::api& x) { return new sge::overlay::view (x); }},
        { sge::type_id<sge::input::view>(), [] (const sge::runtime::api& x) { return new sge::input::view (x); }},
        { sge::type_id<sge::instrumentation::view>(), [] (const sge::runtime::api& x) { return new sge::instrumentation::view (x); }},
    };
    extensions->systems = {};
}

void terminate () { config.reset (); }

void update (sge::app::response& r, const sge::app::api& sge) {

    if (sge.ext<sge::input::view>().keyboard.key_just_pressed (sge::input::keyboard_key::escape)) { sge.runtime.system__request_shutdown (); }
    if (sge.ext<sge::input::view>().keyboard.key_just_pressed (sge::input::keyboard_key::o)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::imgui); }
    if (sge.ext<sge::input::view>().keyboard.key_just_pressed (sge::input::keyboard_key::f)) { sge.runtime.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen); }

    char temp[32];
    sprintf (temp, "%s (%d fps)", config->app_name.c_str (), sge.runtime.timer__get_fps ());

    sge.runtime.system__set_state_string (sge::runtime::system_string_state::title, temp);

    sge.ext<sge::instrumentation::view>();
    push.time = sge.ext<sge::instrumentation::view>().timer();
    r.push_constants_changed = true;
}


//--------------------------------------------------------------------------------------------------------------------//
namespace sge::app { // HOOK UP TO SGE

void               initialise          ()                              { ::initialise (); }
configuration&     get_configuration   ()                              { return *::config; }
content&           get_content         ()                              { return *::computation; }
extensions&        get_extensions      ()                              { return *::extensions; }
void               start               (const api& sge)                {}
void               update              (response& r, const api& sge)   { ::update (r, sge); }
void               debug_ui            (response& r, const api& sge)   {}
void               stop                (const api& sge)                {}
void               terminate           ()                              { ::terminate (); }

}

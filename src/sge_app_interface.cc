#include "sge_app_interface.hh"

#include "sge_app.hh"

#include "sge_ext_overlay.hh"

#include "sge_ext_tools.hh"
#include "sge_ext_gizmo.hh"
#include "sge_ext_toy.hh"

namespace sge::app::internal {

std::unique_ptr<sge::app::api> user_api;
std::unique_ptr<extensions> standard_extensions;
    
api* create_user_api (sge::runtime::api& r) {
    user_api = std::make_unique<api>(r);
    return user_api.get();
}

void delete_user_api (sge::app::api* z) {
    assert (z == user_api.get());
    user_api.reset();
}

extensions& get_standard_extensions () {

    if (!standard_extensions.get()) {
        standard_extensions = std::make_unique<sge::app::extensions>();
        
        standard_extensions->views = {
            { sge::runtime::type_id<sge::ext::overlay>(), [] (const sge::runtime::api& x) { return new sge::ext::overlay (x); }},
            { sge::runtime::type_id<sge::ext::keyboard>(), [] (const sge::runtime::api& x) { return new sge::ext::keyboard (x); }},
            { sge::runtime::type_id<sge::ext::mouse>(), [] (const sge::runtime::api& x) { return new sge::ext::mouse (x); }},
            { sge::runtime::type_id<sge::ext::gamepad>(), [] (const sge::runtime::api& x) { return new sge::ext::gamepad (x); }},
            { sge::runtime::type_id<sge::ext::instrumentation>(), [] (const sge::runtime::api& x) { return new sge::ext::instrumentation (x); }},
            { sge::runtime::type_id<sge::ext::freecam>(), [] (const sge::runtime::api& x) { return new sge::ext::freecam (x); }},
            { sge::runtime::type_id<sge::ext::gizmo>(), [] (const sge::runtime::api& x) { return new sge::ext::gizmo (x); }},
            { sge::runtime::type_id<sge::ext::toy>(), [] (const sge::runtime::api& x) { return new sge::ext::toy (x); }},
        };
        standard_extensions->systems = {
            { sge::runtime::type_id<sge::ext::tools>(), [] (sge::runtime::api& x) { return new sge::ext::tools (x); }},
        };
    }

    return *standard_extensions.get ();
}

}

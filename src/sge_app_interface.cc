#include "sge_app_interface.hh"

#include "sge_app.hh"

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
        };
        standard_extensions->systems = {};
    }

    return *standard_extensions.get ();
}

}

#pragma once

#include "sge.hh"
#include "sge_runtime.hh"
#include "sge_app_interface.hh"
#include "sge_ext_overlay.hh"
#include "sge_ext_keyboard.hh"
#include "sge_ext_mouse.hh"
#include "sge_ext_gamepad.hh"
#include "sge_ext_instrumentation.hh"
#include "sge_ext_freecam.hh"


namespace sge::app {

struct api { // used by an SGE app to interact with SGE
    
    runtime::api& runtime;
    
    struct input_container {
        sge::ext::keyboard& keyboard;
        sge::ext::mouse& mouse;
        sge::ext::gamepad& gamepad;
    } input;

    sge::ext::instrumentation& instrumentation;
    sge::ext::freecam& freecam;

    api (runtime::api& z_api)
        : runtime (z_api)
        , input (input_container {
            ext <sge::ext::keyboard>(),
            ext <sge::ext::mouse>(),
            ext <sge::ext::gamepad>(),
        })
        , instrumentation (ext <sge::ext::instrumentation>())
        , freecam (ext <sge::ext::freecam>())
    {
    }
    
    template<typename T>
    T& ext () const {
        static_assert(std::is_base_of<runtime::extension, T>::value, "T must inherit from extension");
        size_t id = runtime::type_id <T>();
        runtime::extension* ext = runtime.extension_get (id);
        assert (ext);
        T* ext_t = static_cast <T*> (ext);
        assert (ext_t);
        return *ext_t;
    }
};


    
}

#pragma once

#include "sge.hh"

// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class instrumentation : public runtime::view {

public:
    instrumentation (const runtime::api& z) : runtime::view (z, "Instrumentation") {}

    uint32_t fps () const { return sge.timer__get_fps (); }
    float dt () const { return sge.timer__get_delta (); }
    float timer () const { return sge.timer__get_time (); }
    
    virtual void managed_debug_ui () override {
        ImGui::Text ("%d FPS", fps ());
    }
};

}

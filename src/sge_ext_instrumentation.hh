#pragma once

#include "sge.hh"

// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class instrumentation : public runtime::view {
    std::array<float, 512> fps_data;
public:
    instrumentation (const runtime::api& z) : runtime::view (z, "Instrumentation") {
        for (int i = 0; i < fps_data.size (); ++i) {
            fps_data[i] = 60;
        }
    }

    uint32_t fps () const { return sge.timer__get_fps (); }
    float dt () const { return sge.timer__get_delta (); }
    float timer () const { return sge.timer__get_time (); }
    
    virtual void update () override {
        for (int i = 1; i < fps_data.size (); ++i) {
            fps_data[i-1] = fps_data[i];
        }
        fps_data[fps_data.size () - 1] = 1.0f / sge.timer__get_delta ();
    }
    
    virtual void managed_debug_ui () override {
        char overlay[32];
        sprintf(overlay, "%d FPS", fps ());
        ImGui::PlotLines("", fps_data.data(), fps_data.size (), 0, overlay, 0.0f, 200.0f, ImVec2(fps_data.size (), 200));
    }
};

}

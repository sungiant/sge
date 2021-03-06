#pragma once

#include "sge.hh"

// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class instrumentation : public runtime::view {
    std::array<float, 512> fps_data;

    float max_fps = 60;
    float min_fps = 30;

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
        
        const float fps = 1.0f / sge.timer__get_delta ();
        fps_data[fps_data.size () - 1] = fps;
        
        min_fps = std::min (min_fps, fps);
        max_fps = std::max (max_fps, fps);
    }
    
    virtual void managed_debug_ui () override {
        char overlay[32];
        sprintf(overlay, "%d FPS", fps ());
        ImGui::PlotLines("", fps_data.data(), fps_data.size (), 0, overlay, min_fps, max_fps, ImVec2(fps_data.size (), 200));
    }
};

}

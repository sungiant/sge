#pragma once

#include "sge.hh"
#include "sge_runtime.hh"

namespace sge::ext {

class presentation : public runtime::system {
private:


public:
    presentation (runtime::api& z) : runtime::system (z, "Presentation") {}

    virtual void managed_debug_ui () override {

        ImGui::Text ("Canvas size: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::canvas_width),
            sge.system__get_state_int (sge::runtime::system_int_state::canvas_height));
    
        ImGui::Text ("Canvas offset: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::canvas_offset_x),
            sge.system__get_state_int (sge::runtime::system_int_state::canvas_offset_y));
        
        ImGui::Text ("Maximum canvas size: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::max_canvas_width),
            sge.system__get_state_int (sge::runtime::system_int_state::max_canvas_height));
        
        if (ImGui::Button ("Toggle Fullscreen")) {
            sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
        }

        if (ImGui::Button ("Set 800x600")) {
            sge.system__set_state_int (sge::runtime::system_int_state::canvas_width, 800);
            sge.system__set_state_int (sge::runtime::system_int_state::canvas_height, 600);
        }
    }
};

}

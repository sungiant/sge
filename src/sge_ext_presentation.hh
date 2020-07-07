#pragma once

#include "sge.hh"
#include "sge_runtime.hh"

namespace sge::ext {

class presentation : public runtime::system {
private:

public:
    presentation (runtime::api& z) : runtime::system (z, "Presentation") {}

    virtual void managed_debug_ui () override {

        ImGui::Text ("Compute target size: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::compute_width),
            sge.system__get_state_int (sge::runtime::system_int_state::compute_height));
    
        ImGui::Text ("Screen size: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::screen_width),
            sge.system__get_state_int (sge::runtime::system_int_state::screen_height));
        
        ImGui::Text ("Maximum resolution: %dx%d",
            sge.system__get_state_int (sge::runtime::system_int_state::display_width),
            sge.system__get_state_int (sge::runtime::system_int_state::display_height));
        
        if (ImGui::Button ("Toggle Fullscreen")) {
            sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
        }

        if (ImGui::Button ("Set 800x600")) {
            sge.system__set_state_int (sge::runtime::system_int_state::screen_width, 800);
            sge.system__set_state_int (sge::runtime::system_int_state::screen_height, 600);
        }
    }
};

}

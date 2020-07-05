#pragma once

#include "sge.hh"
#include "sge_runtime.hh"

namespace sge::ext {

class tools : public runtime::system {
private:

public:
    tools (runtime::api& z) : runtime::system (z) {}

    virtual void debug_ui () override {
        ImGui::Begin ("Tools");
        {
            if (ImGui::Button ("Toggle Fullscreen")) {
                sge.system__toggle_state_bool (sge::runtime::system_bool_state::fullscreen);
            }

            if (ImGui::Button ("Set 800x600")) {
                sge.system__set_state_int (sge::runtime::system_int_state::screenwidth, 800);
                sge.system__set_state_int (sge::runtime::system_int_state::screenheight, 600);
            }
        }
        ImGui::End ();
    }
};

}

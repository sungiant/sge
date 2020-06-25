#pragma once

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <utility>
#include <optional>
#include <algorithm>
#include <functional>
#include <cctype>
#include <imgui/imgui.h>

#include "sge.hh"


// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::instrumentation {

class view : public runtime::view {

public:
    view (const runtime::api& z) : runtime::view (z) {}

    uint32_t fps () const { return sge.timer__get_fps (); }
    float dt () const { return sge.timer__get_delta (); }
    float timer () const { return sge.timer__get_time (); }
};

}

#pragma once

#include "sge.hh"
#include "sge_math.hh"

namespace sge::data {

    struct vertex_pos_col { math::vector3 position; uint32_t colour; }; // ABGR

    static std::vector<vertex_pos_col> unit_cube = { // anti-clockwise winding
        // +X: red
        {{  1.0f,  1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f, -1.0f }, 0xFF0000FF},
        {{  1.0f,  1.0f, -1.0f }, 0xFF0000FF},
        {{  1.0f,  1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f,  1.0f }, 0xFF0000FF},
        {{  1.0f, -1.0f, -1.0f }, 0xFF0000FF},
        // +Y: green
        {{  1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{  1.0f,  1.0f,  1.0f }, 0xFF00FF00},
        {{  1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF00FF00},
        // +Z: blue
        {{  1.0f,  1.0f,  1.1f }, 0xFF6A0DAD},
        {{ -1.0f, -1.0f,  1.1f }, 0xFF6A0DAD},
        {{  1.0f, -1.0f,  1.1f }, 0xFF6A0DAD},
        
        {{  1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        {{  1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        {{  1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f,  1.0f,  1.0f }, 0xFFFF0000},
        {{ -1.0f, -1.0f,  1.0f }, 0xFFFF0000},
        // -X: dark red
        {{ -1.0f, -1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF000033},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF000033},
        {{ -1.0f,  1.0f,  1.0f }, 0xFF000033},
        // -Y: dark green
        {{ -1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f,  1.0f }, 0xFF003300},
        {{ -1.0f, -1.0f,  1.0f }, 0xFF003300},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f, -1.0f }, 0xFF003300},
        {{  1.0f, -1.0f,  1.0f }, 0xFF003300},
        // -Z: dark blue
        {{ -1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{  1.0f,  1.0f, -1.0f }, 0xFF330000},
        {{  1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{ -1.0f, -1.0f, -1.0f }, 0xFF330000},
        {{ -1.0f,  1.0f, -1.0f }, 0xFF330000},
        {{  1.0f,  1.0f, -1.0f }, 0xFF330000},
        
        
    };


}

#pragma once

// SGE-VK
// -------------------------------------
// Vulkan graphics backend.
// -------------------------------------

#include <memory>

#include "sge_vk_kernel.hpp"
#include "sge_vk_presentation.hpp"
#include "sge_vk_compute_target.hpp"
#include "sge_vk_fullscreen_render.hpp"
#include "sge_vk_imgui.hpp"

namespace sge::vk {
    struct vk {
        std::unique_ptr<kernel>             kernel;
        std::unique_ptr<presentation>       presentation;
        std::unique_ptr<compute_target>     compute_target;
        std::unique_ptr<fullscreen_render>  fullscreen_render;
        std::unique_ptr<imgui>              imgui;
        bool                                imgui_on = true;

#if TARGET_WIN32
        void create (HINSTANCE, HWND, int, int);
#elif TARGET_MACOSX
        void create (void*, int, int);
#endif
        
        void create_systems (std::vector<std::function<void ()>>&);
        void destroy ();
        void update (bool&, std::vector<bool>&, float);
    };

};

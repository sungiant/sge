// SGE-VK
// ---------------------------------- //
// Vulkan graphics backend.
// ---------------------------------- //

#pragma once

#if TARGET_WIN32
#include <windows.h>
#endif

#if TARGET_LINUX && !VARIANT_HEADLESS
#include <xcb/xcb.h>
#endif

#include <memory>

#include "sge_vk_kernel.hh"
#include "sge_vk_presentation.hh"
#include "sge_vk_compute_target.hh"
#include "sge_vk_fullscreen_render.hh"
#include "sge_vk_imgui.hh"

namespace sge::vk {
    struct vk {
        std::unique_ptr<kernel>             kernel;
        std::unique_ptr<presentation>       presentation;
        std::unique_ptr<compute_target>     compute_target;
        std::unique_ptr<fullscreen_render>  fullscreen_render;
        std::unique_ptr<imgui>              imgui;
        bool                                imgui_on = true;

#if TARGET_WIN32 && !VARIANT_HEADLESS
        void create (HINSTANCE, HWND, int, int);
#elif TARGET_MACOSX && !VARIANT_HEADLESS
        void create (void*, int, int);
#elif TARGET_LINUX && !VARIANT_HEADLESS
        void create (xcb_connection_t*, xcb_window_t, int, int);
#else
        void create (int, int);
#endif

        void create_systems (std::vector<std::function<void ()>>&);
        void destroy ();
        void update (bool&, std::vector<bool>&, std::vector<std::optional<dataspan>>&, float);
    };

};

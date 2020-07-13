// SGE-VK
// ---------------------------------- //
// Vulkan graphics backend.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_kernel.hh"
#include "sge_vk_presentation.hh"
#include "sge_vk_compute_target.hh"
#include "sge_vk_canvas_render.hh"
#include "sge_vk_imgui.hh"

namespace sge::vk {
    struct vk {
        std::unique_ptr<kernel>             kernel;
        std::unique_ptr<presentation>       presentation;
        std::unique_ptr<compute_target>     compute_target;
        std::unique_ptr<canvas_render>      canvas_render;
        std::unique_ptr<imgui>              imgui;

        struct {
            bool                                imgui_on = true;
            VkExtent2D                          compute_size;
            VkViewport                          canvas_viewport;
        } state;

#if TARGET_WIN32
        void create (HINSTANCE, HWND, int, int);
#elif TARGET_MACOSX
        void create (void*, int, int);
#elif TARGET_LINUX
        void create (xcb_connection_t*, xcb_window_t, int, int);
#else
        void create (int, int);
#endif

        void create_systems (const std::function <void()>&);
        void destroy ();
        void update (bool&, std::vector<bool>&, std::vector<std::optional<dataspan>>&, float);

        int get_user_viewport_x      () const { return state.canvas_viewport.x; }
        int get_user_viewport_y      () const { return state.canvas_viewport.y; }
        int get_user_viewport_width  () const { return state.canvas_viewport.width; }
        int get_user_viewport_height () const { return state.canvas_viewport.height; }

        void debug_ui ();

    private:

        VkSemaphore submit_all (image_index);
        VkExtent2D calculate_compute_size ();
        VkViewport calculate_canvas_viewport ();

    };

};

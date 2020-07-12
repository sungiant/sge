// SGE-VK-PRESENTATION
// ---------------------------------- //
// Vulkan presentation logic.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_types.hh"
#include "sge_vk_allocator.hh"

namespace sge::vk {

class kernel;

class presentation {

public:
    enum class surface_status : uint8_t { OK, ZERO, LOST, FATAL };
    enum class swapchain_status : uint8_t { OK, SUBOPTIMAL, OUT_OF_DATE, LOST, FATAL };

    presentation (const struct context&, const queue_identifier& qid
#if TARGET_WIN32
        , HINSTANCE, HWND
#elif TARGET_MACOSX
        , void*
#elif TARGET_LINUX
        , xcb_connection_t*, xcb_window_t
#endif
        );
    ~presentation () {};

    void                                configure                              (const std::vector<queue_identifier>&);

    void                                create                                 ();
    void                                create_r ();
    void                                destroy_r ();
    void                                destroy                                ();
    
    std::variant<swapchain_status, image_index> next_image                             ();
    surface_status                      check_surface_status                   ();

    size_t                              num_frame_buffers                      ()                   const { return state.swapchain_frame_buffers.size (); }
    const VkFramebuffer&                frame_buffer                           (image_index i)      const { return state.swapchain_frame_buffers[i]; }
    const VkSemaphore&                  image_available                        ()                   const { return state.image_available; }
    const VkRenderPass&                 canvas_render_pass                 ()                   const { return state.canvas_render_pass; }
    const VkRenderPass&                 imgui_render_pass                      ()                   const { return state.imgui_render_pass; }
    const VkExtent2D&                   extent                                 ()                   const { return state.swapchain_extent; }
    const VkSwapchainKHR&               swapchain                              ()                   const { return state.swapchain; }



    bool in_limbo () { return state.was_last_call_to_create_r_successful.has_value () && state.was_last_call_to_create_r_successful.value () == false; }
private:

    struct state {
        VkSurfaceKHR                    surface;
        VkSurfaceCapabilitiesKHR        surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR>   present_modes;
        VkSwapchainKHR                  swapchain;
        VkSurfaceFormatKHR              swapchain_surface_format;
        VkPresentModeKHR                swapchain_present_mode;
        VkExtent2D                      swapchain_extent;
        std::vector<VkImage>            swapchain_images;
        std::vector<VkImageView>        swapchain_image_views;
        VkRenderPass                    canvas_render_pass;
        VkRenderPass                    imgui_render_pass;
        std::vector<VkFramebuffer>      swapchain_frame_buffers;
        VkSemaphore                     image_available;
        VkImage                         depth_stencil_image;
        VkDeviceMemory                  depth_stencil_memory;
        VkImageView                     depth_stencil_view;
        std::vector<queue_family_index> queue_families_requiring_swapchain_access;

        std::optional<bool>             was_last_call_to_create_r_successful;
    };

    const context&                      context;
    const queue_identifier              identifier;
#if TARGET_WIN32
    const HINSTANCE                     app_hinst;
    const HWND                          app_hwnd;
#elif TARGET_MACOSX
    const void*                         app_view;
#elif TARGET_LINUX
    const xcb_connection_t*             app_connection;
    const xcb_window_t                  app_window;
#endif
    state                               state;




    void                                create_surface                         ();
    void                                create_swapchain                       ();
    void                                create_image_views                     ();
    void                                create_render_passes                   ();
    void                                create_framebuffers                    ();
    void                                create_depth_stencil                   ();
};

}


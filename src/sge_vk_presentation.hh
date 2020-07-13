// SGE-VK-PRESENTATION
// ---------------------------------- //
// Vulkan presentation logic.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_context.hh"
#include "sge_vk_allocator.hh"

namespace sge::vk {

class kernel;


class presentation {

public:

    enum class surface_status : uint8_t { OK, ZERO, LOST, FATAL };
    enum class swapchain_status : uint8_t { OK, SUBOPTIMAL, OUT_OF_DATE, LOST, FATAL };

    enum resource_bit : uint32_t {
        SEMAPHORE = (1 << 0),
        SURFACE = (1 << 1),
        SWAPCHAIN = (1 << 2),
        IMAGE_VIEWS = (1 << 3),
        DEPTH_STENCIL = (1 << 4),
        RENDER_PASSES = (1 << 5),
        FRAMEBUFFERS = (1 << 6),
    };

    typedef uint32_t resource_flags;

    static const resource_flags static_resources = resource_bit::SEMAPHORE | resource_bit::SURFACE;
    static const resource_flags transient_resources = resource_bit::SWAPCHAIN | resource_bit::IMAGE_VIEWS | resource_bit::DEPTH_STENCIL | resource_bit::RENDER_PASSES | resource_bit::FRAMEBUFFERS;
    static const resource_flags all_resources = static_resources | transient_resources;


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

    void                                configure                               (const std::vector<queue_identifier>&);

    void                                create_resources                        (resource_flags);
    void                                destroy_resources                       (resource_flags);

    std::variant<swapchain_status, image_index> next_image                      ();
    surface_status                      check_surface_status                    ();

    size_t                              num_frame_buffers                       ()                   const { return state.swapchain_frame_buffers.size (); }
    const VkFramebuffer&                frame_buffer                            (image_index i)      const { return state.swapchain_frame_buffers[i]; }
    const VkSemaphore&                  image_available                         ()                   const { return state.image_available; }
    const VkRenderPass&                 canvas_render_pass                      ()                   const { return state.canvas_render_pass; }
    const VkRenderPass&                 imgui_render_pass                       ()                   const { return state.imgui_render_pass; }
    const VkExtent2D&                   extent                                  ()                   const { return state.swapchain_extent; }
    const VkSwapchainKHR&               swapchain                               ()                   const { return state.swapchain; }

private:
    void                                create_semaphore                        ();
    void                                create_surface                          ();
    void                                create_swapchain                        ();
    void                                create_image_views                      ();
    void                                create_render_passes                    ();
    void                                create_framebuffers                     ();
    void                                create_depth_stencil                    ();

    void                                destroy_semaphore                       ();
    void                                destroy_surface                         ();
    void                                destroy_swapchain                       ();
    void                                destroy_image_views                     ();
    void                                destroy_render_passes                   ();
    void                                destroy_framebuffers                    ();
    void                                destroy_depth_stencil                   ();

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

        uint32_t                        resource_status;
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


};

}


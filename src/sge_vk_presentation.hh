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
        SYNCHRONISATION = (1 << 0),
        SURFACE = (1 << 1),
        SWAPCHAIN = (1 << 2),
        DEPTH_STENCIL = (1 << 3),
        RENDER_PASS = (1 << 4),
        FRAMEBUFFER = (1 << 5),
    };

    typedef uint32_t resource_flags;

    static const resource_flags static_resources = resource_bit::SYNCHRONISATION | resource_bit::SURFACE;
    static const resource_flags transient_resources = resource_bit::SWAPCHAIN | resource_bit::DEPTH_STENCIL | resource_bit::RENDER_PASS | resource_bit::FRAMEBUFFER;
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
    ~presentation ();

    void                                        configure                       (const std::vector<queue_identifier>&);

    void                                        create_resources                (resource_flags);
    void                                        destroy_resources               (resource_flags);

    std::variant<swapchain_status, image_index> next_image                      ();
    surface_status                              check_surface_status            ();

    size_t                                      num_frame_buffers               ()                  const { return state.frame_buffer.value.size (); }
    const VkFramebuffer&                        frame_buffer                    (image_index i)     const { return state.frame_buffer.value[i]; }
    const VkSemaphore&                          image_available                 ()                  const { return state.synchronisation.image_available; }
    const VkRenderPass&                         canvas_render_pass              ()                  const { return state.render_pass.canvas; }
    const VkRenderPass&                         imgui_render_pass               ()                  const { return state.render_pass.imgui; }
    const VkExtent2D&                           extent                          ()                  const { return state.swapchain.extent; }
    const VkSwapchainKHR&                       swapchain                       ()                  const { return state.swapchain.value; }

private:
    void                                        create_synchronisation          ();
    void                                        create_surface                  ();
    void                                        create_swapchain                ();
    void                                        create_render_pass              ();
    void                                        create_framebuffer              ();
    void                                        create_depth_stencil            ();

    void                                        destroy_synchronisation         ();
    void                                        destroy_surface                 ();
    void                                        destroy_swapchain               ();
    void                                        destroy_render_pass             ();
    void                                        destroy_framebuffer             ();
    void                                        destroy_depth_stencil           ();


    const context&                              context;
    const queue_identifier                      queue_id;
#if TARGET_WIN32
    const HINSTANCE                             app_hinst;
    const HWND                                  app_hwnd;
#elif TARGET_MACOSX
    const void*                                 app_view;
#elif TARGET_LINUX
    const xcb_connection_t*                     app_connection;
    const xcb_window_t                          app_window;
#endif

    struct {
        std::vector<queue_family_index>         queue_families_requiring_swapchain_access;
        uint32_t                                resource_status;
        struct {
            VkSemaphore                         image_available     = VK_NULL_HANDLE;
        }                                       synchronisation;
        struct {
            VkSurfaceKHR                        value               = VK_NULL_HANDLE;
            VkSurfaceCapabilitiesKHR            capabilities;
            std::vector<VkSurfaceFormatKHR>     formats;
            std::vector<VkPresentModeKHR>       present_modes;
        }                                       surface;
        struct {
            VkSwapchainKHR                      value               = VK_NULL_HANDLE;
            VkSurfaceFormatKHR                  surface_format;
            VkPresentModeKHR                    present_mode;
            VkExtent2D                          extent              = {0, 0};
            std::vector<VkImage>                images;
            std::vector<VkImageView>            image_views;
        }                                       swapchain;
        struct {
            VkImage                             image               = VK_NULL_HANDLE;
            VkDeviceMemory                      memory              = VK_NULL_HANDLE;
            VkImageView                         view                = VK_NULL_HANDLE;
        }                                       depth_stencil;
        struct {
            VkRenderPass                        canvas              = VK_NULL_HANDLE;
            VkRenderPass                        imgui               = VK_NULL_HANDLE;
        }                                       render_pass;
        struct {
            std::vector<VkFramebuffer>          value;
        }                                       frame_buffer;

    }                                           state;

};

}


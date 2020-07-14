// SGE-VK-CANVAS-RENDER
// ---------------------------------- //
// Pipeline for the user app canvas.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_buffer.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_context.hh"
#include "sge_vk_texture.hh"

namespace sge::vk {

class presentation;

class canvas_render {
public:

    typedef std::function<const VkDescriptorImageInfo&()> tex_fn;
    typedef std::function<VkViewport ()> viewport_fn;

    enum resource_bit : uint32_t {
        SYNCHRONISATION = (1 << 0),
        DESCRIPTOR_SET_LAYOUT = (1 << 1),
        COMMAND_POOL = (1 << 2),
        DESCRIPTOR_POOL = (1 << 3),
        DESCRIPTOR_SET = (1 << 4),
        PIPELINE = (1 << 5),
        COMMAND_BUFFER = (1 << 6),
    };

    typedef uint32_t resource_flags;

    static const resource_flags static_resources = resource_bit::SYNCHRONISATION | resource_bit::DESCRIPTOR_SET_LAYOUT | resource_bit::COMMAND_POOL;
    static const resource_flags transient_resources = resource_bit::DESCRIPTOR_POOL | resource_bit::DESCRIPTOR_SET | resource_bit::PIPELINE | resource_bit::COMMAND_BUFFER;
    static const resource_flags all_resources = static_resources | transient_resources;

    canvas_render (
        const context&,
        const queue_identifier,
        const class presentation&,
        const tex_fn&, // the texture to render
        const viewport_fn&);

    ~canvas_render () {}

    const VkQueue                       get_queue                               ()                const { return context.get_queue (identifier); };
    const VkCommandBuffer               get_command_buffer                      (image_index i)   const { return state.command_buffers[i]; }
    const VkSemaphore                   get_render_finished                     ()                const { return state.render_finished; }

    void                                create_resources                        (resource_flags);
    void                                destroy_resources                       (resource_flags);

    //void                                refresh_command_buffers                 ();

private:
    void                                create_synchronisation                  ();
    void                                create_descriptor_set_layout            ();
    void                                create_command_pool                     ();
    void                                create_descriptor_pool                  ();
    void                                create_descriptor_set                   ();
    void                                create_pipeline                         ();
    void                                create_command_buffer                   ();

    void                                destroy_synchronisation                 ();
    void                                destroy_descriptor_set_layout           ();
    void                                destroy_command_pool                    ();
    void                                destroy_descriptor_pool                 ();
    void                                destroy_descriptor_set                  ();
    void                                destroy_pipeline                        ();
    void                                destroy_command_buffer                  ();

    struct state {
        VkViewport                      current_viewport                        = {};
        uint32_t                        resource_status                         = 0;

        VkSemaphore                     render_finished                         = VK_NULL_HANDLE;

        VkDescriptorSetLayout           descriptor_set_layout                   = VK_NULL_HANDLE;

        VkCommandPool                   command_pool                            = VK_NULL_HANDLE;

        VkDescriptorPool                descriptor_pool                         = VK_NULL_HANDLE;

        VkDescriptorSet                 descriptor_set                          = VK_NULL_HANDLE;

        VkPipelineLayout                pipeline_layout                         = VK_NULL_HANDLE;
        VkPipeline                      pipeline                                = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer>    command_buffers;
    };

    const context&                      context;
    const queue_identifier              identifier;
    const presentation&                 presentation;
    const tex_fn                        compute_tex;
    const viewport_fn                   get_viewport_fn;
    state                               state;



};

}

// SGE-VK-CANVAS-RENDER
// ---------------------------------- //
// Pipeline for the user app canvas.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_types.hh"
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

    canvas_render (
        const context&,
        const queue_identifier,
        const class presentation&,
        const tex_fn&, // the texture to render
        const viewport_fn&);

    ~canvas_render () {}

    const VkQueue                       get_queue                               ()                  const { return context.get_queue (identifier); };
    const VkCommandBuffer               get_command_buffer                      (image_index i)     const { return state.command_buffers[i]; }
    const VkSemaphore                   get_render_finished                     ()                  const { return state.render_finished; }

    void                                create                                  ();
    void                                destroy                                 ();
    void                                refresh_full                            ();

    void                                refresh_command_buffers                 ();

private:

    struct state {
        VkDescriptorSetLayout           descriptor_set_layout;
        VkDescriptorPool                descriptor_pool;
        VkDescriptorSet                 descriptor_set;
        VkPipelineLayout                pipeline_layout;
        VkPipeline                      pipeline;
        VkCommandPool                   command_pool;
        std::vector<VkCommandBuffer>    command_buffers;
        VkSemaphore                     render_finished;
        VkViewport                      current_viewport;
    };

    const context&                      context;
    const queue_identifier              identifier;
    const presentation&                 presentation;
    const tex_fn                        compute_tex;
    const viewport_fn                   get_viewport_fn;
    state                               state;


private:

    void                                create_r                                ();
    void                                destroy_r                               ();
    void                                create_descriptor_set_layout            ();
    void                                create_command_pool                     ();
    void                                create_descriptor_pool                  ();
    void                                create_descriptor_set                   ();
    void                                create_pipeline                         ();
    void                                create_command_buffers                  ();

};

}

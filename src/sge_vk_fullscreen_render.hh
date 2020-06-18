#pragma once

// SGE-VK-FULLSCREEN-RENDER
// -------------------------------------
// Noddy pipeline for quad rendering.
// -------------------------------------

#include <vector>

#include <vulkan/vulkan.h>
#include <functional>

#include "sge_math.hh"
#include "sge_vk_types.hh"
#include "sge_vk_buffer.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_context.hh"
#include "sge_vk_texture.hh"

namespace sge::vk {

class presentation;

class fullscreen_render {
public:

    typedef std::function<const VkDescriptorImageInfo&()> tex_fn;

    fullscreen_render (
        const context&,
        const queue_identifier,
        const class presentation&,
        const tex_fn); // the texture to render
    ~fullscreen_render () {}

    const VkQueue                       get_queue                               ()                  const { return context.get_queue (identifier); };
    const VkCommandBuffer               get_command_buffer                      (image_index i)     const { return state.command_buffers[i]; }
    const VkSemaphore                   get_render_finished                     ()                  const { return state.render_finished; }

    void                                create                                  ();
    void                                destroy                                 ();
    void                                refresh                                 ();

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
    };

    const context&                      context;
    const queue_identifier              identifier;
    const presentation&                 presentation;
    const tex_fn                        compute_tex;
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

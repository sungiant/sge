// SGE-VK-FULLSCREEN-RENDER
// ---------------------------------- //
// Noddy pipeline for quad rendering.
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
    void                                refresh_full                            ();
    
    void                                refresh_command_buffers                 ();
    
    bool                                need_command_buffers_refresh            () {
        update_target_viewport ();
        return state.target_viewport.has_value();
        
    }
    
    void set_custom_viewport_fn (const std::function<VkViewport(const class presentation&)>& z_fn) { state.custom_viewport_fn = z_fn; update_target_viewport (); }
    void has_custom_viewport_fn () { state.custom_viewport_fn.has_value (); }
    void clear_custom_viewport_fn () { state.custom_viewport_fn = std::nullopt; update_target_viewport (); }
    
private:
    void update_target_viewport () {
        const auto updated = state.custom_viewport_fn.has_value()
            ? state.custom_viewport_fn.value() (presentation)
            : default_viewport_fn (presentation);
        
        if (!utils::equal (updated, state.current_viewport))
            state.target_viewport = updated;
        else state.target_viewport.reset ();
    }
    
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
        std::optional<VkViewport>       target_viewport;
        std::optional<std::function<VkViewport(const class presentation&)>> custom_viewport_fn;
    };

    const context&                      context;
    const queue_identifier              identifier;
    const presentation&                 presentation;
    const tex_fn                        compute_tex;
    state                               state;
    const std::function<VkViewport(const class presentation&)> default_viewport_fn;


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

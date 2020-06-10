#pragma once

// SGE-VK-COMPUTE-TARGET
// -------------------------------------
// Pipeline for processing user code.
// -------------------------------------

#include <vector>
#include <functional>

#include <vulkan/vulkan.h>

#include "sge_utils.hpp"
#include "sge_math.hpp"
#include "sge_app.hpp"

#include "sge_vk_types.hpp"
#include "sge_vk_buffer.hpp"
#include "sge_vk_utils.hpp"
#include "sge_vk_context.hpp"
#include "sge_vk_texture.hpp"

namespace sge::vk {

class presentation;

class compute_target {
public:

    compute_target (const struct context&, const struct queue_identifier&, const class presentation&, const struct sge::app::content&);
    ~compute_target () {};
    
    void                                create                                  ();
    void                                destroy                                 ();
    void                                recreate                                ();
    void                                refresh                                 ();
    void                                enqueue                                 ();
    void                                update                                  (bool&, std::vector<bool>&);

    void                                append_pre_render_submissions           (std::vector<VkSemaphore>&, std::vector<VkPipelineStageFlags>&);

    const texture&                      get_pre_render_texture                  () const { return state.compute_tex; }

private:

    struct state {
        texture                         compute_tex;
        VkDescriptorPool                descriptor_pool;
        VkDescriptorSet                 descriptor_set;
        VkDescriptorSetLayout           descriptor_set_layout;
        VkPipeline                      pipeline;
        VkPipelineLayout                pipeline_layout;
        VkShaderModule                  compute_shader_module;
        VkCommandPool                   command_pool;
        VkCommandBuffer                 command_buffer;
        VkSemaphore                     compute_complete;
        VkFence                         fence;
        std::vector<device_buffer>      uniform_buffers;
    };

    const context&                      context;
    const queue_identifier              identifier;
    const presentation&                 presentation;
    const sge::app::content&            content;
    state                               state;


    void                                create_r                                ();
    void                                create_rl                               ();
    void                                destroy_rl                              ();
    void                                destroy_r                               ();
    void                                create_buffer                           ();
    void                                create_descriptor_set_layout            ();
    void                                create_descriptor_set                   ();
    void                                create_compute_pipeline                 ();
    void                                destroy_compute_pipeline                ();
    void                                create_command_buffer                   ();
    void                                destroy_command_buffer                  ();
    void                                record_command_buffer                   ();
    void                                run_command_buffer                      ();
    void                                prepare_texture_target                  (VkFormat);
    void                                prepare_uniform_buffers                 ();
    void                                update_uniform_buffer                   (int);


};

}

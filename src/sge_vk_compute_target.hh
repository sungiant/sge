// SGE-VK-COMPUTE-TARGET
// ---------------------------------- //
// Pipeline for processing user code.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_app_interface.hh"
#include "sge_vk_types.hh"
#include "sge_vk_buffer.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_context.hh"
#include "sge_vk_texture.hh"

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
    void                                update                                  (bool&, std::vector<bool>&, std::vector<std::optional<dataspan>>&);

    void                                append_pre_render_submissions           (std::vector<VkSemaphore>&, std::vector<VkPipelineStageFlags>&);

    const texture&                      get_pre_render_texture                  () const { return state.compute_tex; }
    void                                end_of_frame ();
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
        std::vector<device_buffer>      blob_staging_buffers;
        std::vector<device_buffer>      blob_storage_buffers;
        std::vector<dataspan>           latest_blob_infos; // keep track of sizes needed for user storage blobs as these can change at runtime.

        std::vector<std::optional<dataspan>> pending_blob_changes;
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
    void                                destroy_texture_target                  ();

    void                                prepare_uniform_buffers                 ();
    void                                update_uniform_buffer                   (int);
    void                                destroy_uniform_buffers                 ();

    void                                prepare_blob_buffers                    ();
    void                                prepare_blob_buffer                     (int, dataspan);
    void                                copy_blob_from_staging_to_storage       (int);
    void                                update_blob_buffer                      (int, dataspan);
    void                                destroy_blob_buffer                     (int);
    void                                destroy_blob_buffers                    ();


};

}

// SGE-VK-IMGUI
// ---------------------------------- //
// ImGui rendering pipeline.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_vk_buffer.hh"
#include "sge_vk_context.hh"
#include "sge_vk_types.hh"

namespace sge::vk {

class presentation;

class imgui {
public:

    typedef std::vector<std::function<void ()>> debug_fns;

    imgui (const struct context&, const struct queue_identifier, const class presentation&, const std::function <void()>&);
    ~imgui ();

    void                                refresh                     ();
    void                                create                      ();
    void                                destroy                     ();
    void                                enqueue                     (image_index);

    const VkQueue                       get_queue                   ()                              const { return context.get_queue (queue_identifier); };
    const VkCommandBuffer               get_command_buffer          (image_index i)                 const { return state.command_buffers[i]; }
    const VkSemaphore                   get_render_finished         ()                              const { return state.render_finished; }

private:

    struct push {
        sge::math::vector2              scale;
        sge::math::vector2              translation;
    };

    struct state {
        push                            push;
        std::vector<VkShaderModule>     shaders;
        VkSemaphore                     render_finished;
        VkCommandPool                   command_pool;
        std::vector<VkCommandBuffer>    command_buffers;
        VkSampler                       sampler;
        device_buffer                   vertex_buffer;
        device_buffer                   index_buffer;
        int32_t                         vertex_count = 0;
        int32_t                         index_count = 0;
        VkDeviceMemory                  font_memory = VK_NULL_HANDLE;
        VkImage                         font_image = VK_NULL_HANDLE;
        VkImageView                     font_view = VK_NULL_HANDLE;
        VkPipelineCache                 pipeline_cache;
        VkPipelineLayout                pipeline_layout;
        VkPipeline                      pipeline;
        VkDescriptorPool                descriptor_pool;
        VkDescriptorSetLayout           descriptor_set_layout;
        VkDescriptorSet                 descriptor_set;
    };

    const context&                      context;
    const queue_identifier              queue_identifier;
    const presentation&                 presentation;
    const std::function <void()>        imgui_fn;
    state                               state;

    void                                init                        (float, float);
    void                                release                     ();
    VkPipelineShaderStageCreateInfo     load_shader                 (std::string, VkShaderStageFlagBits);
    void                                init_resources              (VkRenderPass, VkQueue);
    void                                update_buffers              ();
    void                                draw_frame                  (VkCommandBuffer);
    void                                record_command_buffer       (int32_t);
    void                                create_command_pool         ();
    void                                create_command_buffers      ();
};


}

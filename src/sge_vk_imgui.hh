// SGE-VK-IMGUI
// ---------------------------------- //
// ImGui rendering pipeline.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_math.hh"
#include "sge_vk_buffer.hh"
#include "sge_vk_context.hh"

namespace sge::vk {

class presentation;

class imgui {
public:

    enum resource_bit : uint32_t {
        SYNCHRONISATION = (1 << 0),
        COMMAND_POOL = (1 << 1),
        SHADER = (1 << 2),
        SAMPLER = (1 << 3),
        FONT_TEXTURE = (1 << 4),
        FONT_DESCRIPTOR = (1 << 5),
        PIPELINE = (1 << 6),
        COMMAND_BUFFER = (1 << 7),
        VERTEX_BUFFER = (1 << 8),
        INDEX_BUFFER = (1 << 9),
    };

    typedef uint32_t resource_flags;

    static const resource_flags static_resources = resource_bit::SYNCHRONISATION | resource_bit::COMMAND_POOL | resource_bit::SHADER | resource_bit::SAMPLER | resource_bit::FONT_TEXTURE | resource_bit::FONT_DESCRIPTOR | resource_bit::COMMAND_BUFFER |  resource_bit::PIPELINE;
    static const resource_flags transient_resources = resource_bit::VERTEX_BUFFER | resource_bit::INDEX_BUFFER;
    static const resource_flags all_resources = static_resources | transient_resources;

    typedef std::vector<std::function<void ()>> debug_fns;

    imgui (const struct context&, const struct queue_identifier, const class presentation&, const std::function <void()>&);
    ~imgui ();


    void                                    create_resources            (resource_flags);
    void                                    destroy_resources           (resource_flags);
    void                                    record                      (image_index);

    const VkQueue                           get_queue                   ()                const { return context.get_queue (identifier); };
    const VkCommandBuffer                   get_command_buffer          (image_index i)   const { return state.command_buffers[i]; }
    const VkSemaphore                       get_render_finished         ()                const { return state.synchronisation.render_finished; }

    void                                    debug_ui                    ();

private:

    void                                    create_synchronisation ();
    void                                    create_shader ();
    void                                    create_sampler ();
    void                                    create_font_texture ();
    void                                    create_font_descriptor ();
    void                                    create_pipeline ();
    void                                    create_command_pool ();
    void                                    create_command_buffer ();
    void                                    create_vertex_buffer ();
    void                                    create_index_buffer ();

    void                                    destroy_synchronisation ();
    void                                    destroy_shader ();
    void                                    destroy_sampler ();
    void                                    destroy_font_texture ();
    void                                    destroy_font_descriptor ();
    void                                    destroy_pipeline ();
    void                                    destroy_command_pool ();
    void                                    destroy_command_buffer ();
    void                                    destroy_vertex_buffer ();
    void                                    destroy_index_buffer ();

    const context&                          context;
    const queue_identifier                  identifier;
    const presentation&                     presentation;
    const std::function <void ()>           imgui_fn;

    struct push {
        sge::math::vector2                  scale                   = { 0, 0 };
        sge::math::vector2                  translation             = { 0, 0 };
    };

    struct {
        push                                push;
        uint32_t                            resource_status         = 0;
        struct {
            VkSemaphore                     render_finished         = VK_NULL_HANDLE;
        }                                   synchronisation;

        VkCommandPool                       command_pool;
        struct {
            VkShaderModule                  vertex                  = VK_NULL_HANDLE;
            VkShaderModule                  fragment                = VK_NULL_HANDLE;
        }                                   shader;

        VkSampler                           sampler                 = VK_NULL_HANDLE;
        struct {
            VkPipelineCache                 cache                   = VK_NULL_HANDLE;
            VkPipelineLayout                layout                  = VK_NULL_HANDLE;
            VkPipeline                      value                   = VK_NULL_HANDLE;
        }                                   pipeline;
        struct {
            VkDeviceMemory                  memory                  = VK_NULL_HANDLE;
            VkImage                         image                   = VK_NULL_HANDLE;
            VkImageView                     view                    = VK_NULL_HANDLE;
        }                                   font;
        struct {
            VkDescriptorPool                pool                    = VK_NULL_HANDLE;
            VkDescriptorSetLayout           set_layout              = VK_NULL_HANDLE;
            VkDescriptorSet                 set                     = VK_NULL_HANDLE;
        } font_descriptor;

        std::vector<VkCommandBuffer>        command_buffers;
        struct {
            device_buffer                   value;
            int32_t                         count                   = 0;
            int32_t                         create_count            = 0;
        }                                   vertex_buffer;
        struct {
            device_buffer                   value;
            int32_t                         count                   = 0;
            int32_t                         create_count            = 0;
        }                                   index_buffer;

    }                                       state;


};


}

#include "sge_vk_fullscreen_render.hh"

#include "sge_vk_presentation.hh"
#include "sge_utils.hh"

namespace sge::vk {

fullscreen_render::fullscreen_render (const struct context& context, const queue_identifier qid, const class presentation& p, const std::function<const VkDescriptorImageInfo&()> tex)
    : context (context)
    , identifier (qid)
    , presentation (p)
    , compute_tex (tex)
    , default_viewport_fn ([](const class presentation& p) { return utils::init_VkViewport ((float) p.extent ().width, (float) p.extent ().height, 0.0f, 1.0f); })
{
    state.current_viewport = default_viewport_fn (presentation);
}

void fullscreen_render::create () {
    const auto semaphore_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_info, context.allocation_callbacks, &state.render_finished));;

    create_descriptor_set_layout ();
    create_command_pool ();
    create_r ();
}


void fullscreen_render::destroy () {
    destroy_r ();

    vkDestroyCommandPool (context.logical_device, state.command_pool, context.allocation_callbacks);
    state.command_pool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout (context.logical_device, state.descriptor_set_layout, context.allocation_callbacks);
    state.descriptor_set_layout = VK_NULL_HANDLE;

    vkDestroySemaphore (context.logical_device, state.render_finished, context.allocation_callbacks);
    state.render_finished = VK_NULL_HANDLE;
}

void fullscreen_render::create_r () {
    create_pipeline ();
    create_descriptor_pool ();
    create_descriptor_set ();
    create_command_buffers ();
}

void fullscreen_render::destroy_r () {

    vkFreeCommandBuffers (context.logical_device, state.command_pool, static_cast<uint32_t>(state.command_buffers.size ()), state.command_buffers.data ());
    vkFreeDescriptorSets (context.logical_device, state.descriptor_pool, 1, &state.descriptor_set);

    vkDestroyDescriptorPool (context.logical_device, state.descriptor_pool, context.allocation_callbacks);
    state.descriptor_pool = VK_NULL_HANDLE;

    vkDestroyPipeline (context.logical_device, state.pipeline, context.allocation_callbacks);
    state.pipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout (context.logical_device, state.pipeline_layout, context.allocation_callbacks);
    state.pipeline_layout = VK_NULL_HANDLE;
}

void fullscreen_render::refresh_full () {
    destroy_r ();
    
    update_target_viewport ();
    if (state.target_viewport.has_value()) {
        state.current_viewport = state.target_viewport.value();
        state.target_viewport.reset();
    }
    
    create_r ();
}
    
void fullscreen_render::refresh_command_buffers () {
    vkFreeCommandBuffers (context.logical_device, state.command_pool, static_cast<uint32_t>(state.command_buffers.size ()), state.command_buffers.data ());
    
    update_target_viewport ();
    if (state.target_viewport.has_value()) {
        state.current_viewport = state.target_viewport.value();
        state.target_viewport.reset();
    }
    
    create_command_buffers ();
}

void fullscreen_render::create_descriptor_set_layout () {
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings = {
        utils::init_VkDescriptorSetLayoutBinding (
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0)
    };

    const auto descriptor_layout = utils::init_VkDescriptorSetLayoutCreateInfo (descriptor_set_layout_bindings);
    vk_assert (vkCreateDescriptorSetLayout (context.logical_device, &descriptor_layout, context.allocation_callbacks, &state.descriptor_set_layout));
}

void fullscreen_render::create_command_pool () {
    const auto pool_info = utils::init_VkCommandPoolCreateInfo (identifier.family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk_assert (vkCreateCommandPool (context.logical_device, &pool_info, context.allocation_callbacks, &state.command_pool));
}

void fullscreen_render::create_descriptor_pool () {

    std::vector<VkDescriptorPoolSize> pool_sizes = {
        utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
        utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info =
        utils::init_VkDescriptorPoolCreateInfo (
            pool_sizes,
            2);
    descriptor_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    vk_assert (vkCreateDescriptorPool (
        context.logical_device,
        &descriptor_pool_create_info,
        context.allocation_callbacks,
        &state.descriptor_pool));

}

void fullscreen_render::create_descriptor_set () {

    VkDescriptorSetLayout layouts[] = { state.descriptor_set_layout };
    const auto allocate_info = utils::init_VkDescriptorSetAllocateInfo (state.descriptor_pool, layouts, 1);
    vk_assert (vkAllocateDescriptorSets (context.logical_device, &allocate_info, &state.descriptor_set));

    VkDescriptorImageInfo ii = compute_tex ();

    std::vector<VkWriteDescriptorSet> write_descriptor_sets =
    {
        utils::init_VkWriteDescriptorSet (
            state.descriptor_set,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            0,
            &ii,
            1)
    };

    vkUpdateDescriptorSets (context.logical_device, (uint32_t) write_descriptor_sets.size (), write_descriptor_sets.data (), 0, NULL);
}

void fullscreen_render::create_pipeline () {

    std::vector<uint8_t> vert;
    std::vector<uint8_t> frag;
    sge::utils::get_file_stream (vert, "sge_fullscreen_render.vert.spv");
    sge::utils::get_file_stream (frag, "sge_fullscreen_render.frag.spv");
    VkShaderModule vertex_shader       = utils::create_shader_module (context.logical_device, context.allocation_callbacks, vert);
    VkShaderModule fragment_shader     = utils::create_shader_module (context.logical_device, context.allocation_callbacks, frag);

    const auto vertex_shader_stage_info      = utils::init_VkPipelineShaderStageCreateInfo (VK_SHADER_STAGE_VERTEX_BIT, vertex_shader, "main");
    const auto fragment_shader_stage_info    = utils::init_VkPipelineShaderStageCreateInfo (VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader, "main");

    VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_info, fragment_shader_stage_info };

    const auto empty_input_state             = utils::init_VkPipelineVertexInputStateCreateInfo ();
    const auto input_assembly                = utils::init_VkPipelineInputAssemblyStateCreateInfo (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    const auto scissor                       = utils::init_VkRect2D (presentation.extent ());
    const auto viewport_state                = utils::init_VkPipelineViewportStateCreateInfo (VkViewport(), scissor); // viewport is ignored here as we set it in the command buffer
    const auto rasterizer                    = utils::init_VkPipelineRasterizationStateCreateInfo (VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    const auto multisampling                 = utils::init_VkPipelineMultisampleStateCreateInfo (VK_SAMPLE_COUNT_1_BIT);

    const auto colour_blend_attachment = utils::init_VkPipelineColorBlendAttachmentState ();
    auto colour_blending = utils::init_VkPipelineColorBlendStateCreateInfo (colour_blend_attachment);
    colour_blending.logicOpEnable = VK_FALSE;
    colour_blending.logicOp = VK_LOGIC_OP_COPY;
    colour_blending.blendConstants[0] = 0.0f;
    colour_blending.blendConstants[1] = 0.0f;
    colour_blending.blendConstants[2] = 0.0f;
    colour_blending.blendConstants[3] = 0.0f;

    const auto pipeline_layout_info = utils::init_VkPipelineLayoutCreateInfo(1, &state.descriptor_set_layout);

    vk_assert (vkCreatePipelineLayout (context.logical_device, &pipeline_layout_info, context.allocation_callbacks, &state.pipeline_layout));
     
    const std::vector<VkDynamicState> dynamic_states_to_enable = { VK_DYNAMIC_STATE_VIEWPORT };
    const auto dynamic_state = utils::init_VkPipelineDynamicStateCreateInfo (dynamic_states_to_enable);
     
    const auto depth_stencil_state = utils::init_VkPipelineDepthStencilStateCreateInfo (VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    auto pipeline_info = utils::init_VkGraphicsPipelineCreateInfo (state.pipeline_layout, presentation.fullscreen_render_pass ());
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &empty_input_state;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil_state;
    pipeline_info.pColorBlendState = &colour_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    vk_assert (vkCreateGraphicsPipelines (context.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, context.allocation_callbacks, &state.pipeline));

    vkDestroyShaderModule (context.logical_device, fragment_shader, context.allocation_callbacks);
    vkDestroyShaderModule (context.logical_device, vertex_shader, context.allocation_callbacks);
}


void fullscreen_render::create_command_buffers () {

    state.command_buffers.resize (presentation.num_frame_buffers ());

    const auto allocate_info = utils::init_VkCommandBufferAllocateInfo (state.command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, (uint32_t) state.command_buffers.size ());

    vk_assert (vkAllocateCommandBuffers (context.logical_device, &allocate_info, state.command_buffers.data ()));

    auto render_pass_begin_info = utils::init_VkRenderPassBeginInfo ();
    render_pass_begin_info.renderPass = presentation.fullscreen_render_pass ();
    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = presentation.extent ();

    for (int i = 0; i < state.command_buffers.size (); i++) {
        const auto begin_info = utils::init_VkCommandBufferBeginInfo (VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        vkBeginCommandBuffer (state.command_buffers[i], &begin_info);

        render_pass_begin_info.framebuffer = presentation.frame_buffer ((image_index) i);

        std::array<VkClearValue, 2> clear_values;
        clear_values[0].color = { { 1.0f, 0.584f, 0.929f, 1.0f } };
        clear_values[1].depthStencil = { 1.0f, 0 };

        render_pass_begin_info.clearValueCount = (uint32_t) clear_values.size ();
        render_pass_begin_info.pClearValues = clear_values.data ();
         
        vkCmdSetViewport(state.command_buffers[i], 0, 1, &state.current_viewport);
        vkCmdBeginRenderPass (state.command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline (state.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);
        vkCmdBindDescriptorSets (state.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline_layout, 0, 1, &state.descriptor_set, 0, NULL);
         
        vkCmdDraw (state.command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass (state.command_buffers[i]);

        vk_assert (vkEndCommandBuffer (state.command_buffers[i]));
    }
}

}

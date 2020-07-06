#include "sge_vk_imgui.hh"

#include "sge_utils.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_presentation.hh"

namespace sge::vk {


imgui::imgui (const struct context& z_context, const struct queue_identifier z_queue_identifier, const class presentation& z_presentation, const std::function <void()>& z_imgui_fn)
    : context (z_context)
    , queue_identifier (z_queue_identifier)
    , presentation (z_presentation)
    , imgui_fn (z_imgui_fn)
{
    ImGui::CreateContext ();
};

imgui::~imgui () {
    ImGui::DestroyContext ();
}

void imgui::create () {
    auto semaphore_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_info, context.allocation_callbacks, &state.render_finished));
    create_command_pool ();
    create_command_buffers ();
    init ((float)presentation.extent ().width, (float)presentation.extent ().height);
    init_resources (presentation.imgui_render_pass (), get_queue ());
}

void imgui::destroy () {
    release ();
    vkFreeCommandBuffers (context.logical_device, state.command_pool, static_cast<uint32_t>(state.command_buffers.size ()), state.command_buffers.data ());
    vkDestroyCommandPool (context.logical_device, state.command_pool, context.allocation_callbacks);
    vkDestroySemaphore (context.logical_device, state.render_finished, context.allocation_callbacks);
    state.render_finished = VK_NULL_HANDLE;
}

void imgui::refresh () {
    ImGuiIO& io = ImGui::GetIO ();
    io.DisplaySize = ImVec2 ((float)presentation.extent ().width, (float)presentation.extent ().height);
}

void imgui::enqueue (image_index i) {
    record_command_buffer (i);
}


void imgui::create_command_pool () {
    auto pool_info = utils::init_VkCommandPoolCreateInfo (queue_identifier.family_index);
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_assert (vkCreateCommandPool (context.logical_device, &pool_info, context.allocation_callbacks, &state.command_pool));
}

void imgui::create_command_buffers () {
    // Create one command buffer for each swap chain image and reuse for rendering
    state.command_buffers.resize (presentation.num_frame_buffers ());

    auto cmdBufAllocateInfo =
        utils::init_VkCommandBufferAllocateInfo (
            state.command_pool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(state.command_buffers.size ()));

    vk_assert (vkAllocateCommandBuffers (context.logical_device, &cmdBufAllocateInfo, state.command_buffers.data ()));
}


void imgui::record_command_buffer (int32_t i) {
    auto buffer_info = utils::init_VkCommandBufferBeginInfo ();

    VkClearValue clear_values[2];
    clear_values[0].color = { { 1.0f, 0.2f, 0.2f, 1.0f} };
    clear_values[1].depthStencil = { 1.0f, 0 };

    auto render_pass_info = utils::init_VkRenderPassBeginInfo ();
    render_pass_info.renderPass = presentation.imgui_render_pass ();
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = presentation.extent ();
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values;
    
    ImGui::NewFrame ();
    imgui_fn ();
    ImGui::Render ();
    
    update_buffers ();

    render_pass_info.framebuffer = presentation.frame_buffer (i);
    vk_assert (vkBeginCommandBuffer (state.command_buffers[i], &buffer_info));

    vkCmdBeginRenderPass (state.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    auto viewport = utils::init_VkViewport ((float)presentation.extent ().width, (float)presentation.extent ().height, 0.0f, 1.0f);
    vkCmdSetViewport (state.command_buffers[i], 0, 1, &viewport);

    auto scissor = utils::init_VkRect2D (presentation.extent ().width, presentation.extent ().height);
    vkCmdSetScissor (state.command_buffers[i], 0, 1, &scissor);

    draw_frame (state.command_buffers[i]);
    vkCmdEndRenderPass (state.command_buffers[i]);
    vk_assert (vkEndCommandBuffer (state.command_buffers[i]));

}

void imgui::init (float width, float height) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.GrabRounding = 4.0f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.98f, 0.52f, 0.26f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.98f, 0.52f, 0.26f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.88f, 0.60f, 0.44f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.93f, 0.47f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.98f, 0.52f, 0.26f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.98f, 0.52f, 0.26f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.80f, 0.58f, 0.46f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.98f, 0.52f, 0.26f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.98f, 0.52f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.98f, 0.40f, 0.06f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.98f, 0.52f, 0.26f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.98f, 0.52f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.98f, 0.52f, 0.26f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.80f, 0.38f, 0.14f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.80f, 0.38f, 0.14f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.98f, 0.52f, 0.26f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.98f, 0.52f, 0.26f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.84f, 0.79f, 0.76f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.98f, 0.52f, 0.26f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.88f, 0.70f, 0.60f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.94f, 0.93f, 0.92f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.91f, 0.80f, 0.74f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.35f, 0.86f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.00f, 0.38f, 0.90f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.97f, 0.01f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.98f, 0.52f, 0.26f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.98f, 0.52f, 0.26f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.98f, 0.52f, 0.26f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void imgui::release () {
    for (auto& shaderModule : state.shaders) {
        vkDestroyShaderModule (context.logical_device, shaderModule, context.allocation_callbacks);
    }

    state.vertex_buffer.destroy (context.allocation_callbacks);
    state.index_buffer.destroy (context.allocation_callbacks);
    vkDestroyImage (context.logical_device, state.font_image, context.allocation_callbacks);
    vkDestroyImageView (context.logical_device, state.font_view, context.allocation_callbacks);
    vkFreeMemory (context.logical_device, state.font_memory, context.allocation_callbacks);
    vkDestroySampler (context.logical_device, state.sampler, context.allocation_callbacks);
    vkDestroyPipelineCache (context.logical_device, state.pipeline_cache, context.allocation_callbacks);
    vkDestroyPipeline (context.logical_device, state.pipeline, context.allocation_callbacks);
    vkDestroyPipelineLayout (context.logical_device, state.pipeline_layout, context.allocation_callbacks);
    vkDestroyDescriptorPool (context.logical_device, state.descriptor_pool, context.allocation_callbacks);
    vkDestroyDescriptorSetLayout (context.logical_device, state.descriptor_set_layout, context.allocation_callbacks);
}

VkPipelineShaderStageCreateInfo imgui::load_shader (std::string fileName, VkShaderStageFlagBits stage) {
    std::vector<uint8_t> content;
    sge::utils::get_file_stream (content, fileName.c_str ());
    auto module = utils::create_shader_module (context.logical_device, context.allocation_callbacks, content);
    assert (module != VK_NULL_HANDLE);
    auto shader_stage = utils::init_VkPipelineShaderStageCreateInfo (stage, module, "main");
    state.shaders.push_back (shader_stage.module);
    return shader_stage;
}

void imgui::init_resources (VkRenderPass renderPass, VkQueue copy_queue) {
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* font_data;
    int tex_width, tex_height;

    io.Fonts->GetTexDataAsRGBA32(&font_data, &tex_width, &tex_height);
    VkDeviceSize upload_size = (uint64_t) tex_width * (uint64_t) tex_height * (uint64_t) 4 * (uint64_t) (sizeof (char));

    VkImageCreateInfo image_info = utils::init_VkImageCreateInfo ();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent.width = tex_width;
    image_info.extent.height = tex_height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vk_assert (vkCreateImage (context.logical_device, &image_info, context.allocation_callbacks, &state.font_image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements (context.logical_device, state.font_image, &memory_requirements);

    auto memory_alloc_info = utils::init_VkMemoryAllocateInfo ();
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk_assert (vkAllocateMemory (context.logical_device, &memory_alloc_info, context.allocation_callbacks, &state.font_memory));

    vk_assert (vkBindImageMemory (context.logical_device, state.font_image, state.font_memory, 0));

    auto view_info = utils::init_VkImageViewCreateInfo ();
    view_info.image = state.font_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    vk_assert (vkCreateImageView (context.logical_device, &view_info, context.allocation_callbacks, &state.font_view));

    vk::device_buffer staging_buffer;

    context.create_buffer (
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        upload_size);

    staging_buffer.map ();
    memcpy (staging_buffer.mapped, font_data, upload_size);
    staging_buffer.unmap ();

    VkCommandBuffer copy_command = context.create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    utils::set_image_layout (
        copy_command,
        state.font_image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy buffer_copy_region = {};
    buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy_region.imageSubresource.layerCount = 1;
    buffer_copy_region.imageExtent.width = tex_width;
    buffer_copy_region.imageExtent.height = tex_height;
    buffer_copy_region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage (
        copy_command,
        staging_buffer.buffer,
        state.font_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &buffer_copy_region
    );

    utils::set_image_layout (
        copy_command,
        state.font_image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    context.flush_command_buffer (copy_command, copy_queue, true);

    staging_buffer.destroy (context.allocation_callbacks);

    auto sampler_info = utils::init_VkSamplerCreateInfo ();
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vk_assert (vkCreateSampler (context.logical_device, &sampler_info, context.allocation_callbacks, &state.sampler));

    std::vector<VkDescriptorPoolSize> pool_sizes = { utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) };

    auto descriptor_pool_info   = utils::init_VkDescriptorPoolCreateInfo (pool_sizes, 2);
    vk_assert (vkCreateDescriptorPool (context.logical_device, &descriptor_pool_info, context.allocation_callbacks, &state.descriptor_pool));

    std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = { utils::init_VkDescriptorSetLayoutBinding (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0), };
    auto descriptor_layout = utils::init_VkDescriptorSetLayoutCreateInfo (set_layout_bindings);
    vk_assert (vkCreateDescriptorSetLayout (context.logical_device, &descriptor_layout, context.allocation_callbacks, &state.descriptor_set_layout));

    auto desc_set_alloc_info    = utils::init_VkDescriptorSetAllocateInfo (state.descriptor_pool, &state.descriptor_set_layout, 1);
    vk_assert (vkAllocateDescriptorSets (context.logical_device, &desc_set_alloc_info, &state.descriptor_set));

    auto font_descriptor        = utils::init_VkDescriptorImageInfo (state.sampler, state.font_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = { utils::init_VkWriteDescriptorSet (state.descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &font_descriptor, 1) };
    vkUpdateDescriptorSets (context.logical_device, static_cast<uint32_t>(write_descriptor_sets.size ()), write_descriptor_sets.data (), 0, nullptr);

    auto pipeline_cache_info    = utils::init_VkPipelineCacheCreateInfo ();
    vk_assert (vkCreatePipelineCache (context.logical_device, &pipeline_cache_info, context.allocation_callbacks, &state.pipeline_cache));

    auto push_constant_range    = utils::init_VkPushConstantRange (VK_SHADER_STAGE_VERTEX_BIT, sizeof (push), 0);
    auto pipeline_layout_create_info = utils::init_VkPipelineLayoutCreateInfo (1, &state.descriptor_set_layout);
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    vk_assert (vkCreatePipelineLayout (context.logical_device, &pipeline_layout_create_info, context.allocation_callbacks, &state.pipeline_layout));

    auto input_assembly_state   = utils::init_VkPipelineInputAssemblyStateCreateInfo (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    auto rasterization_state    = utils::init_VkPipelineRasterizationStateCreateInfo (VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    VkPipelineColorBlendAttachmentState blend_attachment_state {};
    blend_attachment_state.blendEnable = VK_TRUE;
    blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    auto color_blend_state      = utils::init_VkPipelineColorBlendStateCreateInfo (1, &blend_attachment_state);
    auto depth_stencil_state    = utils::init_VkPipelineDepthStencilStateCreateInfo (VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
    auto viewport_state         = utils::init_VkPipelineViewportStateCreateInfo ();
    auto multisample_state      = utils::init_VkPipelineMultisampleStateCreateInfo (VK_SAMPLE_COUNT_1_BIT);

    std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    auto dynamic_state          = utils::init_VkPipelineDynamicStateCreateInfo (dynamic_states);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {};

    auto pipeline_create_info   = utils::init_VkGraphicsPipelineCreateInfo (state.pipeline_layout, renderPass);

    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.stageCount = static_cast<uint32_t>(shaderStages.size ());
    pipeline_create_info.pStages = shaderStages.data ();

    std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
        utils::init_VkVertexInputBindingDescription (0, sizeof (ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
        utils::init_VkVertexInputAttributeDescription (0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof (ImDrawVert, pos)),   // Location 0: Position
        utils::init_VkVertexInputAttributeDescription (0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof (ImDrawVert, uv)),    // Location 1: UV
        utils::init_VkVertexInputAttributeDescription (0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof (ImDrawVert, col)),  // Location 0: Color
    };
    VkPipelineVertexInputStateCreateInfo vertexInputState = utils::init_VkPipelineVertexInputStateCreateInfo ();
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size ());
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data ();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size ());
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data ();

    pipeline_create_info.pVertexInputState = &vertexInputState;

    shaderStages[0] = load_shader ("sge_imgui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = load_shader ("sge_imgui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    vk_assert (vkCreateGraphicsPipelines (context.logical_device, state.pipeline_cache, 1, &pipeline_create_info, context.allocation_callbacks, &state.pipeline));
}

void imgui::update_buffers () {
    ImDrawData* imDrawData = ImGui::GetDrawData ();
    VkDeviceSize vertex_buffer_size = imDrawData->TotalVtxCount * sizeof (ImDrawVert);
    VkDeviceSize index_buffer_size = imDrawData->TotalIdxCount * sizeof (ImDrawIdx);

    if ((vertex_buffer_size == 0) || (index_buffer_size == 0)) {
        return;
    }

    if ((state.vertex_buffer.buffer == VK_NULL_HANDLE) || (state.vertex_count != imDrawData->TotalVtxCount)) {
        state.vertex_buffer.unmap ();
        state.vertex_buffer.destroy (context.allocation_callbacks);
        context.create_buffer (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &state.vertex_buffer, vertex_buffer_size);
        state.vertex_count = imDrawData->TotalVtxCount;
        state.vertex_buffer.map ();
    }

    if ((state.index_buffer.buffer == VK_NULL_HANDLE) || (state.index_count < imDrawData->TotalIdxCount)) {
        state.index_buffer.unmap ();
        state.index_buffer.destroy (context.allocation_callbacks);
        context.create_buffer (VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &state.index_buffer, index_buffer_size);
        state.index_count = imDrawData->TotalIdxCount;
        state.index_buffer.map ();
    }

    ImDrawVert* vertex_dest = (ImDrawVert*)state.vertex_buffer.mapped;
    ImDrawIdx* index_dest = (ImDrawIdx*)state.index_buffer.mapped;

    for (int n = 0; n < imDrawData->CmdListsCount; n++) {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];
        memcpy (vertex_dest, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof (ImDrawVert));
        memcpy (index_dest, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof (ImDrawIdx));
        vertex_dest += cmd_list->VtxBuffer.Size;
        index_dest += cmd_list->IdxBuffer.Size;
    }

    state.vertex_buffer.flush ();
    state.index_buffer.flush ();
}

void imgui::draw_frame (VkCommandBuffer command_buffer) {
    ImGuiIO& io = ImGui::GetIO();

    vkCmdBindDescriptorSets (command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline_layout, 0, 1, &state.descriptor_set, 0, nullptr);
    vkCmdBindPipeline (command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);

    auto viewport = utils::init_VkViewport (ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
    vkCmdSetViewport (command_buffer, 0, 1, &viewport);

    state.push.scale = sge::math::vector2{ 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
    state.push.translation = sge::math::vector2{ -1.0f, -1.0f };
    vkCmdPushConstants (command_buffer, state.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof (push), &state.push);

    ImDrawData* draw_data = ImGui::GetDrawData ();
    int32_t vertex_offset = 0;
    int32_t index_offset = 0;

    if (draw_data->CmdListsCount > 0) {

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers (command_buffer, 0, 1, &state.vertex_buffer.buffer, offsets);
        vkCmdBindIndexBuffer (command_buffer, state.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

        for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissor_rect;
                scissor_rect.offset.x = std::max ((int32_t)(draw_cmd->ClipRect.x), 0);
                scissor_rect.offset.y = std::max ((int32_t)(draw_cmd->ClipRect.y), 0);
                scissor_rect.extent.width = (uint32_t)(draw_cmd->ClipRect.z - draw_cmd->ClipRect.x);
                scissor_rect.extent.height = (uint32_t)(draw_cmd->ClipRect.w - draw_cmd->ClipRect.y);
                vkCmdSetScissor (command_buffer, 0, 1, &scissor_rect);
                vkCmdDrawIndexed (command_buffer, draw_cmd->ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += draw_cmd->ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    }
}

}

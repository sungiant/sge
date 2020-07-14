#include "sge_vk_imgui.hh"

#include "sge_utils.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_presentation.hh" // todo, remove this dependency

namespace sge::vk {


imgui::imgui (const struct context& z_context, const struct queue_identifier z_queue_identifier, const class presentation& z_presentation, const std::function <void()>& z_imgui_fn)
    : context (z_context)
    , identifier (z_queue_identifier)
    , presentation (z_presentation)
    , imgui_fn (z_imgui_fn)
{
    ImGui::CreateContext ();
    ImGuiIO& io = ImGui::GetIO ();
    io.DisplaySize = ImVec2 ((float)presentation.extent ().width, (float)presentation.extent ().height);
    io.DisplayFramebufferScale = ImVec2 (1.0f, 1.0f);

    ImGuiStyle& style = ImGui::GetStyle ();
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.GrabRounding = 4.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4 (0.00f, 0.00f, 0.00f, 1.00f);                    colors[ImGuiCol_TextDisabled] = ImVec4 (0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4 (0.94f, 0.94f, 0.94f, 0.94f);                colors[ImGuiCol_ChildBg] = ImVec4 (0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4 (1.00f, 1.00f, 1.00f, 0.98f);                 colors[ImGuiCol_Border] = ImVec4 (0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow] = ImVec4 (0.00f, 0.00f, 0.00f, 0.00f);            colors[ImGuiCol_FrameBg] = ImVec4 (1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4 (0.98f, 0.52f, 0.26f, 0.40f);          colors[ImGuiCol_FrameBgActive] = ImVec4 (0.98f, 0.52f, 0.26f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4 (0.88f, 0.60f, 0.44f, 1.00f);                 colors[ImGuiCol_TitleBgActive] = ImVec4 (0.93f, 0.47f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4 (1.00f, 1.00f, 1.00f, 0.51f);        colors[ImGuiCol_MenuBarBg] = ImVec4 (0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4 (0.98f, 0.98f, 0.98f, 0.53f);             colors[ImGuiCol_ScrollbarGrab] = ImVec4 (0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4 (0.49f, 0.49f, 0.49f, 0.80f);    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4 (0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4 (0.98f, 0.52f, 0.26f, 1.00f);               colors[ImGuiCol_SliderGrab] = ImVec4 (0.98f, 0.52f, 0.26f, 0.78f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4 (0.80f, 0.58f, 0.46f, 0.60f);        colors[ImGuiCol_Button] = ImVec4 (0.98f, 0.52f, 0.26f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4 (0.98f, 0.52f, 0.26f, 1.00f);           colors[ImGuiCol_ButtonActive] = ImVec4 (0.98f, 0.40f, 0.06f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4 (0.98f, 0.52f, 0.26f, 0.31f);                  colors[ImGuiCol_HeaderHovered] = ImVec4 (0.98f, 0.52f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4 (0.98f, 0.52f, 0.26f, 1.00f);            colors[ImGuiCol_Separator] = ImVec4 (0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4 (0.80f, 0.38f, 0.14f, 0.78f);        colors[ImGuiCol_SeparatorActive] = ImVec4 (0.80f, 0.38f, 0.14f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4 (0.80f, 0.80f, 0.80f, 0.56f);              colors[ImGuiCol_ResizeGripHovered] = ImVec4 (0.98f, 0.52f, 0.26f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4 (0.98f, 0.52f, 0.26f, 0.95f);        colors[ImGuiCol_Tab] = ImVec4 (0.84f, 0.79f, 0.76f, 0.93f);
    colors[ImGuiCol_TabHovered] = ImVec4 (0.98f, 0.52f, 0.26f, 0.80f);              colors[ImGuiCol_TabActive] = ImVec4 (0.88f, 0.70f, 0.60f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4 (0.94f, 0.93f, 0.92f, 0.99f);            colors[ImGuiCol_TabUnfocusedActive] = ImVec4 (0.91f, 0.80f, 0.74f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4 (0.39f, 0.39f, 0.39f, 1.00f);               colors[ImGuiCol_PlotLinesHovered] = ImVec4 (0.35f, 0.86f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4 (0.00f, 0.38f, 0.90f, 1.00f);           colors[ImGuiCol_PlotHistogramHovered] = ImVec4 (0.97f, 0.01f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4 (0.98f, 0.52f, 0.26f, 0.35f);          colors[ImGuiCol_DragDropTarget] = ImVec4 (0.98f, 0.52f, 0.26f, 0.95f);
    colors[ImGuiCol_NavHighlight] = ImVec4 (0.98f, 0.52f, 0.26f, 0.80f);            colors[ImGuiCol_NavWindowingHighlight] = ImVec4 (0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4 (0.20f, 0.20f, 0.20f, 0.20f);       colors[ImGuiCol_ModalWindowDimBg] = ImVec4 (0.20f, 0.20f, 0.20f, 0.35f);
};

imgui::~imgui () {
    ImGui::DestroyContext ();
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::debug_ui () {
    ImGui::Text ("ImGui");
    ImGui::BulletText ("vertex count: %d [buffer v%d]", state.vertex_buffer.count, state.vertex_buffer.create_count);
    ImGui::BulletText ("index count: %d [buffer v%d]", state.index_buffer.count, state.index_buffer.create_count);
}

void imgui::record (image_index i) {
    ImGui::GetIO ().DisplaySize = ImVec2 { (float) presentation.extent ().width, (float) presentation.extent ().height };
    ImGui::NewFrame ();
    imgui_fn ();
    ImGui::Render ();

    state.push.scale = sge::math::vector2 { 2.0f / (float) presentation.extent ().width, 2.0f / (float) presentation.extent ().height };
    state.push.translation = sge::math::vector2{ -1.0f, -1.0f };


    ImDrawData* const imDrawData = ImGui::GetDrawData ();
    const VkDeviceSize vertex_buffer_size = imDrawData->TotalVtxCount * sizeof (ImDrawVert);
    const VkDeviceSize index_buffer_size = imDrawData->TotalIdxCount * sizeof (ImDrawIdx);

    if ((vertex_buffer_size == 0) || (index_buffer_size == 0)) {
        return;
    }

    if (state.vertex_buffer.count == 0) { // vertex buffer hasn't been created yet
        create_resources (resource_bit::VERTEX_BUFFER);
    }
    else if (state.vertex_buffer.count != imDrawData->TotalVtxCount) { // vertex buffer is no longer suitable
        destroy_resources (resource_bit::VERTEX_BUFFER);
        create_resources (resource_bit::VERTEX_BUFFER);
    }

    if (state.index_buffer.count == 0) { // index buffer hasn't been created yet
        create_resources (resource_bit::INDEX_BUFFER);
    }
    else if (state.index_buffer.count != imDrawData->TotalIdxCount) { // index buffer is no longer suitable
        destroy_resources (resource_bit::INDEX_BUFFER);
        create_resources (resource_bit::INDEX_BUFFER);
    }

    assert (state.vertex_buffer.value.buffer != VK_NULL_HANDLE);
    assert (state.index_buffer.value.buffer != VK_NULL_HANDLE);

    ImDrawVert* vertex_dest = (ImDrawVert*)state.vertex_buffer.value.mapped;
    ImDrawIdx* index_dest = (ImDrawIdx*)state.index_buffer.value.mapped;

    for (int n = 0; n < imDrawData->CmdListsCount; n++) {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];
        memcpy (vertex_dest, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof (ImDrawVert));
        memcpy (index_dest, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof (ImDrawIdx));
        vertex_dest += cmd_list->VtxBuffer.Size;
        index_dest += cmd_list->IdxBuffer.Size;
    }

    state.vertex_buffer.value.flush ();
    state.index_buffer.value.flush ();


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
    render_pass_info.framebuffer = presentation.frame_buffer (i);

    const VkCommandBuffer command_buffer = state.command_buffers[i];
    auto buffer_info = utils::init_VkCommandBufferBeginInfo ();
    vk_assert (vkBeginCommandBuffer (command_buffer, &buffer_info));

    vkCmdBeginRenderPass (command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    auto viewport = utils::init_VkViewport (ImGui::GetIO ().DisplaySize.x, ImGui::GetIO ().DisplaySize.y, 0.0f, 1.0f);
    vkCmdSetViewport (command_buffer, 0, 1, &viewport);
    auto scissor = utils::init_VkRect2D (presentation.extent ().width, presentation.extent ().height);
    vkCmdSetScissor (command_buffer, 0, 1, &scissor);
    vkCmdBindDescriptorSets (command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline.layout, 0, 1, &state.font_descriptor.set, 0, nullptr);
    vkCmdBindPipeline (command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline.value);
    vkCmdPushConstants (command_buffer, state.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof (push), &state.push);

    ImDrawData* draw_data = ImGui::GetDrawData ();
    int32_t vertex_offset = 0;
    int32_t index_offset = 0;

    if (draw_data->CmdListsCount > 0) {

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers (command_buffer, 0, 1, &state.vertex_buffer.value.buffer, offsets);
        vkCmdBindIndexBuffer (command_buffer, state.index_buffer.value.buffer, 0, VK_INDEX_TYPE_UINT16);

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

    vkCmdEndRenderPass (command_buffer);
    vk_assert (vkEndCommandBuffer (command_buffer));
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_resources (resource_flags flags) {
    using namespace sge::utils;
    if (get_flag_at_mask (flags, SYNCHRONISATION)) { assert (!get_flag_at_mask (state.resource_status, SYNCHRONISATION)); create_synchronisation  (); set_flag_at_mask (state.resource_status, SYNCHRONISATION, true ); }
    if (get_flag_at_mask (flags, COMMAND_POOL))    { assert (!get_flag_at_mask (state.resource_status, COMMAND_POOL));    create_command_pool     (); set_flag_at_mask (state.resource_status, COMMAND_POOL,    true ); }
    if (get_flag_at_mask (flags, SHADER))          { assert (!get_flag_at_mask (state.resource_status, SHADER));          create_shader           (); set_flag_at_mask (state.resource_status, SHADER,          true ); }
    if (get_flag_at_mask (flags, SAMPLER))         { assert (!get_flag_at_mask (state.resource_status, SAMPLER));         create_sampler          (); set_flag_at_mask (state.resource_status, SAMPLER,         true ); }
    if (get_flag_at_mask (flags, FONT_TEXTURE))    { assert (!get_flag_at_mask (state.resource_status, FONT_TEXTURE));    create_font_texture     (); set_flag_at_mask (state.resource_status, FONT_TEXTURE,    true ); }
    if (get_flag_at_mask (flags, FONT_DESCRIPTOR)) { assert (!get_flag_at_mask (state.resource_status, FONT_DESCRIPTOR)); create_font_descriptor  (); set_flag_at_mask (state.resource_status, FONT_DESCRIPTOR, true ); }
    if (get_flag_at_mask (flags, PIPELINE))        { assert (!get_flag_at_mask (state.resource_status, PIPELINE));        create_pipeline         (); set_flag_at_mask (state.resource_status, PIPELINE,        true ); }
    if (get_flag_at_mask (flags, COMMAND_BUFFER))  { assert (!get_flag_at_mask (state.resource_status, COMMAND_BUFFER));  create_command_buffer   (); set_flag_at_mask (state.resource_status, COMMAND_BUFFER,  true ); }
    if (get_flag_at_mask (flags, VERTEX_BUFFER))   { assert (!get_flag_at_mask (state.resource_status, VERTEX_BUFFER));   create_vertex_buffer    (); set_flag_at_mask (state.resource_status, VERTEX_BUFFER,   true ); }
    if (get_flag_at_mask (flags, INDEX_BUFFER))    { assert (!get_flag_at_mask (state.resource_status, INDEX_BUFFER));    create_index_buffer     (); set_flag_at_mask (state.resource_status, INDEX_BUFFER,    true ); }
}

void imgui::destroy_resources (resource_flags flags) {
    using namespace sge::utils;
    if (get_flag_at_mask (flags, INDEX_BUFFER))    { assert ( get_flag_at_mask (state.resource_status, INDEX_BUFFER));    destroy_index_buffer    (); set_flag_at_mask (state.resource_status, INDEX_BUFFER,    false); }
    if (get_flag_at_mask (flags, VERTEX_BUFFER))   { assert ( get_flag_at_mask (state.resource_status, VERTEX_BUFFER));   destroy_vertex_buffer   (); set_flag_at_mask (state.resource_status, VERTEX_BUFFER,   false); }
    if (get_flag_at_mask (flags, COMMAND_BUFFER))  { assert ( get_flag_at_mask (state.resource_status, COMMAND_BUFFER));  destroy_command_buffer  (); set_flag_at_mask (state.resource_status, COMMAND_BUFFER,  false); }
    if (get_flag_at_mask (flags, PIPELINE))        { assert ( get_flag_at_mask (state.resource_status, PIPELINE));        destroy_pipeline        (); set_flag_at_mask (state.resource_status, PIPELINE,        false); }
    if (get_flag_at_mask (flags, FONT_DESCRIPTOR)) { assert ( get_flag_at_mask (state.resource_status, FONT_DESCRIPTOR)); destroy_font_descriptor (); set_flag_at_mask (state.resource_status, FONT_DESCRIPTOR, false); }
    if (get_flag_at_mask (flags, FONT_TEXTURE))    { assert ( get_flag_at_mask (state.resource_status, FONT_TEXTURE));    destroy_font_texture    (); set_flag_at_mask (state.resource_status, FONT_TEXTURE,    false); }
    if (get_flag_at_mask (flags, SAMPLER))         { assert ( get_flag_at_mask (state.resource_status, SAMPLER));         destroy_sampler         (); set_flag_at_mask (state.resource_status, SAMPLER,         false); }
    if (get_flag_at_mask (flags, SHADER))          { assert ( get_flag_at_mask (state.resource_status, SHADER));          destroy_shader          (); set_flag_at_mask (state.resource_status, SHADER,          false); }
    if (get_flag_at_mask (flags, COMMAND_POOL))    { assert ( get_flag_at_mask (state.resource_status, COMMAND_POOL));    destroy_command_pool    (); set_flag_at_mask (state.resource_status, COMMAND_POOL,    false); }
    if (get_flag_at_mask (flags, SYNCHRONISATION)) { assert ( get_flag_at_mask (state.resource_status, SYNCHRONISATION)); destroy_synchronisation (); set_flag_at_mask (state.resource_status, SYNCHRONISATION, false); }
}


//--------------------------------------------------------------------------------------------------------------------//
void imgui::create_synchronisation () {
    auto semaphore_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_info, context.allocation_callbacks, &state.synchronisation.render_finished));
}

void imgui::destroy_synchronisation () {
    vkDestroySemaphore (context.logical_device, state.synchronisation.render_finished, context.allocation_callbacks);
    state.synchronisation.render_finished = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_command_pool () {
    auto pool_info = utils::init_VkCommandPoolCreateInfo (identifier.family_index);
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_assert (vkCreateCommandPool (context.logical_device, &pool_info, context.allocation_callbacks, &state.command_pool));
}

void imgui::destroy_command_pool () {
    vkDestroyCommandPool (context.logical_device, state.command_pool, context.allocation_callbacks);
    state.command_pool = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_shader () {
    std::vector<uint8_t> vert_spirv, frag_spirv;
    sge::utils::get_file_stream (vert_spirv, "sge_imgui.vert.spv");
    sge::utils::get_file_stream (frag_spirv, "sge_imgui.frag.spv");
    const auto vertex_create_info = vk::utils::init_VkShaderModuleCreateInfo (vert_spirv.size (), reinterpret_cast<const uint32_t*>(vert_spirv.data ()));
    const auto fragment_create_info = vk::utils::init_VkShaderModuleCreateInfo (frag_spirv.size (), reinterpret_cast<const uint32_t*>(frag_spirv.data ()));
    vk_assert (vkCreateShaderModule (context.logical_device, &vertex_create_info, context.allocation_callbacks, &state.shader.vertex));
    vk_assert (vkCreateShaderModule (context.logical_device, &fragment_create_info, context.allocation_callbacks, &state.shader.fragment));
}

void imgui::destroy_shader () {

    vkDestroyShaderModule (context.logical_device, state.shader.vertex, context.allocation_callbacks);
    vkDestroyShaderModule (context.logical_device, state.shader.fragment, context.allocation_callbacks);
    state.shader.vertex = VK_NULL_HANDLE;
    state.shader.fragment = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_sampler () {
    auto sampler_info = utils::init_VkSamplerCreateInfo ();
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vk_assert (vkCreateSampler (context.logical_device, &sampler_info, context.allocation_callbacks, &state.sampler));
}

void imgui::destroy_sampler () {
    vkDestroySampler (context.logical_device, state.sampler, context.allocation_callbacks);
    state.sampler = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_font_texture () {

    ImGuiIO& io = ImGui::GetIO ();
    unsigned char* font_data;
    int tex_width, tex_height;

    io.Fonts->GetTexDataAsRGBA32 (&font_data, &tex_width, &tex_height);
    VkDeviceSize upload_size = (uint64_t)tex_width * (uint64_t)tex_height * (uint64_t)4 * (uint64_t)(sizeof (char));

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
    vk_assert (vkCreateImage (context.logical_device, &image_info, context.allocation_callbacks, &state.font.image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements (context.logical_device, state.font.image, &memory_requirements);

    auto memory_alloc_info = utils::init_VkMemoryAllocateInfo ();
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk_assert (vkAllocateMemory (context.logical_device, &memory_alloc_info, context.allocation_callbacks, &state.font.memory));

    vk_assert (vkBindImageMemory (context.logical_device, state.font.image, state.font.memory, 0));

    auto view_info = utils::init_VkImageViewCreateInfo ();
    view_info.image = state.font.image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    vk_assert (vkCreateImageView (context.logical_device, &view_info, context.allocation_callbacks, &state.font.view));

    vk::device_buffer staging_buffer;

    context.create_buffer (
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        upload_size);

    staging_buffer.map ();
    memcpy (staging_buffer.mapped, font_data, upload_size);
    staging_buffer.unmap ();

    VkCommandBuffer copy_command = context.create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, identifier, true);

    utils::set_image_layout (
        copy_command,
        state.font.image,
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
        state.font.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &buffer_copy_region
    );

    utils::set_image_layout (
        copy_command,
        state.font.image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    context.flush_command_buffer (copy_command, identifier, true);

    staging_buffer.destroy (context.allocation_callbacks);
}

void imgui::destroy_font_texture () {
    vkDestroyImage (context.logical_device, state.font.image, context.allocation_callbacks);
    vkDestroyImageView (context.logical_device, state.font.view, context.allocation_callbacks);
    vkFreeMemory (context.logical_device, state.font.memory, context.allocation_callbacks);
    state.font.image = VK_NULL_HANDLE;
    state.font.view = VK_NULL_HANDLE;
    state.font.memory = VK_NULL_HANDLE;
}


//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_font_descriptor () {
    const std::vector<VkDescriptorPoolSize> pool_sizes = { utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) };

    const auto descriptor_pool_info = utils::init_VkDescriptorPoolCreateInfo (pool_sizes, 2);
    vk_assert (vkCreateDescriptorPool (context.logical_device, &descriptor_pool_info, context.allocation_callbacks, &state.font_descriptor.pool));

    const std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = { utils::init_VkDescriptorSetLayoutBinding (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0), };
    auto descriptor_layout = utils::init_VkDescriptorSetLayoutCreateInfo (set_layout_bindings);
    vk_assert (vkCreateDescriptorSetLayout (context.logical_device, &descriptor_layout, context.allocation_callbacks, &state.font_descriptor.set_layout));

    const auto desc_set_alloc_info = utils::init_VkDescriptorSetAllocateInfo (state.font_descriptor.pool, &state.font_descriptor.set_layout, 1);
    vk_assert (vkAllocateDescriptorSets (context.logical_device, &desc_set_alloc_info, &state.font_descriptor.set));

    const auto font_descriptor = utils::init_VkDescriptorImageInfo (state.sampler, state.font.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    const std::vector<VkWriteDescriptorSet> write_descriptor_sets = { utils::init_VkWriteDescriptorSet (state.font_descriptor.set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &font_descriptor, 1) };
    vkUpdateDescriptorSets (context.logical_device, static_cast<uint32_t>(write_descriptor_sets.size ()), write_descriptor_sets.data (), 0, nullptr);
}

void imgui::destroy_font_descriptor () {
    vkDestroyDescriptorPool (context.logical_device, state.font_descriptor.pool, context.allocation_callbacks);
    vkDestroyDescriptorSetLayout (context.logical_device, state.font_descriptor.set_layout, context.allocation_callbacks);
    state.font_descriptor.pool = VK_NULL_HANDLE;
    state.font_descriptor.set_layout = VK_NULL_HANDLE;
    state.font_descriptor.set = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//
void imgui::create_command_buffer () {
    // Create one command buffer for each swap chain image and reuse for rendering
    state.command_buffers.resize (presentation.num_frame_buffers ());

    auto cmdBufAllocateInfo =
        utils::init_VkCommandBufferAllocateInfo (
            state.command_pool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(state.command_buffers.size ()));

    vk_assert (vkAllocateCommandBuffers (context.logical_device, &cmdBufAllocateInfo, state.command_buffers.data ()));
}

void imgui::destroy_command_buffer () {

    vkFreeCommandBuffers (context.logical_device, state.command_pool, static_cast<uint32_t>(state.command_buffers.size ()), state.command_buffers.data ());
    state.command_buffers.clear ();
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_vertex_buffer () {
    const int imgui_vtx_count = ImGui::GetDrawData ()->TotalVtxCount;
    const VkDeviceSize imgui_vtx_buffer_size = imgui_vtx_count * sizeof (ImDrawVert);
    assert (imgui_vtx_count > 0);
    context.create_buffer (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &state.vertex_buffer.value, imgui_vtx_buffer_size);
    state.vertex_buffer.count = imgui_vtx_count;
    state.vertex_buffer.value.map ();
    state.vertex_buffer.create_count++;
}

void imgui::destroy_vertex_buffer () {
    state.vertex_buffer.value.unmap ();
    state.vertex_buffer.value.destroy (context.allocation_callbacks);
    state.vertex_buffer.value = {};
    state.vertex_buffer.count = 0;
}

//--------------------------------------------------------------------------------------------------------------------//

void imgui::create_index_buffer () {
    const int imgui_idx_count = ImGui::GetDrawData ()->TotalIdxCount;
    const VkDeviceSize imgui_idx_buffer_size = imgui_idx_count * sizeof (ImDrawIdx);
    assert (imgui_idx_count > 0);
    context.create_buffer (VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &state.index_buffer.value, imgui_idx_buffer_size);
    state.index_buffer.count = imgui_idx_count;
    state.index_buffer.value.map ();
    state.index_buffer.create_count++;
}

void imgui::destroy_index_buffer () {
    state.index_buffer.value.unmap ();
    state.index_buffer.value.destroy (context.allocation_callbacks);
    state.index_buffer.value = {};
    state.index_buffer.count = 0;
}

//--------------------------------------------------------------------------------------------------------------------//
void imgui::create_pipeline () {

    const auto pipeline_cache_info = utils::init_VkPipelineCacheCreateInfo ();
    vk_assert (vkCreatePipelineCache (context.logical_device, &pipeline_cache_info, context.allocation_callbacks, &state.pipeline.cache));

    const auto push_constant_range = utils::init_VkPushConstantRange (VK_SHADER_STAGE_VERTEX_BIT, sizeof (push), 0);
    auto pipeline_layout_create_info = utils::init_VkPipelineLayoutCreateInfo (1, &state.font_descriptor.set_layout);
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
    vk_assert (vkCreatePipelineLayout (context.logical_device, &pipeline_layout_create_info, context.allocation_callbacks, &state.pipeline.layout));

    const auto input_assembly_state = utils::init_VkPipelineInputAssemblyStateCreateInfo (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    const auto rasterization_state = utils::init_VkPipelineRasterizationStateCreateInfo (VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    VkPipelineColorBlendAttachmentState blend_attachment_state{};
    blend_attachment_state.blendEnable = VK_TRUE;
    blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    const auto color_blend_state = utils::init_VkPipelineColorBlendStateCreateInfo (1, &blend_attachment_state);
    const auto depth_stencil_state = utils::init_VkPipelineDepthStencilStateCreateInfo (VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
    const auto viewport_state = utils::init_VkPipelineViewportStateCreateInfo ();
    const auto multisample_state = utils::init_VkPipelineMultisampleStateCreateInfo (VK_SAMPLE_COUNT_1_BIT);

    const std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const auto dynamic_state = utils::init_VkPipelineDynamicStateCreateInfo (dynamic_states);

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {
        utils::init_VkPipelineShaderStageCreateInfo (VK_SHADER_STAGE_VERTEX_BIT, state.shader.vertex, "main"),
        utils::init_VkPipelineShaderStageCreateInfo (VK_SHADER_STAGE_FRAGMENT_BIT, state.shader.fragment, "main")
    };

    const std::vector<VkVertexInputBindingDescription> vertex_input_bindings = {
        utils::init_VkVertexInputBindingDescription (0, sizeof (ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    const std::vector<VkVertexInputAttributeDescription> vertex_input_attributes = {
        utils::init_VkVertexInputAttributeDescription (0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof (ImDrawVert, pos)),   // Location 0: Position
        utils::init_VkVertexInputAttributeDescription (0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof (ImDrawVert, uv)),    // Location 1: UV
        utils::init_VkVertexInputAttributeDescription (0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof (ImDrawVert, col)),  // Location 0: Color
    };

    auto vertex_input_state = utils::init_VkPipelineVertexInputStateCreateInfo ();
    vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_bindings.size ());
    vertex_input_state.pVertexBindingDescriptions = vertex_input_bindings.data ();
    vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size ());
    vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data ();

    auto pipeline_create_info = utils::init_VkGraphicsPipelineCreateInfo (state.pipeline.layout, presentation.imgui_render_pass ());
    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.stageCount = static_cast<uint32_t>(shaderStages.size ());
    pipeline_create_info.pStages = shaderStages.data ();
    pipeline_create_info.pVertexInputState = &vertex_input_state;

    vk_assert (vkCreateGraphicsPipelines (context.logical_device, state.pipeline.cache, 1, &pipeline_create_info, context.allocation_callbacks, &state.pipeline.value));

}

void imgui::destroy_pipeline () {
    vkDestroyPipelineCache (context.logical_device, state.pipeline.cache, context.allocation_callbacks);
    vkDestroyPipeline (context.logical_device, state.pipeline.value, context.allocation_callbacks);
    vkDestroyPipelineLayout (context.logical_device, state.pipeline.layout, context.allocation_callbacks);
    state.pipeline.cache = VK_NULL_HANDLE;
    state.pipeline.value = VK_NULL_HANDLE;
    state.pipeline.layout = VK_NULL_HANDLE;
}



}

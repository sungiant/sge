#include "sge_vk_compute_target.hpp"

#include "sge_vk_presentation.hpp"

#include <cassert>

namespace sge::vk {

compute_target::compute_target (const struct vk::context& z_context, const struct vk::queue_identifier& z_qid, const class presentation& p, const struct sge::app::content& z_content)
    : context (z_context)
    , identifier (z_qid)
    , presentation (p)
    , content (z_content)
{}

void compute_target::recreate () {
    destroy_r ();
    create_r ();
}
void compute_target::refresh () {
    destroy_rl ();
    create_rl ();
}

void compute_target::create () {
    auto semaphore_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_info, context.allocation_callbacks, &state.compute_complete));

    auto fenceCreateInfo = utils::init_VkFenceCreateInfo (VK_FENCE_CREATE_SIGNALED_BIT);
    vk_assert (vkCreateFence (context.logical_device, &fenceCreateInfo, context.allocation_callbacks, &state.fence));

    create_r ();
}

void compute_target::create_r () {
    prepare_texture_target (VK_FORMAT_R8G8B8A8_UNORM);
    prepare_uniform_buffers ();
    prepare_blob_buffers ();
    create_rl ();
}

void compute_target::create_rl () {
    create_descriptor_set_layout ();
    create_descriptor_set ();
    create_compute_pipeline ();
    create_command_buffer ();
    record_command_buffer ();
}

void compute_target::destroy_rl () {
    destroy_command_buffer ();
    destroy_compute_pipeline ();

    vkDestroyDescriptorPool (context.logical_device, state.descriptor_pool, context.allocation_callbacks);
    state.descriptor_pool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout (context.logical_device, state.descriptor_set_layout, context.allocation_callbacks);
    state.descriptor_set_layout = VK_NULL_HANDLE;

}
void compute_target::destroy_r () {
    destroy_rl ();
    destroy_blob_buffers ();
    destroy_uniform_buffers ();
    destroy_texture_target ();

}
void compute_target::destroy () {
    destroy_r ();

    state.compute_tex.destroy ();
    vkDestroySemaphore (context.logical_device, state.compute_complete, context.allocation_callbacks);
    state.compute_complete = VK_NULL_HANDLE;

    vkDestroyFence (context.logical_device, state.fence, context.allocation_callbacks);
    state.fence = VK_NULL_HANDLE;
}


void compute_target::enqueue () {
    auto submitInfo = utils::init_VkSubmitInfo ();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &state.command_buffer;

    VkSemaphore signalSemaphores[] = { state.compute_complete };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkWaitForFences (context.logical_device, 1, &state.fence, VK_TRUE, UINT64_MAX);
    vkResetFences (context.logical_device, 1, &state.fence);

    vk_assert (vkQueueSubmit (context.get_queue (identifier), 1, &submitInfo, state.fence));
}

void compute_target::append_pre_render_submissions (std::vector<VkSemaphore>& wait_on, std::vector<VkPipelineStageFlags>& stage_flags) {
    wait_on.emplace_back (state.compute_complete);
    stage_flags.emplace_back (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void compute_target::update (bool& push_flag, std::vector<bool>& ubo_flags) {

    if (push_flag) {
        record_command_buffer ();
        push_flag = false;
    }

    for (int i = 0; i < content.uniforms.size (); ++i) {
        if (ubo_flags[i]) {
            update_uniform_buffer (i);
            ubo_flags[i] = false;
        }
    }
}

void compute_target::prepare_uniform_buffers () {
    state.uniform_buffers.resize (content.uniforms.size ());
    for (int i = 0; i < content.uniforms.size (); ++i) {
        auto& u = content.uniforms[i];
        context.create_buffer (
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &state.uniform_buffers[i],
            u.size);

        update_uniform_buffer (i);
    }
}

void compute_target::update_uniform_buffer (int ubo_idx) {
    auto& u = content.uniforms[ubo_idx];
    state.uniform_buffers[ubo_idx].map ();
    assert (state.uniform_buffers[ubo_idx].size == u.size);
    memcpy (state.uniform_buffers[ubo_idx].mapped, u.address, u.size);
    state.uniform_buffers[ubo_idx].unmap ();
}

void compute_target::destroy_uniform_buffers () {
    for (int i = 0; i < content.uniforms.size (); ++i) {
        state.uniform_buffers[i].destroy (context.allocation_callbacks);
    }
    state.uniform_buffers.clear ();
}

void compute_target::copy_blob_from_staging_to_storage (int blob_idx) {
    assert (state.blob_staging_buffers[blob_idx].size == state.blob_storage_buffers[blob_idx].size);
    VkCommandBuffer copy_command = context.create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copy_region = {};
    copy_region.size = state.blob_staging_buffers[blob_idx].size;
    vkCmdCopyBuffer (
        copy_command,
        state.blob_staging_buffers[blob_idx].buffer,
        state.blob_storage_buffers[blob_idx].buffer,
        1, &copy_region);
    VkQueue queue = context.get_queue (identifier);
    context.flush_command_buffer (copy_command, queue, true);
    vkQueueWaitIdle (queue);
}

void compute_target::prepare_blob_buffers () {
    const int num_storage_buffers = content.blobs.size ();
    state.blob_staging_buffers.resize (num_storage_buffers);
    state.blob_storage_buffers.resize (num_storage_buffers);
    for (int i = 0; i < num_storage_buffers; ++i) {
        auto& blob = content.blobs[i];
        // copy user data into a staging buffer
        context.create_buffer (
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &state.blob_staging_buffers[i],
            blob.size,
            blob.address);
        // create an empty buffer on the gpu of the same size
        context.create_buffer(
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &state.blob_storage_buffers[i],
            blob.size);
        // copy the data from staging to gpu storage
        copy_blob_from_staging_to_storage (i);
    }
}

void compute_target::update_blob_buffer (int blob_idx, uint64_t size, void* data) {
    state.blob_staging_buffers[blob_idx].map ();
    state.blob_staging_buffers[blob_idx].copy (data, size);
    state.blob_staging_buffers[blob_idx].unmap ();
    copy_blob_from_staging_to_storage (blob_idx);
}

void compute_target::destroy_blob_buffers () {
    for (int i = 0; i < state.blob_staging_buffers.size (); ++i) {
        state.blob_storage_buffers[i].destroy (context.allocation_callbacks);
        state.blob_staging_buffers[i].destroy (context.allocation_callbacks);
    }
    state.blob_storage_buffers.clear ();
    state.blob_staging_buffers.clear ();
}

void compute_target::prepare_texture_target (VkFormat format) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties (context.physical_device, format, &formatProperties);
    assert (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    auto e = presentation.extent ();
    state.compute_tex.width = e.width;
    state.compute_tex.height = e.height;

    auto imageCreateInfo = utils::init_VkImageCreateInfo ();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { e.width, e.height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageCreateInfo.flags = 0;

    auto memAllocInfo = utils::init_VkMemoryAllocateInfo ();
    VkMemoryRequirements memReqs;

    vk_assert (vkCreateImage (context.logical_device, &imageCreateInfo, context.allocation_callbacks, &state.compute_tex.image));
    vkGetImageMemoryRequirements (context.logical_device, state.compute_tex.image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk_assert (vkAllocateMemory (context.logical_device, &memAllocInfo, context.allocation_callbacks, &state.compute_tex.device_memory));
    vk_assert (vkBindImageMemory (context.logical_device, state.compute_tex.image, state.compute_tex.device_memory, 0));

    VkCommandBuffer layoutCmd = context.create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    state.compute_tex.image_layout = VK_IMAGE_LAYOUT_GENERAL;
    utils::set_image_layout (
        layoutCmd,
        state.compute_tex.image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        state.compute_tex.image_layout);
    
    VkQueue queue = context.get_queue (identifier);
    context.flush_command_buffer (layoutCmd, queue, true);
    vk_assert (vkQueueWaitIdle (queue));

    auto sampler = utils::init_VkSamplerCreateInfo ();
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vk_assert (vkCreateSampler (context.logical_device, &sampler, context.allocation_callbacks, &state.compute_tex.sampler));

    VkImageViewCreateInfo view = utils::init_VkImageViewCreateInfo ();
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view.image = state.compute_tex.image;
    vk_assert (vkCreateImageView (context.logical_device, &view, context.allocation_callbacks, &state.compute_tex.view));

    state.compute_tex.descriptor.imageLayout = state.compute_tex.image_layout;
    state.compute_tex.descriptor.imageView = state.compute_tex.view;
    state.compute_tex.descriptor.sampler = state.compute_tex.sampler;
    state.compute_tex.context = &context;
}

void compute_target::destroy_texture_target () {
    state.compute_tex.destroy ();
}

void compute_target::create_descriptor_set_layout () {
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings = {
        utils::init_VkDescriptorSetLayoutBinding (
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0)
    };

    int idx = 1;
    for (int i = 0; i < state.uniform_buffers.size (); ++i) {
        descriptor_set_layout_bindings.emplace_back (
            utils::init_VkDescriptorSetLayoutBinding (
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_COMPUTE_BIT,
                idx++));
    }

    for (int i = 0; i < state.blob_storage_buffers.size (); ++i) {
        descriptor_set_layout_bindings.emplace_back (
            utils::init_VkDescriptorSetLayoutBinding (
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_COMPUTE_BIT,
                idx++));
    }

    auto descriptor_set_layout_create_info = utils::init_VkDescriptorSetLayoutCreateInfo (descriptor_set_layout_bindings);
    vk_assert (vkCreateDescriptorSetLayout (
        context.logical_device,
        &descriptor_set_layout_create_info,
        context.allocation_callbacks,
        &state.descriptor_set_layout));
}

void compute_target::create_descriptor_set () {
    std::vector<VkDescriptorPoolSize> pool_sizes = { utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1), };

    if (content.uniforms.size ()) {
        pool_sizes.emplace_back (utils::init_VkDescriptorPoolSize (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t) content.uniforms.size ()));
    }

    auto descriptor_pool_create_info = utils::init_VkDescriptorPoolCreateInfo (pool_sizes, (uint32_t) content.uniforms.size () + 1, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    vk_assert (vkCreateDescriptorPool (context.logical_device, &descriptor_pool_create_info, context.allocation_callbacks, &state.descriptor_pool));

    auto descriptor_set_allocate_info = utils::init_VkDescriptorSetAllocateInfo (state.descriptor_pool, &state.descriptor_set_layout, 1);
    vk_assert (vkAllocateDescriptorSets (context.logical_device, &descriptor_set_allocate_info, &state.descriptor_set));

    auto write_descriptor_set = utils::init_VkWriteDescriptorSet (state.descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &state.compute_tex.descriptor, 1);
    vkUpdateDescriptorSets (context.logical_device, 1, &write_descriptor_set, 0, nullptr);

    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
        utils::init_VkWriteDescriptorSet (
            state.descriptor_set,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            0,
            &state.compute_tex.descriptor, 1)
    };

    int idx = 1;
    for (int i = 0; i < state.uniform_buffers.size (); ++i) {
        write_descriptor_sets.emplace_back (
            utils::init_VkWriteDescriptorSet (
                state.descriptor_set,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                idx++,
                &state.uniform_buffers[i].descriptor, 1));
    };

    for (int i = 0; i < state.blob_storage_buffers.size (); ++i) {
        write_descriptor_sets.emplace_back (
            utils::init_VkWriteDescriptorSet (
                state.descriptor_set,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                idx++,
                &state.blob_storage_buffers[i].descriptor, 1));
    };

    vkUpdateDescriptorSets (context.logical_device, (uint32_t) write_descriptor_sets.size (), write_descriptor_sets.data (), 0, nullptr);
}

void compute_target::create_compute_pipeline () {

    std::vector<uint8_t> output;
    sge::utils::get_file_stream (output, content.shader_path.c_str ());
    state.compute_shader_module = utils::create_shader_module (context.logical_device, context.allocation_callbacks, output);

    auto shader_stage_create_info = utils::init_VkPipelineShaderStageCreateInfo (VK_SHADER_STAGE_COMPUTE_BIT, state.compute_shader_module, "main");

    auto pipeline_layout_create_info = utils::init_VkPipelineLayoutCreateInfo (1, &state.descriptor_set_layout);

    if (content.push_constants.has_value ()) {
        //https://stackoverflow.com/questions/50956414/what-is-a-push-constant-in-vulkan
        assert (content.push_constants.value ().size <= 128);
        VkPushConstantRange pushConstantRange =
            utils::init_VkPushConstantRange (
                VK_SHADER_STAGE_COMPUTE_BIT,
                (uint32_t) content.push_constants.value ().size);

        pipeline_layout_create_info.pushConstantRangeCount = 1;
        pipeline_layout_create_info.pPushConstantRanges = &pushConstantRange;
    }

    vk_assert (vkCreatePipelineLayout (context.logical_device, &pipeline_layout_create_info, context.allocation_callbacks, &state.pipeline_layout));

    auto pipeline_create_info = utils::init_VkComputePipelineCreateInfo (state.pipeline_layout);
    pipeline_create_info.stage = shader_stage_create_info;
    vk_assert (vkCreateComputePipelines (
        context.logical_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_create_info,
        context.allocation_callbacks,
        &state.pipeline));
}

void compute_target::destroy_compute_pipeline () {
    vkDestroyPipeline (context.logical_device, state.pipeline, context.allocation_callbacks);
    state.pipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout (context.logical_device, state.pipeline_layout, context.allocation_callbacks);
    state.pipeline_layout = VK_NULL_HANDLE;
    vkDestroyShaderModule (context.logical_device, state.compute_shader_module, context.allocation_callbacks);
    state.compute_shader_module = VK_NULL_HANDLE;
}

void compute_target::create_command_buffer () {
    auto command_pool_create_info = utils::init_VkCommandPoolCreateInfo (identifier.family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk_assert (vkCreateCommandPool (context.logical_device, &command_pool_create_info, context.allocation_callbacks, &state.command_pool));
    auto command_buffer_allocate_info = utils::init_VkCommandBufferAllocateInfo (state.command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    vk_assert (vkAllocateCommandBuffers (context.logical_device, &command_buffer_allocate_info, &state.command_buffer));
}

void compute_target::destroy_command_buffer () {
    state.command_buffer = VK_NULL_HANDLE;

    vkDestroyCommandPool (context.logical_device, state.command_pool, context.allocation_callbacks);
    state.command_pool = VK_NULL_HANDLE;
}

void compute_target::record_command_buffer () {
    auto begin_info = utils::init_VkCommandBufferBeginInfo ();
    vk_assert (vkBeginCommandBuffer (state.command_buffer, &begin_info));
    if (content.push_constants.has_value ()) {
        vkCmdPushConstants (
            state.command_buffer,
            state.pipeline_layout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            (uint32_t) content.push_constants.value ().size,
            content.push_constants.value ().address);
    }

    vkCmdBindPipeline (state.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, state.pipeline);
    vkCmdBindDescriptorSets (
        state.command_buffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        state.pipeline_layout,
        0,
        1,
        &state.descriptor_set,
        0,
        NULL);


    const uint32_t workgroup_size_x = 16;
    const uint32_t workgroup_size_y = 16;
    const uint32_t workgroup_size_z = 1;

    const auto e = presentation.extent ();

    vkCmdDispatch (
        state.command_buffer,
        (uint32_t) ceil (e.width / float (workgroup_size_x)),
        (uint32_t) ceil (e.height / float (workgroup_size_y)),
        workgroup_size_z);
    vk_assert (vkEndCommandBuffer (state.command_buffer));
}

void compute_target::run_command_buffer () {
    auto submit_info = utils::init_VkSubmitInfo ();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &state.command_buffer;
    VkFence fence;
    auto fence_create_info = utils::init_VkFenceCreateInfo ();
    vk_assert (vkCreateFence (context.logical_device, &fence_create_info, context.allocation_callbacks, &fence));
    vk_assert (vkQueueSubmit (context.get_queue (identifier), 1, &submit_info, fence));
    vk_assert (vkWaitForFences (context.logical_device, 1, &fence, VK_TRUE, 100000000000));
    vkDestroyFence (context.logical_device, fence, context.allocation_callbacks);
}

}

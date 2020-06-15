#pragma once

// SGE-VK-TEXTURE
// -------------------------------------
// Vulkan texture helper type.
// -------------------------------------

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>

#include <vulkan/vulkan.h>

#include "sge_vk_context.hpp"
#include "sge_vk_buffer.hpp"
#include "sge_vk_utils.hpp"

namespace sge::vk {

struct texture {

    const context* context;
    VkImage image;
    VkImageLayout image_layout;
    VkDeviceMemory device_memory;
    VkImageView view;
    uint32_t width;
    uint32_t height;
    uint32_t mip_levels;
    uint32_t layer_count;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;

    void destroy () {
        vkDestroyImageView (context->logical_device, view, context->allocation_callbacks);
        view = VK_NULL_HANDLE;
        vkDestroyImage (context->logical_device, image, context->allocation_callbacks);
        image = VK_NULL_HANDLE;
        if (sampler) {
            vkDestroySampler (context->logical_device, sampler, context->allocation_callbacks);
            sampler = VK_NULL_HANDLE;
        }
        vkFreeMemory (context->logical_device, device_memory, context->allocation_callbacks);
        device_memory = VK_NULL_HANDLE;
    }

    void from_buffer (
        void* buffer,
        VkDeviceSize buffer_size,
        VkFormat format,
        uint32_t texture_width,
        uint32_t texture_height,
        struct context& context,
        VkQueue copy_queue,
        VkFilter filter = VK_FILTER_LINEAR,
        VkImageUsageFlags image_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        assert (buffer);

        this->context = &context;
        width = texture_width;
        height = texture_height;
        mip_levels = 1;

        auto alloc_info = utils::init_VkMemoryAllocateInfo ();
        VkMemoryRequirements memory_requirements;

        VkCommandBuffer copy_command = context.create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBuffer staging_buffer;
        VkDeviceMemory staging_memory;

        auto buffer_create_info = utils::init_VkBufferCreateInfo ();
        buffer_create_info.size = buffer_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vk_assert (vkCreateBuffer (context.logical_device, &buffer_create_info, context.allocation_callbacks, &staging_buffer));

        vkGetBufferMemoryRequirements (context.logical_device, staging_buffer, &memory_requirements);

        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vk_assert (vkAllocateMemory (context.logical_device, &alloc_info, context.allocation_callbacks, &staging_memory));
        vk_assert (vkBindBufferMemory (context.logical_device, staging_buffer, staging_memory, 0));

        uint8_t *data;
        vk_assert (vkMapMemory (context.logical_device, staging_memory, 0, memory_requirements.size, 0, (void **)&data));
        std::memcpy (data, buffer, buffer_size);
        vkUnmapMemory (context.logical_device, staging_memory);

        VkBufferImageCopy buffer_copy_region = {};
        buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_copy_region.imageSubresource.mipLevel = 0;
        buffer_copy_region.imageSubresource.baseArrayLayer = 0;
        buffer_copy_region.imageSubresource.layerCount = 1;
        buffer_copy_region.imageExtent.width = width;
        buffer_copy_region.imageExtent.height = height;
        buffer_copy_region.imageExtent.depth = 1;
        buffer_copy_region.bufferOffset = 0;

        auto image_create_info = utils::init_VkImageCreateInfo ();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.mipLevels = mip_levels;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.extent = { width, height, 1 };
        image_create_info.usage = image_usage_flags;

        if (!(image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
            image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        vk_assert (vkCreateImage (context.logical_device, &image_create_info, context.allocation_callbacks, &image));

        vkGetImageMemoryRequirements (context.logical_device, image, &memory_requirements);

        alloc_info.allocationSize = memory_requirements.size;

        alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vk_assert (vkAllocateMemory (context.logical_device, &alloc_info, context.allocation_callbacks, &device_memory));
        vk_assert (vkBindImageMemory (context.logical_device, image, device_memory, 0));

        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = mip_levels;
        subresource_range.layerCount = 1;

        utils::set_image_layout (
            copy_command,
            image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        vkCmdCopyBufferToImage (
            copy_command,
            staging_buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &buffer_copy_region
        );

        this->image_layout = image_layout;
        utils::set_image_layout (
            copy_command,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            image_layout,
            subresource_range,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        context.flush_command_buffer (copy_command, copy_queue);

        vkFreeMemory (context.logical_device, staging_memory, context.allocation_callbacks);
        vkDestroyBuffer (context.logical_device, staging_buffer, context.allocation_callbacks);

        auto sampler_create_info = utils::init_VkSamplerCreateInfo();
        sampler_create_info.magFilter = filter;
        sampler_create_info.minFilter = filter;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
        vk_assert (vkCreateSampler (context.logical_device, &sampler_create_info, context.allocation_callbacks, &sampler));

        auto view_create_info = utils::init_VkImageViewCreateInfo ();
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = format;
        view_create_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view_create_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.image = image;
        vk_assert (vkCreateImageView (context.logical_device, &view_create_info, context.allocation_callbacks, &view));

        descriptor.sampler = sampler;
        descriptor.imageView = view;
        descriptor.imageLayout = image_layout;
    }

};

}

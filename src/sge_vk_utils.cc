#include "sge_vk_utils.hh"

#include <map>
#include <sstream>
#include <vector>
#include <numeric>
#include <cassert>
#include <iostream>
#include <fstream>
#include <unordered_map>

#undef max
#undef min

#include <algorithm>

namespace sge::vk::utils {

uint32_t choose_memory_type (VkPhysicalDevice physical_device, const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags required_flags, VkMemoryPropertyFlags preferred_flags) {

    VkPhysicalDeviceMemoryProperties device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties (physical_device, &device_memory_properties);

    uint32_t selected_type = ~0u;
    uint32_t memory_type;

    for (memory_type = 0; memory_type < 32; ++memory_type) {
        if (memory_requirements.memoryTypeBits & (1 << memory_type)) {
            const VkMemoryType& type = device_memory_properties.memoryTypes[memory_type];
            if ((type.propertyFlags & preferred_flags) == preferred_flags) {
                selected_type = memory_type;
                break;
            }
        }
    }

    if (selected_type != ~0u) {
        for (memory_type = 0; memory_type < 32; ++memory_type) {
            if (memory_requirements.memoryTypeBits & (1 << memory_type)) {

                const VkMemoryType& type = device_memory_properties.memoryTypes[memory_type];
                if ((type.propertyFlags & required_flags) == required_flags) {
                    selected_type = memory_type;
                    break;
                }
            }
        }
    }

    return selected_type;
}

VkSurfaceFormatKHR choose_swapchain_surface_format (const std::vector<VkSurfaceFormatKHR>& available_formats) {
    if (available_formats.size () == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED) {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR choose_swapchain_present_mode (const std::vector<VkPresentModeKHR> available_present_modes) {
    VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
        else if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            best_mode = available_present_mode;
        }
    }

    return best_mode;
}

VkExtent2D choose_swapchain_extent (const VkSurfaceCapabilitiesKHR& capabilities, const int view_w, const int view_h) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max ()) {
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actual_extent = { static_cast<uint32_t>(view_w), static_cast<uint32_t>(view_h) };
        actual_extent.width = std::max (capabilities.minImageExtent.width, std::min (capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = std::max (capabilities.minImageExtent.height, std::min (capabilities.maxImageExtent.height, actual_extent.height));
        return actual_extent;
    }
}

VkShaderModule create_shader_module (VkDevice device, const VkAllocationCallbacks* ac, const std::vector<uint8_t>& spirv) {
    auto create_info = init_VkShaderModuleCreateInfo (spirv.size (), reinterpret_cast<const uint32_t*>(spirv.data ()));
    VkShaderModule shader_module;
    vk_assert (vkCreateShaderModule (device, &create_info, ac, &shader_module));

    return shader_module;
}

VkBool32 get_supported_depth_format (VkPhysicalDevice physical_device, VkFormat* depth_format) {
    std::vector<VkFormat> depth_formats = { // Since all depth formats may be optional, we need to find a suitable depth format to use.  Start with the highest precision packed format.
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depth_formats) {
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties (physical_device, format, &format_props);
        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) { // Format must support depth stencil attachment for optimal tiling.
            *depth_format = format;
            return true;
        }
    }

    return false;
}

void set_image_layout(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout previous_layout,
    VkImageLayout new_layout,
    VkImageSubresourceRange subresource_range,
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dst_stage_mask)
{
    VkImageMemoryBarrier imageMemoryBarrier = init_VkImageMemoryBarrier ();
    imageMemoryBarrier.oldLayout = previous_layout;
    imageMemoryBarrier.newLayout = new_layout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresource_range;

    switch (previous_layout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:                         imageMemoryBarrier.srcAccessMask = 0;                                               break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;                        break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:          imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:  imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;    break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:              imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;                     break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:              imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;                    break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:          imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;                       break;
        default: break;
    }

    switch (new_layout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:              imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;                    break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:              imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;                     break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:          imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (imageMemoryBarrier.srcAccessMask == 0)
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default: break;
    }

    vkCmdPipelineBarrier(
        command_buffer,
        src_stage_mask,
        dst_stage_mask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

void set_image_layout(
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask)
{

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    set_image_layout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

}

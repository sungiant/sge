#include "sge_vk_utils.hh"

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

std::string to_string_VkResult (VkResult type)
{
    std::string result;
    switch (type) {
    case VK_SUCCESS: result = "VK_SUCCESS"; break;
    case VK_NOT_READY: result = "VK_NOT_READY"; break;
    case VK_TIMEOUT: result = "VK_TIMEOUT"; break;
    case VK_EVENT_SET: result = "VK_EVENT_SET"; break;
    case VK_EVENT_RESET: result = "VK_EVENT_RESET"; break;
    case VK_INCOMPLETE: result = "VK_INCOMPLETE"; break;
    case VK_ERROR_OUT_OF_HOST_MEMORY: result = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: result = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
    case VK_ERROR_INITIALIZATION_FAILED: result = "VK_ERROR_INITIALIZATION_FAILED"; break;
    case VK_ERROR_DEVICE_LOST: result = "VK_ERROR_DEVICE_LOST"; break;
    case VK_ERROR_MEMORY_MAP_FAILED: result = "VK_ERROR_MEMORY_MAP_FAILED"; break;
    case VK_ERROR_LAYER_NOT_PRESENT: result = "VK_ERROR_LAYER_NOT_PRESENT"; break;
    case VK_ERROR_EXTENSION_NOT_PRESENT: result = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
    case VK_ERROR_FEATURE_NOT_PRESENT: result = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
    case VK_ERROR_INCOMPATIBLE_DRIVER: result = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
    case VK_ERROR_TOO_MANY_OBJECTS: result = "VK_ERROR_TOO_MANY_OBJECTS"; break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED: result = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
    case VK_ERROR_FRAGMENTED_POOL: result = "VK_ERROR_FRAGMENTED_POOL"; break;

        //        case VK_ERROR_UNKNOWN                                                   : result = "VK_ERROR_UNKNOWN"; break;
    case VK_ERROR_OUT_OF_POOL_MEMORY: result = "VK_ERROR_OUT_OF_POOL_MEMORY"; break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: result = "VK_ERROR_INVALID_EXTERNAL_HANDLE"; break;
        //       case VK_ERROR_FRAGMENTATION                                             : result = "VK_ERROR_FRAGMENTATION"; break;
       //        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS                            : result = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"; break;
    case VK_ERROR_SURFACE_LOST_KHR: result = "VK_ERROR_SURFACE_LOST_KHR"; break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: result = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; break;
    case VK_SUBOPTIMAL_KHR: result = "VK_SUBOPTIMAL_KHR"; break;
    case VK_ERROR_OUT_OF_DATE_KHR: result = "VK_ERROR_OUT_OF_DATE_KHR"; break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: result = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"; break;
    case VK_ERROR_VALIDATION_FAILED_EXT: result = "VK_ERROR_VALIDATION_FAILED_EXT"; break;
    case VK_ERROR_INVALID_SHADER_NV: result = "VK_ERROR_INVALID_SHADER_NV"; break;
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: result = "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"; break;
    case VK_ERROR_NOT_PERMITTED_EXT: result = "VK_ERROR_NOT_PERMITTED_EXT"; break;
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: result = "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"; break;
    }
    return result;
}
std::string to_string_Version (uint32_t version)
{
    std::stringstream ss;
    ss << ((version >> 22) & 0x3FF) << ".";
    ss << ((version >> 12) & 0x3FF) << ".";
    ss << ((version >> 0) & 0xFFF);
    return ss.str ();
}

std::string to_string_VkPhysicalDeviceType (VkPhysicalDeviceType type)
{
    std::string result;
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: result = "VK_PHYSICAL_DEVICE_TYPE_OTHER"; break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU: result = "VK_PHYSICAL_DEVICE_TYPE_CPU"; break;
    }
    return result;
}

std::string to_string_VkFormat (VkFormat format)
{
    switch (format) {
    case VK_FORMAT_UNDEFINED: return "VK_FORMAT_UNDEFINED";
    case VK_FORMAT_R4G4_UNORM_PACK8: return "VK_FORMAT_R4G4_UNORM_PACK8";
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
    case VK_FORMAT_R5G6B5_UNORM_PACK16: return "VK_FORMAT_R5G6B5_UNORM_PACK16";
    case VK_FORMAT_B5G6R5_UNORM_PACK16: return "VK_FORMAT_B5G6R5_UNORM_PACK16";
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
    case VK_FORMAT_R8_UNORM: return "VK_FORMAT_R8_UNORM";
    case VK_FORMAT_R8_SNORM: return "VK_FORMAT_R8_SNORM";
    case VK_FORMAT_R8_USCALED: return "VK_FORMAT_R8_USCALED";
    case VK_FORMAT_R8_SSCALED: return "VK_FORMAT_R8_SSCALED";
    case VK_FORMAT_R8_UINT: return "VK_FORMAT_R8_UINT";
    case VK_FORMAT_R8_SINT: return "VK_FORMAT_R8_SINT";
    case VK_FORMAT_R8_SRGB: return "VK_FORMAT_R8_SRGB";
    case VK_FORMAT_R8G8_UNORM: return "VK_FORMAT_R8G8_UNORM";
    case VK_FORMAT_R8G8_SNORM: return "VK_FORMAT_R8G8_SNORM";
    case VK_FORMAT_R8G8_USCALED: return "VK_FORMAT_R8G8_USCALED";
    case VK_FORMAT_R8G8_SSCALED: return "VK_FORMAT_R8G8_SSCALED";
    case VK_FORMAT_R8G8_UINT: return "VK_FORMAT_R8G8_UINT";
    case VK_FORMAT_R8G8_SINT: return "VK_FORMAT_R8G8_SINT";
    case VK_FORMAT_R8G8_SRGB: return "VK_FORMAT_R8G8_SRGB";
    case VK_FORMAT_R8G8B8_UNORM: return "VK_FORMAT_R8G8B8_UNORM";
    case VK_FORMAT_R8G8B8_SNORM: return "VK_FORMAT_R8G8B8_SNORM";
    case VK_FORMAT_R8G8B8_USCALED: return "VK_FORMAT_R8G8B8_USCALED";
    case VK_FORMAT_R8G8B8_SSCALED: return "VK_FORMAT_R8G8B8_SSCALED";
    case VK_FORMAT_R8G8B8_UINT: return "VK_FORMAT_R8G8B8_UINT";
    case VK_FORMAT_R8G8B8_SINT: return "VK_FORMAT_R8G8B8_SINT";
    case VK_FORMAT_R8G8B8_SRGB: return "VK_FORMAT_R8G8B8_SRGB";
    case VK_FORMAT_B8G8R8_UNORM: return "VK_FORMAT_B8G8R8_UNORM";
    case VK_FORMAT_B8G8R8_SNORM: return "VK_FORMAT_B8G8R8_SNORM";
    case VK_FORMAT_B8G8R8_USCALED: return "VK_FORMAT_B8G8R8_USCALED";
    case VK_FORMAT_B8G8R8_SSCALED: return "VK_FORMAT_B8G8R8_SSCALED";
    case VK_FORMAT_B8G8R8_UINT: return "VK_FORMAT_B8G8R8_UINT";
    case VK_FORMAT_B8G8R8_SINT: return "VK_FORMAT_B8G8R8_SINT";
    case VK_FORMAT_B8G8R8_SRGB: return "VK_FORMAT_B8G8R8_SRGB";
    case VK_FORMAT_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_SNORM: return "VK_FORMAT_R8G8B8A8_SNORM";
    case VK_FORMAT_R8G8B8A8_USCALED: return "VK_FORMAT_R8G8B8A8_USCALED";
    case VK_FORMAT_R8G8B8A8_SSCALED: return "VK_FORMAT_R8G8B8A8_SSCALED";
    case VK_FORMAT_R8G8B8A8_UINT: return "VK_FORMAT_R8G8B8A8_UINT";
    case VK_FORMAT_R8G8B8A8_SINT: return "VK_FORMAT_R8G8B8A8_SINT";
    case VK_FORMAT_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
    case VK_FORMAT_B8G8R8A8_UNORM: return "VK_FORMAT_B8G8R8A8_UNORM";
    case VK_FORMAT_B8G8R8A8_SNORM: return "VK_FORMAT_B8G8R8A8_SNORM";
    case VK_FORMAT_B8G8R8A8_USCALED: return "VK_FORMAT_B8G8R8A8_USCALED";
    case VK_FORMAT_B8G8R8A8_SSCALED: return "VK_FORMAT_B8G8R8A8_SSCALED";
    case VK_FORMAT_B8G8R8A8_UINT: return "VK_FORMAT_B8G8R8A8_UINT";
    case VK_FORMAT_B8G8R8A8_SINT: return "VK_FORMAT_B8G8R8A8_SINT";
    case VK_FORMAT_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
    case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
    case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
    case VK_FORMAT_R16_UNORM: return "VK_FORMAT_R16_UNORM";
    case VK_FORMAT_R16_SNORM: return "VK_FORMAT_R16_SNORM";
    case VK_FORMAT_R16_USCALED: return "VK_FORMAT_R16_USCALED";
    case VK_FORMAT_R16_SSCALED: return "VK_FORMAT_R16_SSCALED";
    case VK_FORMAT_R16_UINT: return "VK_FORMAT_R16_UINT";
    case VK_FORMAT_R16_SINT: return "VK_FORMAT_R16_SINT";
    case VK_FORMAT_R16_SFLOAT: return "VK_FORMAT_R16_SFLOAT";
    case VK_FORMAT_R16G16_UNORM: return "VK_FORMAT_R16G16_UNORM";
    case VK_FORMAT_R16G16_SNORM: return "VK_FORMAT_R16G16_SNORM";
    case VK_FORMAT_R16G16_USCALED: return "VK_FORMAT_R16G16_USCALED";
    case VK_FORMAT_R16G16_SSCALED: return "VK_FORMAT_R16G16_SSCALED";
    case VK_FORMAT_R16G16_UINT: return "VK_FORMAT_R16G16_UINT";
    case VK_FORMAT_R16G16_SINT: return "VK_FORMAT_R16G16_SINT";
    case VK_FORMAT_R16G16_SFLOAT: return "VK_FORMAT_R16G16_SFLOAT";
    case VK_FORMAT_R16G16B16_UNORM: return "VK_FORMAT_R16G16B16_UNORM";
    case VK_FORMAT_R16G16B16_SNORM: return "VK_FORMAT_R16G16B16_SNORM";
    case VK_FORMAT_R16G16B16_USCALED: return "VK_FORMAT_R16G16B16_USCALED";
    case VK_FORMAT_R16G16B16_SSCALED: return "VK_FORMAT_R16G16B16_SSCALED";
    case VK_FORMAT_R16G16B16_UINT: return "VK_FORMAT_R16G16B16_UINT";
    case VK_FORMAT_R16G16B16_SINT: return "VK_FORMAT_R16G16B16_SINT";
    case VK_FORMAT_R16G16B16_SFLOAT: return "VK_FORMAT_R16G16B16_SFLOAT";
    case VK_FORMAT_R16G16B16A16_UNORM: return "VK_FORMAT_R16G16B16A16_UNORM";
    case VK_FORMAT_R16G16B16A16_SNORM: return "VK_FORMAT_R16G16B16A16_SNORM";
    case VK_FORMAT_R16G16B16A16_USCALED: return "VK_FORMAT_R16G16B16A16_USCALED";
    case VK_FORMAT_R16G16B16A16_SSCALED: return "VK_FORMAT_R16G16B16A16_SSCALED";
    case VK_FORMAT_R16G16B16A16_UINT: return "VK_FORMAT_R16G16B16A16_UINT";
    case VK_FORMAT_R16G16B16A16_SINT: return "VK_FORMAT_R16G16B16A16_SINT";
    case VK_FORMAT_R16G16B16A16_SFLOAT: return "VK_FORMAT_R16G16B16A16_SFLOAT";
    case VK_FORMAT_R32_UINT: return "VK_FORMAT_R32_UINT";
    case VK_FORMAT_R32_SINT: return "VK_FORMAT_R32_SINT";
    case VK_FORMAT_R32_SFLOAT: return "VK_FORMAT_R32_SFLOAT";
    case VK_FORMAT_R32G32_UINT: return "VK_FORMAT_R32G32_UINT";
    case VK_FORMAT_R32G32_SINT: return "VK_FORMAT_R32G32_SINT";
    case VK_FORMAT_R32G32_SFLOAT: return "VK_FORMAT_R32G32_SFLOAT";
    case VK_FORMAT_R32G32B32_UINT: return "VK_FORMAT_R32G32B32_UINT";
    case VK_FORMAT_R32G32B32_SINT: return "VK_FORMAT_R32G32B32_SINT";
    case VK_FORMAT_R32G32B32_SFLOAT: return "VK_FORMAT_R32G32B32_SFLOAT";
    case VK_FORMAT_R32G32B32A32_UINT: return "VK_FORMAT_R32G32B32A32_UINT";
    case VK_FORMAT_R32G32B32A32_SINT: return "VK_FORMAT_R32G32B32A32_SINT";
    case VK_FORMAT_R32G32B32A32_SFLOAT: return "VK_FORMAT_R32G32B32A32_SFLOAT";
    case VK_FORMAT_R64_UINT: return "VK_FORMAT_R64_UINT";
    case VK_FORMAT_R64_SINT: return "VK_FORMAT_R64_SINT";
    case VK_FORMAT_R64_SFLOAT: return "VK_FORMAT_R64_SFLOAT";
    case VK_FORMAT_R64G64_UINT: return "VK_FORMAT_R64G64_UINT";
    case VK_FORMAT_R64G64_SINT: return "VK_FORMAT_R64G64_SINT";
    case VK_FORMAT_R64G64_SFLOAT: return "VK_FORMAT_R64G64_SFLOAT";
    case VK_FORMAT_R64G64B64_UINT: return "VK_FORMAT_R64G64B64_UINT";
    case VK_FORMAT_R64G64B64_SINT: return "VK_FORMAT_R64G64B64_SINT";
    case VK_FORMAT_R64G64B64_SFLOAT: return "VK_FORMAT_R64G64B64_SFLOAT";
    case VK_FORMAT_R64G64B64A64_UINT: return "VK_FORMAT_R64G64B64A64_UINT";
    case VK_FORMAT_R64G64B64A64_SINT: return "VK_FORMAT_R64G64B64A64_SINT";
    case VK_FORMAT_R64G64B64A64_SFLOAT: return "VK_FORMAT_R64G64B64A64_SFLOAT";
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
    case VK_FORMAT_D16_UNORM: return "VK_FORMAT_D16_UNORM";
    case VK_FORMAT_X8_D24_UNORM_PACK32: return "VK_FORMAT_X8_D24_UNORM_PACK32";
    case VK_FORMAT_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
    case VK_FORMAT_S8_UINT: return "VK_FORMAT_S8_UINT";
    case VK_FORMAT_D16_UNORM_S8_UINT: return "VK_FORMAT_D16_UNORM_S8_UINT";
    case VK_FORMAT_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
    case VK_FORMAT_BC2_UNORM_BLOCK: return "VK_FORMAT_BC2_UNORM_BLOCK";
    case VK_FORMAT_BC2_SRGB_BLOCK: return "VK_FORMAT_BC2_SRGB_BLOCK";
    case VK_FORMAT_BC3_UNORM_BLOCK: return "VK_FORMAT_BC3_UNORM_BLOCK";
    case VK_FORMAT_BC3_SRGB_BLOCK: return "VK_FORMAT_BC3_SRGB_BLOCK";
    case VK_FORMAT_BC4_UNORM_BLOCK: return "VK_FORMAT_BC4_UNORM_BLOCK";
    case VK_FORMAT_BC4_SNORM_BLOCK: return "VK_FORMAT_BC4_SNORM_BLOCK";
    case VK_FORMAT_BC5_UNORM_BLOCK: return "VK_FORMAT_BC5_UNORM_BLOCK";
    case VK_FORMAT_BC5_SNORM_BLOCK: return "VK_FORMAT_BC5_SNORM_BLOCK";
    case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
    case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
    case VK_FORMAT_BC7_UNORM_BLOCK: return "VK_FORMAT_BC7_UNORM_BLOCK";
    case VK_FORMAT_BC7_SRGB_BLOCK: return "VK_FORMAT_BC7_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
    case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
    }
}

std::string to_string_VkColorSpaceKHR (VkColorSpaceKHR colorSpace)
{
    std::string result;
    switch (colorSpace) {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: result = "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR"; break;
    }
    return result;
}

std::string to_string_VkPresentModeKHR (VkPresentModeKHR mode)
{
    std::string result;
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: result = "VK_PRESENT_MODE_IMMEDIATE_KHR"; break;
    case VK_PRESENT_MODE_MAILBOX_KHR: result = "VK_PRESENT_MODE_MAILBOX_KHR"; break;
    case VK_PRESENT_MODE_FIFO_KHR: result = "VK_PRESENT_MODE_FIFO_KHR"; break;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: result = "VK_PRESENT_MODE_FIFO_RELAXED_KHR"; break;
    }
    return result;
}

std::string to_string_VkSurfaceTransformFlagsKHR (VkSurfaceTransformFlagsKHR transform)
{
    std::string result;
    switch (transform) {
    case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR: result = "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR: result = "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR: result = "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR: result = "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR: result = "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR: result = "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR: result = "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR: result = "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR"; break;
    case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR: result = "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR"; break;
    }
    return result;
}

std::string to_string_VkCompositeAlphaFlagsKHR (VkCompositeAlphaFlagsKHR mode)
{
    std::string result;
    switch (mode) {
    case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR: result = "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR"; break;
    case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR: result = "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR"; break;
    case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR: result = "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR"; break;
    case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR: result = "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR"; break;
    }
    return result;
}

std::string to_string_VkFormatFeatureFlagBits (VkFormatFeatureFlagBits value)
{
    std::string result;
    switch (value) {
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT: result = "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT"; break;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT: result = "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT"; break;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT: result = "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT"; break;
    case VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT: result = "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT"; break;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT: result = "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT"; break;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT: result = "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT"; break;
    case VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT: result = "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT"; break;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT: result = "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT"; break;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT: result = "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT"; break;
    case VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT: result = "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT"; break;
    case VK_FORMAT_FEATURE_BLIT_SRC_BIT: result = "VK_FORMAT_FEATURE_BLIT_SRC_BIT"; break;
    case VK_FORMAT_FEATURE_BLIT_DST_BIT: result = "VK_FORMAT_FEATURE_BLIT_DST_BIT"; break;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT: result = "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT"; break;
    case VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR: result = "VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR"; break;
    case VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR: result = "VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR"; break;
    }
    return result;
}

std::string to_string_short_VkFormatFeatureFlagBits (VkFormatFeatureFlagBits value)
{
    std::string result;
    switch (value) {
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT: result = "SAMPLED_IMAGE"; break;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT: result = "STORAGE_IMAGE"; break;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT: result = "STORAGE_IMAGE_ATOMIC"; break;
    case VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT: result = "UNIFORM_TEXEL_BUFFER"; break;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT: result = "STORAGE_TEXEL_BUFFER"; break;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT: result = "STORAGE_TEXEL_BUFFER_ATOMIC"; break;
    case VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT: result = "VERTEX_BUFFER"; break;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT: result = "COLOR_ATTACHMENT"; break;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT: result = "COLOR_ATTACHMENT_BLEND"; break;
    case VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT: result = "DEPTH_STENCIL_ATTACHMENT"; break;
    case VK_FORMAT_FEATURE_BLIT_SRC_BIT: result = "BLIT_SRC"; break;
    case VK_FORMAT_FEATURE_BLIT_DST_BIT: result = "BLIT_DST"; break;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT: result = "SAMPLED_IMAGE_FILTER_LINEAR"; break;
    case VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR: result = "TRANSFER_SRC"; break;
    case VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR: result = "TRANSFER_DST"; break;
    }
    return result;
}

std::string to_string_VkImageType (VkImageType value)
{
    std::string result;
    switch (value) {
    case VK_IMAGE_TYPE_1D: result = "1D"; break;
    case VK_IMAGE_TYPE_2D: result = "2D"; break;
    case VK_IMAGE_TYPE_3D: result = "3D"; break;
    }
    return result;
}

std::string to_string_VkImageUsageFlagBits (VkImageUsageFlagBits value)
{
    std::string result;
    switch (value) {
    case VK_IMAGE_USAGE_TRANSFER_SRC_BIT: result = "TRANSFER_SRC"; break;
    case VK_IMAGE_USAGE_TRANSFER_DST_BIT: result = "TRANSFER_DST"; break;
    case VK_IMAGE_USAGE_SAMPLED_BIT: result = "SAMPLED"; break;
    case VK_IMAGE_USAGE_STORAGE_BIT: result = "STORAGE"; break;
    case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: result = "COLOR_ATTACHMENT"; break;
    case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: result = "DEPTH_STENCIL_ATTACHMENT"; break;
    case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT: result = "TRANSIENT_ATTACHMENT"; break;
    case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT: result = "INPUT_ATTACHMENT"; break;
    }
    return result;
}

std::string to_string_VkSampleCountFlags (VkSampleCountFlags value)
{
    std::string result;
    bool append = false;
    if (value & VK_SAMPLE_COUNT_1_BIT) {
        result += "1";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_2_BIT) {
        result += append ? " / " : "";
        result += "2";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_4_BIT) {
        result += append ? " / " : "";
        result += "4";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_8_BIT) {
        result += append ? " / " : "";
        result += "8";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_16_BIT) {
        result += append ? " / " : "";
        result += "16";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_32_BIT) {
        result += append ? " / " : "";
        result += "32";
        append = true;
    }
    if (value & VK_SAMPLE_COUNT_64_BIT) {
        result += append ? " / " : "";
        result += "64";
    }
    return result;
}

std::string to_string_VkMemoryPropertyFlags (VkMemoryPropertyFlags value) {
    std::vector<std::string> ss;
    if (value & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { ss.emplace_back ("DEVICE_LOCAL"); }
    if (value & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { ss.emplace_back ("HOST_VISIBLE"); }
    if (value & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { ss.emplace_back ("HOST_COHERENT"); }
    if (value & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { ss.emplace_back ("HOST_CACHED"); }
    if (value & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) { ss.emplace_back ("LAZILY_ALLOCATED"); }
    if (value & VK_MEMORY_PROPERTY_PROTECTED_BIT) { ss.emplace_back ("PROTECTED"); }
    std::string s = std::accumulate (std::begin (ss), std::end (ss), std::string (),
        [](std::string& x, std::string& s) {
            return x.empty () ? s : x + ", " + s;
        });

    return s;
}

std::string to_string_VkQueueFlags (VkQueueFlags value) {
    std::vector<std::string> ss;
    if (value & VK_QUEUE_GRAPHICS_BIT) { ss.emplace_back ("GRAPHICS"); }
    if (value & VK_QUEUE_COMPUTE_BIT) { ss.emplace_back ("COMPUTE"); }
    if (value & VK_QUEUE_TRANSFER_BIT) { ss.emplace_back ("TRANSFER"); }
    if (value & VK_QUEUE_SPARSE_BINDING_BIT) { ss.emplace_back ("SPARSE_BINDING"); }
    if (value & VK_QUEUE_PROTECTED_BIT) { ss.emplace_back ("PROTECTED"); }
    std::string s = std::accumulate (std::begin (ss), std::end (ss), std::string (),
        [](std::string& x, std::string& s) {
            return x.empty () ? s : x + ", " + s;
        });

    return s;
}

}

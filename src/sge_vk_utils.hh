// SGE-VK-UTILS
// ---------------------------------- //
// Helpers for working with Vulkan.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_logging.hh"

#if CONFIGURATION_FINAL
#define vk_assert(x) x
#else
#define vk_assert(x) { VkResult r=x; if (r != VK_SUCCESS) { std::cout << "vk_assert failed on: " << utils::to_string_VkResult (r) << " in " << __FILE__ << " at line " << __LINE__ << std::endl; } ASSERT (r == VK_SUCCESS); }
#endif

namespace sge::vk::utils {

uint32_t                choose_memory_type                              (VkPhysicalDevice, const VkMemoryRequirements&, VkMemoryPropertyFlags, VkMemoryPropertyFlags = 0);
VkSurfaceFormatKHR      choose_swapchain_surface_format                 (const std::vector<VkSurfaceFormatKHR>&);
VkPresentModeKHR        choose_swapchain_present_mode                   (const std::vector<VkPresentModeKHR>);
VkExtent2D              choose_swapchain_extent                         (const VkSurfaceCapabilitiesKHR&, const int, const int);
VkShaderModule          create_shader_module                            (VkDevice, const VkAllocationCallbacks*, const std::vector<uint8_t>&);
VkBool32                get_supported_depth_format                      (VkPhysicalDevice, VkFormat*);

void set_image_layout (
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

void set_image_layout (
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

inline bool equal(const VkViewport& l, const VkViewport& r) {
    return l.x == r.x
        && l.y == r.y
        && l.width == r.width
        && l.height == r.height
        && l.minDepth == r.minDepth
        && l.maxDepth == r.maxDepth;
}

inline bool equal(const VkExtent2D& l, const VkExtent2D& r) {
    return l.width == r.width && l.height == r.height;
}

std::string to_string_VkResult (VkResult);
std::string to_string_Version (uint32_t);
std::string to_string_VkPhysicalDeviceType (VkPhysicalDeviceType);
std::string to_string_VkFormat (VkFormat);
std::string to_string_VkColorSpaceKHR (VkColorSpaceKHR);
std::string to_string_VkPresentModeKHR (VkPresentModeKHR);
std::string to_string_VkSurfaceTransformFlagsKHR (VkSurfaceTransformFlagsKHR);
std::string to_string_VkCompositeAlphaFlagsKHR (VkCompositeAlphaFlagsKHR);
std::string to_string_VkFormatFeatureFlagBits (VkFormatFeatureFlagBits);
std::string to_string_VkFormatFeatureFlagBits (VkFormatFeatureFlagBits);
std::string to_string_VkImageType (VkImageType);
std::string to_string_VkImageUsageFlagBits (VkImageUsageFlagBits);
std::string to_string_VkSampleCountFlags (VkSampleCountFlags);
std::string to_string_VkMemoryPropertyFlags (VkMemoryPropertyFlags);
std::string to_string_VkQueueFlags (VkQueueFlags);


#include "sge_vk_utils.inl"

}

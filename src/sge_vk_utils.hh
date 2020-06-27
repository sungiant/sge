// SGE-VK-UTILS
// ---------------------------------- //
// Helpers for working with Vulkan.
// ---------------------------------- //

#pragma once

#include "sge.hh"

#if CONFIGURATION_FINAL
#define vk_assert(x) x
#else
#define vk_assert(x) { VkResult r=(x); ASSERT (r == VK_SUCCESS); }
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


#include "sge_vk_utils.inl"

}

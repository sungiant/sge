#include "sge_vk_presentation.hh"

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cassert>

#if TARGET_MACOSX
#include <MoltenVK/mvk_vulkan.h>
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

#if TARGET_WIN32
#include <windows.h>
#endif

#undef max

#include "sge_vk_context.hh"
#include "sge_vk_utils.hh"

namespace sge::vk {

presentation:: presentation (const struct context& context, const queue_identifier& qid
#if TARGET_WIN32
    , HINSTANCE hi, HWND hw
#elif TARGET_MACOSX
    , void* v
#elif TARGET_LINUX
    , xcb_connection_t* c, xcb_window_t w
#endif
)
    : context (context)
    , identifier (qid)
#if TARGET_WIN32
    , app_hinst (hi)
    , app_hwnd (hw)
#elif TARGET_MACOSX
    , app_view (v)
#elif TARGET_LINUX
    , app_connection (c)
    , app_window (w)
#endif
    , state ()
{}

void presentation::configure (const std::vector<queue_identifier>& external_queues_requiring_swapchain_access) {    
    auto& xs = state.queue_families_requiring_swapchain_access;

    xs.clear ();
    xs.emplace_back (identifier.family_index);

    for (auto& qid : external_queues_requiring_swapchain_access) {
        // make sure all queues neededing access to the swap chain are on the same physical device.
        // i don't think it is possible to give a queue on a different device direct access.
        assert (qid.physical_device == identifier.physical_device);
        if (std::find (xs.begin (), xs.end (), qid.family_index) == state.queue_families_requiring_swapchain_access.end ()) {
            xs.emplace_back (qid.family_index);
        }
    }

    std::sort (xs.begin (), xs.end ());
}

void presentation::create () {

    // semaphore
    auto semaphore_create_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_create_info, context.allocation_callbacks, &state.image_available));
    // surface
    create_surface ();
    // refreshable
    create_r ();
}

void presentation::refresh () {
    destroy_r ();
    create_r ();
}

void presentation::destroy () {
    // refreshable
    destroy_r ();
    // surface
    vkDestroySurfaceKHR (context.instance, state.surface, context.allocation_callbacks);
    state.surface = VK_NULL_HANDLE;
    // semaphore
    vkDestroySemaphore (context.logical_device, state.image_available, context.allocation_callbacks);
    state.image_available = VK_NULL_HANDLE;
}

void presentation::create_r () {

    vk_assert (vkGetPhysicalDeviceSurfaceCapabilitiesKHR (identifier.physical_device, state.surface, &state.surface_capabilities));

#if SGE_DEBUG_MODE
    std::cout << "Refreshing for surface: " <<
        "" << state.surface_capabilities.currentExtent.width << 
        " x " << state.surface_capabilities.currentExtent.height << "\n";
#endif

    create_swapchain ();
    create_image_views ();
    create_depth_stencil ();
    create_render_passes ();
    create_framebuffers ();
}

void presentation::destroy_r () {
    // frame buffers
    for (auto framebuffer : state.swapchain_frame_buffers) {
        vkDestroyFramebuffer (context.logical_device, framebuffer, context.allocation_callbacks);
    }
    state.swapchain_frame_buffers.clear ();

    // render passes
    vkDestroyRenderPass (context.logical_device, state.imgui_render_pass, context.allocation_callbacks);
    state.imgui_render_pass = VK_NULL_HANDLE;
    vkDestroyRenderPass (context.logical_device, state.fullscreen_render_pass, context.allocation_callbacks);
    state.fullscreen_render_pass = VK_NULL_HANDLE;

    vkDestroyImageView (context.logical_device, state.depth_stencil_view, context.allocation_callbacks);
    vkDestroyImage (context.logical_device, state.depth_stencil_image, context.allocation_callbacks);
    vkFreeMemory (context.logical_device, state.depth_stencil_memory, context.allocation_callbacks);

    // image views
    for (auto image_view : state.swapchain_image_views) {
        vkDestroyImageView (context.logical_device, image_view, context.allocation_callbacks);
    }
    state.swapchain_image_views.clear ();

    // swapchain
    vkDestroySwapchainKHR (context.logical_device, state.swapchain, context.allocation_callbacks);
    state.swapchain = VK_NULL_HANDLE;

}

std::variant<VkResult, image_index> presentation::next_image () {
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR (context.logical_device, state.swapchain, std::numeric_limits<uint64_t>::max (), state.image_available, VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_ERROR_SURFACE_LOST_KHR) {
        return result;
    }

    assert (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    return image_index;
}

void presentation::create_surface () {
    auto physical_device = identifier.physical_device;

#if TARGET_WIN32
    auto surface_create_info = utils::init_VkWin32SurfaceCreateInfoKHR (app_hinst, app_hwnd);
    vk_assert (vkCreateWin32SurfaceKHR (context.instance, &surface_create_info, context.allocation_callbacks, &state.surface));
#elif TARGET_MACOSX
    auto surface_create_info = utils::init_VkMacOSSurfaceCreateInfoMVK (const_cast<void*> (app_view));
    vk_assert (vkCreateMacOSSurfaceMVK (context.instance, &surface_create_info, context.allocation_callbacks, &state.surface));

    //{ // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateMetalSurfaceEXT.html
    //
    //    const MTKView* metal_view = static_cast<MTKView *> (app_view ());
    //    const CAMetalLayer* metal_layer = metal_view->layer;
    //    VkMetalSurfaceCreateInfoEXT surface_create_info = {
    //        VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
    //        nullptr,
    //        0,
    //        metal_layer
    //    };
    //    VkResult vkCreateMetalSurfaceEXT (instance (), &surface_create_info, context.allocation_callbacks, &state.surface);
    //    assert (result == VK_SUCCESS);
    //}
#elif TARGET_LINUX  
    auto surface_create_info = utils::init_VkXcbSurfaceCreateInfoKHR (app_connection, app_window);
    vk_assert (vkCreateXcbSurfaceKHR (context.instance, &surface_create_info, context.allocation_callbacks, &state.surface));
#else
#error
#endif

    uint32_t surface_format_count;
    vk_assert (vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, state.surface, &surface_format_count, nullptr));
    state.surface_formats.resize (surface_format_count);
    vk_assert (vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, state.surface, &surface_format_count, state.surface_formats.data ()));

    uint32_t present_mode_count;
    vk_assert (vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, state.surface, &present_mode_count, nullptr));
    state.present_modes.resize (present_mode_count);
    vk_assert (vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, state.surface, &present_mode_count, state.present_modes.data ()));
    vk_assert (vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, state.surface, &state.surface_capabilities));

    VkBool32 surface_supported;
    vk_assert (vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, identifier.family_index, state.surface, &surface_supported));
    assert (surface_supported);

}

void presentation::create_swapchain () {

    state.swapchain_surface_format = utils::choose_swapchain_surface_format (state.surface_formats);
    state.swapchain_present_mode = utils::choose_swapchain_present_mode (state.present_modes);
    state.swapchain_extent = utils::choose_swapchain_extent (
        state.surface_capabilities,
        state.surface_capabilities.currentExtent.width,
        state.surface_capabilities.currentExtent.height);

    uint32_t image_count = state.surface_capabilities.minImageCount + 1;
    if (state.surface_capabilities.maxImageCount > 0 && image_count > state.surface_capabilities.maxImageCount) {
        image_count = state.surface_capabilities.maxImageCount;
    }

    auto swap_chain_create_info = utils::init_VkSwapchainCreateInfoKHR();
    swap_chain_create_info.surface = state.surface;
    swap_chain_create_info.minImageCount = 3;
    swap_chain_create_info.imageFormat = state.swapchain_surface_format.format;
    swap_chain_create_info.imageColorSpace = state.swapchain_surface_format.colorSpace;
    swap_chain_create_info.imageExtent = state.swapchain_extent;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |  VK_IMAGE_USAGE_STORAGE_BIT;
    swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_create_info.preTransform = state.surface_capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = state.swapchain_present_mode;
    swap_chain_create_info.clipped = VK_TRUE;

    if (state.queue_families_requiring_swapchain_access.size () > 0) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = (uint32_t) state.queue_families_requiring_swapchain_access.size ();
        swap_chain_create_info.pQueueFamilyIndices = state.queue_families_requiring_swapchain_access.data ();
    }

    vk_assert (vkCreateSwapchainKHR (context.logical_device, &swap_chain_create_info, context.allocation_callbacks, &state.swapchain));

    uint32_t swapchain_image_count = 0;
    vk_assert (vkGetSwapchainImagesKHR (context.logical_device, state.swapchain, &swapchain_image_count, nullptr));
    state.swapchain_images.resize (swapchain_image_count);

    vk_assert (vkGetSwapchainImagesKHR (context.logical_device, state.swapchain, &swapchain_image_count, state.swapchain_images.data ()));
}


void presentation::create_image_views () {
    state.swapchain_image_views.resize (state.swapchain_images.size ());

    for (size_t i = 0; i < state.swapchain_images.size (); i++) {
      auto create_info = utils::init_VkImageViewCreateInfo ();
      create_info.image = state.swapchain_images[i];
      create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      create_info.format = state.swapchain_surface_format.format;
      create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      create_info.subresourceRange.baseMipLevel = 0;
      create_info.subresourceRange.levelCount = 1;
      create_info.subresourceRange.baseArrayLayer = 0;
      create_info.subresourceRange.layerCount = 1;

      vk_assert (vkCreateImageView (context.logical_device, &create_info, context.allocation_callbacks, &state.swapchain_image_views[i]));
    }
}

void presentation::create_render_passes () {

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pDepthStencilAttachment = &depthReference;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {};

    attachments[0].format = state.swapchain_surface_format.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkFormat depthFormat;
    VkBool32 validDepthFormat = utils::get_supported_depth_format (context.physical_device, &depthFormat);
    assert (validDepthFormat);
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto renderPassInfo = utils::init_VkRenderPassCreateInfo();
    renderPassInfo.attachmentCount = (uint32_t) attachments.size ();
    renderPassInfo.pAttachments = attachments.data ();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = (uint32_t) dependencies.size ();
    renderPassInfo.pDependencies = dependencies.data ();

    vk_assert (vkCreateRenderPass (context.logical_device, &renderPassInfo, context.allocation_callbacks, &state.fullscreen_render_pass));

    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;    // don't clear colour
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // do clear depth
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // do clear stencil
    vk_assert (vkCreateRenderPass (context.logical_device, &renderPassInfo, context.allocation_callbacks, &state.imgui_render_pass));
}

void presentation::create_depth_stencil () {

    VkFormat depthFormat;
    VkBool32 validDepthFormat = utils::get_supported_depth_format  (context.physical_device, &depthFormat);
    assert (validDepthFormat);

    auto imageCI = utils::init_VkImageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthFormat;
    imageCI.extent = { extent ().width, extent ().height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vk_assert (vkCreateImage (context.logical_device, &imageCI, context.allocation_callbacks, &state.depth_stencil_image));
    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements (context.logical_device, state.depth_stencil_image, &memReqs);

    auto alloc_info = utils::init_VkMemoryAllocateInfo ();
    alloc_info.allocationSize = memReqs.size;
    alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vk_assert (vkAllocateMemory (context.logical_device, &alloc_info, context.allocation_callbacks, &state.depth_stencil_memory));
    vk_assert (vkBindImageMemory (context.logical_device, state.depth_stencil_image, state.depth_stencil_memory, 0));

    auto imageViewCI = utils::init_VkImageViewCreateInfo ();;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = state.depth_stencil_image;
    imageViewCI.format = depthFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    vk_assert (vkCreateImageView (context.logical_device, &imageViewCI, context.allocation_callbacks, &state.depth_stencil_view));
}

void presentation::create_framebuffers () {

    state.swapchain_frame_buffers.resize (state.swapchain_image_views.size ());

    VkImageView attachments[2];
    attachments[1] = state.depth_stencil_view;

    auto framebufferInfo = utils::init_VkFramebufferCreateInfo ();
    framebufferInfo.renderPass = state.fullscreen_render_pass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = state.swapchain_extent.width;
    framebufferInfo.height = state.swapchain_extent.height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < state.swapchain_image_views.size (); i++) {
        attachments[0] = state.swapchain_image_views[i];
        vk_assert (vkCreateFramebuffer (context.logical_device, &framebufferInfo, context.allocation_callbacks, &state.swapchain_frame_buffers[i]));
    }
}

}

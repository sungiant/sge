#include "sge_vk_presentation.hh"

#include "sge_vk_context.hh"
#include "sge_utils.hh"

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
    , queue_id (qid)
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


presentation::~presentation () {
    assert (state.resource_status == 0);
};

void presentation::configure (const std::vector<queue_identifier>& external_queues_requiring_swapchain_access) {
    auto& xs = state.queue_families_requiring_swapchain_access;

    xs.clear ();
    xs.emplace_back (queue_id.family_index);

    for (auto& qid : external_queues_requiring_swapchain_access) {
        // make sure all queues neededing access to the swap chain are on the same physical device.
        // i don't think it is possible to give a queue on a different device direct access.
        assert (qid.physical_device == queue_id.physical_device);
        if (std::find (xs.begin (), xs.end (), qid.family_index) == state.queue_families_requiring_swapchain_access.end ()) {
            xs.emplace_back (qid.family_index);
        }
    }

    std::sort (xs.begin (), xs.end ());
}

presentation::surface_status presentation::check_surface_status () {
    const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR (queue_id.physical_device, state.surface.value, &state.surface.capabilities);

    if (result == VK_ERROR_SURFACE_LOST_KHR)
        return surface_status::LOST;

    if (result != VK_SUCCESS)
        return surface_status::FATAL;

    if (state.surface.capabilities.currentExtent.width == 0 || state.surface.capabilities.currentExtent.height == 0)
        return surface_status::ZERO;

    return surface_status::OK;
}

std::variant<presentation::swapchain_status, sge::vk::image_index> presentation::next_image () {
    uint32_t image_index;
    const VkResult result = vkAcquireNextImageKHR (context.logical_device, state.swapchain.value, std::numeric_limits<uint64_t>::max (), state.synchronisation.image_available, VK_NULL_HANDLE, &image_index);
    switch (result) {
        case VK_SUCCESS:                                        return image_index;
        case VK_SUBOPTIMAL_KHR:                                 return presentation::swapchain_status::SUBOPTIMAL;
        case VK_ERROR_OUT_OF_DATE_KHR:                          return presentation::swapchain_status::OUT_OF_DATE;
        case VK_ERROR_SURFACE_LOST_KHR:                         return presentation::swapchain_status::LOST;
        case VK_TIMEOUT:                                        // not expecting this as timeout is currently max ulong
        case VK_NOT_READY:
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_DEVICE_LOST:
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:      return presentation::swapchain_status::FATAL;
    }
}


//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_resources (resource_flags flags) {
    using namespace sge::utils;
    if (get_flag_at_mask (flags, SYNCHRONISATION)) { assert (!get_flag_at_mask (state.resource_status, SYNCHRONISATION)); create_synchronisation ();  set_flag_at_mask (state.resource_status, SYNCHRONISATION, true ); }
    if (get_flag_at_mask (flags, SURFACE))         { assert (!get_flag_at_mask (state.resource_status, SURFACE));         create_surface ();          set_flag_at_mask (state.resource_status, SURFACE,         true ); }
    if (get_flag_at_mask (flags, SWAPCHAIN))       { assert (!get_flag_at_mask (state.resource_status, SWAPCHAIN));       create_swapchain ();        set_flag_at_mask (state.resource_status, SWAPCHAIN,       true ); }
    if (get_flag_at_mask (flags, DEPTH_STENCIL))   { assert (!get_flag_at_mask (state.resource_status, DEPTH_STENCIL));   create_depth_stencil ();    set_flag_at_mask (state.resource_status, DEPTH_STENCIL,   true ); }
    if (get_flag_at_mask (flags, RENDER_PASSES))   { assert (!get_flag_at_mask (state.resource_status, RENDER_PASSES));   create_render_pass ();      set_flag_at_mask (state.resource_status, RENDER_PASSES,   true ); }
    if (get_flag_at_mask (flags, FRAMEBUFFERS))    { assert (!get_flag_at_mask (state.resource_status, FRAMEBUFFERS));    create_framebuffer ();      set_flag_at_mask (state.resource_status, FRAMEBUFFERS,    true ); }
}

void presentation::destroy_resources (resource_flags flags) {
    using namespace sge::utils;
    if (get_flag_at_mask (flags, FRAMEBUFFERS))    { assert ( get_flag_at_mask (state.resource_status, FRAMEBUFFERS));    destroy_framebuffer ();     set_flag_at_mask (state.resource_status, FRAMEBUFFERS,    false); }
    if (get_flag_at_mask (flags, RENDER_PASSES))   { assert ( get_flag_at_mask (state.resource_status, RENDER_PASSES));   destroy_render_pass ();     set_flag_at_mask (state.resource_status, RENDER_PASSES,   false); }
    if (get_flag_at_mask (flags, DEPTH_STENCIL))   { assert ( get_flag_at_mask (state.resource_status, DEPTH_STENCIL));   destroy_depth_stencil ();   set_flag_at_mask (state.resource_status, DEPTH_STENCIL,   false); }
    if (get_flag_at_mask (flags, SWAPCHAIN))       { assert ( get_flag_at_mask (state.resource_status, SWAPCHAIN));       destroy_swapchain ();       set_flag_at_mask (state.resource_status, SWAPCHAIN,       false); }
    if (get_flag_at_mask (flags, SURFACE))         { assert ( get_flag_at_mask (state.resource_status, SURFACE));         destroy_surface ();         set_flag_at_mask (state.resource_status, SURFACE,         false); }
    if (get_flag_at_mask (flags, SYNCHRONISATION)) { assert ( get_flag_at_mask (state.resource_status, SYNCHRONISATION)); destroy_synchronisation (); set_flag_at_mask (state.resource_status, SYNCHRONISATION, false); }
}

//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_synchronisation () {
    auto semaphore_create_info = utils::init_VkSemaphoreCreateInfo ();
    vk_assert (vkCreateSemaphore (context.logical_device, &semaphore_create_info, context.allocation_callbacks, &state.synchronisation.image_available));
}

void presentation::destroy_synchronisation () {
    vkDestroySemaphore (context.logical_device, state.synchronisation.image_available, context.allocation_callbacks);
    state.synchronisation.image_available = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_surface () {
    auto physical_device = queue_id.physical_device;

#if TARGET_WIN32
    auto surface_create_info = utils::init_VkWin32SurfaceCreateInfoKHR (app_hinst, app_hwnd);
    vk_assert (vkCreateWin32SurfaceKHR (context.instance, &surface_create_info, context.allocation_callbacks, &state.surface.value));
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
    //    VkResult vkCreateMetalSurfaceEXT (instance (), &surface_create_info, context.allocation_callbacks, &state.surface.value);
    //    assert (result == VK_SUCCESS);
    //}
#elif TARGET_LINUX
    auto surface_create_info = utils::init_VkXcbSurfaceCreateInfoKHR (const_cast<xcb_connection_t*>(app_connection), app_window);
    vk_assert (vkCreateXcbSurfaceKHR (context.instance, &surface_create_info, context.allocation_callbacks, &state.surface.value));
#else
#error
#endif

    uint32_t surface_format_count;
    vk_assert (vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, state.surface.value, &surface_format_count, nullptr));
    state.surface.formats.resize (surface_format_count);
    vk_assert (vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, state.surface.value, &surface_format_count, state.surface.formats.data ()));

    uint32_t present_mode_count;
    vk_assert (vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, state.surface.value, &present_mode_count, nullptr));
    state.surface.present_modes.resize (present_mode_count);
    vk_assert (vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, state.surface.value, &present_mode_count, state.surface.present_modes.data ()));
    vk_assert (vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, state.surface.value, &state.surface.capabilities));

    VkBool32 surface_supported;
    vk_assert (vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, queue_id.family_index, state.surface.value, &surface_supported));
    assert (surface_supported);
}

void presentation::destroy_surface () {
    vkDestroySurfaceKHR (context.instance, state.surface.value, context.allocation_callbacks);
    state.surface.value = VK_NULL_HANDLE;
}


//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_swapchain () {

    state.swapchain.surface_format = utils::choose_swapchain_surface_format (state.surface.formats);
    state.swapchain.present_mode = utils::choose_swapchain_present_mode (state.surface.present_modes);
    state.swapchain.extent = utils::choose_swapchain_extent (
        state.surface.capabilities,
        state.surface.capabilities.currentExtent.width,
        state.surface.capabilities.currentExtent.height);

    uint32_t image_count = state.surface.capabilities.minImageCount + 1;
    if (state.surface.capabilities.maxImageCount > 0 && image_count > state.surface.capabilities.maxImageCount) {
        image_count = state.surface.capabilities.maxImageCount;
    }

    auto swap_chain_create_info = utils::init_VkSwapchainCreateInfoKHR();
    swap_chain_create_info.surface = state.surface.value;
    swap_chain_create_info.minImageCount = 3;
    swap_chain_create_info.imageFormat = state.swapchain.surface_format.format;
    swap_chain_create_info.imageColorSpace = state.swapchain.surface_format.colorSpace;
    swap_chain_create_info.imageExtent = state.swapchain.extent;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |  VK_IMAGE_USAGE_STORAGE_BIT;
    swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_create_info.preTransform = state.surface.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = state.swapchain.present_mode;
    swap_chain_create_info.clipped = VK_TRUE;

    if (state.queue_families_requiring_swapchain_access.size () > 0) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = (uint32_t) state.queue_families_requiring_swapchain_access.size ();
        swap_chain_create_info.pQueueFamilyIndices = state.queue_families_requiring_swapchain_access.data ();
    }

    vk_assert (vkCreateSwapchainKHR (context.logical_device, &swap_chain_create_info, context.allocation_callbacks, &state.swapchain.value));
    uint32_t swapchain_image_count = 0;
    vk_assert (vkGetSwapchainImagesKHR (context.logical_device, state.swapchain.value, &swapchain_image_count, nullptr));
    state.swapchain.images.resize (swapchain_image_count);
    vk_assert (vkGetSwapchainImagesKHR (context.logical_device, state.swapchain.value, &swapchain_image_count, state.swapchain.images.data ()));

    state.swapchain.image_views.resize (state.swapchain.images.size ());

    for (size_t i = 0; i < state.swapchain.images.size (); i++) {
        auto create_info = utils::init_VkImageViewCreateInfo ();
        create_info.image = state.swapchain.images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = state.swapchain.surface_format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        vk_assert (vkCreateImageView (context.logical_device, &create_info, context.allocation_callbacks, &state.swapchain.image_views[i]));
    }
}

void presentation::destroy_swapchain () {
    for (auto image_view : state.swapchain.image_views) {
        vkDestroyImageView (context.logical_device, image_view, context.allocation_callbacks);
    }
    state.swapchain.image_views.clear ();

    vkDestroySwapchainKHR (context.logical_device, state.swapchain.value, context.allocation_callbacks);
    state.swapchain.value = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_render_pass () {

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

    attachments[0].format = state.swapchain.surface_format.format;
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

    vk_assert (vkCreateRenderPass (context.logical_device, &renderPassInfo, context.allocation_callbacks, &state.render_pass.canvas));

    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;    // don't clear colour
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // do clear depth
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // do clear stencil
    vk_assert (vkCreateRenderPass (context.logical_device, &renderPassInfo, context.allocation_callbacks, &state.render_pass.imgui));
}


void presentation::destroy_render_pass () {
    vkDestroyRenderPass (context.logical_device, state.render_pass.imgui, context.allocation_callbacks);
    state.render_pass.imgui = VK_NULL_HANDLE;
    vkDestroyRenderPass (context.logical_device, state.render_pass.canvas, context.allocation_callbacks);
    state.render_pass.canvas = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_depth_stencil () {

    VkFormat depth_format;
    VkBool32 valid_format = utils::get_supported_depth_format  (context.physical_device, &depth_format);
    assert (valid_format);

    auto image_create_info = utils::init_VkImageCreateInfo();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = { extent ().width, extent ().height, 1 };
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vk_assert (vkCreateImage (context.logical_device, &image_create_info, context.allocation_callbacks, &state.depth_stencil.image));
    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements (context.logical_device, state.depth_stencil.image, &memory_requirements);

    auto alloc_info = utils::init_VkMemoryAllocateInfo ();
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = utils::choose_memory_type (context.physical_device, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vk_assert (vkAllocateMemory (context.logical_device, &alloc_info, context.allocation_callbacks, &state.depth_stencil.memory));
    vk_assert (vkBindImageMemory (context.logical_device, state.depth_stencil.image, state.depth_stencil.memory, 0));

    auto image_view_create_info = utils::init_VkImageViewCreateInfo ();;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = state.depth_stencil.image;
    image_view_create_info.format = depth_format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) {
        image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    vk_assert (vkCreateImageView (context.logical_device, &image_view_create_info, context.allocation_callbacks, &state.depth_stencil.view));
}


void presentation::destroy_depth_stencil () {
    vkDestroyImageView (context.logical_device, state.depth_stencil.view, context.allocation_callbacks);
    state.depth_stencil.view = VK_NULL_HANDLE;
    vkDestroyImage (context.logical_device, state.depth_stencil.image, context.allocation_callbacks);
    state.depth_stencil.image = VK_NULL_HANDLE;
    vkFreeMemory (context.logical_device, state.depth_stencil.memory, context.allocation_callbacks);
    state.depth_stencil.memory = VK_NULL_HANDLE;

}

//--------------------------------------------------------------------------------------------------------------------//

void presentation::create_framebuffer () {

    state.frame_buffer.value.resize (state.swapchain.image_views.size ());

    VkImageView attachments[2];
    attachments[1] = state.depth_stencil.view;

    auto framebufferInfo = utils::init_VkFramebufferCreateInfo ();
    framebufferInfo.renderPass = state.render_pass.canvas;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = state.swapchain.extent.width;
    framebufferInfo.height = state.swapchain.extent.height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < state.swapchain.image_views.size (); i++) {
        attachments[0] = state.swapchain.image_views[i];
        vk_assert (vkCreateFramebuffer (context.logical_device, &framebufferInfo, context.allocation_callbacks, &state.frame_buffer.value[i]));
    }
}

void presentation::destroy_framebuffer () {
    for (auto framebuffer : state.frame_buffer.value) {
        vkDestroyFramebuffer (context.logical_device, framebuffer, context.allocation_callbacks);
    }
    state.frame_buffer.value.clear ();
}

}

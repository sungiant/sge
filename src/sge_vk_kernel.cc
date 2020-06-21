#include "sge_vk_kernel.hh"

#include <cassert>
#include <sstream>
#include <iostream>

#include "sge_vk_allocator.hh"
#include "sge_vk_types.hh"

namespace sge::vk {

const std::vector<const char*> required_instance_layers =
#if TARGET_MACOSX
    { /*"MoltenVK"*/ };
#else
    { "VK_LAYER_KHRONOS_validation" }; // todo: make this optional
#endif

const std::vector<const char*> required_instance_extensions =
#if TARGET_WIN32
    { "VK_KHR_device_group_creation", "VK_KHR_win32_surface", "VK_KHR_surface" };
#elif TARGET_MACOSX
    { "VK_EXT_debug_report", "VK_MVK_moltenvk", "VK_MVK_macos_surface", "VK_EXT_metal_surface", "VK_KHR_surface", "VK_KHR_device_group_creation" };
#elif TARGET_LINUX
    { "VK_KHR_xcb_surface", "VK_KHR_surface" };
#else
    {};
#endif

const std::vector<const char*> required_device_layers = { "VK_LAYER_RENDERDOC_Capture" };
const std::vector<const char*> required_device_extensions = { "VK_KHR_swapchain" };



static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report (VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    (void) flags; (void) object; (void) location; (void) messageCode; (void) pUserData; (void) pLayerPrefix; // Unused arguments
    fprintf (stderr, "[vulkan] ObjectType: %i\n\t* Message: %s\n", objectType, pMessage);
    return VK_FALSE;
}

//--------------------------------------------------------------------------------------------------------------------//

kernel::kernel ()
#if SGE_VK_USE_CUSTOM_ALLOCATOR
    : custom_allocator (std::make_unique<allocator> ())
    , custom_allocator_callbacks (*custom_allocator.get ())
#endif
{
}

const context& kernel::primary_context () const {
    return state.contexts.front ();
}

queue_identifier kernel::primary_work_queue () const {
    queue_identifier queue_identifier = {};
    queue_identifier.physical_device = primary_context ().physical_device;
    queue_identifier.family_index = 0;
    queue_identifier.number = 0;
    return queue_identifier;
}

VkQueue kernel::get_queue (queue_identifier id) const {
    assert (primary_context ().physical_device == id.physical_device); // double check that the queue is associated with the primary physical device, todo: support multidevice later.
    return state.logical_device_info.at (get_logical_device (id.physical_device)).queues.at (id.family_index)[id.number];
}


VkDevice kernel::get_logical_device (VkPhysicalDevice physical_device) const {
    assert (state.device_map_inv.find (physical_device) != state.device_map_inv.end ());
    return state.device_map_inv.at (physical_device);
}

VkPhysicalDevice kernel::get_physical_device (VkDevice logical_device) const {
    assert (state.device_map.find (logical_device) != state.device_map.end ());
    return state.device_map.at (logical_device);
}

VkAllocationCallbacks* kernel::allocation_callbacks () const {
    if (custom_allocator_callbacks.has_value ()) {
        return const_cast<VkAllocationCallbacks*>(&custom_allocator_callbacks.value ());
    }
    return nullptr;
}

void kernel::append_debug_fns (std::vector<std::function<void ()>>& debug_fns) {
    if (custom_allocator.get ()) {
        debug_fns.emplace_back ([this]() { custom_allocator->debug_ui (); });
    }
}


//--------------------------------------------------------------------------------------------------------------------//

void kernel::create () {
    create_instance ();
    get_physical_devices ();
    create_logical_devices ();

    // now copy data into contexts structure.
    // todo: simplify and remove duplication

    state.contexts.clear ();
    for (int i = 0; i < state.physical_devices.size (); ++i) {
        state.contexts.emplace_back (context (
            allocation_callbacks (),
            state.instance,
            state.physical_devices[i],
            get_logical_device (state.physical_devices[i]),
            state.physical_device_info[state.physical_devices[i]],
            state.logical_device_info[get_logical_device (state.physical_devices[i])],
            state.logical_device_default_command_pool[get_logical_device (state.physical_devices[i])]
        ));
    }
}

void kernel::destroy () {
    for (auto kvp : state.logical_device_info) {
        vkDestroyCommandPool (kvp.first, state.logical_device_default_command_pool[kvp.first], allocation_callbacks ());
        vkDestroyDevice (kvp.first, allocation_callbacks ());
    }
    state.logical_device_info.clear ();
    state.physical_device_info.clear ();
    state.physical_devices.clear ();
    state.device_map.clear ();
    state.device_map_inv.clear ();
#if TARGET_MACOSX
    if (std::find (required_instance_extensions.begin (), required_instance_extensions.end (), "VK_EXT_debug_report") != required_instance_extensions.end ()) {
        vkDestroyDebugReportCallbackEXT (state.instance, state.debug_report_callback, allocation_callbacks ());
        state.debug_report_callback = VK_NULL_HANDLE;
    }
#endif
    vkDestroyInstance (state.instance, allocation_callbacks ());
    state.instance = VK_NULL_HANDLE;
}

void kernel::create_instance () {
    const auto app_info = utils::init_VkApplicationInfo ("SGE App");
    const auto instance_create_info = utils::init_VkInstanceCreateInfo (&app_info, required_instance_layers, required_instance_extensions);
    vk_assert (vkCreateInstance (&instance_create_info, allocation_callbacks (), &state.instance));


#if TARGET_MACOSX
    if (std::find (required_instance_extensions.begin (), required_instance_extensions.end (), "VK_EXT_debug_report") != required_instance_extensions.end ()) {
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr (state.instance, "vkCreateDebugReportCallbackEXT");
        assert (vkCreateDebugReportCallbackEXT);

        auto debug_report_ci = utils::init_VkDebugReportCallbackCreateInfoEXT (debug_report);
        vk_assert (vkCreateDebugReportCallbackEXT (state.instance, &debug_report_ci, allocation_callbacks (), &state.debug_report_callback));
    }

    MVKConfiguration config {};
    size_t sz = sizeof (MVKConfiguration);
    vkGetMoltenVKConfigurationMVK (state.instance, &config, &sz);
    config.debugMode = false;
#if SGE_PROFILING_MODE
    config.performanceTracking = true;
    config.performanceLoggingFrameCount = true;
#endif
    config.switchSystemGPU = true;
    vkSetMoltenVKConfigurationMVK (state.instance, &config, &sz);
#endif
}


void kernel::get_physical_devices () {

    // get groups
    uint32_t deviceGroupCount = 0;
    vkEnumeratePhysicalDeviceGroups (state.instance, &deviceGroupCount, nullptr);
    state.device_group_properties.resize (deviceGroupCount);
    vkEnumeratePhysicalDeviceGroups (state.instance, &deviceGroupCount, state.device_group_properties.data ());

    // get physical devices
    uint32_t physical_device_count = 0;
    vk_assert (vkEnumeratePhysicalDevices (state.instance, &physical_device_count, nullptr));
    state.physical_devices.resize (physical_device_count);
    vk_assert (vkEnumeratePhysicalDevices (state.instance, &physical_device_count, state.physical_devices.data ()));

    // get info
    for (auto physical_device : state.physical_devices) {
        
        state.physical_device_info[physical_device] = vk::physical_device_info {};
        
        VkPhysicalDeviceProperties physical_device_properties = {};
        vkGetPhysicalDeviceProperties (physical_device, &physical_device_properties);
        
        state.physical_device_info[physical_device].name = physical_device_properties.deviceName;
        state.physical_device_info[physical_device].driver_version = physical_device_properties.driverVersion;
        state.physical_device_info[physical_device].vulkan_api_version = physical_device_properties.apiVersion;

        uint32_t queue_family_properties_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_properties_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties (queue_family_properties_count);
        vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_properties_count, &queue_family_properties[0]);

        state.physical_device_info[physical_device].queue_families.resize (queue_family_properties_count);

        for (uint32_t i = 0; i < queue_family_properties_count; ++i) {
            auto& queue_family = state.physical_device_info[physical_device].queue_families[i];
            queue_family.count = queue_family_properties[i].queueCount;
            queue_family.flags = queue_family_properties[i].queueFlags;
            queue_family.index = i;


            // todo: move this logic to sge_vk_presentation
            VkBool32 can_present = false;
#if TARGET_WIN32
            can_present = vkGetPhysicalDeviceWin32PresentationSupportKHR (physical_device, i);
#elif TARGET_MACOSX
            MVKPhysicalDeviceMetalFeatures metal_features = {};
            size_t sz = sizeof (MVKPhysicalDeviceMetalFeatures);
            vkGetPhysicalDeviceMetalFeaturesMVK (physical_device, &metal_features, &sz);
            // not 100% sure on how to determine if a particular physical device supports presentation with MoltenVK...
            // try this for now:
            can_present = metal_features.minSwapchainImageCount > 0 && metal_features.maxSwapchainImageCount > 0;

#elif TARGET_LINUX
            can_present = true; // vkGetPhysicalDeviceXcbPresentationSupportKHR (physical_device, i, app_connection, app_window);
#else
            can_present = false;
#endif
            queue_family.can_present = can_present;

        }

        std::cout << "vulkan api version: " << VK_VERSION_MAJOR (physical_device_properties.apiVersion) << "." << VK_VERSION_MINOR (physical_device_properties.apiVersion) << "." << VK_VERSION_PATCH (physical_device_properties.apiVersion) << "\n";
        std::cout << "vulkan driver version: " << VK_VERSION_MAJOR (physical_device_properties.driverVersion) << "." << VK_VERSION_MINOR (physical_device_properties.driverVersion) << "." << VK_VERSION_PATCH (physical_device_properties.driverVersion) << "\n";
    }
}

void kernel::create_logical_devices () {

    for (auto physical_device : state.physical_devices) {

        VkDevice logical_device;
        auto& physical_device_info = state.physical_device_info[physical_device];

        std::vector<std::vector<float>> queue_priorities (physical_device_info.queue_families.size ());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos (physical_device_info.queue_families.size ());

        for (int i = 0; i < physical_device_info.queue_families.size (); ++i) {
            queue_priorities[i].resize (physical_device_info.queue_families[i].count);
            queue_create_infos[i] = utils::init_VkDeviceQueueCreateInfo (physical_device_info.queue_families[i].index, physical_device_info.queue_families[i].count, queue_priorities[i]);
        }

        VkPhysicalDeviceFeatures features = {};

        vkGetPhysicalDeviceFeatures (physical_device, &features);

#if !TARGET_MACOSX
        features.wideLines = true;
#endif
        auto device_create_info = utils::init_VkDeviceCreateInfo (queue_create_infos, required_device_layers, required_device_extensions);

        vk_assert (vkCreateDevice (physical_device, &device_create_info, allocation_callbacks (), &logical_device));

        state.device_map[logical_device] = physical_device;
        state.device_map_inv[physical_device] = logical_device;

        state.logical_device_info[logical_device] = vk::logical_device_info {};

        for (auto& queue_family : physical_device_info.queue_families) {
            state.logical_device_info[logical_device].queues[queue_family.index] = std::vector<VkQueue>(queue_family.count);

            for (uint32_t j = 0; j < queue_family.count; ++j) {
                vkGetDeviceQueue (logical_device, queue_family.index, j, &state.logical_device_info[logical_device].queues[queue_family.index][j]);
            }
        }

        auto cmdPoolInfo = utils::init_VkCommandPoolCreateInfo (0);
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkCommandPool default_command_pool;
        vk_assert (vkCreateCommandPool (logical_device, &cmdPoolInfo, allocation_callbacks (), &default_command_pool));
        state.logical_device_default_command_pool[logical_device] = default_command_pool;
    }

}
}

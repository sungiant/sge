#include "sge_vk_logging.hh"
#include "sge_vk_utils.hh"

namespace sge::vk::logging {

void cout_env_vars () {

    if(const char* env_p = std::getenv("VULKAN_SDK"))
        std::cout << "VULKAN_SDK: " << env_p << '\n';

    if(const char* env_p = std::getenv("DYLD_LIBRARY_PATH"))
        std::cout << "DYLD_LIBRARY_PATH: " << env_p << '\n';

    if(const char* env_p = std::getenv("VK_LAYER_PATH"))
        std::cout << "VK_LAYER_PATH: " << env_p << '\n';

    if(const char* env_p = std::getenv("VK_ICD_FILENAMES"))
        std::cout << "VK_ICD_FILENAMES: " << env_p << '\n';
}

void cout_layer_properties () {
    uint32_t instance_layer_count;
    VkResult result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
    std::cout << " ! " <<  instance_layer_count << " layers found!\n";
    if (instance_layer_count > 0) {
        std::vector<VkLayerProperties> instance_layers (instance_layer_count);
        result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
        for (int i = 0; i < instance_layer_count; ++i) {
            std::cout << " ! * " <<  instance_layers[i].layerName << "\n";
        }
    }
    std::cout << "\n";
}

void cout_device_layer_properties (VkPhysicalDevice physical_device) {
    uint32_t device_layer_count;
    VkResult result = vkEnumerateDeviceLayerProperties(physical_device, &device_layer_count, nullptr);
    std::cout << " ! " <<  device_layer_count << " device layers found!\n";
    if (device_layer_count > 0) {
        std::vector<VkLayerProperties> device_layers (device_layer_count);
        result = vkEnumerateInstanceLayerProperties(&device_layer_count, device_layers.data());
        for (int i = 0; i < device_layer_count; ++i) {
            std::cout << " ! * " <<  device_layers[i].layerName << "\n";
        }
    }
    std::cout << "\n";
}

void cout_instance_extension_properties () {
    uint32_t instance_extension_count;

    vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);

    std::cout << " ! " <<  instance_extension_count << " instance extensions found!\n";

    if (instance_extension_count > 0) {
        std::vector<VkExtensionProperties> instance_extension_properties (instance_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extension_properties.data());
        for (int i = 0; i < instance_extension_count; ++i) {
            std::cout << " ! * " << instance_extension_properties[i].extensionName << "\n";
        }
    }
    std::cout << "\n";
}

void cout_device_extension_properties (VkPhysicalDevice physical_device) {
    uint32_t device_extension_count;

    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);

    std::cout << " * " << device_extension_count << " device extensions found!\n";

    if (device_extension_count > 0) {
        std::vector<VkExtensionProperties> device_extension_properties (device_extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device,nullptr, &device_extension_count, device_extension_properties.data());
        for (int i = 0; i < device_extension_count; ++i) {
            std::cout << "   - " << device_extension_properties[i].extensionName << "\n";
        }
    }
    std::cout << "\n";
}


void cout_physical_device_properties(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    std::cout << "Physical-device properties" << "\n";
    std::cout << " * Device name: " << properties.deviceName << "\n";
    std::cout << " * Device type: ";
    switch (properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: std::cout << "Integrated GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: std::cout << "Discrete GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: std::cout << "Virtual GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU: std::cout << "CPU"; break;
    default: std::cout << "UNKNOWN"; break;
    }
    std::cout << "\n\n";
}

void cout_physical_device_features(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    std::cout << "Physical-device features" << "\n";
    std::cout << " * Geometry shaders: " << (features.geometryShader ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << " * Tesselation shaders: " << (features.tessellationShader ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << " * Wide lines: " << (features.wideLines ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "\n";
}

void cout_physical_device_memory_properties(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    std::cout << "Physical-device memory properties" << "\n";

    std::unordered_map<uint32_t, std::vector <std::pair <uint32_t, VkMemoryPropertyFlags>>> heap_properties;

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        auto& memory_type = memory_properties.memoryTypes[i];

        if (heap_properties.find(memory_type.heapIndex) == heap_properties.end())
        {
            heap_properties[memory_type.heapIndex] = std::vector <std::pair <uint32_t, VkMemoryPropertyFlags>>();
        }

        heap_properties[memory_type.heapIndex].emplace_back(std::pair<uint32_t, VkMemoryPropertyFlags>(i, memory_type.propertyFlags));
    }



    std::cout << " * Memory type count: " << memory_properties.memoryTypeCount << "\n";
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        std::cout << "   #" << i << ": " << utils::to_string_VkMemoryPropertyFlags(memory_properties.memoryTypes[i].propertyFlags) << "\n";
    }
    std::cout << " * Memory heap count: " << memory_properties.memoryHeapCount << "\n";

    for (uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i) {

        std::cout << "   #" << i << ": " << memory_properties.memoryHeaps[i].size / 1024 / 1024 << "mb" << "\n";
        std::cout << "     - Supported Memory Types: " << "\n";

        for (int k = 0; k < heap_properties[i].size(); ++k)
        {
            std::cout << "       #" << heap_properties[i][k].first << " (" << utils::to_string_VkMemoryPropertyFlags(heap_properties[i][k].second) << ")" << "\n";
        }

    }
    std::cout << "\n";
}

#if TARGET_MACOSX
void cout_mvk_physical_device_properties (VkPhysicalDevice physical_device) {

    MVKPhysicalDeviceMetalFeatures metal_features = {};
    size_t sz = sizeof(MVKPhysicalDeviceMetalFeatures);
    vkGetPhysicalDeviceMetalFeaturesMVK(physical_device, &metal_features, &sz);

    std::cout << "Metal Shading Language Version: " << metal_features.mslVersion << "\n";
}
#endif

void cout_physical_device_queue_family_properties(VkPhysicalDevice physical_device) {
    uint32_t queue_family_properties_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, &queue_family_properties[0]);


    std::cout << "Physical-device queue family properties" << "\n";

    for (uint32_t i = 0; i < queue_family_properties_count; ++i) {

        std::cout << " * Queue family #" << i << "\n";

        VkBool32 canPresent = false;
#if TARGET_WIN32 && !HEADLESS
        canPresent = vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, i);
#endif

        std::string s = utils::to_string_VkQueueFlags(queue_family_properties[i].queueFlags);

#if !HEADLESS
        std::cout << "   - Presentable: " << canPresent << "\n";
#endif

        std::cout << "   - Capabilities: " << s << "\n";
        std::cout << "   - Queue count: " << queue_family_properties[i].queueCount << "\n";

        VkExtent3D vec3 = queue_family_properties[i].minImageTransferGranularity;
        std::cout << "   - Min image transfer granularity: (w:" << vec3.width << ", h:" << vec3.height << ", d:" << vec3.depth << ")\n";
    }
    std::cout << "\n";
}

void cout_physical_device_format_properties(VkPhysicalDevice physical_device, VkFormat format) {
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
    std::cout << "Physical-device format properties for " << utils::to_string_VkFormat(format) << "\n";
    std::cout << " * Linear tiling: " << (format_properties.linearTilingFeatures ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << " * Optimal tiling: " << (format_properties.optimalTilingFeatures ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << " * Buffer: " << (format_properties.bufferFeatures ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "\n";
}


#if !HEADLESS && TARGET_MACOSX
void cout_physical_device_metal_features (const MVKPhysicalDeviceMetalFeatures& metal_features) {
    std::cout << "  $ Physical device metal features (via MoltenVK): " << "\n";
    std::cout << "  - mslVersion: " << metal_features.mslVersion << "\n";
    std::cout << "  - Metal Shading Language Version: " << metal_features.mslVersion << "\n";
    std::cout << "  - Indirect drawing: " << (metal_features.indirectDrawing ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - baseVertexInstanceDrawing: " << (metal_features.baseVertexInstanceDrawing ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    //std::cout << "  - dynamicMTLBuffers: " << (metal_features.dynamicMTLBuffers ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - shaderSpecialization: " << (metal_features.baseVertexInstanceDrawing ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - ioSurfaces: " << (metal_features.ioSurfaces ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - texelBuffers: " << (metal_features.texelBuffers ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - layeredRendering: " << (metal_features.layeredRendering ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - presentModeImmediate: " << (metal_features.presentModeImmediate ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - stencilViews: " << (metal_features.stencilViews ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - multisampleArrayTextures: " << (metal_features.multisampleArrayTextures ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - samplerClampToBorder: " << (metal_features.samplerClampToBorder ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - maxTextureDimension: " << metal_features.maxTextureDimension << "\n";
    std::cout << "  - maxPerStageBufferCount: " << metal_features.maxPerStageBufferCount << "\n";
    std::cout << "  - maxPerStageTextureCount: " << metal_features.maxPerStageTextureCount << "\n";
    std::cout << "  - maxPerStageSamplerCount: " << metal_features.maxPerStageSamplerCount << "\n";
    std::cout << "  - maxMTLBufferSize: " << metal_features.maxMTLBufferSize / 1024 / 1024 << "mb" << "\n";
    std::cout << "  - mtlBufferAlignment: " << metal_features.mtlBufferAlignment << "\n";
    std::cout << "  - maxQueryBufferSize: " << metal_features.maxQueryBufferSize / 1024 / 1024 << "mb" << "\n";
    std::cout << "  - mtlCopyBufferAlignment: " << metal_features.mtlCopyBufferAlignment << "\n";
    std::cout << "  - supportedSampleCounts: " << metal_features.supportedSampleCounts << "\n";
    std::cout << "  - minSwapchainImageCount: " << metal_features.minSwapchainImageCount << "\n";
    std::cout << "  - maxSwapchainImageCount: " << metal_features.maxSwapchainImageCount << "\n";
    std::cout << "  - combinedStoreResolveAction: " << (metal_features.combinedStoreResolveAction ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - arrayOfTextures: " << (metal_features.arrayOfTextures ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - arrayOfSamplers: " << (metal_features.arrayOfSamplers ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - mslVersionEnum: " << metal_features.mslVersionEnum << "\n";
    std::cout << "  - depthSampleCompare: " << (metal_features.depthSampleCompare ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - events: " << (metal_features.events ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - memoryBarriers: " << (metal_features.memoryBarriers ? "SUPPORTED" : "UNSUPPORTED") << "\n";
    std::cout << "  - multisampleLayeredRendering: " << (metal_features.multisampleLayeredRendering ? "SUPPORTED" : "UNSUPPORTED") << "\n";

}
#endif

void cout_physical_device_groups(VkInstance instance) {

    uint32_t device_group_count;

    vkEnumeratePhysicalDeviceGroups(instance, &device_group_count, nullptr);

    std::cout << " * " << device_group_count << " device groups found!\n";

    if (device_group_count > 0) {
        std::vector<VkPhysicalDeviceGroupProperties> device_group_properties(device_group_count);
        vkEnumeratePhysicalDeviceGroups(instance, &device_group_count, device_group_properties.data());
        for (int i = 0; i < device_group_count; ++i) {
            std::cout << "   - " << device_group_properties[i].sType << "\n";
            std::cout << "   - " << device_group_properties[i].physicalDeviceCount << "\n";
        }
    }
    std::cout << "\n";
}



}

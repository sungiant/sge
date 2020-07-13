#pragma once

#include "sge.hh"


namespace sge::vk::logging {

void cout_env_vars ();
void cout_layer_properties ();
void cout_device_layer_properties (VkPhysicalDevice physical_device);
void cout_instance_extension_properties ();
void cout_device_extension_properties (VkPhysicalDevice physical_device);

void cout_physical_device_properties(VkPhysicalDevice physical_device);
void cout_physical_device_features(VkPhysicalDevice physical_device);
void cout_physical_device_memory_properties(VkPhysicalDevice physical_device);
void cout_physical_device_queue_family_properties(VkPhysicalDevice physical_device);

void cout_physical_device_format_properties(VkPhysicalDevice physical_device, VkFormat format);

void cout_physical_device_groups(VkInstance instance);

#if TARGET_MACOSX
void cout_physical_device_metal_features (const MVKPhysicalDeviceMetalFeatures& metal_features);
#endif

}

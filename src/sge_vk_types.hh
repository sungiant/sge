// SGE-VK-TYPES
// ---------------------------------- //
// Common graphics types / helpers.
// ---------------------------------- //

#pragma once

#include "sge.hh"

namespace sge::vk {

typedef uint32_t queue_family_index;
typedef uint32_t queue_number;
typedef uint32_t queue_flags;
typedef uint32_t queue_count;
typedef uint32_t image_index;

struct physical_device_queue_family_info {
    queue_family_index              index = 0;
    queue_flags                     flags = 0;
    queue_count                     count = 0; // num supported queues in queue family
    bool                            can_present = false;

    bool supports_gfx               () { return flags & VK_QUEUE_GRAPHICS_BIT; }
    bool supports_compute           () { return flags & VK_QUEUE_COMPUTE_BIT; }
    bool supports_transfer          () { return flags & VK_QUEUE_TRANSFER_BIT; }
    bool supports_sparse_mem        () { return flags & VK_QUEUE_SPARSE_BINDING_BIT; }
    bool supports_protected_mem     () { return flags & VK_QUEUE_PROTECTED_BIT; }
};

struct physical_device_info {
    std::string name;
    std::string driver_version;
    std::string vulkan_api_version;
    std::vector<physical_device_queue_family_info> queue_families;
};

struct logical_device_info {
    std::unordered_map<queue_family_index, std::vector<VkQueue> > queues;

};

struct queue_identifier {

    VkPhysicalDevice        physical_device = VK_NULL_HANDLE;
    queue_family_index      family_index = 0;
    queue_number            number = 0;

    bool operator == (const queue_identifier& other) const { return physical_device == other.physical_device && family_index == other.family_index && number == other.number; }
    bool operator != (const queue_identifier& other) const { return !(*this == other); }

};

}

#pragma once

// SGE-VK-BUFFER
// -------------------------------------
// Vulkan buffer helper type.
// -------------------------------------

#include <cassert>
#include <cstring>

#include <vulkan/vulkan.h>

#include "sge_vk_utils.hh"

namespace sge::vk {

struct device_buffer {
    VkDevice device = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor = {};
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void* mapped = nullptr;

    VkBufferUsageFlags usage_flags;
    VkMemoryPropertyFlags memory_property_flags;

    void map (VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
        vk_assert (vkMapMemory (device, memory, offset, size, 0, &mapped));
    }

    void unmap () {
        if (mapped) {
            vkUnmapMemory (device, memory);
            mapped = nullptr;
        }
    }

    void bind (VkDeviceSize offset = 0) {
        vk_assert (vkBindBufferMemory (device, buffer, memory, offset));
    }

    void setup_descriptor (VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
        descriptor.offset = offset;
        descriptor.buffer = buffer;
        descriptor.range = size;
    }

    void copy (void* data, VkDeviceSize size) {
        assert (mapped);
        std::memcpy (mapped, data, size);
    }

    void flush (VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
        auto mapped_range = utils::init_VkMappedMemoryRange ();
        mapped_range.memory = memory;
        mapped_range.offset = offset;
        mapped_range.size = size;
        vk_assert (vkFlushMappedMemoryRanges (device, 1, &mapped_range));
    }

    void invalidate (VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
        auto mapped_range = utils::init_VkMappedMemoryRange ();
        mapped_range.memory = memory;
        mapped_range.offset = offset;
        mapped_range.size = size;
        vk_assert (vkInvalidateMappedMemoryRanges (device, 1, &mapped_range));
    }

    void destroy (const VkAllocationCallbacks* ac) {
        if (buffer) {
            vkDestroyBuffer (device, buffer, ac);
        }
        if (memory) {
            vkFreeMemory (device, memory, ac);
        }
    }
};

}

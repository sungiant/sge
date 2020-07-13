// SGE-VK-CONTEXT
// ---------------------------------- //
// Useful view of aggregated contexual
// info about a given Vulkan workgroup.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_buffer.hh"

namespace sge::vk {

typedef uint32_t queue_family_index;
typedef uint32_t queue_number;
typedef uint32_t image_index;

struct queue_identifier {

    VkPhysicalDevice        physical_device = VK_NULL_HANDLE;
    queue_family_index      family_index = 0;
    queue_number            number = 0;

    bool operator == (const queue_identifier& other) const { return physical_device == other.physical_device && family_index == other.family_index && number == other.number; }
    bool operator != (const queue_identifier& other) const { return !(*this == other); }

};

struct queue_family_info {
    const uint32_t                      index = 0;
    const VkQueueFlags                  flags = 0;
    const uint32_t                      count = 0; // num supported queues in queue family
    const bool                          can_present = false;

    bool supports_gfx () const { return flags & VK_QUEUE_GRAPHICS_BIT; }
    bool supports_compute () const { return flags & VK_QUEUE_COMPUTE_BIT; }
    bool supports_transfer () const { return flags & VK_QUEUE_TRANSFER_BIT; }
    bool supports_sparse_mem () const { return flags & VK_QUEUE_SPARSE_BINDING_BIT; }
    bool supports_protected_mem () const { return flags & VK_QUEUE_PROTECTED_BIT; }

    uint32_t num_supported_operations () const { return (uint32_t)supports_gfx () + (uint32_t)supports_compute () + (uint32_t)supports_transfer () + (uint32_t)supports_sparse_mem () + (uint32_t)supports_protected_mem (); }
};

struct physical_device_info {
    const std::string name;
    const uint32_t driver_version;
    const uint32_t vulkan_api_version;
    const std::vector<queue_family_info> queue_families;

    const queue_family_index best_queue_family_for (VkQueueFlags required_flags) const {
        uint32_t choice = 0;
        uint32_t best_minimum = std::numeric_limits<int>::max ();
        for (int i = 0; i < queue_families.size (); ++i) {
            if ((queue_families[i].flags & required_flags) == required_flags) {
                const int n = queue_families[i].num_supported_operations ();
                if (n < best_minimum) {
                    best_minimum = n;
                    choice = i;
                }
            }
        }
        return choice;
    }
};

struct logical_device_info {
    std::unordered_map<queue_family_index, std::vector<VkQueue>> queues;
    std::unordered_map<queue_family_index, VkCommandPool> default_command_pools;
};


struct context {

    const VkAllocationCallbacks const*  allocation_callbacks;
    const VkInstance                    instance;
    const VkPhysicalDevice              physical_device;
    const VkDevice                      logical_device;
    const physical_device_info&         physical_device_info;
    logical_device_info&          logical_device_info;

    context (
        const VkAllocationCallbacks const* z_allocation_callbacks,
        const VkInstance z_instance,
        const VkPhysicalDevice z_physical_device,
        const VkDevice z_logical_device,
        const struct physical_device_info& z_physical_device_info,
        struct logical_device_info& z_logical_device_info)
        : allocation_callbacks (z_allocation_callbacks)
        , instance (z_instance)
        , physical_device (z_physical_device)
        , logical_device (z_logical_device)
        , physical_device_info (z_physical_device_info)
        , logical_device_info (z_logical_device_info)
    {}

    VkQueue get_queue (const queue_identifier& id) const {
        assert (physical_device == id.physical_device); // double check that the queue is associated with this context's physical device.
        return logical_device_info.queues.at (id.family_index)[id.number];
    }

    void create_buffer (
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        VkBuffer* buffer,
        VkDeviceSize size,
        VkDeviceMemory* memory,
        void* data = nullptr) const {

        auto bufferCreateInfo = utils::init_VkBufferCreateInfo (usage_flags, size);
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vk_assert (vkCreateBuffer (logical_device, &bufferCreateInfo, allocation_callbacks, buffer));

        VkMemoryRequirements memReqs;
        auto alloc_info = utils::init_VkMemoryAllocateInfo ();
        vkGetBufferMemoryRequirements (logical_device, *buffer, &memReqs);
        alloc_info.allocationSize = memReqs.size;
        alloc_info.memoryTypeIndex = utils::choose_memory_type (physical_device, memReqs, memory_property_flags);
        vk_assert (vkAllocateMemory (logical_device, &alloc_info, allocation_callbacks, memory));

        if (data != nullptr) {
            void* mapped;
            vk_assert (vkMapMemory (logical_device, *memory, 0, size, 0, &mapped));
            memcpy (mapped, data, size);
            if ((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            {
                VkMappedMemoryRange mappedRange = utils::init_VkMappedMemoryRange ();
                mappedRange.memory = *memory;
                mappedRange.offset = 0;
                mappedRange.size = size;
                vkFlushMappedMemoryRanges (logical_device, 1, &mappedRange);
            }
            vkUnmapMemory (logical_device, *memory);
        }

        vkBindBufferMemory (logical_device, *buffer, *memory, 0);
    }

    void create_buffer (
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        device_buffer* buffer,
        VkDeviceSize size,
        void* data = nullptr) const {

        buffer->device = logical_device;

        auto bufferCreateInfo = utils::init_VkBufferCreateInfo (usage_flags, size);
        vk_assert (vkCreateBuffer (logical_device, &bufferCreateInfo, allocation_callbacks, &buffer->buffer));

        VkMemoryRequirements memReqs;
        auto alloc_info = utils::init_VkMemoryAllocateInfo ();
        vkGetBufferMemoryRequirements (logical_device, buffer->buffer, &memReqs);
        alloc_info.allocationSize = memReqs.size;
        alloc_info.memoryTypeIndex = utils::choose_memory_type (physical_device, memReqs, memory_property_flags);
        vk_assert (vkAllocateMemory (logical_device, &alloc_info, allocation_callbacks, &buffer->memory));

        buffer->alignment = memReqs.alignment;
        buffer->size = size;
        buffer->usage_flags = usage_flags;
        buffer->memory_property_flags = memory_property_flags;

        if (data != nullptr) {
            buffer->map ();
            memcpy (buffer->mapped, data, size);
            if ((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
                buffer->flush ();

            buffer->unmap ();
        }

        buffer->setup_descriptor ();

        buffer->bind ();
    }

    void copy_buffer (device_buffer* src, device_buffer* dst, queue_identifier qid, VkBufferCopy* copy_region = nullptr) const {
        assert (dst->size <= src->size);
        assert (src->buffer);
        VkCommandBuffer copy_command = create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, qid, true);
        VkBufferCopy bufferCopy{};
        if (copy_region == nullptr) {
            bufferCopy.size = src->size;
        }
        else {
            bufferCopy = *copy_region;
        }

        vkCmdCopyBuffer (copy_command, src->buffer, dst->buffer, 1, &bufferCopy);

        flush_command_buffer (copy_command, qid);
    }

    VkCommandBuffer create_command_buffer (VkCommandBufferLevel level, queue_identifier qid, bool begin = false) const {
        auto cmdBufAllocateInfo = utils::init_VkCommandBufferAllocateInfo (logical_device_info.default_command_pools[qid.family_index], level, 1);

        VkCommandBuffer cmdBuffer;
        vk_assert (vkAllocateCommandBuffers (logical_device, &cmdBufAllocateInfo, &cmdBuffer));

        if (begin) {
            auto cmdBufInfo = utils::init_VkCommandBufferBeginInfo ();
            vk_assert (vkBeginCommandBuffer (cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    void flush_command_buffer (VkCommandBuffer command_buffer, queue_identifier qid, bool free = true) const {
        if (command_buffer == VK_NULL_HANDLE) {
            return;
        }
        vk_assert (vkEndCommandBuffer (command_buffer));

        auto submitInfo = utils::init_VkSubmitInfo ();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        VkQueue queue = logical_device_info.queues[qid.family_index][qid.number];

        auto fenceInfo = utils::init_VkFenceCreateInfo (0);
        VkFence fence;
        vk_assert (vkCreateFence (logical_device, &fenceInfo, allocation_callbacks, &fence));
        vk_assert (vkQueueSubmit (queue, 1, &submitInfo, fence));
        vk_assert (vkWaitForFences (logical_device, 1, &fence, VK_TRUE, 99999999999));

        vkDestroyFence (logical_device, fence, allocation_callbacks);

        if (free) {
            vkFreeCommandBuffers (logical_device, logical_device_info.default_command_pools[qid.family_index], 1, &command_buffer);
        }
    }

};

}

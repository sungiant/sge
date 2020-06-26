// SGE-VK-CONTEXT
// ---------------------------------- //
// Useful view of aggregated contexual
// info about a given Vulkan workgroup.
// ---------------------------------- //

#pragma once

#include <vulkan/vulkan.h>
#if TARGET_MACOSX
#include <MoltenVK/mvk_vulkan.h>
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

#include "sge.hh"
#include "sge_vk_types.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_buffer.hh"

namespace sge::vk {

struct context {

    VkAllocationCallbacks*        allocation_callbacks;
    VkInstance                    instance;
    VkPhysicalDevice              physical_device;
    VkDevice                      logical_device;
    physical_device_info&         physical_device_info;
    logical_device_info&          logical_device_info;
    VkCommandPool                 default_command_pool;

    context (
        VkAllocationCallbacks* z_allocation_callbacks,
        VkInstance z_instance,
        VkPhysicalDevice z_physical_device,
        VkDevice z_logical_device,
        struct physical_device_info& z_physical_device_info,
        struct logical_device_info& z_logical_device_info,
        VkCommandPool z_default_command_pool)
        : allocation_callbacks (z_allocation_callbacks)
        , instance (z_instance)
        , physical_device (z_physical_device)
        , logical_device (z_logical_device)
        , physical_device_info (z_physical_device_info)
        , logical_device_info (z_logical_device_info)
        , default_command_pool (z_default_command_pool)
    {}

    ~context () {}

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

    void copy_buffer (device_buffer* src, device_buffer* dst, VkQueue queue, VkBufferCopy* copy_region = nullptr) const {
        assert (dst->size <= src->size);
        assert (src->buffer);
        VkCommandBuffer copy_command = create_command_buffer (VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkBufferCopy bufferCopy{};
        if (copy_region == nullptr) {
            bufferCopy.size = src->size;
        }
        else {
            bufferCopy = *copy_region;
        }

        vkCmdCopyBuffer (copy_command, src->buffer, dst->buffer, 1, &bufferCopy);

        flush_command_buffer (copy_command, queue);
    }

    VkCommandBuffer create_command_buffer (VkCommandBufferLevel level, bool begin = false) const {
        auto cmdBufAllocateInfo = utils::init_VkCommandBufferAllocateInfo (default_command_pool, level, 1);

        VkCommandBuffer cmdBuffer;
        vk_assert (vkAllocateCommandBuffers (logical_device, &cmdBufAllocateInfo, &cmdBuffer));

        if (begin) {
            auto cmdBufInfo = utils::init_VkCommandBufferBeginInfo ();
            vk_assert (vkBeginCommandBuffer (cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    void flush_command_buffer (VkCommandBuffer command_buffer, VkQueue queue, bool free = true) const {
        if (command_buffer == VK_NULL_HANDLE) {
            return;
        }
        vk_assert (vkEndCommandBuffer (command_buffer));

        auto submitInfo = utils::init_VkSubmitInfo ();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        auto fenceInfo = utils::init_VkFenceCreateInfo (0);
        VkFence fence;
        vk_assert (vkCreateFence (logical_device, &fenceInfo, allocation_callbacks, &fence));
        vk_assert (vkQueueSubmit (queue, 1, &submitInfo, fence));
        vk_assert (vkWaitForFences (logical_device, 1, &fence, VK_TRUE, 99999999999));

        vkDestroyFence (logical_device, fence, allocation_callbacks);

        if (free) {
            vkFreeCommandBuffers (logical_device, default_command_pool, 1, &command_buffer);
        }
    }

};

}

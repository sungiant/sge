// SGE-VK-ALLOCATOR
// -------------------------------------
// Custom Vulkan allocator (placeholder)
// -------------------------------------

#pragma once

#include <vulkan/vulkan.h>

namespace sge::vk {

struct allocator {
    ~allocator ();

    void debug_ui () const;
    operator VkAllocationCallbacks () const;

private:
    static void* VKAPI_CALL allocation      (void* pUserData, size_t size,  size_t alignment, VkSystemAllocationScope allocationScope);
    static void* VKAPI_CALL reallocation    (void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    static void  VKAPI_CALL free            (void* pUserData, void* pMemory);

    void* allocation    (size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void* reallocation  (void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void  free          (void* pMemory);

    // some noddy tracking for now
    uint32_t allocation_count = 0;
    uint32_t reallocation_count = 0;
    uint32_t free_count = 0;
};

}

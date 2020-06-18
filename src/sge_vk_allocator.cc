#include "sge_vk_allocator.hh"

#include <memory>
#include <iostream>
#include <stdlib.h>

#include <imgui/imgui.h>

#if TARGET_WIN32
#define aligned_alloc _aligned_malloc
#define aligned_realloc _aligned_realloc
#define aligned_free _aligned_free
#else
void aligned_free (void *memblock)
{
    if (!memblock) return;
    free (memblock);
};

void * aligned_realloc (
    void *memblock,
    size_t size,
    size_t alignment)
{
    void* p = nullptr;
#if TARGET_MACOSX
    if (__builtin_available (macOS 10.15, *))
        p = aligned_alloc (alignment, size);
    else { assert (false); }
#else
    p = aligned_alloc (alignment, size);
#endif
    memcpy (p, memblock, size);
    aligned_free (memblock);
    return p;
};

#endif

namespace sge::vk {
    
void* VKAPI_CALL allocator::allocation (void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return static_cast<allocator*>(pUserData)->allocation (size, alignment, allocationScope);
}
void* VKAPI_CALL allocator::reallocation (void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return static_cast<allocator*>(pUserData)->reallocation (pOriginal, size, alignment, allocationScope);
}
void  VKAPI_CALL allocator::free (void* pUserData, void* pMemory) {
    return static_cast<allocator*>(pUserData)->free (pMemory);
}

void* allocator::allocation (size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    allocation_count++;

#if TARGET_MACOSX
    if (__builtin_available (macOS 10.15, *))
        return aligned_alloc (size, alignment);
    else { assert (false); return nullptr; }
#else
return aligned_alloc (size, alignment);
#endif
    
}
    
void* allocator::reallocation (void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    reallocation_count++;
    return aligned_realloc (pOriginal, size, alignment);
}
void  allocator::free (void* pMemory) {
    free_count++;
    return aligned_free (pMemory);
}
    
allocator::operator VkAllocationCallbacks () const {
    VkAllocationCallbacks result;
    result.pUserData = (void*)this;
    result.pfnAllocation = &allocator::allocation;
    result.pfnReallocation = &allocator::reallocation;
    result.pfnFree = &allocator::free;
    result.pfnInternalAllocation = nullptr;
    result.pfnInternalFree = nullptr;

    return result;
};

void allocator::debug_ui () const {
    ImGui::Begin ("SGE VK allocator");
    {
        ImGui::Text ("allocation count: %d", allocation_count);
        ImGui::Text ("reallocation count: %d", reallocation_count);
        ImGui::Text ("free count: %d", free_count);
    }
    ImGui::End ();
}

allocator::~allocator () {}

}

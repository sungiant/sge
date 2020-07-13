// SGE-VK-KERNAL
// ---------------------------------- //
// Logic for setting up Vulkan and
// keeping track of things.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_context.hh"

namespace sge::vk {

struct allocator;

class kernel {
public:
    kernel ();
    ~kernel () = default;

    const context&                      primary_context                         () const;

    queue_identifier                    primary_graphics_queue_id               () const;
    queue_identifier                    primary_compute_queue_id                () const;
    queue_identifier                    primary_transfer_queue_id               () const;

    VkDevice                            get_logical_device                      (VkPhysicalDevice) const;
    VkPhysicalDevice                    get_physical_device                     (VkDevice) const;
    VkQueue                             get_queue                               (queue_identifier) const;

    void                                create                                  ();
    void                                destroy                                 ();

    void                                debug_ui ();


    VkQueue                             primary_graphics_queue () const { return get_queue (primary_graphics_queue_id ()); }
    VkQueue                             primary_compute_queue () const { return get_queue (primary_compute_queue_id ()); }
    VkQueue                             primary_transfer_queue () const { return get_queue (primary_transfer_queue_id ()); }

private:

    struct state {
        VkInstance                                                              instance = VK_NULL_HANDLE;

        // populated by: get_physical_devices
        std::unordered_map<VkPhysicalDevice, physical_device_info>              physical_device_info;

        // populated by: create_logical_devices
        std::unordered_map<VkDevice, logical_device_info>                       logical_device_info;
        std::unordered_map<VkDevice, VkPhysicalDevice>                          device_map;
        std::unordered_map<VkPhysicalDevice, VkDevice>                          device_map_inv;
        std::vector<context>                                                    contexts;


        VkDebugReportCallbackEXT                                                debug_report_callback;
    };

    const std::unique_ptr<allocator>                                            custom_allocator;
    const std::optional<VkAllocationCallbacks>                                  custom_allocator_callbacks;

    state                                                                       state = {};

    
    VkAllocationCallbacks*              allocation_callbacks                    () const;
    void                                create_instance                         ();
    void                                get_physical_devices                    ();
    void                                create_logical_devices                  ();
};

}

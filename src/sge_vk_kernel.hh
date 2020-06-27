// SGE-VK-KERNAL
// ---------------------------------- //
// Logic for setting up Vulkan.
// ---------------------------------- //

#pragma once

#include "sge.hh"
#include "sge_vk_types.hh"
#include "sge_vk_utils.hh"
#include "sge_vk_context.hh"

namespace sge::vk {

struct allocator;

class kernel {
public:
    kernel ();
    ~kernel () = default;

    const context&                      primary_context                         () const;
    queue_identifier                    primary_work_queue                      () const;
    VkDevice                            get_logical_device                      (VkPhysicalDevice) const;
    VkPhysicalDevice                    get_physical_device                     (VkDevice) const;
    VkQueue                             get_queue                               (queue_identifier) const;

    void                                create                                  ();
    void                                destroy                                 ();

    void                                append_debug_fns (std::vector<std::function<void ()>>&);
    
    const char*                         get_physical_device_name                () const { return primary_context().physical_device_info.name.c_str (); }


private:

    struct state {
        VkInstance                                                              instance = VK_NULL_HANDLE;
        std::vector<VkPhysicalDeviceGroupPropertiesKHR>                         device_group_properties;
        std::vector<VkPhysicalDevice>                                           physical_devices;
        std::unordered_map<VkPhysicalDevice, physical_device_info>              physical_device_info;
        std::unordered_map<VkDevice, logical_device_info>                       logical_device_info;
        std::unordered_map<VkDevice, VkCommandPool>                             logical_device_default_command_pool;
        VkDebugReportCallbackEXT                                                debug_report_callback;
        std::unordered_map<VkDevice, VkPhysicalDevice>                          device_map;
        std::unordered_map<VkPhysicalDevice, VkDevice>                          device_map_inv;
        std::vector<context>                                                    contexts;
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

// SGE-VK-UTILS (INLINE)
// -------------------------------------
// Helpers for creating Vulkan objects.
// -------------------------------------
// - Full types listed here for reference purposes.

#pragma once

inline VkViewport init_VkViewport (float width, float height, float min_depth, float max_depth) {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    return viewport;
}

inline VkRect2D init_VkRect2D (uint32_t width, uint32_t height, int32_t offset_x = 0, int32_t offset_y = 0) {
    VkRect2D rect{};
    rect.extent.width = width;
    rect.extent.height = height;
    rect.offset.x = offset_x;
    rect.offset.y = offset_y;
    return rect;
}

inline VkRect2D init_VkRect2D (VkExtent2D extent, VkOffset2D offset = {}) {
    VkRect2D rect{};
    rect.extent = extent;
    rect.offset = offset;
    return rect;
}

inline VkRect2D init_VkRect2D (int32_t width, int32_t height, int32_t offset_x = 0, int32_t offset_y = 0) {
    VkRect2D rect{};
    rect.extent.width = width;
    rect.extent.height = height;
    rect.offset.x = offset_x;
    rect.offset.y = offset_y;
    return rect;
}

inline VkDescriptorPoolSize init_VkDescriptorPoolSize (VkDescriptorType type, uint32_t descriptor_count) {
    VkDescriptorPoolSize pool_size{};
    pool_size.type = type;
    pool_size.descriptorCount = descriptor_count;
    return pool_size;
}

inline VkDescriptorSetLayoutBinding init_VkDescriptorSetLayoutBinding (VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding, uint32_t descriptor_count = 1) {
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorType = type;
    layout_binding.descriptorCount = descriptor_count;
    layout_binding.stageFlags = stage_flags;
    //layout_binding.pImmutableSamplers;
    return layout_binding;
}

inline VkDescriptorImageInfo init_VkDescriptorImageInfo (VkSampler sampler, VkImageView image_view, VkImageLayout image_layout) {
    VkDescriptorImageInfo image_info{};
    image_info.sampler = sampler;
    image_info.imageView = image_view;
    image_info.imageLayout = image_layout;
    return image_info;
}

inline VkVertexInputBindingDescription init_VkVertexInputBindingDescription (uint32_t binding, uint32_t stride, VkVertexInputRate input_rate) {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = binding;
    binding_description.stride = stride;
    binding_description.inputRate = input_rate;
    return binding_description;
}

inline VkVertexInputAttributeDescription init_VkVertexInputAttributeDescription (uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
    VkVertexInputAttributeDescription attribute_description{};
    attribute_description.location = location;
    attribute_description.binding = binding;
    attribute_description.format = format;
    attribute_description.offset = offset;
    return attribute_description;
}

inline VkPushConstantRange init_VkPushConstantRange (VkShaderStageFlags stage_flags, uint32_t size, uint32_t offset = 0) {
    VkPushConstantRange range{};
    range.stageFlags = stage_flags;
    range.offset = offset;
    range.size = size;
    return range;
}


inline VkPipelineColorBlendAttachmentState init_VkPipelineColorBlendAttachmentState () {
    VkPipelineColorBlendAttachmentState state = {};
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    return state;
}

inline VkPipelineColorBlendAttachmentState init_VkPipelineColorBlendAttachmentState_opaque () {
    VkPipelineColorBlendAttachmentState state = {};
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    return state;
}

inline VkPipelineColorBlendAttachmentState init_VkPipelineColorBlendAttachmentState_subtract () {
    VkPipelineColorBlendAttachmentState state = {};
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.alphaBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
    return state;
}

inline VkPipelineColorBlendAttachmentState init_VkPipelineColorBlendAttachmentState_additive () {
    VkPipelineColorBlendAttachmentState state = {};
    state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    return state;
}


//--------------------------------------------------------------------------------------------------------------------//

inline VkApplicationInfo init_VkApplicationInfo (const char* app_name) {
    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
    app_info.pEngineName = "SGE";
    app_info.engineVersion = VK_MAKE_VERSION (1, 0, 0);
    app_info.apiVersion = VK_MAKE_VERSION (1, 0, 0);
    return app_info;
}

inline VkInstanceCreateInfo init_VkInstanceCreateInfo (const VkApplicationInfo* app_info, const std::vector<const char*>& required_instance_layers, const std::vector<const char*>& required_instance_extensions) {
    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.pApplicationInfo = app_info;
    create_info.enabledLayerCount = (uint32_t) required_instance_layers.size ();
    create_info.ppEnabledLayerNames = required_instance_layers.data ();
    create_info.enabledExtensionCount = (uint32_t) required_instance_extensions.size ();
    create_info.ppEnabledExtensionNames = required_instance_extensions.data ();
    return create_info;
}

inline VkMemoryAllocateInfo init_VkMemoryAllocateInfo () {
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    //allocate_info.allocationSize;
    //allocate_info.memoryTypeIndex;
    return allocate_info;
}

inline VkMappedMemoryRange init_VkMappedMemoryRange () {
    VkMappedMemoryRange mapped_memory_range {};
    mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_memory_range.pNext = nullptr;
    //mapped_memory_range.memory;
    //mapped_memory_range.offset;
    //mapped_memory_range.size;
    return mapped_memory_range;
}

inline VkCommandBufferAllocateInfo init_VkCommandBufferAllocateInfo (VkCommandPool command_pool, VkCommandBufferLevel level, uint32_t buffer_count) {
    VkCommandBufferAllocateInfo allocate_info {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.commandPool = command_pool;
    allocate_info.level = level;
    allocate_info.commandBufferCount = buffer_count;
    return allocate_info;
}

inline VkCommandBufferBeginInfo init_VkCommandBufferBeginInfo (VkCommandBufferUsageFlags flags = 0) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = flags;
    //begin_info.pInheritanceInfo;
    return begin_info;
}

inline VkRenderPassBeginInfo init_VkRenderPassBeginInfo () {
    VkRenderPassBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.pNext = nullptr;
    //begin_info.renderPass;
    //begin_info.framebuffer;
    //begin_info.renderArea;
    //begin_info.clearValueCount;
    //begin_info.pClearValues;
    return begin_info;
}

inline VkImageMemoryBarrier init_VkImageMemoryBarrier () {
    VkImageMemoryBarrier image_memory_barrier {};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.pNext = nullptr;
    //image_memory_barrier.srcAccessMask;
    //image_memory_barrier.dstAccessMask;
    //image_memory_barrier.oldLayout;
    //image_memory_barrier.newLayout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //image_memory_barrier.image;
    //image_memory_barrier.subresourceRange;
    return image_memory_barrier;
}

inline VkImageCreateInfo init_VkImageCreateInfo () {
    VkImageCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.imageType;
    //create_info.format;
    //create_info.extent;
    //create_info.mipLevels;
    //create_info.arrayLayers;
    //create_info.samples;
    //create_info.tiling;
    //create_info.usage;
    //create_info.sharingMode;
    //create_info.queueFamilyIndexCount;
    //create_info.pQueueFamilyIndices;
    //create_info.initialLayout;
    return create_info;
}

inline VkSamplerCreateInfo init_VkSamplerCreateInfo () {
    VkSamplerCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.magFilter;
    //create_info.minFilter;
    //create_info.mipmapMode;
    //create_info.addressModeU;
    //create_info.addressModeV;
    //create_info.addressModeW;
    create_info.mipLodBias = 0.0f;
    //create_info.anisotropyEnable;
    create_info.maxAnisotropy = 1.0f;
    //create_info.compareEnable;
    //create_info.compareOp;
    create_info.minLod = 0.0f;
    create_info.maxLod = 0.0f;
    //create_info.borderColor;
    //create_info.unnormalizedCoordinates;
    return create_info;
}

inline VkImageViewCreateInfo init_VkImageViewCreateInfo () {
    VkImageViewCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.image;
    //create_info.viewType;
    //create_info.format;
    //create_info.components;
    //create_info.subresourceRange;
    return create_info;
}

inline VkFenceCreateInfo init_VkFenceCreateInfo (VkFenceCreateFlags flags = 0) {
    VkFenceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    return create_info;
}

inline VkSubmitInfo init_VkSubmitInfo () {
    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    //submit_info.waitSemaphoreCount;
    //submit_info.pWaitSemaphores;
    //submit_info.pWaitDstStageMask;
    //submit_info.commandBufferCount;
    //submit_info.pCommandBuffers;
    //submit_info.signalSemaphoreCount;
    //submit_info.pSignalSemaphores;
    return submit_info;
}

inline VkBufferCreateInfo init_VkBufferCreateInfo () {
    VkBufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.size;
    //create_info.usage;
    //create_info.sharingMode;
    //create_info.queueFamilyIndexCount;
    //create_info.pQueueFamilyIndices;
    return create_info;
}

inline VkBufferCreateInfo init_VkBufferCreateInfo (VkBufferUsageFlags usage, VkDeviceSize size) {
    VkBufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.size = size;
    create_info.usage = usage;
    //create_info.sharingMode;
    //create_info.queueFamilyIndexCount;
    //create_info.pQueueFamilyIndices;
    return create_info;
}

inline VkDescriptorPoolCreateInfo init_VkDescriptorPoolCreateInfo (const std::vector<VkDescriptorPoolSize>& pool_sizes, uint32_t max_sets, VkDescriptorPoolCreateFlags flags = 0) {
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size ());
    create_info.pPoolSizes = pool_sizes.data ();
    return create_info;
}

inline VkDescriptorSetLayoutCreateInfo init_VkDescriptorSetLayoutCreateInfo (const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.pBindings = bindings.data ();
    create_info.bindingCount = static_cast<uint32_t>(bindings.size ());
    return create_info;
}

inline VkPipelineLayoutCreateInfo init_VkPipelineLayoutCreateInfo (uint32_t set_layout_count, const VkDescriptorSetLayout* set_layouts) {
    VkPipelineLayoutCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    //create_info.pushConstantRangeCount;
    //create_info.pPushConstantRanges;
    return create_info;
}

inline VkDescriptorSetAllocateInfo init_VkDescriptorSetAllocateInfo (VkDescriptorPool descriptor_pool, const VkDescriptorSetLayout* set_layouts, uint32_t descriptor_set_count) {
    VkDescriptorSetAllocateInfo allocate_info {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorPool = descriptor_pool;
    allocate_info.descriptorSetCount = descriptor_set_count;
    allocate_info.pSetLayouts = set_layouts;
    return allocate_info;
}

inline VkWriteDescriptorSet init_VkWriteDescriptorSet (VkDescriptorSet dst_set, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* buffer_info, uint32_t descriptor_count) {
    VkWriteDescriptorSet descriptor_set {};
    descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_set.pNext = nullptr;
    descriptor_set.dstSet = dst_set;
    descriptor_set.dstBinding = binding;
    //descriptor_set.dstArrayElement;
    descriptor_set.descriptorCount = descriptor_count;
    descriptor_set.descriptorType = type;
    //descriptor_set.pImageInfo;
    descriptor_set.pBufferInfo = buffer_info;
    //descriptor_set.pTexelBufferView;
    return descriptor_set;
}

inline VkWriteDescriptorSet init_VkWriteDescriptorSet (VkDescriptorSet dst_set, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo *image_info, uint32_t descriptor_count) {
    VkWriteDescriptorSet descriptor_set {};
    descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_set.pNext = nullptr;
    descriptor_set.dstSet = dst_set;
    descriptor_set.dstBinding = binding;
    //descriptor_set.dstArrayElement;
    descriptor_set.descriptorCount = descriptor_count;
    descriptor_set.descriptorType = type;
    descriptor_set.pImageInfo = image_info;
    //descriptor_set.pBufferInfo;
    //descriptor_set.pTexelBufferView;
    return descriptor_set;
}

inline VkPipelineVertexInputStateCreateInfo init_VkPipelineVertexInputStateCreateInfo () {
    VkPipelineVertexInputStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.vertexBindingDescriptionCount = 0;
    create_info.pVertexBindingDescriptions = nullptr;
    create_info.vertexAttributeDescriptionCount = 0;
    create_info.pVertexAttributeDescriptions = nullptr;
    return create_info;
}

inline VkPipelineInputAssemblyStateCreateInfo init_VkPipelineInputAssemblyStateCreateInfo (VkPrimitiveTopology topology, VkBool32 primitive_restart_enable) {
    VkPipelineInputAssemblyStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //create_info.flags;
    create_info.topology = topology;
    create_info.primitiveRestartEnable = primitive_restart_enable;
    return create_info;
}

inline VkPipelineRasterizationStateCreateInfo init_VkPipelineRasterizationStateCreateInfo (VkPolygonMode polygon_mode, VkCullModeFlags cull_mode, VkFrontFace front_face, VkPipelineRasterizationStateCreateFlags flags = 0) {
    VkPipelineRasterizationStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.depthClampEnable = VK_FALSE;
    create_info.rasterizerDiscardEnable = VK_FALSE;
    create_info.polygonMode = polygon_mode;
    create_info.cullMode = front_face;
    create_info.frontFace = front_face;
    create_info.depthBiasEnable = VK_FALSE;
    //create_info.depthBiasConstantFactor;
    //create_info.depthBiasClamp;
    //create_info.depthBiasSlopeFactor;
    create_info.lineWidth = 1.0f;
    return create_info;
}

inline VkPipelineColorBlendStateCreateInfo init_VkPipelineColorBlendStateCreateInfo (const VkPipelineColorBlendAttachmentState& attachment) {
    VkPipelineColorBlendStateCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.logicOpEnable;
    //create_info.logicOp;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &attachment;
    //create_info.blendConstants;
    return create_info;
}

inline VkPipelineColorBlendStateCreateInfo init_VkPipelineColorBlendStateCreateInfo (uint32_t attachment_count, const VkPipelineColorBlendAttachmentState * attachments) {
    VkPipelineColorBlendStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.logicOpEnable;
    //create_info.logicOp;
    create_info.attachmentCount = attachment_count;
    create_info.pAttachments = attachments;
    //create_info.blendConstants;
    return create_info;
}

inline VkPipelineDepthStencilStateCreateInfo init_VkPipelineDepthStencilStateCreateInfo (VkBool32 depth_test_enable, VkBool32 depth_write_enable, VkCompareOp depth_compare_op) {
    VkPipelineDepthStencilStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.depthTestEnable;
    //create_info.depthWriteEnable;
    create_info.depthTestEnable = depth_test_enable;
    create_info.depthWriteEnable = depth_write_enable;
    create_info.depthCompareOp = depth_compare_op;
    //create_info.depthBoundsTestEnable;
    //create_info.stencilTestEnable;
    //create_info.front;
    //create_info.back;
    create_info.back.compareOp = VK_COMPARE_OP_ALWAYS;
    //create_info.minDepthBounds;
    //create_info.maxDepthBounds;
    return create_info;
}

inline VkPipelineViewportStateCreateInfo init_VkPipelineViewportStateCreateInfo (const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors, VkPipelineViewportStateCreateFlags flags = 0) {

    assert (viewports.size () == scissors.size ());
    VkPipelineViewportStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.viewportCount = (uint32_t) viewports.size ();
    create_info.pViewports = viewports.data ();
    create_info.scissorCount = (uint32_t) scissors.size ();
    create_info.pScissors = scissors.data ();
    return create_info;
}


inline VkPipelineViewportStateCreateInfo init_VkPipelineViewportStateCreateInfo (const VkViewport& viewport, const VkRect2D& scissor, VkPipelineViewportStateCreateFlags flags = 0) {
    VkPipelineViewportStateCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.viewportCount = 1;
    create_info.pViewports = &viewport;
    create_info.scissorCount = 1;
    create_info.pScissors = &scissor;
    return create_info;
}

inline VkPipelineViewportStateCreateInfo init_VkPipelineViewportStateCreateInfo (VkPipelineViewportStateCreateFlags flags = 0) {
    VkPipelineViewportStateCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.viewportCount = 1;
    create_info.pViewports = nullptr;
    create_info.scissorCount = 1;
    create_info.pScissors = nullptr;
    return create_info;
}

inline VkPipelineMultisampleStateCreateInfo init_VkPipelineMultisampleStateCreateInfo (VkSampleCountFlagBits rasterization_samples, VkPipelineMultisampleStateCreateFlags flags = 0) {
    VkPipelineMultisampleStateCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.rasterizationSamples = rasterization_samples;
    create_info.sampleShadingEnable = VK_FALSE;
    //create_info.minSampleShading;
    //create_info.pSampleMask;
    create_info.alphaToCoverageEnable = VK_FALSE;
    create_info.alphaToOneEnable = VK_FALSE;
    return create_info;
}

inline VkPipelineDynamicStateCreateInfo init_VkPipelineDynamicStateCreateInfo (const std::vector<VkDynamicState>& dynamic_states, VkPipelineDynamicStateCreateFlags flags = 0) {
    VkPipelineDynamicStateCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size ());
    create_info.pDynamicStates = dynamic_states.data ();
    return create_info;
}

inline VkGraphicsPipelineCreateInfo init_VkGraphicsPipelineCreateInfo (VkPipelineLayout layout, VkRenderPass render_pass, VkPipelineCreateFlags flags = 0) {
    VkGraphicsPipelineCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    //create_info.stageCount;
    //create_info.pStages;
    //create_info.pVertexInputState;
    //create_info.pInputAssemblyState;
    //create_info.pTessellationState;
    //create_info.pViewportState;
    //create_info.pRasterizationState;
    //create_info.pMultisampleState;
    //create_info.pDepthStencilState;
    //create_info.pColorBlendState;
    //create_info.pDynamicState;
    create_info.layout = layout;
    create_info.renderPass = render_pass;
    //create_info.subpass;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;
    return create_info;
}

inline VkGraphicsPipelineCreateInfo init_VkGraphicsPipelineCreateInfo () {
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.stageCount;
    //create_info.pStages;
    //create_info.pVertexInputState;
    //create_info.pInputAssemblyState;
    //create_info.pTessellationState;
    //create_info.pViewportState;
    //create_info.pRasterizationState;
    //create_info.pMultisampleState;
    //create_info.pDepthStencilState;
    //create_info.pColorBlendState;
    //create_info.pDynamicState;
    //create_info.layout;
    //create_info.renderPass;
    //create_info.subpass;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;
    return create_info;
}

inline VkComputePipelineCreateInfo init_VkComputePipelineCreateInfo (VkPipelineLayout layout, VkPipelineCreateFlags flags = 0) {
    VkComputePipelineCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    //create_info.stage;
    create_info.layout = layout;
    //create_info.basePipelineHandle;
    //create_info.basePipelineIndex;
    return create_info;
}

inline VkPresentInfoKHR init_VkPresentInfoKHR () {
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    //present_info.waitSemaphoreCount;
    //present_info.pWaitSemaphores;
    //present_info.swapchainCount;
    //present_info.pSwapchains;
    //present_info.pImageIndices;
    //present_info.pResults;
    return present_info;
}

inline VkSwapchainCreateInfoKHR init_VkSwapchainCreateInfoKHR () {
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.surface;
    //create_info.minImageCount;
    //create_info.imageFormat;
    //create_info.imageColorSpace;
    //create_info.imageExtent;
    //create_info.imageArrayLayers;
    //create_info.imageUsage;
    //create_info.imageSharingMode;
    //create_info.queueFamilyIndexCount;
    //create_info.pQueueFamilyIndices;
    //create_info.preTransform;
    //create_info.compositeAlpha;
    //create_info.presentMode;
    //create_info.clipped;
    //create_info.oldSwapchain;
    return create_info;
}

inline VkShaderModuleCreateInfo init_VkShaderModuleCreateInfo (size_t size, const uint32_t* spirv) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.codeSize = size;
    create_info.pCode = spirv;
    return create_info;
}

inline VkSemaphoreCreateInfo init_VkSemaphoreCreateInfo () {
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    return create_info;
}

inline VkRenderPassCreateInfo init_VkRenderPassCreateInfo () {
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.attachmentCount;
    //create_info.pAttachments;
    //create_info.subpassCount;
    //create_info.pSubpasses;
    //create_info.dependencyCount;
    //create_info.pDependencies;
    return create_info;
}

inline VkFramebufferCreateInfo init_VkFramebufferCreateInfo () {
    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.renderPass;
    //create_info.attachmentCount;
    //create_info.pAttachments;
    //create_info.width;
    //create_info.height;
    //create_info.layers;
    return create_info;
}

inline VkCommandPoolCreateInfo init_VkCommandPoolCreateInfo (uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0) {
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.queueFamilyIndex = queue_family_index;
    return create_info;
}

inline VkPipelineShaderStageCreateInfo init_VkPipelineShaderStageCreateInfo (VkShaderStageFlagBits stage, VkShaderModule module, const char* name) {
    VkPipelineShaderStageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.stage = stage;
    create_info.module = module;
    create_info.pName = name;
    //create_info.pSpecializationInfo;
    return create_info;
}

inline VkPipelineCacheCreateInfo init_VkPipelineCacheCreateInfo () {
    VkPipelineCacheCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    //create_info.initialDataSize;
    //create_info.pInitialData;
    return create_info;
}

inline VkDebugReportCallbackCreateInfoEXT init_VkDebugReportCallbackCreateInfoEXT (PFN_vkDebugReportCallbackEXT debug_callback) {
    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT
        | VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_DEBUG_BIT_EXT
        | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    create_info.pfnCallback = debug_callback;
    create_info.pUserData = nullptr;
    return create_info;
}

inline VkDeviceQueueCreateInfo init_VkDeviceQueueCreateInfo (uint32_t queue_family_index, uint32_t queue_count, const std::vector<float>& queue_priorities) {
    assert (queue_count > 0);
    assert (queue_priorities.size () == queue_count);
    VkDeviceQueueCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.queueFamilyIndex = queue_family_index;
    create_info.queueCount = queue_count;
    create_info.pQueuePriorities = queue_priorities.data ();
    return create_info;
}

inline VkDeviceCreateInfo init_VkDeviceCreateInfo (const std::vector<VkDeviceQueueCreateInfo>& queues, const std::vector<const char*>& layers, const std::vector<const char*>& extensions) {
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.queueCreateInfoCount = (uint32_t)queues.size ();
    create_info.pQueueCreateInfos = queues.data ();
    create_info.enabledLayerCount = (uint32_t) layers.size ();
    create_info.ppEnabledLayerNames = layers.data ();
    create_info.enabledExtensionCount = (uint32_t) extensions.size ();
    create_info.ppEnabledExtensionNames = extensions.data ();
    //create_info.pEnabledFeatures;
    return create_info;
}

#if TARGET_WIN32 && !VARIANT_HEADLESS

inline VkWin32SurfaceCreateInfoKHR init_VkWin32SurfaceCreateInfoKHR (HINSTANCE hinstance, HWND hwnd) {
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags;
    create_info.hinstance = hinstance;
    create_info.hwnd = hwnd;
    return create_info;
}

#endif

#if TARGET_MACOSX && !VARIANT_HEADLESS

inline VkMacOSSurfaceCreateInfoMVK init_VkMacOSSurfaceCreateInfoMVK (void* view) {
    VkMacOSSurfaceCreateInfoMVK create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.pView = view;
    return create_info;
}

#endif

#if TARGET_LINUX && !VARIANT_HEADLESS

inline VkXcbSurfaceCreateInfoKHR init_VkXcbSurfaceCreateInfoKHR (xcb_connection_t* connection, xcb_window_id window) {
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    //create_info.flags;
    create_info.connection = connection;
    create_info.window = window;
    return create_info;
}

#endif

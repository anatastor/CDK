
#pragma once

#include "renderer/renderer_vulkan.inl"


DeviceQueueIndices fill_DeviceQueueIndicies (VulkanContext* context, DeviceQueueIndices* out);

uint8 create_physical_device (VulkanContext* context);
uint8 create_logical_device (VulkanContext* context);
uint8 create_swapchain (VulkanContext* context);
void close_swapchain(VulkanContext* context);
uint8 recreate_swapchain(VulkanContext* context);
uint8 create_imageViews (VulkanContext* context);


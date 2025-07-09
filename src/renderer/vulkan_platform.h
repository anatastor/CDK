
#pragma once

#include "renderer_vulkan.inl"


uint8 cdk_platform_vulkan_get_required_extensions (const char*** extensions);

VkResult cdk_platform_create_vulkan_surface (PlatformState* pltState, VkInstance* instance, VkSurfaceKHR* surface);

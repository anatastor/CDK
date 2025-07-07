
#pragma once

#include "def.h"

#include <vulkan/vulkan.h>


typedef struct VulkanContext
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;

    VkSurfaceKHR surface;
} VulkanContext;


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
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent; 

    VkImage* swapchainImages;
} VulkanContext;

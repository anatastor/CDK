
#pragma once

#include "def.h"

#include <vulkan/vulkan.h>


#define MAX_FRAMES_IN_FLIGHT    2


typedef struct VulkanContext
{
    VkInstance instance;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    // VkQueue transferQueue; // TODO

    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent; 
    
    uint32 imageCount;
    VkImage* images;
    VkImageView* imageViews;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer* framebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

    uint8 currFrame; // scale with size of MAX_FRAMES_IN_FLIGHT
} VulkanContext;



typedef struct DeviceQueueIndices
{
    uint32 graphicIndex;
    uint32 presentIndex;
    uint32 computeIndex;
    uint32 transferIndex;
} DeviceQueueIndices;



typedef struct SwapchainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
    uint32 formatCount;
    uint32 presentModeCount;
} SwapchainSupport;


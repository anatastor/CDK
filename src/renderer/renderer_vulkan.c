
#include "def.h"
#include <stdlib.h>

#include "core/logger.h"
#include "dataStructures/darray.h"

#include "renderer/renderer_vulkan.inl"
#include "renderer/renderer_vulkan.h"
#include "renderer/vulkan_platform.h"
#include "renderer/renderer_vulkan_device.h"
#include "renderer/renderer_vulkan_pipeline.h"
#include "renderer/renderer_vulkan_framebuffer.h"


static VulkanContext vulkanContext;


uint8 create_commandPool (void);
uint8 create_commandBuffers (void);
void record_commandBuffer (VkCommandBuffer buffer, uint32 imageIndex);
uint8 create_sync_objects (void);


static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (
        VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
        VkDebugUtilsMessageTypeFlagsEXT msgType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData
        )
{
    cdk_log_debug ("[VULKAN DEBUG]: %s", callbackData->pMessage);
    return VK_FALSE;
}


VkResult
CreateDebugUtilsMessengerEXT (VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
{   
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

    if (func)
        return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
    
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}


void
DestroyDebugUtilsMessengerEXT (VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func)
        func (instance, debugMessenger, pAllocator);
}


void
fill_debug_createInfo (VkDebugUtilsMessengerCreateInfoEXT* createInfo)
{
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debug_callback;
    createInfo->pUserData = NULL;
}


void
setup_debug_messenger (void)
{
#ifndef _DEBUG
    return;
#endif

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    fill_debug_createInfo (&createInfo);

    VkResult res = CreateDebugUtilsMessengerEXT (vulkanContext.instance, &createInfo, NULL, &vulkanContext.debugMessenger);
    if (res != VK_SUCCESS)
        cdk_log_error ("[VULKAN DEBUG] unable to create debugger");
}


uint8
create_instance (void)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Vulkan Test App";
    appInfo.applicationVersion = 1;
    // appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
    appInfo.pEngineName = "CDK";
    appInfo.engineVersion = 1;
    // appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // create info
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    const char** reqExtensions = cdk_darray_create (const char*);
    reqExtensions = cdk_darray_insert (reqExtensions, &VK_KHR_SURFACE_EXTENSION_NAME); // == VK_KHR_SURFACE
    cdk_platform_vulkan_get_required_extensions (&reqExtensions);
#ifdef _DEBUG
    reqExtensions = cdk_darray_insert (reqExtensions, &"VK_EXT_debug_utils");
#endif

    createInfo.enabledExtensionCount = cdk_darray_length (reqExtensions);
    createInfo.ppEnabledExtensionNames = reqExtensions;
    
#ifdef _DEBUG
    cdk_log_debug ("[VULKAN] Required extensions:");
    for (uint32 i = 0; i < cdk_darray_length (reqExtensions); i++)
        cdk_log_debug ("\t%s", reqExtensions[i]);


    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    const char *layer = "VK_LAYER_KHRONOS_validation";
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = &layer;

    fill_debug_createInfo (&debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#else
    createInfo.enabledLayerCount = 0;
#endif
       

    // actually create vulkan instance
    VkResult res = vkCreateInstance (&createInfo, NULL, &vulkanContext.instance);
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        cdk_log_error ("cannot find a compatabile vulkan ICD");
        return CDK_FALSE;
    }
    else if (res)
    {
        cdk_log_error ("unkown vulkan error");
        return CDK_FALSE;
    }

    setup_debug_messenger ();


    cdk_darray_destroy (reqExtensions);
    return CDK_TRUE;
}


uint8
vulkan_init (PlatformState* pltState)
{
    if (!create_instance ())
    {   
        cdk_log_error ("[VULKAN]: failed to create instance");
        return CDK_FALSE;
    }
    cdk_log_info ("[VULKAN]: instance created");


    if (!vulkan_create_surface (pltState))
    {
        cdk_log_debug ("[VULKAN]: failed to create surface");
        return CDK_FALSE;
    }
    cdk_log_info ("[VULKAN]: surface created");

    
    create_physical_device (&vulkanContext);
    create_logical_device (&vulkanContext);
    create_swapchain (&vulkanContext);
    create_imageViews (&vulkanContext);
    
    create_render_pass (&vulkanContext);
    create_graphics_pipeline (&vulkanContext);

    create_framebuffer (&vulkanContext);

    create_commandPool ();
    create_commandBuffers ();
    create_sync_objects ();

    vulkanContext.currFrame = 0;
    vulkanContext.framebufferResized = CDK_FALSE;

    cdk_log_info ("Vulkan successfully initialized");
    return CDK_TRUE;
}


void
vulkan_close (void)
{   
    cdk_log_debug ("closing vulkan");

    vkDeviceWaitIdle (vulkanContext.device);

    close_swapchain (&vulkanContext);
    
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(vulkanContext.device, vulkanContext.imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(vulkanContext.device, vulkanContext.renderFinishedSemaphores[i], NULL);
        vkDestroyFence(vulkanContext.device, vulkanContext.inFlightFences[i], NULL);
    }

    vkDestroyCommandPool (vulkanContext.device, vulkanContext.commandPool, NULL);
    
    /*
     * in close_swapchain
    for (uint32 i = 0; i < vulkanContext.imageCount; i++)
        vkDestroyFramebuffer(vulkanContext.device, vulkanContext.framebuffers[i], NULL);
    free (vulkanContext.framebuffers);
    vulkanContext.framebuffers = NULL;
    */

    vkDestroyPipeline (vulkanContext.device, vulkanContext.graphicsPipeline, NULL);
    vkDestroyPipelineLayout (vulkanContext.device, vulkanContext.pipelineLayout, NULL);
    vkDestroyRenderPass (vulkanContext.device, vulkanContext.renderPass, NULL);
    
    /*
     * in close_swapchain
    for (uint32 i = 0; i < vulkanContext.imageCount; i++)
        vkDestroyImageView (vulkanContext.device, vulkanContext.imageViews[i], NULL);
    free (vulkanContext.imageViews);
    vulkanContext.imageViews = NULL;
    */

    // TODO allowed or necessary ???
    free (vulkanContext.images);
    vulkanContext.images = NULL;

    // vkDestroySwapchainKHR (vulkanContext.device, vulkanContext.swapchain, NULL);
    vkDestroySurfaceKHR (vulkanContext.instance, vulkanContext.surface, NULL);
    vkDestroyDevice (vulkanContext.device, NULL);

#ifdef _DEBUG
    DestroyDebugUtilsMessengerEXT (vulkanContext.instance, vulkanContext.debugMessenger, NULL);
#endif

    vkDestroyInstance (vulkanContext.instance, NULL);
}


uint8
vulkan_create_surface (PlatformState *pltState)
{
    VkResult res = cdk_platform_create_vulkan_surface (pltState, &vulkanContext.instance, &vulkanContext.surface);
    if (res != VK_SUCCESS)
    {
        cdk_log_error ("[VULKAN] error in creating surface");
        return CDK_FALSE;
    }

    cdk_log_info ("[VULKAN] successfully created surface");
    return CDK_TRUE;
}


uint8
create_commandPool (void)
{   
    DeviceQueueIndices indices = fill_DeviceQueueIndicies (&vulkanContext, NULL);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicIndex;  

    if (vkCreateCommandPool(vulkanContext.device, &poolInfo, NULL, &vulkanContext.commandPool) != VK_SUCCESS) {
        cdk_log_error ("[VULKAN]: failed to create command pool!");
        return CDK_FALSE;
    }

    cdk_log_info ("[VULKAN]: successfully created command pool");
    return CDK_TRUE;
}


uint8
create_commandBuffers (void)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanContext.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(vulkanContext.device, &allocInfo, vulkanContext.commandBuffers) != VK_SUCCESS) {
        cdk_log_error ("[VULKAN]: failed to allocate command buffers!");
        return CDK_FALSE;
    }

    cdk_log_info ("[VULKAN]: successfully created command buffer");
    return CDK_TRUE;
}


void
record_commandBuffer (VkCommandBuffer commandBuffer, uint32 imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        cdk_log_error ("[VULKAN]: failed to begin recording command buffer!");
        return;
    }

    // starting render pass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vulkanContext.renderPass;
    renderPassInfo.framebuffer = vulkanContext.framebuffers[imageIndex];

    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = vulkanContext.swapchainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // TODO define color by html code RGBA
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // begin graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext.graphicsPipeline);
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float32) vulkanContext.swapchainExtent.width;
    viewport.height = (float32) vulkanContext.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport (commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vulkanContext.swapchainExtent;
    vkCmdSetScissor (commandBuffer, 0, 1, &scissor);

    vkCmdDraw (commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        cdk_log_error ("[VULKAN]: failed to record command buffer!");
    }
}


uint8
create_sync_objects (void)
{
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(vulkanContext.device, &semaphoreInfo, NULL, &vulkanContext.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkanContext.device, &semaphoreInfo, NULL, &vulkanContext.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vulkanContext.device, &fenceInfo, NULL, &vulkanContext.inFlightFences[i]) != VK_SUCCESS)
        {
            cdk_log_error ("[VULKAN]: failed to create semaphores at index %i!", i);
            return CDK_FALSE;
        }
    }

    return CDK_TRUE;
}


uint8 
renderer_draw_frame ()
{
    uint64 currFrame = vulkanContext.currFrame;

    vkWaitForFences(vulkanContext.device, 1, &vulkanContext.inFlightFences[currFrame], VK_TRUE, UINT64_MAX);

    uint32 imageIndex;
    VkResult res;
    res = vkAcquireNextImageKHR(vulkanContext.device, vulkanContext.swapchain, UINT64_MAX,
            vulkanContext.imageAvailableSemaphores[currFrame], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain (&vulkanContext);
        return CDK_FALSE;
    }
    else if (res != VK_SUCCESS && res == VK_SUBOPTIMAL_KHR)
        cdk_log_warn ("[VULKAN]: failed to acquire swap chain image!");

    vkResetFences(vulkanContext.device, 1, &vulkanContext.inFlightFences[currFrame]); // only reset if something should render


    vkResetCommandBuffer(vulkanContext.commandBuffers[currFrame], 0);
    record_commandBuffer (vulkanContext.commandBuffers[currFrame], imageIndex);
    
    // submit command buffer 
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vulkanContext.imageAvailableSemaphores[currFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanContext.commandBuffers[currFrame];

    VkSemaphore signalSemaphores[] = {vulkanContext.renderFinishedSemaphores[currFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, vulkanContext.inFlightFences[currFrame]) != VK_SUCCESS) {
        cdk_log_error ("[VULKAN]: failed to submit draw command buffer!");
        return CDK_FALSE;
    }

    // presentation
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // wait for render finished
    VkSwapchainKHR swapchains[] = {vulkanContext.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL; // Optional
    
    res = vkQueuePresentKHR(vulkanContext.presentQueue, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || vulkanContext.framebufferResized)
    {
        recreate_swapchain (&vulkanContext);
        vulkanContext.framebufferResized = CDK_FALSE;
    }
    else if (res != VK_SUCCESS)
        cdk_log_warn ("[VULKAN]: failed to present swapchain!");

    vulkanContext.currFrame = (currFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return CDK_TRUE;
}

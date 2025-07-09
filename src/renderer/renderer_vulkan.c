
#include "def.h"
#include <stdlib.h>

#include "core/logger.h"
#include "dataStructures/darray.h"

#include "renderer_vulkan.inl"
#include "renderer_vulkan.h"
#include "vulkan_platform.h"
#include "renderer_vulkan_device.h"


static VulkanContext vulkanContext;


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

    cdk_log_info ("Vulkan successfully initialized");
    return CDK_TRUE;
}


void
vulkan_close (void)
{   
#ifdef _DEBUG
    cdk_log_debug ("closing vulkan");
    DestroyDebugUtilsMessengerEXT (vulkanContext.instance, vulkanContext.debugMessenger, NULL);
#endif
    
    // TODO allowed or necessary ???
    free (vulkanContext.swapchainImages);
    vulkanContext.swapchainImages = NULL;

    vkDestroySwapchainKHR (vulkanContext.device, vulkanContext.swapchain, NULL);
    vkDestroySurfaceKHR (vulkanContext.instance, vulkanContext.surface, NULL);
    vkDestroyDevice (vulkanContext.device, NULL);

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

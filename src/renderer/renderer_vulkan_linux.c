
#include "def.h"

#if defined(CDK_PLATFORM_LINUX)

#include "core/logger.h"
#include "dataStructures/darray.h"

#include "renderer_vulkan.h"




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
list_available_layers ()
{
    uint32 layerCount;
    vkEnumerateInstanceLayerProperties (&layerCount, NULL);

    VkLayerProperties *availableLayers = cdk_darray_reserve (VkLayerProperties, layerCount);
    vkEnumerateInstanceLayerProperties (&layerCount, availableLayers);
        
    cdk_log_debug ("[VULKAN]: available layers:");
    for (uint32 i = 0; i < layerCount; i++)
        cdk_log_info ("\t%s", availableLayers[i].layerName);

    cdk_darray_destroy (availableLayers);
}



uint8
vulkan_init (void)
{
    // list_available_layers ();

    // application info
    VkApplicationInfo appInfo = {}; // auto 0 members
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


    // platform specific extensions
    const char* extensions[] = {
        "VK_KHR_surface"
#ifdef CDK_PLATFORM_LINUX
        , "VK_KHR_xlib_surface"
#elifdef CDK_PLATFORM_WIN
        , "VK_KHR_win32_surface"
#endif
#ifdef _DEBUG
        ,"VK_EXT_debug_utils"
#endif
    };


#ifdef _DEBUG
#define _extensionCount 3
#else
#define _extensionCount 2
#endif

    createInfo.enabledExtensionCount = _extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    
#ifdef _DEBUG
    cdk_log_debug ("[VULKAN] Required extensions:");
    for (uint32 i = 0; i < _extensionCount; i++)
        cdk_log_debug ("\t%s", extensions[i]);


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


    vulkan_get_physical_device ();
    vulkan_create_logical_device ();

    return CDK_TRUE;
}


void
vulkan_close (void)
{
#ifdef _DEBUG
    cdk_log_debug ("closing vulkan");
    DestroyDebugUtilsMessengerEXT (vulkanContext.instance, vulkanContext.debugMessenger, NULL);
#endif
    
    vkDestroyDevice (vulkanContext.device, NULL);

    vkDestroyInstance (vulkanContext.instance, NULL);
}


uint8
is_device_suitable (VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties (device, &deviceProperties);
    vkGetPhysicalDeviceFeatures (device, &deviceFeatures);
    
    // discrete gpu & geometry shader as in tutorial
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
        return CDK_TRUE;

    // do not check device type for my system as it lacks a discrete gpu
    // PHYSICAL_DEVICE_TYPE_CPU
    if (deviceFeatures.geometryShader)
        return CDK_TRUE;

    return CDK_FALSE;
}


struct QueueFamilyIndices
{
    uint32 graphics;
};


void
print_bit (uint32 n)
{
    char bits[33];
    for (uint32 i = 0, l = 31; i < 32 && l >= 0; i++, l--)
    {
        bits[i] = (n&(1<<l)) ? '1' : '0';
    }
    bits[32] = 0;

    cdk_log_debug ("\t\t[BIT] %s", bits);
}


struct QueueFamilyIndices
find_queueFamily (VkPhysicalDevice device)
{
    struct QueueFamilyIndices indices;
    
    uint32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (device, &familyCount, NULL);
    VkQueueFamilyProperties* family = cdk_darray_reserve (VkQueueFamilyProperties, familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties (device, &familyCount, family);

    for (uint32 i = 0; i < familyCount; i++)
    {
        cdk_log_info ("\t[VULKAN]: queueFamily %u", family[0].queueFlags);
        // print_bit (family[0].queueFlags);
        if (family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) // device supports graphics operations
            indices.graphics = i;
    }

    cdk_darray_destroy (family);
    return indices;
}


void
vulkan_get_physical_device (void)
{
    vulkanContext.physicalDevice = VK_NULL_HANDLE;

    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices (vulkanContext.instance, &deviceCount, NULL);

    if (deviceCount == 0)
        cdk_log_error ("failed to find device with vulkan support");
    
    VkPhysicalDevice *devices = cdk_darray_reserve (VkPhysicalDevice, deviceCount);
    vkEnumeratePhysicalDevices (vulkanContext.instance, &deviceCount, devices);
        
    cdk_log_info ("[VULKAN] available devices:");
    for (uint32 i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties (devices[i], &deviceProperties);
        vkGetPhysicalDeviceFeatures (devices[i], &deviceFeatures);

        cdk_log_info ("\tname: %s", deviceProperties.deviceName);
        cdk_log_info ("\ttype: %i", deviceProperties.deviceType);
        cdk_log_info ("");

        // find_queueFamily (devices[i]);

        // TODO select best device based on type and features
        // TODO select drive based on queue family and required queues
    }
    vulkanContext.physicalDevice = devices[0]; // select first device for testing

    if (vulkanContext.physicalDevice == VK_NULL_HANDLE)
    {
        cdk_log_error ("no valid GPU found!");
        return;
    }

    cdk_darray_destroy (devices); // destroy allowed, or memcpy above ?
}


void
vulkan_create_logical_device (void)
{
    cdk_log_debug ("[VULKAN] creating logical device");

    struct QueueFamilyIndices indices = find_queueFamily (vulkanContext.physicalDevice);
    
    float32 queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {0};

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphics;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {}; // no features required
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0; // TODO look into device specific extensions --> swapchain
    
    VkResult res = vkCreateDevice (vulkanContext.physicalDevice, &createInfo, NULL, &vulkanContext.device);
    if (res != VK_SUCCESS)
        cdk_log_error ("[VULKAN] error in creating logical device");

    vkGetDeviceQueue (vulkanContext.device, indices.graphics, 0, &vulkanContext.graphicsQueue);

    cdk_log_info ("[VULKAN] successfully created logical device");
}


uint8
vulkan_create_surface (PlatformState *pltState)
{
    return CDK_TRUE;
}


#endif

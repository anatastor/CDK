

#include "def.h"

#if defined(CDK_PLATFORM_WIN)

#define VK_USE_PLATFORM_WIN32_KHR

#include "core/logger.h"
#include "dataStructures/darray.h"

#include "renderer_vulkan.inl"
#include "renderer_vulkan.h"
#include "vulkan_platform.h"


static VulkanContext vulkanContext;


struct SwapChainStissl
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
} SwapChainStissl;


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
vulkan_init (PlatformState* pltState)
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
        , "VK_KHR_win32_surface"
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
    


    vulkan_create_surface (pltState);

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
    
    vkDestroySurfaceKHR (vulkanContext.instance, vulkanContext.surface, NULL);
    vkDestroyDevice (vulkanContext.device, NULL);

    vkDestroyInstance (vulkanContext.instance, NULL);
}


uint8
check_device_extension_support (VkPhysicalDevice device)
{
    const char* reqExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    uint32 extensionCount;
    vkEnumerateDeviceExtensionProperties (device, NULL, &extensionCount, NULL);
    VkExtensionProperties *availableExtensions = cdk_darray_reserve (VkExtensionProperties, extensionCount);
    cdk_darray_set_length (availableExtensions, extensionCount);
    vkEnumerateDeviceExtensionProperties (device, NULL, &extensionCount, availableExtensions);
    
    // TODO iterate over required extensions and return false if one is not found
    for (uint32 i = 0; i < extensionCount; i++)
    {
        // cdk_log_debug ("\t\t%s", availableExtensions[i].extensionName);
        if (strcmp (reqExtensions[0], availableExtensions[i].extensionName) == 0)
        {   
            cdk_log_debug ("[VULKAN] required extension found");
            return CDK_TRUE;
        }
    }
    
    cdk_darray_destroy (availableExtensions);
    cdk_log_debug ("[VULKAN] required extension NOT found");
    return CDK_FALSE;
}


uint8
is_device_suitable (VkPhysicalDevice device)
{   
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties (device, &deviceProperties);
    vkGetPhysicalDeviceFeatures (device, &deviceFeatures);

    
    uint8 swapChainAdequate = CDK_FALSE;
    if (!check_device_extension_support (device)) return CDK_FALSE;
    struct SwapChainStissl swapChainSupport = query_swapchain_support (device);
    swapChainAdequate = cdk_darray_length (swapChainSupport.formats) && cdk_darray_length (swapChainSupport.presentModes);
    
    // ???
    // check if indices are all set
    // struct QueueFamilyIndices indices = find_queueFamily (device);
    
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
    uint32 graphicFamily;
    uint32 presentFamily;
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
        cdk_log_info ("\t[VULKAN]: queueFamily: %i (flags: %u)", i, family[0].queueFlags);
        // print_bit (family[0].queueFlags);
        if (family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) // device supports graphics operations
        {    
            indices.graphicFamily = i;
            cdk_log_debug ("\t\tgraphics: %i", indices.graphicFamily);
        }

        VkBool32 presentSupport = CDK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR (device, i, vulkanContext.surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
            cdk_log_debug ("\t\tpresent: %i", indices.presentFamily);
        }
    }

    cdk_darray_destroy (family);
    return indices;
}


struct SwapChainStissl
query_swapchain_support (VkPhysicalDevice device)
{
    struct SwapChainStissl details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (device, vulkanContext.surface, &details.capabilities);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR (device, vulkanContext.surface, &formatCount, NULL);
    if (formatCount != 0)
    {
        details.formats = cdk_darray_reserve (VkSurfaceFormatKHR, formatCount);
        cdk_darray_set_length (details.formats, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR (device, vulkanContext.surface, &formatCount, details.formats);
    }

    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR (device, vulkanContext.surface, &presentModeCount, NULL);
    if (presentModeCount != 0)
    {
        details.presentModes = cdk_darray_reserve (VkSurfaceFormatKHR, formatCount);
        cdk_darray_set_length (details.presentModes, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR (device, vulkanContext.surface, &formatCount, details.presentModes);
    }

    return details;
}


void
vulkan_get_physical_device (void)
{
    cdk_log_debug ("[VULKAN] getting physical device");
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


        is_device_suitable (devices[i]);

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


uint32*
get_unique_queueFamilies (struct QueueFamilyIndices indices)
{
    uint32* queueFamilies = cdk_darray_create (uint32);
    
    if (indices.graphicFamily == indices.presentFamily)
    {   
        cdk_log_debug ("[VULKAN] graphicsFamily == presentFamily");
        cdk_darray_insert (queueFamilies, indices.graphicFamily);
    }
    else
    {
        cdk_log_debug ("[VULKAN] graphicsFamily != presentFamily");
        cdk_darray_insert (queueFamilies, indices.graphicFamily);
        cdk_darray_insert (queueFamilies, indices.presentFamily);
    }

    cdk_log_debug ("[VULKAN] %i unique queueFamilies found (%u)", cdk_darray_length (queueFamilies), queueFamilies[0]);

    return queueFamilies;
}


void
vulkan_create_logical_device (void)
{
    cdk_log_debug ("[VULKAN] creating logical device");


    const char* reqExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    struct QueueFamilyIndices indices = find_queueFamily (vulkanContext.physicalDevice);
    
    float32 queuePriority = 1.0f;
    
    uint32* uniqueQueueFamilies = get_unique_queueFamilies (indices);
    VkDeviceQueueCreateInfo* queueCreateInfos = cdk_darray_create (VkDeviceQueueCreateInfo);
    for (uint32 i = 0; i < cdk_darray_length (uniqueQueueFamilies); i++)
    {   
        VkDeviceQueueCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        createInfo.queueCount = 1;
        createInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos = cdk_darray_insert (queueCreateInfos, createInfo);
    }
    cdk_log_debug ("[VULKAN] %i queueCreateInfo generated", cdk_darray_length (queueCreateInfos));
    cdk_darray_destroy (uniqueQueueFamilies);
    

    VkPhysicalDeviceFeatures deviceFeatures = {}; // no features required
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = cdk_darray_length (queueCreateInfos);
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1; // length of reqExtensions
    createInfo.ppEnabledExtensionNames = reqExtensions;
    
    cdk_log_debug ("[VULKAN] calling create device ...");
    VkResult res = vkCreateDevice (vulkanContext.physicalDevice, &createInfo, NULL, &vulkanContext.device);
    cdk_log_debug ("[VULKAN] VkResult %i", res);
    if (res != VK_SUCCESS)
        cdk_log_error ("[VULKAN] error in creating logical device");

    cdk_log_debug ("[VULKAN] device created");

    vkGetDeviceQueue (vulkanContext.device, indices.graphicFamily, 0, &vulkanContext.graphicsQueue);
    vkGetDeviceQueue (vulkanContext.device, indices.presentFamily, 0, &vulkanContext.graphicsQueue);
    
    cdk_darray_destroy (queueCreateInfos);
    cdk_log_info ("[VULKAN] successfully created logical device");
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


#endif

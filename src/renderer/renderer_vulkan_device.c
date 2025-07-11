
#include "def.h"
#include "renderer/renderer_vulkan_device.h"

#include "core/logger.h"
#include "dataStructures/darray.h"

#include <stdlib.h> // TODO malloc in platform layer
#include <string.h> // TODO basic string comparision in utils lib?
   

static const char* _reqDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const uint8 _reqDeviceExtensionsCount = 1;


uint8 is_device_suitable (VkPhysicalDevice device, VkSurfaceKHR surface);
// uint8 create_logical_device (VulkanContext* context, VkDeviceQueueIndicies queuesIndicies);

uint32* get_unique_queueIndices (DeviceQueueIndices indices); // returns cdk_darray --> free
uint8 check_device_extension_support (VkPhysicalDevice device); 

SwapchainSupport query_swapchain_support (VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR  select_swapchain_format (VkSurfaceFormatKHR* formats);
VkPresentModeKHR    select_swapchain_presentMode (VkPresentModeKHR* presentModes);
VkExtent2D          select_swapchain_extent (VkSurfaceCapabilitiesKHR capabilities);


uint8
create_physical_device (VulkanContext* context)
{
    cdk_log_debug ("[VULKAN] getting physical device");
    context->physicalDevice = VK_NULL_HANDLE;

    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices (context->instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        cdk_log_error ("failed to find devices with vulkan support");
        return CDK_FALSE;
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices (context->instance, &deviceCount, devices);
     
    cdk_log_info ("[VULKAN]: available devices:");
    cdk_log_info ("[VULKAN]:\tID  | TYPE | NAME");
    for (uint32 i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties (devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures (devices[i], &features);

        cdk_log_info ("[VULKAN]:\t[%i] |    %i | %s",
                i,
                properties.deviceType,
                properties.deviceName);


        // VkPhysicalDeviceMemoryProperties memory;
        // vkGetPhysicalDeviceMemoryProperties (devices[i], &memory);
        // cdk_log_info ("\ttype: %i", memory.);
        

        is_device_suitable (devices[i], context->surface);

        // find_queueFamily (devices[i]);

        // TODO select best device based on type and features
        // TODO select drive based on queue family and required queues
    }


    context->physicalDevice = devices[0]; // select first device for testing

    if (context->physicalDevice == VK_NULL_HANDLE)
    {
        cdk_log_error ("no valid GPU found!");
        return CDK_FALSE;
    }

    return CDK_TRUE;
}


uint8
is_device_suitable (VkPhysicalDevice device, VkSurfaceKHR surface)
{   
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties (device, &deviceProperties);
    vkGetPhysicalDeviceFeatures (device, &deviceFeatures);

    // supports swapchain? 
    if (!check_device_extension_support (device))
        return CDK_FALSE;

    SwapchainSupport swapchainSupport = query_swapchain_support (device, surface);
    if (swapchainSupport.formatCount < 1 || swapchainSupport.presentModeCount < 1)
    {
        cdk_log_error ("no swapchain support present");
        return CDK_FALSE;
    }
    cdk_darray_destroy (swapchainSupport.formats);
    cdk_darray_destroy (swapchainSupport.presentModes);
    cdk_log_info ("swapchain is supported");   

    
    // TODO check for queueFamiliyIndices
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


uint8
create_logical_device (VulkanContext* context)
{
    cdk_log_debug ("[VULKAN] creating logical device");

    DeviceQueueIndices indices = fill_DeviceQueueIndicies (context, NULL); // TODO update to return value
    
    float32 queuePriority = 1.0f;
    
    uint32* uniqueQueueIndices = get_unique_queueIndices (indices);
    VkDeviceQueueCreateInfo* queueCreateInfos = cdk_darray_create (VkDeviceQueueCreateInfo);
    for (uint32 i = 0; i < cdk_darray_length (uniqueQueueIndices); i++)
    {   
        VkDeviceQueueCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = uniqueQueueIndices[i];
        createInfo.queueCount = 1; // TODO some graphic card support 2 queues --> multithreading
        createInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos = cdk_darray_insert (queueCreateInfos, createInfo);
    }
    cdk_log_debug ("[VULKAN] %i queueCreateInfo generated", cdk_darray_length (queueCreateInfos));
    cdk_darray_destroy (uniqueQueueIndices);
    

    VkPhysicalDeviceFeatures deviceFeatures = {}; // no features required
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = cdk_darray_length (queueCreateInfos);
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = _reqDeviceExtensionsCount; // enable swapchain extension
    createInfo.ppEnabledExtensionNames = _reqDeviceExtensions;
    
    cdk_log_debug ("[VULKAN] calling create device ...");
    VkResult res = vkCreateDevice (context->physicalDevice, &createInfo, NULL, &context->device);
    cdk_log_debug ("[VULKAN] VkResult %i", res);
    if (res != VK_SUCCESS)
        cdk_log_error ("[VULKAN] error in creating logical device");

    cdk_log_debug ("[VULKAN] device created");

    vkGetDeviceQueue (context->device, indices.graphicIndex, 0, &context->graphicsQueue);
    vkGetDeviceQueue (context->device, indices.presentIndex, 0, &context->presentQueue);
    
    cdk_darray_destroy (queueCreateInfos);
    cdk_log_info ("[VULKAN] successfully created logical device");

    return CDK_TRUE;
}


DeviceQueueIndices
fill_DeviceQueueIndicies (VulkanContext* context, DeviceQueueIndices* out)
{
    DeviceQueueIndices indices = {};
    indices.graphicIndex = -1;
    indices.presentIndex = -1;
    indices.computeIndex = -1;
    indices.transferIndex = -1;


    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties (context->physicalDevice, &deviceProperties);

    uint32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (context->physicalDevice, &familyCount, NULL);
    VkQueueFamilyProperties queueFamilies[familyCount]; // TODO check against familyCount == 0
    vkGetPhysicalDeviceQueueFamilyProperties (context->physicalDevice, &familyCount, queueFamilies);
    
    uint8 minTransferScore = 255;
    for (uint32 i = 0; i < familyCount; i++)
    {
        uint8 currTransferScore = 0;

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) // device supports graphics operations
        {    
            indices.graphicIndex = i;
            ++currTransferScore;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.computeIndex = i;
            ++currTransferScore;
        }
        
        /*
        // not yet required
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {   
            // take currTransferScore if it is the current lowest
            // increases likelyhood for a dedicated transfer queue
            if (currTransferScore <= minTransferScore)
            {
                minTransferScore = currTransferScore;
                indices.computeIndex = i;
            }
        }
        */

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR (context->physicalDevice, i, context->surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentIndex = i;
            ++currTransferScore;
        }
    }

    cdk_log_info ("[VULKAN]: Graphics | Present | Compute | Transfer | Name");
    cdk_log_info ("[VULKAN]:        %d |       %d |       %d |      %d | %s",
            indices.graphicIndex != -1,
            indices.presentIndex != -1,
            indices.computeIndex != -1,
            indices.transferIndex != -1,
            deviceProperties.deviceName);
    
    if (indices.graphicIndex == -1 || indices.presentIndex == -1)
    {
        cdk_log_fatal ("invalid QueueFamiliyIndices");
        return indices;
    }

    return indices;
}


uint32*
get_unique_queueIndices (DeviceQueueIndices indices)
{
    uint32* queueIndices = cdk_darray_create (uint32);
    
    if (indices.graphicIndex == indices.presentIndex)
    {   
        cdk_log_debug ("[VULKAN] graphicsIndex == presentIndex");
        cdk_darray_insert (queueIndices, indices.graphicIndex);
    }
    else
    {
        cdk_log_debug ("[VULKAN] graphicsIndex != presentIndex");
        cdk_darray_insert (queueIndices, indices.graphicIndex);
        cdk_darray_insert (queueIndices, indices.presentIndex);
    }

    return queueIndices;
}


uint8
check_device_extension_support (VkPhysicalDevice device)
{
    uint32 extensionCount;
    vkEnumerateDeviceExtensionProperties (device, NULL, &extensionCount, NULL);
    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties (device, NULL, &extensionCount, availableExtensions);

    for (uint8 i = 0; i < _reqDeviceExtensionsCount; i++)
    {
        uint8 isExtAvailable = CDK_FALSE;
        for (uint32 j = 0; j < extensionCount; j++)
            if (strcmp (_reqDeviceExtensions[i], availableExtensions[j].extensionName) == 0)
            {
                isExtAvailable = CDK_TRUE;
                break;
            }

        if (isExtAvailable == CDK_FALSE)
        {
            cdk_log_error ("[VULKAN]: required extension: %s not availiable", _reqDeviceExtensions[i]);
            return CDK_FALSE;
        }
    }
    
    cdk_log_debug ("[VULKAN]: all required extensions available");
    return CDK_TRUE;
}


SwapchainSupport
query_swapchain_support (VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapchainSupport details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (device, surface, &details.capabilities);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, NULL);
    if (formatCount != 0)
    {
        details.formatCount = formatCount;
        details.formats = cdk_darray_reserve (VkSurfaceFormatKHR, formatCount);
        cdk_darray_set_length (details.formats, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, details.formats);
    }

    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentModeCount, NULL);
    if (presentModeCount != 0)
    {
        details.presentModeCount = presentModeCount;
        details.presentModes = cdk_darray_reserve (VkPresentModeKHR, presentModeCount);
        cdk_darray_set_length (details.presentModes, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentModeCount, details.presentModes);
    }

    return details;
}


VkSurfaceFormatKHR
select_swapchain_format (VkSurfaceFormatKHR* formats)
{
    for (uint32 i = 0; i < cdk_darray_length (formats); i++)
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            cdk_log_info ("[VULKAN]: format %i supports BGRA8 & nonlinear colorspace");
            return formats[i];
        }

    cdk_log_warn ("[VULKAN]: no format supports BGRA8 & nonlinear colorspace; defaulting to 0");
    return formats[0];   
}


VkPresentModeKHR
select_swapchain_presentMode (VkPresentModeKHR* presentModes)
{   
    cdk_log_debug ("[VULKAN]: num of present Modes: %u", cdk_darray_length (presentModes));
    for (uint32 i = 0; i < cdk_darray_length (presentModes); i++)
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) // TODO check other presentModes
        {
            cdk_log_info ("[VULKAN]: mode %u supports MAILBOX", i);
            return presentModes[i];
        }

    cdk_log_warn ("[VULKAN]: no present mode supports XY; defaulting to 0");
    return presentModes[0];

    // return VK_PRESENT_MODE_FIFO_KHR; // tutorial ? 
}


VkExtent2D
select_swapchain_extent (VkSurfaceCapabilitiesKHR capabilities)
{   
    return capabilities.currentExtent;
    /*
     * tutorial ???
 if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
    */
}


uint8
create_swapchain (VulkanContext* context)
{   
    cdk_log_debug ("[VULKAN]: creating swapchain");
    SwapchainSupport swapchainSupport = query_swapchain_support (context->physicalDevice, context->surface);

    VkSurfaceFormatKHR surfaceFormat = select_swapchain_format (swapchainSupport.formats);
    VkPresentModeKHR presentMode = select_swapchain_presentMode (swapchainSupport.presentModes);
    VkExtent2D swapchainExtent = select_swapchain_extent (swapchainSupport.capabilities);


    uint32 minImageCount = swapchainSupport.capabilities.minImageCount;
    uint32 maxImageCount = swapchainSupport.capabilities.maxImageCount;
    cdk_log_info ("[VULKAN]: minImageCount: %i", minImageCount);
    cdk_log_info ("[VULKAN]: maxImageCount: %i", maxImageCount);

    uint32 imageCount = (maxImageCount > 0 && minImageCount + 1 <= maxImageCount) ? maxImageCount : minImageCount + 1;
    cdk_log_info ("[VULKAN]: choosen imageCount: %i", imageCount);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context->surface;
    createInfo.minImageCount = imageCount; // minImageCount might also be reasonable
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // ?
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = NULL;


    DeviceQueueIndices queueIndices = fill_DeviceQueueIndicies (context, NULL);
    uint32* uniqueIndices = get_unique_queueIndices (queueIndices);
    if (queueIndices.graphicIndex == queueIndices.presentIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // optional
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
    }
    createInfo.pQueueFamilyIndices = uniqueIndices;

    context->swapchainImageFormat = surfaceFormat.format;
    context->swapchainExtent = swapchainExtent; 

    VkResult res = vkCreateSwapchainKHR (context->device, &createInfo, NULL, &context->swapchain);
    if (res != VK_SUCCESS)
    {   
        // TODO free memory on error
        cdk_log_error ("[VULKAN]: error in creating swapchain");
        return CDK_FALSE;
    }

    vkGetSwapchainImagesKHR(context->device, context->swapchain, &imageCount, NULL);
    context->images = malloc (sizeof (VkImage) * imageCount); // TODO free ???
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &imageCount, context->images);
    context->imageCount = imageCount;

    
    cdk_darray_destroy (swapchainSupport.formats);
    cdk_darray_destroy (swapchainSupport.presentModes);
    cdk_darray_destroy (uniqueIndices);
    
    cdk_log_info ("[VULKAN]: swapchain created successfully");
    return CDK_TRUE;
}


uint8
create_imageViews (VulkanContext* context)
{   
    cdk_log_debug ("[VULKAN]: creating images");
    context->imageViews = malloc (sizeof (VkImageView) * context->imageCount);
    for (uint32 i = 0; i < context->imageCount; i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = context->images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = context->swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView (context->device, &createInfo, NULL, &context->imageViews[i]);
        if (res != VK_SUCCESS)
        {
            cdk_log_error ("[VULKAN]: unable to create image view (%i)", i);
            return CDK_FALSE;
        }
    }

    cdk_log_info ("[VULKAN]: images created successfully");
    return CDK_TRUE;
}

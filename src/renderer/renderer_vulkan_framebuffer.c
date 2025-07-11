
#include "def.h"
#include "renderer/renderer_vulkan_framebuffer.h"

#include "core/logger.h"

#include <stdlib.h>


uint8
create_framebuffer (VulkanContext* context)
{   
    context->framebuffers = malloc (sizeof (VkFramebuffer) * context->imageCount);
    for (uint32 i = 0; i < context->imageCount; i++)
    {
        VkImageView attachments[] = {context->imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = context->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = context->swapchainExtent.width;
        framebufferInfo.height = context->swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &context->framebuffers[i]) != VK_SUCCESS)
        {
            cdk_log_error ("[VULKAN]: failed to create framebuffer!");
            return CDK_FALSE;
        }
    }

    cdk_log_info ("[VULKAN]: successfully created framebuffers");
    return CDK_TRUE;
}



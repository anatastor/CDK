
#include <stdio.h>
#include <stdlib.h>

#include <def.h>
#include <core/logger.h>
#include <core/input.h>
#include <dataStructures/darray.h>

#include <platform/platform.h>

#include <renderer/renderer_vulkan.h>



#if defined(CDK_PLATFORM_WIN)
#   define PLATFORM "WINDOWS"
#elif defined(CDK_PLATFORM_LINUX)
#   define PLATFORM "LINUX"
#else
#   define PLATFORM "?"
#endif



int
main (void)
{
#if defined(CDK_PLATFORM_WIN)
    cdk_log_fatal ("PLATFORM: %s\n", PLATFORM);
#else
    cdk_log_info ("PLATFORM: %s\n", PLATFORM);
#endif
    
    input_init ();
    PlatformState pltState;
    cdk_platform_create (&pltState, 
            "Crabp", 100, 100, 600, 600);

    uint8 res = vulkan_init (&pltState);
    if (res)
        cdk_log_info ("vulkan initialized")
    else
        cdk_log_info ("error");

    
    uint8 running = CDK_TRUE;
    float64 currTime = cdk_platform_time ();
    float64 lastTime = 0;
    uint64 frameCount = 0;
    while (running)
    {   
        // TODO callback for window resize
        // TODO callback minimizing window might crash vulkan swapchain
        running = cdk_platform_update (&pltState);
        // input_update (0.f);
        uint32 x;
        uint32 y;
        // input_get_mouse_pos (&x, &y);
        // cdk_log_info ("mouse (%i, %i)", x, y);
        
        renderer_draw_frame ();
        frameCount++;

        if (frameCount % 1000 == 0)
        {
            float64 deltaTime = currTime - lastTime;
            cdk_log_debug ("avg frame time (1000 frames): %lf", (currTime - lastTime) / 1000);
            lastTime = currTime;
            currTime = cdk_platform_time ();
        }
    }

    cdk_log_debug ("closing down ...");

    vulkan_close ();
    cdk_platform_shutdown (&pltState);

    return EXIT_SUCCESS;
}

//
//#include <stdio.h>
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


#define FRAMES_PER_SECOND   60



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
    uint64 frameCount = 0;
    const float64 desiredDeltaTime = 1000.0 / FRAMES_PER_SECOND; // milliseconds

    float64 time = 0.0;
    float64 avgTime = cdk_platform_time ();
    float64 lastAvgTime = 0.0;
    // TODO callback for window resize
    // TODO callback minimizing window might crash vulkan swapchain
    while (running)
    {
        time = cdk_platform_time ();
        running = cdk_platform_update (&pltState);
        // input_update (0.f);
        uint32 x;
        uint32 y;
        // input_get_mouse_pos (&x, &y);
        // cdk_log_info ("mouse (%i, %i)", x, y);
        
        renderer_draw_frame ();
        
        float64 deltaTime = (cdk_platform_time () - time) * 1000; // milliseconds
        uint64 sleepTime = (uint64) (desiredDeltaTime - deltaTime);
        sleepTime = (sleepTime < 0 || sleepTime > 1000) ? 1 : sleepTime;
        // cdk_log_debug ("sleeping %u ms; %lf", sleepTime, deltaTime);
        cdk_platform_sleep (sleepTime);

        frameCount++;
        if (frameCount % 100 == 0)
        {   
            float64 dt = cdk_platform_time () - avgTime;
            avgTime = cdk_platform_time ();
            cdk_log_debug ("avg frame time (100 frames): %f ms", dt);
            frameCount = 0;
        }
    }

    cdk_log_debug ("closing down ...");

    vulkan_close ();
    cdk_platform_shutdown (&pltState);

    return EXIT_SUCCESS;
}

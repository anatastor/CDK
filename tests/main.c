
#include <stdio.h>
#include <stdlib.h>

#include <def.h>
#include <core/logger.h>
#include <core/input.h>
#include <dataStructures/darray.h>

#include <platform/platform.h>

// #include <renderer_vulkan.h>

// #include <vulkantest.h>


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
    printf ("PLATFORM: %s\n", PLATFORM);

    cdk_log_debug ("Test %.2f", 3.14);
    cdk_log_info ("Test %.2f", 3.14);
    cdk_log_warn ("Test %.2f", 3.14);
    cdk_log_error ("Test %.2f", 3.14);
    cdk_log_fatal ("Test %.2f", 3.14);

    printf ("void: %u\n", sizeof (void));
    printf ("int8: %u\n", sizeof (int8));
    printf ("int16: %u\n", sizeof (int16));
    printf ("int32: %u\n", sizeof (int32));
    printf ("int64: %u\n", sizeof (int64));


    printf ("\nDynamic String Array\n");
    char** cArray = cdk_darray_create (char*);
    cdk_darray_insert (cArray, &"KLAUS");
    cdk_darray_insert (cArray, &"PETER");
    for (int i = 0; i < cdk_darray_length (cArray); i++)
        printf ("i: %i\t%s\n", i, cArray[i]);

    printf ("\nDynamic Int Array\n");
    uint32* test = cdk_darray_create (uint32);
    
    test = cdk_darray_insert (test, 32);
    test = cdk_darray_insert (test, 64);
    test = cdk_darray_insert (test, 128);
    for (int i = 0; i < cdk_darray_capacity (test); i++)
        printf ("i: %i\t%u\n", i, test[i]);

    printf ("\n");
    
    
    uint32 klaus;
    test = cdk_darray_removeat (test, 1, &klaus);
    printf ("length: %u\n", cdk_darray_length (test));
    printf ("klaus: %u\n", klaus);
    for (int i = 0; i < cdk_darray_length (test); i++)
        printf ("i: %i\t%u\n", i, test[i]);

    
    /*
    uint64 res = vulkan_test ();
    cdk_log_debug ("vulkan test res: %u", res);
    */


    input_init ();
    PlatformState pltState;
    cdk_platform_create (&pltState, 
            "BIEBER", 100, 100, 200, 400);

    uint8 running = CDK_FALSE; // CDK_TRUE;

    
    /*
    uint8 res = vulkan_init ();
    if (res)
        cdk_log_info ("vulkan initialized")
    else
        cdk_log_info ("error");
    */

    
    /*
    float64 currTime = cdk_platform_time ();
    float64 lastTime = 0;
    */
    while (running)
    {   
        running = cdk_platform_update (&pltState);
        // input_update (0.f);
        uint32 x;
        uint32 y;
        // input_get_mouse_pos (&x, &y);
        // cdk_log_info ("mouse (%i, %i)", x, y);
        
        /*
        float64 deltaTime = currTime - lastTime;
        cdk_log_debug ("frame time: %lf", currTime - lastTime);
        lastTime = currTime;
        currTime = cdk_platform_time ();
        */
    }

    vulkan_close ();
    cdk_platform_shutdown (&pltState);

    test = cdk_darray_destroy (test);

    return EXIT_SUCCESS;
}

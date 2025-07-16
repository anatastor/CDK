
#pragma once

#include "def.h"
#include "core/logger.h"
//#include <vulkan/vulkan.h>


typedef struct PlatformState
{   
    void* iState;
} PlatformState;



uint8 cdk_platform_create (PlatformState* pltState,
        const char* name,
        uint32 x, uint32 y,
        uint32 width, uint32 height);


uint8 cdk_platform_update (PlatformState *pltState);


void cdk_platform_shutdown (PlatformState* pltState);

void cdk_platform_console_write (log_level level, const char* msg);

float64 cdk_platform_time (void);
void cdk_platform_sleep (uint64 miliseconds);


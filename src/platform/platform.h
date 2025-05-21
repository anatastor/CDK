
#pragma once

#include "def.h"
#include "core/logger.h"


typedef struct PlatformState
{   
    void* iState;
} PlatformState;



uint8 cdk_window_create (PlatformState* pltState,
        const char* name,
        uint32 x, uint32 y,
        uint32 width, uint32 height);


uint8 cdk_window_update (PlatformState *pltState);


void cdk_shutdown (PlatformState* pltState);

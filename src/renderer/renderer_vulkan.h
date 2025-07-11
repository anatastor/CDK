
#pragma once

#include "platform/platform.h"



uint8 vulkan_init (PlatformState* pltState);
void vulkan_close (void);


uint8 vulkan_create_surface (PlatformState *pltState);

uint8 renderer_draw_frame ();


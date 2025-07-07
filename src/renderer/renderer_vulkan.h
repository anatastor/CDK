
#pragma once

#include "platform/platform.h"



uint8 vulkan_init (PlatformState* pltState);
void vulkan_close (void);

void vulkan_get_physical_device (void);
void vulkan_create_logical_device (void);


uint8 vulkan_create_surface (PlatformState *pltState);


#pragma once

#include <stdint.h>

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned long   uint64;


typedef char    int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float   float32;
typedef double  float64;


#define CDK_TRUE    1
#define CDK_FALSE   0



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define CDK_PLATFORM_WIN
#       ifndef _WIN64
#           error "64bit Windows required!"
#       endif
#   elif defined(__linux) || defined(__gnu_linux__)
#       define CDK_PLATFORM_LINUX
#endif

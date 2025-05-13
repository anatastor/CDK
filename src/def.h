
#pragma once


typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned long   uint64;


typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef long    int64;

typedef float   float32;
typedef double  float64;


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define CDK_PLATFORM_WIN
#       ifndef _WIN64
#           error "64bit Windows required!"
#       endif
#   elif defined(__linux) || defined(__gnu_linux__)
#       define CDK_PLATFORM_LINUX
#endif

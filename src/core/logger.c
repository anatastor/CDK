
#include "logger.h"
#include "platform/platform.h"
//#include "asserts.h"


#include <stdio.h>
#include <stdarg.h>
#include <string.h> 




void
logger_init ()
{
    // TODO
}


void
logger_exit ()
{
    // TODO
}



void
logger_log (log_level level, const char *msg, ...)
{
    const char *levelStrings[] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};


    char outMsg [_LOGGER_BUFFER_SIZE];
    memset (outMsg, 0, sizeof (outMsg));


    va_list argPtr;
    va_start (argPtr, msg);
    vsnprintf (outMsg, _LOGGER_BUFFER_SIZE, msg, argPtr);
    va_end (argPtr);
    

    char logMsg[_LOGGER_BUFFER_SIZE];
    sprintf (logMsg, "%s%s\n", levelStrings[level], outMsg);
    
    cdk_platform_console_write (level, logMsg);
    // printf ("%s", logMsg);
}

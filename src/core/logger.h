
#pragma once


#define CDK_LOG_WARN  1
#define CDK_LOG_INFO  1
#define CDK_LOG_DEBUG 1
#define CDK_LOG_TRACE 1

#ifdef _DEBUG
#define CDK_LOG_DEBUG 1
#endif


#define _LOGGER_BUFFER_SIZE 32000



typedef enum log_level
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} log_level;


void logger_init ();
void logger_exit ();


void logger_log (log_level level, const char *msg, ...);


#define cdk_log_fatal(msg, ...) logger_log (LOG_LEVEL_FATAL, msg, ##__VA_ARGS__);

#ifndef KERROR
#define cdk_log_error(msg, ...) logger_log (LOG_LEVEL_ERROR, msg, ##__VA_ARGS__);
#endif

#if CDK_LOG_WARN == 1
#define cdk_log_warn(msg, ...) logger_log (LOG_LEVEL_WARN, msg, ##__VA_ARGS__);
#else
#define cdk_log_warn(msg, ...)
#endif

#if CDK_LOG_INFO == 1
#define cdk_log_info(msg, ...) logger_log (LOG_LEVEL_INFO, msg, ##__VA_ARGS__);
#else
#define cdk_log_info(msg, ...)
#endif

#if CDK_LOG_DEBUG == 1
#define cdk_log_debug(msg, ...) logger_log (LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__);
#else
#define cdk_log_debug(msg, ...)
#endif

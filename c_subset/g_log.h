/*
	Usage:
	
	g_LOG_LEVEL_TRACE     ->      g_log_trace
 	g_LOG_LEVEL_DEBUG     ->      g_log_debug
	g_LOG_LEVEL_INFO      ->      g_log_info
	g_LOG_LEVEL_WARN      ->      g_log_warn
	g_LOG_LEVEL_ERROR     ->      g_log_error
	g_LOG_LEVEL_FATAL     ->      g_log_fatal
		
	for example, a file called main.c, before include any header files, 
	choose one of the 6 above, define a macro, and use g_log_XXX:
	
		#define g_LOG_LEVEL_DEBUG
		#include "log.h"
		
		int main(){
			g_log_trace("trace mode.");           
			g_log_debug("debug mode.");           
			g_log_info("info mode.");             
		}
	
	will output like:
	[2023-05-22 00:00:00] [DEBUG] [main.c:main:5] debug mode.
	[2023-05-22 00:00:00] [INFO] [main.c:main:6] info mode.
*/
#ifndef __G_LOG_LOG_H__
#define __G_LOG_LOG_H__

#include <stdio.h>
#include <time.h>

#ifdef g_LOG_LEVEL_TRACE
#define g_LOG_ENABLED_TRACE
#define g_LOG_ENABLED_DEBUG
#define g_LOG_ENABLED_INFO
#define g_LOG_ENABLED_WARN
#define g_LOG_ENABLED_ERROR
#define g_LOG_ENABLED_FATAL

#elif defined(g_LOG_LEVEL_DEBUG)
#define g_LOG_ENABLED_DEBUG
#define g_LOG_ENABLED_INFO
#define g_LOG_ENABLED_WARN
#define g_LOG_ENABLED_ERROR
#define g_LOG_ENABLED_FATAL

#elif defined(g_LOG_LEVEL_INFO)
#define g_LOG_ENABLED_INFO
#define g_LOG_ENABLED_WARN
#define g_LOG_ENABLED_ERROR
#define g_LOG_ENABLED_FATAL

#elif defined(g_LOG_LEVEL_WARN)
#define g_LOG_ENABLED_WARN
#define g_LOG_ENABLED_ERROR
#define g_LOG_ENABLED_FATAL

#elif defined(g_LOG_LEVEL_ERROR)
#define g_LOG_ENABLED_ERROR
#define g_LOG_ENABLED_FATAL

#elif defined(g_LOG_LEVEL_FATAL)
#define g_LOG_ENABLED_FATAL
#endif

/**
* @enum log level enums. used for g_log() function below.
*/
typedef enum {
	g_log_level_trace,
	g_log_level_debug,
	g_log_level_info,
	g_log_level_warn,
	g_log_level_error,
	g_log_level_fatal
} g_log_level;

/**
* @desc get the current time string, formatted by the fmt param.
* @param[buf] the buffer to hold the final time string.
* @param[bufLen] buffer length. at least 20.
* @param[fmt] format, like "%Y-%m-%d %H:%M:%S"
*/
void g_get_now(char* buf, size_t bufLen, const char* fmt){
	time_t now;
	struct tm *tm_now;
	
	time(&now);
	tm_now = localtime(&now);
	strftime(buf, bufLen, fmt, tm_now);
}

/**
* @desc output the log message.
* @param[level] log level.
* @param[fileName] 
* @param[functionName]
* @param[lineNumber]
* @param[msg]
*/
void g_log(g_log_level level, const char* fileName, const char* functionName, const int lineNumber, const char* msg) {
	char now[20];
	g_get_now(now, sizeof(now)/sizeof(char), "%Y-%m-%d %H:%M:%S");

	switch(level){
		case g_log_level_trace:
		#ifdef g_LOG_ENABLED_TRACE
			fprintf(stdout, "[%s] [TRACE] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		case g_log_level_debug:
		#ifdef g_LOG_ENABLED_DEBUG
			fprintf(stdout, "[%s] [DEBUG] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		case g_log_level_info:
		#ifdef g_LOG_ENABLED_INFO
			fprintf(stdout, "[%s] [INFO] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		case g_log_level_warn:
		#ifdef g_LOG_ENABLED_WARN
			fprintf(stdout, "[%s] [WARN] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		case g_log_level_error:
		#ifdef g_LOG_ENABLED_ERROR
			fprintf(stderr, "[%s] [ERROR] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		case g_log_level_fatal:
		#ifdef g_LOG_ENABLED_FATAL
			fprintf(stderr, "[%s] [FATAL] [%s:%s:%d] %s\n", now, fileName, functionName, lineNumber, msg);
		#endif
			break;
		default:
			break;
	}
}

/**
* @define
* @desc 6 macros to simplify the usage of the g_log() function.
*/
#define g_log_trace(msg) \
	g_log(g_log_level_trace, __FILE__, __FUNCTION__, __LINE__, msg)
	
#define g_log_debug(msg) \
	g_log(g_log_level_debug, __FILE__, __FUNCTION__, __LINE__, msg)
	
#define g_log_info(msg) \
	g_log(g_log_level_info, __FILE__, __FUNCTION__, __LINE__, msg)
	
#define g_log_warn(msg) \
	g_log(g_log_level_warn, __FILE__, __FUNCTION__, __LINE__, msg)
	
#define g_log_error(msg) \
	g_log(g_log_level_error, __FILE__, __FUNCTION__, __LINE__, msg)
	
#define g_log_fatal(msg) \
	g_log(g_log_level_fatal, __FILE__, __FUNCTION__, __LINE__, msg)

#endif

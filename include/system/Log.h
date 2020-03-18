#ifndef LOG_H
#define LOG_H

#if !defined(LOG_HANDLE)
#define LOG_HANDLE "Project-Koi"
#endif

#ifndef ANDROID

#include <cstdio>
#include <cstdlib>

#if !defined(PANIC)
#define PANIC(...) { printf("[%s] ERROR: ", LOG_HANDLE); printf(__VA_ARGS__); printf("\n"); }
#endif

#if !defined(WARN)
#define WARN(...) { printf("[%s] WARN: ", LOG_HANDLE); printf(__VA_ARGS__); printf("\n"); }
#endif

#if !defined(INFO)
#define INFO(...) { printf("[%s] INFO: ", LOG_HANDLE); printf(__VA_ARGS__); printf("\n"); }
#endif

#if !defined(SUPPRESS_DEBUG)
#if !defined(DEBUG)
#define DEBUG(...) { printf("[%s] DEBUG: ", LOG_HANDLE); printf(__VA_ARGS__); printf("\n"); }
#endif
#else
#if !defined(DEBUG)
#define DEBUG(...)
#endif
#endif

#else

#include <android/log.h>

#if !defined(PANIC)
#define PANIC(...) __android_log_print(ANDROID_LOG_ERROR, LOG_HANDLE, __VA_ARGS__)
#endif
#if !defined(WARN)
#define WARN(...) __android_log_print(ANDROID_LOG_WARN, LOG_HANDLE, __VA_ARGS__)
#endif
#if !defined(INFO)
#define INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_HANDLE, __VA_ARGS__)
#endif
#if !defined(DEBUG)
#define DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_HANDLE, __VA_ARGS__)
#endif

#endif

#define VALIDATE(result, ...) { if(!(result)) { PANIC(__VA_ARGS__); throw std::runtime_error("helpful error message"); } }

#endif
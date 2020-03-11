#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#if !defined(LOG_HANDLE)
#define LOG_HANDLE "Project-Koi"
#endif

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

#define VALIDATE(result, ...) { if(!(result)) { PANIC(__VA_ARGS__); throw std::runtime_error("helpful error message"); } }

#endif
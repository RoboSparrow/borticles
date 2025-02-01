#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOG(sev, msg) do { fprintf(stderr, "[%s](%s:%d) %s\n", sev, __FILE__, __LINE__, msg); } while (0)
#define LOG_F(sev, fmt, ...) do { fprintf(stderr, "[%s](%s:%d) " fmt "\n", sev, __FILE__, __LINE__, __VA_ARGS__); } while (0)

#define LOG_INFO(msg) LOG("info", msg)
#define LOG_INFO_F(fmt, ...) LOG_F("info", fmt, __VA_ARGS__)

#define LOG_WARN(msg) LOG("warn", msg)
#define LOG_WARN_F(fmt, ...) LOG_F("warn", fmt, __VA_ARGS__)

#define LOG_ERROR(msg) LOG("error", msg)
#define LOG_ERROR_F(fmt, ...) LOG_F("error", fmt, __VA_ARGS__)

#define EXIT_IF(expr, msg)      \
    do {                        \
        if (expr) {             \
            LOG("Fatal", msg);  \
            exit(EXIT_FAILURE); \
        }                       \
    } while (0)

#define EXIT_IF_F(expr, fmt, ...)             \
    do {                                      \
        if (expr) {                           \
            LOG_F("Fatal", fmt, __VA_ARGS__); \
            exit(EXIT_FAILURE);               \
        }                                     \
    } while (0)

#endif

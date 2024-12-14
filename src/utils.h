#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

#define LOG_INFO(msg)                                                   \
    do {                                                                \
        fprintf(stderr, "[Info](%s:%d) %s\n", __FILE__, __LINE__, msg); \
    } while (0)

#define LOG_INFO_F(fmt, ...)                                                         \
    do {                                                                             \
        fprintf(stderr, "[Info](%s:%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define LOG_ERROR(msg)                                                   \
    do {                                                                 \
        fprintf(stderr, "[Error](%s:%d) %s\n", __FILE__, __LINE__, msg); \
    } while (0)

#define LOG_ERROR_F(fmt, ...)                                                         \
    do {                                                                              \
        fprintf(stderr, "[Error](%s:%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define EXIT_IF(expr, msg)                                                   \
    do {                                                                     \
        if (expr) {                                                          \
            fprintf(stderr, "[Fatal](%s:%d) %s\n", __FILE__, __LINE__, msg); \
            exit(EXIT_FAILURE);                                              \
        }                                                                    \
    } while (0)

#define EXIT_IF_F(expr, fmt, ...)                                                         \
    do {                                                                                  \
        if (expr) {                                                                       \
            fprintf(stderr, "[Fatal](%s:%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
            exit(EXIT_FAILURE);                                                           \
        }                                                                                 \
    } while (0)

void freez(void *ptr);
float rand_range_f(float min, float max);
char *load_file_alloc(const char *path);

// TODO this is a temp location of these defs
typedef struct {float x, y;}       vec2;
typedef struct {float x, y, z, w;} vec4;
typedef struct {float r, g, b, a;} rgba;
typedef struct {float x, y, width, height;} rect;

typedef enum {
    BUF_VERTEXES,
    BUF_INDEXES,

    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} BufferObjects;

#endif

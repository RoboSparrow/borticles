#ifndef __UTILS_H__
#define __UTILS_H__

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

#endif

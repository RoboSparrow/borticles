#ifndef __UTILS_H__
#define __UTILS_H__

#define BIT_SET(v, flag) ((v) |= (1 << (flag)))
#define BIT_CLEAR(v, flag) ((v) &= ~(1 << (flag)))
#define BIT_CHECK(v, flag) ((v) & (1 << (flag)))
#define BIT_TOGGLE(v, flag) ((v) ^= (1 << (flag)))

void freez(void *ptr);
float rand_range_f(float min, float max);
char *load_file_alloc(const char *path);

#endif

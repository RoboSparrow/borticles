#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void freez(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

char *load_file_alloc(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        LOG_ERROR_F("failed to load file '%s'", path);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);    // rewind

    char *buffer = calloc(sz, sizeof(char));
    if (!buffer) {
        LOG_ERROR_F("failed to allocate memory for file '%s'", path);
        return NULL;
    }

    size_t len = fread(buffer, sizeof(char), len, fp);
    if (ferror(fp) != 0 ) {
        LOG_ERROR_F("failed to allocate memory for file '%s'", path);
        freez(buffer);
        return NULL;
    }
    buffer[len++] = '\0';
    fclose(fp);

    return buffer;
}

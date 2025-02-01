#ifndef __ALGORITHMS_H__
#define __ALGORITHMS_H__

typedef enum {
    ALGO_NONE       = 1 << 0, // 1
    ALGO_ATTRACTION = 1 << 1, // 2
    ALGO_NOMADIC    = 1 << 2, // 4
} Algotithm;

// algorithm handlers

void bort_init_default(ShaderInfo *shader, State *state, Borticle *bort, size_t index);
void bort_update_default(ShaderInfo *shader, State *state, Borticle *bort, size_t index);

void bort_init_nomadic(ShaderInfo *shader, State *state, Borticle *bort, size_t index);
void bort_update_nomadic(ShaderInfo *shader, State *state, Borticle *bort, size_t index);
#endif

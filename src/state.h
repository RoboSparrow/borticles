// globals
#ifndef __STATE_H__
#define __STATE_H__

#include <stdio.h>

#include "borticle.h"
#include "quadtree.h"
#include "vec.h"

#define WORLD_WIDTH 800
#define WORLD_HEIGHT 600

#define POP_MAX 1000

typedef enum {
    ALGO_NONE = 1 << 0, // 1
} Algotithm;

typedef struct State {
    int width, height;

    unsigned int fps;
    unsigned int paused;

    // algorithms
    unsigned int algorithms;

    // population
    unsigned int pop_max;
    unsigned int pop_len;

    Borticle *population;
    QNode *tree;

    // vbos
    vec4 *positions;
    rgba *colors;

} State;

State *state_create();
void state_set_len(State *state, unsigned int len);
void state_destroy(State *state);

void state_print(FILE *fp, State *state);

#endif

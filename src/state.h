#ifndef __STATE_H__
#define __STATE_H__

#include <stdio.h>
#include "raylib.h"

#include "qtree/qtree.h"

#include "vec.h"
#include "borticle.h"

////
// State
////

#define WORLD_WIDTH 800
#define WORLD_HEIGHT 600

#define POP_MAX 10000

typedef struct State {
    int width, height;

    unsigned int fps;
    bool paused;

    Color bg_color;
    Color fg_color;

    // algorithms
    unsigned int algorithms; // bitflag

    // Gravitational constant G
    float grav_g;
    // Threshold for using center of mass approximation vs direct summation
    float bh_theta;

    // population
    unsigned int pop_max;
    unsigned int pop_len;

    Borticle *population;
    QTree *tree;

    // sngle borticle to track
    Borticle *selected;

    // vbos
    vec4 *positions;
    rgba *colors;

    // ui
    bool ui_minimized;

    bool ui_debug;
    bool ui_borticles;
    bool ui_qtree;

} State;

State *state_create();
void state_destroy(State *state);

void state_set_pop_len(State *state, unsigned int len);
Borticle *state_get_borticle(State *state, int index);

void state_print(FILE *fp, State *state);

////
// Algorithms
////

typedef enum {
    ALGO_NONE       = 1 << 0, // 1
    ALGO_NOMADIC    = 1 << 1, // 2
    ALGO_BARNES_HUT = 1 << 2, // 4
} Algotithm;
#define ALGO_LEN 3
extern const char *algorithms[ALGO_LEN];

// algorithm handlers

void bort_init_default(State *state, Borticle *bort, size_t index);
void bort_update_default(State *state, Borticle *bort, size_t index);

void bort_init_nomadic(State *state, Borticle *bort, size_t index);
void bort_update_nomadic(State *state, Borticle *bort, size_t index);

void bort_init_barnes_hut(State *state, Borticle *bort, size_t index);
void bort_update_barnes_hut(State *state, Borticle *bort, size_t index);
#endif

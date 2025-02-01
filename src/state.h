#ifndef __STATE_H__
#define __STATE_H__

#include <stdio.h>
#include "raylib.h"

#include "qtree/qtree.h"

#include "vec.h"
#include "borticle.h"

#define WORLD_WIDTH 800
#define WORLD_HEIGHT 600

#define POP_MAX 1000

typedef struct State {
    int width, height;

    unsigned int fps;
    unsigned int paused;

    Color bg_color;
    Color fg_color;

    // algorithms
    unsigned int algorithms;

    // Gravitational constant G
    float grav_g;

    // population
    unsigned int pop_max;
    unsigned int pop_len;

    Borticle *population;
    QTree *tree;

    // vbos
    vec4 *positions;
    rgba *colors;

} State;

State *state_create();
void state_destroy(State *state);

void state_set_pop_len(State *state, unsigned int len);
Borticle *state_get_borticle(State *state, int index);

void state_print(FILE *fp, State *state);

#endif

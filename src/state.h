// globals
#ifndef __STATE_H__
#define __STATE_H__

#include "borticle.h"
#include "vec.h"

#define WORLD_WIDTH 800
#define WORLD_HEIGHT 600

#define POP_MAX 1000


typedef struct State {
    int width, height;

    unsigned int fps;
    unsigned int paused;

    unsigned int pop_max;
    unsigned int pop_len;
    Borticle *population;

    // vbos
    vec4 *positions;
    rgba *colors;

} State;

State *state_create();
void state_set_len(State *state, unsigned int len);
void state_destroy(State *state);

void state_print(FILE *fp, State *state);

#endif

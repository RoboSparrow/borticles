#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include "external/math_3d.h"
#include "qtree/qtree.h"

#include "vec.h"
#include "utils.h"
#include "shader.h"

typedef struct State State;

typedef struct Borticle {
    unsigned int id;
    vec3_t pos, vel, acc;
    float size;  // TODO replace with pointer to vbo array
    rgba  color; // TODO replace with pointer to vbo array
} Borticle;

void bort_print(FILE *fp, Borticle *bort);

// shaders
void bort_init_shaders(ShaderInfo *shader);
void bort_init_matrices(ShaderInfo *shader, float model[4][4], float view[4][4], float projection[4][4]);
void bort_init_shaders_data(ShaderInfo *shader, State *state);
void bort_cleanup_shaders(ShaderInfo *shader);

// population
void bort_init(State *state, unsigned int start, unsigned int end);
void bort_update(State *state);
void bort_draw_2D(ShaderInfo *shader, State *state);

#endif

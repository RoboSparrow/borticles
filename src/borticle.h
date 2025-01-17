#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include "external/math_3d.h"

#include "utils.h"
#include "quadtree.h"
#include "shader.h"

typedef struct State State;

typedef struct {
    unsigned int id;
    vec3_t pos, vel, acc;
    float size;  // TODO replace with pointer to flat array
    rgba  color; // TODO replace with pointer to flat array

    QNode *quadrant; // current qtree quadrant
} Borticle;

void bort_print(FILE *fp, Borticle *bort);

// shaders
void bort_init_shaders(ShaderInfo *shader);
void bort_init_matrices(ShaderInfo *shader, float model[4][4], float view[4][4], float projection[4][4]);
void bort_init_shaders_data(ShaderInfo *shader, State *state);
void bort_cleanup_shaders(ShaderInfo *shader);

// population
void bort_init(ShaderInfo *shader, State *state);
void bort_update(ShaderInfo *shader, State *state);
void bort_draw_2D(ShaderInfo *shader, State *state);

// quadtree
void qtree_init_shaders(ShaderInfo *shader);
void qtree_draw_2D(ShaderInfo *shader, State *state);
void qtree_cleanup_shaders(ShaderInfo *shader);

// algorithms
void bort_init_default(ShaderInfo *shader, State *state, Borticle *bort, size_t index);
void bort_update_default(ShaderInfo *shader, State *state, Borticle *bort, size_t index);

void bort_init_attraction(ShaderInfo *shader, State *state, Borticle *bort, size_t index);
void bort_update_attraction(ShaderInfo *shader, State *state, Borticle *bort, size_t index);

#endif

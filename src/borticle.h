#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include <glad/glad.h>
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
} Borticle;

// shaders
void bort_init_shaders(Shader *shader);
void bort_init_matrices(Shader *shader, float model[4][4], float view[4][4], float projection[4][4]);
void bort_init_shaders_data(Shader *shader, State *state);
void bort_cleanup_shaders(Shader *shader);

// boticle population
void bort_init(Shader *shader, State *state);
void bort_update(Shader *shader, State *state);
void bort_draw_2D(Shader *shader, State *state);

//
void qtree_init_shaders(Shader *shader);
void qtree_draw_2D(Shader *shader, State *state);
void qtree_cleanup_shaders(Shader *shader);
#endif

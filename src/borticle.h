#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include <glad/glad.h>
#include "external/math_3d.h"

#include "utils.h"
#include "quadtree.h"
#include "shader.h"

typedef struct {
    unsigned int id;
    vec3_t pos, vel, acc;
    float size;  // TODO replace with pointer to flat array
    rgba  color; // TODO replace with pointer to flat array
} Borticle;

// init
void bort_init_shaders(ShaderState *state);
void bort_init_matrices(ShaderState *state, float model[4][4], float view[4][4], float projection[4][4]);
void bort_init_shaders_data(ShaderState *state, unsigned int pop_len);

// update
void bort_update(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);

// draw
void bort_draw_2D(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);

// exit
void bort_cleanup_shaders(ShaderState *state);

//
void qtree_init_shaders(ShaderState *state);
void qtree_draw_2D(QNode *tree, ShaderState *state);
void qtree_cleanup_shaders(ShaderState *state);
#endif

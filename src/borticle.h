#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include <glad/glad.h>

#include "utils.h"
#include "quadtree.h"
#include "shader.h"

typedef struct {
    unsigned int id;
    vec3 pos, vel, acc;
    float size;
    rgba  color;
} Borticle;

// init
void bort_init_shaders(ShaderState *state);
void bort_init_shaders_data(ShaderState *state, unsigned int pop_len);

// update
void bort_update(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);

// draw
void bort_draw_2D(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);
#endif

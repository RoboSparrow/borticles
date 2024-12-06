#ifndef __BORTICLE_H__
#define __BORTICLE_H__

#include <glad/glad.h>
#include "utils.h"

typedef struct {
    unsigned int id;
    vec3 pos, vel, acc;
    float size;
    rgba  color;
} Borticle;

// init
void bort_init_shaders(GLuint *program);
void bort_init_shaders_data(int *vao, int *vbo, unsigned int pop_len, float width, float height);

// update
void bort_update(Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);

// draw
void bort_draw_2D(unsigned int program, GLuint *vao, GLuint *vbo, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len);

#endif

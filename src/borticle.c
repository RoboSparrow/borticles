#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include "external/math_3d.h"

#include "utils.h"
#include "shader.h"
#include "borticle.h"

typedef enum {
    BUF_VERTEXES,
    BUF_INDEXES,

    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} BufferObjects;

void bort_init_shaders(ShaderState *state){
    GLuint vert_sh = shader_load("shaders/borticle.vert", GL_VERTEX_SHADER);
    GLuint frag_sh = shader_load("shaders/borticle.frag", GL_FRAGMENT_SHADER);
    state->program = shader_program(vert_sh, frag_sh, 0);
}

void bort_init_matrices(ShaderState *state, float model[4][4], float view[4][4], float projection[4][4]) {
    glUseProgram(state->program);
    state->loc_model      = shader_set_uniform_mat4(state->program, "model", model);
    state->loc_view       = shader_set_uniform_mat4(state->program, "view", view);
    state->loc_projection = shader_set_uniform_mat4(state->program, "projection", projection);
    glUseProgram(0);
}

void bort_init_shaders_data(ShaderState *state, unsigned int pop_len) {
    float cx  = (float) state->vp_width / 2;
    float cy  = (float) state->vp_height / 2;
    float sz = 5.f;

    float vertices[] = {
        // x    y     z
        cx     , cy + sz, 0.0f, // bottom left
        cx + sz, cy + sz, 0.0f, // bottom right
        cx + sz, cy     , 0.0f, // top right
        cx     , cy     , 0.0f  // top left

    };

    GLuint indexes[] = {
        1, 0, 2, 3
    };

    glGenVertexArrays(1, state->vao);
    glGenBuffers(BUF_NUM, state->vbo);

    // 1. bind the vao

    glBindVertexArray(state->vao[0]);

    // 2. bind the buffers

    //  - vertices
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_VERTEXES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //  - indexes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->vbo[BUF_INDEXES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), &indexes[0], GL_STATIC_DRAW);

    // - set up positions data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * pop_len, NULL, GL_DYNAMIC_DRAW);    // NULL (empty) buffer

    // - set up colors data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgba) * pop_len, NULL, GL_STREAM_DRAW);    // NULL (empty) buffer

    // 3. cleanup

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void _update_size(ShaderState *state, Borticle *bort, size_t index) {
}

static void _update_position(ShaderState *state, Borticle *bort, size_t index) {
    float dirx = (bort->vel.x > 0) ? 1 : -1;
    float diry = (bort->vel.y > 0) ? 1 : -1;

    bort->vel.x = (dirx * bort->acc.x);
    bort->vel.y = (diry * bort->acc.y);

    bort->pos.x += bort->vel.x;
    bort->pos.y += bort->vel.y;
    if (
           bort->pos.x < 0
        || bort->pos.x > state->vp_width
        || bort->pos.y < 0
        || bort->pos.y > state->vp_height
    ) {
        // reset
        bort->pos.x = state->vp_width / 2.f;
        bort->pos.y = state->vp_height / 2.f;
        bort->vel.x = rand_range_f(-10.f, 10.f);
        bort->vel.y = rand_range_f(-10.f, 10.f);
        // printf("%d p: {%f,%f}, v: {%f,%f}\n", bort->id, bort->pos.x, bort->pos.y, bort->vel.x, bort->vel.y);
    }
    // printf("%d {%f,%f}\n", bort->id, bort->pos.x, bort->pos.y);
}

static void _update_color(ShaderState *state, Borticle *bort, size_t index) {
    // float ax = fabsf(bort->pos.x);
    // float ay = fabsf(bort->pos.y);
    // bort->color.a = (ax > ay) ? 1.f - ax : 1.f - ay;
}

void bort_update(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len) {
    if (!pop) {
        return;
    }

    for (size_t i = 0; i < pop_len; i++) {
        // update bort TODO accl, vel
        _update_size(state, &pop[i], i);
        _update_position(state, &pop[i], i);
        _update_color(state, &pop[i], i);

        // update vertx data
        positions[i] = (vec4) {pop[i].pos.x, pop[i].pos.y, pop[i].pos.z, pop[i].size};
        colors[i] = pop[i].color;

        qnode_insert(tree, (vec2) {pop[i].pos.x, pop[i].pos.y}, pop[i].id);
    }
}

/**
 * prepares drawing to window
 */
void bort_draw_2D(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len) {
    if (!state || !tree) {
        return;
    }
    glUseProgram(state->program);
    glBindVertexArray(state->vao[0]);

    glVertexAttribDivisor(0, 0); // vertex
    glVertexAttribDivisor(1, 1); // positions
    glVertexAttribDivisor(2, 1); // colors

    // vertex
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_VERTEXES]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); // 3 points, float data, no rgba

    // positions
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_POSITIONS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * pop_len, &positions[0]);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_COLORS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rgba) * pop_len, &colors[0]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0, pop_len);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
    glUseProgram(0);
}

void bort_cleanup_shaders(ShaderState *state) {
    if (!state) {
        return;
    }
    glDeleteVertexArrays(1, state->vao);
    glDeleteBuffers(BUF_NUM, state->vbo);
}

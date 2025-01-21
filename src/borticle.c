#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>

#include "utils.h"
#include "log.h"

#include "state.h"
#include "algorithms.h"

#include "shader.h"
#include "borticle.h"

#include "quadtree/qnode.h"

typedef enum {
    BUF_VERTEXES,
    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} BufferObjects;

void bort_init_shaders(ShaderInfo *shader) {
    GLuint vert_sh = shader_load("shaders/borticle.vert", GL_VERTEX_SHADER);
    GLuint frag_sh = shader_load("shaders/borticle.frag", GL_FRAGMENT_SHADER);
    shader->program = shader_program(vert_sh, frag_sh, 0);
}

void bort_init_matrices(ShaderInfo *shader, float model[4][4], float view[4][4], float projection[4][4]) {
    glUseProgram(shader->program);
    shader->loc_model      = shader_set_uniform_mat4(shader->program, "model", model);
    shader->loc_view       = shader_set_uniform_mat4(shader->program, "view", view);
    shader->loc_projection = shader_set_uniform_mat4(shader->program, "projection", projection);
    glUseProgram(0);
}

void bort_init_shaders_data(ShaderInfo *shader, State *state) {
    float cx  = (float) state->width / 2;
    float cy  = (float) state->height / 2;
    float sz = 5.f;

    float vertices[] = {
        // x    y     z
        cx, cy, 0.0f,
    };

    glUseProgram(shader->program);

    glGenVertexArrays(1, shader->vao);
    glGenBuffers(BUF_NUM, shader->vbo);

    // 1. bind the vao

    glBindVertexArray(shader->vao[0]);

    // 2. bind the buffers

    //  - vertices
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_VERTEXES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // - set up positions data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * state->pop_len, NULL, GL_DYNAMIC_DRAW);    // NULL (empty) buffer

    // - set up colors data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgba) * state->pop_len, NULL, GL_STREAM_DRAW);    // NULL (empty) buffer

    // 3. cleanup

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * Initializes a poplation of borticles
 */
void bort_init(ShaderInfo *shader, State *state) {
    Borticle *bort;

    float hw = (float) state->width / 2;
    float hh = (float) state->height / 2;

    for (size_t i = 0; i < state->pop_len; i++) {
        bort = &state->population[i];

        bort->id = i;
        bort->pos = (vec3_t) {hw, hh, 0.f};
        bort->color = (rgba) {1.f, 1.f, 1.f, 1.f};
        bort->quadrant = NULL;

        if (state->algorithms == ALGO_NONE) {
            bort_init_default(shader, state, bort, i);
        }

        if ((state->algorithms & ALGO_ATTRACTION) > 0) {
            bort_init_attraction(shader, state, bort, i);
        }
    }
}

/**
 * Updates a poplation of borticles
 */
void bort_update(ShaderInfo *shader, State *state) {
    Borticle *bort;

    for (size_t i = 0; i < state->pop_len; i++) {
        bort = &state->population[i];

        if (state->algorithms == ALGO_NONE) {
            bort_update_default(shader, state, bort, i);
        }

        if ((state->algorithms & ALGO_ATTRACTION) > 0) {
            bort_update_attraction(shader, state, bort, i);
        }

        // update vertex data for vbos
        state->positions[i] = (vec4) {
            bort->pos.x,
            bort->pos.y,
            bort->pos.z,
            bort->size
        };
        state->colors[i] = bort->color;

        // insert boricle into qtree
        bort->quadrant = qnode_insert(
            state->tree,
            (vec2) {
                bort->pos.x,
                bort->pos.y
            },
            state->population[i].id
        );

        // bort_print(stdout, bort);
    }
}

/**
 * prepares drawing to window
 */
void bort_draw_2D(ShaderInfo *shader, State *state) {
    if (!shader) {
        return;
    }
    glUseProgram(shader->program);
    glBindVertexArray(shader->vao[0]);

    glVertexAttribDivisor(0, 0); // vertex
    glVertexAttribDivisor(1, 1); // positions
    glVertexAttribDivisor(2, 1); // colors

    // vertex
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_VERTEXES]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); // 3 points, float data, no rgba

    // positions
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_POSITIONS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * state->pop_len, &state->positions[0]);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[BUF_COLORS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rgba) * state->pop_len, &state->colors[0]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArraysInstanced(GL_POINTS, 0, 1, state->pop_len);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
    glUseProgram(0);
}

void bort_cleanup_shaders(ShaderInfo *shader) {
    if (!shader) {
        return;
    }
    glDeleteVertexArrays(1, shader->vao);
    glDeleteBuffers(BUF_NUM, shader->vbo);
}

void bort_print(FILE *fp, Borticle *bort) {
    if (!fp) {
        return;
    }
    if (!bort) {
        fprintf(fp, "<NULL>\n");
        return;
    }

    fprintf(fp,
        "{\n"
        "  id: %d\n"
        "  pos: {%.2f, %.2f, %.2f}\n"
        "  vel: {%.2f, %.2f, %.2f}\n"
        "  acc: {%.2f, %.2f, %.2f}\n"
        "  size: %.2f\n"
        "  color: {%.2f, %.2f, %.2f, %.2f}\n"
        ,

        bort->id,
        bort->pos.x, bort->pos.y, bort->pos.z,
        bort->vel.x, bort->vel.y, bort->vel.z,
        bort->acc.x, bort->acc.y, bort->acc.z,
        bort->size,
        bort->color.r, bort->color.g, bort->color.b, bort->color.a
    );

    if (bort->quadrant) {
        fprintf(fp,
            "  quadrant: {depth:%d, sz:%ld, area:{x:%.2f, y:%.2f, w:%.2f, h:%.2f}}\n",
            bort->quadrant->depth,
            bort->quadrant->sz,
            bort->quadrant->area.x, bort->quadrant->area.y, bort->quadrant->area.width, bort->quadrant->area.height
        );
    } else {
        fprintf(fp,
            "  quadrant: <NULL>\n"
        );
    }

    fprintf(fp, "}\n");
}

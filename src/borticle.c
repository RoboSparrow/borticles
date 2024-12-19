#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include "external/math_3d.h"

#include "utils.h"
#include "log.h"

#include "shader.h"
#include "borticle.h"

typedef enum {
    BUF_VERTEXES,
    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} BufferObjects;

void bort_init_shaders(ShaderState *state) {
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
        cx, cy, 0.0f,
    };

    glUseProgram(state->program);

    glGenVertexArrays(1, state->vao);
    glGenBuffers(BUF_NUM, state->vbo);

    // 1. bind the vao

    glBindVertexArray(state->vao[0]);

    // 2. bind the buffers

    //  - vertices
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_VERTEXES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // - set up positions data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * pop_len, NULL, GL_DYNAMIC_DRAW);    // NULL (empty) buffer

    // - set up colors data (empty)
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[BUF_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgba) * pop_len, NULL, GL_STREAM_DRAW);    // NULL (empty) buffer

    // 3. cleanup

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
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
    }
    // printf("%d {%f,%f}\n", bort->id, bort->pos.x, bort->pos.y);
}

static void _update_color(ShaderState *state, Borticle *bort, size_t index) {}

void bort_update(ShaderState *state, QNode *tree, Borticle pop[], vec4 positions[], rgba colors[], size_t pop_len) {
    if (!pop) {
        return;
    }

    for (size_t i = 0; i < pop_len; i++) {
        // update bort TODO accl, vel
        _update_size(state, &pop[i], i);
        _update_position(state, &pop[i], i);
        _update_color(state, &pop[i], i);

        // update vertex data for vbos
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

    glDrawArraysInstanced(GL_POINTS, 0, 1, pop_len);

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


////
// rendering qtree
////

#define QTREE_RENDER_MAX 1000 * 4

struct QuadArray {
    vec2 vertexes[QTREE_RENDER_MAX];
    size_t v_count;
};

static  void _make_quad_array(QNode *root, struct QuadArray *qa) {
    if (qa->v_count >= QTREE_RENDER_MAX - 1) {
        LOG_INFO_F("reached quad_array limits: %d\n", QTREE_RENDER_MAX);
        return;
    }

    vec2 nw = {
        root->area.x,
        root->area.y,
    };

    vec2 ne = {
        root->area.x + root->area.width,
        root->area.y,
    };

    vec2 se = {
        root->area.x + root->area.width,
        root->area.y + root->area.height,
    };

    vec2 sw = {
        root->area.x,
        root->area.y + root->area.height,
    };

    // nw -> ne
    qa->vertexes[qa->v_count] = nw;
    qa->v_count++;
    qa->vertexes[qa->v_count] = ne;
    qa->v_count++;

    // ne -> se
    qa->vertexes[qa->v_count] = ne;
    qa->v_count++;
    qa->vertexes[qa->v_count] = se;
    qa->v_count++;

    /*
    // se -> sw
    qa->vertexes[qa->v_count] = se;
    qa->v_count++;
    qa->vertexes[qa->v_count] = sw;
    qa->v_count++;

    // sw -> ne
    qa->vertexes[qa->v_count] = sw;
    qa->v_count++;
    qa->vertexes[qa->v_count] = nw;
    qa->v_count++;
    */

    // printf("++++ (%ld) %d, %f, %f, %f, %f\n", qa->count, root->depth, root->area.x, root->area.y, root->area.width, root->area.height);

    if(root->nw != NULL) _make_quad_array(root->nw, qa);
    if(root->ne != NULL) _make_quad_array(root->ne, qa);
    if(root->sw != NULL) _make_quad_array(root->sw, qa);
    if(root->se != NULL) _make_quad_array(root->se, qa);
}

void _print_quad_array(FILE *fp, struct QuadArray *quads) {
    if (!fp) {
        return;
    }
    if (!quads) {
        fprintf(fp, "[]\n");
        return;
    }
    fprintf(fp, "[");
    for (size_t i = 0; i < quads->v_count; i++){
        if (i % 4 == 0){
            fprintf(fp, "\n    ");
        }
        fprintf(fp, "{#:%ld, x:%f, y:%f}%s", i, quads->vertexes[i].x, quads->vertexes[i].y, (i < quads->v_count -1) ? ", " : "");
    }
    fprintf(fp, "\n]\n");
}

void qtree_init_shaders(ShaderState *state) {
    GLuint vsh = shader_load("shaders/qtree.vert", GL_VERTEX_SHADER);
    GLuint fsh = shader_load("shaders/qtree.frag", GL_FRAGMENT_SHADER);
    GLuint gsh = 0;
    state->program = shader_program(vsh, fsh, gsh);

    glUseProgram(state->program);

    glGenVertexArrays(1, state->vao);
    glGenBuffers(3, state->vbo);

    //bind the vao
    glBindVertexArray(state->vao[0]);

    // bind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * QTREE_RENDER_MAX, NULL, GL_DYNAMIC_DRAW);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * prepares drawing to window
 */
void qtree_draw_2D(QNode *tree, ShaderState *state) {
    if (!tree) {
        return;
    }

    // update
    struct QuadArray quads = {0};
    if (tree) {
        _make_quad_array(tree, &quads);
    }
    // _print_quad_array(stdout, &quads);

    // draw
    glUseProgram(state->program);
    glBindVertexArray(state->vao[0]);

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * quads.v_count, &quads.vertexes[0]);

    GLenum mode = GL_LINES;
    size_t stride = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glDrawArrays(mode, 0, quads.v_count);

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

void qtree_cleanup_shaders(ShaderState *state) {
    if (!state) {
        return;
    }
    glDeleteVertexArrays(1, state->vao);
    glDeleteBuffers(2, state->vbo);
}

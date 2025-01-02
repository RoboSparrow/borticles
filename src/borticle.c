#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include "external/math_3d.h"

#include "utils.h"
#include "log.h"

#include "shader.h"
#include "borticle.h"
#include "state.h"

typedef enum {
    BUF_VERTEXES,
    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} BufferObjects;

void bort_init_shaders(Shader *shader) {
    GLuint vert_sh = shader_load("shaders/borticle.vert", GL_VERTEX_SHADER);
    GLuint frag_sh = shader_load("shaders/borticle.frag", GL_FRAGMENT_SHADER);
    shader->program = shader_program(vert_sh, frag_sh, 0);
}

void bort_init_matrices(Shader *shader, float model[4][4], float view[4][4], float projection[4][4]) {
    glUseProgram(shader->program);
    shader->loc_model      = shader_set_uniform_mat4(shader->program, "model", model);
    shader->loc_view       = shader_set_uniform_mat4(shader->program, "view", view);
    shader->loc_projection = shader_set_uniform_mat4(shader->program, "projection", projection);
    glUseProgram(0);
}

void bort_init_shaders_data(Shader *shader, State *state) {
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

static void _update_size(Shader *shader, State *state, Borticle *bort, size_t index) {}

static void _update_position(Shader *shader, State *state, Borticle *bort, size_t index) {
    float dirx = (bort->vel.x > 0) ? 1 : -1;
    float diry = (bort->vel.y > 0) ? 1 : -1;

    bort->vel.x = (dirx * bort->acc.x);
    bort->vel.y = (diry * bort->acc.y);

    bort->pos.x += bort->vel.x;
    bort->pos.y += bort->vel.y;
    if (
           bort->pos.x < 0
        || bort->pos.x > state->width
        || bort->pos.y < 0
        || bort->pos.y > state->height
    ) {
        // reset
        bort->pos.x = state->width / 2.f;
        bort->pos.y = state->height / 2.f;
        bort->vel.x = rand_range_f(-10.f, 10.f);
        bort->vel.y = rand_range_f(-10.f, 10.f);
    }
    //printf("%d {%f,%f}\n", bort->id, bort->pos.x, bort->pos.y);
}

static void _update_color(Shader *shader, State *state, Borticle *bort, size_t index) {}

/**
 * Initializes a poplation of borticles
 */
void bort_init(Shader *shader, State *state) {
    Borticle *bort;

    float hw = (float) state->width / 2;
    float hh = (float) state->height / 2;

    for (size_t i = 0; i < state->pop_len; i++) {
        bort = &state->population[i];

        bort->id = i;
        bort->pos = (vec3_t) {hw, hh, 0.f};

        if (BIT_CHECK(state->algorithms, ALGO_NONE)) {
            bort->color = (rgba) {
                rand_range_f(0.f, 1.f),
                rand_range_f(0.f, 1.f),
                rand_range_f(0.f, 1.f),
                1.f
            };
            bort->vel = (vec3_t) {
                rand_range_f(-10.f, 10.f),
                rand_range_f(-10.f, 10.f),
                0.f
            };
            bort->acc = (vec3_t) {
                rand_range_f(0.1f, 5.f),
                rand_range_f(0.1f, 5.f),
                0.f
            };
            bort->size = rand_range_f(0.1f, 6.f);
        }
    }

}

/**
 * Updates a poplation of borticles
 */
void bort_update(Shader *shader, State *state) {
    Borticle *bort;

    for (size_t i = 0; i < state->pop_len; i++) {
        bort = &state->population[i];

        if (BIT_CHECK(state->algorithms, ALGO_NONE)) {
            // update bort TODO accl, vel
            _update_size(shader, state, bort, i);
            _update_position(shader, state, bort, i);
            _update_color(shader, state, bort, i);
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
        qnode_insert(
            state->tree,
            (vec2) {
                bort->pos.x,
                bort->pos.y
            },
            state->population[i].id
        );
    }
}

/**
 * prepares drawing to window
 */
void bort_draw_2D(Shader *shader, State *state) {
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

void bort_cleanup_shaders(Shader *shader) {
    if (!shader) {
        return;
    }
    glDeleteVertexArrays(1, shader->vao);
    glDeleteBuffers(BUF_NUM, shader->vbo);
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

void qtree_init_shaders(Shader *shader) {
    GLuint vsh = shader_load("shaders/qtree.vert", GL_VERTEX_SHADER);
    GLuint fsh = shader_load("shaders/qtree.frag", GL_FRAGMENT_SHADER);
    GLuint gsh = 0;
    shader->program = shader_program(vsh, fsh, gsh);

    glUseProgram(shader->program);

    glGenVertexArrays(1, shader->vao);
    glGenBuffers(3, shader->vbo);

    //bind the vao
    glBindVertexArray(shader->vao[0]);

    // bind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * QTREE_RENDER_MAX, NULL, GL_DYNAMIC_DRAW);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * prepares drawing to window
 */
void qtree_draw_2D(Shader *shader, State *state) {
    // update
    struct QuadArray quads = {0};
    if (state->tree) {
        _make_quad_array(state->tree, &quads);
    }
    // _print_quad_array(stdout, &quads);

    // draw
    glUseProgram(shader->program);
    glBindVertexArray(shader->vao[0]);

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * quads.v_count, &quads.vertexes[0]);

    GLenum mode = GL_LINES;
    size_t stride = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glDrawArrays(mode, 0, quads.v_count);

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

void qtree_cleanup_shaders(Shader *shader) {
    if (!shader) {
        return;
    }
    glDeleteVertexArrays(1, shader->vao);
    glDeleteBuffers(2, shader->vbo);
}

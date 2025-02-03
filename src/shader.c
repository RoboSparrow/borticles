#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

#include "qtree/qtree.h"

#include "vec.h"
#include "state.h"
#include "shader.h"

#include "log.h"
#include "utils.h"

struct Vec2List {
    vec2 *vertexes;
    size_t len;
};

unsigned int shader_load(const char *path, int type) {
    char *src = load_file_alloc(path);
    if (!src) {
        LOG_ERROR_F("Loading shader file failed: '%s'\n", path);
        return 0;
    }

    unsigned int shader = glCreateShader(type);
    const char *src_ = (const char*) src;
    glShaderSource(shader, 1, &src_, NULL);
    glCompileShader(shader);

    int success;
    char msg[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        freez(src);
        glGetShaderInfoLog(shader, 512, NULL, msg);
        LOG_ERROR_F("Creating shader '%s' failed: '%s'\n", path, msg);
        return 0;
    }

    freez(src);
    return shader;
}

int shader_set_uniform_mat4(unsigned int program, char *name, float mat[4][4]) {
    int loc = glGetUniformLocation(program, name);
    if (loc < 0) {
        LOG_ERROR_F("shader program %d: loading uniform location '%s' failed (%d)", program, name, loc);
        return loc;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
    return loc;
}

unsigned int shader_program(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader) {
    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geometryShader) {
       glAttachShader(program, geometryShader);
    }

    int success;
    char msg[512];

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(program, 512, NULL, msg);
        LOG_ERROR_F("Compiling and linking of shader program failed: '%s'\n", msg);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader) {
       glDeleteShader(geometryShader);
    }

    return program;
}

////
// quadtree render
////

#define QTREE_RENDER_MAX 24000 // fixed memory size required for setting up vbo

void qtree_init_shaders(ShaderInfo *shader) {
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

struct QuadArray {
    vec2 *vertexes;
    size_t max;
    size_t len;
};

struct QuadArray *_create_quad_array(size_t max) {
    struct QuadArray *quads = malloc(sizeof(struct QuadArray));
    EXIT_IF(quads == NULL, "could not allocate for QuadArray");

    quads->vertexes = malloc(max * sizeof(vec2));
    EXIT_IF_F(quads->vertexes == NULL, "could not allocate for max %ld vertexes in QuadArray", max);

    quads->max = max;
    quads->len = 0;

    return quads;
}

static void _fill_quad_array(QNode *root, struct QuadArray *quads) {

    if (quads->len >= QTREE_RENDER_MAX - 1) {
        return;
    }

    if (quads->len >= quads->max - 1) {
        quads->max += 16; // 4* 4
        quads->vertexes = realloc(quads->vertexes, quads->max * sizeof(struct QuadArray));
        EXIT_IF_F(quads->vertexes == NULL, "could not reallocate for max %ld vertexes in QuadArray", quads->max);
    }

    vec2 nw = root->self_nw;


    vec2 ne = (vec2) {
        root->self_se.x,
        root->self_nw.y
    };

    vec2 se = root->self_se;

    vec2 sw = (vec2) {
        root->self_nw.x,
        root->self_se.y
    };

    // nw -> ne
    quads->vertexes[quads->len] = nw;
    quads->len++;
    quads->vertexes[quads->len] = ne;
    quads->len++;

    // ne -> se
    quads->vertexes[quads->len] = ne;
    quads->len++;
    quads->vertexes[quads->len] = se;
    quads->len++;

    if(root->nw != NULL) _fill_quad_array(root->nw, quads);
    if(root->ne != NULL) _fill_quad_array(root->ne, quads);
    if(root->sw != NULL) _fill_quad_array(root->sw, quads);
    if(root->se != NULL) _fill_quad_array(root->se, quads);
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
    for (size_t i = 0; i < quads->len; i++){
        if (i % 4 == 0){
            fprintf(fp, "\n    ");
        }
        fprintf(fp, "{#:%ld, x:%f, y:%f}%s", i, quads->vertexes[i].x, quads->vertexes[i].y, (i < quads->len -1) ? ", " : "");
    }
    fprintf(fp, "\n]\n");
}

/**
 * qnode_walk(state->tree->root, _qtree_draw_quad, NULL);
*/
static void _qtree_draw_quad(QNode *node) {
   DrawRectangleLinesEx((Rectangle) {
        (int) node->self_nw.x,
        (int) node->self_nw.y,
        (int) (node->self_se.x - node->self_nw.x),
        (int) (node->self_se.y - node->self_nw.y)
    }, 0.5f, GRAY);
}
/**
 * prepares drawing to window
 */
void qtree_draw_2D(ShaderInfo *shader, State *state) {
    if (!state->tree) {
        return;
    }

    if (!state->ui_qtree) {
        return;
    }

    //qnode_walk(state->tree->root, _qtree_draw_quad, NULL);
    //return;
    // update

    struct QuadArray *quads = _create_quad_array(state->tree->length * 4);
    _fill_quad_array(state->tree->root, quads);
    // _print_quad_array(stderr, quads);

    // draw
    glUseProgram(shader->program);
    glBindVertexArray(shader->vao[0]);

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * quads->len, &quads->vertexes[0]);

    GLenum mode = GL_LINES;
    size_t stride = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glDrawArrays(mode, 0, quads->len);

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

void qtree_cleanup_shaders(ShaderInfo *shader) {
    if (!shader) {
        return;
    }
    glDeleteVertexArrays(1, shader->vao);
    glDeleteBuffers(2, shader->vbo);
}

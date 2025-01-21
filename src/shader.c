#include <stdio.h>
#include <glad/glad.h>

#include "quadtree/qlist.h"

#include "vec.h"
#include "state.h"
#include "shader.h"

#include "log.h"
#include "utils.h"

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

////
// quadtree render
////

/**
 * prepares drawing to window
 */
void qtree_draw_2D(ShaderInfo *shader, State *state) {
    // update
    struct QList quads = {0};
    if (state->tree) {
        qlist_create(state->tree, &quads);
    }
    // qlist_print(stdout, &quads);

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

void qtree_cleanup_shaders(ShaderInfo *shader) {
    if (!shader) {
        return;
    }
    glDeleteVertexArrays(1, shader->vao);
    glDeleteBuffers(2, shader->vbo);
}

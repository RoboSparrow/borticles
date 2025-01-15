#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>

typedef struct Shader {
    GLuint program;
    GLuint vao[5];
    GLuint vbo[5];

    // glGetUniformLocations
    GLint loc_model;
    GLint loc_view;
    GLint loc_projection;

    float mat_model[4][4];
    float mat_view[4][4];
    float mat_projection[4][4];
} Shader;

unsigned int shader_load(const char *path, int type);
int shader_set_uniform_mat4(unsigned int program, char *name, float mat[4][4]);
// TODO glDetachShader
unsigned int shader_program(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader);

#endif

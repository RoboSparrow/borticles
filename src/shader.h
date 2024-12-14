#ifndef __SHADER_H__
#define __SHADER_H__

typedef struct ShaderState {
    GLuint program;
    GLuint vao[5];
    GLuint vbo[5];
    int vp_width;
    int vp_height;
} ShaderState;

unsigned int shader_load(const char *path, int type);
// TODO glDetachShader
unsigned int shader_program(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader);

#endif

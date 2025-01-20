#include "utils.h"
#include "log.h"
#include "shader.h"

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

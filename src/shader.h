#ifndef __SHADER_H__
#define __SHADER_H__

unsigned int shader_load(const char *path, int type);
// TODO glDetachShader
unsigned int shader_program(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader);

#endif

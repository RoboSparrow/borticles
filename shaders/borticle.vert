#version 410 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 positions;
layout(location = 2) in vec4 colors;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;

void main() {
    color = colors;

    gl_PointSize = positions.w;

    vec4 pos = vec4(positions.xyz, 1.0);
    gl_Position = projection * view * model * pos;
}

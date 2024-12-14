#version 410 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 positions;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(vertex.xyz, 1.0);
}

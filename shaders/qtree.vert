#version 410 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // note that we read the multiplication from right to left
    vec3 pos = vertex + vec3(position, 0.0);
    //gl_Position = projection * view * scaled * vec4(vertex, 1.0);
    // vec3 pos = vec3(position, 0.0);
    gl_Position = projection * view * model * vec4(pos, 1.0); // !!
    // gl_Position = projection * view * model * vec4(vertex, 1.0);
    //gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
}

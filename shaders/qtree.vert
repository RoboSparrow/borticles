#version 410 core

layout(location = 0) in vec2 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
}

//#version 410 core
//
//in vec4 position;
//
//void main() {
//    gl_Position = vec4(position.x, position.y, position.z, position.w);
//}

#version 410 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 positions;
layout(location = 2) in vec4 colors;

out vec4 color;

void main() {
    color = colors;
    gl_Position = vec4(vertex, 1.0) + positions;
}

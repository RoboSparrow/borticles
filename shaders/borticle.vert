
#version 410 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 positions;
layout(location = 2) in vec4 colors;

out vec4 color;

void main() {
    color = colors;

    float scl = positions.w;
    mat4 scaled = mat4(scl, 0.0, 0.0, 0.0,  // 1. column
                       0.0, scl, 0.0, 0.0,  // 2. column
                       0.0, 0.0, scl, 0.0,  // 3. column
                       0.0, 0.0, 0.0, 1.0); // 4. column

    gl_Position = scaled * (vec4(vertex, 1.0) + vec4(positions.xyz, 1.0));
}

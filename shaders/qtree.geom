#version 410 core

layout (points) in;
layout (line_strip, max_vertices = 5) out;

in vec2 sizes[];

void main() {
    float x = gl_in[0].gl_Position.x;
    float y = gl_in[0].gl_Position.y;
    float w = sizes[0].x;
    float h = sizes[0].y;

    gl_Position = vec4(x,     y,      0.0, 1.0);
    EmitVertex(); // nw
    gl_Position = vec4(x + w, y,      0.0, 1.0);
    EmitVertex(); // ne
    gl_Position = vec4(x + w, y + h,  0.0, 1.0);
    EmitVertex(); // se
    gl_Position = vec4(x,     y + h,  0.0, 1.0);
    EmitVertex(); // sw
    gl_Position = vec4(x,     y,      0.0, 1.0);
    EmitVertex(); // nw closing
    EndPrimitive();
}

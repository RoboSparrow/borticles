//#version 330 core
#version 410 core

layout (points) in;
layout (line_strip, max_vertices = 4) out;

in vec4 quads[];

void main() {

    vec4 pos = vec4(quads[0].xy, 0.0, 1.0);
    float w = quads[0].z;
    float h = quads[0].w;
    gl_Position = vec4(pos.x,     pos.y,      0.0, 1.0);
    EmitVertex(); // nw
    gl_Position = vec4(pos.x + w, pos.y,      0.0, 1.0);
    EmitVertex(); // ne
    gl_Position = vec4(pos.x + w, pos.y + h,  0.0, 1.0);
    EmitVertex(); // se
    gl_Position = vec4(pos.x,     pos.y + h,  0.0, 1.0);
    EmitVertex(); // sw
    gl_Position = vec4(pos.x,     pos.y,      0.0, 1.0);
    EmitVertex(); // nw closing
    EndPrimitive();

/*
    gl_Position = vec4(0.5, 0.5, 0.0, 1.0);
    EmitVertex();
    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
*/
}

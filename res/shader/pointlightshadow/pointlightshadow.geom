#version 410 core

/**
 * Point Shadow Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out vec4 FragPos;

uniform mat4 u_shadowMatrices[6];

void main()
{
    for(int i = 0; i < 6; i++) {
        gl_Layer = i;

        for(int j = 0; j < 3; ++j) {
            FragPos = gl_in[j].gl_Position;
            gl_Position = u_shadowMatrices[i] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

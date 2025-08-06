#version 410 core

/**
 * Point Shadow Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) in vec3 aPos; // Vertex-Positionsattribut

uniform mat4 u_model; // Modell-Matrix

void main()
{
    gl_Position = u_model * vec4(aPos, 1.0);
}

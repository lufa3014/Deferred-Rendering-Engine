#version 410 core

/**
 * Null Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) in vec3 aPos; // Vertex-Positionsattribut

uniform mat4 u_projection; // Projektions-Matrix
uniform mat4 u_view; // View-Matrix
uniform mat4 u_model; // Modell-Matrix

void main()
{
    // Setzen der Position des Vertex durch Multiplikation der Matrizen
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
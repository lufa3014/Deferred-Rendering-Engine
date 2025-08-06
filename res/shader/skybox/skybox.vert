#version 410 core

/**
 * Skybox Shader.
 *
 * Autor: stud105751, stud104645
 */

layout (location = 0) in vec3 position; // Vertex-Positionsattribut

// Eigenschaften, die an den Fragmentshader weitergegeben werden sollen.
out VS_OUT {
    vec3 TexCoords; // Texturkoordinaten für den Cube-Map-Sampler
} vs_out;

uniform mat4 u_view; // View-Matrix
uniform mat4 u_projection; // Projektions-Matrix

void main()
{
    vs_out.TexCoords = position; // Übergabe der Position als Texturkoordinaten

    // View-Matrix bearbeiten, um nur die Rotationskomponenten zu verwenden
    mat4 rotView = mat4(mat3(u_view));

    // z als 1.0 um sicherzustellen, dass die Skybox im Hintergrund ist
    gl_Position = (u_projection * rotView * vec4(position, 1.0)).xyww;
}

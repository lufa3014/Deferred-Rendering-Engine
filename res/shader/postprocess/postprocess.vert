#version 410 core

/**
 * Postprocess Shader.
 *
 * Autor: stud105751, stud104645
 */

layout (location = 0) in vec2 aPos; // Vertex-Positionsattribut
layout (location = 1) in vec2 aTexCoords; // Texturkoordinaten-Attribut

out vec2 TexCoords; // Ausgabe der Texturkoordinaten an den Fragment-Shader

void main() {
    TexCoords = aTexCoords; // Übergabe der Texturkoordinaten an den Fragment-Shader
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); // Setzen der Position des Vertex
}

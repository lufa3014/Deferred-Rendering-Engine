#version 410 core

/**
 * Fog Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec3 gFinal; // Ausgabe der finalen Farbe

in vec2 TexCoords; // UV-Koordinaten des Fullscreen-Quads

uniform sampler2D u_position; // Textur mit den Positionen der Fragmente
uniform sampler2D u_normal; // Textur mit den Normalen der Fragmente
uniform sampler2D u_final; // Textur mit der finalen Farbe

uniform vec3 u_cameraPos; // Position der Kamera
uniform vec3 u_fogColor; // Farbe des Nebels
uniform float u_fogDensity; // Dichte des Nebels

void main()
{
    vec3 position = texture(u_position, TexCoords).rgb; // Fragmentposition
    vec3 normal = texture(u_normal, TexCoords).rgb; // Normalenvektor
    vec3 color = texture(u_final, TexCoords).rgb; // Finale Farbe

    if (normal == vec3(0.0)) {
        gFinal = u_fogColor; // Setzen der finalen Farbe auf die Nebelfarbe, wenn keine Normalen vorhanden sind
    } else {
        // https://www.mbsoftworks.sk/tutorials/opengl4/020-fog/
        // Exp2 Nebel-Formel: e^-(density * coord)^2
        gFinal = mix(color, u_fogColor, clamp(1.0 - exp(-pow(u_fogDensity * distance(u_cameraPos, position), 2.0)), 0.0, 1.0));
    }
}

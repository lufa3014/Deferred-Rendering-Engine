#version 410 core

/**
 * Blur Shader.
 *
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec4 FragColor; // Ausgabe der finalen Farbe

in vec2 TexCoords; // UV-Koordinaten des Fullscreen-Quads

uniform sampler2D u_image; // Eingabebild

// https://learnopengl.com/Advanced-Lighting/Bloom
uniform bool u_horizontal; // Gibt an, ob der Blur horizontal oder vertikal ist
uniform float u_weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216); // Gewichtungen für den Blur

void main() {
    vec2 texOffset = 1.0 / textureSize(u_image, 0); // Größe eines einzelnen Texels
    vec3 result = texture(u_image, TexCoords).rgb * u_weight[0]; // Beitrag des aktuellen Fragments
    if (u_horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_image, TexCoords + vec2(texOffset.x * i, 0.0)).rgb * u_weight[i]; // Beitrag der benachbarten Fragmente in horizontaler Richtung
            result += texture(u_image, TexCoords - vec2(texOffset.x * i, 0.0)).rgb * u_weight[i]; // Beitrag der benachbarten Fragmente in horizontaler Richtung
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_image, TexCoords + vec2(0.0, texOffset.y * i)).rgb * u_weight[i]; // Beitrag der benachbarten Fragmente in vertikaler Richtung
            result += texture(u_image, TexCoords - vec2(0.0, texOffset.y * i)).rgb * u_weight[i]; // Beitrag der benachbarten Fragmente in vertikaler Richtung
        }
    }

    FragColor = vec4(result, 1.0); // Setzen der finalen Farbe
}
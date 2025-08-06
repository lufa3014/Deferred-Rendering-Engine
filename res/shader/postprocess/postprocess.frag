#version 410 core

/**
 * Postprocess Shader.
 *
 * Autor: stud105751, stud104645
 */

in vec2 TexCoords; // Eingabe der Texturkoordinaten vom Vertex-Shader

uniform sampler2D u_bloom; // Bloom-Textur
uniform sampler2D u_final; // Textur der finalen gerenderten Szene

uniform float u_exposure; // Belichtungswert
uniform float u_gamma; // Gamma-Wert

void main() {
    // Abtasten der finalen Farbe und der Bloom-Farbe aus den Texturen
    vec3 color = texture(u_final, TexCoords).rgb;
    color += texture(u_bloom, TexCoords).rgb;

    // Anwenden der Belichtungskorrektur
    color = vec3(1.0) - exp(-color * u_exposure);
    // Anwenden der Gamma-Korrektur
    color = pow(color, vec3(1.0 / u_gamma));

    // Setzen der Fragmentfarbe
    gl_FragColor = vec4(color, 1.0);
}

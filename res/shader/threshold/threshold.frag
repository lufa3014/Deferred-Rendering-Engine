#version 410 core

/**
 * Threshold Shader.
 *
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec4 FragColor; // Ausgabe der Fragmentfarbe

in vec2 TexCoords; // Eingabe der Texturkoordinaten vom Vertex-Shader

uniform sampler2D u_final; // Textur der finalen gerenderten Szene
uniform sampler2D u_emission; // Emissionstextur

uniform float u_colorWeight; // Gewichtung für die finale Farbe
uniform float u_emissionWeight; // Gewichtung für die Emissionsfarbe
uniform float u_threshold; // Schwellenwert für den Bloom-Effekt

void main()
{
    // Abtasten der finalen Farbe und der Emissionsfarbe aus den Texturen
    vec3 pixelColor = texture(u_final, TexCoords).rgb;
    vec3 emissionColor = texture(u_emission, TexCoords).rgb;

    // Kombinieren der Farben mit ihren jeweiligen Gewichtungen
    vec3 bloomColor = pixelColor * u_colorWeight + emissionColor * u_emissionWeight;

    // Finden der hellsten Komponente der kombinierten Farbe
    float brightest = max(max(bloomColor.r, bloomColor.g), bloomColor.b);

    // Wenn die hellste Komponente über dem Schwellenwert liegt, wird die Bloom-Farbe ausgegeben
    // Andernfalls wird Schwarz ausgegeben
    FragColor = brightest > u_threshold ? vec4(bloomColor, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}

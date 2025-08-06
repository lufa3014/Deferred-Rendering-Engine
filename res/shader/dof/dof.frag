#version 410 core

/**
 * Depth Of Field Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec3 gFinal; // Ausgabe der finalen Farbe

in vec2 TexCoords; // UV-Koordinaten des Fullscreen-Quads

uniform sampler2D u_position; // Textur mit den Positionen der Fragmente
uniform sampler2D u_final; // Textur mit der finalen Farbe
uniform sampler2D u_finalBlur;

uniform vec3 u_cameraPos; // Position der Kamera
uniform float u_focusDistance = 15;
uniform float u_depthOfField = 12;
uniform bool u_useDoF;

void main()
{
    vec3 position = texture(u_position, TexCoords).rgb; // Fragmentposition

    // Lies die Farben aus der scharfen und der geblurten Textur
    vec3 sharpColor = texture(u_final,     TexCoords).rgb;
    vec3 blurColor  = texture(u_finalBlur, TexCoords).rgb;

    // Entfernungsberechnung zwischen Kamera und Fragment
    float dist = distance(u_cameraPos, position);

    // Wie stark sind wir von der Fokusdistanz entfernt?
    float diff = abs(dist - u_focusDistance);

    // "Halbe" DoF-Spanne zur weichen Transition
    float halfRange = u_depthOfField * 0.5;

    // Faktor für Mischung zwischen Sharp/Blur:
    //  - Ist diff < halfRange, sind wir (annähernd) im Fokus => factor ~ 0
    //  - Ist diff >> halfRange, sind wir sehr unscharf => factor ~ 1
    float factor = clamp(diff / halfRange, 0.0, 1.0);

    // Mische die scharfe Farbe mit der Blur-Farbe
    vec3 finalColor = mix(sharpColor, blurColor, factor);

    // Schreibe das Ergebnis ins Ausgabefragment
    gFinal = u_useDoF ? finalColor : sharpColor;
}

#version 410 core

/**
 * Skybox Shader.
 *
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec3 gFinal; // Ausgabe der finalen Farbe

// Eigenschaften, die vom Vertex-Shader weitergegeben wurden.
in VS_OUT {
    vec3 TexCoords; // Texturkoordinaten für den Cube-Map-Sampler
} fs_in;

uniform samplerCube u_skybox; // Cube-Map-Sampler für die Skybox
uniform sampler2D u_normal;
uniform sampler2D u_final;

uniform vec2 u_screenSize;

void main() {
    // Abtasten der Skybox-Textur mit den übergebenen Texturkoordinaten
    vec2 quadTexCoord = gl_FragCoord.xy / u_screenSize;
    gFinal = texture(u_normal, quadTexCoord).rgb == vec3(0.0) ? texture(u_final, quadTexCoord).rgb + texture(u_skybox, fs_in.TexCoords).rgb : texture(u_final, quadTexCoord).rgb;
}

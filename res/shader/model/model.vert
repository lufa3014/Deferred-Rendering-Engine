#version 410 core

/**
 * 3D Modell Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

// Eigenschaften, die an den tesc weitergegeben werden sollen.
out VS_OUT {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform vec3 u_cameraPos;

/**
 * Hauptfunktion des Vertex-Shaders.
 * Hier werden die Daten weiter gereicht.
 */
void main()
{
    vs_out.TexCoords = texCoord;

    // Wenn Model-Matrix Skalierungen enthält (insbesondere nicht-uniforme Skalierungen)
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));

    vec3 N = normalize(normalMatrix * normal);

    // Transformiere die Tangente ähnlich zur Normale
    vec3 T = normalize(normalMatrix * tangent);

    // Die Tangente T wird orthogonal zur Normalen N gemacht und normalisiert,
    // um sicherzustellen, dass T und N orthogonale Vektoren sind.
    T = normalize(T - dot(T, N) * N);

    // Berechne die Binormale als Kreuzprodukt von Tangente und Normale
    vec3 B = cross(N, T);

    // Die TBN-Matrix für die Umwandlung in den World Space
    vs_out.TBN = mat3(T, B, N);

    // Transformation der Position in den Welt-Raum.
    vs_out.Position = vec4(u_model * vec4(position, 1.0));

    vs_out.Normal = N;
    vs_out.CameraPos = u_cameraPos;

    // Position im Eye Space für die Nebelberechnung im Fragment Shader
    mat4 mv = u_view * u_model;
    vs_out.EyeSpacePosition = mv * vec4(position, 1.0);

    mat4 mvp = u_projection * mv;
    gl_Position = mvp * vec4(position, 1.0);

    vs_out.TangentCameraPos = vs_out.TBN * vs_out.CameraPos;
    vs_out.TangentFragPos = vs_out.TBN * vs_out.Position.xyz;

}

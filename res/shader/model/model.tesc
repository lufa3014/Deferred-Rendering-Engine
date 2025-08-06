#version 410 core

/**
 * 3D Modell Shader für den Tesselation Control Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann,  stud105751, stud104645
 */

layout (vertices = 3) out;

in VS_OUT  {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} tesc_in[];

out TESC_OUT {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} tesc_out[];

// Uniforms für die Tesselation
uniform bool u_doTessellation;
uniform int u_minTessellation;
uniform int u_maxTessellation;

/**
 * Berechnet das Tesselationslevel basierend auf den Abständen der Kamera zu zwei Eckpunkten.
 * Interpoliert zwischen den definierten minimalen und maximalen Tesselationsentfernungen.
 *
 * @param abstand_0 Abstand der Kamera zum ersten Eckpunkt
 * @param abstand_1 Abstand der Kamera zum zweiten Eckpunkt
 * @return Tesselationslevel
 */
float GetTessLevel(float distance_0, float distance_1)
{
    // Mittleren Abstand berechnen
    float avgDistance = (distance_0 + distance_1) / 2.0;
    float minTessDistance = 1.0f;
    float maxTessDistance = 20.0f;

    // Smoothstep für die kameraabhängige Tesselation verwenden
    float t = smoothstep(maxTessDistance, minTessDistance, avgDistance);

    // Falls tesseliert wird, wird hier zwischen der minimalen und maximalen Tesselationsstufe interpoliert
    return (u_doTessellation ? mix(float(u_minTessellation), float(u_maxTessellation), t) : 1.0f);
}

/**
 * Einsprungpunkt für den Tessellation Control Shader.
 */
void main()
{
    //Werte durchreichen
    tesc_out[gl_InvocationID].Position = tesc_in[gl_InvocationID].Position;
    tesc_out[gl_InvocationID].Normal = tesc_in[gl_InvocationID].Normal;
    tesc_out[gl_InvocationID].TexCoords = tesc_in[gl_InvocationID].TexCoords;
    tesc_out[gl_InvocationID].CameraPos = tesc_in[gl_InvocationID].CameraPos;
    tesc_out[gl_InvocationID].EyeSpacePosition = tesc_in[gl_InvocationID].EyeSpacePosition;
    tesc_out[gl_InvocationID].TangentCameraPos = tesc_in[gl_InvocationID].TangentCameraPos;
    tesc_out[gl_InvocationID].TangentFragPos = tesc_in[gl_InvocationID].TangentFragPos;

    //Weitergereicht für Normalmapping
    tesc_out[gl_InvocationID].TBN = tesc_in[gl_InvocationID].TBN;

    //Distanz zu jedem Eckpunkt des Dreiecks
    float distanceToVertex_0 = distance(tesc_in[0].CameraPos, tesc_in[0].Position.xyz);
    float distanceToVertex_1 = distance(tesc_in[1].CameraPos, tesc_in[1].Position.xyz);
    float distanceToVertex_2 = distance(tesc_in[2].CameraPos, tesc_in[2].Position.xyz);

    //Berechne die äußeren Kanten der Tessellation
    gl_TessLevelOuter[0] = GetTessLevel(distanceToVertex_1, distanceToVertex_2);
    gl_TessLevelOuter[1] = GetTessLevel(distanceToVertex_2, distanceToVertex_0);
    gl_TessLevelOuter[2] = GetTessLevel(distanceToVertex_0, distanceToVertex_1);

    //Innere Tessellation
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}

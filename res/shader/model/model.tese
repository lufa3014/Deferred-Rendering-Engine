#version 410 core

/**
 * 3D Modell Shader für den Tesselation Evaluation Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann,  stud105751, stud104645
 */

// verschiedene Unterteilungsmethoden sind möglich
// "equal_spacing" kann durch "fractional_odd_spacing" oder
// "fractional_even_spacing" ersetzt werden.
layout (triangles, equal_spacing, ccw) in;

in TESC_OUT {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} tese_in[];

out TESE_OUT {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} tese_out;

// Uniforms
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_mvpMatrix;

// Struktur für Materialeigenschaften.
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;

    bool useDiffuseMap;
    sampler2D diffuseMap;

    bool useSpecularMap;
    sampler2D specularMap;

    bool useNormalMap;
    sampler2D normalMap;

    bool useEmissionMap;
    sampler2D emissionMap;

    sampler2D displacementMap;
};

uniform Material u_material;

/** Struct für Displacement-Mapping */
struct Displacement {
    bool use;
    float factor;
};

uniform Displacement u_displacementData;

////////////////////////////////// FUNKTIONEN /////////////////////////////////

vec3 calcDisplacement(vec3 position, vec3 normal) {
    float displacement = texture(u_material.displacementMap, tese_out.TexCoords).r;
    displacement *= u_displacementData.factor;

    return position + normal * displacement * (u_displacementData.use ? 1.0f : 0.0f);
}

/**
 * Baryzentrische Interpolation von 3 4D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2)
{
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

/**
 * Baryzentrische Interpolation von 3 2D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

/**
 * Baryzentrische Interpolation von 3 3D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

/**
 * Einsprungpunkt für den Tessellation Evaluation Shader.
 */
void main() {

    // Setze die out-Variablen für den Fragment Shader
    tese_out.TexCoords = interpolate2D(tese_in[0].TexCoords, tese_in[1].TexCoords, tese_in[2].TexCoords);
    tese_out.Normal = normalize(interpolate3D(tese_in[0].Normal, tese_in[1].Normal, tese_in[2].Normal));
    tese_out.CameraPos = interpolate3D(tese_in[0].CameraPos, tese_in[1].CameraPos, tese_in[2].CameraPos);
    tese_out.EyeSpacePosition = interpolate4D(tese_in[0].EyeSpacePosition, tese_in[1].EyeSpacePosition, tese_in[2].EyeSpacePosition);

    // Position berechnen
    vec4 newPosition = interpolate4D(tese_in[0].Position, tese_in[1].Position, tese_in[2].Position);
    //displacement mapping
    newPosition = vec4(calcDisplacement(newPosition.xyz, tese_out.Normal), 1.0f);
    tese_out.Position = vec4(u_model * newPosition);
    gl_Position = u_projection * u_view * newPosition;

    tese_out.TangentCameraPos = interpolate3D(tese_in[0].TangentCameraPos, tese_in[1].TangentCameraPos, tese_in[2].TangentCameraPos);
    tese_out.TangentFragPos = interpolate3D(tese_in[0].TangentFragPos, tese_in[1].TangentFragPos, tese_in[2].TangentFragPos);

    // Tangente, Bitangente und Normale baryzentrisch interpolieren
    vec3 interpolatedT = normalize(interpolate3D(tese_in[0].TBN[0], tese_in[1].TBN[0], tese_in[2].TBN[0]));
    vec3 interpolatedB = normalize(interpolate3D(tese_in[0].TBN[1], tese_in[1].TBN[1], tese_in[2].TBN[1]));
    vec3 interpolatedN = normalize(interpolate3D(tese_in[0].TBN[2], tese_in[1].TBN[2], tese_in[2].TBN[2]));
    // Komponenten zusammenfügen
    tese_out.TBN = mat3(interpolatedT, interpolatedB, interpolatedN);
}

#version 410 core

/**
 * 3D Modell Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gAmbientShi;
layout (location = 4) out vec3 gEmission;

// Eigenschaften, die von dem Vertextshader weitergegeben wurden.
in TESE_OUT {
    vec3 CameraPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 EyeSpacePosition;
    vec4 Position;
    mat3 TBN;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} fs_in;

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

// Aktives material.
uniform Material u_material;

// Der aktuelle Render-Modus (Verwendet das `RenderMode`-Enum)
uniform int u_renderMode;

// Alpha Clipping
uniform float u_clipping;

// Normal Mapping
uniform bool u_useNormalMapping;
uniform bool u_useTwoChannelNormalMaps;

/**
 * Hauptfunktion des Fragment-Shaders.
 * Hier wird die Farbe des Fragmentes bestimmt.
 */
void main()
{
    // Erstes Alpha-Clipping: Wenn die diffuse Textur verwendet wird und die Alpha-Komponente
    // des Texturwerts unterhalb des Clipping-Schwellenwerts liegt, wird das Fragment verworfen.
    if (u_material.useDiffuseMap && texture(u_material.diffuseMap, fs_in.TexCoords).a < u_clipping)
    {
        discard;
    }

    vec3 normal = vec3(0);
    if (u_material.useNormalMap && u_useNormalMapping)
    {
        // After sampling the normal map
        normal = texture(u_material.normalMap, fs_in.TexCoords).rgb;
        normal = normal * 2.0 - 1.0;

        // Z-Komponente berechnen, falls wir einen zweikanaligen Normal Map verwenden
        if (u_useTwoChannelNormalMaps)
        {
            normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
        }

        // Transformation von Tangent Space nach World Space
        normal = normalize(fs_in.TBN * normal);
    }
    else
    {
        normal = normalize(fs_in.Normal);
    }

    vec3 albedo = (u_material.useDiffuseMap ? texture(u_material.diffuseMap, fs_in.TexCoords).rgb : vec3(1.0, 0.0, 1.0)) * u_material.diffuse;

    //- Specular
    //    Red channel: Occlusion
    //    Green channel: Roughness
    //    Blue channel: Metalness
    float metalness = (u_material.useSpecularMap ? texture(u_material.specularMap, fs_in.TexCoords).b : 0.0) * u_material.specular.b;

    vec3 ambient = (u_material.useDiffuseMap ? texture(u_material.diffuseMap, fs_in.TexCoords).rgb : vec3(1.0, 0.0, 1.0)) * u_material.ambient;

    float shininess = u_material.shininess;

    gPosition = fs_in.Position.xyz;
    gNormal = normal;
    gAlbedoSpec = vec4(albedo, metalness);
    gAmbientShi = vec4(ambient, shininess);
    gEmission = (u_material.useEmissionMap ? texture(u_material.emissionMap, fs_in.TexCoords).rgb : vec3(0.0)) * u_material.emission;
}

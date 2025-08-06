#version 410 core

/**
 * Dirlight Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec3 gFinal; // Ausgabe der finalen Farbe

in vec2 TexCoords; // UV-Koordinaten des Fullscreen-Quads

// Struktur für die Richtungslichtquelle
struct DirLight
{
    vec3 direction; // Richtung des Lichts

    vec3 ambient; // Umgebungslichtanteil
    vec3 diffuse; // Diffuses Licht
    vec3 specular; // Spekularlicht
};

uniform sampler2D u_position; // Textur mit den Positionen der Fragmente
uniform sampler2D u_normal; // Textur mit den Normalen der Fragmente
uniform sampler2D u_albedoSpec; // Textur mit Albedo- und Spekularinformationen
uniform sampler2D u_ambientShi; // Textur mit Ambient- und Shininessinformationen
uniform sampler2D u_emission; // Textur mit Emissionsinformationen
uniform sampler2D u_shadowMap;

uniform DirLight u_dirLight; // Richtungslichtquelle

uniform mat4 u_lightSpace;

uniform vec3 u_cameraPos; // Position der Kamera

uniform bool u_isActive;
uniform bool u_showShadows;
uniform bool u_usePCF;

// Berechnung des Lichts für die Richtungslichtquelle
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambientMap, vec3 diffuseMap, vec3 specularMap, float shininess, float shadow)
{
    vec3 lightDir   = normalize(light.direction);

    float diff = max(dot(lightDir, normal), 0.0); // Berechnung des diffusen Anteils

    vec3 reflectDir = reflect(-lightDir, normal); // Berechnung der Reflexionsrichtung
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); // Berechnung des spekularen Anteils

    vec3 ambient  = light.ambient         * ambientMap; // Berechnung des Umgebungslichts
    vec3 diffuse  = light.diffuse  * diff * diffuseMap; // Berechnung des diffusen Lichts
    vec3 specular = light.specular * spec * specularMap; // Berechnung des spekularen Lichts

    return (1.0 - shadow) * (diffuse + specular); // Summe der Lichtanteile
}

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float CalcShadows(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, bool usePCF)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    float bias = max(.0005 * (1.0 - dot(normal, lightDir)), .00025);

    float shadow = 0;
    if (usePCF) {
        vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    } else {
        shadow  = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    }

    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

void main()
{
    vec3 fragPos    = texture(u_position, TexCoords).rgb; // Fragmentposition
    vec3 normal     = normalize(texture(u_normal, TexCoords).rgb); // Normalenvektor
    vec3 albedo     = texture(u_albedoSpec, TexCoords).rgb; // Albedo-Informationen
    vec3 ambient    = texture(u_ambientShi, TexCoords).rgb;
    vec3 specular   = texture(u_albedoSpec, TexCoords).aaa; // Spekularinformationen
    vec3 emission   = texture(u_emission, TexCoords).rgb; // Emissionsinformationen
    vec3 lightDir   = normalize(u_dirLight.direction);

    vec4 fragPosLightSpace = u_lightSpace * vec4(fragPos, 1.0);

    float shininess = texture(u_ambientShi, TexCoords).a;

    vec3 viewDir = normalize(u_cameraPos - fragPos); // Blickrichtung

    float shadow = u_showShadows ? CalcShadows(fragPosLightSpace, normal, lightDir, u_usePCF) : 0;
    vec3 light = u_isActive ? CalcDirLight(u_dirLight, normal, viewDir, ambient, albedo, specular, shininess, shadow) : vec3(0.0);
    gFinal = (emission != vec3(0.0) ? emission : light);
}

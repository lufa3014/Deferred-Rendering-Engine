#version 410 core

/**
 * Pointlight Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) out vec3 gFinal; // Ausgabe der finalen Farbe

// Struktur für die Eigenschaften eines Punktlichts
struct PointLight
{
    vec3 position; // Position des Punktlichts

    vec3 ambient; // Umgebungslichtanteil
    vec3 diffuse; // Diffuses Licht
    vec3 specular; // Spekulares Licht

    float constant; // Konstante Abschwächung
    float linear; // Lineare Abschwächung
    float quadratic; // Quadratische Abschwächung
};

in vec2 TexCoords;

uniform sampler2D u_position; // Textur mit den Positionen der Fragmente
uniform sampler2D u_normal; // Textur mit den Normalen der Fragmente
uniform sampler2D u_albedoSpec; // Textur mit Albedo- und Spekularinformationen
uniform sampler2D u_ambientShi; // Textur mit Ambient- und Shininessinformationen
uniform sampler2D u_emission; // Emissionstextur
uniform samplerCube u_shadowMap;

uniform PointLight u_pointLight; // Punktlicht-Uniform

uniform vec3 u_cameraPos; // Position der Kamera

uniform float u_zFar;
uniform bool u_showShadows;
uniform bool u_usePCF;


uniform bool u_isActive;

// Array aus Offset Richtungen für das Samplen, jedes zeigt in eine
// komplett andere Richtung (weniger Redundanz)
vec3 gridsamplingDisk[20] = vec3[] (
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// Berechnet die Beleuchtung durch ein Punktlicht
vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 ambientMap, vec3 diffuseMap, vec3 specularMap, float shininess, float shadow)
{

    vec3 lightDir = normalize(light.position - fragPos);// Richtung zum Licht

    float diff = max(dot(normal, lightDir), 0.0);// Diffuse Beleuchtung

    vec3 reflectDir = reflect(-lightDir, normal);// Reflektierte Richtung
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);// Spekulare Beleuchtung

    float distance = length(light.position - fragPos);// Entfernung zum Licht
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));// Abschwächung

    vec3 ambient  = light.ambient         * ambientMap;// Umgebungslicht
    vec3 diffuse  = light.diffuse  * diff * diffuseMap;// Diffuses Licht
    vec3 specular = light.specular * spec * specularMap;// Spekulares Licht

    ambient  *= attenuation;// Abschwächung anwenden
    diffuse  *= attenuation;// Abschwächung anwenden
    specular *= attenuation;// Abschwächung anwenden


    return (ambient + (1.0 - shadow) * (diffuse + specular)); // Gesamte Beleuchtung
}

//https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
float CalcShadows(vec3 fragPos, vec3 lightPos, bool usePCF)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    float bias = 0.05;
    float shadow = 0;
    if (usePCF) {
        int samples  = 20;

        float viewDistance = length(u_cameraPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / u_zFar)) / 25.0;

        for(int i = 0; i < samples; ++i)
        {
            float closestDepth = texture(u_shadowMap, fragToLight + gridsamplingDisk[i] * diskRadius).r;
            closestDepth *= u_zFar;
            if(currentDepth - bias > closestDepth) {
                shadow += 1.0;
            }
        }
        shadow /= float(samples);
    } else {
        // use the light to fragment vector to sample from the depth map
        float closestDepth = texture(u_shadowMap, fragToLight).r;
        // it is currently in linear range between [0,1]. Re-transform back to original value
        closestDepth *= u_zFar;

        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }

    return shadow;
}

void main()
{

    vec3 fragPos    = texture(u_position, TexCoords).rgb; // Fragmentposition
    vec3 normal     = normalize(texture(u_normal, TexCoords).rgb); // Normalenvektor
    vec3 albedo     = texture(u_albedoSpec, TexCoords).rgb; // Albedo
    vec3 ambient    = texture(u_ambientShi, TexCoords).rgb;
    vec3 specular   = texture(u_albedoSpec, TexCoords).aaa; // Spekularanteil
    vec3 emission   = texture(u_emission, TexCoords).rgb; // Emission

    float shininess = texture(u_ambientShi, TexCoords).a;

    vec3 viewDir = normalize(u_cameraPos - fragPos); // Blickrichtung

    float shadow = u_showShadows ? CalcShadows(fragPos, u_pointLight.position, u_usePCF) : 0;
    vec3 light = u_isActive ? CalcPointLight(u_pointLight, fragPos, normal, viewDir, ambient, albedo, specular, shininess, shadow) : vec3(0.0);
    gFinal = emission != vec3(0.0) ? emission : light;
}

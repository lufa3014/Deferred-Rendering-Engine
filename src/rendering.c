/**
 * Modul zum Rendern der 3D Szene.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#include "rendering.h"

#include <string.h>

#include "shader.h"
#include "model.h"
#include "utils.h"
#include "input.h"
#include "camera.h"
#include "texture.h"
#include "gbuffer.h"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

/**
 * Struktur zur Speicherung der Transformationsdaten.
 */
typedef struct Transform {
    vec3 translation; /**< Die Translation der Transformation. */
    vec3 rotation; /**< Die Rotation der Transformation. */
    vec3 scale; /**< Die Skalierung der Transformation. */
} Transform;

/**
 * Struktur zur Speicherung der Daten einer Skybox.
 */
typedef struct Skybox {
    bool skyboxEnabled; /**< Gibt an, ob die Skybox aktiviert ist. */
    Shader *shader; /**< Der Shader der Skybox. */
    GLuint cubemapTexture; /**< Die Textur der Skybox. */
    GLuint skyboxVAO, skyboxVBO; /**< Vertex Array Object und Vertex Buffer Object der Skybox. */
    GLsizei skyboxVertexCount; /**< Anzahl der Vertices der Skybox. */
} Skybox;

/**
 * Struktur zur Speicherung der Nebeldaten.
 */
typedef struct Fog {
    Shader *shader; /**< Der Shader für den Nebel. */
    bool fogEnabled; /**< Gibt an, ob der Nebel aktiviert ist. */
    float fogDensity; /**< Die Dichte des Nebels. */
    vec3 color; /**< Die Farbe des Nebels. */
} Fog;

/**
 * Struktur zur Speicherung der Normal Map-Daten.
 */
typedef struct NormalMap {
    bool enableNormalMapping; /**< Gibt an, ob Normal Mapping aktiviert ist. */
    bool enableTwoChannelNormalMap; /**< Gibt an, ob Zwei-Kanal-Normal Maps aktiviert sind. */
} NormalMap;

/**
 * Struktur zur Speicherung der Tesselationsdaten.
 */
typedef struct Tesselation {
    bool useTessellation; /**< Gibt an, ob Tesselation verwendet wird. */
    int minTessellation; /**< Minimale Tesselationsstufe. */
    int maxTessellation; /**< Maximale Tesselationsstufe. */
} Tesselation;

/**
 * Struktur zur Speicherung der Displacement-Daten.
 */
typedef struct Displacement {
    bool useDisplacement; /**< Gibt an, ob Displacement verwendet wird. */
    float displacementFactor; /**< Der Displacement-Faktor. */
} Displacement;

/**
 * Struktur zur Speicherung der Daten eines Fullscreen-Quads.
 */
typedef struct FullscreenQuad {
    GLuint quadVAO, quadVBO; /**< Vertex Array Object und Vertex Buffer Object des Fullscreen-Quads. */
} FullscreenQuad;

/**
 * Struktur zur Speicherung der Bloom-Daten.
 */
typedef struct Bloom {
    float colorWeight; /**< Farbgewicht des Bloom-Effekts. */
    float emissionWeight; /**< Emissionsgewicht des Bloom-Effekts. */
    float threshold; /**< Schwellenwert des Bloom-Effekts. */
    int blurIterations; /**< Anzahl der Iterationen für den Bloom-Blur. */
} Bloom;

/**
 * @brief Struktur zur Speicherung der Gamma-Korrektur- und Bloom-Daten für die Nachbearbeitung.
 */
typedef struct Postprocessing {
    float exposure; /**< Die Belichtungseinstellung für die Gamma-Korrektur. */
    float gamma; /**< Der Gamma-Wert für die Bildkorrektur. */
    Bloom bloom; /**< Die Bloom-Effekt-Daten für die Nachbearbeitung. */

    bool useDoF; /**< Gibt an, ob Tiefenunschärfe verwendet werden soll. */
    float focusDistance; /**< Die Fokusdistanz für die Tiefenunschärfe. */
    float depthOfField; /**< Die Stärke der Tiefenunschärfe. */
} Postprocessing;

/**
 * @brief Struktur zur Speicherung der Schattenkarten-Daten.
 */
typedef struct ShadowMap {
    bool needsUpdating; /**< Gibt an, ob die Schattenkarte aktualisiert werden muss. */
    float quadSize; /**< Die Größe des Schattenerfassungsbereichs. */
    float zNear; /**< Der nahe Z-Wert für die Schattenprojektion. */
    float zFar; /**< Der ferne Z-Wert für die Schattenprojektion. */
    mat4* cubemapMatrices; /**< Die Matrizen für die Schattenwürfelkarte. */

    bool usePCF; /**< Gibt an, ob Percentage Closer Filtering (PCF) verwendet wird. */
    bool showShadows; /**< Gibt an, ob Schatten angezeigt werden sollen. */
    bool dirLightShadowsAlwaysUpdate; /**< Gibt an, ob Richtungslicht-Schatten immer aktualisiert werden. */
    bool dirLightShadowsShouldUpdate; /**< Gibt an, ob Richtungslicht-Schatten aktualisiert werden sollen. */
    bool pointLightShadowsShouldUpdate; /**< Gibt an, ob Punktlicht-Schatten aktualisiert werden sollen. */
} ShadowMap;

/**
 * @brief Struktur zur Speicherung der Beleuchtungsdaten.
 */
typedef struct Light {
    Shader *pointlightShader; /**< Der Shader für Punktlichter. */
    PointLight *defaultPointLight; /**< Das Standard-Punktlicht. */
    bool isPointLightActive; /**< Gibt an, ob das Punktlicht aktiv ist. */

    Shader *dirlightShader; /**< Der Shader für Richtungslichter. */
    DirLight *defaultDirLight; /**< Das Standard-Richtungslicht. */
    bool isDirLightActive; /**< Gibt an, ob das Richtungslicht aktiv ist. */
    float dirLightDistanceMult; /**< Der Multiplikator für die Lichtdistanz. */

    bool hasCreatedDefaultPointLights; /**< Gibt an, ob ein Standard-Punktlicht erstellt wurde. */
    bool hasCreatedDefaultDirLights; /**< Gibt an, ob ein Standard-Richtungslicht erstellt wurde. */
} Light;

/**
 * @brief Struktur zur Speicherung aller für das Rendering erforderlichen Daten.
 */
struct RenderingData {
    Shader *modelShader; /**< Der Shader für das Modell-Rendering. */
    Shader *nullShader; /**< Ein Null-Shader für Debugging-Zwecke. */
    Shader *blurShader; /**< Der Shader für den Blur-Effekt. */
    Shader *thresholdShader; /**< Der Shader für die Schwellenwertberechnung. */
    Shader *postprocessShader; /**< Der Shader für die Nachbearbeitungseffekte. */
    Shader *dirLightShadowShader; /**< Der Shader für Richtungslicht-Schatten. */
    Shader *pointLightShadowShader; /**< Der Shader für Punktlicht-Schatten. */
    Shader *depthOfFieldShader; /**< Der Shader für Tiefenunschärfe-Effekte. */

    RenderMode renderMode; /**< Der aktuelle Rendering-Modus. */
    Model* lightVolume; /**< Das Modell für Lichtvolumina. */
    float clipping; /**< Der Clipping-Wert für die Kamerasicht. */
    Transform transform; /**< Die Transformationsdaten für die Szene. */
    Skybox skybox; /**< Die Skybox-Daten für die Umgebung. */
    Fog fog; /**< Die Nebeleinstellungen für die Szene. */
    NormalMap normalMap; /**< Die Normal Map-Daten für die Oberflächenstruktur. */
    Tesselation tesselation; /**< Die Tesselationsdaten zur Verfeinerung der Geometrie. */
    Displacement displacement; /**< Die Displacement-Daten zur Höhenanpassung der Geometrie. */
    FullscreenQuad fullscreenQuad; /**< Das Fullscreen-Quad für Post-Processing. */
    Postprocessing postprocessing; /**< Die Nachbearbeitungsdaten für Effekte wie Gamma und Bloom. */
    Light light; /**< Die Lichtdaten für die Szene. */
    ShadowMap shadowMap; /**< Die Schattenkartendaten für Lichtquellen. */
    GBuffer *gbuffer; /**< Der G-Buffer zur Speicherung von Szeneninformationen. */
};

typedef struct RenderingData RenderingData;

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Lädt alle Shader, die im Rendering-Modul verwendet werden.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 */
static void rendering_loadShaders(RenderingData *data) {
    if (data == NULL) { return; }

    data->modelShader = shader_createVeTessFrShader("Model",
                                                    UTILS_CONST_RES("shader/model/model.vert"),
                                                    UTILS_CONST_RES("shader/model/model.tesc"),
                                                    UTILS_CONST_RES("shader/model/model.tese"),
                                                    UTILS_CONST_RES("shader/model/model.frag")
    );

    data->skybox.shader = shader_createVeFrShader("Skybox",
                                                  UTILS_CONST_RES("shader/skybox/skybox.vert"),
                                                  UTILS_CONST_RES("shader/skybox/skybox.frag")
    );

    data->nullShader = shader_createVeFrShader("Null",
                                               UTILS_CONST_RES("shader/null/null.vert"),
                                               UTILS_CONST_RES("shader/null/null.frag")
    );

    data->light.pointlightShader = shader_createVeFrShader("Pointlight",
                                                           UTILS_CONST_RES("shader/pointlight/pointlight.vert"),
                                                           UTILS_CONST_RES("shader/pointlight/pointlight.frag")
    );

    data->light.dirlightShader = shader_createVeFrShader("Dirlight",
                                                         UTILS_CONST_RES("shader/dirlight/dirlight.vert"),
                                                         UTILS_CONST_RES("shader/dirlight/dirlight.frag")
    );

    data->blurShader = shader_createVeFrShader("Blur",
                                               UTILS_CONST_RES("shader/blur/blur.vert"),
                                               UTILS_CONST_RES("shader/blur/blur.frag")
    );

    data->thresholdShader = shader_createVeFrShader("Threshold",
                                                    UTILS_CONST_RES("shader/threshold/threshold.vert"),
                                                    UTILS_CONST_RES("shader/threshold/threshold.frag")
    );

    data->fog.shader = shader_createVeFrShader("Fog",
                                               UTILS_CONST_RES("shader/fog/fog.vert"),
                                               UTILS_CONST_RES("shader/fog/fog.frag")
    );

    data->postprocessShader = shader_createVeFrShader("Postprocess",
                                                      UTILS_CONST_RES("shader/postprocess/postprocess.vert"),
                                                      UTILS_CONST_RES("shader/postprocess/postprocess.frag")
    );

    data->dirLightShadowShader = shader_createVeFrShader("DirLightShadow",
                                                         UTILS_CONST_RES("shader/dirshadow/dirshadow.vert"),
                                                         UTILS_CONST_RES("shader/dirshadow/dirshadow.frag")
    );

    data->pointLightShadowShader = shader_createVeGeomFrShader("PointLightShadow",
                                                               UTILS_CONST_RES("shader/pointlightshadow/pointlightshadow.vert"),
                                                               UTILS_CONST_RES("shader/pointlightshadow/pointlightshadow.geom"),
                                                               UTILS_CONST_RES("shader/pointlightshadow/pointlightshadow.frag")
    );

    data->depthOfFieldShader = shader_createVeFrShader("DepthOfField",
                                                       UTILS_CONST_RES("shader/dof/dof.vert"),
                                                       UTILS_CONST_RES("shader/dof/dof.frag")
);
}

/**
 * Zeichnet die Skybox, wenn das Skybox-Rendering aktiviert ist.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param viewMatrix Die View-Matrix der Szene.
 * @param projectionMatrix Die Projection-Matrix der Szene.
 */
static void rendering_drawSkybox(const RenderingData *data, mat4 viewMatrix, mat4 projectionMatrix) {
    shader_useShader(data->skybox.shader);

    // Nur die Rotationskomponenten der View-Matrix verwenden
    mat4 skyboxView;
    glm_mat4_copy(viewMatrix, skyboxView);

    // Translation entfernen
    mat3 rotation;
    glm_mat4_pick3(viewMatrix, rotation);
    glm_mat4_identity(skyboxView);
    glm_mat4_ins3(rotation, skyboxView);

    shader_setMat4(data->skybox.shader, "u_view", &skyboxView);
    shader_setMat4(data->skybox.shader, "u_projection", (mat4 *) projectionMatrix);

    glDepthFunc(GL_LEQUAL); // Sicherstellen, dass die Skybox im Hintergrund bleibt

    // Skybox-Textur und VAO binden und zeichnen
    glBindVertexArray(data->skybox.skyboxVAO);

    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_NUM_COLORATTACH);
    glBindTexture(GL_TEXTURE_CUBE_MAP, data->skybox.cubemapTexture);
    shader_setInt(data->skybox.shader, "u_skybox", DEFAULT_GBUFFER_NUM_COLORATTACH);

    glDrawArrays(GL_TRIANGLES, 0, data->skybox.skyboxVertexCount);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS); // Tiefentest zurücksetzen
}

/**
 * Aktualisiert die Uniforms des Model-Shaders.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 */
static void rendering_updateUniforms(RenderingData *data) {
    shader_useShader(data->modelShader);

    // ModelMatrix festlegen.
    {
        mat4 modelMatrix;
        glm_mat4_identity(modelMatrix);
        glm_translate(modelMatrix, data->transform.translation);
        glm_rotate_x(modelMatrix, glm_rad(data->transform.rotation[0]), modelMatrix);
        glm_rotate_y(modelMatrix, glm_rad(data->transform.rotation[1]), modelMatrix);
        glm_rotate_z(modelMatrix, glm_rad(data->transform.rotation[2]), modelMatrix);
        glm_scale(modelMatrix, data->transform.scale);
        shader_setMat4(data->modelShader, "u_model", &modelMatrix);
    }

    // Render-Modus
    shader_setInt(data->modelShader, "u_renderMode", data->renderMode);

    // Clipping-Daten
    shader_setFloat(data->modelShader, "u_clipping", data->clipping);

    // Fog-Daten
    {
        shader_setBool(data->modelShader, "u_fogEnabled", data->fog.fogEnabled);
        shader_setFloat(data->modelShader, "u_fogDensity", data->fog.fogDensity);
        if (data->fog.fogEnabled) {
            shader_setVec3(data->modelShader, "u_fogColor", &data->fog.color);
        }
    }

    // Normal Map-Daten
    {
        shader_setBool(data->modelShader, "u_useNormalMapping", data->normalMap.enableNormalMapping);
        shader_setBool(data->modelShader, "u_useTwoChannelNormalMaps", data->normalMap.enableTwoChannelNormalMap);
    }

    //Tessellation-Daten
    {
        shader_setBool(data->modelShader, "u_doTessellation", data->tesselation.useTessellation);
        shader_setInt(data->modelShader, "u_minTessellation", data->tesselation.minTessellation);
        shader_setInt(data->modelShader, "u_maxTessellation", data->tesselation.maxTessellation);
    }

    //Displacement-Daten
    {
        shader_setBool(data->modelShader, "u_displacementData.use", data->displacement.useDisplacement);
        shader_setFloat(data->modelShader, "u_displacementData.factor", data->displacement.displacementFactor);
    }
}

/**
 * Setzt die Uniforms für das Punktlicht.
 *
 * @param shader Der Shader, der die Uniforms setzen soll.
 * @param light Das Punktlicht, dessen Uniforms gesetzt werden sollen.
 */
static void setPointLightUniforms(Shader *shader, PointLight light) {
    shader_setVec3(shader, "u_pointLight.position", &light.position);

    shader_setVec3(shader, "u_pointLight.ambient", &light.ambient);
    shader_setVec3(shader, "u_pointLight.diffuse", &light.diffuse);
    shader_setVec3(shader, "u_pointLight.specular", &light.specular);

    shader_setFloat(shader, "u_pointLight.constant", light.constant);
    shader_setFloat(shader, "u_pointLight.linear", light.linear);
    shader_setFloat(shader, "u_pointLight.quadratic", light.quadratic);
}

/**
 * Setzt die Uniforms für das Richtungslicht.
 *
 * @param shader Der Shader, der die Uniforms setzen soll.
 * @param light Das Richtungslicht, dessen Uniforms gesetzt werden sollen.
 */
static void setDirLightUniforms(Shader *shader, DirLight light) {
    shader_setVec3(shader, "u_dirLight.direction", &light.direction);

    shader_setVec3(shader, "u_dirLight.ambient", &light.ambient);
    shader_setVec3(shader, "u_dirLight.diffuse", &light.diffuse);
    shader_setVec3(shader, "u_dirLight.specular", &light.specular);
}

/**
 * Initialisiert die Transformationsdaten.
 *
 * @param transform Die Transformationsdaten, die initialisiert werden sollen.
 */
static void initTransform(Transform *transform) {
    glm_vec3_copy((vec3){ 1, 1, 1 }, transform->scale);
}

/**
 * Initialisiert die Skybox.
 *
 * @param skybox Die Skybox, die initialisiert werden soll.
 */
static void initSkybox(Skybox *skybox) {
    skybox->skyboxEnabled = true;
}

/**
 * Initialisiert die Tesselation.
 *
 * @param tesselation Die Tesselation, die initialisiert werden soll.
 */
static void initTesselation(Tesselation *tesselation) {
    tesselation->useTessellation = false;
    tesselation->minTessellation = 1;
    tesselation->maxTessellation = 20;
}

/**
 * Initialisiert das Displacement.
 *
 * @param displacement Das Displacement, das initialisiert werden soll.
 */
static void initDisplacement(Displacement *displacement) {
    displacement->useDisplacement = false;
    displacement->displacementFactor = 0.1f;
}

/**
 * Initialisiert die Normal Map.
 *
 * @param normalMap Die Normal Map, die initialisiert werden soll.
 */
static void initNormalMap(NormalMap *normalMap) {
    normalMap->enableNormalMapping = true;
    normalMap->enableTwoChannelNormalMap = true;
}

/**
 * Initialisiert den Nebel.
 *
 * @param fog Der Nebel, der initialisiert werden soll.
 */
static void initFog(Fog *fog) {
    fog->fogEnabled = false;
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, fog->color);
    fog->fogDensity = .01f;
}

/**
 * Initialisiert die Postprocessing-Daten.
 *
 * @param postprocessing Die Postprocessing-Daten, die initialisiert werden sollen.
 */
static void initPostprocessing(Postprocessing *postprocessing) {
    postprocessing->exposure = 1.0f;
    postprocessing->gamma = 2.2f;
    postprocessing->bloom.colorWeight = 1.0f;
    postprocessing->bloom.emissionWeight = 1.0f;
    postprocessing->bloom.threshold = 1.0f;
    postprocessing->bloom.blurIterations = 2;
}

/**
 * Initialisiert die Lichtdaten.
 *
 * @param scene Die Szene, die die Lichtdaten enthält.
 * @param light Die Lichtdaten, die initialisiert werden sollen.
 */
static void initLight(Scene *scene, Light *light, GBuffer* gbuffer, ShadowMap* shadowMap) {
    if (scene != NULL && scene->countDirLights > 0) {
        light->defaultDirLight = scene->dirLights[0];
        light->hasCreatedDefaultDirLights = false;
    } else {
        light->defaultDirLight = light_createDirLight((vec3){0.0f, 5.0f, 5.0f}, (vec3){1.0f, 1.0f, 1.0f});
        light->hasCreatedDefaultDirLights = true;
    }

    if (scene != NULL && scene->countPointLights > 0) {
        light->defaultPointLight = scene->pointLights[0];
        light->hasCreatedDefaultPointLights = false;

        gbuffer_initializePointLightFBOs(gbuffer, scene->countPointLights, POINT_SHADOW_SIZE);
    } else {
        light->defaultPointLight = light_createPointLight((vec3){0.0f, 1.0f, 0.0f}, (vec3){1.0f, 1.0f, 0.0f});
        light->hasCreatedDefaultPointLights = true;

        gbuffer_initializePointLightFBOs(gbuffer, 1, POINT_SHADOW_SIZE);
    }

    shadowMap->needsUpdating = true;

    light->isDirLightActive = true;
    light->isPointLightActive = true;
}

/**
 * Rendert ein Fullscreen-Quad.
 *
 * @param fullscreenQuad Das Fullscreen-Quad, das gerendert werden soll.
 */
static void renderFullscreenQuad(FullscreenQuad fullscreenQuad) {
    glBindVertexArray(fullscreenQuad.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

/**
 * Berechnet den Skalierungsfaktor für das Punktlicht-Volumen.
 *
 * @param light Das Punktlicht, dessen Volumen skaliert werden soll.
 * @return Der Skalierungsfaktor für das Punktlicht-Volumen.
 */
static float calcPointLightVolumeScale(const PointLight light) {
    const float maxIntensity = fmaxf(fmaxf(light.diffuse[0], light.diffuse[1]), light.diffuse[2]);
    return (-light.linear + sqrtf(powf(light.linear, 2.0f) - 4 * light.quadratic * (light.constant - maxIntensity * (256.0f / 5.0f)))) / (2 * light.quadratic);
}

/**
 * Rendert das Lichtvolumen.
 */
static void renderLightVolume(const RenderingData* data, Shader* shader) {
    model_drawModel(data->lightVolume, shader);
}

/**
 * Parst die Farb-Attachments für das Licht.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param shader Der Shader, der die Attachments setzen soll.
 */
static void parseColorAttachmentsForLight(RenderingData *data, Shader *shader) {
    GLuint albedoTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    shader_setInt(shader, "u_albedoSpec", DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC);

    GLuint ambientShiTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI);
    glBindTexture(GL_TEXTURE_2D, ambientShiTex);
    shader_setInt(shader, "u_ambientShi", DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI);

    GLuint positionTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_POSITION);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_POSITION);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    shader_setInt(shader, "u_position", DEFAULT_GBUFFER_COLORATTACH_POSITION);

    GLuint normalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_NORMAL);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_NORMAL);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    shader_setInt(shader, "u_normal", DEFAULT_GBUFFER_COLORATTACH_NORMAL);

    GLuint emissionTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_EMISSION);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_EMISSION);
    glBindTexture(GL_TEXTURE_2D, emissionTex);
    shader_setInt(shader, "u_emission", DEFAULT_GBUFFER_COLORATTACH_EMISSION);
}

/**
 * Parst die Farb-Attachments für den Schwellenwert.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param shader Der Shader, der die Attachments setzen soll.
 */
static void parseColorAttachmentsForThreshold(RenderingData *data, Shader *shader) {
    GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
    glBindTexture(GL_TEXTURE_2D, finalTex);
    shader_setInt(shader, "u_final", DEFAULT_GBUFFER_COLORATTACH_FINAL);

    GLuint emissionTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_EMISSION);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_EMISSION);
    glBindTexture(GL_TEXTURE_2D, emissionTex);
    shader_setInt(shader, "u_emission", DEFAULT_GBUFFER_COLORATTACH_EMISSION);
}

/**
 * Parst die Farb-Attachments für den Blur-Effekt.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param shader Der Shader, der die Attachments setzen soll.
 * @param isHorizontal Gibt an, ob der Blur horizontal ist.
 * @param isEntry Gibt an, ob es der Einstiegspunkt ist.
 */
static void parseColorAttachmentsForBlur(RenderingData *data, Shader *shader, bool isHorizontal, bool isEntry, bool isDepthOfField) {
    if (isEntry) {
        if (isDepthOfField) {
            const GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, finalTex);
        } else {
            const GLuint thresholdTex = gbuffer_getBlurTexture(data->gbuffer, BLUR_GBUFFER_COLORATTACH_BLUR_H);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, thresholdTex);
        }

        shader_setInt(shader, "u_image", 0);
    } else {
        BLUR_GBUFFER_TEXTURE_TYPE textureType = isHorizontal ? BLUR_GBUFFER_COLORATTACH_BLUR_V : BLUR_GBUFFER_COLORATTACH_BLUR_H;
        GLuint blurTex = gbuffer_getBlurTexture(data->gbuffer, textureType);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blurTex);
        shader_setInt(shader, "u_image", 0);
    }
}

/**
 * Parst die Farb-Attachments für den Nebel.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param shader Der Shader, der die Attachments setzen soll.
 */
static void parseColorAttachmentsForFog(RenderingData *data, Shader *shader) {
    GLuint positionTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_POSITION);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_POSITION);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    shader_setInt(shader, "u_position", DEFAULT_GBUFFER_COLORATTACH_POSITION);

    GLuint normalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_NORMAL);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_NORMAL);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    shader_setInt(shader, "u_normal", DEFAULT_GBUFFER_COLORATTACH_NORMAL);

    GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
    glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
    glBindTexture(GL_TEXTURE_2D, finalTex);
    shader_setInt(shader, "u_final", DEFAULT_GBUFFER_COLORATTACH_FINAL);
}

/**
 * @brief Führt den Schatten-Rendering-Pass für eine Punktlichtquelle aus.
 *
 * @param data Zeiger auf die RenderingData-Struktur mit Informationen zur Schattenverarbeitung.
 * @param scene Zeiger auf das Model, das im Schatten-Pass gerendert werden soll.
 * @param modelMatrix Zeiger auf die Modellmatrix zur Transformation des Models.
 * @param pointLightPosition Zeiger auf die Position der Punktlichtquelle.
 * @param index Index der Punktlichtquelle in der Schattenpufferstruktur.
 * @param width Breite des ursprünglichen Viewports zur Wiederherstellung nach dem Rendern.
 * @param height Höhe des ursprünglichen Viewports zur Wiederherstellung nach dem Rendern.
 *
 * Diese Funktion rendert Schatten für eine Punktlichtquelle, indem sie:
 * 1. Die Perspektivprojektion für die Schattenwürfelkarte erstellt.
 * 2. Sechs verschiedene Ansichten generiert, die die sechs Seiten des Würfels abdecken.
 * 3. Die Schattenpuffer für das aktuelle Licht bindet.
 * 4. Shader-Uniforms setzt, darunter Modellmatrix, Schattenmatrizen und Lichtposition.
 * 5. Das Szenenmodell rendert.
 * 6. Den ursprünglichen Framebuffer und Viewport wiederherstellt.
 */
static void performPointLightShadowPass(RenderingData *data, Model* scene, mat4* modelMatrix, vec3 *pointLightPosition, int index, int width, int height)
{
    common_pushRenderScope("Pointlight-Shadow-Pass");
    {
        shader_useShader(data->pointLightShadowShader);
        glEnable(GL_DEPTH_TEST);

        mat4 projection;
        const float znear = .1f, zfar = 200;
        glm_perspective(((90.0f) * (float)M_PI / 180.0f), ((float)POINT_SHADOW_SIZE / (float)POINT_SHADOW_SIZE), znear, zfar, projection);

        mat4 view;
        vec3 position;

        glm_vec3_add(*pointLightPosition, (vec3){ 1, 0, 0 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, -1, 0 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[0]);

        glm_vec3_add(*pointLightPosition, (vec3){ -1, 0, 0 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, -1, 0 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[1]);

        glm_vec3_add(*pointLightPosition, (vec3){ 0, 1, 0 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, 0, 1 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[2]);

        glm_vec3_add(*pointLightPosition, (vec3){ 0, -1, 0 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, 0, -1 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[3]);

        glm_vec3_add(*pointLightPosition, (vec3){ 0, 0, 1 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, -1, 0 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[4]);

        glm_vec3_add(*pointLightPosition, (vec3){ 0, 0, -1 }, position);
        glm_lookat(*pointLightPosition, position, (vec3){ 0, -1, 0 }, view);
        glm_mat4_mul(projection, view, data->shadowMap.cubemapMatrices[5]);

        glViewport(0, 0, POINT_SHADOW_SIZE, POINT_SHADOW_SIZE);
        gbuffer_bindGBufferForPointLightShadow(data->gbuffer, index);

        shader_setMat4(data->pointLightShadowShader, "u_model", modelMatrix);
        shader_setMat4Array(data->pointLightShadowShader, "u_shadowMatrices", data->shadowMap.cubemapMatrices, 6);
        shader_setVec3(data->pointLightShadowShader, "u_position", pointLightPosition);
        shader_setFloat(data->pointLightShadowShader, "u_zFar", zfar);

        model_drawModel(scene, data->pointLightShadowShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    common_popRenderScope();
}

/**
 * Führt den Punktlicht-Pass durch.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param projectionMatrix Die Projektionsmatrix der Szene.
 * @param viewMatrix Die View-Matrix der Szene.
 * @param modelMatrix Die Model-Matrix der Szene
 * @param cameraPosition Die Position der Kamera.
 * @param pointLight Das Punktlicht, das gerendert werden soll.
 */
static void performPointLightPass(RenderingData *data, mat4 *projectionMatrix, mat4 *viewMatrix, mat4 modelMatrix, vec3 *cameraPosition, PointLight *pointLight, int index) {
    mat4 model;
    glm_mat4_copy(modelMatrix, model);

    glm_translate(model, pointLight->position);

    const float radius = calcPointLightVolumeScale(*pointLight);
    vec3 scale = { radius, radius, radius };
    glm_scale(model, scale);

    // common_pushRenderScope("Stencil-Pass");
    // {
    //     gbuffer_bindGBufferForStencilPass(data->gbuffer);
    //     shader_useShader(data->nullShader);
    //
    //     shader_setMat4(data->nullShader, "u_projection", projectionMatrix);
    //     shader_setMat4(data->nullShader, "u_view", viewMatrix);
    //     shader_setMat4(data->nullShader, "u_model", &model);
    //
    //     glEnable(GL_DEPTH_TEST);
    //     glClear(GL_STENCIL_BUFFER_BIT);
    //     glEnable(GL_STENCIL_TEST);
    //     glStencilFunc(GL_ALWAYS, 0, 0);
    //     glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    //     glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    //     glDisable(GL_CULL_FACE);
    //
    //     if (data->light.isPointLightActive) {
    //         renderLightVolume(data, data->nullShader);
    //     }
    // }
    // common_popRenderScope();

    common_pushRenderScope("Pointlight-Pass");
    {
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass only where stencil != 0
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE); // Additive blending

        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);

        gbuffer_bindGBufferForLightPass(data->gbuffer);
        shader_useShader(data->light.pointlightShader);

        shader_setMat4(data->light.pointlightShader, "u_projection", projectionMatrix);
        shader_setMat4(data->light.pointlightShader, "u_view", viewMatrix);
        shader_setMat4(data->light.pointlightShader, "u_model", &model);

        parseColorAttachmentsForLight(data, data->light.pointlightShader);

        setPointLightUniforms(data->light.pointlightShader, *pointLight);
        shader_setBool(data->light.pointlightShader, "u_isActive", data->light.isPointLightActive);
        shader_setVec3(data->light.pointlightShader, "u_cameraPos", cameraPosition);
        shader_setBool(data->light.pointlightShader, "u_showShadows", data->shadowMap.showShadows);
        shader_setBool(data->light.pointlightShader, "u_usePCF", data->shadowMap.usePCF);

        const float zfar = 200;
        shader_setFloat(data->light.pointlightShader, "u_zFar", zfar);

        GLuint shadowMap = gbuffer_getPointLightShadowMap(data->gbuffer, index);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_NUM_COLORATTACH);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
        shader_setInt(data->light.pointlightShader, "u_shadowMap", DEFAULT_GBUFFER_NUM_COLORATTACH);

        if (data->light.isPointLightActive) {
            renderFullscreenQuad(data->fullscreenQuad);
        }

        glCullFace(GL_BACK);
        glDisable(GL_BLEND);
    }
    common_popRenderScope();
}

/**
 * @brief Berechnet die Licht-Raum-Matrix für die gerichtete Beleuchtung.
 *
 * @param data Zeiger auf die RenderingData-Struktur, die Licht- und Schatteninformationen enthält.
 * @param lightSpace Zeiger auf die Matrix, in die die berechnete Licht-Raum-Transformation gespeichert wird.
 *
 * Diese Funktion berechnet die Licht-Raum-Matrix für die gerichtete Beleuchtung, indem sie:
 * 1. Die Blickrichtung des Lichts normalisiert und skaliert.
 * 2. Eine orthografische Projektionsmatrix basierend auf den Schattenkarteneinstellungen erstellt.
 * 3. Eine View-Matrix erstellt, die das Licht auf den Ursprung ausrichtet.
 * 4. Die Projektions- und View-Matrix multipliziert, um die endgültige Licht-Raum-Matrix zu erzeugen.
 */
static void CalcDirLightSpace(RenderingData* data, mat4* lightSpace)
{
    vec3 eye;
    glm_vec3_copy(data->light.defaultDirLight->direction, eye);
    glm_normalize(eye);
    glm_vec3_scale(eye, data->light.dirLightDistanceMult, eye);

    mat4 projection, view;
    glm_ortho(-data->shadowMap.quadSize, data->shadowMap.quadSize, -data->shadowMap.quadSize, data->shadowMap.quadSize, data->shadowMap.zNear, data->shadowMap.zFar, projection);
    glm_lookat(eye, GLM_VEC3_ZERO, GLM_YUP, view);
    glm_mat4_mul(projection, view, *lightSpace);
}

/**
 * @brief Performs the directional light shadow pass for the scene.
 *
 * @param data Pointer to the RenderingData structure containing rendering-related information.
 * @param cameraPosition The position of the camera (currently unused).
 * @param modelMatrix Pointer to the model matrix used for rendering the scene.
 * @param lightSpace Pointer to the light space transformation matrix.
 * @param scene Pointer to the Model structure representing the scene to be rendered.
 * @param width The width of the viewport for restoring after rendering.
 * @param height The height of the viewport for restoring after rendering.
 *
 * This function renders the scene to the directional light shadow map by:
 * 1. Setting up the shader and enabling depth testing.
 * 2. Binding the shadow map framebuffer.
 * 3. Configuring viewport for shadow map resolution.
 * 4. Rendering the scene with front-face culling to prevent shadow artifacts.
 * 5. Restoring the original framebuffer and viewport settings.
 */
static void PerformDirLightShadowPass(RenderingData* data, vec3 cameraPosition, mat4* modelMatrix, mat4* lightSpace, Model* scene, int width, int height)
{
    (void)cameraPosition;

    common_pushRenderScope("DirLight-Shadow-Pass");
    {
        shader_useShader(data->dirLightShadowShader);
        glEnable(GL_DEPTH_TEST);

        shader_setMat4(data->dirLightShadowShader, "u_model", modelMatrix);
        shader_setMat4(data->dirLightShadowShader, "u_lightSpace", lightSpace);

        glViewport(0, 0, DIR_SHADOW_SIZE, DIR_SHADOW_SIZE);
        gbuffer_bindGBufferForDirLightShadows(data->gbuffer);

        glCullFace(GL_FRONT);
        model_drawModel(scene, data->dirLightShadowShader);
        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    common_popRenderScope();
}

/**
 * Führt den Richtungslicht-Pass durch.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 * @param cameraPosition Die Position der Kamera.
 * @param dirLight Das Richtungslicht, das gerendert werden soll.
 */
static void performDirLightPass(RenderingData *data, vec3 *cameraPosition, DirLight *dirLight, mat4 *lightSpace) {
    common_pushRenderScope("DirLight-Pass");
    {
        shader_useShader(data->light.dirlightShader);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE); // Additive blending

        gbuffer_bindGBufferForLightPass(data->gbuffer);

        parseColorAttachmentsForLight(data, data->light.dirlightShader);

        setDirLightUniforms(data->light.dirlightShader, *dirLight);
        shader_setBool(data->light.dirlightShader, "u_isActive", data->light.isDirLightActive);
        shader_setVec3(data->light.dirlightShader, "u_cameraPos", cameraPosition);
        shader_setMat4(data->light.dirlightShader, "u_lightSpace", lightSpace);
        shader_setBool(data->light.dirlightShader, "u_showShadows", data->shadowMap.showShadows);
        shader_setBool(data->light.dirlightShader, "u_usePCF", data->shadowMap.usePCF);

        GLuint shadowMap = gbuffer_getDirLightShadowMap(data->gbuffer);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_NUM_COLORATTACH);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        shader_setInt(data->light.dirlightShader, "u_shadowMap", DEFAULT_GBUFFER_NUM_COLORATTACH);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);

        if (data->light.isDirLightActive) {
            renderFullscreenQuad(data->fullscreenQuad);
        }

        glDisable(GL_BLEND);
    }
    common_popRenderScope();
}

/**
 * Führt den Threshold-Pass durch.
 *
 * Dieser Pass extrahiert helle Bildanteile (Lichter, Glühen) aus der Final-Buffer-Phase,
 * um sie für nachfolgende Bloom-Effekte zu nutzen. Er wird üblicherweise zwischen
 * der Beleuchtungs- und der Bloom-Blur-Phase ausgeführt.
 *
 * @param data Zugriff auf das Rendering-Datenobjekt.
 */
static void performThresholdPass(RenderingData *data) {
    common_pushRenderScope("Threshold-Pass");
    {
        shader_useShader(data->thresholdShader);

        gbuffer_bindGBufferForThreshold(data->gbuffer);
        parseColorAttachmentsForThreshold(data, data->thresholdShader);

        shader_setFloat(data->thresholdShader, "u_colorWeight", data->postprocessing.bloom.colorWeight);
        shader_setFloat(data->thresholdShader, "u_emissionWeight", data->postprocessing.bloom.emissionWeight);
        shader_setFloat(data->thresholdShader, "u_threshold", data->postprocessing.bloom.threshold);

        renderFullscreenQuad(data->fullscreenQuad);
    }
    common_popRenderScope();
}

/**
 * Führt den Blur-Pass für den Bloom-Effekt durch.
 *
 * Dieser Pass führt mehrfache horizontale und vertikale Weichzeichnungsdurchläufe (Blur-Iterationen) durch,
 * um aus den zuvor extrahierten hellen Bildanteilen einen weichen Bloom-Effekt zu erzeugen.
 *
 * @param data Zugriff auf das Rendering-Datenobjekt.
 * @param isDepthOfField
 */
static void performBlurPass(RenderingData *data, bool isDepthOfField) {
    common_pushRenderScope("Blur-Pass");
    {
        bool isHorizontal = true;
        bool isEntry = true;
        for (int i = 0; i < data->postprocessing.bloom.blurIterations * 2; ++i) {
            shader_useShader(data->blurShader);

            gbuffer_bindGBufferForBlur(data->gbuffer, isHorizontal);

            parseColorAttachmentsForBlur(data, data->blurShader, isHorizontal, isEntry, isDepthOfField);

            shader_setInt(data->blurShader, "u_horizontal", isHorizontal);

            renderFullscreenQuad(data->fullscreenQuad);

            isHorizontal = !isHorizontal;
            if (isEntry) {
                isEntry = false;
            }
        }
    }
    common_popRenderScope();
}

/**
 * Führt den Nebel-Pass durch.
 *
 * Dieser Pass mischt auf Basis der Kameraposition und zuvor gerenderter Tiefeninformationen
 * Nebel über das aktuelle Bild. Dabei wird die in den Shader-Uniforms
 * gesetzte Nebeldichte und -farbe genutzt, um Objekte in der Ferne in Nebel eintauchen zu lassen.
 *
 * @param data Zugriff auf das Rendering-Datenobjekt.
 * @param cameraPosition Die Position der Kamera in Weltkoordinaten.
 */
static void performFogPass(RenderingData *data, vec3 *cameraPosition) {
    common_pushRenderScope("Fog-Pass");
    {
        shader_useShader(data->fog.shader);
        gbuffer_bindGBufferForFog(data->gbuffer);
        parseColorAttachmentsForFog(data, data->fog.shader);

        shader_setVec3(data->fog.shader, "u_cameraPos", cameraPosition);
        shader_setVec3(data->fog.shader, "u_fogColor", &data->fog.color);
        shader_setFloat(data->fog.shader, "u_fogDensity", data->fog.fogDensity);

        renderFullscreenQuad(data->fullscreenQuad);
    }
    common_popRenderScope();
}

/**
 * Führt den Skybox-Pass durch.
 *
 * Dieser Pass rendert eine Skybox um die Szene herum. Die Skybox wird
 * anhand der Normal- und Final-Buffers im Hintergrund eingeblendet.
 *
 * @param data Zugriff auf das Rendering-Datenobjekt.
 * @param viewMatrix Die aktuelle View-Matrix der Kamera.
 * @param projectionMatrix Die aktuelle Projektions-Matrix.
 * @param width Breite des aktuellen Framebuffers / Fensters.
 * @param height Höhe des aktuellen Framebuffers / Fensters.
 */
static void performSkyboxPass(RenderingData *data, mat4 viewMatrix, mat4 projectionMatrix, int width, int height) {
    common_pushRenderScope("Skybox-Pass");
    {
        shader_useShader(data->skybox.shader);

        gbuffer_bindGBufferForPostprocess(data->gbuffer);

        GLuint normalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_NORMAL);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_NORMAL);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        shader_setInt(data->skybox.shader, "u_normal", DEFAULT_GBUFFER_COLORATTACH_NORMAL);

        GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glBindTexture(GL_TEXTURE_2D, finalTex);
        shader_setInt(data->skybox.shader, "u_final", DEFAULT_GBUFFER_COLORATTACH_FINAL);

        vec2 screenSize = {(float) width, (float) height};
        shader_setVec2(data->skybox.shader, "u_screenSize", &screenSize);

        rendering_drawSkybox(data, viewMatrix, projectionMatrix);
    }
    common_popRenderScope();
}


/**
 * @brief Performs the depth of field post-processing pass.
 *
 * @param data Pointer to the RenderingData structure containing rendering-related information.
 * @param cameraPos Pointer to the vec3 structure representing the camera position.
 *
 * This function applies depth of field effects by performing a blur pass, binding necessary textures,
 * and rendering the final processed image using the depth of field shader.
 */
static void performDepthOfFieldPass(RenderingData *data, vec3 *cameraPos)
{
    common_pushRenderScope("DepthOfField-Pass");
    {
        performBlurPass(data, true);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        shader_useShader(data->depthOfFieldShader);

        GLuint positionTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_POSITION);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_POSITION);
        glBindTexture(GL_TEXTURE_2D, positionTex);
        shader_setInt(data->depthOfFieldShader, "u_position", DEFAULT_GBUFFER_COLORATTACH_POSITION);

        const GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glBindTexture(GL_TEXTURE_2D, finalTex);
        shader_setInt(data->depthOfFieldShader, "u_final", DEFAULT_GBUFFER_COLORATTACH_FINAL);

        const GLuint blurTex = gbuffer_getBlurTexture(data->gbuffer, BLUR_GBUFFER_COLORATTACH_BLUR_V);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_NUM_COLORATTACH + BLUR_GBUFFER_COLORATTACH_BLUR_V);
        glBindTexture(GL_TEXTURE_2D, blurTex);
        shader_setInt(data->depthOfFieldShader, "u_finalBlur", DEFAULT_GBUFFER_NUM_COLORATTACH + BLUR_GBUFFER_COLORATTACH_BLUR_V);

        shader_setVec3(data->depthOfFieldShader, "u_cameraPos", cameraPos);
        shader_setFloat(data->depthOfFieldShader, "u_focusDistance", data->postprocessing.focusDistance);
        shader_setFloat(data->depthOfFieldShader, "u_depthOfField", data->postprocessing.depthOfField);
        shader_setBool(data->depthOfFieldShader, "u_useDoF", data->postprocessing.useDoF);

        renderFullscreenQuad(data->fullscreenQuad);
    }
    common_popRenderScope();
}

/**
 * Handhabt den RENDER_MODE_PHONG.
 *
 * Hierbei wird der aktuell gerenderte Framebuffer einfach auf den Hauptframebuffer geblittet,
 * um einen Phong-shading-basierten Render-Output anzuzeigen.
 *
 * @param width Breite des Framebuffers.
 * @param height Höhe des Framebuffers.
 */
static void handleRenderModePhong(int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

/**
 * Handhabt den RENDER_MODE_DEBUG.
 *
 * Hierbei wird das G-Buffer-Inhaltsdebugging ausgeführt, indem verschiedene G-Buffer-Texturen
 * (Albedo/Spec, Normal, Position, Emission) in ein 2x2-Raster auf den Hauptframebuffer
 * geschrieben werden. Dies hilft beim Debuggen und Verständnis der gerenderten Szene.
 *
 * @param data Zugriff auf das Rendering-Datenobjekt, insbesondere den G-Buffer.
 * @param width Breite des Fensters.
 * @param height Höhe des Fensters.
 */
static void handleRenderModeDebug(RenderingData *data, int width, int height) {
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    gbuffer_bindForRead(data->gbuffer);

    gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC);
    glBlitFramebuffer(0, 0, width, height, 0, halfHeight, halfWidth, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_COLORATTACH_NORMAL);
    glBlitFramebuffer(0, 0, width, height, halfWidth, halfHeight, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_COLORATTACH_POSITION);
    glBlitFramebuffer(0, 0, width, height, 0, 0, halfWidth, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_COLORATTACH_EMISSION);
    glBlitFramebuffer(0, 0, width, height, halfWidth, 0, width, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

void rendering_init(ProgContext *ctx) {
    ctx->rendering = malloc(sizeof(RenderingData));
    RenderingData *data = ctx->rendering;

    memset(data, 0, sizeof(RenderingData));

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    rendering_loadShaders(data);

    // Standardwerte setzen
    {
        initTransform(&data->transform);
        initSkybox(&data->skybox);
        initTesselation(&data->tesselation);
        initDisplacement(&data->displacement);
        initNormalMap(&data->normalMap);
        initFog(&data->fog);
        initPostprocessing(&data->postprocessing);

        data->renderMode = RENDER_MODE_PHONG;
        data->clipping = .1f;
    }

    data->shadowMap.needsUpdating = true;
    data->shadowMap.quadSize = 10.0f;
    data->shadowMap.zNear = 0.1f;
    data->shadowMap.zFar = 150.f;
    data->shadowMap.cubemapMatrices = malloc(sizeof(mat4) * 6);
    data->shadowMap.showShadows = true;
    data->shadowMap.usePCF = true;

    data->light.dirLightDistanceMult = 20;

    data->postprocessing.useDoF = true;
    data->postprocessing.focusDistance = 10;
    data->postprocessing.depthOfField = 8;

    data->lightVolume = model_loadModel(RESOURCE_PATH "/model/sphere.fbx");

    data->gbuffer = gbuffer_createGBuffer(ctx->winData->width, ctx->winData->height, DIR_SHADOW_SIZE);

    utils_createQuad(&data->fullscreenQuad.quadVAO, &data->fullscreenQuad.quadVBO);

    // Skybox-Textur laden
    {
        const char *cubemapFaces[] = {
            RESOURCE_PATH "textures/skybox/interstellar_ft.tga", RESOURCE_PATH "textures/skybox/interstellar_bk.tga",
            RESOURCE_PATH "textures/skybox/interstellar_up.tga", RESOURCE_PATH "textures/skybox/interstellar_dn.tga",
            RESOURCE_PATH "textures/skybox/interstellar_rt.tga", RESOURCE_PATH "textures/skybox/interstellar_lf.tga"
        };
        data->skybox.cubemapTexture = texture_loadCubemap(cubemapFaces);
        if (data->skybox.cubemapTexture == 0) {
            fprintf(stderr, "Error: Cubemap textures could not be loaded.\n");
        }

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_CUBEMAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, data->skybox.cubemapTexture);
    }

    // Erstelle die Geometrie für die Skybox
    utils_createCube(&data->skybox.skyboxVAO, &data->skybox.skyboxVBO, &data->skybox.skyboxVertexCount);

    rendering_updateUniforms(data);
}

void rendering_draw(const ProgContext *ctx) {
    RenderingData *data = ctx->rendering;
    InputData *input = ctx->input;

    if (input->rendering.hasUpdatedScene) {
        rendering_updateSceneData(ctx);
        input->rendering.hasUpdatedScene = false;
    }

    // Bildschirm leeren (jetzt wird das GBuffer gereinigt)
    glClearColor(
        input->rendering.clearColor[0],
        input->rendering.clearColor[1],
        input->rendering.clearColor[2],
        input->rendering.clearColor[3]
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    mat4 projectionMatrix;
    mat4 viewMatrix;

    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);
    glm_translate(modelMatrix, data->transform.translation);
    glm_rotate_x(modelMatrix, glm_rad(data->transform.rotation[0]), modelMatrix);
    glm_rotate_y(modelMatrix, glm_rad(data->transform.rotation[1]), modelMatrix);
    glm_rotate_z(modelMatrix, glm_rad(data->transform.rotation[2]), modelMatrix);
    glm_scale(modelMatrix, data->transform.scale);

    mat4 dirlightSpace;
    CalcDirLightSpace(data, &dirlightSpace);

    // Zuerst die Projection Matrix aufsetzen.
    const float aspect = (float) ctx->winData->width / (float) ctx->winData->height;
    const float zoom = camera_getZoom(input->mainCamera);
    glm_perspective(glm_rad(zoom), aspect, 0.1f, 200.0f, projectionMatrix);

    // Dann die View-Matrix bestimmen.
    camera_getViewMatrix(input->mainCamera, viewMatrix);

    vec3 cameraPosition;
    camera_getPosition(input->mainCamera, cameraPosition);

    common_pushRenderScope("Geometry-Pass");
    {
        gbuffer_bindGBufferForGeomPass(data->gbuffer);
        shader_useShader(data->modelShader);

        glEnable(GL_DEPTH_TEST);

        // Überprüfen, ob der Wireframe-Modus verwendet werden soll.
        if (input->showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE); // Deaktivierung des Face Cullings
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE); // Aktivierung des Face Cullings
        }

        shader_setVec3(data->modelShader, "u_cameraPos", &cameraPosition);
        shader_setMat4(data->modelShader, "u_projection", &projectionMatrix);
        shader_setMat4(data->modelShader, "u_view", &viewMatrix);

        if (input->rendering.userScene) {
            model_drawModel(input->rendering.userScene->model, data->modelShader);

            if (data->shadowMap.needsUpdating || data->shadowMap.dirLightShadowsShouldUpdate || data->shadowMap.dirLightShadowsAlwaysUpdate) {
                PerformDirLightShadowPass(data, cameraPosition, &modelMatrix, &dirlightSpace, input->rendering.userScene->model, ctx->winData->width, ctx->winData->height);
                data->shadowMap.dirLightShadowsShouldUpdate = false;
            }

            if (data->shadowMap.needsUpdating || data->shadowMap.pointLightShadowsShouldUpdate) {

                for (int i = 0; i < input->rendering.userScene->countPointLights; ++i) {
                    vec3 pointLightPosition;
                    glm_vec3_copy(input->rendering.userScene->pointLights[i]->position, pointLightPosition);

                    performPointLightShadowPass(data, input->rendering.userScene->model, &modelMatrix, &pointLightPosition, i, ctx->winData->width, ctx->winData->height);
                }

                data->shadowMap.pointLightShadowsShouldUpdate = false;
            }

            data->shadowMap.needsUpdating = false;
        }
    }
    common_popRenderScope();

    common_pushRenderScope("Light-Pass");
    {
        gbuffer_bindGBufferForLightPass(data->gbuffer);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);

        if (input->rendering.userScene) {
             for (int i = 0; i < input->rendering.userScene->countPointLights; ++i) {
                 PointLight *pointLight = input->rendering.userScene->pointLights[i];
                 performPointLightPass(data, &projectionMatrix, &viewMatrix, modelMatrix, &cameraPosition, pointLight, i);
             }

             if (input->rendering.userScene->countPointLights <= 0) {
                 performPointLightPass(data, &projectionMatrix, &viewMatrix, modelMatrix, &cameraPosition, data->light.defaultPointLight, 0);
             }

            glClear(GL_STENCIL_BUFFER_BIT);
            glDisable(GL_STENCIL_TEST);

            for (int i = 0; i < input->rendering.userScene->countDirLights; ++i) {
                DirLight *dirLight = input->rendering.userScene->dirLights[i];
                performDirLightPass(data, &cameraPosition, dirLight, &dirlightSpace);
            }

            if (input->rendering.userScene->countDirLights <= 0) {
                performDirLightPass(data, &cameraPosition, data->light.defaultDirLight, &dirlightSpace);
            }
        }
    }
    common_popRenderScope();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    performThresholdPass(data);
    performBlurPass(data, false);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (data->fog.fogEnabled) {
        performFogPass(data, &cameraPosition);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    common_pushRenderScope("PostProcess-Pass");
    {
        shader_useShader(data->postprocessShader);

        if (data->skybox.skyboxEnabled) {
            gbuffer_bindGBufferForPostprocess(data->gbuffer);
        }

        const GLuint finalTex = gbuffer_getDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
        glBindTexture(GL_TEXTURE_2D, finalTex);
        shader_setInt(data->postprocessShader, "u_final", DEFAULT_GBUFFER_COLORATTACH_FINAL);

        const GLuint bloomTex = gbuffer_getBlurTexture(data->gbuffer, BLUR_GBUFFER_COLORATTACH_BLUR_V);
        glActiveTexture(GL_TEXTURE0 + DEFAULT_GBUFFER_NUM_COLORATTACH + BLUR_GBUFFER_COLORATTACH_BLUR_V);
        glBindTexture(GL_TEXTURE_2D, bloomTex);
        shader_setInt(data->postprocessShader, "u_bloom", DEFAULT_GBUFFER_NUM_COLORATTACH + BLUR_GBUFFER_COLORATTACH_BLUR_V);

        shader_setFloat(data->postprocessShader, "u_exposure", data->postprocessing.exposure);
        shader_setFloat(data->postprocessShader, "u_gamma", data->postprocessing.gamma);

        renderFullscreenQuad(data->fullscreenQuad);
    }
    common_popRenderScope();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    if (data->skybox.skyboxEnabled) {
        performSkyboxPass(data, viewMatrix, projectionMatrix, ctx->winData->width, ctx->winData->height);
    }

    performDepthOfFieldPass(data, &cameraPosition);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    int width = ctx->winData->width;
    int height = ctx->winData->height;
    if (data->renderMode == RENDER_MODE_PHONG) {
        handleRenderModePhong(width, height);
    } else if (data->renderMode == RENDER_MODE_DEBUG) {
        handleRenderModeDebug(data, width, height);
    }

    gbuffer_clearBlurTexture(data->gbuffer, BLUR_GBUFFER_COLORATTACH_BLUR_H);
    gbuffer_clearBlurTexture(data->gbuffer, BLUR_GBUFFER_COLORATTACH_BLUR_V);
    gbuffer_clearDefaultTexture(data->gbuffer, DEFAULT_GBUFFER_COLORATTACH_FINAL);

    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void rendering_cleanup(const ProgContext *ctx) {
    const RenderingData *data = ctx->rendering;

    shader_deleteShader(data->modelShader);
    shader_deleteShader(data->skybox.shader);
    shader_deleteShader(data->fog.shader);
    shader_deleteShader(data->light.dirlightShader);
    shader_deleteShader(data->light.pointlightShader);
    shader_deleteShader(data->postprocessShader);
    shader_deleteShader(data->nullShader);
    shader_deleteShader(data->blurShader);
    shader_deleteShader(data->thresholdShader);
    shader_deleteShader(data->dirLightShadowShader);
    shader_deleteShader(data->pointLightShadowShader);
    shader_deleteShader(data->depthOfFieldShader);

    if (data->shadowMap.cubemapMatrices != NULL) { free(data->shadowMap.cubemapMatrices); }
    if (data->lightVolume != NULL) { model_deleteModel(data->lightVolume); }

    if (data->skybox.skyboxVAO != 0) { glDeleteVertexArrays(1, &data->skybox.skyboxVAO); }
    if (data->skybox.skyboxVBO != 0) { glDeleteBuffers(1, &data->skybox.skyboxVBO); }
    if (data->skybox.cubemapTexture != 0) { glDeleteTextures(1, &data->skybox.cubemapTexture); }

    if (data->gbuffer != NULL) { gbuffer_deleteGBuffer(data->gbuffer); }

    free(ctx->rendering);
}

bool rendering_recompileShader(const ProgContext *ctx) {
    bool shaderOK = shader_recompileShader(&ctx->rendering->modelShader)
            && shader_recompileShader(&ctx->rendering->skybox.shader)
            && shader_recompileShader(&ctx->rendering->fog.shader)
            && shader_recompileShader(&ctx->rendering->light.dirlightShader)
            && shader_recompileShader(&ctx->rendering->light.pointlightShader)
            && shader_recompileShader(&ctx->rendering->postprocessShader)
            && shader_recompileShader(&ctx->rendering->nullShader)
            && shader_recompileShader(&ctx->rendering->blurShader)
            && shader_recompileShader(&ctx->rendering->thresholdShader)
            && shader_recompileShader(&ctx->rendering->dirLightShadowShader)
            && shader_recompileShader(&ctx->rendering->pointLightShadowShader)
            && shader_recompileShader(&ctx->rendering->dirLightShadowShader)
            && shader_recompileShader(&ctx->rendering->depthOfFieldShader);

    rendering_updateUniforms(ctx->rendering);
    return shaderOK;
}

void rendering_updateSceneData(const ProgContext *ctx) {
    Light *light = &ctx->rendering->light;

    if (ctx->rendering->light.defaultPointLight != NULL && light->hasCreatedDefaultPointLights) {
        light_deletePointLight(ctx->rendering->light.defaultPointLight);
    }

    if (ctx->rendering->light.defaultDirLight != NULL && light->hasCreatedDefaultDirLights) {
        light_deleteDirLight(ctx->rendering->light.defaultDirLight);
    }

    initLight(ctx->input->rendering.userScene, light, ctx->rendering->gbuffer, &ctx->rendering->shadowMap);
}

void rendering_updateFramebuffer(const ProgContext *ctx) {
    gbuffer_deleteGBuffer(ctx->rendering->gbuffer);
    ctx->rendering->gbuffer = gbuffer_createGBuffer(ctx->winData->width, ctx->winData->height, DIR_SHADOW_SIZE);
    rendering_updateSceneData(ctx);
}

//////////////////////////// GETTER UND SETTER //////////////////////////////

RenderMode rendering_getSelectedRenderMode(const ProgContext *ctx) {
    return ctx->rendering->renderMode;
}

void rendering_selectRenderMode(const ProgContext *ctx, const RenderMode mode) {
    if (ctx->rendering->renderMode == mode) { return; }

    ctx->rendering->renderMode = mode;
    rendering_updateUniforms(ctx->rendering);
}

bool rendering_getSkyboxEnabled(const ProgContext *ctx) {
    return ctx->rendering->skybox.skyboxEnabled;
}

void rendering_enableSkybox(const ProgContext *ctx) {
    ctx->rendering->skybox.skyboxEnabled = true;
}

void rendering_disableSkybox(const ProgContext *ctx) {
    ctx->rendering->skybox.skyboxEnabled = false;
}

bool rendering_getNormalMappingEnabled(const ProgContext *ctx) {
    return ctx->rendering->normalMap.enableNormalMapping;
}

void rendering_enableNormalMapping(const ProgContext *ctx) {
    if (ctx->rendering->normalMap.enableNormalMapping) { return; }

    ctx->rendering->normalMap.enableNormalMapping = true;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_disableNormalMapping(const ProgContext *ctx) {
    if (!ctx->rendering->normalMap.enableNormalMapping) { return; }

    ctx->rendering->normalMap.enableNormalMapping = false;
    rendering_updateUniforms(ctx->rendering);
}

bool rendering_getTwoChannelNormalMapEnabled(const ProgContext *ctx) {
    return ctx->rendering->normalMap.enableTwoChannelNormalMap;
}

void rendering_enableTwoChannelNormalMap(const ProgContext *ctx) {
    if (ctx->rendering->normalMap.enableTwoChannelNormalMap) { return; }

    ctx->rendering->normalMap.enableTwoChannelNormalMap = true;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_disableTwoChannelNormalMap(const ProgContext *ctx) {
    if (!ctx->rendering->normalMap.enableTwoChannelNormalMap) { return; }

    ctx->rendering->normalMap.enableTwoChannelNormalMap = false;
    rendering_updateUniforms(ctx->rendering);
}

bool rendering_getFogEnabled(const ProgContext *ctx) {
    return ctx->rendering->fog.fogEnabled;
}

void rendering_enableFog(const ProgContext *ctx) {
    if (ctx->rendering->fog.fogEnabled) { return; }

    ctx->rendering->fog.fogEnabled = true;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_disableFog(const ProgContext *ctx) {
    if (!ctx->rendering->fog.fogEnabled) { return; }

    ctx->rendering->fog.fogEnabled = false;
    rendering_updateUniforms(ctx->rendering);
}

float rendering_getFogDensity(const ProgContext *ctx) {
    return ctx->rendering->fog.fogDensity;
}

void rendering_setFogDensity(const ProgContext *ctx, const float density) {
    if (ctx->rendering->fog.fogDensity == density) { return; }

    ctx->rendering->fog.fogDensity = density;
    rendering_updateUniforms(ctx->rendering);
}

float rendering_getAlphaClipping(const ProgContext *ctx) {
    return ctx->rendering->clipping;
}

void rendering_setAlphaClipping(const ProgContext *ctx, const float clipping) {
    if (ctx->rendering->clipping == clipping) { return; }

    ctx->rendering->clipping = clipping;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_getTranslation(const ProgContext *ctx, vec3 outTranslation) {
    glm_vec3_copy(ctx->rendering->transform.translation, outTranslation);
}

void rendering_getRotation(const ProgContext *ctx, vec3 outRotation) {
    glm_vec3_copy(ctx->rendering->transform.rotation, outRotation);
}

void rendering_getScale(const ProgContext *ctx, vec3 outScale) {
    glm_vec3_copy(ctx->rendering->transform.scale, outScale);
}

bool rendering_getTesselationEnabled(const ProgContext *ctx) {
    return ctx->rendering->tesselation.useTessellation;
}

bool rendering_getDisplacementEnabled(const ProgContext *ctx) {
    return ctx->rendering->displacement.useDisplacement;
}

int rendering_getTesselationMin(const ProgContext *ctx) {
    return ctx->rendering->tesselation.minTessellation;
}

int rendering_getTesselationMax(const ProgContext *ctx) {
    return ctx->rendering->tesselation.maxTessellation;
}

void rendering_setTranslation(const ProgContext *ctx, vec3 translation) {
    if (glm_vec3_eqv_eps(ctx->rendering->transform.translation, translation)) { return; }

    glm_vec3_copy(translation, ctx->rendering->transform.translation);
    rendering_updateUniforms(ctx->rendering);
}

void rendering_setRotation(const ProgContext *ctx, vec3 rotation) {
    if (glm_vec3_eqv_eps(ctx->rendering->transform.rotation, rotation)) { return; }

    glm_vec3_copy(rotation, ctx->rendering->transform.rotation);
    rendering_updateUniforms(ctx->rendering);
}

void rendering_setScale(const ProgContext *ctx, vec3 scale) {
    if (glm_vec3_eqv_eps(ctx->rendering->transform.scale, scale)) { return; }

    glm_vec3_copy(scale, ctx->rendering->transform.scale);
    rendering_updateUniforms(ctx->rendering);
}

void rendering_enableTesselation(const ProgContext *ctx) {
    if (ctx->rendering->tesselation.useTessellation) { return; }

    ctx->rendering->tesselation.useTessellation = true;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_disableTesselation(const ProgContext *ctx) {
    if (!ctx->rendering->tesselation.useTessellation) { return; }

    ctx->rendering->tesselation.useTessellation = false;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_setTesselationMin(const ProgContext *ctx, int min) {
    if (ctx->rendering->tesselation.minTessellation == min) { return; }

    ctx->rendering->tesselation.minTessellation = min;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_setTesselationMax(const ProgContext *ctx, int max) {
    if (ctx->rendering->tesselation.maxTessellation == max) { return; }

    ctx->rendering->tesselation.maxTessellation = max;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_enableDisplacement(const ProgContext *ctx) {
    if (ctx->rendering->displacement.useDisplacement) { return; }

    ctx->rendering->displacement.useDisplacement = true;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_disableDisplacement(const ProgContext *ctx) {
    if (!ctx->rendering->displacement.useDisplacement) { return; }

    ctx->rendering->displacement.useDisplacement = false;
    rendering_updateUniforms(ctx->rendering);
}

void rendering_setDisplacementFactor(const ProgContext *ctx, float factor) {
    if (ctx->rendering->displacement.displacementFactor == factor) { return; }

    ctx->rendering->displacement.displacementFactor = factor;
    rendering_updateUniforms(ctx->rendering);
}

float rendering_getDisplacementFactor(const ProgContext *ctx) {
    return ctx->rendering->displacement.displacementFactor;
}

float rendering_getGammaExposure(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.exposure;
}

float rendering_getGamma(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.gamma;
}

void rendering_setGammaExposure(const ProgContext *ctx, float exposure) {
    ctx->rendering->postprocessing.exposure = exposure;
}

void rendering_setGamma(const ProgContext *ctx, float gamma) {
    ctx->rendering->postprocessing.gamma = gamma;
}

float rendering_getThreshold(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.bloom.threshold;
}

float rendering_getThresholdEmissionWeight(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.bloom.emissionWeight;
}

float rendering_getThresholdColorWeight(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.bloom.colorWeight;
}

int rendering_getBloomBlurIterations(const ProgContext *ctx) {
    return ctx->rendering->postprocessing.bloom.blurIterations;
}

void rendering_setThreshold(const ProgContext *ctx, float threshold) {
    ctx->rendering->postprocessing.bloom.threshold = threshold;
}

void rendering_setThresholdEmissionWeight(const ProgContext *ctx, float emissionWeight) {
    ctx->rendering->postprocessing.bloom.emissionWeight = emissionWeight;
}

void rendering_setThresholdColorWeight(const ProgContext *ctx, float colorWeight) {
    ctx->rendering->postprocessing.bloom.colorWeight = colorWeight;
}

void rendering_setBloomBlurIterations(const ProgContext *ctx, int blurIterations) {
    ctx->rendering->postprocessing.bloom.blurIterations = blurIterations;
}

void rendering_setFogColor(const ProgContext *ctx, vec4 fogColor) {
    ctx->rendering->fog.color[0] = fogColor[0];
    ctx->rendering->fog.color[1] = fogColor[1];
    ctx->rendering->fog.color[2] = fogColor[2];
}

void rendering_getFogColor(const ProgContext *ctx, vec4 outFogColor) {
    outFogColor[0] = ctx->rendering->fog.color[0];
    outFogColor[1] = ctx->rendering->fog.color[1];
    outFogColor[2] = ctx->rendering->fog.color[2];
    outFogColor[3] = 1;
}

bool rendering_getIsPointLightActive(const ProgContext *ctx) {
    return ctx->rendering->light.isPointLightActive;
}

bool rendering_getIsDirLightActive(const ProgContext *ctx) {
    return ctx->rendering->light.isDirLightActive;
}

void rendering_flipIsPointLightActive(const ProgContext *ctx) {
    ctx->rendering->light.isPointLightActive ^= 1;
}

void rendering_flipIsDirLightActive(const ProgContext *ctx) {
    ctx->rendering->light.isDirLightActive ^= 1;
}

void rendering_setPointLightColor(const ProgContext *ctx, vec4 color) {
    vec3 pos;
    glm_vec3_copy(ctx->rendering->light.defaultPointLight->position, pos);
    light_deletePointLight(ctx->rendering->light.defaultPointLight);

    vec3 lightColor;
    lightColor[0] = color[0];
    lightColor[1] = color[1];
    lightColor[2] = color[2];

    ctx->rendering->light.defaultPointLight = light_createPointLight(pos, lightColor);

    if (ctx->input->rendering.userScene != NULL && ctx->input->rendering.userScene->countPointLights > 0) {
        ctx->input->rendering.userScene->pointLights[0] = ctx->rendering->light.defaultPointLight;
    }
}

void rendering_getPointLightColor(const ProgContext *ctx, vec4 outColor) {
    outColor[0] = ctx->rendering->light.defaultPointLight->diffuse[0] / DIFFUSE_FACTOR;
    outColor[1] = ctx->rendering->light.defaultPointLight->diffuse[1] / DIFFUSE_FACTOR;
    outColor[2] = ctx->rendering->light.defaultPointLight->diffuse[2] / DIFFUSE_FACTOR;
    outColor[3] = 1;
}

void rendering_setDirLightColor(const ProgContext *ctx, vec4 color) {
    vec3 dir;
    glm_vec3_copy(ctx->rendering->light.defaultDirLight->direction, dir);
    light_deleteDirLight(ctx->rendering->light.defaultDirLight);

    vec3 lightColor;
    lightColor[0] = color[0];
    lightColor[1] = color[1];
    lightColor[2] = color[2];

    ctx->rendering->light.defaultDirLight = light_createDirLight(dir, lightColor);

    if (ctx->input->rendering.userScene != NULL && ctx->input->rendering.userScene->countDirLights > 0) {
        ctx->input->rendering.userScene->dirLights[0] = ctx->rendering->light.defaultDirLight;
    }
}


void rendering_getDirLightColor(const ProgContext *ctx, vec4 outColor) {
    outColor[0] = ctx->rendering->light.defaultDirLight->diffuse[0] / DIFFUSE_FACTOR;
    outColor[1] = ctx->rendering->light.defaultDirLight->diffuse[1] / DIFFUSE_FACTOR;
    outColor[2] = ctx->rendering->light.defaultDirLight->diffuse[2] / DIFFUSE_FACTOR;
    outColor[3] = 1;
}

void rendering_setDirLightDirection(const ProgContext *ctx, vec3 dir) {
    glm_vec3_copy(dir, ctx->rendering->light.defaultDirLight->direction);
}

void rendering_getDirLightDirection(const ProgContext *ctx, vec3 outDir) {
    glm_vec3_copy(ctx->rendering->light.defaultDirLight->direction, outDir);
}

bool rendering_getShowShadows(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.showShadows;
}

bool rendering_getUsePCF(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.usePCF;
}

bool rendering_getAlwaysUpdateDirShadows(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.dirLightShadowsAlwaysUpdate;
}

void rendering_setAlwaysUpdateDirShadows(const ProgContext *ctx, bool value)
{
    ctx->rendering->shadowMap.dirLightShadowsAlwaysUpdate = value;
}

void rendering_setShouldUpdateDirShadows(const ProgContext *ctx)
{
    ctx->rendering->shadowMap.dirLightShadowsShouldUpdate = true;
}

void rendering_setShouldUpdatePointShadows(const ProgContext *ctx)
{
    ctx->rendering->shadowMap.pointLightShadowsShouldUpdate = true;
}

void rendering_setshowShadows(const ProgContext *ctx, bool value)
{
    ctx->rendering->shadowMap.showShadows = value;
}

void rendering_setUsePCF(const ProgContext *ctx, bool value)
{
    ctx->rendering->shadowMap.usePCF = value;
}

float rendering_getDirLightDistanceMult(const ProgContext *ctx)
{
    return ctx->rendering->light.dirLightDistanceMult;
}

void rendering_setDirLightDistanceMult(const ProgContext *ctx, float value)
{
    ctx->rendering->light.dirLightDistanceMult = value;
}

void rendering_setUseDoF(const ProgContext *ctx, bool value)
{
    ctx->rendering->postprocessing.useDoF = value;
}

void rendering_setDepthOfField(const ProgContext *ctx, float value)
{
    ctx->rendering->postprocessing.depthOfField = value;
}

void rendering_setFocusDistance(const ProgContext *ctx, float value)
{
    ctx->rendering->postprocessing.focusDistance = value;
}

bool rendering_getUseDoF(const ProgContext *ctx)
{
    return ctx->rendering->postprocessing.useDoF;
}

float rendering_getDepthOfField(const ProgContext *ctx)
{
    return ctx->rendering->postprocessing.depthOfField;
}

float rendering_getFocusDistance(const ProgContext *ctx)
{
    return ctx->rendering->postprocessing.focusDistance;
}

float rendering_getZNear(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.zNear;
}

float rendering_getZFar(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.zFar;
}

float rendering_getQuadSize(const ProgContext *ctx)
{
    return ctx->rendering->shadowMap.quadSize;
}

void rendering_setZNear(const ProgContext *ctx, float value)
{
    ctx->rendering->shadowMap.zNear = value;;
}

void rendering_setZFar(const ProgContext *ctx, float value)
{
    ctx->rendering->shadowMap.zFar = value;
}

void rendering_setQuadSize(const ProgContext *ctx, float value)
{
    ctx->rendering->shadowMap.quadSize = value;
}

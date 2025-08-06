/**
 * Modul für das Verwalten eines Geometry-Buffers (kurz GBuffer).
 *
 * Dieses Modul dient nur der Orientierung und sollte
 * umfangreich verändert werden. Viele Stellen sind mit einem TODO markiert,
 * diese müssen von euch gefüllt werden. Grundsätzlich dürft ihr auch die Struktur
 * komplett über Bord werfen und euer eigenes Modul aufsetzen.
 * Wenn ihr dieses Modul als Grundlage nehmt, denkt daran alle TODOs und diesen
 * Kommentar zu entfernen. Passt auch den Autor an!
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Jonas Sorgenfrei, Nicolas Hollmann
 */

#include "gbuffer.h"

#include <string.h>

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

/**
 * @struct GBuffer
 * @brief Struktur zur Speicherung der GBuffer-Informationen für das Rendering.
 *
 * Die Struktur enthält Framebuffer-Objekte (FBOs) und Texturen für verschiedene Rendering-Pässe,
 * einschließlich Standard-Rendering, Blur-Effekt und Schattenberechnung für Punkt- und Richtungslichter.
 */
struct GBuffer
{
    GLuint defaultFBO; /**< Das Standard-Framebuffer-Objekt für das Haupt-Rendering. */
    GLuint blurFBO; /**< Das Framebuffer-Objekt für den Blur-Effekt. */
    GLuint dirLightShadowFBO; /**< Das Framebuffer-Objekt für Richtungslicht-Schatten. */
    GLuint *pointLightFBOs; /**< Dynamisches Array von Framebuffer-Objekten für Punktlicht-Schatten. */

    GLuint defaultTextures[DEFAULT_GBUFFER_NUM_COLORATTACH]; /**< Array der Standard-GBuffer-Texturen. */
    GLuint blurTextures[BLUR_GBUFFER_NUM_COLORATTACH]; /**< Array der Blur-Texturen für den Blur-Pass. */
    GLuint depthTexture; /**< Tiefen-Textur für das Standard-Framebuffer. */

    GLuint *pointLightDepthMaps; /**< Dynamisches Array von Tiefentexturen für Punktlichter. */
    GLuint dirLightDepthMap; /**< Tiefen-Textur für das Richtungslicht. */

    int allocatedPointLightFBOs; /**< Anzahl der allokierten Punktlicht-FBOs. */
    int shadowSize; /**< Die Auflösung der Schatten-Texturen. */
};

/**
 * @brief Bereinigt die Ressourcen für Punktlicht-Framebuffer-Objekte und Tiefentexturen.
 *
 * Diese Funktion löscht die Framebuffer und Tiefentexturen, die für Punktlichter verwendet wurden,
 * und gibt den belegten Speicher frei.
 *
 * @param gbuffer Der GBuffer, dessen Punktlicht-FBOs bereinigt werden sollen.
 */
static void cleanupPointLightFBOs(GBuffer *gbuffer) {
    if (gbuffer->pointLightFBOs) {
        for (int i = 0; i < gbuffer->allocatedPointLightFBOs; ++i) {
            glDeleteFramebuffers(1, &gbuffer->pointLightFBOs[i]);
        }
        free(gbuffer->pointLightFBOs);
    }

    if (gbuffer->pointLightDepthMaps) {
        for (int i = 0; i < gbuffer->allocatedPointLightFBOs; ++i) {
            glDeleteTextures(1, &gbuffer->pointLightDepthMaps[i]);
        }
        free(gbuffer->pointLightDepthMaps);
    }

    gbuffer->pointLightFBOs = NULL;
    gbuffer->pointLightDepthMaps = NULL;
}

/**
 * @brief Reserviert Speicher für eine Punktlicht-Framebuffer-Array.
 *
 * @param gbuffer Der GBuffer.
 * @param count Die Anzahl der Punktlichter.
 */
static void gbuffer_mallocPointLightFBOArray(GBuffer* gbuffer, const int count)
{
    cleanupPointLightFBOs(gbuffer);

    gbuffer->allocatedPointLightFBOs = count;
    gbuffer->pointLightFBOs      = malloc(sizeof(GLuint) * count);
    gbuffer->pointLightDepthMaps = malloc(sizeof(GLuint) * count);
}

/**
 * @brief Erstellt ein Framebuffer-Objekt für ein Punktlicht.
 *
 * @param gbuffer Der GBuffer.
 * @param index Der Index des Punktlichts.
 * @param depthMapSize Die Größe der Depth Map.
 */
static void gbuffer_createPointLightFBO(GBuffer* gbuffer, const int index, const int depthMapSize)
{
    glGenFramebuffers(1, &gbuffer->pointLightFBOs[index]);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->pointLightFBOs[index]);
    common_labelObjectByType(GL_FRAMEBUFFER, gbuffer->pointLightFBOs[index], "Point Light FBO " + index);

    {
        glGenTextures(1, &gbuffer->pointLightDepthMaps[index]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gbuffer->pointLightDepthMaps[index]);
        for (int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, depthMapSize, depthMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbuffer->pointLightDepthMaps[index], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

GBuffer *gbuffer_createGBuffer(const int width, const int height, const int shadowSize) {
    // Wie immer muss zuerst der Speicher für den GBuffer angefordert werden.
    GBuffer *gbuffer = malloc(sizeof(GBuffer));
    memset(gbuffer, 0, sizeof(GBuffer));

    gbuffer->shadowSize = shadowSize;

    // Dann erstellen wir unser FBO (Framebuffer Object) und binden es direkt.
    glGenFramebuffers(1, &gbuffer->defaultFBO);
    glGenFramebuffers(1, &gbuffer->blurFBO);
    glGenFramebuffers(1, &gbuffer->dirLightShadowFBO);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->defaultFBO);

        // Position
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_POSITION]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_POSITION]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_POSITION, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_POSITION], 0);

        // Normal
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_NORMAL]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_NORMAL]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_NORMAL, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_NORMAL], 0);

        // Albedo Specular (metalness)
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC], 0);

        // Ambient Shininess
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI], 0);

        // Emission
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_EMISSION]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_EMISSION]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_EMISSION, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_EMISSION], 0);

        // Finales Ausgabebild
        glGenTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_FINAL]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_FINAL]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_FINAL, GL_TEXTURE_2D, gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_FINAL], 0);
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->defaultFBO);

        // Textur für Depth & Stencil Buffer
        glGenRenderbuffers(1, &gbuffer->depthTexture);
        glBindRenderbuffer(GL_RENDERBUFFER, gbuffer->depthTexture);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gbuffer->depthTexture);
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->blurFBO);

        // Blur Horizontal
        glGenTextures(1, &gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_H]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_H]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + BLUR_GBUFFER_COLORATTACH_BLUR_H, GL_TEXTURE_2D, gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_H], 0);

        // Blur Vertical
        glGenTextures(1, &gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_V]);
        glBindTexture(GL_TEXTURE_2D, gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_V]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + BLUR_GBUFFER_COLORATTACH_BLUR_V, GL_TEXTURE_2D, gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_V], 0);
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->dirLightShadowFBO);

        static const GLfloat borders[] = { 1.f, 1.f, 1.f, 1.f };

        glGenTextures(1, &gbuffer->dirLightDepthMap);
        glBindTexture(GL_TEXTURE_2D, gbuffer->dirLightDepthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gbuffer->shadowSize, gbuffer->shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borders);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbuffer->dirLightDepthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // Wir prüfen dann noch, ob das FBO erfolgreich angelegt wurde.
    const GLenum fboState = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboState != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Error: FBO not complete because:\n");

        // Wir geben auch den Grund für den Fehlschlag aus. Hier werden jedoch nur
        // die häufigsten Fälle abgefragt. Für mehr Informationen, oder wenn ihr
        // "Unknown" erhaltet, schaut euch die Dokumentation unter
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
        // an oder fragt nach.
        switch (fboState)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                fprintf(stderr, "\tIncomplete Attachment.\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                fprintf(stderr, "\tMissing Attachments.\n");
                break;

            case GL_FRAMEBUFFER_UNSUPPORTED:
                fprintf(stderr, "\tFramebuffer Unsupported.\n");
                break;

            default:
                fprintf(stderr, "\tUnknown. Please check gbuffer.c\n");
                break;
        }
    }

    common_labelObjectByType(GL_FRAMEBUFFER, gbuffer->defaultFBO, "Default FBO");
    common_labelObjectByType(GL_FRAMEBUFFER, gbuffer->blurFBO, "Blur FBO");
    common_labelObjectByType(GL_FRAMEBUFFER, gbuffer->dirLightShadowFBO, "Directional Light FBO");

    // Zum Schluss wechseln wir zurück zum Standard FBO und
    // geben den GBuffer zurück.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return gbuffer;
}

GLuint gbuffer_getDefaultTexture(GBuffer* gbuffer, DEFAULT_GBUFFER_TEXTURE_TYPE type) {
    return gbuffer->defaultTextures[type];
}

GLuint gbuffer_getBlurTexture(GBuffer* gbuffer, BLUR_GBUFFER_TEXTURE_TYPE type) {
    return gbuffer->blurTextures[type];
}

GLuint gbuffer_getDirLightShadowMap(GBuffer* gbuffer) {
    return gbuffer->dirLightDepthMap;
}

GLuint gbuffer_getPointLightShadowMap(GBuffer* gbuffer, const int index) {
    if (index < 0 || index >= gbuffer->allocatedPointLightFBOs) {
        fprintf(stderr, "Zugriff auf nicht vorhandene Punktlicht Schatten Textur");
    }

    return gbuffer->pointLightDepthMaps[index];
}

void gbuffer_clearDefaultTexture(GBuffer *gbuffer, DEFAULT_GBUFFER_TEXTURE_TYPE textureType)
{
    // GBuffer FBO binden und den entsprechenden Color-Buffer leeren
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->defaultFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + textureType);
    glClear(GL_COLOR_BUFFER_BIT);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gbuffer_clearBlurTexture(GBuffer *gbuffer, BLUR_GBUFFER_TEXTURE_TYPE textureType)
{
    // GBuffer FBO binden und den entsprechenden Color-Buffer leeren
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->blurFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + textureType);
    glClear(GL_COLOR_BUFFER_BIT);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gbuffer_bindGBufferForGeomPass(GBuffer *gbuffer)
{
    // GBuffer FBO binden
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->defaultFBO);

    // Die zu verwendenden Attribute konfigurieren.
    GLenum drawBuffer[] = {
            GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_POSITION, // location 0
            GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_NORMAL,
            GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC,
            GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI,
            GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_EMISSION,
    };

    // Danach übergeben wir das Array mit den Color-Attachments.
    // ACHTUNG: Die Reihenfolge im Array gibt die Positionen im Shader an.
    glDrawBuffers(sizeof(drawBuffer) / sizeof(drawBuffer[0]), drawBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gbuffer_bindGBufferForStencilPass(GBuffer *gbuffer)
{
    // GBuffer-FBO binden
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->defaultFBO);
    glDrawBuffer(GL_NONE);
}

void gbuffer_bindGBufferForLightPass(GBuffer *gbuffer)
{
    // GBuffer FBO binden und das finale Render-Ausgabebild als DrawBuffer setzen
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->defaultFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
}

void gbuffer_bindGBufferForThreshold(GBuffer *gbuffer)
{
    // GBuffer FBO binden und den Threshold-Pass konfigurieren
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->blurFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + BLUR_GBUFFER_COLORATTACH_BLUR_H);
}

void gbuffer_bindGBufferForBlur(GBuffer *gbuffer, bool isHorizontal)
{
    // GBuffer FBO binden und den Blur-Pass konfigurieren
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->blurFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + (isHorizontal ? BLUR_GBUFFER_COLORATTACH_BLUR_H : BLUR_GBUFFER_COLORATTACH_BLUR_V));
}

void gbuffer_bindGBufferForFog(GBuffer *gbuffer)
{
    // GBuffer FBO binden und den Fog-Pass konfigurieren
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->defaultFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
}

void gbuffer_bindGBufferForPostprocess(GBuffer *gbuffer)
{
    // GBuffer FBO binden und den Fog-Pass konfigurieren
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->defaultFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + DEFAULT_GBUFFER_COLORATTACH_FINAL);
}

void gbuffer_bindGBufferForDirLightShadows(GBuffer *gbuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->dirLightShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void gbuffer_bindGBufferForPointLightShadow(GBuffer *gbuffer, const int index)
{
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->pointLightFBOs[index]);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_TEXTURE_TYPE textureType)
{
    // Auswahl von welchem Attachment gelesen werden soll.
    glReadBuffer(GL_COLOR_ATTACHMENT0 + textureType);
}

void gbuffer_bindForRead(GBuffer* gBuffer) {
    // GBuffer FBO zum Lesen binden
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->defaultFBO);
}

void gbuffer_initializePointLightFBOs(GBuffer* gbuffer, const int count, const int depthMapSize)
{
    // Speicher reservieren
    gbuffer_mallocPointLightFBOArray(gbuffer, count);

    // Punktlicht-FBOs erstellen
    for (int i = 0; i < count; ++i) {
        gbuffer_createPointLightFBO(gbuffer, i, depthMapSize);
    }
}

void gbuffer_deleteGBuffer(GBuffer *gbuffer)
{
    // FBO löschen.
    glDeleteFramebuffers(1, &gbuffer->defaultFBO);
    glDeleteFramebuffers(1, &gbuffer->blurFBO);
    glDeleteFramebuffers(1, &gbuffer->dirLightShadowFBO);

    // Die angehängten Texturen löschen.
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_POSITION]);
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_NORMAL]);
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC]);
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI]);
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_EMISSION]);
    glDeleteTextures(1, &gbuffer->defaultTextures[DEFAULT_GBUFFER_COLORATTACH_FINAL]);

    glDeleteTextures(1, &gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_H]);
    glDeleteTextures(1, &gbuffer->blurTextures[BLUR_GBUFFER_COLORATTACH_BLUR_V]);

    glDeleteTextures(1, &gbuffer->dirLightDepthMap);

    glDeleteRenderbuffers(1, &gbuffer->depthTexture);

    cleanupPointLightFBOs(gbuffer);

    free(gbuffer);
}

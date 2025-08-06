/**
 * Modul für das Verwalten eines Geometry-Buffers (kurz GBuffer).
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Jonas Sorgenfrei, Nicolas Hollmann, stud105751, stud104645
 */

#ifndef GBUFFER_H
#define GBUFFER_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Aufzählungstyp für die unterschiedlichen Color Attachments des GBuffers.
typedef enum DEFAULT_GBUFFER_TEXTURE_TYPE {
    DEFAULT_GBUFFER_COLORATTACH_POSITION,
    DEFAULT_GBUFFER_COLORATTACH_NORMAL,
    DEFAULT_GBUFFER_COLORATTACH_ALBEDOSPEC,
    DEFAULT_GBUFFER_COLORATTACH_EMISSION,
    DEFAULT_GBUFFER_COLORATTACH_AMBIENTSHI,
    DEFAULT_GBUFFER_COLORATTACH_FINAL,

    DEFAULT_GBUFFER_NUM_COLORATTACH
} DEFAULT_GBUFFER_TEXTURE_TYPE;

typedef enum BLUR_GBUFFER_TEXTURE_TYPE {
    BLUR_GBUFFER_COLORATTACH_BLUR_V,
    BLUR_GBUFFER_COLORATTACH_BLUR_H,

    BLUR_GBUFFER_NUM_COLORATTACH
} BLUR_GBUFFER_TEXTURE_TYPE;

// GBuffer Datentyp.
struct GBuffer;
typedef struct GBuffer GBuffer;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erstellt einen neuen GBuffer mit der angegebenen Größe.
 *
 * @param width die Breite des Buffers
 * @param height die Höhe des Buffers
 * @param shadowSize Schatten Textur Auflösung
 * @return der neue GBuffer
 */
GBuffer *gbuffer_createGBuffer(int width, int height, int shadowSize);

/**
 * Gibt die Textur des angegebenen Typs zurück.
 *
 * @param gbuffer der GBuffer
 * @param type der Typ der Textur
 * @return die Textur-ID
 */
GLuint gbuffer_getDefaultTexture(GBuffer* gbuffer, DEFAULT_GBUFFER_TEXTURE_TYPE type);


/**
 * Gibt die Textur des angegebenen Typs zurück.
 *
 * @param gbuffer der GBuffer
 * @param type der Typ der Textur
 * @return die Textur-ID
 */
GLuint gbuffer_getBlurTexture(GBuffer* gbuffer, BLUR_GBUFFER_TEXTURE_TYPE type);

/**
 * @brief Gibt die Depth Map für das Richtungslicht zurück.
 *
 * @param gbuffer Der GBuffer.
 * @return Die Textur-ID der Depth Map.
 */
GLuint gbuffer_getDirLightShadowMap(GBuffer* gbuffer);

/**
 * @brief Gibt die Depth Map für das Punktlicht zurück.
 *
 * @param gbuffer Der GBuffer.
 * @param index Der Index des Punktlichts.
 * @return Die Textur-ID der Punktlicht-Depth Map.
 */
GLuint gbuffer_getPointLightShadowMap(GBuffer* gbuffer, int index);

/**
 * Bindet das GBuffer FBO, setzt die finale Render Ausgabe als DrawBuffer und
 * cleart den Color-Buffer dieses Attachments.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_clearDefaultTexture(GBuffer *gbuffer, DEFAULT_GBUFFER_TEXTURE_TYPE textureType);

/**
 * Bindet das GBuffer FBO, setzt die finale Render Ausgabe als DrawBuffer und
 * cleart den Color-Buffer dieses Attachments.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_clearBlurTexture(GBuffer *gbuffer, BLUR_GBUFFER_TEXTURE_TYPE textureType);

/**
 * Bindet das GBuffer FBO und setzt die entsprechenden Color_Attachments für
 * das den Geometry Pass Render Vorgang. Der Farb und Tiefenbuffer wird geleert.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForGeomPass(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO und deaktiviert das schreiben auf einen
 * Draw-Buffer
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForStencilPass(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO, setzt die finale Render Ausgabe als DrawBuffer und
 * bindet die GBuffer Texturen als Texturen
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForLightPass(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO für den Threshold-Pass.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForThreshold(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO für den Blur-Pass.
 *
 * @param gbuffer der GBuffer
 * @param isHorizontal gibt an, ob der Blur horizontal ist
 */
void gbuffer_bindGBufferForBlur(GBuffer *gbuffer, bool isHorizontal);

/**
 * Bindet das GBuffer FBO für den Fog-Pass.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForFog(GBuffer *gbuffer);

/**
 * @brief Bindet das GBuffer FBO für die Post-Processing Phase.
 *
 * @param gbuffer Der GBuffer.
 */
void gbuffer_bindGBufferForPostprocess(GBuffer *gbuffer);

/**
 * @brief Bindet das GBuffer FBO für das Rendering von Richtungslicht-Schatten.
 *
 * @param gbuffer Der GBuffer.
 */
void gbuffer_bindGBufferForDirLightShadows(GBuffer *gbuffer);


/**
 * @brief Bindet das GBuffer FBO für das Rendering von Punktlicht-Schatten.
 *
 * @param gbuffer Der GBuffer.
 * @param index Der Index des Punktlichts.
 */
void gbuffer_bindGBufferForPointLightShadow(GBuffer *gbuffer, int index);

/**
 * Setzt die Textur zum Lesen aus dem Framebuffer
 *
 * @param textureType welches Color Attachment fürs Lesen gebunden werden soll
 */
void gbuffer_bindGBufferForTextureRead(DEFAULT_GBUFFER_TEXTURE_TYPE textureType);

/**
 * Bindet den GBuffer zum Lesen.
 *
 * @param gBuffer der GBuffer
 */
void gbuffer_bindForRead(GBuffer* gBuffer);

/**
 * @brief Initialisiert die Framebuffer und Tiefentexturen für Punktlichter.
 *
 * Diese Funktion bereinigt bestehende Punktlicht-FBOs, reserviert Speicher
 * für neue FBOs und erstellt die Punktlicht-Framebuffer-Objekte und zugehörigen Tiefentexturen.
 *
 * @param gbuffer Zeiger auf den GBuffer, der die Punktlicht-FBOs verwaltet.
 * @param count Die Anzahl der Punktlichter.
 * @param depthMapSize Die Größe der Tiefentextur für jedes Punktlicht.
 */
void gbuffer_initializePointLightFBOs(GBuffer* gbuffer, int count, int depthMapSize);
/**
 * Löscht den übergebenen GBuffer wieder.
 *
 * @param gbuffer der zu löschende GBuffer
 */
void gbuffer_deleteGBuffer(GBuffer* gbuffer);

#endif //GBUFFER_H

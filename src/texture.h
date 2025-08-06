/**
 * Modul für das Laden und Schreiben von Texturen.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

typedef enum {
 TEXTURE_UNIT_DIFFUSE_MAP   =  0,  // Diffuse Texture
 TEXTURE_UNIT_SPECULAR_MAP  =  1,  // Specular Texture
 TEXTURE_UNIT_NORMAL_MAP    =  2,  // Normal Map Texture
 TEXTURE_UNIT_EMISSION_MAP  =  3,  // Emission Map Texture
                                    // 4 -> Skybox
 TEXTURE_UNIT_DISPLACEMENT_MAP = 5, // Displacement Map Texture
 TEXTURE_UNIT_CUBEMAP       = 10, // Cube Map Texture
} TextureUnit;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt eine OpenGL Textur aus einer Bilddatei.
 * Es werden auch DDS Dateien unterstützt.
 *
 * Im Fehlerfall wird immer eine korrekte Textur-ID zurückgegeben. Allerdings
 * fehlen unter umständen die nötigen Bilddaten.
 *
 * @param filename der Pfad zur Bilddatei
 * @param wrapping der Wrapping Modus (z.B. GL_REPEAT, GL_MIRRORED_REPEAT,
 *        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER)
 * @return eine OpenGL Textur ID
 */
GLuint texture_loadTexture(const char* filename, GLenum wrapping, bool useSRGB);

/**
 * Lädt eine Cubemap-Textur aus einer Liste von Bilddateien.
 * Diese Funktion gibt die OpenGL Textur-ID der erstellten Cubemap zurück.
 *
 * @param faces Ein Array von Dateinamen, die die sechs Seiten der Cubemap darstellen.
 *              Die Reihenfolge sollte sein: right, left, top, bottom, front, back.
 * @return Die OpenGL Textur-ID der Cubemap oder 0 im Fehlerfall.
 */
GLuint texture_loadCubemap(const char* faces[]);

/**
 * Löscht eine zuvor angelegte Textur wieder.
 * Die Textur-ID muss valide und noch nicht gelöscht sein.
 *
 * @param textureId die Textur-ID der Textur, die gelöscht werden soll.
 */
void texture_deleteTexture(GLuint textureId);

/**
 * Speichert einen Screenshot in dem Programmverzeichnis.
 * Der Dateiname lautet screenshot_yyyy-MM-dd_hh-mm-ss.png wobei das aktuelle
 * Datum und die aktuelle Uhrzeit eingesetzt wird.
 * Es wird grundsätzlich der aktive Framebuffer ausgelesen.
 *
 * @param ctx der aktuelle Programmkontext
 */
void texture_saveScreenshot(ProgContext* ctx);

/**
 * Leert den Textur-Cache.
 */
void texture_EmptyTextureCache(void);
#endif // TEXTURE_H

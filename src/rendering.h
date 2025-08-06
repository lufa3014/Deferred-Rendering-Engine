/**
 * Modul zum Rendern der 3D Szene.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#ifndef RENDERING_H
#define RENDERING_H

#include "common.h"

typedef enum {
 RENDER_MODE_PHONG,
 RENDER_MODE_DEBUG,
 // Neue Modi hier hinzufügen...

 RENDER_MODE_COUNT // Anzahl der Modi, um die Dropdown-Länge zu bestimmen
} RenderMode;

#define DIR_SHADOW_SIZE 1024
#define POINT_SHADOW_SIZE 512

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Initialisiert das Rendering-Modul.
 *
 * @param ctx Programmkontext.
 */
void rendering_init(ProgContext* ctx);

/**
 * Rendert die 3D Szene.
 *
 * @param ctx Programmkontext.
 */
void rendering_draw(const ProgContext* ctx);

/**
 * Gibt den aktuell ausgewählten Anzeigemodus zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Rendering-Modus, basierend auf der `RenderMode`-Enum.
 */
RenderMode rendering_getSelectedRenderMode(const ProgContext* ctx);

/**
 * Setzt den aktuellen Anzeigemodus für das Rendering.
 *
 * Diese Funktion ermöglicht es, zwischen verschiedenen Rendering-Modi zur Laufzeit zu wechseln.
 * Dies ist nützlich, um beispielsweise Debug-Informationen anzuzeigen (z.B. Normalen, Textur-Koordinaten)
 * oder alternative visuelle Effekte (z.B. Cubemap-Reflektionen) zu aktivieren.
 *
 * @param ctx Programmkontext.
 * @param mode Der Rendering-Modus, der angewendet werden soll. Muss ein gültiger Wert aus dem
 *             `RenderMode`-Enum sein. Beispiele sind:
 *             - `RENDER_MODE_PHONG` für klassisches Phong-Shading.
 *             - `RENDER_MODE_NORMALS` um Normalen als Farbe anzuzeigen.
 *             - `RENDER_MODE_CUBEMAP_REFLECTION` für reflektierende Oberflächen.
 *             - Weitere Modi siehe `RenderMode`-Enum.
 */
void rendering_selectRenderMode(const ProgContext* ctx, RenderMode mode);

/**
 * Rekompiliert die Shader, die im Rendering-Modul verwendet werden.
 *
 * Diese Funktion lädt die Shader-Dateien neu, kompiliert sie erneut und verlinkt
 * die Shader-Programme. Sie ermöglicht es, Änderungen an den Shader-Quellcodes
 * zur Laufzeit zu übernehmen, ohne das Programm neu zu starten.
 *
 * @param ctx Programmkontext.
 * @return true, wenn die Operation erfolgreich war, false wenn nicht.
 */
bool rendering_recompileShader(const ProgContext* ctx);

/**
 * Gibt den Status des Skybox-Renderings zurück.
 *
 * @param ctx Programmkontext.
 * @return true, wenn das Skybox-Rendering aktiviert ist, sonst false.
 */
bool rendering_getSkyboxEnabled(const ProgContext* ctx);

/**
 * Aktiviert das Skybox-Rendering.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_enableSkybox(const ProgContext* ctx);

/**
 * Deaktiviert das Skybox-Rendering.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_disableSkybox(const ProgContext* ctx);

/**
 * Gibt den Status des Normalen-Mappings zurück.
 * @param ctx Programmkontext.
 * @return true, wenn das Normalen-Mapping aktiviert ist, sonst false.
 */
bool rendering_getNormalMappingEnabled(const ProgContext* ctx);

/**
 * Aktiviert das Normalen-Mapping.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_enableNormalMapping(const ProgContext* ctx);

/**
 * Deaktiviert das Normalen-Mapping.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_disableNormalMapping(const ProgContext* ctx);

/**
 * Aktiviert das Zwei-Kanal-Normalen-Mapping.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_enableTwoChannelNormalMap(const ProgContext* ctx);

/**
 * Deaktiviert das Zwei-Kanal-Normalen-Mapping.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_disableTwoChannelNormalMap(const ProgContext* ctx);

/**
 * Gibt den Status des Zwei-Kanal-Normalen-Mappings zurück.
 *
 * @param ctx Der Programmkontext.
 * @return true, wenn das Zwei-Kanal-Normalen-Mapping aktiviert ist, sonst false.
 */
bool rendering_getTwoChannelNormalMapEnabled(const ProgContext* ctx);

/**
 * Gibt die Translation der Szene zurück.
 *
 * @param ctx Der Programmkontext.
 * @param outTranslation Der Vektor, in den die Translation geschrieben wird.
 */
void rendering_getTranslation(const ProgContext* ctx, vec3 outTranslation);

/**
 * Gibt die Rotation der Szene zurück.
 *
 * @param ctx Der Programmkontext.
 * @param outRotation Der Vektor, in den die Rotation geschrieben wird.
 */
void rendering_getRotation(const ProgContext* ctx, vec3 outRotation);

/**
 * Gibt die Skalierung der Szene zurück.
 *
 * @param ctx Der Programmkontext.
 * @param outScale Der Vektor, in den die Skalierung geschrieben wird.
 */
void rendering_getScale(const ProgContext* ctx, vec3 outScale);

/**
 * Gibt den Status der Tesselation zurück.
 *
 * @param ctx Der Programmkontext.
 * @return true, wenn die Tesselation aktiviert ist, sonst false.
 */
bool rendering_getTesselationEnabled(const ProgContext* ctx);

/**
 * Gibt den Status des Displacement zurück.
 *
 * @param ctx Der Programmkontext.
 * @return true, wenn die Tesselation aktiviert ist, sonst false.
 */
bool rendering_getDisplacementEnabled(const ProgContext* ctx);

/**
 * Gibt die minimale Tesselationsstufe zurück.
 *
 * @param ctx Der Programmkontext.
 * @return Die minimale Tesselationsstufe.
 */
int rendering_getTesselationMin(const ProgContext* ctx);

/**
 * Gibt die maximale Tesselationsstufe zurück.
 *
 * @param ctx Der Programmkontext.
 * @return Die maximale Tesselationsstufe.
 */
int rendering_getTesselationMax(const ProgContext* ctx);

/**
 * Setzt die Translation der Szene.
 * @param ctx Der Programmkontext.
 * @param translation Die neue Translation.
 */
void rendering_setTranslation(const ProgContext* ctx, vec3 translation);

/**
 * Setzt die Rotation der Szene.
 * @param ctx Der Programmkontext.
 * @param rotation Die neue Rotation.
 */
void rendering_setRotation(const ProgContext* ctx, vec3 rotation);

/**
 * Setzt die Skalierung der Szene.
 * @param ctx Der Programmkontext.
 * @param scale Die neue Skalierung.
 */
void rendering_setScale(const ProgContext* ctx, vec3 scale);

/**
 * Aktiviert die Tesselation.
 * @param ctx Der Programmkontext.
 */
void rendering_enableTesselation(const ProgContext* ctx);

/**
 * Deaktiviert die Tesselation.
 * @param ctx Der Programmkontext.
 */
void rendering_disableTesselation(const ProgContext* ctx);

/**
 * Aktiviert das displacement.
 * @param ctx Der Programmkontext.
 */
void rendering_enableDisplacement(const ProgContext* ctx);

/**
 * Deaktiviert das Displacement.
 * @param ctx Der Programmkontext.
 */
void rendering_disableDisplacement(const ProgContext* ctx);

/**
 * Setzt die minimale Tesselationsstufe.
 * @param ctx Der Programmkontext.
 * @param min Die neue minimale Tesselationsstufe.
 */
void rendering_setTesselationMin(const ProgContext* ctx, int min);

/**
 * Setzt die maximale Tesselationsstufe.
 * @param ctx Der Programmkontext.
 * @param max Die neue maximale Tesselationsstufe.
 */
void rendering_setTesselationMax(const ProgContext* ctx, int max);

/**
 * Gibt den Status des Nebel-Renderings zurück.
 *
 * @param ctx Programmkontext.
 * @return true, wenn das Nebel-Rendering aktiviert ist, sonst false.
 */
bool rendering_getFogEnabled(const ProgContext* ctx);

/**
 * Aktiviert das Nebel-Rendering.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_enableFog(const ProgContext* ctx);

/**
 * Deaktiviert das Nebel-Rendering.
 *
 * @param ctx Der Programmkontext.
 */
void rendering_disableFog(const ProgContext* ctx);

/**
 * Gibt die aktuelle Dichte des Nebels zurück.
 *
 * @param ctx Programmkontext.
 * @return Die aktuelle Nebeldichte. Höhere Werte bedeuten dichteren Nebel.
 */
float rendering_getFogDensity(const ProgContext* ctx);

/**
 * Setzt die Dichte des Nebels.
 *
 * @param ctx Programmkontext.
 * @param density Die neue Nebeldichte. Höhere Werte bedeuten dichteren Nebel.
 */
void rendering_setFogDensity(const ProgContext* ctx, float density);

/**
 * Gibt den aktuellen Alpha-Clipping-Schwellenwert zurück.
 *
 * @param ctx Programmkontext.
 * @return Aktueller Alpha-Clipping-Wert.
 */
float rendering_getAlphaClipping(const ProgContext* ctx);

/**
 * Setzt den Alpha-Clipping-Schwellenwert.
 *
 * @param ctx Programmkontext.
 * @param clipping Neuer Alpha-Clipping-Schwellenwert.
 */
void rendering_setAlphaClipping(const ProgContext* ctx, float clipping);

/**
 * Gibt die Ressourcen des Rendering-Moduls wieder frei.
 *
 * @param ctx Programmkontext.
 */
void rendering_cleanup(const ProgContext* ctx);

void rendering_updateSceneData(const ProgContext* ctx);

/**
 * Setzt den Displacement Factor.
 *
 * @param ctx Programmkontext.
 * @param clipping Neuer Alpha-Clipping-Schwellenwert.
 */
void rendering_setDisplacementFactor(const ProgContext* ctx, float factor);
/**
 * Gibt den Displacement-Faktor zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Displacement-Faktor.
 */
float rendering_getDisplacementFactor(const ProgContext *ctx);

/**
 * Aktualisiert den Framebuffer.
 *
 * @param ctx Programmkontext.
 */
void rendering_updateFramebuffer(const ProgContext *ctx);

/**
 * Gibt die aktuelle Gamma-Exposition zurück.
 *
 * @param ctx Programmkontext.
 * @return Die aktuelle Gamma-Exposition.
 */
float rendering_getGammaExposure(const ProgContext *ctx);

/**
 * Gibt den aktuellen Gamma-Wert zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Gamma-Wert.
 */
float rendering_getGamma(const ProgContext *ctx);

/**
 * Setzt die Gamma-Exposition.
 *
 * @param ctx Programmkontext.
 * @param exposure Die neue Gamma-Exposition.
 */
void rendering_setGammaExposure(const ProgContext *ctx, float exposure);

/**
 * Setzt den Gamma-Wert.
 *
 * @param ctx Programmkontext.
 * @param gamma Der neue Gamma-Wert.
 */
void rendering_setGamma(const ProgContext *ctx, float gamma);

/**
 * Gibt den aktuellen Schwellenwert zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Schwellenwert.
 */
float rendering_getThreshold(const ProgContext *ctx);

/**
 * Gibt das aktuelle Emissionsgewicht des Schwellenwerts zurück.
 *
 * @param ctx Programmkontext.
 * @return Das aktuelle Emissionsgewicht des Schwellenwerts.
 */
float rendering_getThresholdEmissionWeight(const ProgContext *ctx);

/**
 * Gibt das aktuelle Farbgewicht des Schwellenwerts zurück.
 *
 * @param ctx Programmkontext.
 * @return Das aktuelle Farbgewicht des Schwellenwerts.
 */
float rendering_getThresholdColorWeight(const ProgContext *ctx);

/**
 * Gibt die Anzahl der Iterationen für den Bloom-Blur zurück.
 *
 * @param ctx Programmkontext.
 * @return Die Anzahl der Iterationen für den Bloom-Blur.
 */
int rendering_getBloomBlurIterations(const ProgContext *ctx);

/**
 * Setzt den Schwellenwert.
 *
 * @param ctx Programmkontext.
 * @param threshold Der neue Schwellenwert.
 */
void rendering_setThreshold(const ProgContext *ctx, float threshold);

/**
 * Setzt das Emissionsgewicht des Schwellenwerts.
 *
 * @param ctx Programmkontext.
 * @param emissionWeight Das neue Emissionsgewicht des Schwellenwerts.
 */
void rendering_setThresholdEmissionWeight(const ProgContext *ctx, float emissionWeight);

/**
 * Setzt das Farbgewicht des Schwellenwerts.
 *
 * @param ctx Programmkontext.
 * @param colorWeight Das neue Farbgewicht des Schwellenwerts.
 */
void rendering_setThresholdColorWeight(const ProgContext *ctx, float colorWeight);

/**
 * Setzt die Anzahl der Iterationen für den Bloom-Blur.
 *
 * @param ctx Programmkontext.
 * @param blurIterations Die neue Anzahl der Iterationen für den Bloom-Blur.
 */
void rendering_setBloomBlurIterations(const ProgContext *ctx, int blurIterations);

/**
 * Setzt die Nebelfarbe.
 *
 * @param ctx Programmkontext.
 * @param fogColor Die neue Nebelfarbe.
 */
void rendering_setFogColor(const ProgContext *ctx, vec4 fogColor);

/**
 * Gibt die aktuelle Nebelfarbe zurück.
 *
 * @param ctx Programmkontext.
 * @param outFogColor Der Vektor, in den die Nebelfarbe geschrieben wird.
 */
void rendering_getFogColor(const ProgContext *ctx, vec4 outFogColor);

/**
 * Gibt den Status der Punktlichtquelle zurück.
 *
 * @param ctx Programmkontext.
 * @return true, wenn die Punktlichtquelle aktiv ist, sonst false.
 */
bool rendering_getIsPointLightActive(const ProgContext *ctx);

/**
 * Gibt den Status der Richtungslichtquelle zurück.
 *
 * @param ctx Programmkontext.
 * @return true, wenn die Richtungslichtquelle aktiv ist, sonst false.
 */
bool rendering_getIsDirLightActive(const ProgContext *ctx);

/**
 * Schaltet den Status der Punktlichtquelle um.
 *
 * @param ctx Programmkontext.
 */
void rendering_flipIsPointLightActive(const ProgContext *ctx);

/**
 * Schaltet den Status der Richtungslichtquelle um.
 *
 * @param ctx Programmkontext.
 */
void rendering_flipIsDirLightActive(const ProgContext *ctx);

/**
 * Setzt die Farbe der Punktlichtquelle.
 *
 * @param ctx Programmkontext.
 * @param color Die neue Farbe der Punktlichtquelle.
 */
void rendering_setPointLightColor(const ProgContext *ctx, vec4 color);

/**
 * Gibt die aktuelle Farbe der Punktlichtquelle zurück.
 *
 * @param ctx Programmkontext.
 * @param outColor Der Vektor, in den die Farbe der Punktlichtquelle geschrieben wird.
 */
void rendering_getPointLightColor(const ProgContext *ctx, vec4 outColor);

/**
 * Setzt die Farbe der Richtungslichtquelle.
 *
 * @param ctx Programmkontext.
 * @param color Die neue Farbe der Richtungslichtquelle.
 */
void rendering_setDirLightColor(const ProgContext *ctx, vec4 color);

/**
 * Gibt die aktuelle Farbe der Richtungslichtquelle zurück.
 *
 * @param ctx Programmkontext.
 * @param outColor Der Vektor, in den die Farbe der Richtungslichtquelle geschrieben wird.
 */
void rendering_getDirLightColor(const ProgContext *ctx, vec4 outColor);

/**
 * Setzt die Richtung der Richtungslichtquelle.
 *
 * @param dir Die neue Richtung der Richtungslichtquelle.
 */
void rendering_setDirLightDirection(const ProgContext *ctx, vec3 dir);

/**
 * @brief Gibt zurück, ob Schatten angezeigt werden sollen.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @return True, wenn Schatten angezeigt werden sollen, sonst False.
 */
bool rendering_getShowShadows(const ProgContext *ctx);

/**
 * @brief Gibt zurück, ob Percentage Closer Filtering (PCF) für Schatten verwendet wird.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @return True, wenn PCF aktiviert ist, sonst False.
 */
bool rendering_getUsePCF(const ProgContext *ctx);

/**
 * @brief Gibt zurück, ob Richtungslicht-Schatten immer aktualisiert werden sollen.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @return True, wenn Richtungslicht-Schatten immer aktualisiert werden sollen, sonst False.
 */
bool rendering_getAlwaysUpdateDirShadows(const ProgContext *ctx);

/**
 * @brief Setzt, ob Richtungslicht-Schatten immer aktualisiert werden sollen.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @param value True, um die ständige Aktualisierung zu aktivieren, sonst False.
 */
void rendering_setAlwaysUpdateDirShadows(const ProgContext *ctx, bool value);

/**
 * @brief Setzt das Flag, dass Richtungslicht-Schatten aktualisiert werden sollen.
 *
 * Diese Funktion markiert die Richtungslicht-Schatten zur Aktualisierung in der nächsten Renderphase.
 *
 * @param ctx Zeiger auf den Programmkontext.
 */
void rendering_setShouldUpdateDirShadows(const ProgContext *ctx);

/**
 * @brief Setzt das Flag, dass Punktlicht-Schatten aktualisiert werden sollen.
 *
 * Diese Funktion markiert die Punktlicht-Schatten zur Aktualisierung in der nächsten Renderphase.
 *
 * @param ctx Zeiger auf den Programmkontext.
 */
void rendering_setShouldUpdatePointShadows(const ProgContext *ctx);

/**
 * @brief Setzt, ob Schatten angezeigt werden sollen.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @param value True, um Schatten anzuzeigen, sonst False.
 */
void rendering_setshowShadows(const ProgContext *ctx, bool value);

/**
 * @brief Setzt, ob Percentage Closer Filtering (PCF) für Schatten verwendet werden soll.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @param value True, um PCF zu aktivieren, sonst False.
 */
void rendering_setUsePCF(const ProgContext *ctx, bool value);

/**
 * @brief Gibt den Distanz-Multiplikator für das Richtungslicht zurück.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @return Der aktuelle Distanz-Multiplikator für das Richtungslicht.
 */
float rendering_getDirLightDistanceMult(const ProgContext *ctx);

/**
 * @brief Setzt den Distanz-Multiplikator für das Richtungslicht.
 *
 * @param ctx Zeiger auf den Programmkontext.
 * @param value Der neue Distanz-Multiplikator für das Richtungslicht.
 */
void rendering_setDirLightDistanceMult(const ProgContext *ctx, float value);

/**
 * Gibt die aktuelle Richtung der Richtungslichtquelle zurück.
 *
 * @param ctx Programmkontext.
 * @param outDir Der Vektor, in den die Richtung der Richtungslichtquelle geschrieben wird.
 */
void rendering_getDirLightDirection(const ProgContext *ctx, vec3 outDir);

/**
 * Setzt, ob Tiefenunschärfe (Depth of Field, DoF) verwendet werden soll.
 *
 * @param ctx Programmkontext.
 * @param value true, um DoF zu aktivieren, false, um es zu deaktivieren.
 */
void rendering_setUseDoF(const ProgContext *ctx, bool value);

/**
 * Setzt den Tiefenunschärfe-Wert (Depth of Field).
 *
 * @param ctx Programmkontext.
 * @param value Der neue DoF-Wert.
 */
void rendering_setDepthOfField(const ProgContext *ctx, float value);

/**
 * Setzt die Fokusdistanz der Szene.
 *
 * @param ctx Programmkontext.
 * @param value Die neue Fokusdistanz.
 */
void rendering_setFocusDistance(const ProgContext *ctx, float value);

/**
 * Gibt den Status der Tiefenunschärfe zurück.
 *
 * @param ctx Programmkontext.
 * @return true, wenn DoF aktiviert ist, sonst false.
 */
bool rendering_getUseDoF(const ProgContext *ctx);

/**
 * Gibt den aktuellen Tiefenunschärfe-Wert (Depth of Field) zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle DoF-Wert.
 */
float rendering_getDepthOfField(const ProgContext *ctx);

/**
 * Gibt die aktuelle Fokusdistanz zurück.
 *
 * @param ctx Programmkontext.
 * @return Die aktuelle Fokusdistanz.
 */
float rendering_getFocusDistance(const ProgContext *ctx);

/**
 * Gibt die aktuelle Near-Z-Koordinate der Kamera zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Z-Near-Wert.
 */
float rendering_getZNear(const ProgContext *ctx);

/**
 * Gibt die aktuelle Far-Z-Koordinate der Kamera zurück.
 *
 * @param ctx Programmkontext.
 * @return Der aktuelle Z-Far-Wert.
 */
float rendering_getZFar(const ProgContext *ctx);

/**
 * Gibt die aktuelle Quad-Größe zurück.
 *
 * @param ctx Programmkontext.
 * @return Die aktuelle Quad-Größe.
 */
float rendering_getQuadSize(const ProgContext *ctx);

/**
 * Setzt die Near-Z-Koordinate der Kamera.
 *
 * @param ctx Programmkontext.
 * @param value Der neue Z-Near-Wert.
 */
void rendering_setZNear(const ProgContext *ctx, float value);

/**
 * Setzt die Far-Z-Koordinate der Kamera.
 *
 * @param ctx Programmkontext.
 * @param value Der neue Z-Far-Wert.
 */
void rendering_setZFar(const ProgContext *ctx, float value);

/**
 * Setzt die Quad-Größe.
 *
 * @param ctx Programmkontext.
 * @param value Die neue Quad-Größe.
 */
void rendering_setQuadSize(const ProgContext *ctx, float value);

#endif // RENDERING_H

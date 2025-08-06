/**
 * Graphisches Nutzerinterface für die Software.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#include "gui.h"

#include <string.h>
#include <sesp/nuklear.h>

#include "window.h"
#include "input.h"
#include "rendering.h"

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

#define STATS_WIDTH (80)
#define STATS_HEIGHT (30)

#define MIN_VAL (-1000.0f)
#define MAX_VAL (1000.0f)

// Definitionen der Fenster IDs
#define GUI_WINDOW_HELP "window_help"
#define GUI_WINDOW_MENU "window_menu"
#define GUI_WINDOW_STATS "window_stats"

static const char *renderModeNames[RENDER_MODE_COUNT] = {
    "Phong",
    "Debug"
};

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datentyp für die GUI Daten
struct GuiData {
    struct nk_glfw glfw;
    struct nk_context *nk;
};

typedef struct GuiData GuiData;

/////////////////////////////// LOKALE CALLBACKS ///////////////////////////////

/**
 * Callback Funktion, die aufgerufen wird, wenn eine Taste gedrückt wurde.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param codepoint 32bit Zeichencode.
 */
static void callback_glfwChar(GLFWwindow *win, unsigned int codepoint) {
    ProgContext *ctx = (ProgContext *) glfwGetWindowUserPointer(win);
    nk_glfw3_char_callback(&ctx->gui->glfw, codepoint);
}

/**
 * Callback Funktion, die aufgerufen wird, wenn das Maus-Scollrad gedreht wird.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param xoff die Veränderung in X-Richtung (z.B. bei Touchpads).
 * @param yoff die Veränderung in Y-Richtung (z.B. klassisches Mausrad).
 */
static void callback_glfwScroll(GLFWwindow *win, double xoff, double yoff) {
    ProgContext *ctx = (ProgContext *) glfwGetWindowUserPointer(win);

    // Prüfen, ob aktuell die GUI aktiv ist.
    if (nk_item_is_any_active(ctx->gui->nk)) {
        // Wenn ja, Event an die GUI weiterleiten.
        nk_gflw3_scroll_callback(&ctx->gui->glfw, xoff, yoff);
    } else {
        // Wenn nicht, Event an das Input-Modul weiterleiten.
        input_scroll(ctx, xoff, yoff);
    }
}

/**
 * Callback Funktion, die aufgerufen wird, wenn eine Maustaste gedrückt wurde.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param button die gedrückte Taste.
 * @param action die Aktion, die das Event ausgelöst hat (z.B. loslassen).
 * @param mods aktivierte Modifikatoren.
 */
static void callback_glfwMouseButton(GLFWwindow *win, int button,
                                     int action, int mods) {
    ProgContext *ctx = (ProgContext *) glfwGetWindowUserPointer(win);

    // Prüfen, ob aktuell die GUI aktiv ist.
    if (nk_item_is_any_active(ctx->gui->nk)) {
        // Wenn ja, Event an die GUI weiterleiten.
        nk_glfw3_mouse_button_callback(
            &ctx->gui->glfw, win,
            button, action, mods
        );
    } else {
        // Wenn nicht, Event an das Input-Modul weiterleiten.
        input_mouseAction(ctx, button, action, mods);
    }
}

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Hilfswidget um einen Colorpicker anzuzeigen, der die Farbe
 * als vec4 speichert.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param col die einstellbare Farbe
 */
static bool gui_widgetColor(struct nk_context *nk, const char *name, vec4 col) {
    // Convert vec4 to nk_colorf
    struct nk_colorf nkColor = {col[0], col[1], col[2], col[3]};
    struct nk_colorf originalColor = nkColor; // Keep a copy of the original color

    // Display title
    nk_layout_row_dynamic(nk, 20, 1);
    nk_label(nk, name, NK_TEXT_LEFT);

    // Display color picker
    nk_layout_row_dynamic(nk, 25, 1);
    if (nk_combo_begin_color(nk, nk_rgb_cf(nkColor), nk_vec2(nk_widget_width(nk), 400))) {
        nk_layout_row_dynamic(nk, 120, 1);
        nkColor = nk_color_picker(nk, nkColor, NK_RGBA);
        nk_layout_row_dynamic(nk, 25, 1);
        nkColor.r = nk_propertyf(nk, "#R:", 0, nkColor.r, 1.0f, 0.01f, 0.005f);
        nkColor.g = nk_propertyf(nk, "#G:", 0, nkColor.g, 1.0f, 0.01f, 0.005f);
        nkColor.b = nk_propertyf(nk, "#B:", 0, nkColor.b, 1.0f, 0.01f, 0.005f);
        nkColor.a = nk_propertyf(nk, "#A:", 0, nkColor.a, 1.0f, 0.01f, 0.005f);
        nk_combo_end(nk);
    }

    // Convert back to vec4
    col[0] = nkColor.r;
    col[1] = nkColor.g;
    col[2] = nkColor.b;
    col[3] = nkColor.a;

    // Check if any component has changed
    return (nkColor.r != originalColor.r || nkColor.g != originalColor.g ||
            nkColor.b != originalColor.b || nkColor.a != originalColor.a);
}

/**
 * Hilfswidget um einen 3D Vektor anzupassen.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param val der einstellbare Vektor
 */
static bool gui_widgetVec3(struct nk_context *nk, const char *name, vec3 val) {
    // Keep a copy of the original vector
    vec3 originalVal = {val[0], val[1], val[2]};

    // Display title
    nk_layout_row_dynamic(nk, 20, 1);
    nk_label(nk, name, NK_TEXT_LEFT);

    // Display input fields for X, Y, Z
    nk_layout_row_dynamic(nk, 25, 3);
    nk_property_float(nk, "#X", MIN_VAL, &val[0], MAX_VAL, 1.0f, 0.1f);
    nk_property_float(nk, "#Y", MIN_VAL, &val[1], MAX_VAL, 1.0f, 0.1f);
    nk_property_float(nk, "#Z", MIN_VAL, &val[2], MAX_VAL, 1.0f, 0.1f);

    // Check if any component has changed
    return (val[0] != originalVal[0] || val[1] != originalVal[1] || val[2] != originalVal[2]);
}

/**
 * Hilfsfunktion um ein float Label anzuzeigen.
 *
 * @param nk der Nuklear Kontext
 * @param value der anzuzeigende Wert
 * @param precision die anzuzeigenden Nachkommastellen
 */
static void gui_display_float(struct  nk_context *nk, float value, int precision) {
    char value_str[32];
    snprintf(value_str, sizeof(value_str), "%.*f", precision, value);
    nk_label(nk, value_str, NK_TEXT_LEFT);
}

/**
 * Zeigt ein Hilfefenster an, in dem alle Maus- und Tastaturbefehle aufgelistet
 * werden.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderHelp(ProgContext *ctx, struct nk_context *nk) {
    // Prüfen, ob die Hilfe überhaupt angezeigt werden soll.
    if (ctx->input->showHelp) {
        // Größe und Position des Fensters beim Öffnen bestimmen.
        float width = ctx->winData->realWidth * 0.25f;
        float height = ctx->winData->realHeight * 0.5f;
        float x = width * 1.5f;
        float y = height * 0.5f;

        // Fenster öffnen.
        if (nk_begin_titled(nk, GUI_WINDOW_HELP,
                            "Hilfe", nk_rect(x, y, width, height),
                            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                            NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            // Vorlage für die Darstellung der Zeilen anlegen.
            nk_layout_row_template_begin(nk, 15);
            nk_layout_row_template_push_dynamic(nk);
            nk_layout_row_template_push_static(nk, 40);
            nk_layout_row_template_end(nk);

            // Hilfsmakro zum einfachen füllen des Fensters.
#define HELP_LINE(dsc, key) { \
                nk_label(nk, (dsc), NK_TEXT_ALIGN_LEFT);\
                nk_label(nk, (key), NK_TEXT_ALIGN_RIGHT);}

            HELP_LINE("Programm beenden", "ESC");
            HELP_LINE("Hilfe umschalten", "F1");
            HELP_LINE("Fullscreen umschalten", "F2");
            HELP_LINE("Wireframe umschalten", "F3");
            HELP_LINE("Menü umschalten", "F4");
            HELP_LINE("Statistiken umschalten", "F5");
            HELP_LINE("Screenshot anfertigen", "F6");
            HELP_LINE("Kamera vorwärst", "W");
            HELP_LINE("Kamera links", "A");
            HELP_LINE("Kamera zurück", "S");
            HELP_LINE("Kamera rechts", "D");
            HELP_LINE("Kamera hoch", "E");
            HELP_LINE("Kamera runter", "Q");
            HELP_LINE("Umsehen", "LMB");
            HELP_LINE("Zoomen", "Scroll");

            // Makro wieder löschen, da es nicht mehr gebraucht wird.
#undef HELP_LINE

            // Vorlage für den Schließen-Button.
            nk_layout_row_template_begin(nk, 25);
            nk_layout_row_template_push_dynamic(nk);
            nk_layout_row_template_push_static(nk, 130);
            nk_layout_row_template_end(nk);

            // Ein leeres Textfeld zum Ausrichten des Buttons.
            nk_label(nk, "", NK_TEXT_ALIGN_LEFT);

            // Button zum Schließen des Fensters.
            if (nk_button_label(nk, "Hilfe schließen")) {
                ctx->input->showHelp = false;
            }
        }
        nk_end(nk);
    }
}

/**
 * Zeigt ein Konfigurationsmenü an, das benutzt werden kann, um die Szene oder
 * das Rendering an sich zu beeinflussen.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderMenu(ProgContext *ctx, struct nk_context *nk) {
    InputData *input = ctx->input;

    // Prüfen, ob das Menü überhaupt angezeigt werden soll.
    if (input->showMenu) {
        // Größe des Fensters beim Öffnen bestimmen.
        float height = ctx->winData->realHeight * 0.7f;

        // Fenster öffnen.
        if (nk_begin_titled(nk, GUI_WINDOW_MENU, "Szenen-Einstellungen",
                            nk_rect(15, 15, 350, height),
                            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                            NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            // Allgemeine Einstellungen anzeigen.
            if (nk_tree_push(nk, NK_TREE_TAB, "Allgemein", NK_MAXIMIZED)) {
                nk_layout_row_dynamic(nk, 30, 2);

                // Hilfe umschalten
                if (nk_button_label(nk, "Hilfe umschalten")) {
                    input->showHelp = !input->showHelp;
                }

                // Fenstermodus/Vollbild umschalten
                if (nk_button_label(nk,
                                    input->isFullscreen ? "Fenstermodus" : "Vollbild")) {
                    input->isFullscreen = !input->isFullscreen;
                    window_updateFullscreen(ctx);
                }

                nk_layout_row_dynamic(nk, 30, 2);
                // Shader Neukompilierung
                if (nk_button_label(nk, "Shader aktual.")) {
                    if (!rendering_recompileShader(ctx)) {
                        fprintf(stderr, "Shader recompilation failed!\n");
                    }
                }

                // Beenden
                if (nk_button_label(nk, "Beenden")) {
                    glfwSetWindowShouldClose(ctx->window, true);
                }

                nk_tree_pop(nk);
            }

            // Einstellungen bezüglich der Darstellung.
            if (nk_tree_push(nk, NK_TREE_TAB, "Darstellung", NK_MINIMIZED)) {
                // Anzeigemodus
                nk_layout_row_dynamic(nk, 25, 1);
                int selectedRenderMode = rendering_getSelectedRenderMode(ctx);
                if (nk_combo_begin_label(nk, renderModeNames[selectedRenderMode], nk_vec2(nk_widget_width(nk), 200))) {
                    nk_layout_row_dynamic(nk, 25, 1);
                    for (int i = 0; i < RENDER_MODE_COUNT; ++i) {
                        if (nk_combo_item_label(nk, renderModeNames[i], NK_TEXT_LEFT)) {
                            rendering_selectRenderMode(ctx, i);
                        }
                    }
                    nk_combo_end(nk);
                }

                if (nk_tree_push(nk, NK_TREE_TAB, "Allgemein", NK_MAXIMIZED)) {
                    // Wireframe
                    nk_bool wireframe = input->showWireframe;
                    if (nk_checkbox_label(nk, "Wireframe", &wireframe)) {
                        input->showWireframe = wireframe;
                    }

                    // Skybox
                    nk_bool skybox = rendering_getSkyboxEnabled(ctx);
                    if (nk_checkbox_label(nk, "Skybox", &skybox)) {
                        if (skybox) {
                            rendering_enableSkybox(ctx);
                            rendering_disableFog(ctx);
                        } else {
                            rendering_disableSkybox(ctx);
                        }
                    }

                    // Nebel-Einstellungen (Checkbox und Slider nebeneinander anzeigen)
                    nk_layout_row_dynamic(nk, 25, 3); // Layout für zwei Spalten: Checkbox und Slider
                    nk_bool fog = rendering_getFogEnabled(ctx);
                    if (nk_checkbox_label(nk, "Nebel", &fog)) {
                        if (fog) {
                            rendering_enableFog(ctx);
                            rendering_disableSkybox(ctx);
                        } else {
                            rendering_disableFog(ctx);
                        }
                    }

                    // Nebel-Dichte-Slider (nur anzeigen, wenn der Nebel aktiviert ist)
                    if (fog) // Zeige den Slider nur, wenn der Nebel aktiviert ist
                    {
                        float fogDensity = rendering_getFogDensity(ctx);
                        if (nk_slider_float(nk, 0.0f, &fogDensity, 0.2f, 0.001f)) {
                            rendering_setFogDensity(ctx, fogDensity);
                        }
                        gui_display_float(nk, fogDensity, 3);

                        vec4 fogColor;
                        rendering_getFogColor(ctx, fogColor);
                        gui_widgetColor(nk, "Nebel Farbe", fogColor);
                        rendering_setFogColor(ctx, fogColor);

                        nk_layout_row_static(nk, 10, 150, 1);
                    }

                    // Alpha Clipping
                    nk_layout_row_dynamic(nk, 25, 3);
                    nk_label(nk, "Alpha Clipping", NK_TEXT_LEFT);
                    float clipping = rendering_getAlphaClipping(ctx);
                    if (nk_slider_float(nk, 0.0f, &clipping, 1.001f, 0.005f)) {
                        rendering_setAlphaClipping(ctx, clipping);
                    }
                    gui_display_float(nk, clipping, 2);

                    gui_widgetColor(nk, "Clear Color", ctx->input->rendering.clearColor);
                    nk_layout_row_static(nk, 10, 150, 1);

                    nk_tree_pop(nk);
                }

                if (nk_tree_push(nk, NK_TREE_TAB, "Normal Mapping", NK_MAXIMIZED)) {
                    nk_bool normalMapping = rendering_getNormalMappingEnabled(ctx);
                    if (nk_checkbox_label(nk, "Aktivieren", &normalMapping)) {
                        if (normalMapping) {
                            rendering_enableNormalMapping(ctx);
                        } else {
                            rendering_disableNormalMapping(ctx);
                        }
                    }
                    nk_layout_row_static(nk, 10, 150, 1);

                    nk_bool twoChannelNormalMap = rendering_getTwoChannelNormalMapEnabled(ctx);
                    if (nk_checkbox_label(nk, "2-Kanal Normal Map", &twoChannelNormalMap)) {
                        if (twoChannelNormalMap) {
                            rendering_enableTwoChannelNormalMap(ctx);
                        } else {
                            rendering_disableTwoChannelNormalMap(ctx);
                        }
                    }
                    nk_layout_row_static(nk, 10, 150, 1);

                    nk_tree_pop(nk);
                }

                if (nk_tree_push(nk, NK_TREE_TAB, "Tessellation", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(nk, 25, 1);
                    nk_bool useTessellation = rendering_getTesselationEnabled(ctx);
                    if (nk_checkbox_label(nk, "Aktivieren", &useTessellation)) {
                        if (useTessellation) {
                            rendering_enableTesselation(ctx);
                        } else {
                            rendering_disableTesselation(ctx);
                        }
                    }

                    // Set Min and Max Tessellation levels
                    nk_layout_row_dynamic(nk, 25, 2);
                    int minTessellation = rendering_getTesselationMin(ctx);
                    int maxTessellation = rendering_getTesselationMax(ctx);

                    // Update Min Tessellation
                    nk_label(nk, "Min", NK_TEXT_LEFT);
                    int newMinTessellation = nk_propertyi(nk, "#Min", 1, minTessellation, 10, 1, 0.01f);
                    if (newMinTessellation != minTessellation) {
                        rendering_setTesselationMin(ctx, newMinTessellation);
                    }

                    // Update Max Tessellation
                    nk_label(nk, "Max", NK_TEXT_LEFT);
                    int newMaxTessellation = nk_propertyi(nk, "#Max", 1, maxTessellation, 100, 1, 0.01f);
                    if (newMaxTessellation != maxTessellation) {
                        rendering_setTesselationMax(ctx, newMaxTessellation);
                    }

                    nk_layout_row_static(nk, 10, 150, 1);

                    if (nk_tree_push(nk, NK_TREE_TAB, "Displacement", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(nk, 25, 1);
                        nk_bool useDisplacement = rendering_getDisplacementEnabled(ctx);
                        if (nk_checkbox_label(nk, "Aktivieren", &useDisplacement)) {
                            if (useDisplacement) {
                                rendering_enableDisplacement(ctx);
                            } else {
                                rendering_disableDisplacement(ctx);
                            }
                        }

                        nk_layout_row_dynamic(nk, 25, 3);
                        nk_label(nk, "Faktor", NK_TEXT_LEFT);
                        float displacementFactor = rendering_getDisplacementFactor(ctx);
                        if (nk_slider_float(nk, 0.0f, &displacementFactor, 1.0f, 0.01f)) {
                            rendering_setDisplacementFactor(ctx, displacementFactor);
                        }
                        gui_display_float(nk, displacementFactor, 2);

                        nk_layout_row_static(nk, 10, 150, 1);

                        nk_tree_pop(nk);
                    }

                    nk_tree_pop(nk);
                }

                if (nk_tree_push(nk, NK_TREE_TAB, "Licht", NK_MAXIMIZED)) {

                    nk_layout_row_dynamic(nk, 25, 2);
                    nk_bool showShadows = rendering_getShowShadows(ctx);
                    if (nk_checkbox_label(nk, "Schatten", &showShadows)) {
                        rendering_setshowShadows(ctx, showShadows);
                    }

                    nk_bool usePCF = rendering_getUsePCF(ctx);
                    if (nk_checkbox_label(nk, "PCF", &usePCF)) {
                        rendering_setUsePCF(ctx, usePCF);
                    }

                    if (nk_tree_push(nk, NK_TREE_TAB, "Richtungslicht", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(nk, 25, 1);
                        nk_bool isDirLightActive = rendering_getIsDirLightActive(ctx);
                        if (nk_checkbox_label(nk, "Aktivieren", &isDirLightActive)) {
                            rendering_flipIsDirLightActive(ctx);
                        }

                        if (isDirLightActive) {
                            nk_layout_row_dynamic(nk, 25, 1);
                            if (nk_button_label(nk, "Schatten aktual.")) {
                                rendering_setShouldUpdateDirShadows(ctx);
                            }

                            nk_layout_row_dynamic(nk, 25, 1);
                            nk_bool alwaysUpdateShadows = rendering_getAlwaysUpdateDirShadows(ctx);
                            if (nk_checkbox_label(nk, "Schatten immer aktual.", &alwaysUpdateShadows)) {
                                rendering_setAlwaysUpdateDirShadows(ctx, alwaysUpdateShadows);
                            }

                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "Distanz Faktor", NK_TEXT_LEFT);
                            float dist = rendering_getDirLightDistanceMult(ctx);
                            if (nk_slider_float(nk, 0.0f, &dist, 40.0f, 0.5f)) {
                                rendering_setDirLightDistanceMult(ctx, dist);
                            }
                            gui_display_float(nk, dist, 2);

                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "zNear", NK_TEXT_LEFT);
                            float znear = rendering_getZNear(ctx);
                            if (nk_slider_float(nk, 0.0f, &znear, 15.0f, 0.05f)) {
                                rendering_setZNear(ctx, znear);
                            }
                            gui_display_float(nk, znear, 2);

                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "zFar", NK_TEXT_LEFT);
                            float zfar = rendering_getZFar(ctx);
                            if (nk_slider_float(nk, 1.0f, &zfar, 300.0f, 1.f)) {
                                rendering_setZFar(ctx, zfar);
                            }
                            gui_display_float(nk, zfar, 2);

                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "quadSize", NK_TEXT_LEFT);
                            float quadSize = rendering_getQuadSize(ctx);
                            if (nk_slider_float(nk, 0.0f, &quadSize, 100.0f, 0.5f)) {
                                rendering_setQuadSize(ctx, quadSize);
                            }
                            gui_display_float(nk, quadSize, 2);

                            vec3 dirLightDirection;
                            rendering_getDirLightDirection(ctx, dirLightDirection);

                            if (gui_widgetVec3(nk, "Richtung", dirLightDirection)) {
                                rendering_setDirLightDirection(ctx, dirLightDirection);
                            }

                            vec4 dirLightColor;
                            rendering_getDirLightColor(ctx, dirLightColor);

                            vec4 temp;
                            glm_vec4_copy(dirLightColor, temp);

                            if (gui_widgetColor(nk, "Farbe", dirLightColor) && !glm_vec4_eqv_eps(dirLightColor, temp)) {
                                rendering_setDirLightColor(ctx, dirLightColor);
                            }
                        }

                        nk_tree_pop(nk);
                    }

                    if (nk_tree_push(nk, NK_TREE_TAB, "Punktlicht", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(nk, 25, 1);
                        nk_bool isPointLightActive = rendering_getIsPointLightActive(ctx);
                        if (nk_checkbox_label(nk, "Aktivieren", &isPointLightActive)) {
                            rendering_flipIsPointLightActive(ctx);
                        }

                        if (isPointLightActive) {
                            nk_layout_row_dynamic(nk, 25, 1);
                            if (nk_button_label(nk, "Schatten aktual.")) {
                                rendering_setShouldUpdatePointShadows(ctx);
                            }

                            nk_layout_row_dynamic(nk, 25, 1);

                            vec4 pointLightColor;
                            rendering_getPointLightColor(ctx, pointLightColor);

                            vec4 temp;
                            glm_vec4_copy(pointLightColor, temp);

                            if (gui_widgetColor(nk, "Farbe", pointLightColor) && !glm_vec4_eqv_eps(pointLightColor, temp)) {
                                rendering_setPointLightColor(ctx, pointLightColor);
                            }
                        }

                        nk_tree_pop(nk);
                    }

                    nk_layout_row_static(nk, 10, 150, 1);

                    nk_tree_pop(nk);
                }

                nk_layout_row_static(nk, 10, 150, 1);

                nk_tree_pop(nk);
            }

            if (nk_tree_push(nk, NK_TREE_TAB, "Postprocessing", NK_MAXIMIZED)) {
                nk_layout_row_dynamic(nk, 25, 3);
                nk_label(nk, "Exposure", NK_TEXT_LEFT);
                float exposure = rendering_getGammaExposure(ctx);
                if (nk_slider_float(nk, 0.0f, &exposure, 3.f, 0.1f)) {
                    rendering_setGammaExposure(ctx, exposure);
                }
                gui_display_float(nk, exposure, 2);

                nk_layout_row_dynamic(nk, 25, 3);
                nk_label(nk, "Gamma", NK_TEXT_LEFT);
                float gamma = rendering_getGamma(ctx);
                if (nk_slider_float(nk, 0.0f, &gamma, 4.f, 0.1f)) {
                    rendering_setGamma(ctx, gamma);
                }
                gui_display_float(nk, gamma, 2);

                if (nk_tree_push(nk, NK_TREE_TAB, "Bloom", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(nk, 25, 3);
                    nk_label(nk, "Threshold", NK_TEXT_LEFT);
                    float threshold = rendering_getThreshold(ctx);
                    if (nk_slider_float(nk, 0.0f, &threshold, 3.f, 0.1f)) {
                        rendering_setThreshold(ctx, threshold);
                    }
                    gui_display_float(nk, threshold, 2);

                    nk_layout_row_dynamic(nk, 25, 3);
                    nk_label(nk, "Color Weight", NK_TEXT_LEFT);
                    float colorWeight = rendering_getThresholdColorWeight(ctx);
                    if (nk_slider_float(nk, 0.0f, &colorWeight, 3.f, 0.1f)) {
                        rendering_setThresholdColorWeight(ctx, colorWeight);
                    }
                    gui_display_float(nk, colorWeight, 2);

                    nk_layout_row_dynamic(nk, 25, 3);
                    nk_label(nk, "Emission Weight", NK_TEXT_LEFT);
                    float emissionWeight = rendering_getThresholdEmissionWeight(ctx);
                    if (nk_slider_float(nk, 0.0f, &emissionWeight, 3.f, 0.1f)) {
                        rendering_setThresholdEmissionWeight(ctx, emissionWeight);
                    }
                    gui_display_float(nk, emissionWeight, 2);

                    nk_layout_row_dynamic(nk, 25, 3);
                    nk_label(nk, "Blur Iterationen", NK_TEXT_LEFT);
                    float blurIterations = (float)rendering_getBloomBlurIterations(ctx);
                    if (nk_slider_float(nk, 0.0f, &blurIterations, 20.f, 1.0f)) {
                        rendering_setBloomBlurIterations(ctx, (int)blurIterations);
                    }
                    gui_display_float(nk, blurIterations, 2);

                    if (nk_tree_push(nk, NK_TREE_TAB, "Schärfentiefe", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(nk, 25, 1);
                        nk_bool useDoF = rendering_getUseDoF(ctx);
                        if (nk_checkbox_label(nk, "Aktivieren", &useDoF)) {
                            rendering_setUseDoF(ctx, useDoF);
                        }

                        if (useDoF) {
                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "Fokus Distanz", NK_TEXT_LEFT);
                            float focusDistance = (float)rendering_getFocusDistance(ctx);
                            if (nk_slider_float(nk, 0.0f, &focusDistance, 50.f, 1.0f)) {
                                rendering_setFocusDistance(ctx, focusDistance);
                            }
                            gui_display_float(nk, focusDistance, 2);

                            nk_layout_row_dynamic(nk, 25, 3);
                            nk_label(nk, "Schärfentiefen Bereich", NK_TEXT_LEFT);
                            float depthOfField = (float)rendering_getDepthOfField(ctx);
                            if (nk_slider_float(nk, 0.0f, &depthOfField, 50.f, 1.0f)) {
                                rendering_setDepthOfField(ctx, depthOfField);
                            }
                            gui_display_float(nk, depthOfField, 2);
                        }

                        nk_tree_pop(nk);
                    }

                    nk_tree_pop(nk);
                }

                nk_tree_pop(nk);
            }

            // Allgemeine Einstellungen anzeigen.
            if (nk_tree_push(nk, NK_TREE_TAB, "Modell Ausrichtung", NK_MAXIMIZED)) {
                vec3 translation, rotation, scale;

                rendering_getTranslation(ctx, translation);
                rendering_getRotation(ctx, rotation);
                rendering_getScale(ctx, scale);

                // Translation
                if (gui_widgetVec3(nk, "Translation", translation)) {
                    rendering_setTranslation(ctx, translation);
                }

                // Rotation
                if (gui_widgetVec3(nk, "Rotation", rotation)) {
                    rendering_setRotation(ctx, rotation);
                }

                // Scale
                if (gui_widgetVec3(nk, "Skalierung", scale)) {
                    rendering_setScale(ctx, scale);
                }

                nk_tree_pop(nk);
            }
        }
        nk_end(nk);
    }
}

/**
 * Zeigt allgemeine Zustandsinformationen über das Programm an.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderStats(ProgContext *ctx, struct nk_context *nk) {
    InputData *input = ctx->input;
    WindowData *win = ctx->winData;

    // Prüfen, ob das Menü überhaupt angezeigt werden soll.
    if (input->showStats) {
        float x = (float) win->realWidth - STATS_WIDTH;

        // Fenster öffnen.
        if (nk_begin(nk, GUI_WINDOW_STATS,
                     nk_rect(x, 0, STATS_WIDTH, STATS_HEIGHT),
                     NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND |
                     NK_WINDOW_NO_INPUT)) {
            // FPS Anzeigen
            nk_layout_row_dynamic(nk, 25, 1);
            char fpsString[15];
            snprintf(fpsString, 14, "FPS: %d", win->fps);
            nk_label(nk, fpsString, NK_TEXT_LEFT);
        }
        nk_end(nk);
    }
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

void gui_init(ProgContext *ctx) {
    ctx->gui = malloc(sizeof(GuiData));
    GuiData *data = ctx->gui;

    // Der neu erzeugte Speicher muss zuerst geleert werden.
    memset(&data->glfw, 0, sizeof(struct nk_glfw));

    // Danach muss ein neuer Nuklear-GLFW3 Kontext erzeugt werden.
    data->nk = nk_glfw3_init(
        &data->glfw,
        ctx->window
    );

    // Nuklear Callbacks registrieren
    glfwSetScrollCallback(ctx->window, callback_glfwScroll);
    glfwSetCharCallback(ctx->window, callback_glfwChar);
    glfwSetMouseButtonCallback(ctx->window, callback_glfwMouseButton);

    // Als nächstes müssen wir einen neuen, leeren Font-Stash erzeugen.
    // Dadurch wird die Default-Font aktiviert.
    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&data->glfw, &atlas);
    nk_glfw3_font_stash_end(&data->glfw);
}

void gui_render(ProgContext *ctx) {
    GuiData *data = ctx->gui;

    // Wir signalisieren Nuklear, das ein neuer Frame gezeichnet wird.
    nk_glfw3_new_frame(&data->glfw);

    // Ab hier können alle unterschiedlichen GUIs aufgebaut werden
    gui_renderHelp(ctx, data->nk);
    gui_renderMenu(ctx, data->nk);
    gui_renderStats(ctx, data->nk);

    // Als letztes rendern wir die GUI
    common_pushRenderScopeSource("Nuklear GUI", GL_DEBUG_SOURCE_THIRD_PARTY);
    nk_glfw3_render(
        &data->glfw,
        NK_ANTI_ALIASING_ON,
        MAX_VERTEX_BUFFER,
        MAX_ELEMENT_BUFFER
    );
    common_popRenderScope();
}

void gui_cleanup(ProgContext *ctx) {
    nk_glfw3_shutdown(&ctx->gui->glfw);
    free(ctx->gui);
}

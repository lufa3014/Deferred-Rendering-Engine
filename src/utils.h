/**
 * Allgemeine Hilfsfunktionen, die überall nützlich sein
 * können.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <glad/glad.h>

////////////////////////////////// KONSTANTEN //////////////////////////////////

// Wenn kein Pfad zu den Ressourcen gesetzt ist, gehen wir davon aus,
// das sie in diesem Verzeichnis liegen.
#ifndef RESOURCE_PATH
    #define RESOURCE_PATH "./res/"
#endif

//////////////////////////////////// MAKROS ////////////////////////////////////

// Gibt den Pfad zu einer Ressource im Ressourcenverzeichnis zurück.
// Der Pfad muss dabei als Literal übergeben werden.
// Wenn der Pfad dynamisch zur Laufzeit ermittelt werden muss,
// sollte die Funktion utils_getResourcePath verwendet werden.
#define UTILS_CONST_RES(res) (RESOURCE_PATH res)

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Hilfsfunktion zum bestimmen eines Pfades im Ressourcenverzeichnis.
 * Der String-Parameter darf nicht mit einem '/' anfangen!
 * Zurückgegeben wird ein neuer, zusammengesetzter Pfad. Dieser muss
 * später von Hand mit free freigegeben werden.
 * Wird für path NULL verwendet, wird nur der Pfad zum Ressourcenordner
 * zurückgegeben. Auch dieser muss mit free wieder freigegeben werden.
 *
 * @param path ein Pfad im Ressourcenverzeichnis oder NULL
 * @return ein neuer String mit dem zusammengesetzten Pfad
 */
char* utils_getResourcePath(const char* path);

/**
 * Hilfsfunktion zum Einlesen einer Datei in den Arbeitsspeicher.
 * Der zurückgegebene String muss mit free wieder gelöscht werden.
 *
 * @param filename der Dateiname der einzulesenden Datei
 * @return der Inhalt der eingelesenen Datei
 */
char* utils_readFile(const char* filename);

/**
 * Prüft, ob ein Suffix am Ende eines Stringes zu finden ist.
 * Diese Funktion kann zum Beispiel genutzt werden, um Dateiendungen
 * abzufragen: utils_hasSuffix(filename, ".png");
 * Ein leeres Suffix ist nie Teil des Subjektes.
 *
 * @param subject der String, bei dem das Suffix gesucht werden soll
 * @param suffix das zu suchende Suffix
 * @return true, wenn das Suffix vorhanden ist, false wenn nicht
 */
bool utils_hasSuffix(const char* subject, const char* suffix);

/**
 * Gibt den Ordnerpfad einer Datei zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt. Auch ".." Verzeichnisse bleiben erhalten.
 *
 * Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 *
 * @param filepath der Dateipfad, aus dem der Ordner extrahiert werden soll
 * @return der Pfad des Ordners
 */
char* utils_getDirectory(const char* filepath);

/**
 * Gibt den Dateinamen in einem Pfad zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt.
 *
 * Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 *
 * @param filepath der Dateipfad, aus dem der Dateiname extrahiert werden soll
 * @return der Dateiname
 */
char* utils_getFilename(const char* filepath);

/**
 * Gibt den Größeren von zwei Integern zurück.
 * Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:
 * Das Makro MAX(a. b) ((a) > (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) > (y++) ? (x++) : (y++))
 * Demnach würde der jeweils größere Wert zweimal erhöht werden. Solche Probleme
 * können auch in anderer Weise auftreten, weshalb eine Funktion hier sicherer
 * ist.
 *
 * @param a der erste Integer.
 * @param b der zweite Integer.
 * @return der gößere Wert.
 */
int utils_maxInt(int a, int b);

/**
 * Gibt den Kleineren von zwei Integern zurück.
 * Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:
 * Das Makro MIN(a. b) ((a) < (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) < (y++) ? (x++) : (y++))
 * Demnach würde der jeweils kleinere Wert zweimal erhöht werden. Solche
 * Probleme können auch in anderer Weise auftreten, weshalb eine Funktion hier
 * sicherer ist.
 *
 * @param a der erste Integer.
 * @param b der zweite Integer.
 * @return der kleinere Wert.
 */
int utils_minInt(int a, int b);

/**
 * Erstellt einen Einheitswürfel und initialisiert ein Vertex Array Object (VAO)
 * und ein Vertex Buffer Object (VBO), die die Würfelgeometrie enthalten.
 * Diese Funktion kann für verschiedene Zwecke verwendet werden, wie z.B.
 * das Rendern einer Skybox oder eines einfachen Würfels in der Szene.
 *
 * Die erzeugten OpenGL-Objekte (VAO und VBO) müssen später manuell mit
 * glDeleteVertexArrays und glDeleteBuffers freigegeben werden.
 *
 * Zusätzlich wird die Anzahl der Vertices des Würfels berechnet und übergeben,
 * sodass dynamische Aufrufe von glDrawArrays ermöglicht werden, ohne feste Werte
 * zu verwenden.
 *
 * @param VAO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Array Object-ID gespeichert wird.
 * @param VBO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Buffer Object-ID gespeichert wird.
 * @param vertexCount Ein Zeiger auf eine GLsizei-Variable, in der die Anzahl
 *        der Vertices gespeichert wird, die für den Würfel definiert sind.
 *        Diese kann für dynamische Zeichenaufrufe verwendet werden.
 */
void utils_createCube(GLuint* VAO, GLuint* VBO, GLsizei* vertexCount);

/**
 * Erstellt ein Quad und initialisiert ein Vertex Array Object (VAO)
 * und ein Vertex Buffer Object (VBO), die die Quadgeometrie enthalten.
 * Diese Funktion kann für verschiedene Zwecke verwendet werden, wie z.B.
 * das Rendern einer 2D-Fläche in der Szene.
 *
 * Die erzeugten OpenGL-Objekte (VAO und VBO) müssen später manuell mit
 * glDeleteVertexArrays und glDeleteBuffers freigegeben werden.
 *
 * @param VAO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Array Object-ID gespeichert wird.
 * @param VBO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Buffer Object-ID gespeichert wird.
 */
void utils_createQuad(GLuint* VAO, GLuint* VBO);

/**
 * Erstellt eine Kugel und initialisiert ein Vertex Array Object (VAO),
 * ein Vertex Buffer Object (VBO) und ein Element Buffer Object (EBO),
 * die die Kugelgeometrie enthalten. Diese Funktion kann für verschiedene
 * Zwecke verwendet werden, wie z.B. das Rendern einer 3D-Kugel in der Szene.
 *
 * Die erzeugten OpenGL-Objekte (VAO, VBO und EBO) müssen später manuell mit
 * glDeleteVertexArrays, glDeleteBuffers und glDeleteBuffers freigegeben werden.
 *
 * Zusätzlich wird die Anzahl der Vertices und Indizes der Kugel berechnet und
 * übergeben, sodass dynamische Aufrufe von glDrawElements ermöglicht werden,
 * ohne feste Werte zu verwenden.
 *
 * @param VAO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Array Object-ID gespeichert wird.
 * @param VBO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Buffer Object-ID gespeichert wird.
 * @param EBO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Element Buffer Object-ID gespeichert wird.
 * @param vertexCount Ein Zeiger auf eine GLsizei-Variable, in der die Anzahl
 *        der Vertices gespeichert wird, die für die Kugel definiert sind.
 *        Diese kann für dynamische Zeichenaufrufe verwendet werden.
 * @param indexCount Ein Zeiger auf eine GLsizei-Variable, in der die Anzahl
 *        der Indizes gespeichert wird, die für die Kugel definiert sind.
 *        Diese kann für dynamische Zeichenaufrufe verwendet werden.
 * @param sectorCount Die Anzahl der Sektoren (Längengrade) der Kugel.
 * @param stackCount Die Anzahl der Stapel (Breitengrade) der Kugel.
 */
void utils_createSphere(GLuint* VAO, GLuint* VBO, GLuint* EBO, GLsizei* vertexCount, GLsizei* indexCount, int sectorCount, int stackCount);

/**
 * \brief Erstellt eine Linie und initialisiert ein Vertex Array Object (VAO)
 * und ein Vertex Buffer Object (VBO), die die Liniengeometrie enthalten.
 * Diese Funktion kann für verschiedene Zwecke verwendet werden, wie z.B.
 * das Rendern einer Achse oder einer einfachen Linie in der Szene.
 *
 * Die erzeugten OpenGL-Objekte (VAO und VBO) müssen später manuell mit
 * glDeleteVertexArrays und glDeleteBuffers freigegeben werden.
 *
 * @param VAO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Array Object-ID gespeichert wird.
 * @param VBO Ein Zeiger auf eine GLuint-Variable, in der die erzeugte
 *        Vertex Buffer Object-ID gespeichert wird.
 */
void utils_createLine(GLuint* VAO, GLuint* VBO);

#endif // UTILS_H

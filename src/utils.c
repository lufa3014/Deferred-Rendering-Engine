/**
 * Allgemeine Hilfsfunktionen, die überall nützlich sein
 * können.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann, stud105751, stud104645
 */

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <corecrt_math.h>
#include <corecrt_math_defines.h>
#include <assert.h>

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

char* utils_getResourcePath(const char* path)
{
    char* resPath;

    // Prüfen, ob ein eigener Pfad angegeben wurde.
    if (path)
    {
        // Wenn ja muss der Pfad erst zusammengesetzt werden.
        resPath = malloc(strlen(RESOURCE_PATH) + strlen(path) + 1);
        strcpy(resPath, RESOURCE_PATH);
        strcat(resPath, path);
    }
    else
    {
        // Wenn nicht reicht eine einfache Kopie.
        resPath = malloc(strlen(RESOURCE_PATH) + 1);
        strcpy(resPath, RESOURCE_PATH);
    }

    return resPath;
}

char* utils_readFile(const char* filename)
{
    // Zuerst muss die Datei geöffnet werden.
    FILE* f = fopen(filename, "rb");
    if (f == NULL)
    {
        fprintf(
            stderr,
            "Error: Could not open file \"%s\" for reading.\n",
            filename
        );
        exit(EXIT_FAILURE);
    }

    // Als nächstes wird die Größe der Datei bestimmt, in dem der Lesecursor
    // an das Ende der Datei gesetzt und danach dessen Position bestimmt wird.
    // Anschließend wird der Cursor wieder an
    // den Start der Datei zurückgesetzt.
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Hier wird der notwendige Speicherplatz für die gesammte Datei reserviert.
    char* content = (char*) malloc((size_t)filesize + 1);
    if (content == NULL)
    {
        fprintf(
            stderr,
            "Error: Could not acquire memory to read the file \"%s\".\n",
            filename
        );
        exit(EXIT_FAILURE);
    }

    // Als nächstes wird der gesammte Inhalt
    // der Datei in einem Schritt eingelesen.
    if ((long) fread(content, 1, filesize, f) != filesize)
    {
        free(content);
        fprintf(stderr, "Error: Could not read the file \"%s\".\n", filename);
        exit(EXIT_FAILURE);
    }

    // Zum Schluss wird die Datei geschlossen und an das Ende des
    // Strings ein Null-Byte geschrieben.
    fclose(f);
    content[filesize] = '\0';
    return content;
}

bool utils_hasSuffix(const char* subject, const char* suffix)
{
    // Zuerst benötigen wir die Längen der beiden Strings.
    size_t subjectLen = strlen(subject);
    size_t suffixLen = strlen(suffix);

    // Wenn das Suffix länger als das Subjekt ist, kann das Subjekt das Suffix
    // gar nicht enthalten.
    if (suffixLen > subjectLen)
    {
        return false;
    }

    // Wir prüfen außerdem, ob das Suffix mindestens ein Zeichen lang ist.
    if (suffixLen < 1)
    {
        return false;
    }

    // Wir berechnen einen Offset für das Subjekt. Dieser Offset markiert den
    // Anfang des Teilstring der am Ende des Subjektes steht und so lang wie
    // das Suffix ist.
    size_t offset = subjectLen - suffixLen;

    // Als letztes führen wir einen Vergleich durch, um zu prüfen, ob das
    // Suffix wirklich am Ende des Subjekts steht. Die Addition subject + offset
    // sorgt dafür, dass nur das Ende vom Subjekt in korrekter Länge verglichen
    // wird.
    return strcmp(subject + offset, suffix) == 0;
}

char* utils_getDirectory(const char* filepath)
{
    // Windows und Unix verwenden unterschiedliche Trennzeichen.
    #ifdef _WIN32
    char* pathSep = "\\/";
    #define PATH_SEP_COUNT 2
    #else
    char* pathSep = "/";
    #define PATH_SEP_COUNT 1
    #endif

    // Zuerst suchen wir das letzte Trennzeichen.
    int i = 0;
    char* lastSlash;
    do {
        lastSlash = strrchr(filepath, pathSep[i]);
        i++;
    } while (!lastSlash && i < PATH_SEP_COUNT);

    // Prüfen, ob überhaupt ein Trennzeichen gefunden wurde.
    if (!lastSlash)
    {
        // Wenn nicht geben wir einfach einen leeren String zurück.
        char* emptyPath = malloc(1);
        emptyPath[0] = '\0';
        return emptyPath;
    }

    // Als nächstes berechnen wir die Größe des neuen Buffers.
    // Dabei wird beachtet, dass ein 0 Byte am Ende folgen muss.
    size_t dirSize = (lastSlash - filepath) + 2;

    // Dann wird der nötige Speicher angelegt und die Daten kopiert.
    char* dirPath = malloc(dirSize);
    strncpy(dirPath, filepath, dirSize - 1);

    // Zum Schluss muss das letzte Byte noch auf 0 gesetzte werden, da dies
    // nicht automatisch beim Kopieren passiert.
    dirPath[dirSize - 1] = '\0';

    return dirPath;
}

char* utils_getFilename(const char* filepath)
{
    // Windows und Unix verwenden unterschiedliche Trennzeichen.
    #ifdef _WIN32
    char* pathSep = "\\/";
    #define PATH_SEP_COUNT 2
    #else
    char* pathSep = "/";
    #define PATH_SEP_COUNT 1
    #endif

    // Zuerst suchen wir das letzte Trennzeichen.
    int i = 0;
    char* lastSlash;
    do {
        lastSlash = strrchr(filepath, pathSep[i]);
        i++;
    } while (!lastSlash && i < PATH_SEP_COUNT);

    // Und bestimmen die Länge des Originals.
    size_t filepathLen = strlen(filepath);

    // Prüfen, ob überhaupt ein Trennzeichen gefunden wurde.
    if (!lastSlash)
    {
        // Wenn nicht geben wir einfach eine Kopie des originalen Strings zurück.
        char* filenameCopy = malloc(filepathLen + 1);
        strncpy(filenameCopy, filenameCopy, filepathLen);
        filenameCopy[filepathLen] = '\0';
        return filenameCopy;
    }

    // Als nächstes berechnen wir die Größe des neuen Buffers.
    // Dabei wird beachtet, dass ein 0 Byte am Ende folgen muss.
    size_t filenameSize = (filepathLen - ((lastSlash + 1) - filepath)) + 1;

    // Dann wird der nötige Speicher angelegt und die Daten kopiert.
    char* filename = malloc(filenameSize);
    strncpy(filename, lastSlash + 1, filenameSize - 1);

    // Zum Schluss muss das letzte Byte noch auf 0 gesetzte werden, da dies
    // nicht automatisch beim Kopieren passiert.
    filename[filenameSize - 1] = '\0';

    return filename;
}

int utils_maxInt(int a, int b)
{
    return a > b ? a : b;
}

int utils_minInt(int a, int b)
{
    return a < b ? a : b;
}

void utils_createCube(GLuint* VAO, GLuint* VBO, GLsizei* vertexCount)
{
    static const float cubeVertices[] = {
        // Vorderseite
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

        // Rückseite
        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

        // Linke Seite
        -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,

        // Rechte Seite
         1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,

        // Oben
        -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,

        // Unten
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f
    };

    *vertexCount = (GLsizei)(sizeof(cubeVertices) / (3 * sizeof(float)));

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void utils_createQuad(GLuint* VAO, GLuint* VBO) {
    static const float quadVertices[] = {
            // Positions   TexCoords
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void utils_createSphere(GLuint* VAO, GLuint* VBO, GLuint* EBO, GLsizei* vertexCount, GLsizei* indexCount, int sectorCount, int stackCount)
{
    float radius = 1.0f;
    const int floatsPerVertex = 3;
    const int totalVertices = (stackCount + 1) * (sectorCount + 1);

    // Allocate memory for vertices
    float* vertices = (float*)malloc(totalVertices * floatsPerVertex * sizeof(float));
    if (!vertices) {
        fprintf(stderr, "Error: Could not allocate memory for sphere vertices.\n");
        exit(EXIT_FAILURE);
    }

    float sectorStep = (float)(2 * M_PI / sectorCount);
    float stackStep = (float)(M_PI / stackCount);

    // Generate vertex positions
    int index = 0;
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = (float)(M_PI / 2) - i * stackStep; // from +pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            vertices[index++] = x;
            vertices[index++] = y;
            vertices[index++] = z;
        }
    }

    *vertexCount = totalVertices;

    // Generate indices for the triangles
    *indexCount = stackCount * sectorCount * 6;
    GLuint* indices = (GLuint*)malloc((*indexCount) * sizeof(GLuint));
    if (!indices) {
        fprintf(stderr, "Error: Could not allocate memory for sphere indices.\n");
        free(vertices);
        exit(EXIT_FAILURE);
    }

    int idx = 0;
    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = i * (sectorCount + 1) + j;
            int second = first + sectorCount + 1;

            indices[idx++] = first;
            indices[idx++] = second;
            indices[idx++] = first + 1;

            indices[idx++] = second;
            indices[idx++] = second + 1;
            indices[idx++] = first + 1;
        }
    }

    // OpenGL buffer setup
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(totalVertices * floatsPerVertex * sizeof(float)), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*indexCount)*sizeof(GLuint), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    free(vertices);
    free(indices);
}

void utils_createLine(GLuint* VAO, GLuint* VBO)
{
    // We will update the line vertices dynamically each frame, so we initialize the buffer with NULL data
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW); // Two vertices, 3 components each

    // Set up vertex attributes
    glEnableVertexAttribArray(0); // Location 0 in the shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Unbind VAO and VBO to prevent unintended modifications
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

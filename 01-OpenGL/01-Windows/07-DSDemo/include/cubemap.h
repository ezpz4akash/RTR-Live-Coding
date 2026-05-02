#pragma once
#include "common.h"
#include "image.h"

extern FILE* gpFile;

typedef struct tagTEXTURECUBEMAP{
    CHAR cubeMapName[256];
    GLuint texID;
}TEXTURECUBEMAP, *PTEXTURECUBEMAP;

void InitializeCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap);
void FetchCubeMapTextures(const CHAR *folderName, PTEXTURECUBEMAP ptTexCubeMap);
void DrawSkyboxCubeMap(PTEXTURECUBEMAP ptTexCubeMap, GLfloat cubeMapSize);
void PrintCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap);
void DestroyCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap);
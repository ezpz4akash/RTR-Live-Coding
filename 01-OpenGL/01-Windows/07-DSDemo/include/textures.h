#pragma once
#include "common.h"

extern FILE* gpFile;

typedef struct tagTEXTURE2D{
    CHAR fileName[256];
    GLuint texID;
    GLint width;
    GLint height;
}TEXTURE2D, *PTEXTURE2D;

typedef struct tagTEXTURESLIST{
    PTEXTURE2D textures;
    GLuint noOfTextures;
}TEXTURES2DLIST, *PTEXTURES2DLIST;

void Initialize2DTextures(PTEXTURES2DLIST ptTextureList);
void Fetch2DTexturesRecursively(const TCHAR *rootDir, PTEXTURES2DLIST ptTextureList);
GLint FindExisting2DTexture(PCHAR texName, PTEXTURES2DLIST ptTextureList, GLuint* pTexID);
void Print2DTextures(PTEXTURES2DLIST ptTextureList);
void Destroy2DTextures(PTEXTURES2DLIST ptTextureList);

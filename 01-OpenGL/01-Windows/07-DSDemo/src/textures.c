#include "common.h"
#include "textures.h"
#include "image.h"

void Initialize2DTextures(PTEXTURES2DLIST ptTextureList){
    ptTextureList->noOfTextures = 0;
    ptTextureList->textures = NULL;
}

void Fetch2DTexturesRecursively(const TCHAR *rootDir, PTEXTURES2DLIST ptTextureList) {
    TCHAR searchPath[MAX_PATH];
    WIN32_FIND_DATA findData;

    _stprintf(searchPath, TEXT("%s/*"), rootDir);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if(hFind == INVALID_HANDLE_VALUE) 
        return;

    do{
        if(_tcscmp(findData.cFileName, TEXT(".")) == 0 || _tcscmp(findData.cFileName, TEXT("..")) == 0)
            continue;

        TCHAR fullPath[MAX_PATH];
        _stprintf(fullPath, TEXT("%s/%s"), rootDir, findData.cFileName);

        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
            Fetch2DTexturesRecursively(fullPath, ptTextureList);
        }
        else{
            int width, height, channels;
            unsigned char* imageData = GetImageData(fullPath, &width, &height, &channels);
            if (imageData) {
                ptTextureList->noOfTextures = ptTextureList->noOfTextures + 1;
                ptTextureList->textures = (PTEXTURE2D)realloc(ptTextureList->textures, ptTextureList->noOfTextures * sizeof(TEXTURE2D));
                
                strcpy_s(ptTextureList->textures[ptTextureList->noOfTextures - 1].fileName, (strlen(findData.cFileName) + 1) * sizeof(CHAR), findData.cFileName);
                ptTextureList->textures[ptTextureList->noOfTextures - 1].width = width;
                ptTextureList->textures[ptTextureList->noOfTextures - 1].height = height;

                glGenTextures(1, &(ptTextureList->textures[ptTextureList->noOfTextures - 1].texID));
                glBindTexture(GL_TEXTURE_2D, ptTextureList->textures[ptTextureList->noOfTextures - 1].texID);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                glBindTexture(GL_TEXTURE_2D, 0);

                FreeImageData(imageData);
                fprintf(gpFile, TEXT("Loading Texture %d -> File: %s\n"), ptTextureList->noOfTextures, ptTextureList->textures[ptTextureList->noOfTextures - 1].fileName);
            }
            else{
                fprintf(gpFile, TEXT("Loading Texture Failed For File: %s\n"), ptTextureList->textures[ptTextureList->noOfTextures - 1].fileName);
            }
        }

    }
    while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

void Print2DTextures(PTEXTURES2DLIST ptTextureList){
    fprintf(gpFile, "\n**** TEXTURES DETAILS ****\n");
    fprintf(gpFile, "No. of textures loaded : %d\n\n", ptTextureList->noOfTextures);
    fprintf(gpFile, "Following are the details of loaded textures: \n");
    for(GLuint i = 0; i < ptTextureList->noOfTextures; i++){
        fprintf(gpFile, "Loaded Texture File : %s\n", ptTextureList->textures[i].fileName);
        fprintf(gpFile, "Loaded Texture File Image Width : %d\n", ptTextureList->textures[i].width);
        fprintf(gpFile, "Loaded Texture File Image Height : %d\n", ptTextureList->textures[i].height);
        fprintf(gpFile, "Loaded Texture ID : %d\n\n", ptTextureList->textures[i].texID);
    }
    fprintf(gpFile, "**** *************** ****\n\n");
}

GLint FindExisting2DTexture(PCHAR texName, PTEXTURES2DLIST ptTextureList, GLuint* pTexID){
    if(ptTextureList->noOfTextures > 0){
        for(GLuint i = 0; i < ptTextureList->noOfTextures; i++){
            if(strcmp(texName, ptTextureList->textures[i].fileName) == 0){
                (*pTexID) = ptTextureList->textures[i].texID;
                return 1;
            }
        }
        fprintf(gpFile, "No Textures With Such Name Exists : %s\n", texName);
    }
    else{
        fprintf(gpFile, "No Textures Loaded In The Global Store\n");
    }
    return -1;
}

void Destroy2DTextures(PTEXTURES2DLIST ptTextureList){
    fprintf(gpFile, "\n**** Deleting Textures While Exiting Started****\n");
    if(ptTextureList){
        for(GLuint i = 0; i < ptTextureList->noOfTextures; i++){
            fprintf(gpFile, "Deleting Texture : %s\n", ptTextureList->textures[i].fileName);
            glDeleteTextures(1, &(ptTextureList->textures[i].texID));
        }
        free(ptTextureList->textures);
        ptTextureList->textures = NULL;
    }
    fprintf(gpFile, "**** Deleting Textures While Exiting Completed ****\n\n");
}
#include "cubemap.h"
#define NO_OF_CUBE_MAP_FACES 6

PCHAR cubeMapFacesName[NO_OF_CUBE_MAP_FACES] = {
    "posx.jpg",
    "negx.jpg",
    "posy.jpg",
    "negy.jpg",
    "posz.jpg",
    "negz.jpg"
};

GLenum cubeMapFacesTextureTarget[NO_OF_CUBE_MAP_FACES] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

void InitializeCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap){
    const PCHAR cubeMapName = "INVALID CUBEMAP";
    strcpy_s(ptTexCubeMap->cubeMapName, (strlen(cubeMapName) + 1) * sizeof(CHAR), cubeMapName);
    ptTexCubeMap->texID = 0;
}

void FetchCubeMapTextures(const CHAR *folderName, PTEXTURECUBEMAP ptTexCubeMap){
    int width, height, nrChannels;

    TCHAR searchPath[MAX_PATH];
    WIN32_FIND_DATA findData;

    glGenTextures(1, &(ptTexCubeMap->texID));
    glBindTexture(GL_TEXTURE_CUBE_MAP, ptTexCubeMap->texID);

    strcpy_s(ptTexCubeMap->cubeMapName, (strlen(folderName) + 1) * sizeof(CHAR), folderName);

    _stprintf(searchPath, TEXT("%s/*"), folderName);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if(hFind == INVALID_HANDLE_VALUE) 
        return;

    do{
        if(_tcscmp(findData.cFileName, TEXT(".")) == 0 || _tcscmp(findData.cFileName, TEXT("..")) == 0)
            continue;

        TCHAR fullPath[MAX_PATH];
        _stprintf(fullPath, TEXT("%s/%s"), folderName, findData.cFileName);

        for(GLuint i = 0; i < NO_OF_CUBE_MAP_FACES; i++){
            if(strcmp(findData.cFileName, cubeMapFacesName[i]) == 0){
                PUCHAR imageData = GetImageData(fullPath, &width, &height, &nrChannels);
                if (imageData) {
                    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
                    glTexImage2D(cubeMapFacesTextureTarget[i], 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                    fprintf(gpFile, TEXT("Loading Cube Map Texture %d -> File: %s\n"), i, cubeMapFacesName[i]);
                    FreeImageData(imageData);
                } else {
                    fprintf(gpFile, "Failed to load %s\n", cubeMapFacesName[i]);
                }
                break;
            }
        }
    }while (FindNextFile(hFind, &findData));

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void DrawSkyboxCubeMap(PTEXTURECUBEMAP ptTexCubeMap, GLfloat cubeMapSize){
    glPushMatrix();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_CUBE_MAP);

        glBindTexture(GL_TEXTURE_CUBE_MAP, ptTexCubeMap->texID);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        /* 
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R); 
        */

        /* 
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP); 
        */

        
        glBegin(GL_QUADS);
        {
            // posx
            glTexCoord3f(1.0, -1.0, -1.0); glVertex3f(cubeMapSize, -cubeMapSize, -cubeMapSize);
            glTexCoord3f(1.0, -1.0,  1.0); glVertex3f(cubeMapSize, -cubeMapSize,  cubeMapSize);
            glTexCoord3f(1.0,  1.0,  1.0); glVertex3f(cubeMapSize,  cubeMapSize,  cubeMapSize);
            glTexCoord3f(1.0,  1.0, -1.0); glVertex3f(cubeMapSize,  cubeMapSize, -cubeMapSize);

            // negx
            glTexCoord3f(-1.0, -1.0,  1.0); glVertex3f(-cubeMapSize, -cubeMapSize,  cubeMapSize);
            glTexCoord3f(-1.0, -1.0, -1.0); glVertex3f(-cubeMapSize, -cubeMapSize, -cubeMapSize);
            glTexCoord3f(-1.0,  1.0, -1.0); glVertex3f(-cubeMapSize,  cubeMapSize, -cubeMapSize);
            glTexCoord3f(-1.0,  1.0,  1.0); glVertex3f(-cubeMapSize,  cubeMapSize,  cubeMapSize);

            // posy
            glTexCoord3f(-1.0, 1.0, -1.0); glVertex3f(-cubeMapSize, cubeMapSize, -cubeMapSize);
            glTexCoord3f(1.0, 1.0, -1.0); glVertex3f( cubeMapSize, cubeMapSize, -cubeMapSize);
            glTexCoord3f(1.0, 1.0, 1.0);  glVertex3f( cubeMapSize, cubeMapSize,  cubeMapSize);
            glTexCoord3f(-1.0, 1.0, 1.0); glVertex3f(-cubeMapSize, cubeMapSize,  cubeMapSize);

            // negy
            glTexCoord3f(-1.0, -1.0,  1.0); glVertex3f(-cubeMapSize, -cubeMapSize,  cubeMapSize);
            glTexCoord3f(1.0, -1.0,  1.0);  glVertex3f( cubeMapSize, -cubeMapSize,  cubeMapSize);
            glTexCoord3f(1.0, -1.0, -1.0);  glVertex3f( cubeMapSize, -cubeMapSize, -cubeMapSize);
            glTexCoord3f(-1.0, -1.0, -1.0); glVertex3f(-cubeMapSize, -cubeMapSize, -cubeMapSize);

            // posz
            glTexCoord3f(-1.0, -1.0, 1.0); glVertex3f(-cubeMapSize, -cubeMapSize, cubeMapSize);
            glTexCoord3f(-1.0,  1.0, 1.0); glVertex3f(-cubeMapSize,  cubeMapSize, cubeMapSize);
            glTexCoord3f(1.0,  1.0, 1.0);  glVertex3f( cubeMapSize,  cubeMapSize, cubeMapSize);
            glTexCoord3f(1.0, -1.0, 1.0);  glVertex3f( cubeMapSize, -cubeMapSize, cubeMapSize);

            // negz
            glTexCoord3f(1.0, -1.0, -1.0); glVertex3f( cubeMapSize, -cubeMapSize, -cubeMapSize);
            glTexCoord3f(1.0,  1.0, -1.0); glVertex3f( cubeMapSize,  cubeMapSize, -cubeMapSize);
            glTexCoord3f(-1.0,  1.0, -1.0);glVertex3f(-cubeMapSize,  cubeMapSize, -cubeMapSize);
            glTexCoord3f(-1.0, -1.0, -1.0);glVertex3f(-cubeMapSize, -cubeMapSize, -cubeMapSize);
        }
        glEnd();
        

        /* 
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R); 
        */

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDisable(GL_TEXTURE_CUBE_MAP);
        glDepthMask(GL_TRUE);
    glPopMatrix();
}

void PrintCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap){

}

void DestroyCubeMapTextures(PTEXTURECUBEMAP ptTexCubeMap){
    fprintf(gpFile, "\n**** Deleting CubeMap Texture Started****\n");
    fprintf(gpFile, "Deleting CubeMap Texture : %s\n", ptTexCubeMap->cubeMapName);
    if(ptTexCubeMap){
        glDeleteTextures(1, &(ptTexCubeMap->texID));
        ptTexCubeMap->texID= 0;
    }
    fprintf(gpFile, "**** Deleting CubeMap Texture Completed ****\n\n");
}
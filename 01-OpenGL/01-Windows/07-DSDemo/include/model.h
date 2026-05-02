#pragma once
#include "common.h"

extern FILE* gpFile;
typedef struct tagMESHDATA{
    CHAR meshName[256];

    GLuint vboVertices;
    GLuint vboNormals;
    GLuint vboTexCoords;
    GLuint vboColors;
    GLuint vboIndices;
    GLsizei indexCount;

    int hasNormals;
    int hasTexCoords;
    int hasColors;
}MESHDATA, *PMESHDATA;

typedef struct tagMESHES{
    PMESHDATA meshesData;
    GLuint noOfMeshes;
}MESHES, *PMESHES;

typedef struct tagMODEL{
    const struct aiScene* scene;
    CHAR modelName[256];
    MESHES modelMeshVBOData;
}MODEL, *PMODEL;

void InitAssimp();
PMODEL LoadModel(PCHAR modelPath);

void InitializeMeshes(PMODEL pModel);
void FetchMeshes(PMODEL pModel);

void PrintMeshes(PMODEL pModel);
void ApplyMaterialAndTexture(const struct aiMaterial *mtl);
void RecursiveRender(const struct aiScene *scene, const struct aiNode* node, PMESHES ptMeshes);
void RenderModel(PMODEL pModel);

void CleanMeshes(PMODEL pModel);
void UnInitializeMeshes(PMODEL pModel);

void UnLoadModel(PMODEL pModel);
void UnInitAssimp();

/* Helper functions */
void color4_to_float4(const C_STRUCT aiColor4D *c, float f[4]);
void set_float4(float f[4], float a, float b, float c, float d);

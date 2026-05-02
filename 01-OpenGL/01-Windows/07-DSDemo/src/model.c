#include "model.h"
#include "render.h"
#include "textures.h"

extern TEXTURES2DLIST all2DTextures;

void InitAssimp(){
	struct aiLogStream stream;
    stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);

	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);
}

PMODEL LoadModel(PCHAR modelPath){
	PMODEL pModel = NULL;

    fprintf(gpFile, "Loading Model Started: %s\n", modelPath);
    const char* extension = strrchr(modelPath, '.');
	if (!extension) {
		fprintf(gpFile, "Please provide a file with a valid extension.");
		return NULL;
	}

	if (AI_FALSE == aiIsExtensionSupported(extension)) {
		fprintf(gpFile, "The specified model file extension is currently unsupported in Assimp");
		return NULL;
	}

	pModel = (PMODEL)malloc(sizeof(MODEL));
	strcpy_s(pModel->modelName, (strlen(modelPath) + 1) * sizeof(CHAR), modelPath);
	pModel->scene = aiImportFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (!(pModel->scene)) {
		fprintf(gpFile, "Failed to load model: %s\n", aiGetErrorString());
		exit(1);
    }

	InitializeMeshes(pModel);
	FetchMeshes(pModel);
	PrintMeshes(pModel);

	fprintf(gpFile, "Loading Model Finished: %s\n", pModel->modelName);
    return pModel;
}

void InitializeMeshes(PMODEL pModel){
	PMESHES ptMeshes = &(pModel->modelMeshVBOData);
	ptMeshes->meshesData = NULL;
	ptMeshes->noOfMeshes = 0;
}

void FetchMeshes(PMODEL pModel){
	PMESHES ptMeshes = &(pModel->modelMeshVBOData);
	for (GLuint i = 0; i < pModel->scene->mNumMeshes; i++) {
		const struct aiMesh* mesh = pModel->scene->mMeshes[i];

		fprintf(gpFile, "===========================\n");
		fprintf(gpFile, "Fetching Data For Mesh : %s\n", mesh->mName.data);

		ptMeshes->noOfMeshes = ptMeshes->noOfMeshes + 1;
		ptMeshes->meshesData = (PMESHDATA)realloc(ptMeshes->meshesData, ptMeshes->noOfMeshes * sizeof(MESHDATA));

		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboVertices = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboNormals = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboTexCoords = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboColors = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboIndices = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasColors = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasNormals = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasTexCoords = 0;
		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].indexCount = 0;

		strcpy_s((ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].meshName), (strlen(mesh->mName.data) + 1) * sizeof(CHAR), mesh->mName.data);

		glGenBuffers(1, &(ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboVertices));
		glBindBuffer(GL_ARRAY_BUFFER, (ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboVertices));
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct aiVector3D) * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);

		if(mesh->mNormals != NULL){
			ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasNormals = TRUE;
			glGenBuffers(1, &(ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboNormals));
			glBindBuffer(GL_ARRAY_BUFFER, (ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboNormals));
			glBufferData(GL_ARRAY_BUFFER, sizeof(struct aiVector3D) * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
		}
		
		if(mesh->mTextureCoords[0] != NULL){
			ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasTexCoords = TRUE;
			glGenBuffers(1, &(ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboTexCoords));
			glBindBuffer(GL_ARRAY_BUFFER, (ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboTexCoords));
			glBufferData(GL_ARRAY_BUFFER, sizeof(struct aiVector3D) * mesh->mNumVertices, mesh->mTextureCoords[0], GL_STATIC_DRAW);
		}

		if(mesh->mColors[0] != NULL){
			ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].hasColors = TRUE;
			glGenBuffers(1, &(ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboColors));
			glBindBuffer(GL_ARRAY_BUFFER, (ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboColors));
			glBufferData(GL_ARRAY_BUFFER, sizeof(struct aiColor4D) * mesh->mNumVertices, mesh->mColors[0], GL_STATIC_DRAW);
		}

		// Assuming triangulated mesh
		unsigned int numIndices = mesh->mNumFaces * 3;
		unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * numIndices);
		
		for (GLuint j = 0; j < mesh->mNumFaces; j++) {
			const struct aiFace* face = &mesh->mFaces[j];
			indices[j * 3 + 0] = face->mIndices[0];
			indices[j * 3 + 1] = face->mIndices[1];
			indices[j * 3 + 2] = face->mIndices[2];
		}

		glGenBuffers(1, &(ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboIndices));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].vboIndices));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);

		free(indices);

		ptMeshes->meshesData[ptMeshes->noOfMeshes - 1].indexCount = numIndices;
	}

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PrintMeshes(PMODEL pModel){
	PMESHES ptMeshes = &(pModel->modelMeshVBOData);
	fprintf(gpFile, "===========================\n");
	if(ptMeshes->noOfMeshes > 0){
		for(GLuint i = 0; i < ptMeshes->noOfMeshes; i++){
			fprintf(gpFile, "===========================\n");
			fprintf(gpFile, "Mesh : %s : VBO Vertices : %u\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].vboVertices);
			fprintf(gpFile, "Mesh : %s : VBO Normals : %u\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].vboNormals);
			fprintf(gpFile, "Mesh : %s : VBO TexCoords : %u\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].vboTexCoords);
			fprintf(gpFile, "Mesh : %s : VBO Colors : %u\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].vboColors);
			fprintf(gpFile, "Mesh : %s : VBO Indices : %u\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].vboIndices);
			fprintf(gpFile, "Mesh : %s : has Normals : %d\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].hasNormals);
			fprintf(gpFile, "Mesh : %s : has Colors : %d\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].hasColors);
			fprintf(gpFile, "Mesh : %s : has TexCoords : %d\n", ptMeshes->meshesData[i].meshName, ptMeshes->meshesData[i].hasTexCoords);
			fprintf(gpFile, "===========================\n");
		}
	}
	else{
		fprintf(gpFile, "No Mesh Data Fetched\n");
	}
	fprintf(gpFile, "===========================\n");
}

void ApplyMaterialAndTexture(const struct aiMaterial *mtl){
	float c[4];

	struct aiColor4D diffuse;
	struct aiColor4D specular;
	struct aiColor4D ambient;
	struct aiString materialName, texturePath;
	GLuint currentDiffuseTextureID = -1;
	GLuint currentNormalTextureID = -1;

	if(AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &materialName)){
		//fprintf(gpFile, "Material Name: %s\n", materialName.data);
	}

	if(aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){
		//fprintf(gpFile, "Material has diffuse texture: %s\n", texturePath.data);
		if(FindExisting2DTexture(texturePath.data, &all2DTextures, &currentDiffuseTextureID) != -1){
			//fprintf(gpFile, "Found Existing Diffuse Texture : %s\n", texturePath.data);
			glBindTexture(GL_TEXTURE_2D, currentDiffuseTextureID);
		}
		else{
			glBindTexture(GL_TEXTURE_2D, 0);
			//fprintf(gpFile, "Did Not Find Existing Diffuse Texture : %s\n", texturePath.data);
		}
	}
	else{
		glBindTexture(GL_TEXTURE_2D, 0);
		//fprintf(gpFile, "No Existing Diffuse Texture\n");
	}

	/* 
	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)){
		color4_to_float4(&diffuse, c);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);
	fprintf(gpFile, "Diffuse Material Colors : %f, %f, %f, %f\n", diffuse.r, diffuse.g, diffuse.b, diffuse.a);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular)){
		color4_to_float4(&specular, c);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	fprintf(gpFile, "Specular Material Colors : %f, %f, %f, %f\n", specular.r, specular.g, specular.b, specular.a);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient)){
		color4_to_float4(&ambient, c);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);
	fprintf(gpFile, "Ambient Material Colors : %f, %f, %f, %f\n", ambient.r, ambient.g, ambient.b, ambient.a);
	*/

	/* 
	GLenum fill_mode;
	int ret1, ret2;
	struct aiColor4D emission;
	ai_real shininess, strength;
	int two_sided;
	int wireframe;
	unsigned int max;
	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	if(ret1 == AI_SUCCESS) {
		max = 1;
		ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
		if(ret2 == AI_SUCCESS)
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE); 
	*/
}

void RecursiveRender(const struct aiScene *scene, const struct aiNode* node, PMESHES ptMeshes) {
	unsigned int n = 0;
	struct aiMatrix4x4 m = node->mTransformation;

	aiTransposeMatrix4(&m);
	glPushMatrix();
	{
		glMultMatrixf((float*)&m);

		for (GLuint eachNode = 0; eachNode < node->mNumMeshes; ++eachNode) {
			const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[eachNode]];

			//fprintf(gpFile, "===========================\n");
			//fprintf(gpFile, "Rendering Mesh : %s\n", mesh->mName.data);

			ApplyMaterialAndTexture(scene->mMaterials[mesh->mMaterialIndex]);

			PMESHDATA pMeshData = &(ptMeshes->meshesData[node->mMeshes[eachNode]]);

			glEnableClientState(GL_VERTEX_ARRAY);

			glBindBuffer(GL_ARRAY_BUFFER, pMeshData->vboVertices);
			glVertexPointer(3, GL_FLOAT, sizeof(struct aiVector3D), (GLvoid*)0);

			if(pMeshData->hasNormals){
				glEnableClientState(GL_NORMAL_ARRAY);
				glBindBuffer(GL_ARRAY_BUFFER, pMeshData->vboNormals);
				glNormalPointer(GL_FLOAT, sizeof(struct aiVector3D), 0);
			}

			if(pMeshData->hasTexCoords){
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindBuffer(GL_ARRAY_BUFFER, pMeshData->vboTexCoords);
				glTexCoordPointer(2, GL_FLOAT, sizeof(struct aiVector3D), 0);
			}

			/* if(pMeshData->hasColors){
				glEnableClientState(GL_COLOR_ARRAY);
				glBindBuffer(GL_ARRAY_BUFFER, pMeshData->vboColors);
				glColorPointer(4, GL_FLOAT, sizeof(struct aiColor4D), 0);
			} */

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMeshData->vboIndices);
			glDrawElements(GL_TRIANGLES, pMeshData->indexCount, GL_UNSIGNED_INT, (GLvoid*)0);

			glDisableClientState(GL_VERTEX_ARRAY);
			if(pMeshData->hasNormals){
				glDisableClientState(GL_NORMAL_ARRAY);
			}
			if(pMeshData->hasTexCoords){
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			if(pMeshData->hasColors){
				glDisableClientState(GL_COLOR_ARRAY);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			//fprintf(gpFile, "Rendering Mesh Completed: %s\n", mesh->mName.data);
			//fprintf(gpFile, "===========================\n\n");
		}

		// Draw all children
		for (n = 0; n < node->mNumChildren; ++n) {
			RecursiveRender(scene, node->mChildren[n], ptMeshes);
		}
	}
	glPopMatrix();
}

void RenderModel(PMODEL pModel){
	//fprintf(gpFile, "No of materials : %d\n", pModel->scene->mNumMaterials);
	RecursiveRender(pModel->scene, pModel->scene->mRootNode, &(pModel->modelMeshVBOData));
}

void CleanMeshes(PMODEL pModel){
	//TODO
	if(pModel->modelMeshVBOData.noOfMeshes > 0){
		for(GLuint i = 0; i < pModel->modelMeshVBOData.noOfMeshes; i++){
			if(pModel->modelMeshVBOData.meshesData[i].vboVertices){
				fprintf(gpFile, "Deleting Vertex VBO For Loaded Mesh from VRAM: %s\n", pModel->modelMeshVBOData.meshesData[i].meshName);
				glDeleteBuffers(1, &(pModel->modelMeshVBOData.meshesData[i].vboVertices));
				pModel->modelMeshVBOData.meshesData[i].vboVertices = 0;
			}
			if(pModel->modelMeshVBOData.meshesData[i].vboNormals){
				fprintf(gpFile, "Deleting Normals VBO For Loaded Mesh from VRAM : %s\n", pModel->modelMeshVBOData.meshesData[i].meshName);
				glDeleteBuffers(1, &(pModel->modelMeshVBOData.meshesData[i].vboNormals));
				pModel->modelMeshVBOData.meshesData[i].vboNormals = 0;
				pModel->modelMeshVBOData.meshesData[i].hasNormals = 0;
			}
			if(pModel->modelMeshVBOData.meshesData[i].vboTexCoords){
				fprintf(gpFile, "Deleting Texture VBO For Loaded Mesh from VRAM : %s\n", pModel->modelMeshVBOData.meshesData[i].meshName);
				glDeleteBuffers(1, &(pModel->modelMeshVBOData.meshesData[i].vboTexCoords));
				pModel->modelMeshVBOData.meshesData[i].vboTexCoords = 0;
				pModel->modelMeshVBOData.meshesData[i].hasTexCoords = 0;
			}
			if(pModel->modelMeshVBOData.meshesData[i].vboColors){
				fprintf(gpFile, "Deleting Colors VBO For Loaded Mesh from VRAM : %s\n", pModel->modelMeshVBOData.meshesData[i].meshName);
				glDeleteBuffers(1, &(pModel->modelMeshVBOData.meshesData[i].vboColors));
				pModel->modelMeshVBOData.meshesData[i].vboColors = 0;
				pModel->modelMeshVBOData.meshesData[i].hasColors = 0;
			}
			if(pModel->modelMeshVBOData.meshesData[i].vboIndices){
				fprintf(gpFile, "Deleting Indices VBO For Loaded Mesh from VRAM : %s\n", pModel->modelMeshVBOData.meshesData[i].meshName);
				glDeleteBuffers(1, &(pModel->modelMeshVBOData.meshesData[i].vboIndices));
				pModel->modelMeshVBOData.meshesData[i].vboIndices = 0;
				pModel->modelMeshVBOData.meshesData[i].indexCount = 0;
			}
		}
	}
}

void UnInitializeMeshes(PMODEL pModel){
	fprintf(gpFile, "Deleting All Meshes related VBO ID's for model from RAM %s\n", pModel->modelName);
	if(pModel->modelMeshVBOData.meshesData){
		free(pModel->modelMeshVBOData.meshesData);
		pModel->modelMeshVBOData.meshesData = NULL;
		pModel->modelMeshVBOData.noOfMeshes = 0;
	}
}

void UnLoadModel(PMODEL pModel){
	CHAR bufferModelName[256];
	if(pModel){	
		strcpy_s(bufferModelName, (strlen(pModel->modelName) + 1) * sizeof(CHAR), pModel->modelName);
		fprintf(gpFile, "\n\n######### Started Cleaning All Data from RAM and VRAM related to Model :  %s #########\n", bufferModelName);
		CleanMeshes(pModel);
		PrintMeshes(pModel);
		UnInitializeMeshes(pModel);
		fprintf(gpFile, "Deleting Assimp Loaded Model : %s\n", pModel->modelName);
		if(pModel->scene)
			aiReleaseImport(pModel->scene);
		free(pModel);
		pModel = NULL;
		fprintf(gpFile, "\n\n######### Finishes Cleaning All Data from RAM and VRAM related to Model :  %s #########\n\n", bufferModelName);
	}
}

void UnInitAssimp(){
	aiDetachAllLogStreams();
}

/* Helper functions */
void color4_to_float4(const struct aiColor4D *c, float f[4]){
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d){
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}
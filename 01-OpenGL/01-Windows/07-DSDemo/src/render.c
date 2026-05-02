#include "render.h"
#include "textures.h"
#include "model.h"
#include "cubemap.h"

extern HDC ghdc;
extern HWND ghWnd;

CURRENTSCENE currentScene = INTRO_SCENE_1;
CURRENTSUBSCENE currentSubScene = SCENE_1_SUBSCENE1;

enum FadePhase {
    FADE_OUT,
    FADE_IN,
    NONE
};

enum FadePhase phase = NONE;
float alphaForFadeEffect    = 1.0f;   
float fadeInSpeed           = 0.01f; 
float fadeOutSpeed          = 0.01f; 

/* Model Loading using Assimp */
TEXTURES2DLIST all2DTextures;
PMODEL pModelCottage = NULL;
PMODEL pModelTrain = NULL;
PMODEL pModelTree = NULL;
PMODEL pModelCabinet = NULL;
PMODEL pModelPlatform = NULL;
PMODEL pModelTrainTrack = NULL;
PMODEL pModelDessert = NULL;
PMODEL pModelGreenField = NULL;
PMODEL pModelCactus = NULL;
PMODEL pModelCityRoad = NULL;
PMODEL pModelCity = NULL;
PMODEL pModelWoman = NULL;
PMODEL pModelRoom = NULL;
PMODEL pModelFood = NULL;

TEXTURECUBEMAP scene1CubeMap;
TEXTURECUBEMAP scene2CubeMap;
TEXTURECUBEMAP scene3CubeMap;
TEXTURECUBEMAP scene8CubeMap;

/* Hard Coded Model Loading */
GLuint vbo, ebo;

/* Animation */
GLfloat anglePyramid = 0.0f;

float yaw = 0.0f, pitch = 0.0f;
float distance = 10.0f;
float lastX, lastY;
int rotating = 0, panning = 0;
float panX = 0.0f, panY = 0.0f;

/* Camera */
GLfloat scene4cameraPosZ = 100.0f;
GLfloat scene4cameraPosX = 0.0f;
GLfloat scene4cameraRadius = 80.0f;
GLfloat scene4cameraPosAngle = 0.0f;

GLfloat scene5cameraPosZ = 100.0f;
GLfloat scene5cameraPosX = 0.0f;
GLfloat scene5cameraRadius = 100.0f;
GLfloat scene5cameraPosAngle = 0.0f;

/* Intro screen 1 Variables */
GLfloat introScene1Timer = 0.0f;
GLfloat introScene2Timer = 0.0f;
GLfloat outroScene1Timer = 0.0f;

/* Scene 1 Variables */
GLfloat scene1CameraPosX = 0.0f;
GLfloat scene1CameraPosY = 50.0f;
GLfloat scene1CameraPosZ = 0.0f;

GLfloat scene1CameraLookAtX = 0.0f;
GLfloat scene1CameraLookAtY = 0.0f;
GLfloat scene1CameraLookAtZ = 0.0f;

GLfloat scene1CameraUpVectorX = 0.0f;
GLfloat scene1CameraUpVectorY = 1.0f;
GLfloat scene1CameraUpVectorZ = 0.0f;

GLfloat scene1CameraRadius = 50.0f;
GLfloat scene1CameraPosAngle = 0.0f;

// Train Movement
GLfloat scene1TrainZ = 400.0f;
GLfloat scene2TrainZ = 300.0f;

/* Scene 2 Variables */
GLfloat scene2CameraPosX = 0.0f;
GLfloat scene2CameraPosY = 0.0f;
GLfloat scene2CameraPosZ = 0.0f;

GLfloat scene2CameraLookAtX = 0.0f;
GLfloat scene2CameraLookAtY = 0.0f;
GLfloat scene2CameraLookAtZ = 0.0f;

GLfloat scene2CameraUpVectorX = 0.0f;
GLfloat scene2CameraUpVectorY = 1.0f;
GLfloat scene2CameraUpVectorZ = 0.0f;

GLfloat scene2CameraRadius = 100.0f;
GLfloat scene2CameraPosAngle = 0.0f;

/* Scene 3 Variables */
GLfloat scene3CameraPosX = 50.0f;
GLfloat scene3CameraPosY = 5.0f;
GLfloat scene3CameraPosZ = 140.0f;

GLfloat scene3CameraLookAtX = 50.0f;
GLfloat scene3CameraLookAtY = 0.0f;
GLfloat scene3CameraLookAtZ = -1000.0f;

GLfloat scene3CameraUpVectorX = 0.0f;
GLfloat scene3CameraUpVectorY = 1.0f;
GLfloat scene3CameraUpVectorZ = 0.0f;

GLfloat scene3CameraRadius = 100.0f;
GLfloat scene3CameraPosAngle = 0.0f;

GLfloat vertices[] =
{
    0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.000000f, 0.447214f,  0.894427f, 0.5, 1.0,
	-1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  0.000000f, 0.447214f,  0.894427f, 0.0, 0.0,
	1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  0.000000f, 0.447214f,  0.894427f, 1.0, 0.0,

	0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.894427f, 0.447214f,  0.000000f, 0.5, 1.0,
	1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  0.894427f, 0.447214f,  0.000000f, 1.0, 0.0,
	1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  0.894427f, 0.447214f,  0.000000f, 0.0, 0.0,

	0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.000000f, 0.447214f, -0.894427f, 0.5, 1.0,
	1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  0.000000f, 0.447214f, -0.894427f, 0.0, 0.0,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.000000f, 0.447214f, -0.894427f, 1.0, 0.0,

	0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, -0.894427f, 0.447214f,  0.000000f, 0.5, 1.0,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -0.894427f, 0.447214f,  0.000000f, 1.0, 0.0,
	-1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, -0.894427f, 0.447214f,  0.000000f, 0.0, 0.0,
};

GLuint indices[] = {
    0, 1, 2,     // Front face
    3, 4, 5,     // Right face
    6, 7, 8,     // Back face
    9,10,11      // Left face
};


/* Timer */
extern double elapsedTime;
extern LARGE_INTEGER frequency;
extern LARGE_INTEGER t1, t2;

extern GLboolean printTimer;
extern GLboolean nPressed;
extern GLboolean sPressed;

void glInitialize(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    Initialize2DTextures(&all2DTextures);
    Fetch2DTexturesRecursively(TEXT("res/textures"), &all2DTextures);
    Print2DTextures(&all2DTextures);

    InitializeCubeMapTextures(&scene1CubeMap);
    FetchCubeMapTextures("res/cubemaps/scene5", &scene1CubeMap);

    InitializeCubeMapTextures(&scene2CubeMap);
    FetchCubeMapTextures("res/cubemaps/scene3", &scene2CubeMap);

    InitializeCubeMapTextures(&scene3CubeMap);
    FetchCubeMapTextures("res/cubemaps/scene1", &scene3CubeMap);

    InitializeCubeMapTextures(&scene8CubeMap);
    FetchCubeMapTextures("res/cubemaps/scene8", &scene8CubeMap);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_NORMALIZE);

    InitAssimp();
    pModelCottage = LoadModel("res/models/Cottage/cottage_obj.obj");
    pModelTrain = LoadModel("res/models/TrainWithEngine/TrainWithEngine.obj");
    pModelTree = LoadModel("res/models/Tree/Tree.obj");
    pModelCabinet = LoadModel("res/models/Cabinet/Door.obj");
    pModelPlatform = LoadModel("res/models/Platform/RailwayStation.obj");
    pModelTrainTrack = LoadModel("res/models/Track/rail_straight.obj");
    pModelDessert = LoadModel("res/models/Dessert/terrain.obj");
    pModelGreenField = LoadModel("res/models/GreenField/terrain.obj");
    pModelCactus = LoadModel("res/models/Cactus/scene.gltf");
    pModelCityRoad = LoadModel("res/models/CityRoads/CityRoad.obj");
    pModelCity = LoadModel("res/models/City/city.obj");
    pModelWoman =  LoadModel("res/models/Woman/Woman.obj");
    pModelRoom = LoadModel("res/models/Room/Room.obj");
    pModelFood = LoadModel("res/models/Food/jamun.obj");

    scene4cameraPosAngle = -(M_PI / 3.0f);
    scene5cameraPosAngle = (M_PI / 2.0f);
}

void glUpdate(){
    if(printTimer){
        fprintf(gpFile, "scene1TrainZ Time : %lf ms\n", scene1TrainZ);
        printTimer = FALSE;
    }
    
    if(currentScene == INTRO_SCENE_1){
        phase = FADE_IN;
        
        introScene1Timer = introScene1Timer + 0.01f;
        //Normal
        /* if(introScene1Timer >= 100.0f){
            phase = FADE_OUT;
        }
        if(introScene1Timer >= 105.0f){
            currentScene = INTRO_SCENE_3;
        } */
        //OBS
        /* if(introScene1Timer >= 80.0f){
            phase = FADE_OUT;
        }
        if(introScene1Timer >= 85.0f){
            currentScene = INTRO_SCENE_3;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = INTRO_SCENE_2;
                nPressed = FALSE;
            }
        }
    }
    
    if(currentScene == INTRO_SCENE_2){
        phase = FADE_IN;
        introScene2Timer = introScene2Timer + 0.01f;

        // Normal
        /* if(introScene2Timer >= 30.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 35.0f){
            currentScene = SCENE_1;
        } */

        // OBS
        /* if(introScene2Timer >= 10.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 15.0f){
            currentScene = SCENE_1;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = INTRO_SCENE_3;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == INTRO_SCENE_3){
        phase = FADE_IN;
        introScene2Timer = introScene2Timer + 0.01f;

        // Normal
        /* if(introScene2Timer >= 30.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 35.0f){
            currentScene = SCENE_1;
        } */

        // OBS
        /* if(introScene2Timer >= 10.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 15.0f){
            currentScene = SCENE_1;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = INTRO_SCENE_4;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == INTRO_SCENE_4){
        phase = FADE_IN;
        introScene2Timer = introScene2Timer + 0.01f;

        // Normal
        /* if(introScene2Timer >= 30.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 35.0f){
            currentScene = SCENE_1;
        } */

        // OBS
        /* if(introScene2Timer >= 10.0f){
            phase = FADE_OUT;
        }
        if(introScene2Timer >= 15.0f){
            currentScene = SCENE_1;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = SCENE_1;
                nPressed = FALSE;
            }
        }
    }
    
    if(currentScene == SCENE_1){
        phase = FADE_IN;
        scene1TrainZ = scene1TrainZ - 0.2f;
        if(currentSubScene == SCENE_1_SUBSCENE1){
            scene1CameraPosX = 0.0f;
            scene1CameraPosY = 250.0f;
            scene1CameraPosZ = scene1TrainZ + 45.0f;

            scene1CameraLookAtX = 0.0f;
            scene1CameraLookAtY = 0.0f;
            scene1CameraLookAtZ = scene1TrainZ + 45.0f;

            scene1CameraUpVectorX = 0.0f;
            scene1CameraUpVectorY = 0.0f;
            scene1CameraUpVectorZ = -1.0f;

            /* if(scene1TrainZ <= -180.0f){
                currentSubScene = SCENE_1_SUBSCENE2;
                scene1TrainZ = 400.0f;
            } */

            if(sPressed){
                currentSubScene = SCENE_1_SUBSCENE2;
                scene1TrainZ = 400.0f;
                sPressed = FALSE;
            }
        }

        if(currentSubScene == SCENE_1_SUBSCENE2){
            scene1CameraPosX = -2.0f;
            scene1CameraPosY = 10.0f;
            scene1CameraPosZ = scene1TrainZ - 5.0f;

            scene1CameraLookAtX = 0.0f;
            scene1CameraLookAtY = 0.0f;
            scene1CameraLookAtZ = -1000.0f;

            scene1CameraUpVectorX = 0.0f;
            scene1CameraUpVectorY = 1.0f;
            scene1CameraUpVectorZ = 0.0f;

            if(scene1CameraPosY >= 10.0f){
                scene1CameraPosY = scene1CameraPosY - 0.1f;
            }

            /* if(scene1TrainZ <= -90.0f){
                currentSubScene = SCENE_1_SUBSCENE3;
                scene1TrainZ = 200.0f;
            } */

            if(sPressed){
                currentSubScene = SCENE_1_SUBSCENE3;
                scene1TrainZ = 200.0f;
                sPressed = FALSE;
            }
        }

        if(currentSubScene == SCENE_1_SUBSCENE3){
            scene1CameraPosX = 20.0f;
            scene1CameraPosY = 3.0f;
            scene1CameraPosZ = -100.0f;

            scene1CameraLookAtX = 0.0f;
            scene1CameraLookAtY = 0.0f;
            scene1CameraLookAtZ = scene1TrainZ;

            scene1CameraUpVectorX = 0.0f;
            scene1CameraUpVectorY = 1.0f;
            scene1CameraUpVectorZ = 0.0f;

            /* if(scene1TrainZ <= -600.0f){
                currentScene = SCENE_3;
            } */

            if(nPressed){
                currentScene = SCENE_3;
                nPressed = FALSE;
            }
        }
        /* if(scene1TrainZ <= -150.0f){
            phase = FADE_OUT;
        }

        if(scene1TrainZ <= -250.0f){
            currentScene = SCENE_2;
        } */
    }

    if(currentScene == SCENE_2){
        phase = FADE_IN;

        scene2CameraPosX = scene2CameraRadius * sin(scene2CameraPosAngle);
        scene2CameraPosY = 13.0f;
        scene2CameraPosZ = scene2CameraRadius * cos(scene2CameraPosAngle);
        scene2CameraPosAngle = scene2CameraPosAngle + 0.003f;

        /* if(scene2CameraPosAngle >=  2 * M_PI){
            currentScene = OUTRO_SCENE_1;
        } */

        /* if(scene2TrainZ <= -650.0f){
            phase = FADE_OUT;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 10.0){
                currentScene = OUTRO_SCENE_1;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == SCENE_3){
        phase = FADE_IN;

        scene3CameraPosY = 10.0f;
        scene3CameraPosZ = scene3CameraPosZ - 0.08f;

        /* if(scene3CameraPosZ <= -100.0f){
            phase = FADE_OUT;
        } */
        /* if(scene3CameraPosZ <= -110.0f){
            currentScene = SCENE_4;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 10.0){
                currentScene = SCENE_4;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == SCENE_4){
        phase = FADE_IN;

        scene4cameraPosAngle = scene4cameraPosAngle + 0.001f;
        if(scene4cameraPosAngle <= (M_PI / 3.0f)){
            scene4cameraPosX = scene4cameraRadius * sin(scene4cameraPosAngle);
            scene4cameraPosZ = scene4cameraRadius * cos(scene4cameraPosAngle);
        }
        /* else{
            phase = FADE_OUT;
        } */

        /* if(scene4cameraPosAngle >= (M_PI / 2.4f)){
            currentScene = SCENE_5;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 10.0){
                currentScene = SCENE_5;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == SCENE_5){
        phase = FADE_IN;

        scene5cameraPosAngle = scene5cameraPosAngle - 0.001f;
        if(scene5cameraPosAngle >= -(M_PI / 2.0f)){
            scene5cameraPosX = scene5cameraRadius * sin(scene5cameraPosAngle);
            scene5cameraPosZ = scene5cameraRadius * cos(scene5cameraPosAngle);
        }
        /* else{
            phase = FADE_OUT;
        } */

        /* if(scene5cameraPosAngle >= (M_PI / 2.4f)){
            currentScene = SCENE_2;
        } */
        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 10.0){
                currentScene = SCENE_2;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == OUTRO_SCENE_1){
        phase = FADE_IN;
        outroScene1Timer = outroScene1Timer + 0.01f;
        /* if(outroScene1Timer >= 20.0f){
            phase = FADE_OUT;
        }
        if(outroScene1Timer >= 25.0f){
            outroScene1Timer = 0.0f;
            phase = NONE;
            currentScene = NONE_SCENE;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = OUTRO_SCENE_2;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == OUTRO_SCENE_2){
        phase = FADE_IN;
        outroScene1Timer = outroScene1Timer + 0.01f;
        /* if(outroScene1Timer >= 20.0f){
            phase = FADE_OUT;
        }
        if(outroScene1Timer >= 25.0f){
            outroScene1Timer = 0.0f;
            phase = NONE;
            currentScene = NONE_SCENE;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = OUTRO_SCENE_3;
                nPressed = FALSE;
            }
        }
    }

    if(currentScene == OUTRO_SCENE_3){
        phase = FADE_IN;
        outroScene1Timer = outroScene1Timer + 0.01f;
        /* if(outroScene1Timer >= 20.0f){
            phase = FADE_OUT;
        }
        if(outroScene1Timer >= 25.0f){
            outroScene1Timer = 0.0f;
            phase = NONE;
            currentScene = NONE_SCENE;
        } */

        if(nPressed){
            phase = FADE_OUT;
            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
            if(elapsedTime > 800.0){
                currentScene = NONE_SCENE;
                nPressed = FALSE;
            }
        }
    }

    if(phase == FADE_OUT){
        alphaForFadeEffect = alphaForFadeEffect + fadeOutSpeed;
        if(alphaForFadeEffect >= 1.0f){
            alphaForFadeEffect = 1.0f;
        }
    }
    else if(phase == FADE_IN){
        alphaForFadeEffect = alphaForFadeEffect - fadeInSpeed;
        if(alphaForFadeEffect <= 0.0f){
            alphaForFadeEffect = 0.0f;
        }
    }
}

void drawPyramid(){
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT,  11 * sizeof(GL_FLOAT), (void*)0);
    glNormalPointer(GL_FLOAT, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
    glTexCoordPointer(2, GL_FLOAT,  11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, indices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void updateCamera() {
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;

    float camX = distance * cos(radPitch) * sin(radYaw);
    float camY = distance * sin(radPitch);
    float camZ = distance * cos(radPitch) * cos(radYaw);

    /* 
        Default Position : 
        Position = 0.000000, 0.000000, 10.000000
        Looking At = 0.000000, 0.000000, 0.000000
        Up Vector = 0.000000, 1.000000, 0.000000
    */
    gluLookAt(camX + panX, camY + panY, camZ, panX, panY, 0.0f, 0.0f, 1.0f, 0.0f);
}

void drawBlackScreen(){
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho(0, 1, 0, 1, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            {
                glLoadIdentity();
                glColor4f(0.0f, 0.0f, 0.0f, alphaForFadeEffect);
                glBegin(GL_QUADS);
                    glVertex2f(0.0f, 0.0f);
                    glVertex2f(1.0f, 0.0f);
                    glVertex2f(1.0f, 1.0f);
                    glVertex2f(0.0f, 1.0f);
                glEnd();
            }
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
        }
        
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    RECT rect;
    GetClientRect(ghWnd, &rect);
    resize(rect.right - rect.left, rect.bottom - rect.top);
}

void glDisplay(){
    //gluLookAt(cameraPosX, 13.0f, cameraPosZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    /* MTB Camera, commented as of now */
    updateCamera();

    /* 
        Note : if you want to set a globa light like sun or light within a room that is static in nature, set the light position after gluLookAt
        The default behaviour is, light position is affected by the model-view transformation
        
        Example : 
        GLfloat light0Position[] = {400.0f, 400.0f, 400.0f, 0.0f}; 
        GLfloat light0Ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};  
        GLfloat light0Diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  
        GLfloat light0Specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

        glLightfv(GL_LIGHT1, GL_POSITION, light0Position);
        glLightfv(GL_LIGHT1, GL_AMBIENT, light0Ambient);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light0Diffuse);
        glEnable(GL_LIGHT1);
    */

    /* 
    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        for(GLuint i = 0; i < all2DTextures.noOfTextures; i++){
            glBindTexture(GL_TEXTURE_2D, all2DTextures.textures[i].texID);
            glPushMatrix();
                glTranslatef(-2.0f, 0.0f, 0.0f);
                glRotatef(anglePyramid, 0.0f, 1.0f, 0.0f);
                drawPyramid();
            glPopMatrix();
            glPushMatrix();
                glTranslatef(2.0f, 0.0f, 0.0f);
                glRotatef(anglePyramid, 0.0f, 1.0f, 0.0f);
                drawPyramid();
            glPopMatrix();
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glDisable(GL_TEXTURE_2D);
    glPopMatrix(); 
    */
    glDisable(GL_DITHER);
    glEnable(GL_MULTISAMPLE);

    if(currentScene == INTRO_SCENE_1){
        setDefaultLighting();

        GLuint introScene1TextureID = 0;
        if(FindExisting2DTexture("Astromedicomp.jpg", &all2DTextures, &introScene1TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, introScene1TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == INTRO_SCENE_2){
        setDefaultLighting();

        GLuint introScene1TextureID = 0;
        if(FindExisting2DTexture("RealTimeRendering.jpg", &all2DTextures, &introScene1TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, introScene1TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == INTRO_SCENE_3){
        setDefaultLighting();

        GLuint introScene1TextureID = 0;
        if(FindExisting2DTexture("FragmentGroupPresents.jpg", &all2DTextures, &introScene1TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, introScene1TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == INTRO_SCENE_4){
        setDefaultLighting();

        GLuint introScene2TextureID = 0;
        if(FindExisting2DTexture("MamachyaGawalaJauya.jpg", &all2DTextures, &introScene2TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, introScene2TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == SCENE_1){
        glDisable(GL_LIGHTING);
        DrawSkyboxCubeMap(&scene1CubeMap, 300.0f);
        glEnable(GL_LIGHTING);

        setDefaultLighting();

        gluLookAt(scene1CameraPosX, scene1CameraPosY, scene1CameraPosZ, scene1CameraLookAtX, scene1CameraLookAtY, scene1CameraLookAtZ, scene1CameraUpVectorX, scene1CameraUpVectorY, scene1CameraUpVectorZ);
        glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(200.0f, 100.0f, 200.0f);

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(20.0f, 20.0f, 20.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if(pModelDessert){
                RenderModel(pModelDessert);
            }

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(5.0f, 1.0f, scene1TrainZ);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glEnable(GL_TEXTURE_2D); 

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(2.0f, 2.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   
            if(pModelTrain){
                RenderModel(pModelTrain);
            }

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(5.0f, 1.0f, 0.0f);
            glScalef(2.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelTrainTrack){
                RenderModel(pModelTrainTrack);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        for(GLfloat x = 40; x <= 400; x = x + 40.0f){
            for(GLfloat z = -500; z <= 500; z = z + 40.0f){
                glPushMatrix();
                glEnable(GL_TEXTURE_2D);
                glTranslatef(x, 0.0f, z);
                glScalef(1.5f, 1.5f, 1.5f);
                if(pModelCactus){
                    RenderModel(pModelCactus);
                }
                glDisable(GL_TEXTURE_2D);
                glPopMatrix();
            }
        }
        

        for(GLfloat x = -20; x >= -400; x = x - 40.0f){
            for(GLfloat z = -500; z <= 500; z = z + 40.0f){
                glPushMatrix();
                glEnable(GL_TEXTURE_2D);
                glTranslatef(x, 0.0f, z);
                glScalef(1.5f, 1.5f, 1.5f);
                if(pModelCactus){
                    RenderModel(pModelCactus);
                }
                glDisable(GL_TEXTURE_2D);
                glPopMatrix();
            }
        }
    }
    else if(currentScene == SCENE_2){
        setDefaultLighting();

        gluLookAt(scene2CameraPosX, scene2CameraPosY, scene2CameraPosZ, scene2CameraLookAtX, scene2CameraLookAtY, scene2CameraLookAtZ, scene2CameraUpVectorX, scene2CameraUpVectorY, scene2CameraUpVectorZ);
        
        glDisable(GL_LIGHTING);
        DrawSkyboxCubeMap(&scene2CubeMap, 300.0f);
        glEnable(GL_LIGHTING);

        glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(200.0f, 100.0f, 200.0f);

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(10.0f, 10.0f, 10.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if(pModelGreenField){
                RenderModel(pModelGreenField);
            }
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glTranslatef(0.0f, 5.0f, 0.0f);
            if(pModelPlatform){
                RenderModel(pModelPlatform);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        scene2TrainZ = scene2TrainZ - 0.5f;
        glPushMatrix();
            glTranslatef(5.0f, 1.0f, scene2TrainZ);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glEnable(GL_TEXTURE_2D); 

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(2.0f, 2.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   
            if(pModelTrain){
                RenderModel(pModelTrain);
            }
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-5.0f, 1.0f, 0.0f);
            glEnable(GL_TEXTURE_2D); 
            
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(2.0f, 2.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   
            if(pModelTrain){
                RenderModel(pModelTrain);
            }
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-5.0f, 1.0f, 0.0f);
            glScalef(2.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelTrainTrack){
                RenderModel(pModelTrainTrack);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(5.0f, 1.0f, 0.0f);
            glScalef(2.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelTrainTrack){
                RenderModel(pModelTrainTrack);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();
        
        glPushMatrix();
            glTranslatef(100.0f, 0.0f, 0.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.7f, 0.7f, 0.7f);
            glEnable(GL_TEXTURE_2D);
            if(pModelCottage){
                RenderModel(pModelCottage);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        /* glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glTranslatef(15.0f, 0.0f, 0.0f);
            glScalef(2.0f, 2.0f, 2.0f);
            glTranslatef(0.0f, 0.0f, 0.0f);
            if(pModelCabinet){
                RenderModel(pModelCabinet);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix(); */
            
        for(GLfloat z = -100; z <= 100; z = z + 10.0f){
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glTranslatef(60.0f, 0.0f, z);
            glScalef(1.5f, 1.5f, 1.5f);
            if(pModelTree){
                RenderModel(pModelTree);
            }
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }

        for(GLfloat z = -100; z <= 100; z = z + 10.0f){
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glTranslatef(-60.0f, 0.0f, z);
            glScalef(1.5f, 1.5f, 1.5f);
            if(pModelTree){
                RenderModel(pModelTree);
            }
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }
    }
    else if(currentScene == SCENE_3){
        glDisable(GL_LIGHTING);
        DrawSkyboxCubeMap(&scene3CubeMap, 500.0f);
        glEnable(GL_LIGHTING);

        setDefaultLighting();

        gluLookAt(scene3CameraPosX, scene3CameraPosY, scene3CameraPosZ, scene3CameraLookAtX, scene3CameraLookAtY, scene3CameraLookAtZ, scene3CameraUpVectorX, scene3CameraUpVectorY, scene3CameraUpVectorZ);
        glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(200.0f, 100.0f, 200.0f);

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glScalef(10.0f, 10.0f, 10.0f);
            glMatrixMode(GL_MODELVIEW);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if(pModelGreenField){
                RenderModel(pModelGreenField);
            }
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(0.0f, 1.0f, 0.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelCityRoad){
                RenderModel(pModelCityRoad);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(0.0f, 1.0f, 0.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelCity){
                RenderModel(pModelCity);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
    else if(currentScene == SCENE_4){
        setDefaultLighting();

        gluLookAt(scene4cameraPosX - 50.0f, 160.0f, scene4cameraPosZ - 180.0f, -50.0f, 150.0f, -180.0f, 0.0f, 1.0f, 0.0f);

        glPushMatrix();
            glScalef(100.0f, 100.0f, 100.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelRoom){
                RenderModel(pModelRoom);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-50.0f, 2.0f, -180.0f);
            glScalef(100.0f, 100.0f, 100.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelWoman){
                RenderModel(pModelWoman);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-50.0f, 95.0f, -110.0f);
            glScalef(10.0f, 10.0f, 10.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelFood){
                RenderModel(pModelFood);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
    else if(currentScene == SCENE_5){
        gluLookAt(scene5cameraPosX - 50.0f, 120.0f, scene5cameraPosZ - 120.0f, -50.0f, 120.0f, -130.0f, 0.0f, 1.0f, 0.0f);

        // Set up global lighting before adding spot light
        GLfloat globalLightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalLightAmbient);

        GLfloat spotLightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
        GLfloat spotLightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat spotLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat spotLightPosition[] = { -50.0f, 170.0f, -120.0f, 1.0f };
        GLfloat spotDirection[] = { 0.0f, -1.0f, 0.0f};
        glLightfv(GL_LIGHT1, GL_AMBIENT, spotLightAmbient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, spotLightDiffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spotLightSpecular);
        glLightfv(GL_LIGHT1, GL_POSITION, spotLightPosition);

        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 4.0f);

        glEnable(GL_LIGHT1);
        glDisable(GL_LIGHT0);

        GLfloat materialAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
        GLfloat materialDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat materialSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
        GLfloat materialShininess[] = {128.0f};
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        glPushMatrix();
            glScalef(100.0f, 100.0f, 100.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelRoom){
                RenderModel(pModelRoom);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-50.0f, 2.0f, -200.0f);
            glScalef(100.0f, 100.0f, 100.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelWoman){
                RenderModel(pModelWoman);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-50.0f, 95.0f, -120.0f);
            glScalef(10.0f, 10.0f, 10.0f);
            glEnable(GL_TEXTURE_2D);
            if(pModelFood){
                RenderModel(pModelFood);
            }
            glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
    else if(currentScene == OUTRO_SCENE_1){
        setDefaultLighting();

        GLuint outroScene2TextureID = 0;
        if(FindExisting2DTexture("OutroScreen.jpg", &all2DTextures, &outroScene2TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, outroScene2TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == OUTRO_SCENE_2){
        setDefaultLighting();

        GLuint outroScene2TextureID = 0;
        if(FindExisting2DTexture("Credits.jpg", &all2DTextures, &outroScene2TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, outroScene2TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    else if(currentScene == OUTRO_SCENE_3){
        setDefaultLighting();
        
        GLuint outroScene2TextureID = 0;
        if(FindExisting2DTexture("Gratitude.jpg", &all2DTextures, &outroScene2TextureID) != -1){
            glBindTexture(GL_TEXTURE_2D, outroScene2TextureID);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
            glScalef(5.0f, -5.0f, 1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 0.0f);
            glEnd();
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    // Fade Effect Black Screen
    drawBlackScreen();
}

void glUninitialize(){
    if(pModelCottage){
        UnLoadModel(pModelCottage);
        pModelCottage = NULL;
    }
    if(pModelTrain){
        UnLoadModel(pModelTrain);
        pModelTrain = NULL;
    }
    if(pModelTree){
        UnLoadModel(pModelTree);
        pModelTree = NULL;
    }
    if(pModelCabinet){
        UnLoadModel(pModelCabinet);
        pModelCabinet = NULL;
    }
    if(pModelPlatform){
        UnLoadModel(pModelPlatform);
        pModelPlatform = NULL;
    }
    if(pModelTrainTrack){
        UnLoadModel(pModelTrainTrack);
        pModelTrainTrack = NULL;
    }
    if(pModelDessert){
        UnLoadModel(pModelDessert);
        pModelDessert = NULL;
    }
    if(pModelGreenField){
        UnLoadModel(pModelGreenField);
        pModelGreenField = NULL;
    }
    if(pModelCactus){
        UnLoadModel(pModelCactus);
        pModelCactus = NULL;
    }
    if(pModelCityRoad){
        UnLoadModel(pModelCityRoad);
        pModelCityRoad = NULL;
    }
    if(pModelCity){
        UnLoadModel(pModelCity);
        pModelCity = NULL;
    }
    if(pModelWoman){
        UnLoadModel(pModelWoman);
        pModelWoman = NULL;
    }
    if(pModelRoom){
        UnLoadModel(pModelRoom);
        pModelRoom = NULL;
    }
    if(pModelFood){
        UnLoadModel(pModelFood);
        pModelFood = NULL;
    }
    UnInitAssimp();
    DestroyCubeMapTextures(&scene1CubeMap);
    DestroyCubeMapTextures(&scene2CubeMap);
    Destroy2DTextures(&all2DTextures);
}

void setDefaultLighting(){
    GLfloat light0Position[] = {400.0f, 400.0f, 400.0f, 0.0f}; 
    GLfloat light0Ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};  
    GLfloat light0Diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  
    GLfloat light0Specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glEnable(GL_LIGHT0);

    
    GLfloat materialAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat materialDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat materialShininess[] = {128.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);
}
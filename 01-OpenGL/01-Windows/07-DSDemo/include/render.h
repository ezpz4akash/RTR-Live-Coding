#pragma once
#include "common.h"

typedef enum tagCURRENTSCENE{
    INTRO_SCENE_1, INTRO_SCENE_2, INTRO_SCENE_3, INTRO_SCENE_4, SCENE_1, SCENE_2, SCENE_3, SCENE_4, SCENE_5, OUTRO_SCENE_1, OUTRO_SCENE_2, OUTRO_SCENE_3, NONE_SCENE    
}CURRENTSCENE;

typedef enum tagCURRENTSUBSCENE{
    SCENE_1_SUBSCENE1, SCENE_1_SUBSCENE2, SCENE_1_SUBSCENE3, SCENE_1_SUBSCENE4,
}CURRENTSUBSCENE;

void glInitialize();
void glUpdate();
void glDisplay();
void glUninitialize();

// Miscellaneous
void setDefaultLighting();
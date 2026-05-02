#include "mouse.h"

extern GLfloat anglePyramid;

extern float yaw, pitch;
extern float distance;
extern float lastX, lastY;
extern int rotating, panning;
extern float panX, panY;

void MiddleMouseDown(int x, int y){
    lastX = x;
    lastY = y;
}

void MiddleMouseDownMove(int x, int y){
    float dx = x - lastX;
    float dy = y - lastY;

    lastX = x;
    lastY = y;

    yaw     = yaw + dx * 0.5f;
    pitch   = pitch + dy * 0.5f;

    if (pitch > 89.0f) 
        pitch = 89.0f;

    if (pitch < -89.0f) 
        pitch = -89.0f;
}

void MiddleMouseDownScrollUp(){
    distance -= 0.5f;
    if (distance < 1.0f) 
        distance = 1.0f;
}

void MiddleMouseDownScrollDown(){
    distance = distance + 2.0f;
}
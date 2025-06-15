// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

// OpenGL related header file
#include <gl/GL.h>      // CoreGL
#include <gl/GLU.h>     // GL Utility

// Custom Header File
#include "OGL.h"

//OpenGL related libraries
#pragma comment(lib, "opengl32.lib")    // CoreGL
#pragma comment(lib, "glu32.lib")       // GL Utility

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/* Global variables : related to fullscreen */
BOOL gbFullScreen = FALSE;
HWND ghWnd = NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

/* Variable related to File I/O */
CHAR gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

/* Active Window related variables */
BOOL gbActiveWindow = FALSE;

/* Exit Key Press Related */
BOOL gbEscapeKeyPressed = FALSE;

/* OpenGL related global variables */
HDC ghdc = NULL;
HGLRC ghrc = NULL;

// Solar system related variables
GLUquadric* quadric = NULL;

// Variables for 24 sphere applications

BOOL bLight = FALSE;
GLfloat lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightModelAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightModelLocalViwer[] = {0.0f};

GLfloat angleForXRotation = 0.0f;
GLfloat angleForYRotation = 0.0f;
GLfloat angleForZRotation = 0.0f;

GLint keyPressed = -1;

// Entry Point Functions
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow){
    int initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);
    void resize(int width, int height);

    //variable declarations
    WNDCLASSEX wndClass;
    HWND hWnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR 6");
    BOOL bDone = FALSE;

    // Create Log File
    gpFile = fopen(gszLogFileName, "w");
    if(gpFile == NULL){
        MessageBox(NULL, TEXT("Log File Creation Failed"), TEXT("File I/O Error"), MB_OK);
        exit(0);
    }
    else{
        fprintf(gpFile, "Program Started Successfully!\n");
        fflush(gpFile);
    }

    // WindowClass Initialization
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style  = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndClass.cbClsExtra = 0;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MIDORIA_ICON));
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.lpszClassName = szAppName;
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MIDORIA_ICON));

    // Registration Of WindowClass
    RegisterClassEx(&wndClass);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create Window
    //hWnd = CreateWindow(szAppName, TEXT("RTR 6 - Akash Musale"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - 24 Sphere - Materials"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
    ghWnd = hWnd;

    // Show Windows
    ShowWindow(hWnd, iCmdShow);

    // Paint Background of the Window
    UpdateWindow(hWnd);

    int result = initialize();
    if(result != 0){
        fprintf(gpFile, "Initialize() Failed!\n");
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
    else{
        fprintf(gpFile, "Initialize() Completed Successfully!\n");
    }

    //Set this window as Active window
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    // Game Loop
    while(bDone == FALSE){
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            if(msg.message == WM_QUIT){
                bDone = TRUE;
            }
            else{
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else{
            if(gbActiveWindow == TRUE){ 
                if(gbEscapeKeyPressed == TRUE){
                    bDone = TRUE;
                }

                /* Render */
                display();

                /* Update */
                update();
            }
        }
    }

    /* Uninitialize */
    uninitialize();

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    //Function declaration
    void toggleFullScreen(void);
    void resize(int width, int height);
    void uninitialize(void);

    switch(uMsg){
        case WM_CREATE:
            ZeroMemory((void*)&wpPrev, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

        case WM_SETFOCUS:
            gbActiveWindow = TRUE;
        break;

        case WM_KILLFOCUS:
            gbActiveWindow = FALSE;
        break;

        case WM_ERASEBKGND:
            return 0;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_KEYDOWN:
            switch(wParam){
                case VK_ESCAPE:
                    gbEscapeKeyPressed = TRUE;
                break;
                default:
                break;
            }
        break;

        case WM_CHAR:
            switch(wParam){
                case 'F':
                case 'f':
                    OutputDebugString(TEXT("F Key Is Pressed (WM_CHAR)\n"));
                    if(gbFullScreen == FALSE){
                        gbFullScreen = TRUE;
                        toggleFullScreen();
                    }
                    else{
                        gbFullScreen = FALSE;
                        toggleFullScreen();
                    }
                break;

                case 'L':
                case 'l':
                    bLight = !bLight;
                    if(bLight){
                        glEnable(GL_LIGHTING);
                    }
                    else{
                        glDisable(GL_LIGHTING);
                    }
                break;

                case 'X':
                case 'x':
                    angleForXRotation = 0.0f;
                    keyPressed = 1;
                break;

                case 'Y':
                case 'y':
                    angleForYRotation = 0.0f;
                    keyPressed = 2;
                break;

                case 'Z':
                case 'z':
                    angleForZRotation = 0.0f;
                    keyPressed = 3;
                break;
            }
        break;

        case WM_CLOSE:
            uninitialize();
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void toggleFullScreen(void){
    // variable declarations
    MONITORINFO mi;

    //code
    if(gbFullScreen == TRUE){
        /* Store existing style */
        dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW){
            ZeroMemory(&mi, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            /* Store existing position */
            if(GetWindowPlacement(ghWnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &mi)){
                SetWindowLong(ghWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom -  mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED); 
            }
        }
        ShowCursor(FALSE);
    }
    else{
        SetWindowPlacement(ghWnd, &wpPrev);
        SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

int initialize(void){
    // function declarations
    void printGLInfo(void);
    void resize(int width, int height);

    // variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    //code

    // Pixel format descriptor initialization
    ZeroMemory((void*)&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 8;

    // GetDC
    ghdc = GetDC(ghWnd);
    if(ghdc == NULL){
        fprintf(gpFile, "GetDC failed\n");
        return -1;
    }

    // Get Matching Pixel Format Index Using HDC and PFD
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0){
        fprintf(gpFile, "ChoosePixelFormat failed\n");
        return -2;
    }

    // Select the pixel format of found index
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE){
        fprintf(gpFile, "SetPixelFormat failed\n");
        return -3;
    }

    // Create rendering context using hdc, pfd and chosen pixel format index
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL){
        fprintf(gpFile, "wglCreateContext failed\n");
        return -4;
    }

    // Make this rendering context as current context
    if(wglMakeCurrent(ghdc, ghrc) == FALSE){
        fprintf(gpFile, "wglMakeCurrent failed\n");
        return -5;
    }

    // print openGL info
    printGLInfo();

    // From hear onwards openGL code starts

    // Tell openGL to choose the color to clear the screen
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    //Depth related code
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Initialize quadric
    quadric = gluNewQuadric();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lightModelLocalViwer);

    // Warm up resize
    RECT rect;
    GetClientRect(ghWnd, &rect);
    resize(rect.right - rect.left, rect.bottom - rect.top);

    return 0;
}

void printGLInfo(void){
    // code

    // print openGL information
    fprintf(gpFile, "OPENGL INFORMATION\n");
    fprintf(gpFile, "******************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "******************\n");
}

void resize(int width, int height){
    //code
    
    //If height becomes zero, make height 1
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    /* Set projection mode */

    // Set matrix projection mode
    glMatrixMode(GL_PROJECTION);

    // Set to identity matrix
    glLoadIdentity();

    // To Perspective projection
    //gluPerspective(45.0, ((GLfloat)width / (GLfloat)height), 0.1, 100.0);

    // To orthographic projection
    if(width <= height){
        glOrtho(0.0f, 15.5f,(0.0f * ((GLfloat)height / (GLfloat)width)), (15.5f * ((GLfloat)height / (GLfloat)width)), -10.0f, 10.0f);
    }
    else{
        glOrtho((0.0f * ((GLfloat)width / (GLfloat)height)), (15.5f * ((GLfloat)width / (GLfloat)height)), 0.0f, 15.5f, -10.0f, 10.0f);
    }

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
}

void display(void){
    void draw24Spheres(void);
    
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();

    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0f;
    if(keyPressed == 1){
        glRotatef(angleForXRotation, 1.0f, 0.0f, 0.0f);
        lightPosition[2] = angleForXRotation;
    }
    else if(keyPressed == 2){
        glRotatef(angleForYRotation, 0.0f, 1.0f, 0.0f);
        lightPosition[0] = angleForYRotation;
    }
    else if(keyPressed == 3){
        glRotatef(angleForZRotation, 0.0f, 0.0f, 1.0f);
        lightPosition[1] = angleForZRotation;
    }

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    glPushMatrix();
        draw24Spheres();
    glPopMatrix();

    // Swap the buffers
    SwapBuffers(ghdc);
}

void draw24Spheres(void){
    // variable declarations
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;

    // ***** 1st sphere on 1st column, emerald ***** 
    // ambient material
    materialAmbient[0] = 0.0215f; // r
    materialAmbient[1] = 0.1745f; // g
    materialAmbient[2] = 0.0215f; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.07568f; // r
    materialDiffuse[1] = 0.61424f; // g
    materialDiffuse[2] = 0.07568f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.633f;    // r
    materialSpecular[1] = 0.727811f; // g
    materialSpecular[2] = 0.633f;    // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 1st column, jade *****
    // ambient material
    materialAmbient[0] = 0.135f;  // r
    materialAmbient[1] = 0.2225f; // g
    materialAmbient[2] = 0.1575f; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.54f; // r
    materialDiffuse[1] = 0.89f; // g
    materialDiffuse[2] = 0.63f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.316228f; // r
    materialSpecular[1] = 0.316228f; // g
    materialSpecular[2] = 0.316228f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 1st column, obsidian *****
    // ambient material
    materialAmbient[0] = 0.05375f; // r
    materialAmbient[1] = 0.05f;    // g
    materialAmbient[2] = 0.06625f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.18275f; // r
    materialDiffuse[1] = 0.17f;    // g
    materialDiffuse[2] = 0.22525f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.332741f; // r
    materialSpecular[1] = 0.328634f; // g
    materialSpecular[2] = 0.346435f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.3f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 1st column, pearl *****
    // ambient material
    materialAmbient[0] = 0.25f;    // r
    materialAmbient[1] = 0.20725f; // g
    materialAmbient[2] = 0.20725f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 1.0f;   // r
    materialDiffuse[1] = 0.829f; // g
    materialDiffuse[2] = 0.829f; // b
    materialDiffuse[3] = 1.0f;  // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.296648f; // r
    materialSpecular[1] = 0.296648f; // g
    materialSpecular[2] = 0.296648f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.088f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 1st column, ruby *****
    // ambient material
    materialAmbient[0] = 0.1745f;  // r
    materialAmbient[1] = 0.01175f; // g
    materialAmbient[2] = 0.01175f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.61424f; // r
    materialDiffuse[1] = 0.04136f; // g
    materialDiffuse[2] = 0.04136f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.727811f; // r
    materialSpecular[1] = 0.626959f; // g
    materialSpecular[2] = 0.626959f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 1st column, turquoise *****
    // ambient material
    materialAmbient[0] = 0.1f;     // r
    materialAmbient[1] = 0.18725f; // g
    materialAmbient[2] = 0.1745f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.396f;   // r
    materialDiffuse[1] = 0.74151f; // g
    materialDiffuse[2] = 0.69102f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.297254f; // r
    materialSpecular[1] = 0.30829f;  // g
    materialSpecular[2] = 0.306678f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);


    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 2nd column, brass *****
    // ambient material
    materialAmbient[0] = 0.329412f; // r
    materialAmbient[1] = 0.223529f; // g
    materialAmbient[2] = 0.027451f; // b
    materialAmbient[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.780392f; // r
    materialDiffuse[1] = 0.568627f; // g
    materialDiffuse[2] = 0.113725f; // b
    materialDiffuse[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.992157f; // r
    materialSpecular[1] = 0.941176f; // g
    materialSpecular[2] = 0.807843f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.21794872f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 2nd column, bronze *****
    // ambient material
    materialAmbient[0] = 0.2125f; // r
    materialAmbient[1] = 0.1275f; // g
    materialAmbient[2] = 0.054f;  // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.714f;   // r
    materialDiffuse[1] = 0.4284f;  // g
    materialDiffuse[2] = 0.18144f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.393548f; // r
    materialSpecular[1] = 0.271906f; // g
    materialSpecular[2] = 0.166721f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.2f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 2nd column, chrome *****
    // ambient material
    materialAmbient[0] = 0.25f; // r
    materialAmbient[1] = 0.25f; // g
    materialAmbient[2] = 0.25f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.4f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.774597f; // r
    materialSpecular[1] = 0.774597f; // g
    materialSpecular[2] = 0.774597f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 2nd column, copper *****
    // ambient material
    materialAmbient[0] = 0.19125f; // r
    materialAmbient[1] = 0.0735f;  // g
    materialAmbient[2] = 0.0225f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.7038f;  // r
    materialDiffuse[1] = 0.27048f; // g
    materialDiffuse[2] = 0.0828f;  // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.256777f; // r
    materialSpecular[1] = 0.137622f; // g
    materialSpecular[2] = 0.086014f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 2nd column, gold *****
    // ambient material
    materialAmbient[0] = 0.24725f; // r
    materialAmbient[1] = 0.1995f;  // g
    materialAmbient[2] = 0.0745f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.75164f; // r
    materialDiffuse[1] = 0.60648f; // g
    materialDiffuse[2] = 0.22648f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.628281f; // r
    materialSpecular[1] = 0.555802f; // g
    materialSpecular[2] = 0.366065f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 2nd column, silver *****
    // ambient material
    materialAmbient[0] = 0.19225f; // r
    materialAmbient[1] = 0.19225f; // g
    materialAmbient[2] = 0.19225f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.50754f; // r
    materialDiffuse[1] = 0.50754f; // g
    materialDiffuse[2] = 0.50754f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.508273f; // r
    materialSpecular[1] = 0.508273f; // g
    materialSpecular[2] = 0.508273f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 3rd column, black *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01f; // r
    materialDiffuse[1] = 0.01f; // g
    materialDiffuse[2] = 0.01f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50f; // r
    materialSpecular[1] = 0.50f; // g
    materialSpecular[2] = 0.50f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 3rd column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.1f;  // g
    materialAmbient[2] = 0.06f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.0f;        // r
    materialDiffuse[1] = 0.50980392f; // g
    materialDiffuse[2] = 0.50980392f; // b
    materialDiffuse[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50196078f; // r
    materialSpecular[1] = 0.50196078f; // g
    materialSpecular[2] = 0.50196078f; // b
    materialSpecular[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 3rd column, green *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.1f;  // r
    materialDiffuse[1] = 0.35f; // g
    materialDiffuse[2] = 0.1f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.45f; // r
    materialSpecular[1] = 0.55f; // g
    materialSpecular[2] = 0.45f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 3rd column, red *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.0f;  // g
    materialDiffuse[2] = 0.0f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.6f;  // g
    materialSpecular[2] = 0.6f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 3rd column, white *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.55f; // r
    materialDiffuse[1] = 0.55f; // g
    materialDiffuse[2] = 0.55f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.70f; // r
    materialSpecular[1] = 0.70f; // g
    materialSpecular[2] = 0.70f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 3rd column, yellow plastic *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.0f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.60f; // r
    materialSpecular[1] = 0.60f; // g
    materialSpecular[2] = 0.50f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 4th column, black *****
    // ambient material
    materialAmbient[0] = 0.02f; // r
    materialAmbient[1] = 0.02f; // g
    materialAmbient[2] = 0.02f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01f; // r
    materialDiffuse[1] = 0.01f; // g
    materialDiffuse[2] = 0.01f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.4f;  // r
    materialSpecular[1] = 0.4f;  // g
    materialSpecular[2] = 0.4f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 4th column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.05f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.5f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04f; // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.7f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 4th column, green *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04f; // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 4th column, red *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.4f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.04f; // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 4th column, white *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.05f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.5f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.7f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 4th column, yellow rubber *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);
    // *******************************************************
    // *******************************************************
    // *******************************************************
}

void update(void){
    //code
    angleForXRotation = angleForXRotation + 0.1f;
    /*  if(angleForXRotation >= 360.0f){
        angleForXRotation = angleForXRotation - 360.0f;
    } */

    angleForYRotation = angleForYRotation + 0.1f;
    /* if(angleForYRotation >= 360.0f){
        angleForYRotation = angleForYRotation - 360.0f;
    } */


    angleForZRotation = angleForZRotation + 0.1f;
    /* if(angleForZRotation >= 360.0f){
        angleForZRotation = angleForZRotation - 360.0f;
    } */
}

void uninitialize(void){
    // function declarations
    void toggleFullScreen(void);

    //code
    
    if(quadric){
        gluDeleteQuadric(quadric);
        quadric = NULL;
    }

    //If use is exiting in fullscreen, then restore the fullscreen back to normal
    if(gbFullScreen == TRUE){
        toggleFullScreen();
        gbFullScreen = FALSE;
    }

    // Make HDC as current context by releasing rendering context as current context
    if(wglGetCurrentContext() == ghrc){
        wglMakeCurrent(NULL, NULL);
    }

    // Delete the rendering context
    if(ghrc){
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }
    
    // Release the HDC
    if(ghdc){
        ReleaseDC(ghWnd, ghdc);
        ghdc = NULL;
    }

    // Destroy Window
    if(ghWnd){
        DestroyWindow(ghWnd);
        ghWnd = NULL;
    }

    // Close the file
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

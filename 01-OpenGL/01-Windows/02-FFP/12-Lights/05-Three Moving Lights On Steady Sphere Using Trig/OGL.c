// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define _USE_MATH_DEFINES

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

GLboolean bLight = FALSE;

GLfloat lightAmbientZero[]  = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseZero[]  = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularZero[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionZero[] = {0.0f, 0.0f, 0.0f, 1.0f}; //Positonal Light

GLfloat lightAmbientOne[]  = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseOne[]  = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightSpecularOne[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightPositionOne[] = {0.0f, 0.0f, 0.0f, 1.0f}; //Positonal Light

GLfloat lightAmbientTwo[]  = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseTwo[]  = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularTwo[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionTwo[] = {0.0f, 0.0f, 0.0f, 1.0f}; //Positonal Light

GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;

GLfloat lightAngleZero = 0.0f;
GLfloat lightAngleOne = 0.0f;
GLfloat lightAngleTwo = 0.0f;

/* Transformation Matrices */
float identityMatrix[16];
float RotationMatrixX[16];
float RotationMatrixY[16];
float RotationMatrixZ[16];

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - Three Moving Lights On Steady Sphere"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Depth related code
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientZero);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuseZero);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecularZero);
    //glLightfv(GL_LIGHT0, GL_POSITION, lightPositionZero);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOne);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuseOne);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecularOne);
    //glLightfv(GL_LIGHT1, GL_POSITION, lightPositionOne);
    glEnable(GL_LIGHT1);

    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbientTwo);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuseTwo);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecularTwo);
    //glLightfv(GL_LIGHT2, GL_POSITION, lightPositionTwo);
    glEnable(GL_LIGHT2);

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // Initialize quadric
    quadric = gluNewQuadric();

    // Warm up resize
    RECT rect;
    GetClientRect(ghWnd, &rect);
    resize(rect.right - rect.left, rect.bottom - rect.top);

    // Identity Matrix
    identityMatrix[0]   = 1.0f;
    identityMatrix[1]   = 0.0f;
    identityMatrix[2]   = 0.0f;
    identityMatrix[3]   = 0.0f;
    identityMatrix[4]   = 0.0f;
    identityMatrix[5]   = 1.0f;
    identityMatrix[6]   = 0.0f;
    identityMatrix[7]   = 0.0f;
    identityMatrix[8]   = 0.0f;
    identityMatrix[9]   = 0.0f;
    identityMatrix[10]  = 1.0f;
    identityMatrix[11]  = 0.0f;
    identityMatrix[12]  = 0.0f;
    identityMatrix[13]  = 0.0f;
    identityMatrix[14]  = 0.0f;
    identityMatrix[15]  = 1.0f;

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
    gluPerspective(45.0, ((GLfloat)width / (GLfloat)height), 0.1, 100.0);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
}

void display(void){
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();

    glPushMatrix();
        // Red Light Rotation
        glPushMatrix();
            //glRotatef(lightAngleZero, 1.0f, 0.0f, 0.0f);
            
            RotationMatrixX[0]   = 1.0f;
            RotationMatrixX[1]   = 0.0f;
            RotationMatrixX[2]   = 0.0f;
            RotationMatrixX[3]   = 0.0f;
            RotationMatrixX[4]   = 0.0f;
            RotationMatrixX[5]   = cos(lightAngleZero);
            RotationMatrixX[6]   = sin(lightAngleZero);
            RotationMatrixX[7]   = 0.0f;
            RotationMatrixX[8]   = 0.0f;
            RotationMatrixX[9]   = -sin(lightAngleZero);
            RotationMatrixX[10]  = cos(lightAngleZero);
            RotationMatrixX[11]  = 0.0f;
            RotationMatrixX[12]  = 0.0f;
            RotationMatrixX[13]  = 0.0f;
            RotationMatrixX[14]  = 0.0f;
            RotationMatrixX[15]  = 1.0f;

            glMultMatrixf(RotationMatrixX);

            lightPositionZero[2] = lightAngleZero;
            glLightfv(GL_LIGHT0, GL_POSITION, lightPositionZero);
        glPopMatrix();

        // Green Light Rotation
        glPushMatrix();
            //glRotatef(lightAngleOne, 0.0f, 1.0f, 0.0f);
            RotationMatrixY[0]   = cos(lightAngleOne);
            RotationMatrixY[1]   = 0.0f;
            RotationMatrixY[2]   = -sin(lightAngleOne);
            RotationMatrixY[3]   = 0.0f;
            RotationMatrixY[4]   = 0.0f;
            RotationMatrixY[5]   = 1.0f;
            RotationMatrixY[6]   = 0.0f;
            RotationMatrixY[7]   = 0.0f;
            RotationMatrixY[8]   = sin(lightAngleOne);
            RotationMatrixY[9]   = 0.0f;
            RotationMatrixY[10]  = cos(lightAngleOne);
            RotationMatrixY[11]  = 0.0f;
            RotationMatrixY[12]  = 0.0f;
            RotationMatrixY[13]  = 0.0f;
            RotationMatrixY[14]  = 0.0f;
            RotationMatrixY[15]  = 1.0f;

            glMultMatrixf(RotationMatrixY);

            lightPositionOne[0] = lightAngleOne;
            glLightfv(GL_LIGHT1, GL_POSITION, lightPositionOne);
        glPopMatrix();

        // Blue Light Rotation
        glPushMatrix();
            //glRotatef(lightAngleTwo, 0.0f, 0.0f, 1.0f);
            RotationMatrixZ[0]   = cos(lightAngleTwo);
            RotationMatrixZ[1]   = sin(lightAngleTwo);
            RotationMatrixZ[2]   = 0.0f;
            RotationMatrixZ[3]   = 0.0f;
            RotationMatrixZ[4]   = -sin(lightAngleTwo);
            RotationMatrixZ[5]   = cos(lightAngleTwo);
            RotationMatrixZ[6]   = 0.0f;
            RotationMatrixZ[7]   = 0.0f;
            RotationMatrixZ[8]   = 0.0f;
            RotationMatrixZ[9]   = 0.0f;
            RotationMatrixZ[10]  = 1.0f;
            RotationMatrixZ[11]  = 0.0f;
            RotationMatrixZ[12]  = 0.0f;
            RotationMatrixZ[13]  = 0.0f;
            RotationMatrixZ[14]  = 0.0f;
            RotationMatrixZ[15]  = 1.0f;

            glMultMatrixf(RotationMatrixZ);
            lightPositionTwo[1] = lightAngleTwo;
            glLightfv(GL_LIGHT2, GL_POSITION, lightPositionTwo);
        glPopMatrix();


    glPopMatrix();

    // Do view transformation
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


    // Save above transformation to modelview matrix stack
    glPushMatrix();
        // Adjust the poles of sphere of sun
        //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gluSphere(quadric, 1.5, 100, 100);

    // WE need to go back to the origin, hence pop/restore the matrix
    glPopMatrix();

    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
    lightAngleZero = lightAngleZero + 0.001f;
    if(lightAngleZero >= 360.0f){
        lightAngleZero = lightAngleZero - 360.0f;
    }

    lightAngleOne = lightAngleOne + 0.001f;
    if(lightAngleOne >= 360.0f){
        lightAngleOne = lightAngleOne - 360.0f;
    }


    lightAngleTwo = lightAngleTwo + 0.001f;
    if(lightAngleTwo >= 360.0f){
        lightAngleTwo = lightAngleTwo - 360.0f;
    }
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

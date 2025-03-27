// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// OpenGL related header file
#include <gl/GL.h>      // CoreGL
#include <gl/GLU.h>     // GL Utility

// Custom Header File
#include "OGL.h"

//OpenGL related libraries
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define GL_PI 3.145
#define DEG_TO_RAD(deg) ((GLfloat)deg * (GL_PI / 180.0f))

#define CHARACTER_WIDTH 20
#define GAP_BETWEEN_CHARACTERS 4

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

GLfloat orangeColor[]   = {255.0f / 255.0f, 103.0f / 255.0f, 31.0f / 255.0f};
GLfloat whiteColor[]    = {1.0f, 1.0f, 1.0f};
GLfloat greenColor[]    = {4.0f / 255.0f, 106.0f / 255.0f, 56.0f / 255.0f};
GLfloat grayColor[]     = {0.5f, 0.5f, 0.5f};

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - Dynamic Bharat"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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
    // gluPerspective(45.0, ((GLfloat)width / (GLfloat)height), 0.1, 100.0);

    // To orthographic projection
    if(width <= height){
        glOrtho(-100.0f, 100.0f,(-100.0f * ((GLfloat)height / (GLfloat)width)), (100.0f * ((GLfloat)height / (GLfloat)width)), -100.0f, 100.0f);
    }
    else{
        glOrtho((-100.0f * ((GLfloat)width / (GLfloat)height)), (100.0f * ((GLfloat)width / (GLfloat)height)), -100.0f, 100.0f, -100.0f, 100.0f);
    }

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
}

void display(void){
    void drawBHARAT();
    void drawJet();

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
    glScalef(1.5f, 1.5f, 0.0f);
    drawBHARAT();

    glLoadIdentity();
    glScalef(0.5f, 0.5f, 1.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    drawJet();

    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
}

void drawBHARAT(){
    void drawB();
    void drawH();
    void drawA1();
    void drawR();
    void drawA2();
    void drawT();

    drawB();
    drawH();
    drawA1();
    drawR();
    drawA2();
    drawT();
}

void drawB(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-70.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-70.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-64.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-64.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-70.0f, 1.0f, 0.0f);
        glVertex3f(-70.0f, -1.0f, 0.0f);
        glVertex3f(-64.0f, -1.0f, 0.0f);
        glVertex3f(-64.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-70.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-70.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-64.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-64.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-64.0f, 15.0f, 0.0f);
        glVertex3f(-64.0f, 9.0f, 0.0f);
        glVertex3f(-56.0f, 9.0f, 0.0f);
        glVertex3f(-56.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(-64.0f, 3.0f, 0.0f);
        glVertex3f(-64.0f, -3.0f, 0.0f);
        glVertex3f(-56.0f, -3.0f, 0.0f);
        glVertex3f(-56.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(greenColor);
        glVertex3f(-64.0f, -15.0f, 0.0f);
        glVertex3f(-56.0f, -15.0f, 0.0f);
        glVertex3f(-56.0f, -9.0f, 0.0f);
        glVertex3f(-64.0f, -9.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        glColor3fv(orangeColor);
        glVertex3f(-56.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-56.0f, 0.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-50.0f, 6.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-50.0f, 12.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        glColor3fv(whiteColor);
        glVertex3f(-56.0f, 0.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-56.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-50.0f, -12.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-50.0f, -3.0f, 0.0f);
    glEnd();
}

void drawH(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-46.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-46.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-40.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-40.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-46.0f, 1.0f, 0.0f);
        glVertex3f(-46.0f, -1.0f, 0.0f);
        glVertex3f(-40.0f, -1.0f, 0.0f);
        glVertex3f(-40.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-46.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-46.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-40.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-40.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(-40.0f, 3.0f, 0.0f);
        glVertex3f(-40.0f, -3.0f, 0.0f);
        glVertex3f(-32.0f, -3.0f, 0.0f);
        glVertex3f(-32.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-32.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-32.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-26.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-26.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-32.0f, 1.0f, 0.0f);
        glVertex3f(-32.0f, -1.0f, 0.0f);
        glVertex3f(-26.0f, -1.0f, 0.0f);
        glVertex3f(-26.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-32.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-32.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-26.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-26.0f, -1.0f, 0.0f);
    glEnd();
}

void drawA1(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);    
        glVertex3f(-22.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-22.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-16.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-16.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-22.0f, 1.0f, 0.0f);
        glVertex3f(-22.0f, -1.0f, 0.0f);
        glVertex3f(-16.0f, -1.0f, 0.0f);
        glVertex3f(-16.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-22.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-22.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-16.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-16.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-16.0f, 15.0f, 0.0f);
        glVertex3f(-16.0f, 9.0f, 0.0f);
        glVertex3f(-8.0f, 9.0f, 0.0f);
        glVertex3f(-8.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3fv(whiteColor);
        glVertex3f(-16.0f, 3.0f, 0.0f);
        glVertex3f(-16.0f, -3.0f, 0.0f);
        glVertex3f(-8.0f, -3.0f, 0.0f);
        glVertex3f(-8.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(-8.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-8.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-2.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(-2.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-8.0f, 1.0f, 0.0f);
        glVertex3f(-8.0f, -1.0f, 0.0f);
        glVertex3f(-2.0f, -1.0f, 0.0f);
        glVertex3f(-2.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(-8.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-8.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(-2.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(-2.0f, -1.0f, 0.0f);
    glEnd();
}

void drawR(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(2.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(2.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(8.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(8.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(2.0f, 1.0f, 0.0f);
        glVertex3f(2.0f, -1.0f, 0.0f);
        glVertex3f(8.0f, -1.0f, 0.0f);
        glVertex3f(8.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(2.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(2.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(8.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(8.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(8.0f, 15.0f, 0.0f);
        glVertex3f(8.0f, 9.0f, 0.0f);
        glVertex3f(16.0f, 9.0f, 0.0f);
        glVertex3f(16.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(8.0f, 3.0f, 0.0f);
        glVertex3f(8.0f, -3.0f, 0.0f);
        glVertex3f(16.0f, -3.0f, 0.0f);
        glVertex3f(16.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        glColor3fv(orangeColor);
        glVertex3f(16.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(16.0f, -3.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(22.0f, 0.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(22.0f, 12.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(8.0f, -3.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(15.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(22.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(16.0f, -3.0f, 0.0f);
    glEnd();
}

void drawA2(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(26.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(26.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(32.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(32.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(26.0f, 1.0f, 0.0f);
        glVertex3f(26.0f, -1.0f, 0.0f);
        glVertex3f(32.0f, -1.0f, 0.0f);
        glVertex3f(32.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(26.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(26.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(32.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(32.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(32.0f, 15.0f, 0.0f);
        glVertex3f(32.0f, 9.0f, 0.0f);
        glVertex3f(40.0f, 9.0f, 0.0f);
        glVertex3f(40.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(32.0f, 3.0f, 0.0f);
        glVertex3f(32.0f, -3.0f, 0.0f);
        glVertex3f(40.0f, -3.0f, 0.0f);
        glVertex3f(40.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(40.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(40.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(46.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(46.0f, 15.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(40.0f, 1.0f, 0.0f);
        glVertex3f(40.0f, -1.0f, 0.0f);
        glVertex3f(46.0f, -1.0f, 0.0f);
        glVertex3f(46.0f, 1.0f, 0.0f);

        glColor3fv(whiteColor);
        glVertex3f(40.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(40.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(46.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(46.0f, -1.0f, 0.0f);
    glEnd();
}

void drawT(){
    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(57.0f, 15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(57.0f, 1.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(63.0f, 1.0f, 0.0f);
        glColor3fv(orangeColor);
        glVertex3f(63.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(57.0f, 1.0f, 0.0f);
        glVertex3f(57.0f, -1.0f, 0.0f);
        glVertex3f(63.0f, -1.0f, 0.0f);
        glVertex3f(63.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(whiteColor);
        glVertex3f(57.0f, -1.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(57.0f, -15.0f, 0.0f);
        glColor3fv(greenColor);
        glVertex3f(63.0f, -15.0f, 0.0f);
        glColor3fv(whiteColor);
        glVertex3f(63.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3fv(orangeColor);
        glVertex3f(50.0f, 15.0f, 0.0f);
        glVertex3f(50.0f, 9.0f, 0.0f);
        glVertex3f(70.0f, 9.0f, 0.0f);
        glVertex3f(70.0f, 15.0f, 0.0f);
    glEnd();
}

void drawJet(){
    glColor3f(0.5f, 0.5f, 0.5f);

    GLfloat xP1, xP2, yP1, yP2, xControl, yControl, radius;

    {
        xP1 = -5.0f;
        yP1 = 30.0f;

        xP2 = 5.0f;
        yP2 = 30.0f;

        xControl = 0.0f;
        yControl = 45.0f;

        glColor3ub(78, 136, 122);
        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-5.0f, 30.0f, 0.0f);
            glVertex3f(-5.0f, -30.0f, 0.0f);
            glVertex3f(5.0f, -30.0f, 0.0f);
            glVertex3f(5.0f, 30.0f, 0.0f);
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-5.0f, -30.0f, 0.0f);
            glVertex3f(-5.0f, -50.0f, 0.0f);
            glVertex3f(5.0f, -50.0f, 0.0f);
            glVertex3f(5.0f, -30.0f, 0.0f);
        glEnd();

        xP1 = -5.0f;
        yP1 = -50.0f;

        xP2 = 5.0f;
        yP2 = -50.0f;

        xControl = 0.0f;
        yControl = -55.0f;

        glColor3ub(78, 136, 122);
        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();
    }

    // Right wing
    {
        glColor3ub(33,86,72);
        glBegin(GL_QUADS);
            glVertex3f(15.0f, 10.0f, 0.0f);
            glVertex3f(15.0f, -15.0f, 0.0f);
            glVertex3f(20.0f, -15.0f, 0.0f);
            glVertex3f(20.0f, 10.0f, 0.0f);
        glEnd();

        xP1 = 15.0f;
        yP1 = -15.0f;

        xP2 = 20.0f;
        yP2 = -15.0f;

        xControl = 17.5f;
        yControl = -20.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(25.0f, 2.0f, 0.0f);
            glVertex3f(25.0f, -20.0f, 0.0f);
            glVertex3f(30.0f, -20.0f, 0.0f);
            glVertex3f(30.0f, 2.0f, 0.0f);
        glEnd();

        xP1 = 25.0f;
        yP1 = -20.0f;

        xP2 = 30.0f;
        yP2 = -20.0f;

        xControl = 27.5f;
        yControl = -25.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        
        glColor3ub(55, 113, 99);
        /* Right wing */
        glBegin(GL_QUADS);
            glVertex3f(5.0f, 10.0f, 0.0f);
            glVertex3f(5.0f, -10.0f, 0.0f);
            glVertex3f(40.0f, -15.0f, 0.0f);
            glVertex3f(40.0f, -10.0f, 0.0f);
        glEnd();

        // Flag
        glColor3fv(orangeColor);
        radius = 4.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(whiteColor);
        radius = 2.5f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(greenColor);
        radius = 1.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();
        glColor3ub(55, 113, 99);
    }
    
    // Left wing
    {
        glColor3ub(33,86,72);
        glBegin(GL_QUADS);
            glVertex3f(-15.0f, 10.0f, 0.0f);
            glVertex3f(-15.0f, -15.0f, 0.0f);
            glVertex3f(-20.0f, -15.0f, 0.0f);
            glVertex3f(-20.0f, 10.0f, 0.0f);
        glEnd();

        xP1 = -15.0f;
        yP1 = -15.0f;

        xP2 = -20.0f;
        yP2 = -15.0f;

        xControl = -17.5f;
        yControl = -20.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-25.0f, 2.0f, 0.0f);
            glVertex3f(-25.0f, -20.0f, 0.0f);
            glVertex3f(-30.0f, -20.0f, 0.0f);
            glVertex3f(-30.0f, 2.0f, 0.0f);
        glEnd();

        xP1 = -25.0f;
        yP1 = -20.0f;

        xP2 = -30.0f;
        yP2 = -20.0f;

        xControl = -27.5f;
        yControl = -25.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + 0.01f){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glColor3ub(55, 113, 99);
        /* Left wing */
        glBegin(GL_QUADS);
            glVertex3f(-5.0f, 10.0f, 0.0f);
            glVertex3f(-5.0f, -10.0f, 0.0f);
            glVertex3f(-40.0f, -15.0f, 0.0f);
            glVertex3f(-40.0f, -10.0f, 0.0f);
        glEnd();

        // Flag
        glColor3fv(orangeColor);
        radius = 4.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(whiteColor);
        radius = 2.5f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(greenColor);
        radius = 1.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();
        glColor3ub(55, 113, 99);
    }

    {
        glBegin(GL_QUADS);
            glVertex3f(-5.0f, -40.0f, 0.0f);
            glVertex3f(-20.0f, -50.0f, 0.0f);
            glVertex3f(-20.0f, -53.0f, 0.0f);
            glVertex3f(-5.0f, -50.0f, 0.0f);
        glEnd();
    }

    {
        glBegin(GL_QUADS);
            glVertex3f(5.0f, -40.0f, 0.0f);
            glVertex3f(20.0f, -50.0f, 0.0f);
            glVertex3f(20.0f, -53.0f, 0.0f);
            glVertex3f(5.0f, -50.0f, 0.0f);
        glEnd();
    }
    
}

void uninitialize(void){
    // function declarations
    void toggleFullScreen(void);

    //code
    
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

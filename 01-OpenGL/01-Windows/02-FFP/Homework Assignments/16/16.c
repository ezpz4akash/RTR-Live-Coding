// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mmsystem.h>

// OpenGL related header file
#include <gl/GL.h>      // CoreGL
#include <gl/GLU.h>     // GL Utility

// Custom Header File
#include "OGL.h"

//OpenGL related libraries
#pragma comment(lib, "opengl32.lib")    // CoreGL
#pragma comment(lib, "glu32.lib")       // GL Utility
#pragma comment(lib, "Winmm.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define GL_PI 3.145
#define DEG_TO_RAD(deg) ((GLfloat)deg * (GL_PI / 180.0f))

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

GLboolean run = FALSE;

typedef struct Point{
    GLfloat x, y;
} Point;

GLfloat zRot = 0.0f;

Point triangleStartPos, triangleEndPos, trianglePos, triangleCoordA, triangleCoordB, triangleCoordC;
Point circleStartPos, circleEndPos, circlePos;
Point wandStartPos, wandEndPos, wandPos;

GLfloat tTrianglePos = 0.0f;
GLfloat tCirclePos = 0.0f;
GLfloat tWandPos = 0.0f;
GLfloat radius = 0.0f;

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - Dynamic Deathly Hallows"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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

                if(run){
                    /* Update */
                    update();
                }
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
                case VK_SPACE:
                    PlaySound(TEXT("PotterTheme.wav"), NULL, SND_ASYNC | SND_FILENAME);
                    run = TRUE;
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

    /* Initialize the position of objects */
    triangleCoordA.x = 0.0f;
    triangleCoordA.y = 1.0f;
    triangleCoordB.x = -1.0f;
    triangleCoordB.y = -1.0f;
    triangleCoordC.x = 1.0f;
    triangleCoordC.y = -1.0f;

    triangleStartPos.x = -6.0f;
    triangleStartPos.y = -5.0f;
    triangleEndPos.x = 0.0f;
    triangleEndPos.y = 0.0f;
    trianglePos.x = triangleStartPos.x;
    trianglePos.y = triangleStartPos.y;

    circleStartPos.x = 6.0f;
    circleStartPos.y = -5.0f;
    circlePos.x = circleStartPos.x;
    circlePos.y = circleStartPos.y;
    
    wandStartPos.x = 0.0f;
    wandStartPos.y = 6.0f;
    wandEndPos.x = 0.0f;
    wandEndPos.y = 0.0f;
    wandPos.x = wandStartPos.x;
    wandPos.y = wandStartPos.y;
    
    GLfloat lenA = sqrtf((triangleCoordC.x - triangleCoordB.x) * (triangleCoordC.x - triangleCoordB.x) + (triangleCoordC.y - triangleCoordB.y) * (triangleCoordC.y - triangleCoordB.y));
    GLfloat lenB = sqrtf((triangleCoordC.x - triangleCoordA.x) * (triangleCoordC.x - triangleCoordA.x) + (triangleCoordC.y - triangleCoordA.y) * (triangleCoordC.y - triangleCoordA.y));
    GLfloat lenC = sqrtf((triangleCoordB.x - triangleCoordA.x) * (triangleCoordB.x - triangleCoordA.x) + (triangleCoordB.y - triangleCoordA.y) * (triangleCoordB.y - triangleCoordA.y));

    circleEndPos.x = ((lenA * triangleCoordA.x + lenB * triangleCoordB.x + lenC * triangleCoordC.x) / (lenA + lenB + lenC));
    circleEndPos.y = ((lenA * triangleCoordA.y + lenB * triangleCoordB.y + lenC * triangleCoordC.y) / (lenA + lenB + lenC));

    GLfloat s = 0.5f * (lenA + lenB + lenC);
    radius = sqrtf(((s - lenA) * (s - lenB) * (s - lenC))/(s));

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

    // Polygon mode
    glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);

    glLineWidth(2.0f);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();

    // Translate triangle backwards by z
    glTranslatef(trianglePos.x, trianglePos.y, -7.0f);

    glRotatef(zRot, 0.0f, 1.0f, 0.0f);

    glColor3ub(255, 215, 0);
    glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
    glEnd();


    // Set to identity matrix
    glLoadIdentity();

    // Translate triangle backwards by z
    glTranslatef(circlePos.x, circlePos.y, -7.0f);

    glRotatef(zRot, 0.0f, 1.0f, 0.0f);

    glBegin(GL_LINE_LOOP);
		for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
			glVertex3f((radius * cos(DEG_TO_RAD(angle))), (radius * sin(DEG_TO_RAD(angle))), 0.0f);
		}
	glEnd();

    // Set to identity matrix
    glLoadIdentity();

    // Translate triangle backwards by z
    glTranslatef(wandPos.x, wandPos.y, -7.0f);

    glBegin(GL_LINES);
		glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -1.0f, 0.0f);
	glEnd();

    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
    if(tTrianglePos <= 1.0f){
        trianglePos.x = (1.0f - tTrianglePos) * triangleStartPos.x + triangleEndPos.x * tTrianglePos;
        trianglePos.y = (1.0f - tTrianglePos) * triangleStartPos.y  + triangleEndPos.y * tTrianglePos;
        tTrianglePos = tTrianglePos + 0.0001f;
    }
    else{
        if(tCirclePos <= 1.0f){
            circlePos.x = (1.0f - tCirclePos) * circleStartPos.x + circleEndPos.x * tCirclePos;
            circlePos.y = (1.0f - tCirclePos) * circleStartPos.y  + circleEndPos.y * tCirclePos;
            tCirclePos = tCirclePos + 0.0001f;
        }
        else{
            if(tWandPos <= 1.0f){
                wandPos.x = (1.0f - tWandPos) * wandStartPos.x + wandEndPos.x * tWandPos;
                wandPos.y = (1.0f - tWandPos) * wandStartPos.y  + wandEndPos.y * tWandPos;
                tWandPos = tWandPos + 0.0001f;
            }
        }
    }

    zRot = zRot + 0.1f;
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

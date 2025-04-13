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

/* Rotation angle variables */
GLfloat angleCube = 0.0f;
GLfloat anglePyramid = 0.0f;

/* Texture related global variables */
GLuint textureStone;
GLuint textureKundali;

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - 3D BW Shapes"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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
    BOOL loadGLTexture(GLuint* texture, TCHAR imageResourceID[]);

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

    /* Load Textures */
    if(!loadGLTexture(&textureStone, MAKEINTRESOURCE(IDBITMAP_STONE))){
        fprintf(gpFile, "loadGLTexture Failed to Load Stone Texture\n");
    }

    if(!loadGLTexture(&textureKundali, MAKEINTRESOURCE(IDBITMAP_KUNDALI))){
        fprintf(gpFile, "loadGLTexture Failed to Load Kundali Texture\n");
    }

    /* Enable Texturing */
    glEnable(GL_TEXTURE_2D);

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

/* 
    Steps to use textures : 
        1. Enable Texturing
        2. Generate Texture Object
        3. Bind the Texture
        4. Unpack the pixel store
        5. Specify Texture Parameters
        6. Give the bytes of image data to opengl using gluBuild2DMipmaps
            - Load image into exe as a resource
            - Use LoadImage to get handle to bitmap image 
            - Copy the image bytes data in bmp structure
            - Use in this step(6)
        7. Disable Texturing
*/
BOOL loadGLTexture(GLuint* texture, TCHAR imageResourceID[]){
    // Variable declarations
    HBITMAP hBitMap = NULL;
    BITMAP bmp;
    BOOL bResult = FALSE;

    // Load the bitmap as image
    hBitMap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitMap){
        bResult = TRUE;

        // Get bitmap structure from Loaded Bitmap image
        GetObject(hBitMap, sizeof(BITMAP), &bmp);

        // Generate openGL Texture Object
        glGenTextures(1, texture);

        // Bind to that texture
        glBindTexture(GL_TEXTURE_2D, *texture);

        // Unpack the image in memory for faster loading, 4~RGBA
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        // Set texture parameters : GL_LINEAR ~ Highest Quality, GL_NEAREST ~ Performance over quality
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               //  ~Mag = magnified = object close
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // ~Min = minified = object far

        // Combination of two functions : glTextImage2D and glGenerateMipMap
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (VOID*)bmp.bmBits);

        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);

        DeleteObject(hBitMap);
        hBitMap = NULL;
    }

    return bResult;
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

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureStone);
    // Pyramid
    {
        // Set to identity matrix
        glLoadIdentity();

        // Translate triangle backwards by z
        glTranslatef(-2.0f, 0.0f, -8.0f);

        // Rotate triangle along y
        glRotatef(anglePyramid, 0.0f, 1.0f, 0.0f);

        glBegin(GL_TRIANGLES);
            /* Front Face */
            {
                glTexCoord2f(0.5f, 1.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
            }

            /* Right Face */
            {
                glTexCoord2f(0.5f, 1.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
            }

            /* Back Face */
            {
                glTexCoord2f(0.5f, 1.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);
            }

            /* Left Face */
            {
                glTexCoord2f(0.5f, 1.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);
                
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
            }
        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, textureKundali);
    {
        // Set to identity matrix
        glLoadIdentity();

        // Translate triangle backwards by z
        glTranslatef(2.0f, 0.0f, -8.0f);

        glScalef(0.75f, 0.75f, 0.75f);

        // Rotate triangle along x,y,z
        glRotatef(angleCube, 1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
            /* Front Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
            }

            /* Right Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
            }

            /* Back Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);
            }

            /* Left Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
            }

            /* Top Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
            }

            /* Bottom Face */
            {
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);

                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);

                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
            }
        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
    anglePyramid = anglePyramid + 0.1f;
    if(anglePyramid >= 360.0f){
        anglePyramid = anglePyramid - 360.0f;
    }

    angleCube = angleCube + 0.1f;
    if(angleCube >= 360.0f){
        angleCube = angleCube - 360.0f;
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

    if(textureKundali){
        glDeleteTextures(1, &textureKundali);
        textureKundali = 0;
    }

    if(textureStone){
        glDeleteTextures(1, &textureStone);
        textureStone = 0;
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

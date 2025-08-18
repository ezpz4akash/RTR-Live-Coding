// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

// OpenGL related header file
#include <gl/glew.h>    // This header must be included before any OpenGL headers
#include <gl/GL.h>      // CoreGL

// Custom Header File
#include "OGL.h"
#include "material.h"

#include "vmath.h"
using namespace vmath;

#include "Sphere.h"

//OpenGL related libraries
#pragma comment(lib, "glew32.lib")      // GLEW
#pragma comment(lib, "opengl32.lib")    // CoreGL
#pragma comment(lib, "Sphere.lib")    // Sphere

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

// Shader related variables
GLuint pv_shaderProgramObject = 0;
GLuint pf_shaderProgramObject = 0;

enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_NORMAL,
};

GLuint gVao_sphere = 0;
GLuint gVbo_sphere_position = 0;
GLuint gVbo_sphere_normal = 0;
GLuint gVbo_sphere_element = 0;

mat4 perspectiveProjectionMatrix;

GLuint pv_modelMatrixUniform = 0;
GLuint pv_viewMatrixUniform = 0;
GLuint pv_projectionMatrixUniform = 0;

GLuint pv_laUniform = 0;               // Light Ambient
GLuint pv_ldUniform = 0;               // Light Diffuse
GLuint pv_lsUniform = 0;               // Light Specular
GLuint pv_lightPositionUniform = 0;    // Light Position

GLuint pv_kaUniform = 0;               // Material Ambient
GLuint pv_ksUniform = 0;               // Material Specular
GLuint pv_kdUniform = 0;               // Material Diffuse
GLuint pv_materialShininessUniform = 0; // Material Shininess

GLuint pv_lKeyPressedUniform = 0;      // Light Key Pressed

GLuint pf_modelMatrixUniform = 0;
GLuint pf_viewMatrixUniform = 0;
GLuint pf_projectionMatrixUniform = 0;

GLuint pf_laUniform = 0;               // Light Ambient
GLuint pf_ldUniform = 0;               // Light Diffuse
GLuint pf_lsUniform = 0;               // Light Specular
GLuint pf_lightPositionUniform = 0;    // Light Position

GLuint pf_kaUniform = 0;               // Material Ambient
GLuint pf_ksUniform = 0;               // Material Specular
GLuint pf_kdUniform = 0;               // Material Diffuse
GLuint pf_materialShininessUniform = 0; // Material Shininess

GLuint pf_lKeyPressedUniform = 0;      // Light Key Pressed

GLfloat lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};

BOOL bLight = FALSE; // Light On/Off
BOOL perVertexperFragmentToggle = FALSE;

/* Sphere related variables */
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices = 0;
unsigned int gNumElements = 0;

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - Lights - TwentyFourSpheres"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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
                    if(gbFullScreen == FALSE){
                        gbFullScreen = TRUE;
                        toggleFullScreen();
                    }
                    else{
                        gbFullScreen = FALSE;
                        toggleFullScreen();
                    }
                break;
                default:
                break;
            }
        break;

        case WM_CHAR:
            switch(wParam){
                case 'Q':
                case 'q':
                    gbEscapeKeyPressed = TRUE;
                break;

                case 'F':
                case 'f':
                    perVertexperFragmentToggle = TRUE;
                break;

                case 'V':
                case 'v':
                    perVertexperFragmentToggle = FALSE;
                break;

                case 'L':
                case 'l':
                    bLight = !bLight;
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

                default:
                    keyPressed = -1;
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

    GLenum glewResult;

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

    // Initialize GLEW
    glewResult = glewInit();
    if(glewResult != GLEW_OK){
        fprintf(gpFile, "glewInit failed: %s\n", glewGetErrorString(glewResult));
        return -6;
    }
    
    // print openGL info
    printGLInfo();

    // Vertex Shader
    /* 
        1. Write the shader source code
        2. Create a shader object
        3. Give the shader source code to the shader object
        4. Compile the shader
        5. Check for compilation errors
    */
    GLuint pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pv_vertexShaderSourceCode = 
        "#version 460 core\n" \
        "in vec4 aPosition;\n" \
        "in vec3 aNormal;\n" \
        "uniform mat4 uModelMatrix;\n" \
        "uniform mat4 uViewMatrix;\n" \
        "uniform mat4 uProjectionMatrix;\n"\
        "uniform vec3 uLa;\n" \
        "uniform vec3 uLd;\n" \
        "uniform vec3 uLs;\n" \
        "uniform vec4 uLightPosition;\n" \
        "uniform vec3 uKa;\n" \
        "uniform vec3 uKd;\n" \
        "uniform vec3 uKs;\n" \
        "uniform float uMaterialShininess;\n" \
        "uniform int  uLKeyPressed;\n" \
        "out vec4 out_phong_ads_Light;\n" \
        "void main(void)\n" \
        "{\n" \
        "   if(uLKeyPressed == 1)\n" \
        "   {\n" \
        "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
        "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" \
        "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n" \
        "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n" \
        "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n" \
        "       vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n" \
        "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" \
        "       vec3 ambient = uLa * uKa;\n" \
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n" \
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" \
        "       out_phong_ads_Light = vec4(ambient + diffuse + specular, 1.0);\n" \
        "   }\n" \
        "   else\n" \
        "   {\n" \
        "       out_phong_ads_Light = vec4(1.0, 1.0, 1.0, 1.0);\n" \
        "   }\n" \
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
        "}\n";
    glShaderSource(pv_vertexShaderObject, 1, (const GLchar **)&pv_vertexShaderSourceCode, NULL);
    glCompileShader(pv_vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(pv_vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pv_vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pv_vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Fragment Shader
    /* 
        1. Write the shader source code
        2. Create a shader object
        3. Give the shader source code to the shader object
        4. Compile the shader
        5. Check for compilation errors
    */
    GLuint pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pv_fragmentShaderSourceCode = 
        "#version 460 core\n" \
        "in vec4 out_phong_ads_Light;\n" \
        "out vec4 FragColor;\n" \
        "void main(void)\n" \
        "{\n" \
        "FragColor = out_phong_ads_Light;\n" \
        "}\n";
    glShaderSource(pv_fragmentShaderObject, 1, (const GLchar **)&pv_fragmentShaderSourceCode, NULL);
    glCompileShader(pv_fragmentShaderObject);  
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pv_fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pv_fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pv_fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Shader Program Object
    pv_shaderProgramObject = glCreateProgram();
    if(pv_shaderProgramObject == 0){
        fprintf(gpFile, "glCreateProgram failed\n");
        return -9;
    }

    // Attach vertex shader to the shader program object
    glAttachShader(pv_shaderProgramObject, pv_vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(pv_shaderProgramObject, pv_fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(pv_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");

    // Link the shader program and check for errors
    glLinkProgram(pv_shaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(pv_shaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pv_shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pv_shaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }
    
    // Get the required uniform locations from the shader program object
    pv_modelMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "uModelMatrix");
    pv_viewMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "uViewMatrix");
    pv_projectionMatrixUniform = glGetUniformLocation(pv_shaderProgramObject, "uProjectionMatrix");
    pv_laUniform = glGetUniformLocation(pv_shaderProgramObject, "uLa");
    pv_ldUniform = glGetUniformLocation(pv_shaderProgramObject, "uLd");
    pv_lsUniform = glGetUniformLocation(pv_shaderProgramObject, "uLs");
    pv_kaUniform = glGetUniformLocation(pv_shaderProgramObject, "uKa");
    pv_kdUniform = glGetUniformLocation(pv_shaderProgramObject, "uKd");
    pv_ksUniform = glGetUniformLocation(pv_shaderProgramObject, "uKs");
    pv_materialShininessUniform = glGetUniformLocation(pv_shaderProgramObject, "uMaterialShininess");
    pv_lightPositionUniform = glGetUniformLocation(pv_shaderProgramObject, "uLightPosition");
    pv_lKeyPressedUniform = glGetUniformLocation(pv_shaderProgramObject, "uLKeyPressed");

    /* 
        1. Write the shader source code
        2. Create a shader object
        3. Give the shader source code to the shader object
        4. Compile the shader
        5. Check for compilation errors
    */
    GLuint pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pf_vertexShaderSourceCode = 
        "#version 460 core\n" \
        "in vec4 aPosition;\n" \
        "in vec3 aNormal;\n" \
        "out vec4 eyeCoordinates;\n" \
        "out vec3 transformedNormal;\n" \
        "out vec3 lightSource;\n" \
        "uniform mat4 uModelMatrix;\n" \
        "uniform mat4 uViewMatrix;\n" \
        "uniform mat4 uProjectionMatrix;\n"\
        "uniform vec4 uLightPosition;\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" \
        "   transformedNormal = (normalMatrix * aNormal);\n" \
        "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
        "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" \
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
        "}\n";
    glShaderSource(pf_vertexShaderObject, 1, (const GLchar **)&pf_vertexShaderSourceCode, NULL);
    glCompileShader(pf_vertexShaderObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pf_vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pf_vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pf_vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Fragment Shader
    /* 
        1. Write the shader source code
        2. Create a shader object
        3. Give the shader source code to the shader object
        4. Compile the shader
        5. Check for compilation errors
    */
    GLuint pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pf_fragmentShaderSourceCode = 
        "#version 460 core\n" \
        "in vec3 transformedNormal;\n" \
        "in vec4 eyeCoordinates;\n" \
        "in vec3 lightSource;\n" \
        "uniform vec3 uLa;\n" \
        "uniform vec3 uLd;\n" \
        "uniform vec3 uLs;\n" \
        "uniform vec3 uKa;\n" \
        "uniform vec3 uKd;\n" \
        "uniform vec3 uKs;\n" \
        "uniform float uMaterialShininess;\n" \
        "uniform int  uLKeyPressed;\n" \
        "out vec4 FragColor;\n" \
        "void main(void)\n" \
        "{\n" \
        "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" \
        "   vec3 normalizedLightSource = normalize(lightSource);\n" \
        "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" \
        "   if(uLKeyPressed == 1)\n" \
        "   {\n" \
        "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n" \
        "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n" \
        "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" \
        "       vec3 ambient = uLa * uKa;\n" \
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n" \
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" \
        "       FragColor = vec4(ambient + diffuse + specular, 1.0);\n" \
        "   }\n" \
        "   else\n" \
        "   {\n" \
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" \
        "   }\n" \
        "}\n";
    glShaderSource(pf_fragmentShaderObject, 1, (const GLchar **)&pf_fragmentShaderSourceCode, NULL);
    glCompileShader(pf_fragmentShaderObject);  
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pf_fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pf_fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Shader Program Object
    pf_shaderProgramObject = glCreateProgram();
    if(pf_shaderProgramObject == 0){
        fprintf(gpFile, "glCreateProgram failed\n");
        return -9;
    }

    // Attach vertex shader to the shader program object
    glAttachShader(pf_shaderProgramObject, pf_vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(pf_shaderProgramObject, pf_fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(pf_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");

    // Link the shader program and check for errors
    glLinkProgram(pf_shaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(pf_shaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pf_shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pf_shaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }
    
    // Get the required uniform locations from the shader program object
    pf_modelMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "uModelMatrix");
    pf_viewMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "uViewMatrix");
    pf_projectionMatrixUniform = glGetUniformLocation(pf_shaderProgramObject, "uProjectionMatrix");
    pf_laUniform = glGetUniformLocation(pf_shaderProgramObject, "uLa");
    pf_ldUniform = glGetUniformLocation(pf_shaderProgramObject, "uLd");
    pf_lsUniform = glGetUniformLocation(pf_shaderProgramObject, "uLs");
    pf_kaUniform = glGetUniformLocation(pf_shaderProgramObject, "uKa");
    pf_kdUniform = glGetUniformLocation(pf_shaderProgramObject, "uKd");
    pf_ksUniform = glGetUniformLocation(pf_shaderProgramObject, "uKs");
    pf_materialShininessUniform = glGetUniformLocation(pf_shaderProgramObject, "uMaterialShininess");
    pf_lightPositionUniform = glGetUniformLocation(pf_shaderProgramObject, "uLightPosition");
    pf_lKeyPressedUniform = glGetUniformLocation(pf_shaderProgramObject, "uLKeyPressed");

    // Provide vertex position, color, texture coordinates, normals, etc. to the shader program object
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    fprintf(gpFile, "Number of Sphere Vertices: %d\n", gNumVertices);
    fprintf(gpFile, "Number of Sphere Elements: %d\n", gNumElements);

    for(int i = 0; i < gNumVertices; i++){
        fprintf(gpFile, "Sphere Vertex %d: Position: (%f, %f, %f), Normal: (%f, %f, %f)\n", 
                i, 
                sphere_vertices[i * 3], sphere_vertices[i * 3 + 1], sphere_vertices[i * 3 + 2],
                sphere_normals[i * 3], sphere_normals[i * 3 + 1], sphere_normals[i * 3 + 2]);
    }

    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);
    {
        // Position VBO
        glGenBuffers(1, &gVbo_sphere_position);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        // Normal VBO
        glGenBuffers(1, &gVbo_sphere_normal);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        // Indices VBO
        glGenBuffers(1, &gVbo_sphere_element);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(0); // Unbind the VAO

    //Depth related code
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // From hear onwards openGL code starts, Tell openGL to choose the color to clear the screen
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    // Warm up resize
    RECT rect;
    GetClientRect(ghWnd, &rect);
    resize(rect.right - rect.left, rect.bottom - rect.top);

    return 0;
}

void printGLInfo(void){
    GLint numExtensions, i;

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    // print openGL information
    fprintf(gpFile, "OPENGL INFORMATION\n");
    fprintf(gpFile, "******************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(gpFile, "Number of OpenGL Extensions : %d\n", numExtensions);
    fprintf(gpFile, "******************\n");

    // print all OpenGL extensions
    for(i = 0; i < numExtensions; i++){
        fprintf(gpFile, "OpenGL Extension %d : %s\n", i + 1, glGetStringi(GL_EXTENSIONS, i));
    }
}

void resize(int width, int height){
    //code
    
    //If height becomes zero, make height 1
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program object
    if(perVertexperFragmentToggle){
        glUseProgram(pf_shaderProgramObject);
    }
    else{
        glUseProgram(pv_shaderProgramObject);
    }
    {
        // Transformations
        mat4 modelMatrix = mat4::identity();
        mat4 viewMatrix = mat4::identity();

        mat4 lightRotationMatrix = mat4::identity();
        mat4 lightTranslationMatrix = mat4::identity();
        mat4 lightTransformMatrix = mat4::identity();
        {
            mat4 translationMatrix = mat4::identity();
            mat4 scaleMatrix = mat4::identity();

            // Prepare transformation matrices
            translationMatrix = vmath::translate(-10.0f, -8.0f, -20.0f);
            scaleMatrix = vmath::scale(2.0f, 2.0f, 2.0f);

            if(perVertexperFragmentToggle){
                glUniformMatrix4fv(pf_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
                glUniformMatrix4fv(pf_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
                glUniform3fv(pf_laUniform, 1, lightAmbient);
                glUniform3fv(pf_ldUniform, 1, lightDiffuse);
                glUniform3fv(pf_lsUniform, 1, lightSpecular);
                glUniform1i(pf_lKeyPressedUniform, bLight ? 1 : 0);

                if(keyPressed == 1){
                    lightRotationMatrix = vmath::rotate(angleForXRotation, 1.0f, 0.0f, 0.0f);
                    lightTranslationMatrix = vmath::translate(0.0f, 0.0f, 20.0f);
                }
                else if(keyPressed == 2){
                    lightRotationMatrix = vmath::rotate(angleForYRotation, 0.0f, 1.0f, 0.0f);
                    lightTranslationMatrix = vmath::translate(20.0f, 0.0f, 0.0f);
                }
                else if(keyPressed == 3){
                    lightRotationMatrix = vmath::rotate(angleForZRotation, 0.0f, 0.0f, 1.0f);
                    lightTranslationMatrix = vmath::translate(0.0f, 20.0f, 0.0f);
                }
                else{
                    lightTranslationMatrix = vmath::translate(10.0f, 10.0f, 10.0f);
                }

                for(int i = 0; i < NO_OF_SPHERES; i++){
                    modelMatrix = translationMatrix * vmath::translate(sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]) * scaleMatrix;

                    /*
                        It is important multiply modelView matrix with light Transforms because we also need to move the light source with the model in this case
                        In this case the modeView matrix is dynamic sphere position, and we need the light to revolve around the sphere
                    */
                    lightTransformMatrix = modelMatrix * viewMatrix * lightRotationMatrix * lightTranslationMatrix;

                    glUniform4fv(pf_lightPositionUniform, 1, vec4(lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]) * lightTransformMatrix.transpose());
                    glUniformMatrix4fv(pf_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
                    glUniform3fv(pf_kaUniform, 1, materials[i].ambient);
                    glUniform3fv(pf_kdUniform, 1, materials[i].diffuse);
                    glUniform3fv(pf_ksUniform, 1, materials[i].specular);
                    glUniform1f(pf_materialShininessUniform, materials[i].shininess);
                    glBindVertexArray(gVao_sphere);
                    {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
                        glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
                    }
                    glBindVertexArray(0);
                }
            }
            else{
                glUniformMatrix4fv(pv_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
                glUniformMatrix4fv(pv_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
                glUniform3fv(pv_laUniform, 1, lightAmbient);
                glUniform3fv(pv_ldUniform, 1, lightDiffuse);
                glUniform3fv(pv_lsUniform, 1, lightSpecular);
                glUniform1i(pv_lKeyPressedUniform, bLight ? 1 : 0);

                if(keyPressed == 1){
                    lightRotationMatrix = vmath::rotate(angleForXRotation, 1.0f, 0.0f, 0.0f);
                    lightTranslationMatrix = vmath::translate(0.0f, 0.0f, 20.0f);
                }
                else if(keyPressed == 2){
                    lightRotationMatrix = vmath::rotate(angleForYRotation, 0.0f, 1.0f, 0.0f);
                    lightTranslationMatrix = vmath::translate(20.0f, 0.0f, 0.0f);
                }
                else if(keyPressed == 3){
                    lightRotationMatrix = vmath::rotate(angleForZRotation, 0.0f, 0.0f, 1.0f);
                    lightTranslationMatrix = vmath::translate(0.0f, 20.0f, 0.0f);
                }
                else{
                    lightTranslationMatrix = vmath::translate(10.0f, 10.0f, 10.0f);
                }

                for(int i = 0; i < NO_OF_SPHERES; i++){
                    modelMatrix = translationMatrix * vmath::translate(sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]) * scaleMatrix;

                    /*
                        It is important multiply modelView matrix with light Transforms because we also need to move the light source with the model in this case
                        In this case the modeView matrix is dynamic sphere position, and we need the light to revolve around the sphere
                    */
                    lightTransformMatrix = modelMatrix * viewMatrix * lightRotationMatrix * lightTranslationMatrix;

                    glUniform4fv(pf_lightPositionUniform, 1, vec4(lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]) * lightTransformMatrix.transpose());
                    glUniformMatrix4fv(pv_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
                    glUniform3fv(pv_kaUniform, 1, materials[i].ambient);
                    glUniform3fv(pv_kdUniform, 1, materials[i].diffuse);
                    glUniform3fv(pv_ksUniform, 1, materials[i].specular);
                    glUniform1f(pv_materialShininessUniform, materials[i].shininess);
                    glBindVertexArray(gVao_sphere);
                    {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
                        glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
                    }
                    glBindVertexArray(0);
                }
            }
        }
    }
    glUseProgram(0);

    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
    if(bLight){
        angleForXRotation = angleForXRotation + 0.5f;
        angleForYRotation = angleForYRotation + 0.5f;
        angleForZRotation = angleForZRotation + 0.5f;
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

    // Free vbo and vao
    if(gVbo_sphere_position){
        glDeleteBuffers(1, &gVbo_sphere_position);
        gVbo_sphere_position = 0;
    }
    if(gVbo_sphere_normal){
        glDeleteBuffers(1, &gVbo_sphere_normal);
        gVbo_sphere_normal = 0;
    }

    if(gVao_sphere){
        glDeleteVertexArrays(1, &gVao_sphere);
        gVao_sphere = 0;
    }

    /* 
        Steps to detach and delete shader objects and shader program object generically:
        1. Check if shader program object is not NULL
        2. Get number of attached shaders using glGetProgramiv with GL_ATTACHED_SHADERS
        3. If number of attached shaders is greater than 0, allocate memory for an array of GLuints
        4. Use glGetAttachedShaders to get the attached shaders into the array
        5. Loop through the array, detach each shader using glDetachShader and delete it using glDeleteShader
        6. Free the allocated memory for the array
        7. Use glUseProgram(0) to unbind the shader program
        8. Finally, delete the shader program object using glDeleteProgram
    */

    // Detach Delete the shader objects and delete the shader program object
    if(pv_shaderProgramObject){
        glUseProgram(pv_shaderProgramObject);
        GLint numShaders;
        glGetProgramiv(pv_shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pv_shaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pv_shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(pv_shaderProgramObject);
        pv_shaderProgramObject = 0;
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

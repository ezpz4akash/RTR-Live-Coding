// Win32 Headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

// OpenGL related header file
#include <gl/glew.h>    // This header must be included before any OpenGL headers
#include <gl/GL.h>      // CoreGL

// Custom Header File
#include "OGL.h"

#include "vmath.h"
using namespace vmath;

//OpenGL related libraries
#pragma comment(lib, "glew32.lib")      // GLEW
#pragma comment(lib, "opengl32.lib")    // CoreGL

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
GLuint shaderProgramObject = 0;

enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_TEXCOORD,
    AMC_ATTRIBUTE_NORMAL,
};

/* 
GLuint vbo_position_cube = 0;
GLuint vbo_color_cube = 0;
GLuint vbo_texcoord_cube = 0;
GLuint vbo_normal_cube = 0; 
*/

GLuint vao_cube = 0;
GLuint vbo = 0;

GLuint modelMatrixUniform = 0;
GLuint viewMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;

mat4 perspectiveProjectionMatrix;

/* Rotation angle variables */
GLfloat angleCube = 0.0f;

GLuint laUniform = 0;               // Light Ambient
GLuint ldUniform = 0;               // Light Diffuse
GLuint lsUniform = 0;               // Light Specular
GLuint lightPositionUniform = 0;    // Light Position

GLuint kaUniform = 0;               // Material Ambient
GLuint ksUniform = 0;               // Material Specular
GLuint kdUniform = 0;               // Material Diffuse
GLuint materialShininessUniform = 0; // Material Shininess

GLuint lKeyPressedUniform = 0;      // Light Key Pressed

GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {100.0f, 100.0f, 100.0f, 1.0f};

GLfloat materialAmbient[] = {0.25f, 0.25f, 0.25f, 1.0f};
GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;

BOOL bLight = FALSE; // Light On/Off

/* For Texture */
GLuint textureMarble;
GLuint textureSamplerUniform;

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
    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale - Interleaved"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, (screenWidth - WIN_WIDTH) / 2, (screenHeight  - WIN_HEIGHT) / 2, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
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
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode = 
        "#version 460 core\n" \
        "in vec4 aPosition;\n"
        "in vec4 aColor;\n" \
        "in vec3 aNormal;\n" \
        "in vec2 aTexCoord;\n" \
        "out vec2 out_texCoord;\n" \
        "out vec4 eyeCoordinates;\n" \
        "out vec3 transformedNormal;\n" \
        "out vec3 lightSource;\n" \
        "uniform mat4 uModelMatrix;\n" \
        "uniform mat4 uViewMatrix;\n" \
        "uniform mat4 uProjectionMatrix;\n"\
        "uniform vec4 uLightPosition;\n" \
        "out vec4 out_color;\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" \
        "   transformedNormal = (normalMatrix * aNormal);\n" \
        "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
        "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" \
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
        "   out_color = aColor;\n" \
        "   out_texCoord = aTexCoord;\n" \
        "}\n";
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
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
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode = 
        "#version 460 core\n" \
        "in vec3 transformedNormal;\n" \
        "in vec4 eyeCoordinates;\n" \
        "in vec3 lightSource;\n" \
        "in vec4 out_color;\n" \
        "in vec2 out_texCoord;\n" \
        "uniform vec3 uLa;\n" \
        "uniform vec3 uLd;\n" \
        "uniform vec3 uLs;\n" \
        "uniform vec3 uKa;\n" \
        "uniform vec3 uKd;\n" \
        "uniform vec3 uKs;\n" \
        "uniform float uMaterialShininess;\n" \
        "uniform int  uLKeyPressed;\n" \
        "out vec4 FragColor;\n" \
        "uniform sampler2D uTextureSampler;\n" \
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
        "       FragColor = vec4(ambient + diffuse + specular, 1.0) * out_color * texture(uTextureSampler, out_texCoord);\n" \
        "   }\n" \
        "   else\n" \
        "   {\n" \
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" \
        "   }\n" \
        "}\n";
    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);  
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Shader Program Object
    shaderProgramObject = glCreateProgram();
    if(shaderProgramObject == 0){
        fprintf(gpFile, "glCreateProgram failed\n");
        return -9;
    }

    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObject, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "aColor");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "aTexCoord");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");

    // Link the shader program and check for errors
    glLinkProgram(shaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(shaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }

    // Get the required uniform locations from the shader program object
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    laUniform = glGetUniformLocation(shaderProgramObject, "uLa");
    ldUniform = glGetUniformLocation(shaderProgramObject, "uLd");
    lsUniform = glGetUniformLocation(shaderProgramObject, "uLs");
    kaUniform = glGetUniformLocation(shaderProgramObject, "uKa");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "uLightPosition");
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLKeyPressed");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    // Provide vertex position, color, texture coordinates, normals, etc. to the shader program object
    
    // Cube
    {
        const GLfloat cube_PCNT[] =
        {
            // front
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 0.0f,
            // right			 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
            1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
            // back				 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 0.0f,
            // left				 
            // position				// color			 // normals				// texcoords
            -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
            // top				 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
            // bottom			 
            // position				// color			 // normals				// texcoords
            1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
        };

        glGenVertexArrays(1, &vao_cube);
        glBindVertexArray(vao_cube);
        {
            // Common VBO for P,C,N,T
            {
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                {
                    //24 * 11 * 4
                    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_PCNT), cube_PCNT, GL_STATIC_DRAW);
                    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(0 * sizeof(float)));
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

                    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

                    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

                    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        glBindVertexArray(0); // Unbind the VAO
    }

    //Depth related code
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // From hear onwards openGL code starts, Tell openGL to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Load Textures */
    if(!loadGLTexture(&textureMarble, MAKEINTRESOURCE(IDBITMAP_MARBLE))){
        fprintf(gpFile, "loadGLTexture Failed to Load Kundali Texture\n");
    }

    perspectiveProjectionMatrix = mat4::identity();

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

        // gluBuild2DMipmaps is Combination of two functions : glTextImage2D and glGenerateMipMap (Used in my terrain example)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, (VOID*)bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_2D);

        //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (VOID*)bmp.bmBits);

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

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void){
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program object
    glUseProgram(shaderProgramObject);
    {
        // Transformations
        mat4 modelMatrix = mat4::identity();
        mat4 viewMatrix = mat4::identity();
        mat4 translationMatrix = mat4::identity();
        mat4 rotationMatrix = mat4::identity();
        mat4 scaleMatrix = mat4::identity();
        {
            translationMatrix = mat4::identity();
            rotationMatrix = mat4::identity();
            scaleMatrix = mat4::identity();

            // Prepare transformation matrices
            translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
            
            rotationMatrix = vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f);

            // Modeview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelMatrix = translationMatrix * rotationMatrix;

            // Pass the model view projection matrix to the shader
            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

            glUniform3f(laUniform, lightAmbient[0], lightAmbient[1], lightAmbient[2]);
            glUniform3f(ldUniform, lightDiffuse[0], lightDiffuse[1], lightDiffuse[2]);
            glUniform3f(lsUniform, lightSpecular[0], lightSpecular[1], lightSpecular[2]);
            glUniform3f(kaUniform, materialAmbient[0], materialAmbient[1], materialAmbient[2]);
            glUniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
            glUniform3f(ksUniform, materialSpecular[0], materialSpecular[1], materialSpecular[2]);
            glUniform1f(materialShininessUniform, materialShininess);
            glUniform4fv(lightPositionUniform, 1, lightPosition);
            glUniform1i(lKeyPressedUniform, bLight ? 1 : 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureMarble);
            glUniform1i(textureSamplerUniform, 0);

            // Bind the VAO
            glBindVertexArray(vao_cube);
            {
                // Draw the Cube
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // front face
                glDrawArrays(GL_TRIANGLE_FAN, 4, 4); // right face
                glDrawArrays(GL_TRIANGLE_FAN, 8, 4); // back face
                glDrawArrays(GL_TRIANGLE_FAN, 12, 4); // left face
                glDrawArrays(GL_TRIANGLE_FAN, 16, 4); // top face
                glDrawArrays(GL_TRIANGLE_FAN, 20, 4); // bottom face
            }
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);

    // Swap the buffers
    SwapBuffers(ghdc);
}

void update(void){
    //code
    angleCube = angleCube + 1.0f;
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

    if(vbo){
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if(vao_cube){
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
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
    if(shaderProgramObject){
        glUseProgram(shaderProgramObject);
        GLint numShaders;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
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

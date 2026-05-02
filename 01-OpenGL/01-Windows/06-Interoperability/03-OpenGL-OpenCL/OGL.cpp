// win32 headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
// custome header files
#include "OGL.h"

// OpenGL related header files
#include <GL/glew.h> // this header file must be included before GL.h ( Why? extensions related info to be included in GL.h)
#include <gl/GL.h>

// // for transformation and projections matrix related maths
#include "vmath.h"
using namespace vmath;

// OpenGL related libraries
#pragma comment(lib, "glew32.lib") // Import library
#pragma comment(lib, "opengl32.lib") // Import library

// OpenCL related headers and lobraries
#include <CL/opencl.h>

#pragma comment(lib, "opencl.lib") // Import library


// OpenGL related global variables
HDC ghdc = NULL;   // handle to device context
HGLRC ghrc = NULL; // handle to graphics library rendering context

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global macro
#define WIN_WIDTH  (800)
#define WIN_HEIGHT (600)


// global variable declarations
// variables related to fullscreen
BOOL gbFullScreen = FALSE;
HWND ghwnd = NULL;
DWORD dwStyle; // could have been local static
WINDOWPLACEMENT wpPrev;

//variables related to fileIO
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// active window related variable
BOOL gbActiveWindow = FALSE;

// esc key related variable
BOOL gbEscKeyIsPressed = FALSE;

// shader related variables
GLuint shaderProgramObject = 0;

enum 
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
};

GLuint vao = 0;
GLuint vbo_cpu = 0;
GLuint vbo_color = 0;
GLuint mvpMatrixUniform = 0;
GLuint fragColorUniform = 0;

mat4 perspectiveProjectionMatrix;

//sine wave related variables

#define MAX_MESH_WIDTH (2048)
#define MAX_MESH_HEIGHT (2048)
#define MIN_MESH_WIDTH (32)
#define MIN_MESH_HEIGHT (32)
#define MESH_DEPTH (4)

unsigned int gMeshWidth = MIN_MESH_WIDTH;
unsigned int gMeshHeight = MIN_MESH_HEIGHT;
unsigned int gMeshDepth = 4;

float position[MAX_MESH_WIDTH*MAX_MESH_HEIGHT*MESH_DEPTH];

GLuint vbo_gpu = 0;
float animationTime = 0.0f;
BOOL bOnGPU = FALSE;

cl_int oclResult;
cl_mem oclGraphicsResource;
cl_device_id oclDeviceID;
cl_context oclContext = NULL;
cl_command_queue oclCommandQueue = NULL;
cl_program oclProgram; 
cl_kernel oclKernel = NULL;



//color related variable nad macros
#define RED_CODE     0
#define GREEN_CODE   1
#define BLUE_CODE    2
#define YELLOW_CODE  3
#define ORANGE_CODE  4
#define PINK_CODE    5
#define BLACK_CODE   6
#define WHITE_CODE   7
#define CYAN_CODE    8
#define BROWN_CODE   9 
#define MAGENTA_CODE 10


GLfloat color[][4] = {{ 1.0f, 0.0f, 0.0f, 1.0f},
                      { 0.0f, 1.0f, 0.0f, 1.0f},
                      { 0.0f, 0.0f, 1.0f, 1.0f},
                      { 1.0f, 1.0f, 0.0f, 1.0f},
                      { 1.0f, 0.74f, 0.0f, 1.0f},
                      { 0.87f, 0.19f, 0.38f, 1.0f},
                      { 0.0f, 0.0f, 0.0f, 1.0f},
                      { 1.0f, 1.0f, 1.0f, 1.0f},
                      { 0.0f, 1.0f, 1.0f, 1.0f},
                      { 0.67f, 0.43f, 0.33f, 1.0f},
                      { 0.5f, 0.0f, 0.5f, 1.0f},
                      { 1.0f, 0.5f, 0.0f, 1.0f}};

unsigned int gColor = ORANGE_CODE;


// entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdLine, int iCmdShow)
{

    // function declarations
    int initialise(void);
    void display(void);
    void uninitialise(void);
    void update(void);

    // variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR6");
    BOOL bDone = FALSE;
    //MONITORINFO mi;

    //code
    // create log file
    gpFile = fopen(gszLogFileName, "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, "Log File Creation Failed", "File IO Error", MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program Started Successfully\n");
    }

    // window class initialisation
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // registration of window class
    RegisterClassEx(&wndclass);

    // create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR 6 - Akash Musale"), 
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                        GetSystemMetrics(SM_CXSCREEN)/2 - WIN_WIDTH/2,
                        GetSystemMetrics(SM_CYSCREEN)/2 - WIN_HEIGHT/2, 
                        WIN_WIDTH, 
                        WIN_HEIGHT, 
                        NULL, 
                        NULL, 
                        hInstance, 
                        NULL);

    ghwnd = hwnd;

    // show window
    ShowWindow(hwnd, iCmdShow);

    // paint background
    UpdateWindow(hwnd);

    // initialise
    int result = initialise();

    if (result != 0)
    {
        fprintf(gpFile, "Initialise function failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "Initialise function completed successfully\n");
    }

    
    // set this window as foreground and active window
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);


    //game loop
    while(bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(gbActiveWindow == TRUE)
            {
                if(gbEscKeyIsPressed == TRUE)
                {
                    bDone = TRUE;
                }
                // render 
                display();

                // update
                update();
            }
        }
    }

    // uninitialise
    uninitialise();

    return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // function declarations
    void ToggleFullScreen(void);
    void resize(int width, int height);
    void uninitialise(void);

    //code 
    switch (iMsg)
    {
    
    case WM_CREATE:
        ZeroMemory((void *)&wpPrev, sizeof(WINDOWPLACEMENT));
        wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

    case WM_CHAR:
        switch(wParam)
        {
            case 'F':
            case 'f':
                if (gbFullScreen == FALSE)
                {
                    ToggleFullScreen();
                    gbFullScreen = TRUE;
                }
                else
                {
                    ToggleFullScreen();
                    gbFullScreen = FALSE;
                }
                break;

            case 'R':
            case 'r':
                gColor = RED_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'G':
            case 'g':
                gColor = GREEN_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'B':
            case 'b':
                gColor = BLUE_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;
            
            case 'Y':
            case 'y':
                gColor = YELLOW_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'O':
            case 'o':
                gColor = ORANGE_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'C':
            case 'c':
                gColor = CYAN_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'M':
            case 'm':
                gColor = MAGENTA_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'K':
            case 'k':
                gColor = BLACK_CODE;
                glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
                break;

            case 'W':
            case 'w':
                gColor = WHITE_CODE;
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                break;

            case 'P':
            case 'p':
                bOnGPU = FALSE;
                break;

            case 'H':
            case 'h':
                bOnGPU = TRUE;
                break;
            
            default:
                break;
        }
        break;
    
    case WM_SETFOCUS:
        gbActiveWindow = TRUE;
        break;

    case WM_KILLFOCUS:
        gbActiveWindow = FALSE;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_ERASEBKGND: // not mandatory, for smooth backgound 
        return (0);
    
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
            case VK_ESCAPE:
                gbEscKeyIsPressed = TRUE;
                break;

            case VK_UP:
                if (gMeshWidth < MAX_MESH_WIDTH)
                {
                    gMeshWidth = gMeshWidth*2;
                }
                if (gMeshHeight < MAX_MESH_HEIGHT)
                {
                    gMeshHeight = gMeshHeight*2;
                }
                break;

            case VK_DOWN:
                if (gMeshWidth > MIN_MESH_WIDTH)
                {
                    gMeshWidth = gMeshWidth/2;
                }
                if (gMeshHeight > MIN_MESH_HEIGHT)
                {
                    gMeshHeight = gMeshHeight/2;
                }
                break;

            default:
                break;   
        }
        break;

    case WM_CLOSE:
        uninitialise();
        break;

    default:
        break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
    // variable declaration
    MONITORINFO mi;
    //code 
    if (gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
           ZeroMemory((void *)&mi, sizeof(MONITORINFO));
           mi.cbSize = sizeof(MONITORINFO);
           if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
           {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & (~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOZORDER | SWP_FRAMECHANGED );
           }
        }
        //optional, hide cursor
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }    
}

int initialise(void)
{
    // function declarations
    void printGLInfo(void);
    void resize(int width, int height);
    void uninitialise(void);

    // variable declaration
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    GLenum glewResult;

    //code
    // pixel format descriptor initialization
    ZeroMemory((void *)&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    //get devixe context
    ghdc = GetDC(ghwnd);
    if (ghdc == NULL)
    {
        fprintf(gpFile, "GetDC() Function failed\n");
        return -1;
    }

    // get matching pixel format index using hdc and pfd
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() Failed\n");
        return -2;
    }

    // select the pixel format of found index
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() function failed\n");
        return -3;
    }

    // create rendering context using hdc, pfd and chosen iPixelFormatIndex
    ghrc = wglCreateContext(ghdc); // wgl* are bridging API
    if (ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed");
        return -4;
    }

    // make this rendering conext as current context
    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed");
        return -5; 
    }

    //initialise glew
    glewResult = glewInit(); //(prototype in glew.h), address is in glew32.lib, text in glew32.dll

    if (glewResult != GLEW_OK)
    {
        fprintf(gpFile, "glewInit() failed");
        return -6;
    }
    // printf GL info
    printGLInfo();

    //from here onwards opengl code starts

    // vertex shader
    // Step 1 Write the shader source code
    const GLchar *vertexShaderSourceCode = 
    "#version 460 core\n" \
    "in vec4 aPosition;\n" \
    "uniform mat4 uMVPMatrix ; \n"
    "out vec4 out_color;\n" \
    "void main(void)" \
    "{\n"\
    "   gl_Position = uMVPMatrix * aPosition;\n" \
    "}\n";

    // Step 2. Create the shader object
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    // Step 3. Give shader source code to shader object
    // second para - numbers of shaders
    // 4th parameter - array of length of every shader
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

    // Step 4. Compile the shader
    glCompileShader(vertexShaderObject);

    // Step 5. Shader compilation error checking
    GLint status = 0;
    GLint infoLogLength = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                // 3rd param, - returned length
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log  = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // fragment shader
    const GLchar *fragmentShaderSourceCode = 
    "#version 460 core\n" \
    "out vec4 FragColor;\n" \
    "uniform vec4 uColor;\n" \
    "void main(void)" \
    "{\n" \
    "   FragColor = uColor;\n" \
    "}\n";

    GLuint fragmentShaderObject =  glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

    glCompileShader(fragmentShaderObject);

    status = 0;
    infoLogLength = 0;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // creade shader program objects
    shaderProgramObject = glCreateProgram();
    
    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObject, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // bind shader attribute at certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");

    // link shader objects to shader program object
    glLinkProgram(shaderProgramObject);


    // check link error
    status = 0;
    infoLogLength = 0;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    //get the required uniform location from the shader
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    fragColorUniform = glGetUniformLocation(shaderProgramObject, "uColor");


    for (unsigned int index = 0; index < sizeof(position)/sizeof(float); index++)
    {
        position[index] = 0;
    }

    // vertex array object for arrays of vertex attributes
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

        //position for cpu
        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_cpu);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_cpu);

        //step3
        glBufferData(GL_ARRAY_BUFFER, sizeof(position), NULL, GL_DYNAMIC_DRAW);

                
        // setp 6. unbind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //position for GPU

        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_gpu);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);

        //step3
        glBufferData(GL_ARRAY_BUFFER, sizeof(position), NULL, GL_DYNAMIC_DRAW);

                
        // setp 6. unbind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // depth related code
    glClearDepth(1.0f); // depth buffer to 1.0
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL); // pass the fragments whose values are less than are equal to glClear depth

    // tell openGl to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // openCL related initialisation
    cl_platform_id oclPlatformID;
    cl_device_id *oclDeviceIDs;
    cl_uint devCount;

    oclResult = clGetPlatformIDs(1, &oclPlatformID, NULL);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "clGetPlatformIDs() failed");
        uninitialise();
        exit(EXIT_FAILURE);
    }

    oclResult = clGetDeviceIDs(oclPlatformID, CL_DEVICE_TYPE_GPU, 0, NULL, &devCount);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "first call to clGetDeviceIDs() failed");
        uninitialise();
        exit(EXIT_FAILURE);
    }
    else if (devCount == 0)
    {
        fprintf(gpFile, "clGetDeviceIDs() returned zero devices");
        uninitialise();
        exit(EXIT_FAILURE);
    }

    //allocate memory to our device id array
    oclDeviceIDs = (cl_device_id *)malloc(sizeof(cl_device_id)*devCount);

    //fill our device id array by calling clGetDeviceIDs again
    oclResult = clGetDeviceIDs(oclPlatformID, CL_DEVICE_TYPE_GPU, devCount, oclDeviceIDs, NULL);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "Second call to clGetDeviceIDs() failed");
        uninitialise();
        exit(EXIT_FAILURE);
    }
    //from above array select zeroth device
    oclDeviceID = oclDeviceIDs[0];

    //initialize open cl context properties array
    cl_context_properties oclContextProperties[] =                     // for linux
    {CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), // CL_GLX_CONTEXT_KHR, (cl_context_properties)glxGetCurrentContext(
     CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),         // CL_GLX_DISPLAY_KHR, (cl_context_properties)wglGetCurrentDisplay()
     CL_CONTEXT_PLATFORM, (cl_context_properties)oclPlatformID,
     0};

    oclContext = clCreateContext(oclContextProperties, 1, &oclDeviceID, NULL, NULL, &oclResult);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "clCreateContext() failed");
        uninitialise();
        exit(EXIT_FAILURE);
    }

    //create ocl command queue
    oclCommandQueue = clCreateCommandQueueWithProperties(oclContext, oclDeviceID, 0, &oclResult);

    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "clCreateCommandQueueWithProperties() failed");
        uninitialise();
        exit(EXIT_FAILURE);
    }

    //write opencl kernel source code

    const char *oclSourceCode = 
        "__kernel void sineWaveKernel(__global float4* pos, unsigned int width, unsigned int height, float time)" \
        "{"\
        "    unsigned int i = get_global_id(0);"\
        "    unsigned int j = get_global_id(1);"\
        "    float u = (float) (i)/ (float) (width);"\
        "    float v = (float) (j)/ (float) (height);"\
        "    u = u*2.0f - 1.0f;"\
        "    v = v*2.0f - 1.0f;"\
        "    float frequency = 4.0f;"\
        "    float w = sin(u * frequency + time) * cos(v * frequency + time) * 0.2;"\
        "    pos[j * width + i] = (float4)(u, w, v, 1.0f);"\
        "}";

    // create OpenCL program using above kernel source code
    oclProgram = clCreateProgramWithSource(oclContext, 1, (const char **)&oclSourceCode, NULL, &oclResult);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile,"clCreateProgramWithSource() Failed : %d\n", oclResult);
        uninitialise();
        exit(EXIT_FAILURE);
    }

    // build OpenCL program 
    oclResult = clBuildProgram(oclProgram, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    if (oclResult != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        fprintf(gpFile, "clBuildProgram() Failed : %d\n", oclResult);
        oclResult = clGetProgramBuildInfo(oclProgram, oclDeviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile,"clGetProgramBuildInfo() Failed : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }
        fprintf(gpFile, "Program Build Log : %s\n", buffer);
        fprintf(gpFile, "clBuildProgram() Failed : %d\n", oclResult);
        uninitialise();
        exit(EXIT_FAILURE);
    }

    // create OpenCL kernel by passing function name that we used in .cl file
    oclKernel = clCreateKernel(oclProgram, "sineWaveKernel", &oclResult);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "clCreateKernel() Failed : %d\n", oclResult);
        uninitialise();
        exit(EXIT_FAILURE);
    }

    //create OpenCL graphics resource
    oclGraphicsResource = clCreateFromGLBuffer(oclContext, CL_MEM_WRITE_ONLY, vbo_gpu, &oclResult);
    if (oclResult != CL_SUCCESS)
    {
        fprintf(gpFile, "clCraeteFromGLBuffer() Failed : %d\n", oclResult);
        uninitialise();
        exit(EXIT_FAILURE);
    }

    // this is analogus to glLoadIdentity in FFP resize
    perspectiveProjectionMatrix = mat4::identity();

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void printGLInfo(void)
{
    // variable declarations
    //GLint numExtensions, i;
    // code
    // printf opengl information
    fprintf(gpFile, "OPEN GL INFORMATION\n");
    fprintf(gpFile, "********************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(gpFile, "********************\n");

   //// get number of extensions
   //glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

   //// print opengl extension
   //fprintf(gpFile, "********************\n");
   //fprintf(gpFile, "Printing %d extensions\n", numExtensions);
   //for (i = 0; i < numExtensions; i++)
   //{
   //    fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
   //}

   //fprintf(gpFile, "********************\n");

}


void resize(int width, int height)
{
    //code
    // if height by accident becomes 0 or less then make height 1
    if (height <= 0)
    {
        height = 1;
    }
    // set the viewport
    glViewport(0, 0, (GLsizei)width, height);

    // create perspective projection matrix
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);

}

void display(void)
{
    // function delcration
    void uninitialise(void);
    // function declaration
    void sineWave(unsigned int mesh_width, unsigned int mesh_height, float time);
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //use shader program object
    glUseProgram(shaderProgramObject);

    // transformations
    mat4 modelViewMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 modelViewProjectionMatrix = mat4::identity();

    //translate
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; // order is important

    // send this matrix to shader in uniform
    // parameter 1. location of uniform
    // parameter 2. number of matrix
    // parameter 3. transpose required?
    // parameter 4.
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    glUniform4fv(fragColorUniform, 1, color[gColor]);

    // bind with vao
    glBindVertexArray(vao);

    //draw sinewave
    if (bOnGPU == TRUE)
    {
        //set 0th Kernel argument
        oclResult = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void *)&oclGraphicsResource);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clSetKernelArg() Failed for 1st Argument : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        oclResult = clSetKernelArg(oclKernel, 1, sizeof(unsigned int), &gMeshWidth);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clSetKernelArg() Failed for 2nd Argument : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        oclResult = clSetKernelArg(oclKernel, 2, sizeof(unsigned int), &gMeshHeight);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clSetKernelArg() Failed for 3rd Argument : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        oclResult = clSetKernelArg(oclKernel, 3, sizeof(unsigned int), &animationTime);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clSetKernelArg() Failed for 4th Argument : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        // aquire OpenCL graphics resource
        oclResult = clEnqueueAcquireGLObjects(oclCommandQueue, 1, &oclGraphicsResource, 0, NULL, NULL);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clEnqueueAcquireGLObjects() Failed : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        //call openCL kernel
        size_t globalWorkSize[2] = {gMeshWidth, gMeshHeight};
        size_t localWorkSize[2] = {8, 8};

        oclResult = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
        if (oclResult != CL_SUCCESS)
        {        
            fprintf(gpFile, "clEnqueueNRRangeKernel() Failed : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

        // release OpenCL graphics resource
        oclResult = clEnqueueReleaseGLObjects(oclCommandQueue, 1, &oclGraphicsResource, 0, NULL, NULL);
        if (oclResult != CL_SUCCESS)
        {
            fprintf(gpFile, "clEnqueueReleaseGLObjects() Failed : %d\n", oclResult);
            uninitialise();
            exit(EXIT_FAILURE);
        }

         // finish the openCl command queues
        clFinish(oclCommandQueue);

        //bind GPU buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);
        // gl buffer data call not required as vbo gpu is already mapped to pPosition

    }
    else
    {
        sineWave(gMeshWidth, gMeshHeight, animationTime);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_cpu);
        glBufferData(GL_ARRAY_BUFFER, gMeshWidth * gMeshHeight * gMeshDepth * sizeof(float), position, GL_DYNAMIC_DRAW);
    }


    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    // step 5. enable the binding point/ target point
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glPointSize(4.0f);


    // draw the vertex arrays 
    // param3 . shader will thrice
    glDrawArrays(GL_POINTS, 0, gMeshWidth * gMeshHeight);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    
    // unbind with vao
    glBindVertexArray(0);

    //unuse shader peogram object
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void sineWave(unsigned int mesh_width, unsigned int mesh_height, float time)
{
    //code
    
    for (unsigned int i = 0; i < mesh_width; i++)
    {
        for (unsigned int j = 0; j < mesh_height; j++)
        {
            float u = (float) (i)/ (float) (mesh_width);
            float v = (float) (j)/ (float) (mesh_height);

            u = u*2.0f - 1.0f;
            v = v*2.0f - 1.0f;
            float frequency = 4.0f;
            float w = sinf(u * frequency + time) * cosf(v * frequency + time) * 0.2;
            unsigned int idx = ((size_t)i * mesh_height + j) * 4;
            position[idx + 0] = u;
            position[idx + 1] = w;
            position[idx + 2] = v;
            position[idx + 3] = 1.0f;
        }
    }

}

void update(void)
{
    //code
    animationTime = animationTime + 0.05;
}

void uninitialise(void)
{
    // function declaration 
    void ToggleFullScreen(void);

    //code
    // if exiting fullscreen restore to normal
    if (gbFullScreen == TRUE)
    {
        ToggleFullScreen();
        gbFullScreen = FALSE;
    }

    // free vbo
    if (vbo_color)
    {
        glDeleteBuffers(1, &vbo_color);
        vbo_color = 0;
    }

    // free vbo
    if (vbo_gpu)
    {
        if (oclGraphicsResource)
        {
            clReleaseMemObject(oclGraphicsResource);
            oclGraphicsResource = NULL;
        }
        glDeleteBuffers(1, &vbo_gpu);
        vbo_gpu = 0;
    }

    if (oclKernel)
    {
        clReleaseKernel(oclKernel);
        oclKernel = NULL;
    }

    if (oclProgram)
    {
        clReleaseProgram(oclProgram);
        oclProgram = NULL;
    }

    
    if (oclCommandQueue)
    {
        clReleaseCommandQueue(oclCommandQueue);
        oclCommandQueue =  NULL;
    }

    if (oclContext)
    {
        clReleaseContext(oclContext);
        oclContext = NULL;
    }

    // free vbo
    if (vbo_cpu)
    {
        glDeleteBuffers(1, &vbo_cpu);
        vbo_cpu = 0;
    }

    // free vao
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
    }

    //detach, delete shader objexts and delete shader prgram objects
    //steps
    // 1. if shader program is still there
    // 2. get number of shaders and continue only if num of shaders is greater than zero
    // 3. create an array to hold shader objects of obtained numbers of shaders
    // 4. get shader objects into this array, continue only if malloc us succeded
    // 5. start a loop for obtained num of shaders, detach and delete every shader
    //6. free the buffer
    // 7. delete the shader program objects
    if (shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
    }



    //make hdc as current conext by releasing releasing renderring context as current context
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // delete the renderring context
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    //release the dc
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
    }

    if(ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // close the log file
    if (gpFile)
    {
        fprintf(gpFile,"Program terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
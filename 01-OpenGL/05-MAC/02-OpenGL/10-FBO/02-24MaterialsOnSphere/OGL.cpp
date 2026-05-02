// win32 headers
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
// custome header files
#include "OGL.h"
#include "Sphere.h"

// OpenGL related header files
#include <GL/glew.h> // this header file must be included before GL.h ( Why? extensions related info to be included in GL.h)
#include <gl/GL.h>

// for transformation and projections matrix related maths
#include "vmath.h"
using namespace vmath;

#include "materials.h"


// OpenGL related libraries
#pragma comment(lib, "glew32.lib") // Import library
#pragma comment(lib, "opengl32.lib") // Import library
#pragma comment(lib, "Sphere.lib")


// OpenGL related global variables
HDC ghdc = NULL;   // handle to device context
HGLRC ghrc = NULL; // handle to graphics library rendering context

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global macro
#define WIN_WIDTH  (800)
#define WIN_HEIGHT (600)

#define FBO_WIDTH  (512)
#define FBO_HEIGHT (512)

// light rotation axis
#define X_AXIS (0)
#define Y_AXIS (1)
#define Z_AXIS (2)
#define NO_AXIS (3)

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
    AMC_ATTRIBUTE_TEXCORDS,
    AMC_ATTRIBUTE_NORMALS
};

GLuint vao_cube = 0;
GLuint vbo_position_cube = 0;
GLuint vbo_texcord_cube = 0;
GLuint mvpMatrixUniform = 0;

mat4 perspectiveProjectionMatrix;

// rotation angles
GLfloat gldAngleCube = 0.0f;

//texture related variables
GLuint textureSamplerUniform;

// FBO related global variables
int winWidth = 0;
int winHeight = 0;
GLuint fbo = 0;
GLuint rbo = 0;
GLuint textureFbo;
int fboResult = -1;

//sphere related variables

GLuint shaderProgramObjectSphere = 0;

GLuint gVao_sphere = 0;
GLuint gVbo_sphere_position = 0;
GLuint gVbo_sphere_normal = 0;
GLuint gVbo_sphere_element = 0;

// light related uniforms
GLuint laUniformSphere = 0;
GLuint ldUniformSphere = 0;
GLuint lsUniformSphere = 0;
GLuint lightPositionUniformSphere = 0;
GLuint LKeyPressedUniformSphere = 0;

//material uniforms
GLuint kaUniformSphere = 0;
GLuint kdUniformSphere = 0;
GLuint ksUniformSphere = 0;
GLuint materialShininessUniformSphere;

// MVP matrix related uniform 
GLuint modelMatrixUniformSphere = 0;
GLuint viewMatrixUniformSphere = 0;
GLuint projectionMatrixUniformSphere = 0;

mat4 perspectiveProjectionMatrixSphere;

//  user control 
BOOL bLightSphere = FALSE;
GLfloat lightAngleSphere = 0.0f;
unsigned int gLightRotationAxisSphere = NO_AXIS;
unsigned int gMaterialIndex = 0;

// light related variables
struct Light
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

struct Light lightSphere;

// sphere material related variables
GLfloat materialAmbientSphere[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialDiffuseSphere[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecularSphere[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininessSphere = 50.0f;

//sphere related variables
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

// entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdLine, int iCmdShow)
{

    // function declarations
    int initialise(void);
    void display(void);
    void uninitialise(void);
    void update(void);
    void updateSphere(void);

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

                updateSphere();
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

            case 'L':
            case 'l':
                bLightSphere = !bLightSphere;
                break;

            case 'X':
            case 'x':
                gLightRotationAxisSphere = X_AXIS;
                break;

            case 'Y':
            case 'y':
                gLightRotationAxisSphere = Y_AXIS;
                break;

            case 'Z':
            case 'z':
                gLightRotationAxisSphere = Z_AXIS;
                break;    

            case 'N':
            case 'n':
                gMaterialIndex = (gMaterialIndex + 1) % 24; 

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
    BOOL createAndPrepareFBOforDrawing(GLint, GLint);
    int initialize_sphere(void);


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
    "in vec2 aTexCord;\n" \
    "uniform mat4 uMVPMatrix;\n" \
    "out vec2 out_texcord;\n" \
    "void main(void)" \
    "{\n"\
    "   gl_Position = uMVPMatrix * aPosition;\n" \
    "   out_texcord = aTexCord;\n" \
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
    "in vec2 out_texcord;\n" \
    "uniform sampler2D uTextureSampler; \n"
    "void main(void)" \
    "{\n" \
    "   FragColor = texture(uTextureSampler, out_texcord);\n" \
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
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCORDS, "aTexCord");

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
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");
      // provide vertex position, color, normal, texcord etc
    const GLfloat cube_position[] = {
        // front
        1.0f,  1.0f,  1.0f, // top-right of front
        -1.0f,  1.0f,  1.0f, // top-left of front
        -1.0f, -1.0f,  1.0f, // bottom-left of front
        1.0f, -1.0f,  1.0f, // bottom-right of front

        // right
        1.0f,  1.0f, -1.0f, // top-right of right
        1.0f,  1.0f,  1.0f, // top-left of right
        1.0f, -1.0f,  1.0f, // bottom-left of right
        1.0f, -1.0f, -1.0f, // bottom-right of right

        // back
        1.0f,  1.0f, -1.0f, // top-right of back
        -1.0f,  1.0f, -1.0f, // top-left of back
        -1.0f, -1.0f, -1.0f, // bottom-left of back
        1.0f, -1.0f, -1.0f, // bottom-right of back

        // left
        -1.0f,  1.0f,  1.0f, // top-right of left
        -1.0f,  1.0f, -1.0f, // top-left of left
        -1.0f, -1.0f, -1.0f, // bottom-left of left
        -1.0f, -1.0f,  1.0f, // bottom-right of left

        // top
        1.0f,  1.0f, -1.0f, // top-right of top
        -1.0f,  1.0f, -1.0f, // top-left of top
        -1.0f,  1.0f,  1.0f, // bottom-left of top
        1.0f,  1.0f,  1.0f, // bottom-right of top

        // bottom
        1.0f, -1.0f,  1.0f, // top-right of bottom
        -1.0f, -1.0f,  1.0f, // top-left of bottom
        -1.0f, -1.0f, -1.0f, // bottom-left of bottom
        1.0f, -1.0f, -1.0f, // bottom-right of bottom
    };

    const GLfloat cube_texcords[] = {
        // front
        1.0f, 1.0f, // top-right of front
        0.0f, 1.0f, // top-left of front
        0.0f, 0.0f, // bottom-left of front
        1.0f, 0.0f, // bottom-right of front

        // right
        1.0f, 1.0f, // top-right of right
        0.0f, 1.0f, // top-left of right
        0.0f, 0.0f, // bottom-left of right
        1.0f, 0.0f, // bottom-right of right

        // back
        1.0f, 1.0f, // top-right of back
        0.0f, 1.0f, // top-left of back
        0.0f, 0.0f, // bottom-left of back
        1.0f, 0.0f, // bottom-right of back

        // left
        1.0f, 1.0f, // top-right of left
        0.0f, 1.0f, // top-left of left
        0.0f, 0.0f, // bottom-left of left
        1.0f, 0.0f, // bottom-right of left

        // top
        1.0f, 1.0f, // top-right of top
        0.0f, 1.0f, // top-left of top
        0.0f, 0.0f, // bottom-left of top
        1.0f, 0.0f, // bottom-right of top

        // bottom
        1.0f, 1.0f, // top-right of bottom
        0.0f, 1.0f, // top-left of bottom
        0.0f, 0.0f, // bottom-left of bottom
        1.0f, 0.0f, // bottom-right of bottom
    };


    // vertex array object for arrays of vertex attributes
    glGenVertexArrays(1, &vao_cube);

    glBindVertexArray(vao_cube);

        //position
        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_position_cube);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);

        //step3
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);

        // step4 tell GPU how to use buffer data
        // parameter 1. bind index
        // parameter 2. size - of vertex
        // parameter 3. Data type of vertex
        // paremeter 4. Normalised (is data normalised?)
        // parameter 5. stride 
        // paremeter 6. pointer only applicable if stride is provided else NULL
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        // step 5. enable the binding point/ target point
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        //texcords
        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_texcord_cube);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord_cube);

        //step3
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcords), cube_texcords, GL_STATIC_DRAW);

        // step4 tell GPU how to use buffer data
        // parameter 1. bind index
        // parameter 2. size - of vertex
        // parameter 3. Data type of vertex
        // paremeter 4. Normalised (is data normalised?)
        // parameter 5. stride 
        // paremeter 6. pointer only applicable if stride is provided else NULL
        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCORDS, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        // step 5. enable the binding point/ target point
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCORDS);

    // unbind the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // depth related code
    glClearDepth(1.0f); // depth buffer to 1.0
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL); // pass the fragments whose values are less than are equal to glClear depth
 
    // tell openGl to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //enable texture
    glEnable(GL_TEXTURE_2D);

    // this is analogus to glLoadIdentity in FFP resize
    perspectiveProjectionMatrix = mat4::identity();

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    //create FBO and call initialise speher on success

    if (createAndPrepareFBOforDrawing(FBO_WIDTH, FBO_HEIGHT) == TRUE)
    {
        fprintf(gpFile, "FBO creation successfull\n");
        fboResult = initialize_sphere();
        if (fboResult != 0)
        {
            fprintf(gpFile, "initialize_sphere failed\n");
            return -10;
        }
        else
        {
            fprintf(gpFile, "initialize_sphere succesfull\n");
        }
    }
    else
    {
        fprintf(gpFile, "createAndPrepareFBOforDrawing failed\n");
        return -11;
    }

    return 0;
}

void printGLInfo(void)
{
    // variable declarations
    GLint numExtensions, i;
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

BOOL createAndPrepareFBOforDrawing(GLint textureWidth, GLint textureHeight)
{
    //variable declaration

    //code 
    //step 1
    // check whether texture width and texture height parameters are compatible
    GLint maxRenderedBufferSize;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderedBufferSize);

    if ((maxRenderedBufferSize < textureWidth) || (maxRenderedBufferSize < textureHeight))
    {
        fprintf(gpFile, "fbo width/height exceeding maxRenderedBufferSize\n");
        return FALSE;
    }

    //step 2
    // create new frame buffer for our sphere
    glGenFramebuffers(1, &fbo);

    //step 3
    // bind with newly created framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //step 4
    //create render buffer as placeholder for depth buffer for our framebuffer
    glGenRenderbuffers(1, &rbo);

    //step 5
    //bind with newly created render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    //step 6
    // give specific storage to this render buffer so depth needed fpr sphere will reside here
    // it is recommanded to have 16 bit depth for mobiles
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    //step 7
    //create empty fully capable and compatible texture object
    glGenTextures(1, &textureFbo);
    glBindTexture(GL_TEXTURE_2D, textureFbo);

    //step 8
    //to create full featured texture give appropriate parameters to the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //step 9
    //create this texture for our FBO
    // last null indicates that this empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

    //step 10
    glGenerateMipmap(GL_TEXTURE_2D);

    //step 11
    // give above empty texture to framebuffer
    /*
        param 1 target - Must be GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, or GL_READ_FRAMEBUFFER.
        param 2 attachment → Which part of the FBO you’re attaching to:

                            GL_COLOR_ATTACHMENT0 + i (for multiple color buffers)
                            GL_DEPTH_ATTACHMENT
                            GL_STENCIL_ATTACHMENT
                            GL_DEPTH_STENCIL_ATTACHMENT

        param 3 textarget → The texture target type: GL_TEXTURE_2D (most common)
        param 4 texture → The texture object’s name (ID).
        param 5 level → Mipmap level of the texture to attach (usually 0).
    */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFbo, 0);

    //step 12
    // attach previously created depth render buffer to our framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //step 13
    // check whether our framebuffer is complete or not
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        
        fprintf(gpFile, "fbo creation is incomplete\n");
        return FALSE;
    }

    //unbind with newly created framebuffer, we will use it when needed
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return TRUE;
}

int initialize_sphere(void)
{

    // function declaration 
    void resize_sphere(int, int);
    // vertex shader
    // Step 1 Write the shader source code
    // lightSource from diffused renamed as lightDirection in fong light
    // eyeCoordinates.xyz is called as swizzeling, we can also do normalize(vec3(eyeCordinate))
    const GLchar *vertexShaderSourceCode = 
    "#version 460 core\n" \
    "in vec4 aPosition;\n" \
    "in vec3 aNormal;\n" \
    "out vec3 out_transformedNormals;\n" \
    "out vec3 out_lightDirection;\n" \
    "out vec3 out_viewerVector;\n" \
    "uniform mat4 uModelMatrix;\n" \
    "uniform mat4 uViewMatrix;\n" \
    "uniform mat4 uProjectionMatrix;\n" \
    "uniform vec4 uLightPosition;\n" \
    "uniform int uLkeyPressed;\n" \

    "void main(void)" \
    "{\n"\
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
    "       mat3 normalMatrix = mat3(uViewMatrix * uModelMatrix);\n" \
    "       vec4 uLightPositionTransformed = uModelMatrix * uLightPosition;\n" \
    "       out_transformedNormals = (normalMatrix * aNormal);\n" \
    "       out_viewerVector = (-eyeCoordinates.xyz);\n"\
    "       out_lightDirection = (vec3(uLightPositionTransformed) - eyeCoordinates.xyz);\n" \
    "   }\n"\
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
    "in vec3 out_transformedNormals;\n" \
    "in vec3 out_lightDirection;\n" \
    "in vec3 out_viewerVector;\n" \
    "uniform vec3 uLa;\n" \
    "uniform vec3 uLd;\n" \
    "uniform vec3 uLs;\n" \
    "uniform vec3 uKa;\n" \
    "uniform vec3 uKd;\n" \
    "uniform vec3 uKs;\n" \
    "uniform float uMaterialShininess;\n" \
    "uniform int uLkeyPressed;\n" \
    "vec3 fong_ads_light;\n" \

    "void main(void)" \
    "{\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
    "       vec3 normalized_transformedNormals = normalize(out_transformedNormals);\n" \
    "       vec3 normalized_viewerVector = normalize(out_viewerVector);\n"
    "       fong_ads_light = vec3(0.0f, 0.0f, 0.0f);\n" \
    "       vec3 normalized_lightDirection = normalize(out_lightDirection);\n"
    "       vec3 reflectionVector = reflect(-normalized_lightDirection, normalized_transformedNormals);\n"
    "       vec3 diffusedLight = uLd * uKd * max(dot(normalized_lightDirection, normalized_transformedNormals), 0.0);\n"
    "       vec3 ambientLight = uLa * uKa;\n"
    "       vec3 specularLight = uLs * uKs * pow(max(dot(reflectionVector, normalized_viewerVector),0.0), uMaterialShininess);\n"
    "       fong_ads_light += ambientLight + diffusedLight + specularLight;\n"
    "   }\n" \
    "   else \n"\
    "   {\n"\
    "       fong_ads_light = vec3(1.0f, 1.0f, 1.0f);\n"
    "   }\n" \
    "   FragColor = vec4(fong_ads_light, 1.0f);\n" \
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
    shaderProgramObjectSphere = glCreateProgram();
    
    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObjectSphere, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObjectSphere, fragmentShaderObject);

    // bind shader attribute at certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObjectSphere, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObjectSphere, AMC_ATTRIBUTE_NORMALS, "aNormal");

    // link shader objects to shader program object
    glLinkProgram(shaderProgramObjectSphere);


    // check link error
    status = 0;
    infoLogLength = 0;

    glGetProgramiv(shaderProgramObjectSphere, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObjectSphere, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObjectSphere, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    //get the required uniform location from the shader
    modelMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uModelMatrix");
    viewMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uViewMatrix");
    projectionMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uProjectionMatrix");

    laUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLa");
    ldUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLd");
    lsUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLs");
    lightPositionUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLightPosition");

    kaUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKa");
    kdUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKd");
    ksUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKs");
    materialShininessUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uMaterialShininess");

    LKeyPressedUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLkeyPressed");

    // provide vertex position, color, normal, texcord etc
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    // vertex array object for arrays of vertex attributes
    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);

    // position vbo
    glGenBuffers(1, &gVbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // normal vbo
    glGenBuffers(1, &gVbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMALS);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // element vbo
    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);

    // depth related code
    glClearDepth(1.0f); // depth buffer to 1.0
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL); // pass the fragments whose values are less than are equal to glClear depth
 
    // tell openGl to choose the color to clear the screen
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // this is analogus to glLoadIdentity in FFP resize
    perspectiveProjectionMatrix = mat4::identity();

    lightSphere.ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lightSphere.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightSphere.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightSphere.position = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // warm up resize
    resize_sphere(FBO_WIDTH, FBO_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    //code
    // if height by accident becomes 0 or less then make height 1
    if (height <= 0)
    {
        height = 1;
    }

    winWidth = width;
    winHeight = height;

    // set the viewport
    glViewport(0, 0, (GLsizei)width, height);

    // create perspective projection matrix
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);
}

void resize_sphere(int width, int height)
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
    perspectiveProjectionMatrixSphere = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);
}


void display(void)
{
    // function declaration
    void resize(int, int);
    void display_sphere(void);

    //code
    //call spehere related code
    if (fboResult == 0)
    {
        // initialise sphere executed successfully
        display_sphere();
    }
    //call cubes resize to compensate spheres resize
    resize(winWidth, winHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //use shader program object
    glUseProgram(shaderProgramObject);

    // transformations
    mat4 modelViewMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 translationMatrix = mat4::identity();

    mat4 rotationMatrixX = mat4::identity();
    mat4 rotationMatrixY = mat4::identity();
    mat4 rotationMatrixZ = mat4::identity();
    mat4 modelViewProjectionMatrix = mat4::identity();
    mat4 scaleMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
    scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
    rotationMatrixX = vmath::rotate(gldAngleCube, 1.0f, 0.0f, 0.0f);
    rotationMatrixY = vmath::rotate(gldAngleCube, 0.0f, 1.0f, 0.0f);
    rotationMatrixZ = vmath::rotate(gldAngleCube, 0.0f, 0.0f, 1.0f);
    //modelViewMatrix = rotationMatrix * translationMatrix;// order is important
    modelViewMatrix = translationMatrix * rotationMatrixX * rotationMatrixY *rotationMatrixZ * scaleMatrix; // order is important
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; // order is important

    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    // for texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureFbo);
    // parameter 2. index of the sampler
    glUniform1i(textureSamplerUniform, 0);
    // bind with vao_rectangle
    glBindVertexArray(vao_cube);

    // draw the vertex arrays 
    // param3 . shader will thrice
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    // unbind with vao_cube
    glBindVertexArray(0);

    //unuse shader peogram object
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void display_sphere(void)
{
 
    // function declaration
    void resize_sphere(int, int);

    //code
    if (fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // call spheres resize
        resize_sphere(FBO_WIDTH, FBO_HEIGHT);
        // call clear color of sphere
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else
    {
        fprintf(gpFile, "fbo invalid\n");
    }

    // fbo related changes
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // transformations
    mat4 modelMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 viewMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 translationMatrix = mat4::identity();

    mat4 lightTranslationMatrix = mat4::identity();
    mat4 scaleMatrix = mat4::identity();



    switch (gLightRotationAxisSphere)
    {
        case X_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 1.0f, 0.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 20.0f, 20.0f);
            break;

        case Y_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 0.0f, 1.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 0.0f, 20.0f);
            break;

        case Z_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 20.0f, 0.0f);
            break;
    
        default:
            lightTranslationMatrix = vmath::rotate(0.0f, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 0.0f, 20.0f);
            break;
    }


    //use per fragment shader program object
    glUseProgram(shaderProgramObjectSphere);

    glUniformMatrix4fv(viewMatrixUniformSphere, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniformSphere, 1, GL_FALSE, perspectiveProjectionMatrixSphere);

    glUniform1i(LKeyPressedUniformSphere, bLightSphere);

    glUniform3fv(laUniformSphere, 1, lightSphere.ambient);
    glUniform3fv(ldUniformSphere, 1, lightSphere.diffuse);
    glUniform3fv(lsUniformSphere, 1, lightSphere.specular);
    glUniform4fv(lightPositionUniformSphere, 1, lightSphere.position * lightTranslationMatrix.transpose());


    glUniform3fv(kaUniformSphere, 1, materialAmbientSphere);
    glUniform3fv(kdUniformSphere, 1, materialDiffuseSphere);
    glUniform3fv(ksUniformSphere, 1, materialSpecularSphere);
    glUniform1f(materialShininessUniformSphere, materialShininessSphere);

    // *** bind vao ***
    glBindVertexArray(gVao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);


    translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
    modelMatrix = translationMatrix;
    glUniformMatrix4fv(modelMatrixUniformSphere, 1, GL_FALSE, modelMatrix);
    glUniform3fv(kaUniformSphere, 1, sphereMaterials[gMaterialIndex].ambient);
    glUniform3fv(kdUniformSphere, 1, sphereMaterials[gMaterialIndex].diffuse);
    glUniform3fv(ksUniformSphere, 1, sphereMaterials[gMaterialIndex].specular);
    glUniform1f(materialShininessUniformSphere, sphereMaterials[gMaterialIndex].shininess);
    
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    // *** unbind vao ***
    glBindVertexArray(0);

    //unuse shader peogram object
    glUseProgram(0);

    if (fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void update(void)
{
    //code

    gldAngleCube = gldAngleCube - 1.0f;

}

void updateSphere(void)
{
    //code
    lightAngleSphere = lightAngleSphere + 2.0f;

}

void uninitialise(void)
{
    // function declaration 
    void ToggleFullScreen(void);
    void uninitialise_sphere(void);


    //code
    if (fboResult == 0)
    {
        uninitialise_sphere();
    }
    // if exiting fullscreen restore to normal
    if (gbFullScreen == TRUE)
    {
        ToggleFullScreen();
        gbFullScreen = FALSE;
    }

    
    if (textureFbo)
    {
        glDeleteTextures(1, &textureFbo);
        textureFbo = 0;
    }

   
    // free vbo
    if (vbo_texcord_cube)
    {
        glDeleteBuffers(1, &vbo_texcord_cube);
        vbo_texcord_cube = 0;
    }

    if (vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    // free vao_reactangle
    if (vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
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

void uninitialise_sphere(void)
{

    if(rbo)
    {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }

    if(fbo)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    // free vbo
    if (gVbo_sphere_position)
    {
        glDeleteBuffers(1, &gVbo_sphere_position);
        gVbo_sphere_position = 0;
    }
    
    if (gVbo_sphere_normal)
    {
        glDeleteBuffers(1, &gVbo_sphere_normal);
        gVbo_sphere_normal = 0;
    }

        
    if (gVbo_sphere_element)
    {
        glDeleteBuffers(1, &gVbo_sphere_element);
        gVbo_sphere_element = 0;
    }


    // free vao
    if (gVao_sphere)
    {
        glDeleteVertexArrays(1, &gVao_sphere);
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
    if (shaderProgramObjectSphere)
    {
        glUseProgram(shaderProgramObjectSphere);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObjectSphere, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObjectSphere, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObjectSphere, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObjectSphere);
    }
}
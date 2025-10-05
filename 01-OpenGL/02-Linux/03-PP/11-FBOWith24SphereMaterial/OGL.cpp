// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

//Xlib header files
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

// OpenGL related header files
#include <GL/glew.h> 
#include <GL/gl.h>
#include <GL/glx.h>

#include "vmath.h"
using namespace vmath;

#include "Sphere.h"
#include "material.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define NO_OF_LIGHTS 3

// Macros for FBO
#define FBO_WIDTH 512
#define FBO_HEIGHT 512

Display *gpDisplay = NULL;
XVisualInfo* gXVisualInfo = NULL; // Like device context in Windows
Window window;
Colormap colormap;

Bool gbFullScreen = False;
Bool bActiveWindow = False;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gGLXFBConfig;

// OpenGL related variables
GLXContext glxContext = NULL;

/* Variable related to File I/O */
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// Shader related variables
GLuint shaderProgramObject_cube = 0;

enum {
    AMC_ATTRIBUTE_POSITION_CUBE = 0,
    AMC_ATTRIBUTE_TEXCOORD_CUBE
};

GLuint vao_cube = 0;
GLuint vbo_position_cube = 0;
GLuint vbo_texcoord_cube = 0;

GLuint mvpMatrixUniform_cube = 0;

mat4 perspectiveProjectionMatrix_cube;

/* For Texture */
GLuint textureSamplerUniform;

/* Rotation angle variables */
GLfloat angleCube = 0.0f;

// What to render on FBO : Three Moving Lights on Steady Sphere
GLuint pf_shaderProgramObject_sphere = 0;

enum {
    AMC_ATTRIBUTE_POSITION_SPHERE = 0,
    AMC_ATTRIBUTE_NORMAL_SPHERE,
};

GLuint gVao_sphere = 0;
GLuint gVbo_sphere_position = 0;
GLuint gVbo_sphere_normal = 0;
GLuint gVbo_sphere_element = 0;

mat4 perspectiveProjectionMatrix_sphere;

GLuint pf_modelMatrixUniform_sphere = 0;
GLuint pf_viewMatrixUniform_sphere = 0;
GLuint pf_projectionMatrixUniform_sphere = 0;

GLuint pf_laUniform_sphere = 0;               // Light Ambient
GLuint pf_ldUniform_sphere = 0;               // Light Diffuse
GLuint pf_lsUniform_sphere = 0;               // Light Specular
GLuint pf_lightPositionUniform_sphere = 0;    // Light Position

GLuint pf_kaUniform_sphere = 0;               // Material Ambient
GLuint pf_ksUniform_sphere = 0;               // Material Specular
GLuint pf_kdUniform_sphere = 0;               // Material Diffuse
GLuint pf_materialShininessUniform_sphere = 0; // Material Shininess

GLuint pf_lKeyPressedUniform_sphere = 0;      // Light Key Pressed

GLfloat lightAmbient_sphere[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightDiffuse_sphere[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular_sphere[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition_sphere[] = {0.0f, 0.0f, 0.0f, 1.0f};

Bool bLight_sphere = False; // Light On/Off

/* Sphere related variables */
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices = 0;
unsigned int gNumElements = 0;

GLfloat angleForXRotation_sphere = 0.0f;
GLfloat angleForYRotation_sphere = 0.0f;
GLfloat angleForZRotation_sphere = 0.0f;
GLint keyPressed_sphere = -1;
GLint materialChoice = 0;

// FBO related variables
int winWidth;
int winHeight;
GLuint fbo; // Frame Buffer Object
GLuint rbo; // Render Buffer Object
GLuint fboTexture; // Texture attached to FBO
int fboResult = -1; // To check FBO completeness

int main(void){
    // Function declarations
    int  initialize(void);
    void resize(int, int);
    void display(void);
    void update(void);
    void toggleFullScreen(void);
    void uninitialize(void);

    // Variable declarations
    int defaultScreen;
    int defaultDepth;
    XSetWindowAttributes windowAttributes;
    Atom windowManagerDeleteAtom;
    XEvent event;

    Screen* screen = NULL;
    int screenWidth, screenHeight;

    KeySym keySym;
    char keys[26]; // Although we need only zeroth index, conventionally the array size = number of alphabets

    GLXFBConfig *pglxFBConfigs = NULL;
    GLXFBConfig bestGLXFBConfig;
    XVisualInfo *pxVisualInfo = NULL;
    int numFBConfigs = 0;
    
    //Add Programmable Pipeline related Frame Buffer attributes into FrameBuffer attributes array.
    int frameBufferAtrributes[] = {
                                    GLX_X_RENDERABLE, True,             //PP
                                    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,  //PP
                                    GLX_RENDER_TYPE, GLX_RGBA_BIT,      //PP
                                    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,  //PP
                                    GLX_DOUBLEBUFFER, True,             //PP
                                    GLX_RED_SIZE, 8,
                                    GLX_GREEN_SIZE, 8,
                                    GLX_BLUE_SIZE, 8,
                                    GLX_ALPHA_SIZE, 8,
                                    GLX_DEPTH_SIZE, 24,
                                    GLX_STENCIL_SIZE, 8,
                                    None
                                };
    Bool bDone = False;

    // Create Log File
    gpFile = fopen(gszLogFileName, "w");
    if(gpFile == NULL){
        fprintf(gpFile, "Log File Creation Failed");
        exit(0);
    }
    else{
        fprintf(gpFile, "Program Started Successfully!\n");
        fflush(gpFile);
    }

    // Open the connection to the X server
    gpDisplay = XOpenDisplay(NULL);
    if (gpDisplay == NULL) {
        fprintf(gpFile, "ERROR: Unable to open X display.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // Create the default screen object
    defaultScreen = XDefaultScreen(gpDisplay);

    // Get default depth
    defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);

    //Accordingly find FBConfigs supporting these FrameBuffer attributes.
    pglxFBConfigs = glXChooseFBConfig(gpDisplay, defaultScreen, frameBufferAtrributes, &numFBConfigs);
    fprintf(gpFile, "Found number of FBConfigs : %d.\n", numFBConfigs);
    if (pglxFBConfigs == NULL || numFBConfigs == 0) {
        fprintf(gpFile, "ERROR: Unable to get framebuffer configurations.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    int indexOfBestFBConfig = -1, indexOfWorstFBConfig = -1;
    int bestNoOfSamples = -1, worstNoOfSamples = 999;

    // Loop through the FBConfigs to find the best one
    /* 
        From above multiple FBConfigs, find that one which has :
        a) Matching visualInfo
        b) Sample buffers to store samples
        c) More than 0 samples
        We will call this FBConfig as best FB config
    */
    for (int i = 0; i < numFBConfigs; i++) {
        pxVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pglxFBConfigs[i]);
        if(pxVisualInfo) {
            int sampleBuffers, samples;
            glXGetFBConfigAttrib(gpDisplay, pglxFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
            glXGetFBConfigAttrib(gpDisplay, pglxFBConfigs[i], GLX_SAMPLES, &samples);
            fprintf(gpFile, "FBConfig %d: Sample Buffers = %d, Samples = %d\n", i, sampleBuffers, samples);
            if (indexOfBestFBConfig < 0 || sampleBuffers && samples > bestNoOfSamples) {
                indexOfBestFBConfig = i;
                bestNoOfSamples = samples;
            }
            if(indexOfWorstFBConfig < 0 || !sampleBuffers || samples < worstNoOfSamples) {
                indexOfWorstFBConfig = i;
                worstNoOfSamples = samples;
            }
            XFree(pxVisualInfo);
        }
    }

    fprintf(gpFile, "Best FBConfig Index: %d with %d samples.\n", indexOfBestFBConfig, bestNoOfSamples);
    bestGLXFBConfig = pglxFBConfigs[indexOfBestFBConfig];
    gGLXFBConfig = bestGLXFBConfig;

    XFree(pglxFBConfigs);

    gXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, gGLXFBConfig);

    //FFP Way of getting visual infos
    /*
    status = XMatchVisualInfo(gpDisplay, defaultScreen, defaultDepth, TrueColor, gXVisualInfo); 
    gXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAtrributes);
    if (!gXVisualInfo) {
        fprintf(gpFile, "ERROR: Unable to get visual info.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }
    */

    // Set the window attributes
    memset(&windowAttributes, 0, sizeof(windowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixmap = 0;
    windowAttributes.background_pixel = XBlackPixel(gpDisplay, gXVisualInfo->screen);
    windowAttributes.colormap = XCreateColormap(gpDisplay, XRootWindow(gpDisplay, gXVisualInfo->screen), gXVisualInfo->visual, AllocNone);
    windowAttributes.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | FocusChangeMask; // PointerMotionMask is for mouse motion events
    colormap = windowAttributes.colormap;

    // Create the window
    window = XCreateWindow(gpDisplay, XRootWindow(gpDisplay, gXVisualInfo->screen),
                            0, 0, WIN_WIDTH, WIN_HEIGHT, 0,
                            gXVisualInfo->depth, InputOutput,
                            gXVisualInfo->visual,
                            CWBorderPixel | CWBackPixel | CWEventMask | CWColormap,
                            &windowAttributes);
    if(!window) {
        fprintf(gpFile, "ERROR: Unable to create window.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // Create atom for window manager delete message to destroy the window
    windowManagerDeleteAtom = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);

    // Set the WM_DELETE_WINDOW protocol
    XSetWMProtocols(gpDisplay, window, &windowManagerDeleteAtom, 1);

    // Set window title
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - FBOWith24SphereMaterial");

    // Map the window to screen to show it
    XMapWindow(gpDisplay, window);

    // Centering the window
    // Changing of x and y in XCreateWindow like we did in win32 will not work here, because it is managed by the window manager.
    // So let the window manager decide the position, and then center it manually using API .
    screen          = XScreenOfDisplay(gpDisplay, gXVisualInfo->screen);
    screenWidth     = XWidthOfScreen(screen);
    screenHeight    = XHeightOfScreen(screen);
    XMoveWindow(gpDisplay, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);

    // Initialize
    int iResult = initialize();
    
    if(iResult == -1){
        uninitialize();
        exit(EXIT_FAILURE);
    }

    fprintf(gpFile, "Initialization successful\n");

    // Game loop
    while (bDone == False) {
        while (XPending(gpDisplay)) {
        // Wait for an event
        XNextEvent(gpDisplay, &event);

        // Handle the event
        switch (event.type) {
            case MapNotify:
                fprintf(gpFile, "Window mapped\n");
            break;
            
            case FocusIn:
                fprintf(gpFile, "Window focused\n"); 
                bActiveWindow = True;
                break;

            case FocusOut:
                fprintf(gpFile, "Window unfocused\n");
                bActiveWindow = False;
            break;

            case ConfigureNotify:
                fprintf(gpFile, "Window resized or moved: %dx%d at (%d, %d)\n", event.xconfigure.width, event.xconfigure.height, event.xconfigure.x, event.xconfigure.y);
                resize(event.xconfigure.width, event.xconfigure.height);
            break;

            case KeyPress:
                keySym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                switch (keySym){
                    case XK_Escape:
                        gbFullScreen = !gbFullScreen;
                        toggleFullScreen();
                    break;                  
                    
                    default:
                        break;
                }

                XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);

                switch(keys[0]){
                    case 'Q':
                    case 'q':
                        bDone = True;
                    break;

                    case 'L':
                    case 'l':
                        bLight_sphere = !bLight_sphere;
                    break;

                    case 'X':
                    case 'x':
                        angleForXRotation_sphere = 0.0f;
                        keyPressed_sphere = 1;
                    break;

                    case 'Y':
                    case 'y':
                        angleForYRotation_sphere = 0.0f;
                        keyPressed_sphere = 2;
                    break;

                    case 'Z':
                    case 'z':
                        angleForZRotation_sphere = 0.0f;
                        keyPressed_sphere = 3;
                    break;

                    case 'N':
                    case 'n':
                        materialChoice = (materialChoice + 1) % 24;
                    break;
                }
            break;

            case ButtonPress:
                switch(event.xbutton.button) {
                    case Button1:
                        fprintf(gpFile, "Left button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    case Button2:
                        fprintf(gpFile, "Middle button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    case Button3:
                        fprintf(gpFile, "Right button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    default:
                        fprintf(gpFile, "Other button %d clicked at (%d, %d)\n", event.xbutton.button, event.xbutton.x, event.xbutton.y);
                    break;
                }
                break;

            case Expose:
                fprintf(gpFile, "Window exposed\n");
            break; 

            case ClientMessage:
                bDone = True;
                exit(EXIT_SUCCESS);
            break;

            default:
                break;
            }
        
        }

        // rendering
        if(bActiveWindow){
            display();
            update();
        }
    }

    uninitialize();
    return 0;
}

void toggleFullScreen(void){
    Atom windowManagerNormalStateAtom = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    Atom windowManagerFullScreenStateAtom = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent event;
    memset((void*)&event, 0, sizeof(XEvent));
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = windowManagerNormalStateAtom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = gbFullScreen ? 1 : 0; // 1 for fullscreen, 0 for normal
    event.xclient.data.l[1] = windowManagerFullScreenStateAtom;
    
    XSendEvent(gpDisplay, 
            XRootWindow(gpDisplay, gXVisualInfo->screen),
            False, 
            SubstructureNotifyMask, 
            &event);

    XFlush(gpDisplay);
    fprintf(gpFile, "Toggled fullscreen mode: %s\n", gbFullScreen ? "ON" : "OFF");
}

int  initialize(void){
    void printGLInfo(void);
    Bool createAndPrepareFBOForDrawing(GLint, GLint);
    int initializeSphere(void);

    GLenum glewResult;

    //Get function pointer for glXCreateContextAttribsARB
    glXCreateContextAttribsARB = 
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    if (!glXCreateContextAttribsARB) {
        fprintf(gpFile, "ERROR: glXCreateContextAttribsARB not found.\n");
        return -1;
    }

    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    glxContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, NULL, True, contextAttribs);
    if (glxContext == NULL) {
        int contextAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            None
        };
        glxContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, NULL, True, contextAttribs);
        fprintf(gpFile, "Cannot get openGL Context 4.6, but received lesser context.\n");
    }
    else{
        fprintf(gpFile, "Received openGL Context 4.6.\n");
    }

    if (!glXMakeCurrent(gpDisplay, window, glxContext)) {
        fprintf(gpFile, "ERROR: glXMakeCurrent failed.\n");
        return -1;
    }

    //FFP way of getting gl Context
    /*
    glxContext = glXCreateContext(gpDisplay, gXVisualInfo, NULL, True);
    if(glxContext == NULL){
        fprintf(gpFile, "ERROR: glXCreateContext Failed.\n");
        return -1;
    }

    glXMakeCurrent(gpDisplay, window, glxContext);
    */

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
        "in vec4 aPosition;\n" \
        "in vec2 aTexCoord;\n" \
        "out vec2 out_texCoord;\n" \
        "uniform mat4 uMVPMatrix;\n" \
        "void main(void)\n" \
        "{\n" \
        "gl_Position = uMVPMatrix * aPosition;\n" \
        "out_texCoord = aTexCoord;\n" \
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
        "out vec4 FragColor;\n" \
        "in vec2 out_texCoord;\n" \
        "uniform sampler2D uTextureSampler;\n" \
        "void main(void)\n" \
        "{\n" \
        "FragColor = texture(uTextureSampler, out_texCoord);\n" \
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
    shaderProgramObject_cube = glCreateProgram();
    if(shaderProgramObject_cube == 0){
        fprintf(gpFile, "glCreateProgram failed\n");
        return -9;
    }

    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObject_cube, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObject_cube, fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject_cube, AMC_ATTRIBUTE_POSITION_CUBE, "aPosition");
    glBindAttribLocation(shaderProgramObject_cube, AMC_ATTRIBUTE_TEXCOORD_CUBE, "aTexCoord");

    // Link the shader program and check for errors
    glLinkProgram(shaderProgramObject_cube);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(shaderProgramObject_cube, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(shaderProgramObject_cube, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(shaderProgramObject_cube, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }

    // Get the required uniform locations from the shader program object
    mvpMatrixUniform_cube = glGetUniformLocation(shaderProgramObject_cube, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject_cube, "uTextureSampler");

    // Provide vertex position, color, texture coordinates, normals, etc. to the shader program object
    
    // Cube
    {
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
        const GLfloat cube_texcoord[] = {
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

        glGenVertexArrays(1, &vao_cube);
        glBindVertexArray(vao_cube);
        {
            // Position VBO
            {
                glGenBuffers(1, &vbo_position_cube);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
                    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION_CUBE, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION_CUBE);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            // TexCoord VBO
            {
                glGenBuffers(1, &vbo_texcoord_cube);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_cube);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoord), cube_texcoord, GL_STATIC_DRAW);
                    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD_CUBE, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD_CUBE);
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
    glEnable(GL_TEXTURE_2D);

    // From hear onwards openGL code starts, Tell openGL to choose the color to clear the screen
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    perspectiveProjectionMatrix_cube = mat4::identity();

    //Create FBO, if successfull call initializeSphere
    if(createAndPrepareFBOForDrawing(FBO_WIDTH, FBO_HEIGHT) == True){
        fprintf(gpFile, "FBO created successfully.\n");
        fboResult = initializeSphere();

        if(fboResult != 0){
            fprintf(gpFile, "initializeSphere() failed with error code: %d\n", fboResult);
            return -11;
        }
        else{
            fprintf(gpFile, "initializeSphere() succeeded.\n");
        }
    }
    else{
        fprintf(gpFile, "createAndPrepareFBOForDrawing() failed.\n");
        return -12;
    }

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

Bool createAndPrepareFBOForDrawing(GLint textureWidth, GLint textureHeight) {
    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    if (textureWidth > maxTexSize || textureHeight > maxTexSize) {
        fprintf(gpFile, "Requested texture size (%d x %d) exceeds max texture size (%d).\n",
                textureWidth, textureHeight, maxTexSize);
        return False;
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Use consistent internal format, format and type
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // attach texture and depth renderbuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(gpFile, "Framebuffer is not complete: 0x%X\n", status);
        // optionally query glGetError() here
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return False;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return True;
}

int initializeSphere(void){
    void resize_sphere(int width, int height);

    // Fragment Shader
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
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar* szInfoLog = NULL;
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
    pf_shaderProgramObject_sphere = glCreateProgram();
    if(pf_shaderProgramObject_sphere == 0){
        fprintf(gpFile, "glCreateProgram failed\n");
        return -9;
    }

    // Attach vertex shader to the shader program object
    glAttachShader(pf_shaderProgramObject_sphere, pf_vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(pf_shaderProgramObject_sphere, pf_fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(pf_shaderProgramObject_sphere, AMC_ATTRIBUTE_POSITION_SPHERE, "aPosition");
    glBindAttribLocation(pf_shaderProgramObject_sphere, AMC_ATTRIBUTE_NORMAL_SPHERE, "aNormal");

    // Link the shader program and check for errors
    glLinkProgram(pf_shaderProgramObject_sphere);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(pf_shaderProgramObject_sphere, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pf_shaderProgramObject_sphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pf_shaderProgramObject_sphere, iInfoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }
    
    // Get the required uniform locations from the shader program object
    pf_modelMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uModelMatrix");
    pf_viewMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uViewMatrix");
    pf_projectionMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uProjectionMatrix");
    pf_laUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLa");
    pf_ldUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLd");
    pf_lsUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLs");
    pf_kaUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKa");
    pf_kdUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKd");
    pf_ksUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKs");
    pf_materialShininessUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uMaterialShininess");
    pf_lightPositionUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLightPosition");
    pf_lKeyPressedUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLKeyPressed");

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
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION_SPHERE, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION_SPHERE);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        // Normal VBO
        glGenBuffers(1, &gVbo_sphere_normal);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL_SPHERE, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL_SPHERE);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    resize_sphere(FBO_WIDTH, FBO_HEIGHT);

    return 0;
}

void resize(int width, int height){
    //code
    
    //If height becomes zero, make height 1
    if(height <= 0)
        height = 1;

    winWidth = width;
    winHeight = height;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix_cube = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void resize_sphere(int width, int height){
    //code
    
    //If height becomes zero, make height 1
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix_sphere = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void){
    void resize(int, int);
    void displaySphere(void);

    //Call sphere related code
    if(fboResult == 0){
        displaySphere();
    }

    //Call cube's resize to compensate sphere's FBO resize
    resize(winWidth, winHeight);

    // Clear color again to compensate sphere's FBO clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program object
    glUseProgram(shaderProgramObject_cube);
    {
        // Transformations
        mat4 modelViewMatrix = mat4::identity();
        mat4 modelViewProjectionMatrix = mat4::identity();
        mat4 translationMatrix = mat4::identity();
        mat4 rotationMatrix = mat4::identity();
        mat4 scaleMatrix = mat4::identity();
        {
            // Prepare transformation matrices
            translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
            
            rotationMatrix = vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f);

            //scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);

            // Modeview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelViewMatrix = translationMatrix * rotationMatrix * scaleMatrix;

            // Prepare final model view projection matrix as a combination of perspective projection matrix and model view matrix and send it to the shader
            modelViewProjectionMatrix = perspectiveProjectionMatrix_cube * modelViewMatrix;

            // Pass the model view projection matrix to the shader
            glUniformMatrix4fv(mvpMatrixUniform_cube, 1, GL_FALSE, modelViewProjectionMatrix);

            // Bind the texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fbo);
            glUniform1i(textureSamplerUniform, 0); // Set the texture sampler to use

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
                glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
            }
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);

    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void displaySphere(void){
    void resize_sphere(int width, int height);

    if(fbo){
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    resize_sphere(FBO_WIDTH, FBO_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program object
    glUseProgram(pf_shaderProgramObject_sphere);
    {
        // Transformations
        mat4 modelMatrix = mat4::identity();
        mat4 viewMatrix = mat4::identity();
        mat4 scaleMatrix = mat4::identity();
        {
            mat4 translationMatrix = mat4::identity();

            viewMatrix = vmath::lookat( vec3(0.0f, 0.0f, 2.0f), 
                                        vec3(0.0f, 0.0f, 0.0f), 
                                        vec3(0.0f, 1.0f, 0.0f));

            
            mat4 lightRotationMatrix = mat4::identity();
            mat4 lightTranslationMatrix = mat4::identity();
            mat4 lightTransformMatrix = mat4::identity();

            glUniformMatrix4fv(pf_viewMatrixUniform_sphere, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(pf_projectionMatrixUniform_sphere, 1, GL_FALSE, perspectiveProjectionMatrix_sphere);
            glUniform3fv(pf_laUniform_sphere, 1, lightAmbient_sphere);
            glUniform3fv(pf_ldUniform_sphere, 1, lightDiffuse_sphere);
            glUniform3fv(pf_lsUniform_sphere, 1, lightSpecular_sphere);
            glUniform1i(pf_lKeyPressedUniform_sphere, bLight_sphere ? 1 : 0);

            if(keyPressed_sphere == 1){
                lightRotationMatrix = vmath::rotate(angleForXRotation_sphere, 1.0f, 0.0f, 0.0f);
                lightTranslationMatrix = vmath::translate(0.0f, 0.0f, 20.0f);
            }
            else if(keyPressed_sphere == 2){
                lightRotationMatrix = vmath::rotate(angleForYRotation_sphere, 0.0f, 1.0f, 0.0f);
                lightTranslationMatrix = vmath::translate(20.0f, 0.0f, 0.0f);
            }
            else if(keyPressed_sphere == 3){
                lightRotationMatrix = vmath::rotate(angleForZRotation_sphere, 0.0f, 0.0f, 1.0f);
                lightTranslationMatrix = vmath::translate(0.0f, 20.0f, 0.0f);
            }
            else{
                lightTranslationMatrix = vmath::translate(10.0f, 10.0f, 10.0f);
            }


            modelMatrix = translationMatrix * scaleMatrix;
            /*
                It is important multiply modelView matrix with light Transforms because we also need to move the light source with the model in this case
                In this case the modeView matrix is dynamic sphere position, and we need the light to revolve around the sphere
            */
            lightTransformMatrix = modelMatrix * viewMatrix * lightRotationMatrix * lightTranslationMatrix;

            glUniform4fv(pf_lightPositionUniform_sphere, 1, vec4(lightPosition_sphere[0], lightPosition_sphere[1], lightPosition_sphere[2], lightPosition_sphere[3]) * lightTransformMatrix.transpose());
            glUniformMatrix4fv(pf_modelMatrixUniform_sphere, 1, GL_FALSE, modelMatrix);
            glUniform3fv(pf_kaUniform_sphere, 1, materials[materialChoice].ambient);
            glUniform3fv(pf_kdUniform_sphere, 1, materials[materialChoice].diffuse);
            glUniform3fv(pf_ksUniform_sphere, 1, materials[materialChoice].specular);
            glUniform1f(pf_materialShininessUniform_sphere, materials[materialChoice].shininess); 
        }

        glBindVertexArray(gVao_sphere);
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
            glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);

    if(fbo){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void update(void){
    //code
    angleCube = angleCube - 0.5f;

    if(bLight_sphere){
        angleForXRotation_sphere = angleForXRotation_sphere + 0.5f;
        angleForYRotation_sphere = angleForYRotation_sphere + 0.5f;
        angleForZRotation_sphere = angleForZRotation_sphere + 0.5f;
    }
}   

void uninitialize(void){
    void uninitialize_sphere(void);

    /* Close the file */
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
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

    //code
    if(fboResult == 0){
        uninitialize_sphere();
    }

    if(vbo_texcoord_cube){
        glDeleteBuffers(1, &vbo_texcoord_cube);
        vbo_texcoord_cube = 0;
    }

    if(vbo_position_cube){
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
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
    if(shaderProgramObject_cube){
        glUseProgram(shaderProgramObject_cube);
        GLint numShaders;
        glGetProgramiv(shaderProgramObject_cube, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(shaderProgramObject_cube, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(shaderProgramObject_cube, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject_cube);
        shaderProgramObject_cube = 0;
    }

    GLXContext currentContext = glXGetCurrentContext();
    if(currentContext && (currentContext == glxContext)){
        glXMakeCurrent(gpDisplay, 0, 0);
    }

    if(glxContext){
        glXDestroyContext(gpDisplay, glxContext);
        glxContext = NULL;
    }

    if(gXVisualInfo){
        free(gXVisualInfo);
        gXVisualInfo = NULL;
    }

    // Destroy the window
    if (window) {
        XDestroyWindow(gpDisplay, window);
        window = 0;
    }

    // Free the colormap
    if (colormap) {
        XFreeColormap(gpDisplay, colormap);
        colormap = 0;
    }

    // Close the display connection
    if (gpDisplay) {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
}

void uninitialize_sphere(void){
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

    if(pf_shaderProgramObject_sphere){
        glUseProgram(pf_shaderProgramObject_sphere);
        GLint numShaders;
        glGetProgramiv(pf_shaderProgramObject_sphere, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pf_shaderProgramObject_sphere, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pf_shaderProgramObject_sphere, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(pf_shaderProgramObject_sphere);
        pf_shaderProgramObject_sphere = 0;
    }

    if(rbo){
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
    
    if(fbo){
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
}

// Compile with: gcc -o XWindow XWindow.c -lX11

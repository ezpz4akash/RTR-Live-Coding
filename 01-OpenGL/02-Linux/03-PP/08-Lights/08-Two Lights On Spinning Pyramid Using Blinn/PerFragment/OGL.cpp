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

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

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
GLuint shaderProgramObject = 0;

enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_NORMAL,
};

GLuint vao_pyramid = 0;
GLuint vbo_position_pyramid = 0;
GLuint vbo_normal_pyramid = 0;

mat4 perspectiveProjectionMatrix;

GLuint modelMatrixUniform = 0;
GLuint viewMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;

GLuint laUniform[2];               // Light Ambient
GLuint ldUniform[2];               // Light Diffuse
GLuint lsUniform[2];               // Light Specular
GLuint lightPositionUniform[2];    // Light Position

GLuint kaUniform = 0;               // Material Ambient
GLuint ksUniform = 0;               // Material Specular
GLuint kdUniform = 0;               // Material Diffuse
GLuint materialShininessUniform = 0; // Material Shininess

GLuint lKeyPressedUniform = 0;      // Light Key Pressed

struct Light {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

struct Light lights[2];

GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;

Bool bLight = False; // Light On/Off
Bool bAnimate = False; // Animation On/Off

/* Rotation angle variables */
GLfloat anglePyramid = 0.0f;

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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - Lights - TwoLightsOnSpinningPyramidUsingBlinnPerFragment");

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
                        bDone = True;
                    break;                  
                    
                    default:
                        break;
                }

                XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);

                switch(keys[0]){
                    case 'F':
                    case 'f':
                        gbFullScreen = !gbFullScreen;
                        toggleFullScreen();
                    break;

                    case 'A':
                    case 'a':
                        bAnimate = !bAnimate;
                    break;

                    case 'L':
                    case 'l':
                        bLight = !bLight;
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
        "in vec3 aNormal;\n" \
        "out vec4 eyeCoordinates;\n" \
        "out vec3 transformedNormal;\n" \
        "out vec3 lightSource[2];\n" \
        "uniform mat4 uModelMatrix;\n" \
        "uniform mat4 uViewMatrix;\n" \
        "uniform mat4 uProjectionMatrix;\n"\
        "uniform vec4 uLightPosition[2];\n" \
        "void main(void)\n" \
        "{\n" \
        "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" \
        "   transformedNormal = (normalMatrix * aNormal);\n" \
        "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
        "   for(int i = 0; i < 2; i++)\n" \
        "   {\n" \
        "       lightSource[i] = (vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n" \
        "   }\n"\
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
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
        "in vec3 lightSource[2];\n" \
        "uniform vec3 uLa[2];\n" \
        "uniform vec3 uLd[2];\n" \
        "uniform vec3 uLs[2];\n" \
        "uniform vec3 uKa;\n" \
        "uniform vec3 uKd;\n" \
        "uniform vec3 uKs;\n" \
        "uniform float uMaterialShininess;\n" \
        "uniform int  uLKeyPressed;\n" \
        "vec4 out_phong_ads_Light;\n" \
        "out vec4 FragColor;\n" \
        "void main(void)\n" \
        "{\n" \
        "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n"\
        "   vec3 normalizedLightSource[3];\n" \
        "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" \
        "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" \
        "   if(uLKeyPressed == 1)\n" \
        "   {\n" \
        "       float tnDotLd[2];\n" \
        "       vec3 halfVector[2];\n" \
        "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" \
        "       vec3 ambient[2];\n" \
        "       vec3 diffuse[2];\n" \
        "       vec3 specular[2];\n" \
        "       for(int i = 0; i < 2; i++)\n" \
        "       {\n" \
        "           normalizedLightSource[i] = normalize(lightSource[i]);\n" \
        "           tnDotLd[i] = max(dot(normalizedLightSource[i], normalizedTransformNormal), 0.0);\n" \
        "           halfVector[i] = normalize(lightSource[i] + viewerVector[i]) / length(lightSource[i] + viewerVector[i]);\n" \
        "           ambient[i] = uLa[i] * uKa;\n" \
        "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n" \
        "           specular[i] = uLs[i] * uKs * pow(max(dot(halfVector[i], normalizedTransformNormal), 0.0), uMaterialShininess);\n" \
        "           out_phong_ads_Light = out_phong_ads_Light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n" \
        "       }\n" \
        "       FragColor = out_phong_ads_Light;\n" \
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

    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    laUniform[0] = glGetUniformLocation(shaderProgramObject, "uLa[0]");
    ldUniform[0] = glGetUniformLocation(shaderProgramObject, "uLd[0]");
    lsUniform[0] = glGetUniformLocation(shaderProgramObject, "uLs[0]");
    laUniform[1] = glGetUniformLocation(shaderProgramObject, "uLa[1]");
    ldUniform[1] = glGetUniformLocation(shaderProgramObject, "uLd[1]");
    lsUniform[1] = glGetUniformLocation(shaderProgramObject, "uLs[1]");
    kaUniform = glGetUniformLocation(shaderProgramObject, "uKa");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "uLightPosition[0]");
    lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "uLightPosition[1]");
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLKeyPressed");

    // Set the lighting parameters
    lights[0].ambient   = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[0].diffuse   = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    lights[0].specular  = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    lights[0].position  = vec4(-2.0f, 0.0f, 0.0f, 1.0f);
    lights[1].ambient   = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[1].diffuse   = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    lights[1].specular  = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    lights[1].position  = vec4(2.0f, 0.0f, 0.0f, 1.0f);

    // Provide vertex position, color, texture coordinates, normals, etc. to the shader program object

    // pyramid
    {
        const GLfloat pyramid_position[] = {
            // front
            0.0f,  1.0f,  0.0f, // front-top
            -1.0f, -1.0f,  1.0f, // front-left
            1.0f, -1.0f,  1.0f, // front-right
            
            // right
            0.0f,  1.0f,  0.0f, // right-top
            1.0f, -1.0f,  1.0f, // right-left
            1.0f, -1.0f, -1.0f, // right-right

            // back
            0.0f,  1.0f,  0.0f, // back-top
            1.0f, -1.0f, -1.0f, // back-left
            -1.0f, -1.0f, -1.0f, // back-right

            // left
            0.0f,  1.0f,  0.0f, // left-top
            -1.0f, -1.0f, -1.0f, // left-left
            -1.0f, -1.0f,  1.0f, // left-right
        };

        GLfloat pyramid_normal[] = {
            // front
            0.000000f, 0.447214f,  0.894427f, // front-top
            0.000000f, 0.447214f,  0.894427f, // front-left
            0.000000f, 0.447214f,  0.894427f, // front-right
                                    
            // right			    
            0.894427f, 0.447214f,  0.000000f, // right-top
            0.894427f, 0.447214f,  0.000000f, // right-left
            0.894427f, 0.447214f,  0.000000f, // right-right

            // back
            0.000000f, 0.447214f, -0.894427f, // back-top
            0.000000f, 0.447214f, -0.894427f, // back-left
            0.000000f, 0.447214f, -0.894427f, // back-right

            // left
            -0.894427f, 0.447214f,  0.000000f, // left-top
            -0.894427f, 0.447214f,  0.000000f, // left-left
            -0.894427f, 0.447214f,  0.000000f, // left-right
        };

        // Create Vertex Array Object (VAO) for array of vertex attributes
        // vao is an array object that stores the state of vertex attributes
        glGenVertexArrays(1, &vao_pyramid);
        glBindVertexArray(vao_pyramid);
        {
            // Position VBO
            {
                glGenBuffers(1, &vbo_position_pyramid);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_position_pyramid);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_position), pyramid_position, GL_STATIC_DRAW);
                    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            // Normal VBO
            {
                glGenBuffers(1, &vbo_normal_pyramid);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_pyramid);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_normal), pyramid_normal, GL_STATIC_DRAW);
                    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
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

    perspectiveProjectionMatrix = mat4::identity();

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

            // Prepare transformation matrices
            translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
            rotationMatrix = vmath::rotate(anglePyramid, 0.0f, 1.0f, 0.0f);

            // Modeview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelMatrix = translationMatrix * rotationMatrix;

            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

            for(int i = 0; i < 2; i++){
                glUniform3fv(laUniform[i], 1, lights[i].ambient);
                glUniform3fv(ldUniform[i], 1, lights[i].diffuse);
                glUniform3fv(lsUniform[i], 1, lights[i].specular);
                glUniform4fv(lightPositionUniform[i], 1, lights[i].position);
            }
            glUniform3f(kaUniform, materialAmbient[0], materialAmbient[1], materialAmbient[2]);
            glUniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
            glUniform3f(ksUniform, materialSpecular[0], materialSpecular[1], materialSpecular[2]);
            glUniform1f(materialShininessUniform, materialShininess);
            
            glUniform1i(lKeyPressedUniform, bLight ? 1 : 0);

            // Bind the VAO for pyramid
            glBindVertexArray(vao_pyramid);
            {
                // Draw the pyramid
                glDrawArrays(GL_TRIANGLES, 0, 12);
            }
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);

    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void update(void){
    //code
    anglePyramid = anglePyramid + 1.0f;
}   

void uninitialize(void){
    /* Close the file */
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
    }

    // Free vbo and vao
    if(vbo_normal_pyramid){
        glDeleteBuffers(1, &vbo_normal_pyramid);
        vbo_normal_pyramid = 0;
    }

    if(vbo_position_pyramid){
        glDeleteBuffers(1, &vbo_position_pyramid);
        vbo_position_pyramid = 0;
    }

    if(vao_pyramid){
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
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

// Compile with: gcc -o XWindow XWindow.c -lX11

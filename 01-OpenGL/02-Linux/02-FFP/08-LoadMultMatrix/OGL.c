// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define _USE_MATH_DEFINES
#include <math.h>

//Xlib header files
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

// OpenGL related header files
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

Display *gpDisplay = NULL;
XVisualInfo* gXVisualInfo = NULL; // Like device context in Windows
Window window;
Colormap colormap;

Bool gbFullScreen = False;
Bool bActiveWindow = False;

// OpenGL related variables
GLXContext glxContext = NULL;

/* Variable related to File I/O */
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

/* Rotation angle variables */
GLfloat angleCube = 0.0f;

/* Transformation Matrices */
float identityMatrix[16];
float translationMatrix[16];
float scaleMatrix[16];
float RotationMatrixX[16];
float RotationMatrixY[16];
float RotationMatrixZ[16];

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

    int frameBufferAtrributes[] = {
                                    GLX_DOUBLEBUFFER,
                                    GLX_RGBA,
                                    GLX_RED_SIZE, 8,
                                    GLX_GREEN_SIZE, 8,
                                    GLX_BLUE_SIZE, 8,
                                    GLX_ALPHA_SIZE, 8,
                                    GLX_DEPTH_SIZE, 24,
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

    // Get visual information
    // status = XMatchVisualInfo(gpDisplay, defaultScreen, defaultDepth, TrueColor, gXVisualInfo); 
    gXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAtrributes);
    if (!gXVisualInfo) {
        fprintf(gpFile, "ERROR: Unable to get visual info.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - LoadMultMatrix");

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

    glxContext = glXCreateContext(gpDisplay, gXVisualInfo, NULL, True);
    if(glxContext == NULL){
        fprintf(gpFile, "ERROR: glXCreateContext Failed.\n");
        return -1;
    }

    glXMakeCurrent(gpDisplay, window, glxContext);

    printGLInfo();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

    // Translation Matrix
    translationMatrix[0]   = 1.0f;
    translationMatrix[1]   = 0.0f;
    translationMatrix[2]   = 0.0f;
    translationMatrix[3]   = 0.0f;
    translationMatrix[4]   = 0.0f;
    translationMatrix[5]   = 1.0f;
    translationMatrix[6]   = 0.0f;
    translationMatrix[7]   = 0.0f;
    translationMatrix[8]   = 0.0f;
    translationMatrix[9]   = 0.0f;
    translationMatrix[10]  = 1.0f;
    translationMatrix[11]  = 0.0f;
    translationMatrix[12]  = 0.0f;
    translationMatrix[13]  = 0.0f;
    translationMatrix[14]  = -8.0f;
    translationMatrix[15]  = 1.0f;

    // Scale Matrix
    scaleMatrix[0]   = 0.75f;
    scaleMatrix[1]   = 0.0f;
    scaleMatrix[2]   = 0.0f;
    scaleMatrix[3]   = 0.0f;
    scaleMatrix[4]   = 0.0f;
    scaleMatrix[5]   = 0.75f;
    scaleMatrix[6]   = 0.0f;
    scaleMatrix[7]   = 0.0f;
    scaleMatrix[8]   = 0.0f;
    scaleMatrix[9]   = 0.0f;
    scaleMatrix[10]  = 0.75f;
    scaleMatrix[11]  = 0.0f;
    scaleMatrix[12]  = 0.0f;
    scaleMatrix[13]  = 0.0f;
    scaleMatrix[14]  = 0.0f;
    scaleMatrix[15]  = 1.0f;

    //Depth related code
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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
    //variable declarations
    float angle = 0.0f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    {
        // Set to identity matrix
        //glLoadIdentity();
        glLoadMatrixf(identityMatrix);

        // Translate triangle backwards by z
        //glTranslatef(0.0f, 0.0f, -6.0f);
        glMultMatrixf(translationMatrix);

        //Scale 
        glMultMatrixf(scaleMatrix);

        // Rotate triangle along x,y,z
        //glRotatef(angleCube, 1.0f, 0.0f, 0.0f);
        //glRotatef(angleCube, 0.0f, 1.0f, 0.0f);
        //glRotatef(angleCube, 0.0f, 0.0f, 1.0f);

        angle = angleCube * (M_PI / 180.0f);

        RotationMatrixX[0]   = 1.0f;
        RotationMatrixX[1]   = 0.0f;
        RotationMatrixX[2]   = 0.0f;
        RotationMatrixX[3]   = 0.0f;
        RotationMatrixX[4]   = 0.0f;
        RotationMatrixX[5]   = cos(angle);
        RotationMatrixX[6]   = sin(angle);
        RotationMatrixX[7]   = 0.0f;
        RotationMatrixX[8]   = 0.0f;
        RotationMatrixX[9]   = -sin(angle);
        RotationMatrixX[10]  = cos(angle);
        RotationMatrixX[11]  = 0.0f;
        RotationMatrixX[12]  = 0.0f;
        RotationMatrixX[13]  = 0.0f;
        RotationMatrixX[14]  = 0.0f;
        RotationMatrixX[15]  = 1.0f;

        glMultMatrixf(RotationMatrixX);

        RotationMatrixY[0]   = cos(angle);
        RotationMatrixY[1]   = 0.0f;
        RotationMatrixY[2]   = -sin(angle);
        RotationMatrixY[3]   = 0.0f;
        RotationMatrixY[4]   = 0.0f;
        RotationMatrixY[5]   = 1.0f;
        RotationMatrixY[6]   = 0.0f;
        RotationMatrixY[7]   = 0.0f;
        RotationMatrixY[8]   = sin(angle);
        RotationMatrixY[9]   = 0.0f;
        RotationMatrixY[10]  = cos(angle);
        RotationMatrixY[11]  = 0.0f;
        RotationMatrixY[12]  = 0.0f;
        RotationMatrixY[13]  = 0.0f;
        RotationMatrixY[14]  = 0.0f;
        RotationMatrixY[15]  = 1.0f;

        glMultMatrixf(RotationMatrixY);

        RotationMatrixZ[0]   = cos(angle);
        RotationMatrixZ[1]   = sin(angle);
        RotationMatrixZ[2]   = 0.0f;
        RotationMatrixZ[3]   = 0.0f;
        RotationMatrixZ[4]   = -sin(angle);
        RotationMatrixZ[5]   = cos(angle);
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

        glBegin(GL_QUADS);
            /* Front Face */
            {
                glColor3f(1.0f, 0.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
            }

            /* Right Face */
            {
                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
            }

            /* Back Face */
            {
                glColor3f(0.0f, 0.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);
            }

            /* Left Face */
            {
                glColor3f(0.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
            }

            /* Top Face */
            {
                glColor3f(1.0f, 1.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
            }

            /* Bottom Face */
            {
                glColor3f(1.0f, 0.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
            }
        glEnd();
    }
    
    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void update(void){
    //code
    angleCube = angleCube + 0.05f;
    if(angleCube >= 360.0f){
        angleCube = angleCube - 360.0f;
    }
}   

void uninitialize(void){
    /* Close the file */
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
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

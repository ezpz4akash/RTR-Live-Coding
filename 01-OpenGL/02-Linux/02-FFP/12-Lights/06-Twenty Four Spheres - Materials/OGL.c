// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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


// Solar system related variables
GLUquadric* quadric = NULL;

Bool bLight = False;

GLfloat lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightModelAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightModelLocalViwer[] = {0.0f};

GLfloat angleForXRotation = 0.0f;
GLfloat angleForYRotation = 0.0f;
GLfloat angleForZRotation = 0.0f;

GLint keyPressed = -1;

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
                                    GLX_DOUBLEBUFFER, True,
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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - Lights - Twenty Four Spheres - Materials");

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

                    case 'L':
                    case 'l':
                        bLight = !bLight;
                        if(bLight){
                            glEnable(GL_LIGHTING);
                        }
                        else{
                            glDisable(GL_LIGHTING);
                        }
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

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    //Depth related code
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Initialize quadric
    quadric = gluNewQuadric();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lightModelLocalViwer);

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

    // To orthographic projection
    if(width <= height){
        glOrtho(0.0f, 15.5f,(0.0f * ((GLfloat)height / (GLfloat)width)), (15.5f * ((GLfloat)height / (GLfloat)width)), -10.0f, 10.0f);
    }
    else{
        glOrtho((0.0f * ((GLfloat)width / (GLfloat)height)), (15.5f * ((GLfloat)width / (GLfloat)height)), 0.0f, 15.5f, -10.0f, 10.0f);
    }

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
}

void display(void){
    void draw24Spheres(void);
    
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();

    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0f;
    if(keyPressed == 1){
        glRotatef(angleForXRotation, 1.0f, 0.0f, 0.0f);
        lightPosition[2] = angleForXRotation;
    }
    else if(keyPressed == 2){
        glRotatef(angleForYRotation, 0.0f, 1.0f, 0.0f);
        lightPosition[0] = angleForYRotation;
    }
    else if(keyPressed == 3){
        glRotatef(angleForZRotation, 0.0f, 0.0f, 1.0f);
        lightPosition[1] = angleForZRotation;
    }

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    glPushMatrix();
        draw24Spheres();
    glPopMatrix();

    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void draw24Spheres(void){
    // variable declarations
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;

    // ***** 1st sphere on 1st column, emerald ***** 
    // ambient material
    materialAmbient[0] = 0.0215f; // r
    materialAmbient[1] = 0.1745f; // g
    materialAmbient[2] = 0.0215f; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.07568f; // r
    materialDiffuse[1] = 0.61424f; // g
    materialDiffuse[2] = 0.07568f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.633f;    // r
    materialSpecular[1] = 0.727811f; // g
    materialSpecular[2] = 0.633f;    // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 1st column, jade *****
    // ambient material
    materialAmbient[0] = 0.135f;  // r
    materialAmbient[1] = 0.2225f; // g
    materialAmbient[2] = 0.1575f; // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.54f; // r
    materialDiffuse[1] = 0.89f; // g
    materialDiffuse[2] = 0.63f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.316228f; // r
    materialSpecular[1] = 0.316228f; // g
    materialSpecular[2] = 0.316228f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 1st column, obsidian *****
    // ambient material
    materialAmbient[0] = 0.05375f; // r
    materialAmbient[1] = 0.05f;    // g
    materialAmbient[2] = 0.06625f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.18275f; // r
    materialDiffuse[1] = 0.17f;    // g
    materialDiffuse[2] = 0.22525f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.332741f; // r
    materialSpecular[1] = 0.328634f; // g
    materialSpecular[2] = 0.346435f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.3f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 1st column, pearl *****
    // ambient material
    materialAmbient[0] = 0.25f;    // r
    materialAmbient[1] = 0.20725f; // g
    materialAmbient[2] = 0.20725f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 1.0f;   // r
    materialDiffuse[1] = 0.829f; // g
    materialDiffuse[2] = 0.829f; // b
    materialDiffuse[3] = 1.0f;  // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.296648f; // r
    materialSpecular[1] = 0.296648f; // g
    materialSpecular[2] = 0.296648f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.088f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 1st column, ruby *****
    // ambient material
    materialAmbient[0] = 0.1745f;  // r
    materialAmbient[1] = 0.01175f; // g
    materialAmbient[2] = 0.01175f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.61424f; // r
    materialDiffuse[1] = 0.04136f; // g
    materialDiffuse[2] = 0.04136f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.727811f; // r
    materialSpecular[1] = 0.626959f; // g
    materialSpecular[2] = 0.626959f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 1st column, turquoise *****
    // ambient material
    materialAmbient[0] = 0.1f;     // r
    materialAmbient[1] = 0.18725f; // g
    materialAmbient[2] = 0.1745f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.396f;   // r
    materialDiffuse[1] = 0.74151f; // g
    materialDiffuse[2] = 0.69102f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.297254f; // r
    materialSpecular[1] = 0.30829f;  // g
    materialSpecular[2] = 0.306678f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);


    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 2nd column, brass *****
    // ambient material
    materialAmbient[0] = 0.329412f; // r
    materialAmbient[1] = 0.223529f; // g
    materialAmbient[2] = 0.027451f; // b
    materialAmbient[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.780392f; // r
    materialDiffuse[1] = 0.568627f; // g
    materialDiffuse[2] = 0.113725f; // b
    materialDiffuse[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.992157f; // r
    materialSpecular[1] = 0.941176f; // g
    materialSpecular[2] = 0.807843f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.21794872f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 2nd column, bronze *****
    // ambient material
    materialAmbient[0] = 0.2125f; // r
    materialAmbient[1] = 0.1275f; // g
    materialAmbient[2] = 0.054f;  // b
    materialAmbient[3] = 1.0f;   // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.714f;   // r
    materialDiffuse[1] = 0.4284f;  // g
    materialDiffuse[2] = 0.18144f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.393548f; // r
    materialSpecular[1] = 0.271906f; // g
    materialSpecular[2] = 0.166721f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.2f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 2nd column, chrome *****
    // ambient material
    materialAmbient[0] = 0.25f; // r
    materialAmbient[1] = 0.25f; // g
    materialAmbient[2] = 0.25f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.4f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.774597f; // r
    materialSpecular[1] = 0.774597f; // g
    materialSpecular[2] = 0.774597f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 2nd column, copper *****
    // ambient material
    materialAmbient[0] = 0.19125f; // r
    materialAmbient[1] = 0.0735f;  // g
    materialAmbient[2] = 0.0225f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.7038f;  // r
    materialDiffuse[1] = 0.27048f; // g
    materialDiffuse[2] = 0.0828f;  // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.256777f; // r
    materialSpecular[1] = 0.137622f; // g
    materialSpecular[2] = 0.086014f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 2nd column, gold *****
    // ambient material
    materialAmbient[0] = 0.24725f; // r
    materialAmbient[1] = 0.1995f;  // g
    materialAmbient[2] = 0.0745f;  // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.75164f; // r
    materialDiffuse[1] = 0.60648f; // g
    materialDiffuse[2] = 0.22648f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.628281f; // r
    materialSpecular[1] = 0.555802f; // g
    materialSpecular[2] = 0.366065f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 2nd column, silver *****
    // ambient material
    materialAmbient[0] = 0.19225f; // r
    materialAmbient[1] = 0.19225f; // g
    materialAmbient[2] = 0.19225f; // b
    materialAmbient[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.50754f; // r
    materialDiffuse[1] = 0.50754f; // g
    materialDiffuse[2] = 0.50754f; // b
    materialDiffuse[3] = 1.0f;    // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.508273f; // r
    materialSpecular[1] = 0.508273f; // g
    materialSpecular[2] = 0.508273f; // b
    materialSpecular[3] = 1.0f;     // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(7.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 3rd column, black *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01f; // r
    materialDiffuse[1] = 0.01f; // g
    materialDiffuse[2] = 0.01f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50f; // r
    materialSpecular[1] = 0.50f; // g
    materialSpecular[2] = 0.50f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 3rd column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.1f;  // g
    materialAmbient[2] = 0.06f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.0f;        // r
    materialDiffuse[1] = 0.50980392f; // g
    materialDiffuse[2] = 0.50980392f; // b
    materialDiffuse[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.50196078f; // r
    materialSpecular[1] = 0.50196078f; // g
    materialSpecular[2] = 0.50196078f; // b
    materialSpecular[3] = 1.0f;       // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 3rd column, green *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.1f;  // r
    materialDiffuse[1] = 0.35f; // g
    materialDiffuse[2] = 0.1f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.45f; // r
    materialSpecular[1] = 0.55f; // g
    materialSpecular[2] = 0.45f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 3rd column, red *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.0f;  // g
    materialDiffuse[2] = 0.0f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.6f;  // g
    materialSpecular[2] = 0.6f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 3rd column, white *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.55f; // r
    materialDiffuse[1] = 0.55f; // g
    materialDiffuse[2] = 0.55f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.70f; // r
    materialSpecular[1] = 0.70f; // g
    materialSpecular[2] = 0.70f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 3rd column, yellow plastic *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.0f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.60f; // r
    materialSpecular[1] = 0.60f; // g
    materialSpecular[2] = 0.50f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(13.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // *******************************************************
    // *******************************************************
    // *******************************************************

    // ***** 1st sphere on 4th column, black *****
    // ambient material
    materialAmbient[0] = 0.02f; // r
    materialAmbient[1] = 0.02f; // g
    materialAmbient[2] = 0.02f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.01f; // r
    materialDiffuse[1] = 0.01f; // g
    materialDiffuse[2] = 0.01f; // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.4f;  // r
    materialSpecular[1] = 0.4f;  // g
    materialSpecular[2] = 0.4f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 14.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 2nd sphere on 4th column, cyan *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.05f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.5f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04f; // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.7f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 11.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 3rd sphere on 4th column, green *****
    // ambient material
    materialAmbient[0] = 0.0f;  // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.4f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.04f; // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 9.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 4th sphere on 4th column, red *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.0f;  // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.4f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.04f; // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 6.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 5th sphere on 4th column, white *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.05f; // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.5f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.7f;  // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 4.0f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);

    // ***** 6th sphere on 4th column, yellow rubber *****
    // ambient material
    materialAmbient[0] = 0.05f; // r
    materialAmbient[1] = 0.05f; // g
    materialAmbient[2] = 0.0f;  // b
    materialAmbient[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

    // diffuse material
    materialDiffuse[0] = 0.5f;  // r
    materialDiffuse[1] = 0.5f;  // g
    materialDiffuse[2] = 0.4f;  // b
    materialDiffuse[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

    // specular material
    materialSpecular[0] = 0.7f;  // r
    materialSpecular[1] = 0.7f;  // g
    materialSpecular[2] = 0.04f; // b
    materialSpecular[3] = 1.0f; // a
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

    // shininess
    materialShininess = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    // geometry
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(19.5f, 1.5f, 0.0f);
    gluSphere(quadric, 1.0f, 50, 50);
    // *******************************************************
    // *******************************************************
    // *******************************************************
}

void update(void){
    //code
    angleForXRotation = angleForXRotation + 0.5f;
    /*  if(angleForXRotation >= 360.0f){
        angleForXRotation = angleForXRotation - 360.0f;
    } */

    angleForYRotation = angleForYRotation + 0.5f;
    /* if(angleForYRotation >= 360.0f){
        angleForYRotation = angleForYRotation - 360.0f;
    } */


    angleForZRotation = angleForZRotation + 0.5f;
    /* if(angleForZRotation >= 360.0f){
        angleForZRotation = angleForZRotation - 360.0f;
    } */
}

void uninitialize(void){
    /* Close the file */
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
    }

    if(quadric){
        gluDeleteQuadric(quadric);
        quadric = NULL;
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

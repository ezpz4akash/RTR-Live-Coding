// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
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

#define GL_PI 3.145
#define DEG_TO_RAD(deg) ((GLfloat)deg * (GL_PI / 180.0f))

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

GLboolean run = False;

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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - Dynamic Deathly Hallows");

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
                    case XK_space:
                        //PlaySound(TEXT("PotterTheme.wav"), NULL, SND_ASYNC | SND_FILENAME);
                        run = True;
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
            if(run){
                /* Update */
                update();
            }
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

    //Depth related code
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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
    glXSwapBuffers(gpDisplay, window);
}

void update(void){
    //code
    if(tTrianglePos <= 1.0f){
        trianglePos.x = (1.0f - tTrianglePos) * triangleStartPos.x + triangleEndPos.x * tTrianglePos;
        trianglePos.y = (1.0f - tTrianglePos) * triangleStartPos.y  + triangleEndPos.y * tTrianglePos;
        tTrianglePos = tTrianglePos + 0.002f;
    }
    else{
        if(tCirclePos <= 1.0f){
            circlePos.x = (1.0f - tCirclePos) * circleStartPos.x + circleEndPos.x * tCirclePos;
            circlePos.y = (1.0f - tCirclePos) * circleStartPos.y  + circleEndPos.y * tCirclePos;
            tCirclePos = tCirclePos + 0.002f;
        }
        else{
            if(tWandPos <= 1.0f){
                wandPos.x = (1.0f - tWandPos) * wandStartPos.x + wandEndPos.x * tWandPos;
                wandPos.y = (1.0f - tWandPos) * wandStartPos.y  + wandEndPos.y * tWandPos;
                tWandPos = tWandPos + 0.002f;
            }
        }
    }

    zRot = zRot + 0.5f;
    
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

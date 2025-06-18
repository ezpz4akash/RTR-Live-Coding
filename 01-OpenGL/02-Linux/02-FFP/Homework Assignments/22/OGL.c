// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

//Xlib header files
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

// For multithreading and process management to play sound asynchronously
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// OpenGL related header files
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define DRAW_CURVE 0

#define GL_PI 3.14159265359f
#define DEG_TO_RAD(deg) ((GLfloat)deg * (GL_PI / 180.0f))
#define CURVE_PRECISION 0.0019f
#define CHAR_ANIMATION 0.002f
#define JETS_SPEED_ON_STRAIGHT_PROJECTION 0.25f

#define CHARACTER_WIDTH 20
#define GAP_BETWEEN_CHARACTERS 4

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

GLfloat orangeColor[]   = {255.0f / 255.0f, 103.0f / 255.0f, 31.0f / 255.0f};
GLfloat whiteColor[]    = {1.0f, 1.0f, 1.0f};
GLfloat greenColor[]    = {4.0f / 255.0f, 106.0f / 255.0f, 56.0f / 255.0f};
GLfloat grayColor[]     = {0.5f, 0.5f, 0.5f};

typedef enum PATH{
    CURVE1, STRAIGHT, CURVE2
}PATH;

PATH JET_PATH = CURVE1;

typedef struct Point{
    GLfloat x, y;
} Point;

Point p1;
Point p2;
Point pControl;
GLfloat curveXPoint;
GLfloat curveYPoint;
GLfloat t = 0.0f;
GLfloat jetRotation = 0.0f;

GLfloat midJetPosX, midJetPosY;

GLboolean colorB = False, colorH = False, colorA1 = False, colorR = False, colorA2 = False, colorT = False;

Point posB, posH, posA1, posR, posA2, posT;
Point posInitialB, posInitialH, posInitialA1, posInitialR, posInitialA2, posInitialT;
Point posFinalB, posFinalH, posFinalA1, posFinalR, posFinalA2, posFinalT;

GLboolean run = False;
GLboolean startJet = False;
GLboolean animateB = True, animateH = False, animateA1 = False, animateR = False, animateA2 = False, animateT = False;

GLfloat charInterpolate = 0.0f;

pid_t soundPid = -1;

void* PlaySoundThread(void* arg) {
    const char* filepath = (const char*)arg;

    pid_t pid = fork();
    if (pid == 0) {
        execlp("aplay", "aplay", filepath, (char*)NULL);
        exit(EXIT_FAILURE); 
    } else if (pid > 0) {
        soundPid = pid;
        int status;
        waitpid(pid, &status, 0);
        
    }
    return NULL;
}

void PlaySoundAsync(const char* filepath) {
    pthread_t tid;
    pthread_create(&tid, NULL, PlaySoundThread, (void*)filepath);
    pthread_detach(tid);
}

void StopSoundAsync() {
    if (soundPid > 0) {
        kill(soundPid, SIGTERM);
        soundPid = -1;
    }
}


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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - Dynamic Bharat");

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
                        //PlaySound(TEXT("RangDeBasanti.wav"), NULL, SND_ASYNC | SND_FILENAME);
                        PlaySoundAsync("RangDeBasanti.wav");
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

    StopSoundAsync();
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

    posInitialB.x   = -200.0f;
    posInitialB.y   = 0.0f;
    posInitialH.x   = -200.0f;
    posInitialH.y   = 120.0f;
    posInitialA1.x  = -200.0f;
    posInitialA1.y  = -120.0f;
    posInitialR.x   = 200.0f;
    posInitialR.y   = 120.0f;
    posInitialA2.x  = 200.0f;
    posInitialA2.y  = 0.0f;
    posInitialT.x   = 200.0f;
    posInitialT.y   = -120.0f;

    posB.x  = posInitialB.x; 
    posB.y  = posInitialB.y; 
    posH.x  = posInitialH.x;
    posH.y  = posInitialH.y;
    posA1.x = posInitialA1.x;
    posA1.y = posInitialA1.y;
    posR.x  = posInitialR.x;
    posR.y  = posInitialR.y;
    posA2.x = posInitialA2.x;
    posA2.y = posInitialA2.y;
    posT.x  = posInitialT.x;
    posT.y  = posInitialT.y;

    posFinalB.x     = 0.0f;
    posFinalB.y     = 0.0f;
    posFinalH.x     = 0.0f;
    posFinalH.y     = 0.0f;
    posFinalA1.x    = 0.0f;
    posFinalA1.y    = 0.0f;
    posFinalR.x     = 0.0f;
    posFinalR.y     = 0.0f;
    posFinalA2.x    = 0.0f;
    posFinalA2.y    = 0.0f;
    posFinalT.x     = 0.0f;
    posFinalT.y     = 0.0f;

    p1.x        = -200.0f, 
    p1.y        = -120.0f;
    p2.x        = -100.0f;
    p2.y        =  -20.0f;
    pControl.x  = -140.0f;
    pControl.y  = -20.0f;

    curveXPoint = p1.x;
    curveYPoint = p1.y;

    midJetPosX  = p1.x;
    midJetPosY  = 0.0f;

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
    // gluPerspective(45.0, ((GLfloat)width / (GLfloat)height), 0.1, 100.0);

    // To orthographic projection
    if(width <= height){
        glOrtho(-100.0f, 100.0f,(-100.0f * ((GLfloat)height / (GLfloat)width)), (100.0f * ((GLfloat)height / (GLfloat)width)), -100.0f, 100.0f);
    }
    else{
        glOrtho((-100.0f * ((GLfloat)width / (GLfloat)height)), (100.0f * ((GLfloat)width / (GLfloat)height)), -100.0f, 100.0f, -100.0f, 100.0f);
    }

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
}

void display(void){
    void drawBHARAT();
    void drawJet(GLfloat*);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    // Set to identity matrix
    glLoadIdentity();
    drawBHARAT();

    glLoadIdentity();
    glTranslatef(curveXPoint, curveYPoint, 0.0f);
    glRotatef(-jetRotation, 0.0f, 0.0f, 1.0f);
    glScalef(0.4f, 0.4f, 1.0f);
    drawJet(greenColor);

    glLoadIdentity();
    glTranslatef(midJetPosX, midJetPosY, 0.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.4f, 0.4f, 1.0f);
    drawJet(whiteColor);

    glLoadIdentity();
    glTranslatef(curveXPoint, -curveYPoint, 0.0f);
    glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(jetRotation, 0.0f, 0.0f, 1.0f);
    glScalef(0.4f, 0.4f, 1.0f);
    drawJet(orangeColor);

    
    #if DRAW_CURVE == 1
        glPointSize(5.0f);
        glLoadIdentity();
        for(GLfloat p = 0.0f; p <= t; p = p + CURVE_PRECISION){
            GLfloat xP1Control = (1 - p) * p1.x + p * pControl.x;
            GLfloat yP1Control = (1 - p) * p1.y + p * pControl.y;

            GLfloat xP2Control = p * p2.x + (1 - p) * pControl.x;
            GLfloat yP2Control = p * p2.y + (1 - p) * pControl.y;

            GLfloat xPoint = (1 - p) * xP1Control + p * xP2Control;
            GLfloat yPoint = (1 - p) * yP1Control + p * yP2Control;

            glBegin(GL_POINTS);
            glVertex3f(xPoint, yPoint, 0.0f);
            glEnd();
        }
    #endif
    
    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void update(void){
    //code

    if(animateB){
        if(charInterpolate <= 1.0f){
            posB.x = (1.0f - charInterpolate) * posInitialB.x + charInterpolate * posFinalB.x;
            posB.y = (1.0f - charInterpolate) * posInitialB.y + charInterpolate * posFinalB.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateB = False;
            animateH = True;
            charInterpolate = 0.0f;
        }
    }

    if(animateH){
        if(charInterpolate <= 1.0f){
            posH.x = (1.0f - charInterpolate) * posInitialH.x + charInterpolate * posFinalH.x;
            posH.y = (1.0f - charInterpolate) * posInitialH.y + charInterpolate * posFinalH.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateH = False;
            animateA1 = True;
            charInterpolate = 0.0f;
        }
    }

    if(animateA1){
        if(charInterpolate <= 1.0f){
            posA1.x = (1.0f - charInterpolate) * posInitialA1.x + charInterpolate * posFinalA1.x;
            posA1.y = (1.0f - charInterpolate) * posInitialA1.y + charInterpolate * posFinalA1.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateA1 = False;
            animateR = True;
            charInterpolate = 0.0f;
        }
    }

    if(animateR){
        if(charInterpolate <= 1.0f){
            posR.x = (1.0f - charInterpolate) * posInitialR.x + charInterpolate * posFinalR.x;
            posR.y = (1.0f - charInterpolate) * posInitialR.y + charInterpolate * posFinalR.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateR = False;
            animateA2 = True;
            charInterpolate = 0.0f;
        }
    }

    if(animateA2){
        if(charInterpolate <= 1.0f){
            posA2.x = (1.0f - charInterpolate) * posInitialA2.x + charInterpolate * posFinalA2.x;
            posA2.y = (1.0f - charInterpolate) * posInitialA2.y + charInterpolate * posFinalA2.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateA2 = False;
            animateT = True;
            charInterpolate = 0.0f;
        }
    }

    if(animateT){
        if(charInterpolate <= 1.0f){
            posT.x = (1.0f - charInterpolate) * posInitialT.x + charInterpolate * posFinalT.x;
            posT.y = (1.0f - charInterpolate) * posInitialT.y + charInterpolate * posFinalT.y;
            charInterpolate = charInterpolate + CHAR_ANIMATION;
        }
        else{
            animateT = False;
            startJet = True;
            charInterpolate = 0.0f;
        }
    }


    if(startJet){
        if(JET_PATH == CURVE1){
            if(t <= 1.0f){
                /* Current point */
                GLfloat xP1Control = (1.0f - t) * p1.x + t * pControl.x;
                GLfloat yP1Control = (1.0f - t) * p1.y + t * pControl.y;
        
                GLfloat xP2Control = t * p2.x + (1.0f - t) * pControl.x;
                GLfloat yP2Control = t * p2.y + (1.0f - t) * pControl.y;
        
                curveXPoint = (1.0f - t) * xP1Control + t * xP2Control;
                curveYPoint = (1.0f - t) * yP1Control + t * yP2Control;
    
                GLfloat currentPointX = curveXPoint;
                GLfloat currentPointY = curveYPoint;
    
                /* Next Point */
                xP1Control = (1.0f - (t + CURVE_PRECISION)) * p1.x + (t + CURVE_PRECISION) * pControl.x;
                yP1Control = (1.0f - (t + CURVE_PRECISION)) * p1.y + (t + CURVE_PRECISION) * pControl.y;
        
                xP2Control = (t + CURVE_PRECISION) * p2.x + (1.0f - (t + CURVE_PRECISION)) * pControl.x;
                yP2Control = (t + CURVE_PRECISION) * p2.y + (1.0f - (t + CURVE_PRECISION)) * pControl.y;
        
                GLfloat nextCurveXPoint = (1.0f - (t + CURVE_PRECISION)) * xP1Control + (t + CURVE_PRECISION) * xP2Control;
                GLfloat nextCurveYPoint = (1.0f - (t + CURVE_PRECISION)) * yP1Control + (t + CURVE_PRECISION) * yP2Control;
    
                GLfloat nextPointX = nextCurveXPoint;
                GLfloat nextPointY = nextCurveYPoint;
    
                GLfloat dy = (nextPointY - currentPointY);
                GLfloat dx = (nextPointX - currentPointX);
    
                jetRotation = 90.0f - (atanf((dy) / (dx)) * (180.0f / GL_PI));
    
                //fprintf(gpFile, "%f, %f, %f, %f\n", curveXPoint, curveYPoint, nextCurveXPoint, nextCurveYPoint);
                //fprintf(gpFile, "%f, %f\n", dy, dx);
                //fprintf(gpFile, "jetRotation : %f\n", jetRotation);
    
                t = t + CURVE_PRECISION;
            }
            else{
                JET_PATH = STRAIGHT;
                t = 0.0f;
            }
        }
    
        if(JET_PATH == STRAIGHT){
            curveXPoint = curveXPoint + JETS_SPEED_ON_STRAIGHT_PROJECTION;
            if(curveXPoint >= 100.0f){
                JET_PATH = CURVE2;
                p1.x    = 100.0f, 
                p1.y    = -20.0f;
                p2.x    = 200.0f;
                p2.y    =  -120.0f;
                pControl.x = 140.0f;
                pControl.y = -20.0f;
    
                curveXPoint = p1.x;
                curveYPoint = p1.y;
    
                t = 0.0f;
            }
            t = t + CURVE_PRECISION;
        }
    
        if(JET_PATH == CURVE2){
            if(t <= 1.0f){
                /* Current point */
                GLfloat xP1Control = (1.0f - t) * p1.x + t * pControl.x;
                GLfloat yP1Control = (1.0f - t) * p1.y + t * pControl.y;
        
                GLfloat xP2Control = t * p2.x + (1.0f - t) * pControl.x;
                GLfloat yP2Control = t * p2.y + (1.0f - t) * pControl.y;
        
                curveXPoint = (1.0f - t) * xP1Control + t * xP2Control;
                curveYPoint = (1.0f - t) * yP1Control + t * yP2Control;
    
                GLfloat currentPointX = curveXPoint;
                GLfloat currentPointY = curveYPoint;
    
                /* Next Point */
                xP1Control = (1.0f - (t + CURVE_PRECISION)) * p1.x + (t + CURVE_PRECISION) * pControl.x;
                yP1Control = (1.0f - (t + CURVE_PRECISION)) * p1.y + (t + CURVE_PRECISION) * pControl.y;
        
                xP2Control = (t + CURVE_PRECISION) * p2.x + (1.0f - (t + CURVE_PRECISION)) * pControl.x;
                yP2Control = (t + CURVE_PRECISION) * p2.y + (1.0f - (t + CURVE_PRECISION)) * pControl.y;
        
                GLfloat nextCurveXPoint = (1.0f - (t + CURVE_PRECISION)) * xP1Control + (t + CURVE_PRECISION) * xP2Control;
                GLfloat nextCurveYPoint = (1.0f - (t + CURVE_PRECISION)) * yP1Control + (t + CURVE_PRECISION) * yP2Control;
    
                GLfloat nextPointX = nextCurveXPoint;
                GLfloat nextPointY = nextCurveYPoint;
    
                GLfloat dy = (nextPointY - currentPointY);
                GLfloat dx = (nextPointX - currentPointX);
                
                /* Fourth quadrant points result in negative angle with x axis, so 90 - (-angle) = 90 + angle with x axis = negative rotation to glRotate for correct */
                jetRotation = 90.0f - (atanf((dy) / (dx)) * (180.0f / GL_PI));
    
                t = t + CURVE_PRECISION;
            }
        }
    
        midJetPosX = midJetPosX + JETS_SPEED_ON_STRAIGHT_PROJECTION;
    
        /* 
            -68.838966
            -33.586815
            1.337936
            40.438107
            72.665070
            112.167480
        */
    
        if(midJetPosX >= -68.838966){
            colorB = True;
        }
        if(midJetPosX >= -33.586815){
            colorH = True;
        }
        if(midJetPosX >= 1.337936){
            colorA1 = True;
        }
        if(midJetPosX >= 40.438107){
            colorR = True;
        }
        if(midJetPosX >= 72.665070){
            colorA2 = True;
        }
        if(midJetPosX >= 112.167480){
            colorT = True;
        }
    }
}   

void drawBHARAT(){
    void drawB();
    void drawH();
    void drawA1();
    void drawR();
    void drawA2();
    void drawT();

    glLoadIdentity();
    glTranslatef(posB.x, posB.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawB();

    glLoadIdentity();
    glTranslatef(posH.x, posH.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawH();

    glLoadIdentity();
    glTranslatef(posA1.x, posA1.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawA1();

    glLoadIdentity();
    glTranslatef(posR.x, posR.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawR();

    glLoadIdentity();
    glTranslatef(posA2.x, posA2.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawA2();

    glLoadIdentity();
    glTranslatef(posT.x, posT.y, 0.0f);
    glScalef(1.5f, 1.5f, 0.0f);
    drawT();
}

void drawB(){
    glBegin(GL_QUADS);
        colorB ? glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-70.0f, 15.0f, 0.0f);
        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-70.0f, 1.0f, 0.0f);
        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, 1.0f, 0.0f);
        colorB ?  glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, 15.0f, 0.0f);

        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-70.0f, 1.0f, 0.0f);
        glVertex3f(-70.0f, -1.0f, 0.0f);
        glVertex3f(-64.0f, -1.0f, 0.0f);
        glVertex3f(-64.0f, 1.0f, 0.0f);

        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-70.0f, -1.0f, 0.0f);
        colorB ?  glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-70.0f, -15.0f, 0.0f);
        colorB ?  glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, -15.0f, 0.0f);
        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorB ?  glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, 15.0f, 0.0f);
        glVertex3f(-64.0f, 9.0f, 0.0f);
        glVertex3f(-56.0f, 9.0f, 0.0f);
        glVertex3f(-56.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, 3.0f, 0.0f);
        glVertex3f(-64.0f, -3.0f, 0.0f);
        glVertex3f(-56.0f, -3.0f, 0.0f);
        glVertex3f(-56.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorB ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-64.0f, -15.0f, 0.0f);
        glVertex3f(-56.0f, -15.0f, 0.0f);
        glVertex3f(-56.0f, -9.0f, 0.0f);
        glVertex3f(-64.0f, -9.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        colorB ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-56.0f, 15.0f, 0.0f);
        colorB ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-56.0f, 0.0f, 0.0f);
        colorB ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-50.0f, 6.0f, 0.0f);
        colorB ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-50.0f, 12.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        colorB ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-56.0f, 0.0f, 0.0f);
        colorB ?  glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-56.0f, -15.0f, 0.0f);
        colorB ?  glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-50.0f, -12.0f, 0.0f);
        colorB ?  glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-50.0f, -3.0f, 0.0f);
    glEnd();
}

void drawH(){
    glBegin(GL_QUADS);
        colorH ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-46.0f, 15.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-46.0f, 1.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-40.0f, 1.0f, 0.0f);
        colorH ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-40.0f, 15.0f, 0.0f);

        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-46.0f, 1.0f, 0.0f);
        glVertex3f(-46.0f, -1.0f, 0.0f);
        glVertex3f(-40.0f, -1.0f, 0.0f);
        glVertex3f(-40.0f, 1.0f, 0.0f);

        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-46.0f, -1.0f, 0.0f);
        colorH ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-46.0f, -15.0f, 0.0f);
        colorH ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-40.0f, -15.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-40.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-40.0f, 3.0f, 0.0f);
        glVertex3f(-40.0f, -3.0f, 0.0f);
        glVertex3f(-32.0f, -3.0f, 0.0f);
        glVertex3f(-32.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorH ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-32.0f, 15.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-32.0f, 1.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-26.0f, 1.0f, 0.0f);
        colorH ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-26.0f, 15.0f, 0.0f);

        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-32.0f, 1.0f, 0.0f);
        glVertex3f(-32.0f, -1.0f, 0.0f);
        glVertex3f(-26.0f, -1.0f, 0.0f);
        glVertex3f(-26.0f, 1.0f, 0.0f);

        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-32.0f, -1.0f, 0.0f);
        colorH ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-32.0f, -15.0f, 0.0f);
        colorH ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-26.0f, -15.0f, 0.0f);
        colorH ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-26.0f, -1.0f, 0.0f);
    glEnd();
}

void drawA1(){
    glBegin(GL_QUADS);
        colorA1 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-22.0f, 15.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-22.0f, 1.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, 1.0f, 0.0f);
        colorA1 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, 15.0f, 0.0f);

        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-22.0f, 1.0f, 0.0f);
        glVertex3f(-22.0f, -1.0f, 0.0f);
        glVertex3f(-16.0f, -1.0f, 0.0f);
        glVertex3f(-16.0f, 1.0f, 0.0f);

        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-22.0f, -1.0f, 0.0f);
        colorA1 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-22.0f, -15.0f, 0.0f);
        colorA1 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, -15.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA1 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, 15.0f, 0.0f);
        glVertex3f(-16.0f, 9.0f, 0.0f);
        glVertex3f(-8.0f, 9.0f, 0.0f);
        glVertex3f(-8.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-16.0f, 3.0f, 0.0f);
        glVertex3f(-16.0f, -3.0f, 0.0f);
        glVertex3f(-8.0f, -3.0f, 0.0f);
        glVertex3f(-8.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA1 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-8.0f, 15.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-8.0f, 1.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-2.0f, 1.0f, 0.0f);
        colorA1 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(-2.0f, 15.0f, 0.0f);

        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-8.0f, 1.0f, 0.0f);
        glVertex3f(-8.0f, -1.0f, 0.0f);
        glVertex3f(-2.0f, -1.0f, 0.0f);
        glVertex3f(-2.0f, 1.0f, 0.0f);

        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-8.0f, -1.0f, 0.0f);
        colorA1 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-8.0f, -15.0f, 0.0f);
        colorA1 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(-2.0f, -15.0f, 0.0f);
        colorA1 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(-2.0f, -1.0f, 0.0f);
    glEnd();
}

void drawR(){
    glBegin(GL_QUADS);
        colorR ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(2.0f, 15.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(2.0f, 1.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, 1.0f, 0.0f);
        colorR ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, 15.0f, 0.0f);

        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(2.0f, 1.0f, 0.0f);
        glVertex3f(2.0f, -1.0f, 0.0f);
        glVertex3f(8.0f, -1.0f, 0.0f);
        glVertex3f(8.0f, 1.0f, 0.0f);

        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(2.0f, -1.0f, 0.0f);
        colorR ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(2.0f, -15.0f, 0.0f);
        colorR ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, -15.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorR ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, 15.0f, 0.0f);
        glVertex3f(8.0f, 9.0f, 0.0f);
        glVertex3f(16.0f, 9.0f, 0.0f);
        glVertex3f(16.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, 3.0f, 0.0f);
        glVertex3f(8.0f, -3.0f, 0.0f);
        glVertex3f(16.0f, -3.0f, 0.0f);
        glVertex3f(16.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
        colorR ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(16.0f, 15.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(16.0f, -3.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(22.0f, 0.0f, 0.0f);
        colorR ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(22.0f, 12.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(8.0f, -3.0f, 0.0f);
        colorR ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(15.0f, -15.0f, 0.0f);
        colorR ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(22.0f, -15.0f, 0.0f);
        colorR ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(16.0f, -3.0f, 0.0f);
    glEnd();
}

void drawA2(){
    glBegin(GL_QUADS);
        colorA2 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(26.0f, 15.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(26.0f, 1.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, 1.0f, 0.0f);
        colorA2 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, 15.0f, 0.0f);

        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(26.0f, 1.0f, 0.0f);
        glVertex3f(26.0f, -1.0f, 0.0f);
        glVertex3f(32.0f, -1.0f, 0.0f);
        glVertex3f(32.0f, 1.0f, 0.0f);

        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(26.0f, -1.0f, 0.0f);
        colorA2 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(26.0f, -15.0f, 0.0f);
        colorA2 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, -15.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA2 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, 15.0f, 0.0f);
        glVertex3f(32.0f, 9.0f, 0.0f);
        glVertex3f(40.0f, 9.0f, 0.0f);
        glVertex3f(40.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(32.0f, 3.0f, 0.0f);
        glVertex3f(32.0f, -3.0f, 0.0f);
        glVertex3f(40.0f, -3.0f, 0.0f);
        glVertex3f(40.0f, 3.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorA2 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(40.0f, 15.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(40.0f, 1.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(46.0f, 1.0f, 0.0f);
        colorA2 ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(46.0f, 15.0f, 0.0f);

        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(40.0f, 1.0f, 0.0f);
        glVertex3f(40.0f, -1.0f, 0.0f);
        glVertex3f(46.0f, -1.0f, 0.0f);
        glVertex3f(46.0f, 1.0f, 0.0f);

        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(40.0f, -1.0f, 0.0f);
        colorA2 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(40.0f, -15.0f, 0.0f);
        colorA2 ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(46.0f, -15.0f, 0.0f);
        colorA2 ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(46.0f, -1.0f, 0.0f);
    glEnd();
}

void drawT(){
    glBegin(GL_QUADS);
        colorT ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(57.0f, 15.0f, 0.0f);
        colorT ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(57.0f, 1.0f, 0.0f);
        colorT ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(63.0f, 1.0f, 0.0f);
        colorT ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(63.0f, 15.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorT ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(57.0f, 1.0f, 0.0f);
        glVertex3f(57.0f, -1.0f, 0.0f);
        glVertex3f(63.0f, -1.0f, 0.0f);
        glVertex3f(63.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorT ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(57.0f, -1.0f, 0.0f);
        colorT ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(57.0f, -15.0f, 0.0f);
        colorT ?   glColor3fv(greenColor) : glColor3fv(grayColor);
        glVertex3f(63.0f, -15.0f, 0.0f);
        colorT ?   glColor3fv(whiteColor) : glColor3fv(grayColor);
        glVertex3f(63.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        colorT ?   glColor3fv(orangeColor) : glColor3fv(grayColor);
        glVertex3f(50.0f, 15.0f, 0.0f);
        glVertex3f(50.0f, 9.0f, 0.0f);
        glVertex3f(70.0f, 9.0f, 0.0f);
        glVertex3f(70.0f, 15.0f, 0.0f);
    glEnd();
}

void drawJet(GLfloat* tailColor){
    glColor3f(0.5f, 0.5f, 0.5f);

    GLfloat xP1, xP2, yP1, yP2, xControl, yControl, radius;

    // Body
    {
        xP1 = -5.0f;
        yP1 = 30.0f;

        xP2 = 5.0f;
        yP2 = 30.0f;

        xControl = 0.0f;
        yControl = 45.0f;

        glColor3ub(78, 136, 122);
        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-5.0f, 30.0f, 0.0f);
            glVertex3f(-5.0f, -30.0f, 0.0f);
            glVertex3f(5.0f, -30.0f, 0.0f);
            glVertex3f(5.0f, 30.0f, 0.0f);
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-5.0f, -30.0f, 0.0f);
            glVertex3f(-5.0f, -50.0f, 0.0f);
            glVertex3f(5.0f, -50.0f, 0.0f);
            glVertex3f(5.0f, -30.0f, 0.0f);
        glEnd();

        /* Flame */
        glBegin(GL_TRIANGLES);
            glColor3fv(tailColor);
            glVertex3f(-5.0f, -50.0f, 0.0f);
            GLint animateY = ((int)(t * (1.0f / CURVE_PRECISION))) % 5;
            animateY == 0 ? glVertex3f(0.0f, -70.0f, 0.0f) : glVertex3f(0.0f, -50.0f, 0.0f);
            glVertex3f(5.0f, -50.0f, 0.0f);
        glEnd();

        xP1 = -5.0f;
        yP1 = -50.0f;

        xP2 = 5.0f;
        yP2 = -50.0f;

        xControl = 0.0f;
        yControl = -55.0f;

        glColor3ub(78, 136, 122);
        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();
    }

    // Right wing
    {
        glColor3ub(33,86,72);
        glBegin(GL_QUADS);
            glVertex3f(15.0f, 10.0f, 0.0f);
            glVertex3f(15.0f, -15.0f, 0.0f);
            glVertex3f(20.0f, -15.0f, 0.0f);
            glVertex3f(20.0f, 10.0f, 0.0f);
        glEnd();

        xP1 = 15.0f;
        yP1 = -15.0f;

        xP2 = 20.0f;
        yP2 = -15.0f;

        xControl = 17.5f;
        yControl = -20.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(25.0f, 2.0f, 0.0f);
            glVertex3f(25.0f, -20.0f, 0.0f);
            glVertex3f(30.0f, -20.0f, 0.0f);
            glVertex3f(30.0f, 2.0f, 0.0f);
        glEnd();

        xP1 = 25.0f;
        yP1 = -20.0f;

        xP2 = 30.0f;
        yP2 = -20.0f;

        xControl = 27.5f;
        yControl = -25.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        
        glColor3ub(55, 113, 99);
        /* Right wing */
        glBegin(GL_QUADS);
            glVertex3f(5.0f, 10.0f, 0.0f);
            glVertex3f(5.0f, -10.0f, 0.0f);
            glVertex3f(40.0f, -15.0f, 0.0f);
            glVertex3f(40.0f, -10.0f, 0.0f);
        glEnd();

        // Flag
        glColor3fv(orangeColor);
        radius = 4.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(whiteColor);
        radius = 2.5f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(greenColor);
        radius = 1.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) + 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();
        glColor3ub(55, 113, 99);
    }
    
    // Left wing
    {
        glColor3ub(33,86,72);
        glBegin(GL_QUADS);
            glVertex3f(-15.0f, 10.0f, 0.0f);
            glVertex3f(-15.0f, -15.0f, 0.0f);
            glVertex3f(-20.0f, -15.0f, 0.0f);
            glVertex3f(-20.0f, 10.0f, 0.0f);
        glEnd();

        xP1 = -15.0f;
        yP1 = -15.0f;

        xP2 = -20.0f;
        yP2 = -15.0f;

        xControl = -17.5f;
        yControl = -20.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
            glVertex3f(-25.0f, 2.0f, 0.0f);
            glVertex3f(-25.0f, -20.0f, 0.0f);
            glVertex3f(-30.0f, -20.0f, 0.0f);
            glVertex3f(-30.0f, 2.0f, 0.0f);
        glEnd();

        xP1 = -25.0f;
        yP1 = -20.0f;

        xP2 = -30.0f;
        yP2 = -20.0f;

        xControl = -27.5f;
        yControl = -25.0f;

        glBegin(GL_POLYGON);
        for(GLfloat t = 0.0f; t <= 1.0f; t = t + CURVE_PRECISION){
            GLfloat xP1Control = (1 - t) * xP1 + t * xControl;
            GLfloat yP1Control = (1 - t) * yP1 + t * yControl;

            GLfloat xP2Control = t * xP2 + (1 - t) * xControl;
            GLfloat yP2Control = t * yP2 + (1 - t) * yControl;

            GLfloat curveXPoint = (1 - t) * xP1Control + t * xP2Control;
            GLfloat curveYPoint = (1 - t) * yP1Control + t * yP2Control;

            glVertex3f(curveXPoint, curveYPoint, 0.0f);
        }
        glEnd();

        glColor3ub(55, 113, 99);
        /* Left wing */
        glBegin(GL_QUADS);
            glVertex3f(-5.0f, 10.0f, 0.0f);
            glVertex3f(-5.0f, -10.0f, 0.0f);
            glVertex3f(-40.0f, -15.0f, 0.0f);
            glVertex3f(-40.0f, -10.0f, 0.0f);
        glEnd();

        // Flag
        glColor3fv(orangeColor);
        radius = 4.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(whiteColor);
        radius = 2.5f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();

        glColor3fv(greenColor);
        radius = 1.0f;
        glBegin(GL_POLYGON);
            for(GLfloat angle = 0.0f ; angle <= (360.0f); angle = angle + 1.0f){
                glVertex3f((radius * cos(DEG_TO_RAD(angle)) - 17.0f), (radius * sin(DEG_TO_RAD(angle))) - 4.0f, 0.0f);
            }
        glEnd();
        glColor3ub(55, 113, 99);
    }

    /* Left lower wing */ 
    {
        glBegin(GL_QUADS);
            glVertex3f(-5.0f, -40.0f, 0.0f);
            glVertex3f(-20.0f, -50.0f, 0.0f);
            glVertex3f(-20.0f, -53.0f, 0.0f);
            glVertex3f(-5.0f, -50.0f, 0.0f);
        glEnd();
    }

    /* Right lower wing */
    {
        glBegin(GL_QUADS);
            glVertex3f(5.0f, -40.0f, 0.0f);
            glVertex3f(20.0f, -50.0f, 0.0f);
            glVertex3f(20.0f, -53.0f, 0.0f);
            glVertex3f(5.0f, -50.0f, 0.0f);
        glEnd();
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

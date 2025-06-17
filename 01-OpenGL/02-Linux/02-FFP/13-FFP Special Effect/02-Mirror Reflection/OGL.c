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

#include <SOIL/SOIL.h>

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

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define MAX_LIMIT_CUBE_TRANSLATE_X  8.0f
#define MIN_LIMIT_CUBE_TRANSLATE_X -8.0f

#define MAX_LIMIT_CUBE_TRANSLATE_Y  4.0f
#define MIN_LIMIT_CUBE_TRANSLATE_Y  -4.0f

#define CLOSETS_LIMIT_CUBE_TRANSLATE_Z  20.0f
#define FARTHEST_LIMIT_CUBE_TRANSLATE_Z 3.0f

#define INCREMENT 0.1f
#define DECREMENT 0.1f


/* Rotation angle variables */
GLfloat angleCube = 0.0f;

/* Variable for lights */
GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 5.0f, 10.0f, 1.0f};

GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialDiffuse[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;

//BOOL bLight = FALSE;

/* Special effect related global variables */
GLuint textureMarble;

GLfloat translateCubeX = 0.0f;
GLfloat translateCubeY = 0.0f;
GLfloat translateCubeZ = 2.0f;

GLfloat cubeScale = 0.50f;

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
    XStoreName(gpDisplay, window, "Akash Musale - RTR6 - Special Effect - Mirror Reflection");

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

                    case XK_Right:
                        if(translateCubeX < MAX_LIMIT_CUBE_TRANSLATE_X){
                            translateCubeX = translateCubeX + INCREMENT;
                        }
                    break;

                    case XK_Left:
                        if(translateCubeX > MIN_LIMIT_CUBE_TRANSLATE_X){
                            translateCubeX = translateCubeX - DECREMENT;
                        }
                    break;

                    case XK_Up:
                        if(translateCubeY < MAX_LIMIT_CUBE_TRANSLATE_Y){
                            translateCubeY = translateCubeY + INCREMENT;
                        }
                    break;

                    case XK_Down:
                        if(translateCubeY > MIN_LIMIT_CUBE_TRANSLATE_Y){
                            translateCubeY = translateCubeY - DECREMENT;
                        }
                    break;

                    /* Cube Towards viewer */
                    case XK_Next:
                        if(translateCubeZ < CLOSETS_LIMIT_CUBE_TRANSLATE_Z){
                            translateCubeZ = translateCubeZ + INCREMENT;
                        }
                    break;

                    /* Cube Away from viewer */
                    case XK_Prior:
                        if(translateCubeZ > FARTHEST_LIMIT_CUBE_TRANSLATE_Z){
                            translateCubeZ = translateCubeZ - DECREMENT;
                        }
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
    Bool loadGLTexture(GLuint* texture, const char* path);

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

    /* Load Textures */
    if(!loadGLTexture(&textureMarble, "marble.bmp")){
        fprintf(gpFile, "loadGLTexture Failed to Load Marble Texture\n");
    }

     /* Enable Texturing */
    glEnable(GL_TEXTURE_2D);

    /* Light Configuration */
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

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

Bool loadGLTexture(GLuint* texture, const char* path){
    int width, height;
    unsigned char* imageData = SOIL_load_image(path, &width, &height, NULL, SOIL_LOAD_RGB);
    
    if (imageData == NULL){
        fprintf(gpFile, "Failed to load image: %s\n", SOIL_last_result());
        return False;
    }

    int rowSize = width * 3; // 3 bytes for RGB
    unsigned char* tempRow = (unsigned char*)malloc(rowSize);
    if (!tempRow) {
        fprintf(gpFile, "Memory allocation failed for tempRow\n");
        SOIL_free_image_data(imageData);
        return False;
    }

    for (int y = 0; y < height / 2; ++y) {
        unsigned char* rowTop = imageData + y * rowSize;
        unsigned char* rowBottom = imageData + (height - y - 1) * rowSize;

        memcpy(tempRow, rowTop, rowSize);
        memcpy(rowTop, rowBottom, rowSize);
        memcpy(rowBottom, tempRow, rowSize);
    }

    free(tempRow);

    // Generate OpenGL texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(imageData);

    return True;
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
    void drawLitCube();
    void drawFloor();
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Set matrix to model view mode
    glMatrixMode(GL_MODELVIEW);

    {
        // Set to identity matrix
        glLoadIdentity();

        // Setup camera
        gluLookAt(-5.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

        // Render the actual cube
        glPushMatrix();
            // Translate triangle backwards by z
            glTranslatef(translateCubeX, translateCubeY, translateCubeZ);

            glScalef(cubeScale, cubeScale, cubeScale);

            // Rotate triangle along x,y,z
            glRotatef(angleCube, 1.0f, 0.0f, 0.0f);
            glRotatef(angleCube, 0.0f, 1.0f, 0.0f);
            glRotatef(angleCube, 0.0f, 0.0f, 1.0f);

            drawLitCube(); 
        glPopMatrix();

        int stencilBits;
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        fprintf(gpFile, "Stencil Bits %d\n", stencilBits);

        // Enable stencil test
        glEnable(GL_STENCIL_TEST);

        // Since we are not rendering anything, disable depth test and color masks
        glDisable(GL_DEPTH_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // Always pass the stencil test
        glStencilFunc(GL_ALWAYS, 1, 1);

        // What should I do when stencil test passes
        // If both stencil and depth passed, replace all value in stencil buffer with 2nd param of stencilFun
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // Define floor as stencil
        drawFloor();

        glEnable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Draw only where there is 1 stored in stencil buffer
        glStencilFunc(GL_EQUAL, 1, 1);

        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Render the reflected cube
        glPushMatrix();
            glScalef(1.0f, 1.0f, -1.0f);

            // Translate triangle backwards by z
            glTranslatef(translateCubeX, translateCubeY, translateCubeZ);

            glScalef(cubeScale, cubeScale, cubeScale);

            // Rotate triangle along x,y,z
            glRotatef(angleCube, 1.0f, 0.0f, 0.0f);
            glRotatef(angleCube, 0.0f, 1.0f, 0.0f);
            glRotatef(angleCube, 0.0f, 0.0f, 1.0f);

            drawLitCube(); 
        glPopMatrix();

        glDisable(GL_STENCIL_TEST);

        glEnable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawFloor();
        glDisable(GL_BLEND);
    }
    
    // Swap the buffers
    glXSwapBuffers(gpDisplay, window);
}

void drawFloor(){
    glPushMatrix(); 
    {
        glTranslatef(0.0f, 0.0f, 0.0f);
        glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
        glScalef(3.0f, 3.0f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, textureMarble);
        glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 0.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glPopMatrix();
}

void drawLitCube(){
    glEnable(GL_LIGHTING);
    glBegin(GL_QUADS);
        /* Front Face */
        {
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
        }

        /* Right Face */
        {
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
        }

        /* Back Face */
        {
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
        }

        /* Left Face */
        {
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);
        }

        /* Top Face */
        {
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
        }

        /* Bottom Face */
        {
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
        }
    glEnd();
    glDisable(GL_LIGHTING);
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

    if(textureMarble){
        glDeleteTextures(1, &textureMarble);
        textureMarble = 0;
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

// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

//Xlib header files
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

Display *gDisplay = NULL;
XVisualInfo gXVisualInfo; // Like device context in Windows
Window window;
Colormap colormap;

Bool gbFullScreen = False;

/* Variable related to File I/O */
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

int main(void){
    // Function declarations
    void toggleFullScreen(void);
    void uninitialize(void);

    // Variable declarations
    int defaultScreen;
    int defaultDepth;
    Status status;
    XSetWindowAttributes windowAttributes;
    Atom windowManagerDeleteAtom;
    XEvent event;

    Screen* screen = NULL;
    int screenWidth, screenHeight;

    KeySym keySym;
    char keys[26]; // Although we need only zeroth index, conventionally the array size = number of alphabets


    // Create Log File
    gpFile = fopen(gszLogFileName, "w");
    if(gpFile == NULL){
        printf("Log File Creation Failed");
        exit(0);
    }
    else{
        fprintf(gpFile, "Program Started Successfully!\n");
        fflush(gpFile);
    }

    // Open the connection to the X server
    gDisplay = XOpenDisplay(NULL);
    if (gDisplay == NULL) {
        printf("ERROR: Unable to open X display.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // Create the default screen object
    defaultScreen = XDefaultScreen(gDisplay);

    // Get default depth
    defaultDepth = XDefaultDepth(gDisplay, defaultScreen);

    memset(&gXVisualInfo, 0, sizeof(gXVisualInfo));

    // Get visual information
    status = XMatchVisualInfo(gDisplay, defaultScreen, defaultDepth, TrueColor, &gXVisualInfo); 
    if (status == 0) {
        printf("ERROR: Unable to get visual info.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // Set the window attributes
    memset(&windowAttributes, 0, sizeof(windowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixmap = 0;
    windowAttributes.background_pixel = XBlackPixel(gDisplay, gXVisualInfo.screen);
    windowAttributes.colormap = XCreateColormap(gDisplay, XRootWindow(gDisplay, gXVisualInfo.screen), gXVisualInfo.visual, AllocNone);
    windowAttributes.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | FocusChangeMask; // PointerMotionMask is for mouse motion events
    colormap = windowAttributes.colormap;

    // Create the window
    window = XCreateWindow(gDisplay, XRootWindow(gDisplay, gXVisualInfo.screen),
                            0, 0, WIN_WIDTH, WIN_HEIGHT, 0,
                            gXVisualInfo.depth, InputOutput,
                            gXVisualInfo.visual,
                            CWBorderPixel | CWBackPixel | CWEventMask | CWColormap,
                            &windowAttributes);
    if(!window) {
        printf("ERROR: Unable to create window.\n");
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // Create atom for window manager delete message to destroy the window
    windowManagerDeleteAtom = XInternAtom(gDisplay, "WM_DELETE_WINDOW", True);

    // Set the WM_DELETE_WINDOW protocol
    XSetWMProtocols(gDisplay, window, &windowManagerDeleteAtom, 1);

    // Set window title
    XStoreName(gDisplay, window, "Akash Musale - RTR6 - LogFile");

    // Map the window to screen to show it
    XMapWindow(gDisplay, window);

    // Centering the window
    // Changing of x and y in XCreateWindow like we did in win32 will not work here, because it is managed by the window manager.
    // So let the window manager decide the position, and then center it manually using API .
    screen = XScreenOfDisplay(gDisplay, gXVisualInfo.screen);
    screenWidth = XWidthOfScreen(screen);
    screenHeight = XHeightOfScreen(screen);
    XMoveWindow(gDisplay, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);

    // Message loop
    while (1) {
        // Wait for an event
        XNextEvent(gDisplay, &event);

        // Handle the event
        switch (event.type) {
            case MapNotify:
                printf("Window mapped\n");
            break;
            
            case FocusIn:
                printf("Window focused\n"); 
                break;

            case FocusOut:
                printf("Window unfocused\n");
            break;

            case ConfigureNotify:
                printf("Window resized or moved: %dx%d at (%d, %d)\n", event.xconfigure.width, event.xconfigure.height, event.xconfigure.x, event.xconfigure.y);
            break;

            case KeyPress:
                keySym = XkbKeycodeToKeysym(gDisplay, event.xkey.keycode, 0, 0);
                switch (keySym){
                    case XK_Escape:
                        uninitialize();
                        exit(EXIT_SUCCESS);
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
                        printf("Left button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    case Button2:
                        printf("Middle button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    case Button3:
                        printf("Right button clicked at (%d, %d)\n", event.xbutton.x, event.xbutton.y);
                    break;
                    default:
                        printf("Other button %d clicked at (%d, %d)\n", event.xbutton.button, event.xbutton.x, event.xbutton.y);
                    break;
                }
                break;

            case Expose:
                printf("Window exposed\n");
            break; 

            case ClientMessage:
                printf("Window destroyed\n");
                uninitialize();
                exit(EXIT_SUCCESS);
            break;

            default:
                break;
        }
    }

    uninitialize();
    return 0;
}

void toggleFullScreen(void){
    Atom windowManagerNormalStateAtom = XInternAtom(gDisplay, "_NET_WM_STATE", False);
    Atom windowManagerFullScreenStateAtom = XInternAtom(gDisplay, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent event;
    memset((void*)&event, 0, sizeof(XEvent));
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = windowManagerNormalStateAtom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = gbFullScreen ? 1 : 0; // 1 for fullscreen, 0 for normal
    event.xclient.data.l[1] = windowManagerFullScreenStateAtom;
    
    XSendEvent(gDisplay, 
            XRootWindow(gDisplay, gXVisualInfo.screen),
            False, 
            SubstructureNotifyMask, 
            &event);

    XFlush(gDisplay);
    printf("Toggled fullscreen mode: %s\n", gbFullScreen ? "ON" : "OFF");
}

void uninitialize(void){
    /* Close the file */
    if(gpFile != NULL){
        fprintf(gpFile, "Program Terminated Successfully!\n");
        fclose(gpFile);
        gpFile = NULL;
    }

    // Destroy the window
    if (window) {
        XDestroyWindow(gDisplay, window);
        window = 0;
    }

    // Free the colormap
    if (colormap) {
        XFreeColormap(gDisplay, colormap);
        colormap = 0;
    }

    // Close the display connection
    if (gDisplay) {
        XCloseDisplay(gDisplay);
        gDisplay = NULL;
    }
}

// Compile with: gcc -o XWindow XWindow.c -lX11

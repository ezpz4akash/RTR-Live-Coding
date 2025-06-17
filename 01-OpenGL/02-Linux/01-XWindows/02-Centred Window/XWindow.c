// Standard libraries used: This project uses the standard C libraries, including stdio.h, stdlib.h, and memory.h.
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

//Xlib header files
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

Display *gDisplay = NULL;
XVisualInfo gXVisualInfo; // Like device context in Windows
Window window;
Colormap colormap;

int main(void){
    // Function declarations
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
    XStoreName(gDisplay, window, "Akash Musale - RTR6");

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
            /*case Expose:
                // Handle expose event
                printf("Window exposed\n");
                break;*/

            case ClientMessage:
                // Handle client message (like window close)
                //if (event.xclient.data.l[0] == windowManagerDeleteAtom) {
                    uninitialize();
                    exit(EXIT_SUCCESS);
                //}
                break;

            default:
                break;
        }
    }

    uninitialize();
    return 0;
}

void uninitialize(void){
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

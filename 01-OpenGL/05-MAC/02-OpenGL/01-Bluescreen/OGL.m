#import <Foundation/Foundation.h> //like stdio.h
#import <Cocoa/Cocoa.h> // like Windows.h

#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

// global function declaration

CVReturn myDisplayLinkCallback(CVDisplayLinkRef, 
                               const CVTimeStamp *, 
                               const CVTimeStamp *, 
                               CVOptionFlags, 
                               CVOptionFlags*, 
                               void*);


//global variables
FILE *gpFile = NULL;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView : NSOpenGLView
-(void)uninitialize;
@end

int main(int argc, char** argv)
{
    //code 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    NSApp = [NSApplication sharedApplication]; //NSApp process created
    
    [NSApp setDelegate:[[AppDelegate alloc]init]];

    [NSApp run];

    [pool release];

    return (0);
}

@implementation AppDelegate
{
    @private
    NSWindow *window;
    GLView *glView;
}

// equivalent to WM_CREATE
-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    //code
    // create log file
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirPath = [appBundle bundlePath];
    NSString *parentDirPath = [appDirPath stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt", parentDirPath];
    const char *pszlogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile = fopen(pszlogFileNameWithPath, "w");
    if (gpFile == NULL)
    {
        //message box
        NSAlert *alert = [[NSAlert alloc]init];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert setMessageText:@ "Failed to create log file"];
        [alert addButtonWithTitle:@"Exit"];
        [alert runModal];
        [alert release];
        [self release];
        [NSApp terminate:self];
    }
    else
    {
        fprintf(gpFile, "Program started successfully\n");
    }

    NSRect winRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    window = [[NSWindow alloc]initWithContentRect:winRect
                                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                backing:NSBackingStoreBuffered
                                defer:NO];

    [window setTitle:@"Akash Musale"];
    [window center];

    glView = [[GLView alloc]initWithFrame:winRect];

    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];  // setfocus, WS_TOP
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    if (gpFile)
    {
        fprintf(gpFile, "Program terminated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
}

-(BOOL)windowShouldClose:(NSWindow *)aWindow
{
    //code
    [glView uninitialize];
    return (YES);
}

-(void)dealloc
{
    //code 
    [glView release];
    [window release];
    [super dealloc];
}

@end

@implementation GLView
{
    CVDisplayLinkRef displayLink;
}


-(id)initWithFrame:(NSRect)frame
{
    //code
    self = [super initWithFrame:frame];

    if (self)
    {
        [[self window]setContentView:self];

       
        // NSOpenGL pixel format initialiser
        
        NSOpenGLPixelFormatAttribute attributes[] =
        {
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask,
            CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize,24,
            NSOpenGLPFADepthSize,24,
            NSOpenGLPFAAlphaSize,8,
            NSOpenGLPFADoubleBuffer,   
            0
        };

        //create NSopenGL pixel format
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:attributes]autorelease];
 
        if (pixelFormat == nil)
        {
            fprintf(gpFile, "NSOpenGLPixel format creation failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            return(nil);
        }

        //create NSOpenGL conttext
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];

        //check if glContext is valid
        if (glContext == nil)
        {
            fprintf(gpFile, "NSOpenGLContext creation failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            return(nil);
        }
     

        //set view's pixel format with created pixel format
        [self setPixelFormat:pixelFormat];

        //set view's contect with created contect
        [self setOpenGLContext:glContext];
 
  
        [self setWantsLayer:YES];
        NSColor *backgColor = [NSColor blackColor];
        struct CGColor *bgColor = [backgColor CGColor];
        [[self layer]setBackgroundColor:bgColor];
    }

    return(self);    
}

-(CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime
{
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

    [self drawView];

    [pool release];

    return(kCVReturnSuccess);
}

-(void)prepareOpenGL
{
    //code
    [super prepareOpenGL];

    // make opengl context as current contxt
    [[self openGLContext] makeCurrentContext];

    //set swap interval to 1 to sync arrival of buffers in souble buffering to avoid tearing
    GLint swapInterval = 1;

    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    // now call our initialize here
    [self initialize];

    // start the display link to create separate rendering thread

    //step1  : create display link
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

    //step2 : setcallback function with display link
    CVDisplayLinkSetOutputCallback(displayLink, &myDisplayLinkCallback, self);

    //step 3
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];

    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj];

    //step 5
    //set above pixel format and context as current display's pixel format and context for displaylink
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    // step 6 start the display link
    CVDisplayLinkStart(displayLink);
    
}

-(void)reshape
{
    //code
    [super reshape];

    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    NSRect viewRect = [self bounds];
    int width = (int)viewRect.size.width;
    int height = (int)viewRect.size.height;
    [self resize:width height:height];

    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

}

-(void)drawView
{
    //code

    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    [self display];
    [self myUpdate];

    //swap buffers
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);

    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

// WM_PAINT
-(void)drawRect:(NSRect)dirtyRect
{
    //code 
    [self drawView]; // to avoid possibility of flickering
}

-(BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];

    return(YES);
}

-(void)keyDown:(NSEvent *)event
{
    //code

    int key = (int)[[event characters]characterAtIndex:0];

    switch(key)
    {
        case 27:
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            break;

        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];
            break;

        default:
            break;

    }
}

-(void)mouseDown:(NSEvent *)event
{
    //code 
}

-(int) initialize
{
    //code

    // printf GL info
    [self printGLInfo];

    //from here onwards opengl code starts


    // depth related code
    glClearDepth(1.0f); // depth buffer to 1.0
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL); // pass the fragments whose values are less than are equal to glClear depth
 
    // tell openGl to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

        
    NSRect viewRect = [self bounds];
    int width = (int)viewRect.size.width;
    int height = (int)viewRect.size.height;
    [self resize:width height:height];

    return(0);
}

-(void)printGLInfo
{
    //code
    // printf opengl information
    fprintf(gpFile, "OPEN GL INFORMATION\n");
    fprintf(gpFile, "********************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(gpFile, "********************\n");
}

-(void)resize:(int)width height:(int)height
{
    //code
    // if height by accident becomes 0 or less then make height 1
    if (height <= 0)
    {
        height = 1;
    }
    // set the viewport
    glViewport(0, 0, (GLsizei)width, height);
    
}

-(void)display
{
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

-(void)myUpdate
{
    //code
}


-(void)uninitialize
{
    //code
    fprintf(gpFile, "Uninitializing\n");
}

-(void)dealloc
{
    //code
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}

@end

CVReturn myDisplayLinkCallback(CVDisplayLinkRef displayLinkRef, 
                               const CVTimeStamp *current, 
                               const CVTimeStamp *outputTime, 
                               CVOptionFlags inputflags, 
                               CVOptionFlags *outputFlags, 
                               void *view)
{
    //code
    CVReturn result = [(GLView *)view getFrameForTime:outputTime];

    return(result);
 
}
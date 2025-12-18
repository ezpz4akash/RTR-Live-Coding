#import <Foundation/Foundation.h> //like stdio.h
#import <Cocoa/Cocoa.h> // like Windows.h
#import <QuartzCore/CVDisplayLink.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import <stdio.h>

//global function declarations
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, 
                                const CVTimeStamp* now, 
                                const CVTimeStamp* outputTime, 
                                CVOptionFlags flagsIn, 
                                CVOptionFlags* flagsOut, 
                                void* view);

FILE* gpFile = NULL;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView : NSOpenGLView
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

    //Create log file
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirPath = [appBundle bundlePath];
    NSString *parentDirPath = [appDirPath stringByDeletingLastPathComponent];
    NSString *logFilePath = [NSString stringWithFormat:@"%@/Log.txt", parentDirPath];
    const char *logFileNameWithPath = [logFilePath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile = fopen(logFileNameWithPath, "w");
    if (gpFile == NULL)
    {
        NSAlert *alert = [[NSAlert alloc]init];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert setMessageText:@"Log File Creation Failed."];
        [alert addButtonWithTitle:@"Exit"];
        [alert runModal];
        [alert release];
        [self release];
        [NSApp terminate:self];
    }

    fprintf(gpFile, "Program Started Successfully. \n");

    NSRect winRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    window = [[NSWindow alloc]initWithContentRect:winRect
                                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                backing:NSBackingStoreBuffered
                                defer:NO];

    [window setTitle:@"AKM : macOS Window"];
    [window center];

    glView = [[GLView alloc]initWithFrame:winRect];

    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];  // setfocus, WS_TOP
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    fprintf(gpFile, "Program Terminated Successfully. \n");
    if (gpFile)
    {
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(bool)windowShouldClose:(NSWindow *)aWindow
{
    //code
    [glView uninitialize];
    return(YES);
}

-(void)windowWillClose:(NSNotification *)notification
{
    //code
    [glView uninitialize];
    [NSApp terminate:self];
}

-(void)dealloc
{
    //code 
    [glView release];
    [window release];
    [super dealloc];
}

@end

@interface GLView ()
-(int)initialize;
-(void)uninitialize;
-(void)resize:(int)width :(int)height;
-(void)printGLInfo;
-(void)myUpdate;
-(void)drawView;
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

        // NSOpenGLPixelFormat Initialization
        NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
        {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
            NSOpenGLPFANoRecovery,NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0
        };

        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:pixelFormatAttributes]autorelease];
        if (pixelFormat == nil)
        {
            fprintf(gpFile, "Error : Unable to allocate Pixel Format. \n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
        }

        //Create OpenGL Context
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];
        if (glContext == nil)
        {
            fprintf(gpFile, "Error : Unable to create OpenGL Context. \n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
        }

        [self setPixelFormat:pixelFormat];
        [self setOpenGLContext:glContext];
        
        [self setWantsLayer:YES];

        NSColor *backgColor = [NSColor blackColor];
        CGColorRef bgColor = [backgColor CGColor];
        [[self layer]setBackgroundColor:bgColor];
    }

    return(self);    
}

-(CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

    [self drawView];

    [pool release];

    return kCVReturnSuccess;
}

-(void)prepareOpenGL
{
    //code
    [super prepareOpenGL];

    //Make the OpenGL Context current
    [[self openGLContext]makeCurrentContext];

    // Set swap interval to 1 to synchronize buffer swaps to avoid tearing
    GLint swapInterval = 1;
    [[self openGLContext]setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    // Call our initialize, Initialize OpenGL
    int result = [self initialize];
    
    // Start the display link to separate the rendering thread from the main thread
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);

    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    CGLContextObj cglContext = ((CGLContextObj)[[self openGLContext]CGLContextObj]);

    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    CVDisplayLinkStart(displayLink);
}

-(void)reshape
{
    //code
    [super reshape];

    [[self openGLContext]makeCurrentContext];

    //Lock the OpenGL Context
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    NSRect viewRect = [self bounds];
    int width = (int)viewRect.size.width;
    int height = (int)viewRect.size.height;
    [self resize:width :height];

    //UnLock the OpenGL Context
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void)drawView
{
    //code
    [[self openGLContext]makeCurrentContext];

    //Lock the OpenGL Context
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    [self display];
    [self myUpdate];

    //Swap Buffers
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);

    //Unlock the OpenGL Context
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void)drawRect:(NSRect)dirtyRect
{
    //code 
    [self drawView];
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

//Stub function
-(int)initialize
{
    //code
    [self printGLInfo];

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue

    return (0);
}


-(void)printGLInfo
{
    //code
    GLint numExtensions, i;

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    // print openGL information
    fprintf(gpFile, "OPENGL INFORMATION\n");
    fprintf(gpFile, "******************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(gpFile, "Number of OpenGL Extensions : %d\n", numExtensions);
    fprintf(gpFile, "******************\n");

    // print all OpenGL extensions
    for(i = 0; i < numExtensions; i++){
        fprintf(gpFile, "OpenGL Extension %d : %s\n", i + 1, glGetStringi(GL_EXTENSIONS, i));
    }
}

-(void)resize:(int)width :(int)height
{
    //code
    if (height <= 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
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
    fprintf(gpFile, "Uninitialize Successfully. \n");
}

-(void)dealloc
{
    //code
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    
    [super dealloc];
}

@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, 
                                const CVTimeStamp* now, 
                                const CVTimeStamp* outputTime, 
                                CVOptionFlags flagsIn, 
                                CVOptionFlags* flagsOut, 
                                void* view)
{
    CVReturn result = [(GLView *)view getFrameForTime:outputTime];
    return (result);
}

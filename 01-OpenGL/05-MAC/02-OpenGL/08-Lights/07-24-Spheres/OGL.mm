#import <Foundation/Foundation.h> //like stdio.h
#import <Cocoa/Cocoa.h> // like Windows.h

#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import "Sphere.h"
#import "materials.h"

// global function declaration

CVReturn myDisplayLinkCallback(CVDisplayLinkRef, 
                               const CVTimeStamp *, 
                               const CVTimeStamp *, 
                               CVOptionFlags, 
                               CVOptionFlags*, 
                               void*);


//global variables
FILE *gpFile = NULL;

using namespace vmath;

enum {
  AMC_ATTRIBUTE_POSITION = 0,
  AMC_ATTRIBUTE_COLOR,
  AMC_ATTRIBUTE_NORMAL,
  AMC_ATTRIBUTE_TEXCOORD,
};

// lighting type
#define PER_VERTEX   (0)
#define PER_FRAGMENT (1)

// light rotation axis
#define X_AXIS (0)
#define Y_AXIS (1)
#define Z_AXIS (2)
#define NO_AXIS (3)

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
        [alert setMessageText:@"Failed to create log file"];
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
    [glView uninitialize];
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
    GLuint shaderProgramObjectPerFragment;
    GLuint shaderProgramObjectPerVertex;
    
    // vao, vbo
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;

    // Per-Fragment Light uniforms
    GLuint modelMatrixUniformPerFragment;
    GLuint viewMatrixUniformPerFragment;
    GLuint projectionMatrixUniformPerFragment;
    GLuint laUniformPerFragment;
    GLuint ldUniformPerFragment;
    GLuint lsUniformPerFragment;
    GLuint lightPositionUniformPerFragment;
    GLuint kaUniformPerFragment;
    GLuint kdUniformPerFragment;
    GLuint ksUniformPerFragment;
    GLuint materialShininessUniformPerFragment;
    GLuint LKeyPressedUniformPerFragment;

    // Per-Vertex Light uniforms
    GLuint modelMatrixUniformPerVertex;
    GLuint viewMatrixUniformPerVertex;
    GLuint projectionMatrixUniformPerVertex;
    GLuint laUniformPerVertex;
    GLuint ldUniformPerVertex;
    GLuint lsUniformPerVertex;
    GLuint lightPositionUniformPerVertex;
    GLuint kaUniformPerVertex;
    GLuint kdUniformPerVertex;
    GLuint ksUniformPerVertex;
    GLuint materialShininessUniformPerVertex;
    GLuint LKeyPressedUniformPerVertex;
    
    Sphere *sphere;
    int gNumElements;
    int gLightingType;
    
    GLfloat lightAngle;
    int gLightRotationAxis;

    mat4 perspectiveProjectionMatrix;

    //use inputs
    BOOL bLight;
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
    int retVal = [self initialize];
    if (retVal != 0)
    {
        fprintf(gpFile, "Initialization Failed\n");
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
        return;
    }

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

        case 'L':
        case 'l':
            bLight = !bLight;
            break;

        case 'V':
        case 'v':
            gLightingType = (gLightingType == PER_VERTEX) ? PER_FRAGMENT : PER_VERTEX;
            break;

        case 'X':
        case 'x':
            gLightRotationAxis = X_AXIS;
            break;

        case 'Y':
        case 'y':
            gLightRotationAxis = Y_AXIS;
            break;

        case 'Z':
        case 'z':
            gLightRotationAxis = Z_AXIS;
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

   // Step 1 Write the shader source code
    const GLchar *vertexShaderSourceCode = 
    "#version 410 core\n" \
    "in vec4 aPosition;\n" \
    "in vec3 aNormal;\n" \
    "out vec3 out_transformedNormals;\n" \
    "out vec3 out_lightDirection;\n" \
    "out vec3 out_viewerVector;\n" \
    "uniform mat4 uModelMatrix;\n" \
    "uniform mat4 uViewMatrix;\n" \
    "uniform mat4 uProjectionMatrix;\n" \
    "uniform vec4 uLightPosition;\n" \
    "uniform int uLkeyPressed;\n" \

    "void main(void)" \
    "{\n"\
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
    "       mat3 normalMatrix = mat3(uViewMatrix * uModelMatrix);\n" \
    "       out_transformedNormals = (normalMatrix * aNormal);\n" \
    "       out_lightDirection = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" \
    "       out_viewerVector = (-eyeCoordinates.xyz);\n"
    "   }\n"\
    "}\n";

    // Step 2. Create the shader object
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    // Step 3. Give shader source code to shader object
    // second para - numbers of shaders
    // 4th parameter - array of length of every shader
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

    // Step 4. Compile the shader
    glCompileShader(vertexShaderObject);

    // Step 5. Shader compilation error checking
    GLint status = 0;
    GLint infoLogLength = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                // 3rd param, - returned length
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log  = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // fragment shader
    const GLchar *fragmentShaderSourceCode = 
    "#version 410 core\n" \
    "out vec4 FragColor;\n" \
    "in vec3 out_transformedNormals;\n" \
    "in vec3 out_lightDirection;\n" \
    "in vec3 out_viewerVector;\n" \
    "uniform vec3 uLa;\n" \
    "uniform vec3 uLd;\n" \
    "uniform vec3 uLs;\n" \
    "uniform vec3 uKa;\n" \
    "uniform vec3 uKd;\n" \
    "uniform vec3 uKs;\n" \
    "uniform float uMaterialShininess;\n" \
    "uniform int uLkeyPressed;\n" \
    "vec3 fong_ads_light;\n" \

    "void main(void)" \
    "{\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
    "       vec3 normalized_transformedNormals = normalize(out_transformedNormals);\n"
    "       vec3 normalized_lightDirection = normalize(out_lightDirection);\n"
    "       vec3 normalized_viewerVector = normalize(out_viewerVector);\n"
    "       vec3 reflectionVector = reflect(-normalized_lightDirection, normalized_transformedNormals);\n"
    "       vec3 diffusedLight = uLd * uKd * max(dot(normalized_lightDirection, normalized_transformedNormals), 0.0);\n"
    "       vec3 ambientLight = uLa * uKa;\n"
    "       vec3 specularLight = uLs * uKs * pow(max(dot(reflectionVector, normalized_viewerVector),0.0), uMaterialShininess);\n"
    "       fong_ads_light = ambientLight + diffusedLight + specularLight;\n"
    "   }\n" \
    "   else \n"\
    "   {\n"\
    "       fong_ads_light = vec3(1.0f, 1.0f, 1.0f);\n"
    "   }\n" \
    "   FragColor = vec4(fong_ads_light, 1.0f);\n" \
    "}\n";

    GLuint fragmentShaderObject =  glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

    glCompileShader(fragmentShaderObject);

    status = 0;
    infoLogLength = 0;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Create per-fragment shader program objects
    shaderProgramObjectPerFragment = glCreateProgram();
    
    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObjectPerFragment, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObjectPerFragment, fragmentShaderObject);

    // bind shader attribute at certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObjectPerFragment, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObjectPerFragment, AMC_ATTRIBUTE_NORMAL, "aNormal");

    // link shader objects to shader program object
    glLinkProgram(shaderProgramObjectPerFragment);


    // check link error
    status = 0;
    infoLogLength = 0;

    glGetProgramiv(shaderProgramObjectPerFragment, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObjectPerFragment, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObjectPerFragment, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    // Get per-fragment uniforms
    modelMatrixUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uModelMatrix");
    viewMatrixUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uViewMatrix");
    projectionMatrixUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uProjectionMatrix");

    laUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uLa");
    ldUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uLd");
    lsUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uLs");
    lightPositionUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uLightPosition");

    kaUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uKa");
    kdUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uKd");
    ksUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uKs");
    materialShininessUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uMaterialShininess");

    LKeyPressedUniformPerFragment = glGetUniformLocation(shaderProgramObjectPerFragment, "uLkeyPressed");

    // ===== Per-Vertex Light Shader =====
    const GLchar *vertexShaderSourceCodePerVertex = 
    "#version 410 core\n" \
    "in vec4 aPosition;\n" \
    "in vec3 aNormal;\n" \
    "uniform mat4 uModelMatrix;\n" \
    "uniform mat4 uViewMatrix;\n" \
    "uniform mat4 uProjectionMatrix;\n" \
    "uniform vec3 uLa;\n" \
    "uniform vec3 uLd;\n" \
    "uniform vec3 uLs;\n" \
    "uniform vec4 uLightPosition;\n" \
    "uniform vec3 uKa;\n" \
    "uniform vec3 uKd;\n" \
    "uniform vec3 uKs;\n" \
    "uniform float uMaterialShininess;\n" \
    "uniform int uLkeyPressed;\n" \
    "out vec3 out_fong_ads_light;\n" \
    "void main(void)" \
    "{\n" \
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
    "   if (uLkeyPressed == 1) \n" \
    "   {\n" \
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
    "       mat3 normalMatrix = mat3(uViewMatrix * uModelMatrix);\n" \
    "       vec3 transformedNormals = normalize(normalMatrix * aNormal);\n" \
    "       vec3 lightDirection = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n" \
    "       vec3 diffusedLight = uLd * uKd * max(dot(lightDirection, transformedNormals), 0.0);\n" \
    "       vec3 ambientLight = uLa * uKa;\n" \
    "       vec3 reflectionVector = reflect(-lightDirection, transformedNormals);\n" \
    "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" \
    "       vec3 specularLight = uLs * uKs * pow(max(dot(reflectionVector, viewerVector),0.0), uMaterialShininess);\n" \
    "       out_fong_ads_light = ambientLight + diffusedLight + specularLight;\n" \
    "   }\n" \
    "   else \n" \
    "   {\n" \
    "       out_fong_ads_light = vec3(1.0f,1.0f,1.0f);\n" \
    "   }\n" \
    "}\n";

    GLuint vertexShaderObjectPerVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObjectPerVertex, 1, (const GLchar **)&vertexShaderSourceCodePerVertex, NULL);
    glCompileShader(vertexShaderObjectPerVertex);

    status = 0;
    infoLogLength = 0;
    szInfoLog = NULL;
    glGetShaderiv(vertexShaderObjectPerVertex, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(vertexShaderObjectPerVertex, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Per-Vertex Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Fragment shader for both (same)
    const GLchar *fragmentShaderSourceCodeSimple = 
    "#version 410 core\n" \
    "out vec4 FragColor;\n" \
    "in vec3 out_fong_ads_light;\n" \
    "void main(void)" \
    "{\n" \
    "   FragColor = vec4(out_fong_ads_light, 1.0f);\n" \
    "}\n";

    GLuint fragmentShaderObjectPerVertex = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObjectPerVertex, 1, (const GLchar **)&fragmentShaderSourceCodeSimple, NULL);
    glCompileShader(fragmentShaderObjectPerVertex);

    status = 0;
    infoLogLength = 0;
    glGetShaderiv(fragmentShaderObjectPerVertex, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObjectPerVertex, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Per-Vertex Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Create per-vertex shader program
    shaderProgramObjectPerVertex = glCreateProgram();
    glAttachShader(shaderProgramObjectPerVertex, vertexShaderObjectPerVertex);
    glAttachShader(shaderProgramObjectPerVertex, fragmentShaderObjectPerVertex);
    glBindAttribLocation(shaderProgramObjectPerVertex, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObjectPerVertex, AMC_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObjectPerVertex);

    status = 0;
    infoLogLength = 0;
    glGetProgramiv(shaderProgramObjectPerVertex, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObjectPerVertex, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObjectPerVertex, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Per-Vertex Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    // Get per-vertex uniforms
    modelMatrixUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uModelMatrix");
    viewMatrixUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uViewMatrix");
    projectionMatrixUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uProjectionMatrix");
    laUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uLa");
    ldUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uLd");
    lsUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uLs");
    lightPositionUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uLightPosition");
    kaUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uKa");
    kdUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uKd");
    ksUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uKs");
    materialShininessUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uMaterialShininess");
    LKeyPressedUniformPerVertex = glGetUniformLocation(shaderProgramObjectPerVertex, "uLkeyPressed");

    // Initialize lighting type
    gLightingType = PER_FRAGMENT;
    lightAngle = 0.0f;
    gLightRotationAxis = NO_AXIS;
    sphere = [[Sphere alloc]init];

    float spherePositionCoords[1146];
    float sphereNormalCoords[1146];
    float sphereTexCoords[764];
    unsigned short sphereElements[2280];
    
    [sphere getSphereVertexData:spherePositionCoords :sphereNormalCoords :sphereTexCoords :sphereElements];

    int numSphereVertices = [sphere getNumberOfSphereVertices];
    gNumElements= [sphere getNumberOfSphereElements];

    // vertex array object for arrays of vertex attributes
    glGenVertexArrays(1, &vao_sphere);

    glBindVertexArray(vao_sphere);

        //position
        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_position_sphere);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);

        //step3
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), spherePositionCoords, GL_STATIC_DRAW);

        // step4 tell GPU how to use buffer data
        // parameter 1. bind index
        // parameter 2. size - of vertex
        // parameter 3. Data type of vertex
        // paremeter 4. Normalised (is data normalised?)
        // parameter 5. stride 
        // paremeter 6. pointer only applicable if stride is provided else NULL
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        // step 5. enable the binding point/ target point
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        //normal
        // Step 1. craete buffer in GPU memory
        glGenBuffers(1, &vbo_normal_sphere);

        // Step2. bind the buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);

        //step3
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), sphereNormalCoords, GL_STATIC_DRAW);

        // step4 tell GPU how to use buffer data
        // parameter 1. bind index
        // parameter 2. size - of vertex
        // parameter 3. Data type of vertex
        // paremeter 4. Normalised (is data normalised?)
        // parameter 5. stride 
        // paremeter 6. pointer only applicable if stride is provided else NULL
        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        // step 5. enable the binding point/ target point
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        // element buffer
        glGenBuffers(1, &vbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, gNumElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);

    glBindVertexArray(0);

    // unbind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // depth related code
    glClearDepth(1.0f); // depth buffer to 1.0
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL); // pass the fragments whose values are less than are equal to glClear depth
 
    // tell openGl to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        
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

    // create perspective projection matrix
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);
    
}

-(void)display
{
    // variable declaration
    GLuint kaUniform, kdUniform, ksUniform, shininessUniform, modelMatrixUniform;
    
    // Light structure
    static struct {
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;
        vec4 position;
    } light;
    
    static BOOL initialized = FALSE;
    if (!initialized)
    {
        initialized = TRUE;
        light.ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        light.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        light.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        light.position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // transformations
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();

    mat4 lightTranslationMatrix = mat4::identity();

    switch (gLightRotationAxis)
    {
        case X_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngle, 1.0f, 0.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 20.0f, 20.0f);
            break;

        case Y_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngle, 0.0f, 1.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 0.0f, 20.0f);
            break;

        case Z_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngle, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 20.0f, 0.0f);
            break;
    
        default:
            lightTranslationMatrix = vmath::rotate(0.0f, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 0.0f, 20.0f);
            break;
    }

    if (gLightingType == PER_FRAGMENT)
    {
        //use per fragment shader program object
        glUseProgram(shaderProgramObjectPerFragment);

        modelMatrixUniform = modelMatrixUniformPerFragment;

        glUniformMatrix4fv(viewMatrixUniformPerFragment, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniformPerFragment, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform1i(LKeyPressedUniformPerFragment, bLight);

        glUniform3fv(laUniformPerFragment, 1, light.ambient);
        glUniform3fv(ldUniformPerFragment, 1, light.diffuse);
        glUniform3fv(lsUniformPerFragment, 1, light.specular);

        glUniform4fv(lightPositionUniformPerFragment, 1, light.position * lightTranslationMatrix.transpose());
  
        kaUniform = kaUniformPerFragment;
        kdUniform = kdUniformPerFragment;
        ksUniform = ksUniformPerFragment;
        shininessUniform = materialShininessUniformPerFragment;
    }
    else
    {
        //use per vertex shader program object
        glUseProgram(shaderProgramObjectPerVertex);

        modelMatrixUniform = modelMatrixUniformPerVertex;
        glUniformMatrix4fv(viewMatrixUniformPerVertex, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniformPerVertex, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform1i(LKeyPressedUniformPerVertex, bLight);

        glUniform3fv(laUniformPerVertex, 1, light.ambient);
        glUniform3fv(ldUniformPerVertex, 1, light.diffuse);
        glUniform3fv(lsUniformPerVertex, 1, light.specular);
        glUniform4fv(lightPositionUniformPerVertex, 1, light.position * lightTranslationMatrix.transpose());

        kaUniform = kaUniformPerVertex;
        kdUniform = kdUniformPerVertex;
        ksUniform = ksUniformPerVertex;
        shininessUniform = materialShininessUniformPerVertex;
    }

    // *** bind vao ***
    glBindVertexArray(vao_sphere);

    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);

    float xTranslate = -5.0f;
    for (unsigned int col = 0; col < 4; col++)
    {
        float yTranslate = 4.0f;
        for (unsigned int row = 0; row < 6; row++)
        {

            translationMatrix = vmath::translate(xTranslate, yTranslate, -13.0f);
            modelMatrix = translationMatrix;
            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniform3fv(kaUniform, 1, sphereMaterials[row + col*6].ambient);
            glUniform3fv(kdUniform, 1, sphereMaterials[row + col*6].diffuse);
            glUniform3fv(ksUniform, 1, sphereMaterials[row + col*6].specular);
            glUniform1f(shininessUniform, sphereMaterials[row + col*6].shininess[0]);
            
            glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
            yTranslate -= 1.66f;

        }
        xTranslate += 3.2f;
    }

    // *** unbind vao ***
    glBindVertexArray(0);

    //unuse shader peogram object
    glUseProgram(0);

}

-(void)myUpdate
{
    //code
    lightAngle = lightAngle + 1.0f;
}


-(void)uninitialize
{
    //code
    fprintf(gpFile, "Uninitializing\n");
    
    // free sphere
    if (sphere)
    {
        [sphere release];
        sphere = nil;
    }
    
    // free vbo
    if (vbo_element_sphere)
    {
        glDeleteBuffers(1, &vbo_element_sphere);
        vbo_element_sphere = 0;
    }

    if (vbo_normal_sphere)
    {
        glDeleteBuffers(1, &vbo_normal_sphere);
        vbo_normal_sphere = 0;
    }

    if (vbo_position_sphere)
    {
        glDeleteBuffers(1, &vbo_position_sphere);
        vbo_position_sphere = 0;
    }

    // free vao_sphere
    if (vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }


    //detach, delete shader objexts and delete shader prgram objects
    // Clean up per-fragment shader
    if (shaderProgramObjectPerFragment)
    {
        glUseProgram(shaderProgramObjectPerFragment);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObjectPerFragment, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObjectPerFragment, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObjectPerFragment, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObjectPerFragment);
        shaderProgramObjectPerFragment = 0;
    }

    // Clean up per-vertex shader
    if (shaderProgramObjectPerVertex)
    {
        glUseProgram(shaderProgramObjectPerVertex);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObjectPerVertex, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObjectPerVertex, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObjectPerVertex, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObjectPerVertex);
        shaderProgramObjectPerVertex = 0;
    }
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
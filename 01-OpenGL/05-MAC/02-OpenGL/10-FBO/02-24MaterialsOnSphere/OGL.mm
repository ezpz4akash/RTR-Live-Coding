#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import "Sphere.h"
#import "materials.h"

using namespace vmath;
CVReturn myDisplayLinkCallback(CVDisplayLinkRef, 
                               const CVTimeStamp *, 
                               const CVTimeStamp *, 
                               CVOptionFlags, 
                               CVOptionFlags*, 
                               void*);


FILE *gpFile = NULL;

enum {
  AMC_ATTRIBUTE_POSITION = 0,
  AMC_ATTRIBUTE_COLOR,
  AMC_ATTRIBUTE_NORMAL,
  AMC_ATTRIBUTE_TEXCOORD,
};

// lighting type
#define PER_VERTEX   (0)
#define PER_FRAGMENT (1)
#define FBO_WIDTH  (512)
#define FBO_HEIGHT (512)

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define NO_AXIS 3

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView : NSOpenGLView
-(void)uninitialize;
@end

int main(int argc, char** argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    NSApp = [NSApplication sharedApplication];
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

-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirPath = [appBundle bundlePath];
    NSString *parentDirPath = [appDirPath stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt", parentDirPath];
    const char *pszlogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile = fopen(pszlogFileNameWithPath, "w");
    if (gpFile == NULL)
    {
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
    [window makeKeyAndOrderFront:self];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    if (gpFile)
    {
        fprintf(gpFile, "Program terminated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void)windowWillClose:(NSNotification *)notification
{
    [glView uninitialize];
    [NSApp terminate:self];
}

-(BOOL)windowShouldClose:(NSWindow *)aWindow
{
    [glView uninitialize];
    return (YES);
}

-(void)dealloc
{
    [glView release];
    [window release];
    [super dealloc];
}
@end

@implementation GLView
{
    CVDisplayLinkRef displayLink;
    
    // Cube shader program
    GLuint shaderProgramObject;
    GLuint vao_cube;
    GLuint vbo_position_cube;
    GLuint vbo_texcord_cube;
    GLuint mvpMatrixUniform;
    GLuint textureSamplerUniform;
    GLfloat gldAngleCube;
    
    // Sphere shader programs (per-fragment and per-vertex)
    GLuint shaderProgramObjectSphere;
    GLuint gVao_sphere;
    GLuint gVbo_sphere_position;
    GLuint gVbo_sphere_normal;
    GLuint gVbo_sphere_element;
    
    // Light uniforms for sphere
    GLuint laUniformSphere;
    GLuint ldUniformSphere;
    GLuint lsUniformSphere;
    GLuint lightPositionUniformSphere;
    GLuint LKeyPressedUniformSphere;
    
    // Material uniforms for sphere
    GLuint kaUniformSphere;
    GLuint kdUniformSphere;
    GLuint ksUniformSphere;
    GLuint materialShininessUniformSphere;
    
    // MVP matrix for sphere
    GLuint modelMatrixUniformSphere;
    GLuint viewMatrixUniformSphere;
    GLuint projectionMatrixUniformSphere;
    
    // Sphere data
    Sphere *sphere;
    unsigned int gNumVertices;
    unsigned int gNumElements;
    
    // Light animation angles
    GLfloat lightAngleSphere;
    unsigned int gLightRotationAxisSphere;
    unsigned int gMaterialIndex;
    
    // Projection matrices
    mat4 perspectiveProjectionMatrix;
    mat4 perspectiveProjectionMatrixSphere;
    
    // User control
    BOOL bLightSphere;
    
    // FBO related
    GLuint fbo;
    GLuint rbo;
    GLuint textureFbo;
    int winWidth;
    int winHeight;
    int fboResult;
    
    // Light structure
    struct Light {
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;
        vec4 position;
    } lightSphere;

}

-(id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        [[self window]setContentView:self];
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

        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:attributes]autorelease];
        if (pixelFormat == nil)
        {
            fprintf(gpFile, "NSOpenGLPixel format creation failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            return(nil);
        }

        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];
        if (glContext == nil)
        {
            fprintf(gpFile, "NSOpenGLContext creation failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            return(nil);
        }

        [self setPixelFormat:pixelFormat];
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
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    [self drawView];
    [pool release];
    return(kCVReturnSuccess);
}

-(void)prepareOpenGL
{
    [super prepareOpenGL];
    [[self openGLContext] makeCurrentContext];
    GLint swapInterval = 1;
    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    int retVal = [self initialize];
    if (retVal != 0)
    {
        fprintf(gpFile, "Initialization Failed\n");
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
        return;
    }

    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &myDisplayLinkCallback, self);
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(void)reshape
{
    [super reshape];
    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    NSRect viewRect = [self bounds];
    int width = (int)viewRect.size.width;
    int height = (int)viewRect.size.height;
    [self resize:width :height];
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)drawView
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    [self display];
    [self update];
    [self updateSphere];
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)drawRect:(NSRect)dirtyRect
{
    [self drawView];
}

-(BOOL)acceptsFirstResponder
{
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)event
{
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
            bLightSphere = !bLightSphere;
            break;
        case 'X':
        case 'x':
            gLightRotationAxisSphere = X_AXIS;
            break;
        case 'Y':
        case 'y':
            gLightRotationAxisSphere = Y_AXIS;
            break;
        case 'Z':
        case 'z':
            gLightRotationAxisSphere = Z_AXIS;
            break;
        case 'N':
        case 'n':
            gMaterialIndex = (gMaterialIndex + 1) % 24;
            break;
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)event
{
}

//  INITIALIZE -
-(int)initialize
{
    [self printGLInfo];

    // Step 1: Cube vertex shader (for texture display)
    const GLchar *vertexShaderSourceCode = 
    "#version 410 core\n" \
    "in vec4 aPosition;\n" \
    "in vec2 aTexCord;\n" \
    "uniform mat4 uMVPMatrix;\n" \
    "out vec2 out_texcord;\n" \
    "void main(void)" \
    "{\n"\
    "   gl_Position = uMVPMatrix * aPosition;\n" \
    "   out_texcord = aTexCord;\n" \
    "}\n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);

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
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Cube fragment shader
    const GLchar *fragmentShaderSourceCode = 
    "#version 410 core\n" \
    "out vec4 FragColor;\n" \
    "in vec2 out_texcord;\n" \
    "uniform sampler2D uTextureSampler;\n" \
    "void main(void)" \
    "{\n" \
    "   FragColor = texture(uTextureSampler, out_texcord);\n" \
    "}\n";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
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
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
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

    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "aTexCord");
    glLinkProgram(shaderProgramObject);

    status = 0;
    infoLogLength = 0;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    // Cube geometry
    const GLfloat cube_position[] = {
        1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f,
        1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
        1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    };

    const GLfloat cube_texcords[] = {
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    glGenBuffers(1, &vbo_position_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glGenBuffers(1, &vbo_texcord_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcords), cube_texcords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjectionMatrix = mat4::identity();

    // Initialize variables
    gldAngleCube = 0.0f;
    bLightSphere = FALSE;
    lightAngleSphere = 0.0f;
    gLightRotationAxisSphere = NO_AXIS;
    gMaterialIndex = 0;

    NSRect viewRect = [self bounds];
    [self resize:(int)viewRect.size.width :(int)viewRect.size.height];

    // Create FBO
    if ([self createAndPrepareFBOforDrawing:FBO_WIDTH :FBO_HEIGHT] == TRUE)
    {
        fprintf(gpFile, "FBO creation successfull\n");
        fboResult = [self initialize_sphere];
        if (fboResult != 0)
        {
            fprintf(gpFile, "initialize_sphere failed\n");
            return -10;
        }
        else
        {
            fprintf(gpFile, "initialize_sphere successfull\n");
        }
    }
    else
    {
        fprintf(gpFile, "createAndPrepareFBOforDrawing failed\n");
        return -11;
    }

    return 0;
}

//  PRINTGLINFO 
-(void)printGLInfo
{
    fprintf(gpFile, "OPEN GL INFORMATION\n");
    fprintf(gpFile, "********************\n");
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(gpFile, "********************\n");
}

//  CREATE AND PREPARE FBO 
-(BOOL)createAndPrepareFBOforDrawing:(GLint)textureWidth :(GLint)textureHeight
{
    GLint maxRenderedBufferSize;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderedBufferSize);

    if ((maxRenderedBufferSize < textureWidth) || (maxRenderedBufferSize < textureHeight))
    {
        fprintf(gpFile, "fbo width/height exceeding maxRenderedBufferSize\n");
        return FALSE;
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    glGenTextures(1, &textureFbo);
    glBindTexture(GL_TEXTURE_2D, textureFbo);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFbo, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(gpFile, "fbo creation is incomplete\n");
        return FALSE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return TRUE;
}

//  INITIALIZE SPHERE 
-(int)initialize_sphere
{
    const GLchar *vertexShaderSourceCode = 
    "#version 410 core\n" \
    "in vec4 aPosition;\n" \
    "in vec3 aNormal;\n" \
    "out vec3 out_transformedNormals;\n" \
    "out vec3 out_lightDirection[3];\n" \
    "out vec3 out_viewerVector;\n" \
    "uniform mat4 uModelMatrix;\n" \
    "uniform mat4 uViewMatrix;\n" \
    "uniform mat4 uProjectionMatrix;\n" \
    "uniform vec4 uLightPosition[3];\n" \
    "uniform int uLkeyPressed;\n" \
    "void main(void)" \
    "{\n"\
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
    "       mat3 normalMatrix = mat3(uViewMatrix * uModelMatrix);\n" \
    "       out_transformedNormals = (normalMatrix * aNormal);\n" \
    "       out_viewerVector = (-eyeCoordinates.xyz);\n"\
    "       for (int light_no = 0; light_no < 3; light_no++)\n" \
    "       {\n" \
    "           out_lightDirection[light_no] = (vec3(uLightPosition[light_no]) - eyeCoordinates.xyz);\n" \
    "       }\n"\
    "   }\n"\
    "}\n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);

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
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    const GLchar *fragmentShaderSourceCode = 
    "#version 410 core\n" \
    "out vec4 FragColor;\n" \
    "in vec3 out_transformedNormals;\n" \
    "in vec3 out_lightDirection[3];\n" \
    "in vec3 out_viewerVector;\n" \
    "uniform vec3 uLa[3];\n" \
    "uniform vec3 uLd[3];\n" \
    "uniform vec3 uLs[3];\n" \
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
    "       vec3 normalized_transformedNormals = normalize(out_transformedNormals);\n" \
    "       vec3 normalized_viewerVector = normalize(out_viewerVector);\n"
    "       fong_ads_light = vec3(0.0f, 0.0f, 0.0f);\n" \
    "       for (int light_no = 0; light_no < 3; light_no++)\n" \
    "       {\n" \
    "           vec3 normalized_lightDirection = normalize(out_lightDirection[light_no]);\n"
    "           vec3 reflectionVector = reflect(-normalized_lightDirection, normalized_transformedNormals);\n"
    "           vec3 diffusedLight = uLd[light_no] * uKd * max(dot(normalized_lightDirection, normalized_transformedNormals), 0.0);\n"
    "           vec3 ambientLight = uLa[light_no] * uKa;\n"
    "           vec3 specularLight = uLs[light_no] * uKs * pow(max(dot(reflectionVector, normalized_viewerVector),0.0), uMaterialShininess);\n"
    "           fong_ads_light += ambientLight + diffusedLight + specularLight;\n"
    "       }\n" \
    "   }\n" \
    "   else \n"\
    "   {\n"\
    "       fong_ads_light = vec3(1.0f, 1.0f, 1.0f);\n"
    "   }\n" \
    "   FragColor = vec4(fong_ads_light, 1.0f);\n" \
    "}\n";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
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
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
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

    shaderProgramObjectSphere = glCreateProgram();
    glAttachShader(shaderProgramObjectSphere, vertexShaderObject);
    glAttachShader(shaderProgramObjectSphere, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObjectSphere, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObjectSphere, AMC_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObjectSphere);

    status = 0;
    infoLogLength = 0;
    glGetProgramiv(shaderProgramObjectSphere, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObjectSphere, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObjectSphere, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader program link Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -9;
    }

    modelMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uModelMatrix");
    viewMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uViewMatrix");
    projectionMatrixUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uProjectionMatrix");
    laUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLa");
    ldUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLd");
    lsUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLs");
    lightPositionUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLightPosition");

    kaUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKa");
    kdUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKd");
    ksUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uKs");
    materialShininessUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uMaterialShininess");
    LKeyPressedUniformSphere = glGetUniformLocation(shaderProgramObjectSphere, "uLkeyPressed");

    sphere = [[Sphere alloc]init];

    float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_textures[764];
    unsigned short sphere_elements[2280];

    [sphere getSphereVertexData:sphere_vertices :sphere_normals :sphere_textures :sphere_elements];
    gNumVertices = [sphere getNumberOfSphereVertices];
    gNumElements = [sphere getNumberOfSphereElements];

    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);

    glGenBuffers(1, &gVbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &gVbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    perspectiveProjectionMatrixSphere = mat4::identity();

    lightSphere.ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lightSphere.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightSphere.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightSphere.position = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    [self resize_sphere:FBO_WIDTH :FBO_HEIGHT];

    return 0;
}

//  RESIZE 
-(void)resize:(int)width :(int)height
{
    if (height <= 0)
        height = 1;

    winWidth = width;
    winHeight = height;

    glViewport(0, 0, (GLsizei)width, height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);
}

//  RESIZE_SPHERE 
-(void)resize_sphere:(int)width :(int)height
{
    if (height <= 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, height);
    perspectiveProjectionMatrixSphere = vmath::perspective(45.0f, (GLdouble)width/(GLdouble)height, 0.1f, 100.0f);
}

//  DISPLAY 
-(void)display
{
    [self display_sphere];
    [self resize:winWidth :winHeight];
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

    mat4 modelViewMatrix = mat4::identity();
    mat4 translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
    mat4 scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
    mat4 rotationMatrixX = vmath::rotate(gldAngleCube, 1.0f, 0.0f, 0.0f);
    mat4 rotationMatrixY = vmath::rotate(gldAngleCube, 0.0f, 1.0f, 0.0f);
    mat4 rotationMatrixZ = vmath::rotate(gldAngleCube, 0.0f, 0.0f, 1.0f);
    
    modelViewMatrix = translationMatrix * rotationMatrixX * rotationMatrixY * rotationMatrixZ * scaleMatrix;
    mat4 modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureFbo);
    glUniform1i(textureSamplerUniform, 0);
    
    glBindVertexArray(vao_cube);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

//  DISPLAY_SPHERE 
-(void)display_sphere
{
    if (fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        [self resize_sphere:FBO_WIDTH :FBO_HEIGHT];
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    else
    {
        fprintf(gpFile, "fbo invalid\n");
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    mat4 lightTranslationMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();

    viewMatrix = vmath::translate(0.0f, 0.0f, -1.5f);
    modelMatrix = viewMatrix;
    switch (gLightRotationAxisSphere)
    {
        case X_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 1.0f, 0.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 20.0f, 20.0f);
            break;
        case Y_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 0.0f, 1.0f, 0.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 0.0f, 20.0f);
            break;
        case Z_AXIS:
            lightTranslationMatrix = vmath::rotate(lightAngleSphere, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(20.0f, 20.0f, 0.0f);
            break;
        default:
            lightTranslationMatrix = vmath::rotate(0.0f, 0.0f, 0.0f, 1.0f);
            lightTranslationMatrix = lightTranslationMatrix * vmath::translate(0.0f, 0.0f, 20.0f);
            break;
    }

    glUseProgram(shaderProgramObjectSphere);

    glUniformMatrix4fv(modelMatrixUniformSphere, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniformSphere, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniformSphere, 1, GL_FALSE, perspectiveProjectionMatrixSphere);

    glUniform1i(LKeyPressedUniformSphere, bLightSphere);

    glUniform3fv(laUniformSphere, 1, lightSphere.ambient);
    glUniform3fv(ldUniformSphere, 1, lightSphere.diffuse);
    glUniform3fv(lsUniformSphere, 1, lightSphere.specular);
    glUniform4fv(lightPositionUniformSphere, 1, lightSphere.position * lightTranslationMatrix.transpose());

    translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
    modelMatrix = translationMatrix;
    glUniformMatrix4fv(modelMatrixUniformSphere, 1, GL_FALSE, modelMatrix);
    glUniform3fv(kaUniformSphere, 1, sphereMaterials[gMaterialIndex].ambient);
    glUniform3fv(kdUniformSphere, 1, sphereMaterials[gMaterialIndex].diffuse);
    glUniform3fv(ksUniformSphere, 1, sphereMaterials[gMaterialIndex].specular);
    glUniform1f(materialShininessUniformSphere, sphereMaterials[gMaterialIndex].shininess[0]);

    glBindVertexArray(gVao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    if (fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

//  UPDATE 
-(void)update
{
    gldAngleCube = gldAngleCube - 1.0f;
    if (gldAngleCube <= 0)
    {
        gldAngleCube = 360.0f;
    }
}

//  UPDATE_SPHERE 
-(void)updateSphere
{
    lightAngleSphere = lightAngleSphere + 2.0f;
}

//  UNINITIALIZE 
-(void)uninitialize
{
    [self uninitialize_sphere];

    if (textureFbo)
    {
        glDeleteTextures(1, &textureFbo);
        textureFbo = 0;
    }

    if (vbo_texcord_cube)
    {
        glDeleteBuffers(1, &vbo_texcord_cube);
        vbo_texcord_cube = 0;
    }

    if (vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    if (vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
    }

    if (shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
    }
}

//  UNINITIALIZE_SPHERE 
-(void)uninitialize_sphere
{
    if(rbo)
    {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }

    if(fbo)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    if (gVbo_sphere_position)
    {
        glDeleteBuffers(1, &gVbo_sphere_position);
        gVbo_sphere_position = 0;
    }
    
    if (gVbo_sphere_normal)
    {
        glDeleteBuffers(1, &gVbo_sphere_normal);
        gVbo_sphere_normal = 0;
    }

    if (gVbo_sphere_element)
    {
        glDeleteBuffers(1, &gVbo_sphere_element);
        gVbo_sphere_element = 0;
    }

    if (gVao_sphere)
    {
        glDeleteVertexArrays(1, &gVao_sphere);
    }

    if (shaderProgramObjectSphere)
    {
        glUseProgram(shaderProgramObjectSphere);
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObjectSphere, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObjectSphere, numShaders, NULL, pShaders);
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObjectSphere, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObjectSphere);
    }

    if (sphere)
    {
        [sphere release];
        sphere = nil;
    }
}

-(void)dealloc
{
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}

@end

CVReturn myDisplayLinkCallback(CVDisplayLinkRef displayLinkRef, const CVTimeStamp *current, const CVTimeStamp *outputTime, CVOptionFlags inputflags, CVOptionFlags *outputFlags, void *view)
{
    CVReturn result = [(GLView *)view getFrameForTime:outputTime];
    return(result);
}

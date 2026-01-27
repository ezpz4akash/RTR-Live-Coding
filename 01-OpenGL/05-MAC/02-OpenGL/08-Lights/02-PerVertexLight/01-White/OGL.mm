#import <Foundation/Foundation.h> //like stdio.h
#import <Cocoa/Cocoa.h> // like Windows.h

#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import "Sphere.h"

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
    GLuint shaderProgramObject;
    // vao, vbo
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;

    // uniform locations
    GLuint modelMatrixUniform;
    GLuint viewMatrixUniform;
    GLuint projectionMatrixUniform;

    // light related uniforms
    GLuint laUniform;
    GLuint ldUniform;
    GLuint lsUniform;
    GLuint lightPositionUniform;

    //material uniforms
    GLuint kaUniform;
    GLuint kdUniform;
    GLuint ksUniform;
    GLuint materialShininessUniform;

    GLuint LKeyPressedUniform;

    
    Sphere *sphere;

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
    "{\n"\
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
    "   if (uLkeyPressed == 1) \n"\
    "   {\n"\
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
    "in vec3 out_fong_ads_light;\n" \
    "void main(void)" \
    "{\n" \
    "   FragColor = vec4(out_fong_ads_light, 1.0f);\n" \
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

    // creade shader program objects
    shaderProgramObject = glCreateProgram();
    
    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObject, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // bind shader attribute at certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");

    // link shader objects to shader program object
    glLinkProgram(shaderProgramObject);


    // check link error
    status = 0;
    infoLogLength = 0;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength *sizeof(GLchar));
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

    //get the required uniform location from the shader
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    laUniform = glGetUniformLocation(shaderProgramObject, "uLa");
    ldUniform = glGetUniformLocation(shaderProgramObject, "uLd");
    lsUniform = glGetUniformLocation(shaderProgramObject, "uLs");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "uLightPosition");

    kaUniform = glGetUniformLocation(shaderProgramObject, "uKa");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");

    LKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLkeyPressed");

    // Initialize sphere
    sphere = [[Sphere alloc]init];

    float spherePositionCoords[1146];
    float sphereNormalCoords[1146];
    float sphereTexCoords[764];
    unsigned short sphereElements[2280];
    
    [sphere getSphereVertexData:spherePositionCoords :sphereNormalCoords :sphereTexCoords :sphereElements];

    int numSphereVertices = [sphere getNumberOfSphereVertices];
    int numSphereElements = [sphere getNumberOfSphereElements];

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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numSphereElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);

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
    //variable declaration 
    static GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    static GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat lightPosition[] = {100.0f, 100.0f, 100.0f, 1.0f};

    // material related variables
    static GLfloat materialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat materiaShininess= 50.0f;
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //use shader program object
    glUseProgram(shaderProgramObject);

       // transformations
    mat4 modelMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 viewMatrix = mat4::identity(); //analogous glLoadIdentity in Display for model view matrix
    mat4 translationMatrix = mat4::identity();



    //translate
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;


    // send this matrix to shader in uniform

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniform1i(LKeyPressedUniform, bLight);

    glUniform3fv(laUniform, 1, lightAmbient);
    glUniform3fv(ldUniform, 1, lightDiffuse);
    glUniform3fv(lsUniform, 1, lightSpecular);
    glUniform4fv(lightPositionUniform, 1, lightPosition);

    glUniform3fv(kaUniform, 1, materialAmbient);
    glUniform3fv(kdUniform, 1, materialDiffuse);
    glUniform3fv(ksUniform, 1, materialSpecular);
    glUniform1f(materialShininessUniform, materiaShininess);



    // bind with vao_sphere
    glBindVertexArray(vao_sphere);

    // draw the vertex arrays 
    glDrawElements(GL_TRIANGLES, [sphere getNumberOfSphereElements], GL_UNSIGNED_SHORT, 0);

    // unbind with vao_triangle
    glBindVertexArray(0);

 
    //unuse shader peogram object
    glUseProgram(0);

}

-(void)myUpdate
{
    //code

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
    //steps
    // 1. if shader program is still there
    // 2. get number of shaders and continue only if num of shaders is greater than zero
    // 3. create an array to hold shader objects of obtained numbers of shaders
    // 4. get shader objects into this array, continue only if malloc us succeded
    // 5. start a loop for obtained num of shaders, detach and delete every shader
    //6. free the buffer
    // 7. delete the shader program objects
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
        shaderProgramObject = 0;
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
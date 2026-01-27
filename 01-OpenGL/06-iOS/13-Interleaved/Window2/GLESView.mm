//
//  MyView.m
//  Window2
//
//  Created by user947254 on 1/18/26.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/CADisplayLink.h>

#import "GLESView.h"
#include "vmath.h"

@implementation GLESView
{
    EAGLContext *eaglContext;

    GLuint framebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;

    CADisplayLink *displayLink;
    CAFrameRateRange frameRateRange;
    BOOL isDisplayLink;
    
    // Shader related variables
    GLuint shaderProgramObject;
    GLuint vao_cube;
    GLuint vbo;  // Single interleaved VBO
    GLuint texture_marble;
    
    // Uniform locations
    GLint modelMatrixUniform;
    GLint viewMatrixUniform;
    GLint projectionMatrixUniform;
    GLint textureSamplerUniform;
    
    // Light uniforms
    GLint laUniform;
    GLint ldUniform;
    GLint lsUniform;
    GLint lightPositionUniform;
    
    // Material uniforms
    GLint kaUniform;
    GLint kdUniform;
    GLint ksUniform;
    GLint materialShininessUniform;
    
    GLint lKeyPressedUniform;
    
    vmath::mat4 perspectiveProjectionMatrix;
    float angle_cube;
    
    // Light properties
    float lightAmbient[4];
    float lightDiffuse[4];
    float lightSpecular[4];
    float lightPosition[4];
    
    // Material properties
    float materialAmbient[4];
    float materialDiffuse[4];
    float materialSpecular[4];
    float materialShininess;
    
    BOOL bLight;
}

-(void)awakeFromNib
{
    [super awakeFromNib];
    {
        //black color
        [self setBackgroundColor:[UIColor blackColor]];
        
        //
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)[super layer];
        [eaglLayer setOpaque:YES];
        [eaglLayer
        setDrawableProperties:[NSDictionary
                                          dictionaryWithObjectsAndKeys:
                                              [NSNumber numberWithBool:NO],
                                              kEAGLDrawablePropertyRetainedBacking,  // do not retain frame
                                              kEAGLColorFormatRGBA8,  // eagl color format 32 bit
                                              kEAGLDrawablePropertyColorFormat, nil]];

        // create context
        eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if (eaglContext == nil) {
            printf("OpenGLES Context creation failed..\n");
            [self uninitialize];
            return;
        }

        // set context
        [EAGLContext setCurrentContext:eaglContext];

        // create framebuffer with color and depth attachments
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      colorRenderbuffer);
        
        GLint width;
        GLint height;

        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      depthRenderbuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("initWithFrame : Framebuffer is not complete..\n");
            
            return;
        }

        // initialize other class variables
        displayLink = nil;
        frameRateRange.minimum = 30.0f;
        frameRateRange.maximum = 60.0f;
        frameRateRange.preferred = 60.0f;
        isDisplayLink = NO;
        
        
        // Call our initialize	
        int result = [self initialize];	
        
        [self startDisplayLink];
        

        // fetch OpenGL related details
        printf("OpenGLES Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("OpenGLES Renderer: %s\n", glGetString(GL_RENDERER));
        printf("OpenGLES Version:  %s\n", glGetString(GL_VERSION));
        printf("GLSL ES  Version:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        //event handling
        //single tap
        UITapGestureRecognizer *singleTapGestureRecognizer =
               [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];

        //double tap
        UITapGestureRecognizer *doubleTapGestureRecongnizer =
               [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecongnizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecongnizer];
        
        //tell single tap recognizer to fail where there is double tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecongnizer];
        
        //swipe
        UISwipeGestureRecognizer *swipeGestureReconginzer =
               [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        [swipeGestureReconginzer setDelegate:self];
        [self addGestureRecognizer:swipeGestureReconginzer];

        //longpress
        UILongPressGestureRecognizer *longPressGestureRecognizer =
               [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        [longPressGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:longPressGestureRecognizer];
        
        
    }
    
    return;
}

+(Class)layerClass
{
    //code
    return ([CAEAGLLayer class]);
}

/*
-(void)drawRect:(CGRect)rect
{
    
}
 */

-(void)layoutSubviews
{
    //code
    [super awakeFromNib];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)[self layer]];

    GLint width;
    GLint height;

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depthRenderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("layoutSubviews : Framebuffer is not complete..\n");
        return;
    }
    
    //call our stub resize
    [self resize:width :height];
    
    //call drawing function here
    [self drawView:self];
}

-(void)drawView:(id)sender
{
    [EAGLContext setCurrentContext:eaglContext];
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    [self myupdate];
    [self display];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:colorRenderbuffer];
}

-(void)startDisplayLink
{
    if(isDisplayLink == NO)
    {
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector((drawView:))];
        [displayLink setPreferredFrameRateRange:frameRateRange];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        isDisplayLink = YES;
    }
}

-(void)stopDisplayLink
{
    if(isDisplayLink == YES)
    {
        [displayLink invalidate];
        isDisplayLink = NO;
    }
}

-(BOOL)becomeFirstResponder
{
    //code
    return (YES);
}

-(void)touchesBegan:(UITouch *)touches withEvent:(UIEvent *)event
{
    //code
}

- (void)onSingleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Toggle light
    bLight = !bLight;
    if(bLight)
        printf("Light ON\n");
    else
        printf("Light OFF\n");
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // code
}

- (void)onSwipe:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    [self uninitialize];
    [self release];
    exit(0);
}

- (void)onLongPress:(UITapGestureRecognizer *)gestureRecognizer {
    // code
}

-(int)initialize
{
    //code
    [self printGLESInfo];
    
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Initialize light properties
    lightAmbient[0] = 0.0f; lightAmbient[1] = 0.0f; lightAmbient[2] = 0.0f; lightAmbient[3] = 1.0f;
    lightDiffuse[0] = 1.0f; lightDiffuse[1] = 1.0f; lightDiffuse[2] = 1.0f; lightDiffuse[3] = 1.0f;
    lightSpecular[0] = 1.0f; lightSpecular[1] = 1.0f; lightSpecular[2] = 1.0f; lightSpecular[3] = 1.0f;
    lightPosition[0] = 100.0f; lightPosition[1] = 100.0f; lightPosition[2] = 100.0f; lightPosition[3] = 1.0f;
    
    // Initialize material properties
    materialAmbient[0] = 0.25f; materialAmbient[1] = 0.25f; materialAmbient[2] = 0.25f; materialAmbient[3] = 1.0f;
    materialDiffuse[0] = 1.0f; materialDiffuse[1] = 1.0f; materialDiffuse[2] = 1.0f; materialDiffuse[3] = 1.0f;
    materialSpecular[0] = 1.0f; materialSpecular[1] = 1.0f; materialSpecular[2] = 1.0f; materialSpecular[3] = 1.0f;
    materialShininess = 128.0f;
    
    bLight = NO;
    
    // Vertex Shader
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec4 aColor;\n"
        "in vec3 aNormal;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 out_texCoord;\n"
        "out vec4 eyeCoordinates;\n"
        "out vec3 transformedNormal;\n"
        "out vec3 lightSource;\n"
        "out vec4 out_color;\n"
        "uniform mat4 uModelMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec4 uLightPosition;\n"
        "void main(void)\n"
        "{\n"
        "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n"
        "   transformedNormal = normalMatrix * aNormal;\n"
        "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n"
        "   lightSource = vec3(uLightPosition) - eyeCoordinates.xyz;\n"
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"
        "   out_color = aColor;\n"
        "   out_texCoord = aTexCoord;\n"
        "}\n";
    glShaderSource(vertexShaderObject, 1, &vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Fragment Shader - Per-Fragment Lighting
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec3 transformedNormal;\n"
        "in vec4 eyeCoordinates;\n"
        "in vec3 lightSource;\n"
        "in vec4 out_color;\n"
        "in vec2 out_texCoord;\n"
        "uniform vec3 uLa;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uLs;\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform int uLKeyPressed;\n"
        "uniform sampler2D uTextureSampler;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n"
        "   vec3 normalizedLightSource = normalize(lightSource);\n"
        "   vec3 normalizedViewerVector = normalize(-eyeCoordinates.xyz);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "       vec3 ambient = uLa * uKa;\n"
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, normalizedViewerVector), 0.0), uMaterialShininess);\n"
        "       FragColor = vec4(ambient + diffuse + specular, 1.0) * out_color * texture(uTextureSampler, out_texCoord);\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       FragColor = out_color * texture(uTextureSampler, out_texCoord);\n"
        "   }\n"
        "}\n";
    glShaderSource(fragmentShaderObject, 1, &fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Shader Program Object
    shaderProgramObject = glCreateProgram();
    if(shaderProgramObject == 0){
        printf("glCreateProgram failed\n");
        return -9;
    }

    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // Bind attribute locations
    glBindAttribLocation(shaderProgramObject, 0, "aPosition");
    glBindAttribLocation(shaderProgramObject, 1, "aColor");
    glBindAttribLocation(shaderProgramObject, 2, "aNormal");
    glBindAttribLocation(shaderProgramObject, 3, "aTexCoord");

    glLinkProgram(shaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(shaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                printf("Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }

    // Get uniform locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    laUniform = glGetUniformLocation(shaderProgramObject, "uLa");
    ldUniform = glGetUniformLocation(shaderProgramObject, "uLd");
    lsUniform = glGetUniformLocation(shaderProgramObject, "uLs");
    kaUniform = glGetUniformLocation(shaderProgramObject, "uKa");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "uLightPosition");
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLKeyPressed");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    // Cube - Interleaved array: Position(3) + Color(3) + Normal(3) + TexCoord(2) = 11 floats per vertex
    {
        const GLfloat cube_PCNT[] = {
            // front face
            // position              // color             // normals           // texcoords
             1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
            // right face
             1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
             1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
            // back face
             1.0f,  1.0f, -1.0f,    1.0f, 1.0f, 0.0f,    0.0f, 0.0f,-1.0f,    1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,    1.0f, 1.0f, 0.0f,    0.0f, 0.0f,-1.0f,    0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,    1.0f, 1.0f, 0.0f,    0.0f, 0.0f,-1.0f,    0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,    1.0f, 1.0f, 0.0f,    0.0f, 0.0f,-1.0f,    1.0f, 0.0f,
            // left face
            -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
            // top face
             1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
            // bottom face
             1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f,    0.0f,-1.0f, 0.0f,    1.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f,    0.0f,-1.0f, 0.0f,    0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f,    0.0f,-1.0f, 0.0f,    0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f,    0.0f,-1.0f, 0.0f,    1.0f, 0.0f,
        };

        glGenVertexArrays(1, &vao_cube);
        glBindVertexArray(vao_cube);
        {
            // Single interleaved VBO for Position, Color, Normal, TexCoord
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            {
                glBufferData(GL_ARRAY_BUFFER, sizeof(cube_PCNT), cube_PCNT, GL_STATIC_DRAW);
                
                // Position - attribute 0
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(0 * sizeof(float)));
                glEnableVertexAttribArray(0);
                
                // Color - attribute 1
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                
                // Normal - attribute 2
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);
                
                // TexCoord - attribute 3
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
                glEnableVertexAttribArray(3);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }

    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    angle_cube = 0.0f;

    // Load marble texture
    texture_marble = [self loadGLTexture:@"marble" :@"bmp"];
    
    return (0);
}

- (GLuint) loadGLTexture:(NSString *)textureFileName :(NSString *)extension
{
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *textureFileNameWithPath = [appBundle pathForResource:textureFileName ofType:extension];

    UIImage *uiImage = [[UIImage alloc] initWithContentsOfFile:textureFileNameWithPath];
    
    CGImageRef cgImage = [uiImage CGImage];

    // get width of image
    int imageWidth = (int)CGImageGetWidth(cgImage);

    // get height of image
    int imageHeight = (int)CGImageGetHeight(cgImage);

    // for image data 
    // step 1:  get CGDataProvider
    CGDataProviderRef cgDataProvider = CGImageGetDataProvider(cgImage);

    // For image data step 2: get CFData representation of CGData
    CFDataRef cfData = CGDataProviderCopyData(cgDataProvider);

    // For image data step 3: get CFData in the form of bytes
    void* imageData = (void*)CFDataGetBytePtr(cfData);

    // usual texture code
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,  
                        GL_RGBA,
                        imageWidth,
                        imageHeight,
                        0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        imageData
                    );

    glGenerateMipmap(GL_TEXTURE_2D);

    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // release our core foundation data
    CFRelease(cfData);

    return (texture);

}

-(void)printGLESInfo
{
    // code
    GLint numExtensions;

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    // print openGL information
    printf("OPENGL INFORMATION\n");
    printf("******************\n");
    printf("OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version : %s\n", glGetString(GL_VERSION));
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

-(void)resize:(int)width :(int)height
{
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, width/height, 0.1f, 100.0f);
}

-(void)display
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);
    {
        // Transformations
        vmath::mat4 modelMatrix = vmath::mat4::identity();
        vmath::mat4 viewMatrix = vmath::mat4::identity();
        vmath::mat4 translationMatrix = vmath::mat4::identity();
        vmath::mat4 rotationMatrix = vmath::mat4::identity();
        
        translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
        
        rotationMatrix = vmath::rotate(angle_cube, 1.0f, 0.0f, 0.0f);
        rotationMatrix = rotationMatrix * vmath::rotate(angle_cube, 0.0f, 1.0f, 0.0f);
        rotationMatrix = rotationMatrix * vmath::rotate(angle_cube, 0.0f, 0.0f, 1.0f);
        
        modelMatrix = translationMatrix * rotationMatrix;
        
        // Pass matrices to shader
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, (const float*)modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, (const float*)viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
        
        // Pass light uniforms
        glUniform3fv(laUniform, 1, lightAmbient);
        glUniform3fv(ldUniform, 1, lightDiffuse);
        glUniform3fv(lsUniform, 1, lightSpecular);
        glUniform4fv(lightPositionUniform, 1, lightPosition);
        
        // Pass material uniforms
        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(materialShininessUniform, materialShininess);
        
        // Pass light toggle
        glUniform1i(lKeyPressedUniform, bLight ? 1 : 0);
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_marble);
        glUniform1i(textureSamplerUniform, 0);
        
        // Draw cube
        glBindVertexArray(vao_cube);
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);  // front
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);  // right
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);  // back
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4); // left
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4); // top
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4); // bottom
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

-(void)myupdate
{
    angle_cube += 1.0f;
}

-(void)uninitialize
{
    if(texture_marble){
        glDeleteTextures(1, &texture_marble);
        texture_marble = 0;
    }

    if(vbo){
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if(vao_cube){
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }
    
    if(shaderProgramObject){
        glUseProgram(shaderProgramObject);
        GLint numShaders;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
    }
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    if(framebuffer)
    {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
    
    if([EAGLContext currentContext] == eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
        eaglContext = nil;
    }
    
}

-(void)dealloc
{
    [self uninitialize];
    [self release];
    [super dealloc];
}

@end

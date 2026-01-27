//
//  MyView.m
//  Window2
//
//  Created by Akash Musale on 1/18/26.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/CADisplayLink.h>

#import "GLESView.h"
#include "vmath.h"
#import "Sphere.h"

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
    GLuint pvShaderProgramObject;
    GLuint pfShaderProgramObject;
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;
    
    // Per-Vertex Lighting Uniforms
    GLint pvModelViewMatrixUniform;
    GLint pvProjectionMatrixUniform;
    GLint pvLaUniform;
    GLint pvLdUniform;
    GLint pvLsUniform;
    GLint pvKaUniform;
    GLint pvKdUniform;
    GLint pvKsUniform;
    GLint pvMaterialShininessUniform;
    GLint pvLightPositionUniform;
    GLint pvLKeyPressedUniform;
    
    // Per-Fragment Lighting Uniforms
    GLint pfModelViewMatrixUniform;
    GLint pfProjectionMatrixUniform;
    GLint pfLaUniform;
    GLint pfLdUniform;
    GLint pfLsUniform;
    GLint pfKaUniform;
    GLint pfKdUniform;
    GLint pfKsUniform;
    GLint pfMaterialShininessUniform;
    GLint pfLightPositionUniform;
    GLint pfLKeyPressedUniform;
    
    Sphere *sphere;
    vmath::mat4 perspectiveProjectionMatrix;
    float angle_sphere;
    
    // Light properties
    float lightAmbient[3];
    float lightDiffuse[3];
    float lightSpecular[3];
    float materialAmbient[3];
    float materialDiffuse[3];
    float materialSpecular[3];
    float materialShininess;
    float lightPosition[3];
    BOOL bLight;
    BOOL bAnimate;
    BOOL bPerVertexPerFragmentToggle;
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
        
        // Initialize light properties
        lightAmbient[0] = 0.1f; lightAmbient[1] = 0.1f; lightAmbient[2] = 0.1f;
        lightDiffuse[0] = 1.0f; lightDiffuse[1] = 1.0f; lightDiffuse[2] = 1.0f;
        lightSpecular[0] = 1.0f; lightSpecular[1] = 1.0f; lightSpecular[2] = 1.0f;
        materialAmbient[0] = 0.0f; materialAmbient[1] = 0.0f; materialAmbient[2] = 0.0f;
        materialDiffuse[0] = 0.5f; materialDiffuse[1] = 0.2f; materialDiffuse[2] = 0.7f;
        materialSpecular[0] = 0.7f; materialSpecular[1] = 0.7f; materialSpecular[2] = 0.7f;
        materialShininess = 128.0f;
        lightPosition[0] = 100.0f; lightPosition[1] = 100.0f; lightPosition[2] = 100.0f;
        bLight = YES;
        bAnimate = YES;
        bPerVertexPerFragmentToggle = NO;
        
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
    bLight = !bLight;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    bPerVertexPerFragmentToggle = !bPerVertexPerFragmentToggle;
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
    
    // ============= PER-VERTEX LIGHTING SHADER SETUP =============
    // Per-Vertex Vertex Shader
    GLuint pvVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pvVertexShaderSourceCode =
        "#version 300 es\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "uniform mat4 uModelViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec3 uLa;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uLs;\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform vec4 uLightPosition;\n"
        "uniform int uLkeyPressed;\n"
        "out vec3 out_phong_ads_Light;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = uProjectionMatrix * uModelViewMatrix * aPosition;\n"
        "    if(uLkeyPressed == 1)\n"
        "    {\n"
        "        vec4 eyeCoordinates = uModelViewMatrix * aPosition;\n"
        "        mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));\n"
        "        vec3 transformedNormal = normalize(normalMatrix * aNormal);\n"
        "        vec3 lightSource = normalize(vec3(uLightPosition - eyeCoordinates));\n"
        "        float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n"
        "        vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n"
        "        vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n"
        "        vec3 ambient = uLa * uKa;\n"
        "        vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "        vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n"
        "        out_phong_ads_Light = ambient + diffuse + specular;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        out_phong_ads_Light = vec3(1.0, 1.0, 1.0);\n"
        "    }\n"
        "}\n";
    glShaderSource(pvVertexShaderObject, 1, &pvVertexShaderSourceCode, NULL);
    glCompileShader(pvVertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(pvVertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pvVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pvVertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PV Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -7;
    }

    // Per-Vertex Fragment Shader
    GLuint pvFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pvFragmentShaderSourceCode =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec3 out_phong_ads_Light;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "    FragColor = vec4(out_phong_ads_Light, 1.0);\n"
        "}\n";
    glShaderSource(pvFragmentShaderObject, 1, &pvFragmentShaderSourceCode, NULL);
    glCompileShader(pvFragmentShaderObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pvFragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pvFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pvFragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PV Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -8;
    }

    // Per-Vertex Shader Program
    pvShaderProgramObject = glCreateProgram();
    if(pvShaderProgramObject == 0){
        printf("PV glCreateProgram failed\n");
        return -9;
    }
    glAttachShader(pvShaderProgramObject, pvVertexShaderObject);
    glAttachShader(pvShaderProgramObject, pvFragmentShaderObject);
    glBindAttribLocation(pvShaderProgramObject, 0, "aPosition");
    glBindAttribLocation(pvShaderProgramObject, 1, "aNormal");
    glLinkProgram(pvShaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(pvShaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pvShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pvShaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                printf("PV Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -10;
    }

    // Per-Vertex Uniform Locations
    pvModelViewMatrixUniform = glGetUniformLocation(pvShaderProgramObject, "uModelViewMatrix");
    pvProjectionMatrixUniform = glGetUniformLocation(pvShaderProgramObject, "uProjectionMatrix");
    pvLaUniform = glGetUniformLocation(pvShaderProgramObject, "uLa");
    pvLdUniform = glGetUniformLocation(pvShaderProgramObject, "uLd");
    pvLsUniform = glGetUniformLocation(pvShaderProgramObject, "uLs");
    pvKaUniform = glGetUniformLocation(pvShaderProgramObject, "uKa");
    pvKdUniform = glGetUniformLocation(pvShaderProgramObject, "uKd");
    pvKsUniform = glGetUniformLocation(pvShaderProgramObject, "uKs");
    pvMaterialShininessUniform = glGetUniformLocation(pvShaderProgramObject, "uMaterialShininess");
    pvLightPositionUniform = glGetUniformLocation(pvShaderProgramObject, "uLightPosition");
    pvLKeyPressedUniform = glGetUniformLocation(pvShaderProgramObject, "uLkeyPressed");

    // ============= PER-FRAGMENT LIGHTING SHADER SETUP =============
    // Per-Fragment Vertex Shader
    GLuint pfVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pfVertexShaderSourceCode =
        "#version 300 es\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "uniform mat4 uModelViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec4 uLightPosition;\n"
        "out vec4 eyeCoordinates;\n"
        "out vec3 transformedNormal;\n"
        "out vec3 lightSource;\n"
        "void main(void)\n"
        "{\n"
        "    eyeCoordinates = uModelViewMatrix * aPosition;\n"
        "    mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));\n"
        "    transformedNormal = normalize(normalMatrix * aNormal);\n"
        "    lightSource = vec3(uLightPosition - eyeCoordinates);\n"
        "    gl_Position = uProjectionMatrix * uModelViewMatrix * aPosition;\n"
        "}\n";
    glShaderSource(pfVertexShaderObject, 1, &pfVertexShaderSourceCode, NULL);
    glCompileShader(pfVertexShaderObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pfVertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pfVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pfVertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PF Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -11;
    }

    // Per-Fragment Fragment Shader
    GLuint pfFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pfFragmentShaderSourceCode =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 eyeCoordinates;\n"
        "in vec3 transformedNormal;\n"
        "in vec3 lightSource;\n"
        "uniform vec3 uLa;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uLs;\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform int uLkeyPressed;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 normalizedTransformNormal = normalize(transformedNormal);\n"
        "    vec3 normalizedLightSource = normalize(lightSource);\n"
        "    vec3 normalizedViewerVector = normalize(-eyeCoordinates.xyz);\n"
        "    if(uLkeyPressed == 1)\n"
        "    {\n"
        "        float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "        vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "        vec3 viewerVector = normalizedViewerVector;\n"
        "        vec3 ambient = uLa * uKa;\n"
        "        vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "        vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n"
        "        FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "    }\n"
        "}\n";
    glShaderSource(pfFragmentShaderObject, 1, &pfFragmentShaderSourceCode, NULL);
    glCompileShader(pfFragmentShaderObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetShaderiv(pfFragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pfFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pfFragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PF Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -12;
    }

    // Per-Fragment Shader Program
    pfShaderProgramObject = glCreateProgram();
    if(pfShaderProgramObject == 0){
        printf("PF glCreateProgram failed\n");
        return -13;
    }
    glAttachShader(pfShaderProgramObject, pfVertexShaderObject);
    glAttachShader(pfShaderProgramObject, pfFragmentShaderObject);
    glBindAttribLocation(pfShaderProgramObject, 0, "aPosition");
    glBindAttribLocation(pfShaderProgramObject, 1, "aNormal");
    glLinkProgram(pfShaderProgramObject);
    iInfoLogLength = 0;
    iStatus = 0;
    szInfoLog = NULL;
    glGetProgramiv(pfShaderProgramObject, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pfShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pfShaderProgramObject, iInfoLogLength, NULL, szInfoLog);
                printf("PF Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        return -14;
    }

    // Per-Fragment Uniform Locations
    pfModelViewMatrixUniform = glGetUniformLocation(pfShaderProgramObject, "uModelViewMatrix");
    pfProjectionMatrixUniform = glGetUniformLocation(pfShaderProgramObject, "uProjectionMatrix");
    pfLaUniform = glGetUniformLocation(pfShaderProgramObject, "uLa");
    pfLdUniform = glGetUniformLocation(pfShaderProgramObject, "uLd");
    pfLsUniform = glGetUniformLocation(pfShaderProgramObject, "uLs");
    pfKaUniform = glGetUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = glGetUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = glGetUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = glGetUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLightPositionUniform = glGetUniformLocation(pfShaderProgramObject, "uLightPosition");
    pfLKeyPressedUniform = glGetUniformLocation(pfShaderProgramObject, "uLkeyPressed");

    // Initialize sphere
    sphere = [[Sphere alloc]init];
    //[sphere processSphereData];
    
    float spherePositionCoords[1146];
    float sphereNormalCoords[1146];
    float sphereTexCoords[764];
    unsigned short sphereElements[2280];

    [sphere getSphereVertexData:spherePositionCoords :sphereNormalCoords :sphereTexCoords :sphereElements];

    int numSphereVertices = [sphere getNumberOfSphereVertices];
    int numSphereElements = [sphere getNumberOfSphereElements];
    
    // Create VAO for sphere
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);

        // position VBO
        glGenBuffers(1, &vbo_position_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), spherePositionCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // normal VBO
        glGenBuffers(1, &vbo_normal_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), sphereNormalCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        // element VBO - IMPORTANT: bind but stay bound in VAO
        glGenBuffers(1, &vbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numSphereElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);

    glBindVertexArray(0);
    
    printf("DEBUG: VAO=%u, PosVBO=%u, NormVBO=%u, ElemVBO=%u\n", vao_sphere, vbo_position_sphere, vbo_normal_sphere, vbo_element_sphere);

    perspectiveProjectionMatrix = vmath::mat4::identity();
    angle_sphere = 0.0f;
    
    return (0);
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
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);
}

-(void)display
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(!sphere || vao_sphere == 0) {
        printf("ERROR: Sphere not initialized (sphere=%p, vao=%u)\n", sphere, vao_sphere);
        return;
    }
    
    // Use appropriate shader program based on toggle
    if(bPerVertexPerFragmentToggle) {
        glUseProgram(pfShaderProgramObject);
    } else {
        glUseProgram(pvShaderProgramObject);
    }
    
    // initialize modelView matrix to identity
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    
    // translate the model
    modelViewMatrix = vmath::translate(0.0f, 0.0f, -3.0f) * modelViewMatrix;
    
    if(bPerVertexPerFragmentToggle) {
        // Per-Fragment Lighting Path
        glUniformMatrix4fv(pfProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
        glUniformMatrix4fv(pfModelViewMatrixUniform, 1, GL_FALSE, (const float*)modelViewMatrix);
        
        // Pass light properties
        glUniform3fv(pfLaUniform, 1, lightAmbient);
        glUniform3fv(pfLdUniform, 1, lightDiffuse);
        glUniform3fv(pfLsUniform, 1, lightSpecular);
        glUniform3fv(pfKaUniform, 1, materialAmbient);
        glUniform3fv(pfKdUniform, 1, materialDiffuse);
        glUniform3fv(pfKsUniform, 1, materialSpecular);
        glUniform1f(pfMaterialShininessUniform, materialShininess);
        glUniform4f(pfLightPositionUniform, lightPosition[0], lightPosition[1], lightPosition[2], 1.0f);
        glUniform1i(pfLKeyPressedUniform, (bLight == YES ? 1 : 0));
    } else {
        // Per-Vertex Lighting Path
        glUniformMatrix4fv(pvProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
        glUniformMatrix4fv(pvModelViewMatrixUniform, 1, GL_FALSE, (const float*)modelViewMatrix);
        
        // Pass light properties
        glUniform3fv(pvLaUniform, 1, lightAmbient);
        glUniform3fv(pvLdUniform, 1, lightDiffuse);
        glUniform3fv(pvLsUniform, 1, lightSpecular);
        glUniform3fv(pvKaUniform, 1, materialAmbient);
        glUniform3fv(pvKdUniform, 1, materialDiffuse);
        glUniform3fv(pvKsUniform, 1, materialSpecular);
        glUniform1f(pvMaterialShininessUniform, materialShininess);
        glUniform4f(pvLightPositionUniform, lightPosition[0], lightPosition[1], lightPosition[2], 1.0f);
        glUniform1i(pvLKeyPressedUniform, (bLight == YES ? 1 : 0));
    }
    
    // bind VAO
    glBindVertexArray(vao_sphere);
    
    // draw sphere using element array
    int numElements = [sphere getNumberOfSphereElements];
    printf("DEBUG display: About to draw %d elements\n", numElements);
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
    
    // unbind VAO
    glBindVertexArray(0);
    
    glUseProgram(0);
}

-(void)myupdate
{
    if(bAnimate)
    {
        angle_sphere += 2.0f;
        if(angle_sphere >= 360.0f)
        {
            angle_sphere = 0.0f;
        }
    }
}

-(void)uninitialize
{
    if(vbo_position_sphere)
    {
        glDeleteBuffers(1, &vbo_position_sphere);
        vbo_position_sphere = 0;
    }

    if(vbo_normal_sphere)
    {
        glDeleteBuffers(1, &vbo_normal_sphere);
        vbo_normal_sphere = 0;
    }

    if(vbo_element_sphere)
    {
        glDeleteBuffers(1, &vbo_element_sphere);
        vbo_element_sphere = 0;
    }

    if(vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    if(sphere)
    {
        [sphere release];
        sphere = nil;
    }
    
    if(pvShaderProgramObject){
        glUseProgram(pvShaderProgramObject);
        GLint numShaders;
        glGetProgramiv(pvShaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pvShaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pvShaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(pvShaderProgramObject);
        pvShaderProgramObject = 0;
    }
    
    if(pfShaderProgramObject){
        glUseProgram(pfShaderProgramObject);
        GLint numShaders;
        glGetProgramiv(pfShaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pfShaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pfShaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
                free(pShaders);
                pShaders = NULL;
            }
        }
        glUseProgram(0);
        glDeleteProgram(pfShaderProgramObject);
        pfShaderProgramObject = 0;
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

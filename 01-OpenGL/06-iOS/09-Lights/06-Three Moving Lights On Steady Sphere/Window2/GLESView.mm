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
    
    // Per-Vertex uniforms
    GLint pvModelMatrixUniform;
    GLint pvViewMatrixUniform;
    GLint pvProjectionMatrixUniform;
    GLint pvLaUniform[3];
    GLint pvLdUniform[3];
    GLint pvLsUniform[3];
    GLint pvLightPositionUniform[3];
    GLint pvKaUniform;
    GLint pvKdUniform;
    GLint pvKsUniform;
    GLint pvMaterialShininessUniform;
    GLint pvLKeyPressedUniform;
    
    // Per-Fragment uniforms
    GLint pfModelMatrixUniform;
    GLint pfViewMatrixUniform;
    GLint pfProjectionMatrixUniform;
    GLint pfLaUniform[3];
    GLint pfLdUniform[3];
    GLint pfLsUniform[3];
    GLint pfLightPositionUniform[3];
    GLint pfKaUniform;
    GLint pfKdUniform;
    GLint pfKsUniform;
    GLint pfMaterialShininessUniform;
    GLint pfLKeyPressedUniform;
    
    Sphere *sphere;
    vmath::mat4 perspectiveProjectionMatrix;
    
    // Light properties - 3 lights
    float lightAmbient[3][4];
    float lightDiffuse[3][4];
    float lightSpecular[3][4];
    float lightPosition[3][4];
    float lightAngle[3];
    
    // Material properties
    float materialAmbient[4];
    float materialDiffuse[4];
    float materialSpecular[4];
    float materialShininess;
    
    BOOL bLight;
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
        
        // Initialize light properties - 3 lights (Red, Green, Blue)
        // Light 0: Red
        lightAmbient[0][0] = 0.0f; lightAmbient[0][1] = 0.0f; lightAmbient[0][2] = 0.0f; lightAmbient[0][3] = 1.0f;
        lightDiffuse[0][0] = 1.0f; lightDiffuse[0][1] = 0.0f; lightDiffuse[0][2] = 0.0f; lightDiffuse[0][3] = 1.0f;
        lightSpecular[0][0] = 1.0f; lightSpecular[0][1] = 0.0f; lightSpecular[0][2] = 0.0f; lightSpecular[0][3] = 1.0f;
        lightPosition[0][0] = 0.0f; lightPosition[0][1] = 0.0f; lightPosition[0][2] = 0.0f; lightPosition[0][3] = 1.0f;
        lightAngle[0] = 0.0f;
        
        // Light 1: Green
        lightAmbient[1][0] = 0.0f; lightAmbient[1][1] = 0.0f; lightAmbient[1][2] = 0.0f; lightAmbient[1][3] = 1.0f;
        lightDiffuse[1][0] = 0.0f; lightDiffuse[1][1] = 1.0f; lightDiffuse[1][2] = 0.0f; lightDiffuse[1][3] = 1.0f;
        lightSpecular[1][0] = 0.0f; lightSpecular[1][1] = 1.0f; lightSpecular[1][2] = 0.0f; lightSpecular[1][3] = 1.0f;
        lightPosition[1][0] = 0.0f; lightPosition[1][1] = 0.0f; lightPosition[1][2] = 0.0f; lightPosition[1][3] = 1.0f;
        lightAngle[1] = 0.0f;
        
        // Light 2: Blue
        lightAmbient[2][0] = 0.0f; lightAmbient[2][1] = 0.0f; lightAmbient[2][2] = 0.0f; lightAmbient[2][3] = 1.0f;
        lightDiffuse[2][0] = 0.0f; lightDiffuse[2][1] = 0.0f; lightDiffuse[2][2] = 1.0f; lightDiffuse[2][3] = 1.0f;
        lightSpecular[2][0] = 0.0f; lightSpecular[2][1] = 0.0f; lightSpecular[2][2] = 1.0f; lightSpecular[2][3] = 1.0f;
        lightPosition[2][0] = 0.0f; lightPosition[2][1] = 0.0f; lightPosition[2][2] = 0.0f; lightPosition[2][3] = 1.0f;
        lightAngle[2] = 0.0f;
        
        // Material properties - white material
        materialAmbient[0] = 0.0f; materialAmbient[1] = 0.0f; materialAmbient[2] = 0.0f; materialAmbient[3] = 1.0f;
        materialDiffuse[0] = 1.0f; materialDiffuse[1] = 1.0f; materialDiffuse[2] = 1.0f; materialDiffuse[3] = 1.0f;
        materialSpecular[0] = 1.0f; materialSpecular[1] = 1.0f; materialSpecular[2] = 1.0f; materialSpecular[3] = 1.0f;
        materialShininess = 128.0f;
        
        bLight = YES;
        bPerVertexPerFragmentToggle = YES; // Start with per-fragment
        
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
    // Toggle light on/off
    bLight = !bLight;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Toggle per-vertex/per-fragment
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
    
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    
    // ==================== PER-VERTEX SHADER PROGRAM ====================
    // Per-Vertex Vertex Shader
    GLuint pvVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pvVertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "uniform mat4 uModelMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec3 uLa[3];\n"
        "uniform vec3 uLd[3];\n"
        "uniform vec3 uLs[3];\n"
        "uniform vec4 uLightPosition[3];\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform int uLKeyPressed;\n"
        "out vec4 out_phong_ads_Light;\n"
        "void main(void)\n"
        "{\n"
        "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n"
        "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n"
        "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n"
        "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n"
        "       vec3 lightSource[3];\n"
        "       float tnDotLd[3];\n"
        "       vec3 reflectedVector[3];\n"
        "       vec3 ambient[3];\n"
        "       vec3 diffuse[3];\n"
        "       vec3 specular[3];\n"
        "       for(int i = 0; i < 3; i++)\n"
        "       {\n"
        "           lightSource[i] = normalize(vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n"
        "           tnDotLd[i] = max(dot(lightSource[i], transformedNormal), 0.0);\n"
        "           reflectedVector[i] = reflect(-lightSource[i], transformedNormal);\n"
        "           ambient[i] = uLa[i] * uKa;\n"
        "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n"
        "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], viewerVector), 0.0), uMaterialShininess);\n"
        "           out_phong_ads_Light = out_phong_ads_Light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n"
        "       }\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       out_phong_ads_Light = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "   }\n"
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"
        "}\n";
    glShaderSource(pvVertexShaderObject, 1, &pvVertexShaderSourceCode, NULL);
    glCompileShader(pvVertexShaderObject);
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
        "precision highp float;\n"
        "in vec4 out_phong_ads_Light;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   FragColor = out_phong_ads_Light;\n"
        "}\n";
    glShaderSource(pvFragmentShaderObject, 1, &pvFragmentShaderSourceCode, NULL);
    glCompileShader(pvFragmentShaderObject);
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
    glAttachShader(pvShaderProgramObject, pvVertexShaderObject);
    glAttachShader(pvShaderProgramObject, pvFragmentShaderObject);
    glBindAttribLocation(pvShaderProgramObject, 0, "aPosition");
    glBindAttribLocation(pvShaderProgramObject, 1, "aNormal");
    glLinkProgram(pvShaderProgramObject);
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
        return -9;
    }

    // Get PV uniform locations
    pvModelMatrixUniform = glGetUniformLocation(pvShaderProgramObject, "uModelMatrix");
    pvViewMatrixUniform = glGetUniformLocation(pvShaderProgramObject, "uViewMatrix");
    pvProjectionMatrixUniform = glGetUniformLocation(pvShaderProgramObject, "uProjectionMatrix");
    pvLaUniform[0] = glGetUniformLocation(pvShaderProgramObject, "uLa[0]");
    pvLdUniform[0] = glGetUniformLocation(pvShaderProgramObject, "uLd[0]");
    pvLsUniform[0] = glGetUniformLocation(pvShaderProgramObject, "uLs[0]");
    pvLightPositionUniform[0] = glGetUniformLocation(pvShaderProgramObject, "uLightPosition[0]");
    pvLaUniform[1] = glGetUniformLocation(pvShaderProgramObject, "uLa[1]");
    pvLdUniform[1] = glGetUniformLocation(pvShaderProgramObject, "uLd[1]");
    pvLsUniform[1] = glGetUniformLocation(pvShaderProgramObject, "uLs[1]");
    pvLightPositionUniform[1] = glGetUniformLocation(pvShaderProgramObject, "uLightPosition[1]");
    pvLaUniform[2] = glGetUniformLocation(pvShaderProgramObject, "uLa[2]");
    pvLdUniform[2] = glGetUniformLocation(pvShaderProgramObject, "uLd[2]");
    pvLsUniform[2] = glGetUniformLocation(pvShaderProgramObject, "uLs[2]");
    pvLightPositionUniform[2] = glGetUniformLocation(pvShaderProgramObject, "uLightPosition[2]");
    pvKaUniform = glGetUniformLocation(pvShaderProgramObject, "uKa");
    pvKdUniform = glGetUniformLocation(pvShaderProgramObject, "uKd");
    pvKsUniform = glGetUniformLocation(pvShaderProgramObject, "uKs");
    pvMaterialShininessUniform = glGetUniformLocation(pvShaderProgramObject, "uMaterialShininess");
    pvLKeyPressedUniform = glGetUniformLocation(pvShaderProgramObject, "uLKeyPressed");

    // ==================== PER-FRAGMENT SHADER PROGRAM ====================
    // Per-Fragment Vertex Shader
    GLuint pfVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pfVertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "out vec4 eyeCoordinates;\n"
        "out vec3 transformedNormal;\n"
        "out vec3 lightSource[3];\n"
        "uniform mat4 uModelMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec4 uLightPosition[3];\n"
        "void main(void)\n"
        "{\n"
        "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n"
        "   transformedNormal = normalMatrix * aNormal;\n"
        "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n"
        "   for(int i = 0; i < 3; i++)\n"
        "   {\n"
        "       lightSource[i] = vec3(uLightPosition[i]) - eyeCoordinates.xyz;\n"
        "   }\n"
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"
        "}\n";
    glShaderSource(pfVertexShaderObject, 1, &pfVertexShaderSourceCode, NULL);
    glCompileShader(pfVertexShaderObject);
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
        return -10;
    }

    // Per-Fragment Fragment Shader
    GLuint pfFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pfFragmentShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec3 transformedNormal;\n"
        "in vec4 eyeCoordinates;\n"
        "in vec3 lightSource[3];\n"
        "uniform vec3 uLa[3];\n"
        "uniform vec3 uLd[3];\n"
        "uniform vec3 uLs[3];\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform int uLKeyPressed;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   vec4 phong_ads_light = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n"
        "   vec3 normalizedViewerVector = normalize(-eyeCoordinates.xyz);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       float tnDotLd[3];\n"
        "       vec3 reflectedVector[3];\n"
        "       vec3 ambient[3];\n"
        "       vec3 diffuse[3];\n"
        "       vec3 specular[3];\n"
        "       for(int i = 0; i < 3; i++)\n"
        "       {\n"
        "           vec3 normalizedLightSource = normalize(lightSource[i]);\n"
        "           tnDotLd[i] = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "           reflectedVector[i] = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "           ambient[i] = uLa[i] * uKa;\n"
        "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n"
        "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], normalizedViewerVector), 0.0), uMaterialShininess);\n"
        "           phong_ads_light = phong_ads_light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n"
        "       }\n"
        "       FragColor = phong_ads_light;\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "   }\n"
        "}\n";
    glShaderSource(pfFragmentShaderObject, 1, &pfFragmentShaderSourceCode, NULL);
    glCompileShader(pfFragmentShaderObject);
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
        return -11;
    }

    // Per-Fragment Shader Program
    pfShaderProgramObject = glCreateProgram();
    glAttachShader(pfShaderProgramObject, pfVertexShaderObject);
    glAttachShader(pfShaderProgramObject, pfFragmentShaderObject);
    glBindAttribLocation(pfShaderProgramObject, 0, "aPosition");
    glBindAttribLocation(pfShaderProgramObject, 1, "aNormal");
    glLinkProgram(pfShaderProgramObject);
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
        return -12;
    }

    // Get PF uniform locations
    pfModelMatrixUniform = glGetUniformLocation(pfShaderProgramObject, "uModelMatrix");
    pfViewMatrixUniform = glGetUniformLocation(pfShaderProgramObject, "uViewMatrix");
    pfProjectionMatrixUniform = glGetUniformLocation(pfShaderProgramObject, "uProjectionMatrix");
    pfLaUniform[0] = glGetUniformLocation(pfShaderProgramObject, "uLa[0]");
    pfLdUniform[0] = glGetUniformLocation(pfShaderProgramObject, "uLd[0]");
    pfLsUniform[0] = glGetUniformLocation(pfShaderProgramObject, "uLs[0]");
    pfLightPositionUniform[0] = glGetUniformLocation(pfShaderProgramObject, "uLightPosition[0]");
    pfLaUniform[1] = glGetUniformLocation(pfShaderProgramObject, "uLa[1]");
    pfLdUniform[1] = glGetUniformLocation(pfShaderProgramObject, "uLd[1]");
    pfLsUniform[1] = glGetUniformLocation(pfShaderProgramObject, "uLs[1]");
    pfLightPositionUniform[1] = glGetUniformLocation(pfShaderProgramObject, "uLightPosition[1]");
    pfLaUniform[2] = glGetUniformLocation(pfShaderProgramObject, "uLa[2]");
    pfLdUniform[2] = glGetUniformLocation(pfShaderProgramObject, "uLd[2]");
    pfLsUniform[2] = glGetUniformLocation(pfShaderProgramObject, "uLs[2]");
    pfLightPositionUniform[2] = glGetUniformLocation(pfShaderProgramObject, "uLightPosition[2]");
    pfKaUniform = glGetUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = glGetUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = glGetUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = glGetUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLKeyPressedUniform = glGetUniformLocation(pfShaderProgramObject, "uLKeyPressed");

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
        return;
    }
    
    // Choose shader based on toggle
    if(bPerVertexPerFragmentToggle) {
        glUseProgram(pfShaderProgramObject);
    } else {
        glUseProgram(pvShaderProgramObject);
    }
    
    // Initialize matrices
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
    
    // Translate sphere
    modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    
    // Bind VAO
    glBindVertexArray(vao_sphere);
    
    // Convert angles to radians
    float radians0 = lightAngle[0] * M_PI / 180.0f;
    float radians1 = lightAngle[1] * M_PI / 180.0f;
    float radians2 = lightAngle[2] * M_PI / 180.0f;
    
    // Compute light positions using trigonometry
    // Light 0 - rotates around Y axis (Red) - varies in X and Z
    float lightPos0[4] = { 20.0f * sinf(radians0), 0.0f, 20.0f * cosf(radians0), 1.0f };
    // Light 1 - rotates around X axis (Green) - varies in Y and Z
    float lightPos1[4] = { 0.0f, 20.0f * sinf(radians1), 20.0f * cosf(radians1), 1.0f };
    // Light 2 - rotates around Z axis (Blue) - varies in X and Y
    float lightPos2[4] = { 20.0f * cosf(radians2), 20.0f * sinf(radians2), 0.0f, 1.0f };
    
    if(bPerVertexPerFragmentToggle) {
        // Per-Fragment rendering
        glUniformMatrix4fv(pfModelMatrixUniform, 1, GL_FALSE, (const float*)modelMatrix);
        glUniformMatrix4fv(pfViewMatrixUniform, 1, GL_FALSE, (const float*)viewMatrix);
        glUniformMatrix4fv(pfProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
        
        // Light 0 - Red
        glUniform3fv(pfLaUniform[0], 1, lightAmbient[0]);
        glUniform3fv(pfLdUniform[0], 1, lightDiffuse[0]);
        glUniform3fv(pfLsUniform[0], 1, lightSpecular[0]);
        glUniform4fv(pfLightPositionUniform[0], 1, lightPos0);
        
        // Light 1 - Green
        glUniform3fv(pfLaUniform[1], 1, lightAmbient[1]);
        glUniform3fv(pfLdUniform[1], 1, lightDiffuse[1]);
        glUniform3fv(pfLsUniform[1], 1, lightSpecular[1]);
        glUniform4fv(pfLightPositionUniform[1], 1, lightPos1);
        
        // Light 2 - Blue
        glUniform3fv(pfLaUniform[2], 1, lightAmbient[2]);
        glUniform3fv(pfLdUniform[2], 1, lightDiffuse[2]);
        glUniform3fv(pfLsUniform[2], 1, lightSpecular[2]);
        glUniform4fv(pfLightPositionUniform[2], 1, lightPos2);
        
        // Material properties
        glUniform3fv(pfKaUniform, 1, materialAmbient);
        glUniform3fv(pfKdUniform, 1, materialDiffuse);
        glUniform3fv(pfKsUniform, 1, materialSpecular);
        glUniform1f(pfMaterialShininessUniform, materialShininess);
        glUniform1i(pfLKeyPressedUniform, bLight ? 1 : 0);
    } else {
        // Per-Vertex rendering
        glUniformMatrix4fv(pvModelMatrixUniform, 1, GL_FALSE, (const float*)modelMatrix);
        glUniformMatrix4fv(pvViewMatrixUniform, 1, GL_FALSE, (const float*)viewMatrix);
        glUniformMatrix4fv(pvProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
        
        // Light 0 - Red
        glUniform3fv(pvLaUniform[0], 1, lightAmbient[0]);
        glUniform3fv(pvLdUniform[0], 1, lightDiffuse[0]);
        glUniform3fv(pvLsUniform[0], 1, lightSpecular[0]);
        glUniform4fv(pvLightPositionUniform[0], 1, lightPos0);
        
        // Light 1 - Green
        glUniform3fv(pvLaUniform[1], 1, lightAmbient[1]);
        glUniform3fv(pvLdUniform[1], 1, lightDiffuse[1]);
        glUniform3fv(pvLsUniform[1], 1, lightSpecular[1]);
        glUniform4fv(pvLightPositionUniform[1], 1, lightPos1);
        
        // Light 2 - Blue
        glUniform3fv(pvLaUniform[2], 1, lightAmbient[2]);
        glUniform3fv(pvLdUniform[2], 1, lightDiffuse[2]);
        glUniform3fv(pvLsUniform[2], 1, lightSpecular[2]);
        glUniform4fv(pvLightPositionUniform[2], 1, lightPos2);
        
        // Material properties
        glUniform3fv(pvKaUniform, 1, materialAmbient);
        glUniform3fv(pvKdUniform, 1, materialDiffuse);
        glUniform3fv(pvKsUniform, 1, materialSpecular);
        glUniform1f(pvMaterialShininessUniform, materialShininess);
        glUniform1i(pvLKeyPressedUniform, bLight ? 1 : 0);
    }
    
    // Draw sphere
    int numElements = [sphere getNumberOfSphereElements];
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

-(void)myupdate
{
    // Update light angles - lights always rotate
    for(int i = 0; i < 3; i++) {
        lightAngle[i] += 3.0f;
        if(lightAngle[i] >= 360.0f) {
            lightAngle[i] = 0.0f;
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

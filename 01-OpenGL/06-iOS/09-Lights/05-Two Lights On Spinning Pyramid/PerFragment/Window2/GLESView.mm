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
    GLuint vao_pyramid;
    GLuint vbo_position_pyramid;
    GLuint vbo_normal_pyramid;
    GLint modelViewMatrixUniform;
    GLint projectionMatrixUniform;
    GLint laUniform[2];
    GLint ldUniform[2];
    GLint lsUniform[2];
    GLint lightPositionUniform[2];
    GLint kaUniform;
    GLint kdUniform;
    GLint ksUniform;
    GLint materialShininessUniform;
    GLint lKeyPressedUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    float angle_pyramid;
    
    // Light / material properties
    float lightAmbient[2][3];
    float lightDiffuse[2][3];
    float lightSpecular[2][3];
    float lightPosition[2][4];
    float materialAmbient[3];
    float materialDiffuse[3];
    float materialSpecular[3];
    float materialShininess;
    BOOL bLight;
    BOOL bAnimation;
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
        
        // Initialize light properties - matching OGL.cpp with two lights
        // Light 0: Red light
        lightAmbient[0][0] = 0.0f; lightAmbient[0][1] = 0.0f; lightAmbient[0][2] = 0.0f;
        lightDiffuse[0][0] = 1.0f; lightDiffuse[0][1] = 0.0f; lightDiffuse[0][2] = 0.0f;
        lightSpecular[0][0] = 1.0f; lightSpecular[0][1] = 0.0f; lightSpecular[0][2] = 0.0f;
        lightPosition[0][0] = -2.0f; lightPosition[0][1] = 0.0f; lightPosition[0][2] = 0.0f; lightPosition[0][3] = 1.0f;
        
        // Light 1: Blue light
        lightAmbient[1][0] = 0.0f; lightAmbient[1][1] = 0.0f; lightAmbient[1][2] = 0.0f;
        lightDiffuse[1][0] = 0.0f; lightDiffuse[1][1] = 0.0f; lightDiffuse[1][2] = 1.0f;
        lightSpecular[1][0] = 0.0f; lightSpecular[1][1] = 0.0f; lightSpecular[1][2] = 1.0f;
        lightPosition[1][0] = 2.0f; lightPosition[1][1] = 0.0f; lightPosition[1][2] = 0.0f; lightPosition[1][3] = 1.0f;
        
        // Material properties - white material
        materialAmbient[0] = 0.0f; materialAmbient[1] = 0.0f; materialAmbient[2] = 0.0f;
        materialDiffuse[0] = 1.0f; materialDiffuse[1] = 1.0f; materialDiffuse[2] = 1.0f;
        materialSpecular[0] = 1.0f; materialSpecular[1] = 1.0f; materialSpecular[2] = 1.0f;
        materialShininess = 128.0f;
        
        bLight = NO;
        bAnimation = YES;

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
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Toggle animation
    bAnimation = !bAnimation;
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
    
    // Vertex Shader
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode =
        "#version 300 es\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "out vec4 eyeCoordinates;\n"
        "out vec3 transformedNormal;\n"
        "out vec3 lightSource[2];\n"
        "uniform mat4 uModelViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec4 uLightPosition[2];\n"
        "void main(void)\n"
        "{\n"
        "   mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));\n"
        "   transformedNormal = normalize(normalMatrix * aNormal);\n"
        "   eyeCoordinates = uModelViewMatrix * aPosition;\n"
        "   for(int i = 0; i < 2; i++)\n"
        "   {\n"
        "       lightSource[i] = (vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n"
        "   }\n"
        "   gl_Position = uProjectionMatrix * uModelViewMatrix * aPosition;\n"
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

    // Fragment Shader
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec3 transformedNormal;\n"
        "in vec4 eyeCoordinates;\n"
        "in vec3 lightSource[2];\n"
        "uniform vec3 uLa[2];\n"
        "uniform vec3 uLd[2];\n"
        "uniform vec3 uLs[2];\n"
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
        "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       float tnDotLd[2];\n"
        "       vec3 reflectedVector[2];\n"
        "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n"
        "       vec3 ambient[2];\n"
        "       vec3 diffuse[2];\n"
        "       vec3 specular[2];\n"
        "       for(int i = 0; i < 2; i++)\n"
        "       {\n"
        "           vec3 normalizedLightSource = normalize(lightSource[i]);\n"
        "           tnDotLd[i] = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "           reflectedVector[i] = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "           ambient[i] = uLa[i] * uKa;\n"
        "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n"
        "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], viewerVector), 0.0), uMaterialShininess);\n"
        "           phong_ads_light = phong_ads_light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n"
        "       }\n"
        "       FragColor = phong_ads_light;\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
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

    // Attach vertex shader to the shader program object
    glAttachShader(shaderProgramObject, vertexShaderObject);

    // Attach fragment shader to the shader program object
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // Bind the vertex attribute at a certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject, 0, "aPosition");
    glBindAttribLocation(shaderProgramObject, 1, "aNormal");

    // Link the shader program and check for errors
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

    // Get the required uniform locations from the shader program object
    modelViewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    laUniform[0] = glGetUniformLocation(shaderProgramObject, "uLa[0]");
    ldUniform[0] = glGetUniformLocation(shaderProgramObject, "uLd[0]");
    lsUniform[0] = glGetUniformLocation(shaderProgramObject, "uLs[0]");
    laUniform[1] = glGetUniformLocation(shaderProgramObject, "uLa[1]");
    ldUniform[1] = glGetUniformLocation(shaderProgramObject, "uLd[1]");
    lsUniform[1] = glGetUniformLocation(shaderProgramObject, "uLs[1]");
    kaUniform = glGetUniformLocation(shaderProgramObject, "uKa");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "uLightPosition[0]");
    lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "uLightPosition[1]");
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLKeyPressed");

    // pyramid
    {
        const GLfloat pyramid_position[] = {
            // front
            0.0f,  1.0f,  0.0f, // front-top
            -1.0f, -1.0f,  1.0f, // front-left
            1.0f, -1.0f,  1.0f, // front-right
            
            // right
            0.0f,  1.0f,  0.0f, // right-top
            1.0f, -1.0f,  1.0f, // right-left
            1.0f, -1.0f, -1.0f, // right-right

            // back
            0.0f,  1.0f,  0.0f, // back-top
            1.0f, -1.0f, -1.0f, // back-left
            -1.0f, -1.0f, -1.0f, // back-right

            // left
            0.0f,  1.0f,  0.0f, // left-top
            -1.0f, -1.0f, -1.0f, // left-left
            -1.0f, -1.0f,  1.0f, // left-right
        };

        const GLfloat pyramid_normal[] = {
            // front
            0.000000f, 0.447214f,  0.894427f,
            0.000000f, 0.447214f,  0.894427f,
            0.000000f, 0.447214f,  0.894427f,

            // right
            0.894427f, 0.447214f,  0.000000f,
            0.894427f, 0.447214f,  0.000000f,
            0.894427f, 0.447214f,  0.000000f,

            // back
            0.000000f, 0.447214f, -0.894427f,
            0.000000f, 0.447214f, -0.894427f,
            0.000000f, 0.447214f, -0.894427f,

            // left
            -0.894427f, 0.447214f,  0.000000f,
            -0.894427f, 0.447214f,  0.000000f,
            -0.894427f, 0.447214f,  0.000000f,
        };

        // Create Vertex Array Object (VAO) for array of vertex attributes
        glGenVertexArrays(1, &vao_pyramid);
        glBindVertexArray(vao_pyramid);
        {
            // Position VBO
            {
                glGenBuffers(1, &vbo_position_pyramid);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_position_pyramid);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_position), pyramid_position, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(0);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            // Normal VBO
            {
                glGenBuffers(1, &vbo_normal_pyramid);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_pyramid);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_normal), pyramid_normal, GL_STATIC_DRAW);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(1);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        glBindVertexArray(0); // Unbind the VAO
    }

    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    angle_pyramid = 0.0f;
    
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
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, width/height, 0.1f, 100.0f);
}

-(void)display
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use the shader program object
    glUseProgram(shaderProgramObject);
    {
        // Transformations
        vmath::mat4 modelViewMatrix = vmath::mat4::identity();
        vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
        vmath::mat4 translationMatrix = vmath::mat4::identity();
        vmath::mat4 rotationMatrix = vmath::mat4::identity();
        vmath::mat4 scaleMatrix = vmath::mat4::identity();
        {
            translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
            rotationMatrix = vmath::rotate(angle_pyramid, 0.0f, 1.0f, 0.0f);

            // Modeview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelViewMatrix = translationMatrix * rotationMatrix;

            // Pass model view and projection matrices
            glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, (const float*)modelViewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
            
            // Pass light and material properties for both lights
            for(int i = 0; i < 2; i++){
                glUniform3f(laUniform[i], lightAmbient[i][0], lightAmbient[i][1], lightAmbient[i][2]);
                glUniform3f(ldUniform[i], lightDiffuse[i][0], lightDiffuse[i][1], lightDiffuse[i][2]);
                glUniform3f(lsUniform[i], lightSpecular[i][0], lightSpecular[i][1], lightSpecular[i][2]);
                glUniform4fv(lightPositionUniform[i], 1, lightPosition[i]);
            }
            glUniform3f(kaUniform, materialAmbient[0], materialAmbient[1], materialAmbient[2]);
            glUniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
            glUniform3f(ksUniform, materialSpecular[0], materialSpecular[1], materialSpecular[2]);
            glUniform1f(materialShininessUniform, materialShininess);
            glUniform1i(lKeyPressedUniform, bLight ? 1 : 0);

            // Bind the VAO for pyramid
            glBindVertexArray(vao_pyramid);
            {
                // Draw the pyramid
                glDrawArrays(GL_TRIANGLES, 0, 12);
            }
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);
}

-(void)myupdate
{
    if(bAnimation)
        angle_pyramid += 10.05f;
}

-(void)uninitialize
{
    if(vbo_normal_pyramid)
    {
        glDeleteBuffers(1, &vbo_normal_pyramid);
        vbo_normal_pyramid = 0;
    }

    if(vbo_position_pyramid)
    {
        glDeleteBuffers(1, &vbo_position_pyramid);
        vbo_position_pyramid = 0;
    }

    if(vao_pyramid)
    {
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
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

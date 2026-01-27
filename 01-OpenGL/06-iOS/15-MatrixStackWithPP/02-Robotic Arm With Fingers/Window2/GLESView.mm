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
#include <stack>

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
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;
    GLint mvpMatrixUniform;
    GLint colorUniform;
    Sphere *sphere;
    vmath::mat4 perspectiveProjectionMatrix;
    int numSphereElements;
    
    // Matrix stack for hierarchical transformations
    std::stack<vmath::mat4> transformationMatrixStack;
    
    // Robotic arm joint angles
    int shoulder;
    int elbow;
    int wrist;
    int finger1;
    int finger2;
    int finger3;
    int finger4;
    int finger5;
    
    // Which joint to rotate (0-7)
    int whatToRotate;
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
        
        // Initialize arm joint angles
        shoulder = 0;
        elbow = 0;
        wrist = 0;
        finger1 = 0;
        finger2 = 0;
        finger3 = 0;
        finger4 = 0;
        finger5 = 0;
        whatToRotate = 0;
        
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
    // Increase the selected joint angle by 10 degrees
    if(whatToRotate == 0) shoulder += 10;
    else if(whatToRotate == 1) elbow += 10;
    else if(whatToRotate == 2) wrist += 10;
    else if(whatToRotate == 3) finger1 += 10;
    else if(whatToRotate == 4) finger2 += 10;
    else if(whatToRotate == 5) finger3 += 10;
    else if(whatToRotate == 6) finger4 += 10;
    else if(whatToRotate == 7) finger5 += 10;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Cycle through joints (0-7)
    whatToRotate++;
    if(whatToRotate > 7) {
        whatToRotate = 0;
    }
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
    
    // Vertex Shader (simple color pass-through)
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode =
        "#version 300 es\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "in vec4 aColor;\n"
        "out vec4 out_color;\n"
        "uniform mat4 uMVPMatrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = uMVPMatrix * aPosition;\n"
        "    out_color = aColor;\n"
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
        "in vec4 out_color;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "    FragColor = out_color;\n"
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
    glBindAttribLocation(shaderProgramObject, 0, "aPosition");
    glBindAttribLocation(shaderProgramObject, 1, "aNormal");
    glBindAttribLocation(shaderProgramObject, 2, "aColor");
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
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    // Initialize sphere
    sphere = [[Sphere alloc] init];
    
    float spherePositionCoords[1146];
    float sphereNormalCoords[1146];
    float sphereTexCoords[764];
    unsigned short sphereElements[2280];

    [sphere getSphereVertexData:spherePositionCoords :sphereNormalCoords :sphereTexCoords :sphereElements];

    int numSphereVertices = [sphere getNumberOfSphereVertices];
    numSphereElements = [sphere getNumberOfSphereElements];
    
    // Create VAO for sphere
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
    {
        // Position VBO
        glGenBuffers(1, &vbo_position_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), spherePositionCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Normal VBO
        glGenBuffers(1, &vbo_normal_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), sphereNormalCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Element VBO
        glGenBuffers(1, &vbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numSphereElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);
        // DO NOT unbind element buffer - it's part of VAO state
    }
    glBindVertexArray(0);

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
    
    glUseProgram(shaderProgramObject);
    {
        vmath::mat4 modelViewMatrix = vmath::mat4::identity();
        vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
        vmath::mat4 scaleMatrix = vmath::mat4::identity();
        vmath::mat4 rotationMatrix = vmath::mat4::identity();
        vmath::mat4 translationMatrix = vmath::mat4::identity();

        // Initial translation to move scene back
        translationMatrix = vmath::translate(0.0f, 0.0f, -12.0f);
        modelViewMatrix = translationMatrix;

        // Brown color for arm (0.5, 0.35, 0.05)
        glVertexAttrib4f(2, 0.5f, 0.35f, 0.05f, 1.0f);

        // ============== UPPER ARM (Shoulder) ==============
        transformationMatrixStack.push(modelViewMatrix);
        {
            // Rotate shoulder, then translate
            rotationMatrix = vmath::rotate((float)shoulder, 0.0f, 0.0f, 1.0f);
            translationMatrix = vmath::translate(1.0f, 0.0f, 0.0f);
            modelViewMatrix = modelViewMatrix * translationMatrix * rotationMatrix;

            // Draw upper arm
            transformationMatrixStack.push(modelViewMatrix);
            {
                scaleMatrix = vmath::scale(2.1f, 0.6f, 1.0f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FOREARM (Elbow) ==============
            translationMatrix = vmath::translate(1.0f, 0.0f, 0.0f);
            rotationMatrix = vmath::rotate((float)elbow, 0.0f, 0.0f, 1.0f);
            modelViewMatrix = modelViewMatrix * translationMatrix * rotationMatrix;
            translationMatrix = vmath::translate(1.0f, 0.0f, 0.0f);
            modelViewMatrix = modelViewMatrix * translationMatrix;

            // Draw forearm
            transformationMatrixStack.push(modelViewMatrix);
            {
                scaleMatrix = vmath::scale(2.1f, 0.6f, 1.0f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== WRIST ==============
            translationMatrix = vmath::translate(1.0f, 0.0f, 0.0f);
            rotationMatrix = vmath::rotate((float)wrist, 0.0f, 0.0f, 1.0f);
            modelViewMatrix = modelViewMatrix * translationMatrix * rotationMatrix;
            translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
            modelViewMatrix = modelViewMatrix * translationMatrix;

            // Draw wrist
            transformationMatrixStack.push(modelViewMatrix);
            {
                scaleMatrix = vmath::scale(1.0f, 0.4f, 1.0f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FINGER 1 (Thumb - 45 degrees Y) ==============
            transformationMatrixStack.push(modelViewMatrix);
            {
                rotationMatrix = vmath::rotate(45.0f, 0.0f, 1.0f, 0.0f);
                translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                rotationMatrix = vmath::rotate((float)finger1, 0.0f, 0.0f, 1.0f);
                translationMatrix = vmath::translate(0.3f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                scaleMatrix = vmath::scale(0.5f, 0.01f, 0.1f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FINGER 2 (Index - 30 degrees Y) ==============
            transformationMatrixStack.push(modelViewMatrix);
            {
                rotationMatrix = vmath::rotate(30.0f, 0.0f, 1.0f, 0.0f);
                translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                rotationMatrix = vmath::rotate((float)finger2, 0.0f, 0.0f, 1.0f);
                translationMatrix = vmath::translate(0.3f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                scaleMatrix = vmath::scale(0.5f, 0.01f, 0.1f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FINGER 3 (Middle - 0 degrees Y) ==============
            transformationMatrixStack.push(modelViewMatrix);
            {
                translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * translationMatrix;

                rotationMatrix = vmath::rotate((float)finger3, 0.0f, 0.0f, 1.0f);
                translationMatrix = vmath::translate(0.3f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                scaleMatrix = vmath::scale(0.5f, 0.01f, 0.1f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FINGER 4 (Ring - -30 degrees Y) ==============
            transformationMatrixStack.push(modelViewMatrix);
            {
                rotationMatrix = vmath::rotate(-30.0f, 0.0f, 1.0f, 0.0f);
                translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                rotationMatrix = vmath::rotate((float)finger4, 0.0f, 0.0f, 1.0f);
                translationMatrix = vmath::translate(0.3f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                scaleMatrix = vmath::scale(0.5f, 0.01f, 0.1f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();

            // ============== FINGER 5 (Pinky - -45 degrees Y) ==============
            transformationMatrixStack.push(modelViewMatrix);
            {
                rotationMatrix = vmath::rotate(-45.0f, 0.0f, 1.0f, 0.0f);
                translationMatrix = vmath::translate(0.5f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                rotationMatrix = vmath::rotate((float)finger5, 0.0f, 0.0f, 1.0f);
                translationMatrix = vmath::translate(0.3f, 0.0f, 0.0f);
                modelViewMatrix = modelViewMatrix * rotationMatrix * translationMatrix;

                scaleMatrix = vmath::scale(0.5f, 0.01f, 0.1f);
                vmath::mat4 scaledMV = modelViewMatrix * scaleMatrix;
                modelViewProjectionMatrix = perspectiveProjectionMatrix * scaledMV;
                glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

                glBindVertexArray(vao_sphere);
                glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
                glBindVertexArray(0);
            }
            modelViewMatrix = transformationMatrixStack.top();
            transformationMatrixStack.pop();
        }
        modelViewMatrix = transformationMatrixStack.top();
        transformationMatrixStack.pop();
    }
    glUseProgram(0);
}

-(void)myupdate
{
    // No automatic animation - arm is controlled by gestures
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

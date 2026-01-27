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
    GLuint vao_cube;
    GLuint vbo_position_cube;
    GLuint vbo_normal_cube;
    GLint modelViewMatrixUniform;
    GLint projectionMatrixUniform;
    GLint ldUniform;
    GLint kdUniform;
    GLint lightPositionUniform;
    GLint lKeyPressedUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    float angle_cube;
    
    // Light properties
    float lightDiffuse[3];
    float materialDiffuse[3];
    float lightPosition[4];
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
        
        
        // Initialize light properties
        lightDiffuse[0] = 1.0f; lightDiffuse[1] = 1.0f; lightDiffuse[2] = 1.0f;
        materialDiffuse[0] = 0.5f; materialDiffuse[1] = 0.5f; materialDiffuse[2] = 0.5f;
        lightPosition[0] = 0.0f; lightPosition[1] = 0.0f; lightPosition[2] = 2.0f; lightPosition[3] = 1.0f;
        bLight = FALSE;
        bAnimation = TRUE;
        
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
        "uniform mat4 uModelViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uKd;\n"
        "uniform vec4 uLightPosition;\n"
        "uniform int uLKeyPressed;\n"
        "out vec4 out_diffuseLight;\n"
        "void main(void)\n"
        "{\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       vec4 eyeCoordinates = uModelViewMatrix * aPosition;\n"
        "       mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));\n"
        "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n"
        "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n"
        "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n"
        "       out_diffuseLight = vec4(uLd * uKd * tnDotLd, 1.0);\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       out_diffuseLight = vec4(1.0, 1.0, 1.0, 1.0);\n"
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
        "in vec4 out_diffuseLight;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "FragColor = out_diffuseLight;\n"
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
    ldUniform = glGetUniformLocation(shaderProgramObject, "uLd");
    kdUniform = glGetUniformLocation(shaderProgramObject, "uKd");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "uLightPosition");
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "uLKeyPressed");

    // Cube
    {
        const GLfloat cube_position[] = {
            // front
            1.0f,  1.0f,  1.0f, // top-right of front
            -1.0f,  1.0f,  1.0f, // top-left of front
            -1.0f, -1.0f,  1.0f, // bottom-left of front
            1.0f, -1.0f,  1.0f, // bottom-right of front

            // right
            1.0f,  1.0f, -1.0f, // top-right of right
            1.0f,  1.0f,  1.0f, // top-left of right
            1.0f, -1.0f,  1.0f, // bottom-left of right
            1.0f, -1.0f, -1.0f, // bottom-right of right

            // back
            1.0f,  1.0f, -1.0f, // top-right of back
            -1.0f,  1.0f, -1.0f, // top-left of back
            -1.0f, -1.0f, -1.0f, // bottom-left of back
            1.0f, -1.0f, -1.0f, // bottom-right of back

            // left
            -1.0f,  1.0f,  1.0f, // top-right of left
            -1.0f,  1.0f, -1.0f, // top-left of left
            -1.0f, -1.0f, -1.0f, // bottom-left of left
            -1.0f, -1.0f,  1.0f, // bottom-right of left

            // top
            1.0f,  1.0f, -1.0f, // top-right of top
            -1.0f,  1.0f, -1.0f, // top-left of top
            -1.0f,  1.0f,  1.0f, // bottom-left of top
            1.0f,  1.0f,  1.0f, // bottom-right of top

            // bottom
            1.0f, -1.0f,  1.0f, // top-right of bottom
            -1.0f, -1.0f,  1.0f, // top-left of bottom
            -1.0f, -1.0f, -1.0f, // bottom-left of bottom
            1.0f, -1.0f, -1.0f, // bottom-right of bottom
        };
        const GLfloat cube_normal[] = {
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,

            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f
        };

        glGenVertexArrays(1, &vao_cube);
        glBindVertexArray(vao_cube);
        {
            // Position VBO
            {
                glGenBuffers(1, &vbo_position_cube);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(0);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            // Normal VBO
            {
                glGenBuffers(1, &vbo_normal_cube);
                glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_cube);
                {
                    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normal), cube_normal, GL_STATIC_DRAW);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                    glEnableVertexAttribArray(1);
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        glBindVertexArray(0); // Unbind the VAO
    }

    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    angle_cube = 0.0f;
    
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
        vmath::mat4 translationMatrix = vmath::mat4::identity();
        vmath::mat4 rotationMatrix = vmath::mat4::identity();
        vmath::mat4 scaleMatrix = vmath::mat4::identity();
        {
            translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
            
            rotationMatrix = vmath::rotate(angle_cube, 1.0f, 0.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angle_cube, 0.0f, 1.0f, 0.0f);
            rotationMatrix = rotationMatrix * vmath::rotate(angle_cube, 0.0f, 0.0f, 1.0f);

            scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);

            // Modeview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelViewMatrix = translationMatrix * rotationMatrix * scaleMatrix;

            // Pass the model view projection matrix to the shader
            glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, (const float*)modelViewMatrix);
            glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
            glUniform3f(ldUniform, lightDiffuse[0], lightDiffuse[1], lightDiffuse[2]);
            glUniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
            glUniform4fv(lightPositionUniform, 1, lightPosition);
            glUniform1i(lKeyPressedUniform, bLight ? 1 : 0);

            // Bind the VAO
            glBindVertexArray(vao_cube);
            {
                // Draw the Cube
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // front face
                glDrawArrays(GL_TRIANGLE_FAN, 4, 4); // right face
                glDrawArrays(GL_TRIANGLE_FAN, 8, 4); // back face
                glDrawArrays(GL_TRIANGLE_FAN, 12, 4); // left face
                glDrawArrays(GL_TRIANGLE_FAN, 16, 4); // top face
                glDrawArrays(GL_TRIANGLE_FAN, 20, 4); // bottom face
            }
            glBindVertexArray(0);
        }
    }
    glUseProgram(0);
}

-(void)myupdate
{
    if(bAnimation)
        angle_cube -= 10.05f;
}

-(void)uninitialize
{
    if(vbo_normal_cube)
    {
        glDeleteBuffers(1, &vbo_normal_cube);
        vbo_normal_cube = 0;
    }

    if(vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    if(vao_cube)
    {
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

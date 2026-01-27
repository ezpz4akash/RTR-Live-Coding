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

#define NO_OF_CIRCLE_POINTS 360

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
    
    // Thin blue lines VAO/VBO
    GLuint vao_thinBlueLine;
    GLuint vbo_position_thinBlueLines;
    int thinBlueLineCount;
    
    // Thick blue lines VAO/VBO
    GLuint vao_thickBlueLine;
    GLuint vbo_position_thickBlueLines;
    int thickBlueLineCount;
    
    // Axis lines VAO/VBO
    GLuint vao_AxisLines;
    GLuint vbo_position_AxisLines;
    GLuint vbo_color_AxisLines;
    
    // Triangle VAO/VBO
    GLuint vao_triangle;
    GLuint vbo_position_triangle;
    
    // Square VAO/VBO
    GLuint vao_square;
    GLuint vbo_position_square;
    
    // Circle VAO/VBO
    GLuint vao_circle;
    GLuint vbo_position_circle;
    
    GLint mvpMatrixUniform;
    GLint colorUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    
    // Toggle flags
    BOOL drawGraph;
    BOOL drawTriangle;
    BOOL drawSquare;
    BOOL drawCircle;
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
    // Toggle graph
    drawGraph = !drawGraph;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Cycle through shapes: none -> triangle -> square -> circle -> all -> none
    static int shapeChoice = 0;
    shapeChoice = (shapeChoice + 1) % 5;
    
    drawTriangle = NO;
    drawSquare = NO;
    drawCircle = NO;
    
    if(shapeChoice == 1) drawTriangle = YES;
    else if(shapeChoice == 2) drawSquare = YES;
    else if(shapeChoice == 3) drawCircle = YES;
    else if(shapeChoice == 4) { drawTriangle = YES; drawSquare = YES; drawCircle = YES; }
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
    
    // Initialize toggle flags
    drawGraph = YES;
    drawTriangle = NO;
    drawSquare = NO;
    drawCircle = NO;
    
    // Vertex Shader
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec4 aColor;\n"
        "out vec4 out_color;\n"
        "uniform mat4 uMVPMatrix;\n"
        "uniform vec4 uColor;\n"
        "uniform int uUseUniformColor;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = uMVPMatrix * aPosition;\n"
        "   if(uUseUniformColor == 1)\n"
        "       out_color = uColor;\n"
        "   else\n"
        "       out_color = aColor;\n"
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
        "precision highp float;\n"
        "in vec4 out_color;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   FragColor = out_color;\n"
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
    glBindAttribLocation(shaderProgramObject, 1, "aColor");
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

    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    colorUniform = glGetUniformLocation(shaderProgramObject, "uColor");
    GLint useUniformColorUniform = glGetUniformLocation(shaderProgramObject, "uUseUniformColor");

    // ============== THIN BLUE LINES ==============
    {
        GLfloat thinBlueLinePosition[32 * 3 * 2 * 2];
        GLint lineCounter = 0;
        GLfloat spacing = (1.0f / 20.0f);
        GLint iVertex = 0;
        
        // Horizontal thin lines
        for(GLfloat y = -1.0f; y < 1.0f + spacing; y = y + spacing){
            if(lineCounter % 5 != 0){
                thinBlueLinePosition[iVertex]       = -1.0f;
                thinBlueLinePosition[iVertex + 1]   = y;
                thinBlueLinePosition[iVertex + 2]   = 0.0f;
                thinBlueLinePosition[iVertex + 3]   = 1.0f;
                thinBlueLinePosition[iVertex + 4]   = y;
                thinBlueLinePosition[iVertex + 5]   = 0.0f;
                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }
        
        // Vertical thin lines
        lineCounter = 0;
        for(GLfloat x = -1.0f; x < 1.0f + spacing; x = x + spacing){
            if(lineCounter % 5 != 0){
                thinBlueLinePosition[iVertex]       = x;
                thinBlueLinePosition[iVertex + 1]   = -1.0f;
                thinBlueLinePosition[iVertex + 2]   = 0.0f;
                thinBlueLinePosition[iVertex + 3]   = x;
                thinBlueLinePosition[iVertex + 4]   = 1.0f;
                thinBlueLinePosition[iVertex + 5]   = 0.0f;
                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }
        
        thinBlueLineCount = iVertex / 3;
        
        glGenVertexArrays(1, &vao_thinBlueLine);
        glBindVertexArray(vao_thinBlueLine);
        {
            glGenBuffers(1, &vbo_position_thinBlueLines);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_thinBlueLines);
            glBufferData(GL_ARRAY_BUFFER, sizeof(thinBlueLinePosition), thinBlueLinePosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    
    // ============== THICK BLUE LINES ==============
    {
        GLfloat thickBlueLinePosition[9 * 3 * 2 * 2];
        GLint lineCounter = 0;
        GLfloat spacing = (1.0f / 20.0f);
        GLint iVertex = 0;
        
        // Horizontal thick lines (every 5th)
        for(GLfloat y = -1.0f; y < 1.0f + spacing; y = y + spacing){
            if(lineCounter % 5 == 0){
                thickBlueLinePosition[iVertex]       = -1.0f;
                thickBlueLinePosition[iVertex + 1]   = y;
                thickBlueLinePosition[iVertex + 2]   = 0.0f;
                thickBlueLinePosition[iVertex + 3]   = 1.0f;
                thickBlueLinePosition[iVertex + 4]   = y;
                thickBlueLinePosition[iVertex + 5]   = 0.0f;
                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }
        
        // Vertical thick lines (every 5th)
        lineCounter = 0;
        for(GLfloat x = -1.0f; x < 1.0f + spacing; x = x + spacing){
            if(lineCounter % 5 == 0){
                thickBlueLinePosition[iVertex]       = x;
                thickBlueLinePosition[iVertex + 1]   = -1.0f;
                thickBlueLinePosition[iVertex + 2]   = 0.0f;
                thickBlueLinePosition[iVertex + 3]   = x;
                thickBlueLinePosition[iVertex + 4]   = 1.0f;
                thickBlueLinePosition[iVertex + 5]   = 0.0f;
                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }
        
        thickBlueLineCount = iVertex / 3;
        
        glGenVertexArrays(1, &vao_thickBlueLine);
        glBindVertexArray(vao_thickBlueLine);
        {
            glGenBuffers(1, &vbo_position_thickBlueLines);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_thickBlueLines);
            glBufferData(GL_ARRAY_BUFFER, sizeof(thickBlueLinePosition), thickBlueLinePosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    
    // ============== AXIS LINES ==============
    {
        GLfloat axisPosition[] = {
            -1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };
        
        GLfloat axisColor[] = {
            1.0f, 0.0f, 0.0f, 1.0f,  // Red for X axis start
            1.0f, 0.0f, 0.0f, 1.0f,  // Red for X axis end
            0.0f, 1.0f, 0.0f, 1.0f,  // Green for Y axis start
            0.0f, 1.0f, 0.0f, 1.0f   // Green for Y axis end
        };
        
        glGenVertexArrays(1, &vao_AxisLines);
        glBindVertexArray(vao_AxisLines);
        {
            glGenBuffers(1, &vbo_position_AxisLines);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_AxisLines);
            glBufferData(GL_ARRAY_BUFFER, sizeof(axisPosition), axisPosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glGenBuffers(1, &vbo_color_AxisLines);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_color_AxisLines);
            glBufferData(GL_ARRAY_BUFFER, sizeof(axisColor), axisColor, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    
    // ============== TRIANGLE ==============
    {
        GLfloat trianglePosition[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };
        
        glGenVertexArrays(1, &vao_triangle);
        glBindVertexArray(vao_triangle);
        {
            glGenBuffers(1, &vbo_position_triangle);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_triangle);
            glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePosition), trianglePosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    
    // ============== SQUARE ==============
    {
        GLfloat squarePosition[] = {
            -0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f
        };
        
        glGenVertexArrays(1, &vao_square);
        glBindVertexArray(vao_square);
        {
            glGenBuffers(1, &vbo_position_square);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_square);
            glBufferData(GL_ARRAY_BUFFER, sizeof(squarePosition), squarePosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    
    // ============== CIRCLE ==============
    {
        #define NO_OF_CIRCLE_POINTS 360
        GLfloat circlePosition[NO_OF_CIRCLE_POINTS * 3];
        GLfloat radius = 0.5f;
        GLint i = 0;
        
        for(GLfloat angle = 0.0f; angle < 360.0f; angle = angle + 1.0f, i = i + 3){
            circlePosition[i]     = radius * cosf(angle * M_PI / 180.0f);
            circlePosition[i + 1] = radius * sinf(angle * M_PI / 180.0f);
            circlePosition[i + 2] = 0.0f;
        }
        
        glGenVertexArrays(1, &vao_circle);
        glBindVertexArray(vao_circle);
        {
            glGenBuffers(1, &vbo_position_circle);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position_circle);
            glBufferData(GL_ARRAY_BUFFER, sizeof(circlePosition), circlePosition, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }

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
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, width/height, 0.1f, 100.0f);
}

-(void)display
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);
    {
        // Transformations
        vmath::mat4 modelViewMatrix = vmath::mat4::identity();
        vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
        vmath::mat4 translationMatrix = vmath::mat4::identity();
        
        translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
        modelViewMatrix = translationMatrix;
        modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);
        
        GLint useUniformColorUniform = glGetUniformLocation(shaderProgramObject, "uUseUniformColor");
        
        if(drawGraph){
            // Draw thin blue lines
            glUniform1i(useUniformColorUniform, 1);
            glUniform4f(colorUniform, 0.0f, 0.0f, 1.0f, 1.0f);
            glBindVertexArray(vao_thinBlueLine);
            {
                glLineWidth(1.0f);
                glDrawArrays(GL_LINES, 0, thinBlueLineCount);
            }
            glBindVertexArray(0);
            
            // Draw thick blue lines
            glUniform4f(colorUniform, 0.0f, 0.0f, 1.0f, 1.0f);
            glBindVertexArray(vao_thickBlueLine);
            {
                glLineWidth(3.0f);
                glDrawArrays(GL_LINES, 0, thickBlueLineCount);
            }
            glBindVertexArray(0);
            
            // Draw axis lines (use vertex color)
            glUniform1i(useUniformColorUniform, 0);
            glBindVertexArray(vao_AxisLines);
            {
                glLineWidth(3.0f);
                glDrawArrays(GL_LINES, 0, 4);
            }
            glBindVertexArray(0);
        }
        
        // Draw shapes with yellow color
        glUniform1i(useUniformColorUniform, 1);
        glUniform4f(colorUniform, 1.0f, 1.0f, 0.0f, 1.0f);
        
        if(drawTriangle){
            glBindVertexArray(vao_triangle);
            {
                glLineWidth(2.0f);
                glDrawArrays(GL_LINE_LOOP, 0, 3);
            }
            glBindVertexArray(0);
        }
        
        if(drawSquare){
            glBindVertexArray(vao_square);
            {
                glLineWidth(2.0f);
                glDrawArrays(GL_LINE_LOOP, 0, 4);
            }
            glBindVertexArray(0);
        }
        
        if(drawCircle){
            glBindVertexArray(vao_circle);
            {
                glLineWidth(2.0f);
                glDrawArrays(GL_LINE_LOOP, 0, NO_OF_CIRCLE_POINTS);
            }
            glBindVertexArray(0);
        }
        
        glLineWidth(1.0f);
    }
    glUseProgram(0);
}

-(void)myupdate
{
    
}

-(void)uninitialize
{
    // Thin blue lines cleanup
    if(vbo_position_thinBlueLines){
        glDeleteBuffers(1, &vbo_position_thinBlueLines);
        vbo_position_thinBlueLines = 0;
    }
    if(vao_thinBlueLine){
        glDeleteVertexArrays(1, &vao_thinBlueLine);
        vao_thinBlueLine = 0;
    }
    
    // Thick blue lines cleanup
    if(vbo_position_thickBlueLines){
        glDeleteBuffers(1, &vbo_position_thickBlueLines);
        vbo_position_thickBlueLines = 0;
    }
    if(vao_thickBlueLine){
        glDeleteVertexArrays(1, &vao_thickBlueLine);
        vao_thickBlueLine = 0;
    }
    
    // Axis lines cleanup
    if(vbo_color_AxisLines){
        glDeleteBuffers(1, &vbo_color_AxisLines);
        vbo_color_AxisLines = 0;
    }
    if(vbo_position_AxisLines){
        glDeleteBuffers(1, &vbo_position_AxisLines);
        vbo_position_AxisLines = 0;
    }
    if(vao_AxisLines){
        glDeleteVertexArrays(1, &vao_AxisLines);
        vao_AxisLines = 0;
    }
    
    // Triangle cleanup
    if(vbo_position_triangle){
        glDeleteBuffers(1, &vbo_position_triangle);
        vbo_position_triangle = 0;
    }
    if(vao_triangle){
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0;
    }
    
    // Square cleanup
    if(vbo_position_square){
        glDeleteBuffers(1, &vbo_position_square);
        vbo_position_square = 0;
    }
    if(vao_square){
        glDeleteVertexArrays(1, &vao_square);
        vao_square = 0;
    }
    
    // Circle cleanup
    if(vbo_position_circle){
        glDeleteBuffers(1, &vbo_position_circle);
        vbo_position_circle = 0;
    }
    if(vao_circle){
        glDeleteVertexArrays(1, &vao_circle);
        vao_circle = 0;
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
    
    if(depthRenderbuffer){
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
    if(colorRenderbuffer){
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    if(framebuffer){
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
    
    if([EAGLContext currentContext] == eaglContext){
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

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
    GLuint vao;
    GLuint vbo_position;
    GLuint vbo_texcoord;
    GLint mvpMatrixUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    
    // Texture
    GLuint smileyTexture;
    GLint textureSamplerUniform;
    GLint keyPressedUniform;
    
    // Single tap count
    int singleTapCount;
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
    // code
    singleTapCount++;
    if(singleTapCount > 3)
        singleTapCount = 0;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    singleTapCount = 0;
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
        "in vec2 aTexCoord;\n"
        "out vec2 out_texCoord;\n"
        "uniform mat4 uMVPMatrix;\n"
        "void main(void)\n"
        "{\n"
        "gl_Position = uMVPMatrix * aPosition;\n"
        "out_texCoord = aTexCoord;\n"
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
        "out vec4 FragColor;\n"
        "uniform int uKeyPress;\n"
        "in vec2 out_texCoord;\n"
        "uniform sampler2D uTextureSampler;\n"
        "void main(void)\n"
        "{\n"
        "if(uKeyPress != -1)\n"
        "FragColor = texture(uTextureSampler, out_texCoord);\n"
        "else\n"
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
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
    glBindAttribLocation(shaderProgramObject, 1, "aTexCoord");

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
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject, "uKeyPress");

    // Provide vertex position, color, texture coordinates, normals, etc. to the shader program object

    const GLfloat rectangle_position[] = {
        1.0f,  1.0f,  0.0f,  // top-right
        -1.0f,  1.0f,  0.0f, // top-left
        -1.0f, -1.0f,  0.0f, // bottom-left
        1.0f, -1.0f,  0.0f  // bottom-right
    };

    const GLfloat rectangle_texcoord[] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    // Create Vertex Array Object (VAO) for array of vertex attributes
    // vao is an array object that stores the state of vertex attributes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    {
        // Position VBO
        {
            glGenBuffers(1, &vbo_position);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
            {
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, NULL, GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(0);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // TexCoord VBO
        {
            glGenBuffers(1, &vbo_texcoord);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
            {
                glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_texcoord), rectangle_texcoord, GL_DYNAMIC_DRAW);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
                glEnableVertexAttribArray(1);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    glBindVertexArray(0); // Unbind the VAO

    perspectiveProjectionMatrix = vmath::mat4::identity();
    
    // Load textures
    smileyTexture = [self loadGLTexture:@"Smiley" :@"bmp"];
    
    singleTapCount = 0;
    
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

- (GLuint) loadGLTexture:(NSString *)textureFileName :(NSString *)extension
{
    NSString *imageFileNameWithPath = [[NSBundle mainBundle] pathForResource:textureFileName ofType:extension];
    UIImage *bmpImage = [[UIImage alloc] initWithContentsOfFile:imageFileNameWithPath];
    if (!bmpImage) {
        printf("Failed to load image: %s.%s\n", [textureFileName UTF8String], [extension UTF8String]);
        return 0;
    }
    
    CGImageRef cgImage = bmpImage.CGImage;
    int width = (int)CGImageGetWidth(cgImage);
    int height = (int)CGImageGetHeight(cgImage);
    CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
    void *pixels = (void *)CFDataGetBytePtr(imageData);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    CFRelease(imageData);
    [bmpImage release];
    
    return texture;
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
        {
            vmath::mat4 translationMatrix = vmath::mat4::identity();

            // Prepare transformation matrices
            translationMatrix = vmath::translate(0.0f, 0.0f, -4.0f);

            // Modelview matrix is the combination of all transformations by multiplying all the necessary transformation matrices
            modelViewMatrix = translationMatrix;

            // Prepare final model view projection matrix as a combination of perspective projection matrix and model view matrix and send it to the shader
            modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

            // Pass the model view projection matrix to the shader
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

            // Bind the texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, smileyTexture);
            glUniform1i(textureSamplerUniform, 0); // Set the texture sampler to use
        }

        // Update position VBO
        const GLfloat rectangle_position[] = {
            1.0f,  1.0f,  0.0f,  // top-right
            -1.0f,  1.0f,  0.0f, // top-left
            -1.0f, -1.0f,  0.0f, // bottom-left
            1.0f, -1.0f,  0.0f  // bottom-right
        };
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_position), rectangle_position, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Bind the VAO
        glBindVertexArray(vao);
        {
            const GLfloat rectangle_texcoord[] = {
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f
            };
            
            if(singleTapCount == 0){
                const GLfloat texcoord0[] = {
                    0.5f, 0.5f,
                    0.0f, 0.5f,
                    0.0f, 0.0f,
                    0.5f, 0.0f
                };
                glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord0), texcoord0, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glUniform1i(keyPressedUniform, singleTapCount);
            }
            else if(singleTapCount == 1){
                glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_texcoord), rectangle_texcoord, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glUniform1i(keyPressedUniform, singleTapCount);
            }
            else if(singleTapCount == 2){
                const GLfloat texcoord2[] = {
                    2.0f, 2.0f,
                    0.0f, 2.0f,
                    0.0f, 0.0f,
                    2.0f, 0.0f
                };
                glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord2), texcoord2, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glUniform1i(keyPressedUniform, singleTapCount);
            }
            else if(singleTapCount == 3){
                const GLfloat texcoord3[] = {
                    0.5f, 0.5f,
                    0.5f, 0.5f,
                    0.5f, 0.5f,
                    0.5f, 0.5f
                };
                glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
                glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord3), texcoord3, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glUniform1i(keyPressedUniform, singleTapCount);
            }

            // Draw the rectangle
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

-(void)myupdate
{
    
}

-(void)uninitialize
{
    if(smileyTexture)
    {
        glDeleteTextures(1, &smileyTexture);
        smileyTexture = 0;
    }

    if(vbo_texcoord)
    {
        glDeleteBuffers(1, &vbo_texcoord);
        vbo_texcoord = 0;
    }

    if(vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0;
    }

    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
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

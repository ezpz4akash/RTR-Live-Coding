//
//  GLESView.mm
//  Window2 - FBO with Sphere (Three Moving Lights) on Rotating Cube
//
//  Created by user947254 on 1/18/26.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/CADisplayLink.h>

#import "GLESView.h"
#include "vmath.h"
#import "Sphere.h"

#define FBO_WIDTH 512
#define FBO_HEIGHT 512
#define NO_OF_LIGHTS 1
#define NO_OF_MATERIALS 24

// 24 Materials Structure
struct Material {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
};

static struct Material materials[24] = {
    // 1st sphere on 1st column, emerald
    {{0.0215f, 0.1745f, 0.0215f, 1.0f}, {0.07568f, 0.61424f, 0.07568f, 1.0f}, {0.633f, 0.727811f, 0.633f, 1.0f}, 0.6f * 128},
    // 2nd sphere on 1st column, jade
    {{0.135f, 0.2225f, 0.1575f, 1.0f}, {0.54f, 0.89f, 0.63f, 1.0f}, {0.316228f, 0.316228f, 0.316228f, 1.0f}, 0.1f * 128},
    // 3rd sphere on 1st column, obsidian
    {{0.05375f, 0.05f, 0.06625f, 1.0f}, {0.18275f, 0.17f, 0.22525f, 1.0f}, {0.332741f, 0.328634f, 0.346435f, 1.0f}, 0.3f * 128},
    // 4th sphere on 1st column, pearl
    {{0.25f, 0.20725f, 0.20725f, 1.0f}, {1.0f, 0.829f, 0.829f, 1.0f}, {0.296648f, 0.296648f, 0.296648f, 1.0f}, 0.088f * 128},
    // 5th sphere on 1st column, ruby
    {{0.1745f, 0.01175f, 0.01175f, 1.0f}, {0.61424f, 0.04136f, 0.04136f, 1.0f}, {0.727811f, 0.626959f, 0.626959f, 1.0f}, 0.6f * 128},
    // 6th sphere on 1st column, turquoise
    {{0.1f, 0.18725f, 0.1745f, 1.0f}, {0.396f, 0.74151f, 0.69102f, 1.0f}, {0.297254f, 0.30829f, 0.306678f, 1.0f}, 0.1f * 128},
    // 1st sphere on 2nd column, brass
    {{0.329412f, 0.223529f, 0.027451f, 1.0f}, {0.780392f, 0.568627f, 0.113725f, 1.0f}, {0.992157f, 0.941176f, 0.807843f, 1.0f}, 0.21794872f * 128},
    // 2nd sphere on 2nd column, bronze
    {{0.2125f, 0.1275f, 0.054f, 1.0f}, {0.714f, 0.4284f, 0.18144f, 1.0f}, {0.393548f, 0.271906f, 0.166721f, 1.0f}, 0.2f * 128},
    // 3rd sphere on 2nd column, chrome
    {{0.25f, 0.25f, 0.25f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, {0.774597f, 0.774597f, 0.774597f, 1.0f}, 0.6f * 128},
    // 4th sphere on 2nd column, copper
    {{0.19125f, 0.0735f, 0.0225f, 1.0f}, {0.7038f, 0.27048f, 0.0828f, 1.0f}, {0.256777f, 0.137622f, 0.086014f, 1.0f}, 0.1f * 128},
    // 5th sphere on 2nd column, gold
    {{0.24725f, 0.1995f, 0.0745f, 1.0f}, {0.75164f, 0.60648f, 0.22648f, 1.0f}, {0.628281f, 0.555802f, 0.366065f, 1.0f}, 0.4f * 128},
    // 6th sphere on 2nd column, silver
    {{0.19225f, 0.19225f, 0.19225f, 1.0f}, {0.50754f, 0.50754f, 0.50754f, 1.0f}, {0.508273f, 0.508273f, 0.508273f, 1.0f}, 0.4f * 128},
    // 1st sphere on 3rd column, black
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.01f, 0.01f, 0.01f, 1.0f}, {0.50f, 0.50f, 0.50f, 1.0f}, 0.25f * 128},
    // 2nd sphere on 3rd column, cyan
    {{0.0f, 0.1f, 0.06f, 1.0f}, {0.0f, 0.50980392f, 0.50980392f, 1.0f}, {0.50980392f, 0.50980392f, 0.50980392f, 1.0f}, 0.25f * 128},
    // 3rd sphere on 3rd column, green
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.1f, 0.35f, 0.1f, 1.0f}, {0.45f, 0.55f, 0.45f, 1.0f}, 0.25f * 128},
    // 4th sphere on 3rd column, red
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.0f, 0.0f, 1.0f}, {0.7f, 0.6f, 0.6f, 1.0f}, 0.25f * 128},
    // 5th sphere on 3rd column, white
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.55f, 0.55f, 0.55f, 1.0f}, {0.70f, 0.70f, 0.70f, 1.0f}, 0.25f * 128},
    // 6th sphere on 3rd column, yellow
    {{0.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f, 0.0f, 1.0f}, {0.60f, 0.60f, 0.50f, 1.0f}, 0.25f * 128},
    // 1st sphere on 4th column, black
    {{0.02f, 0.02f, 0.02f, 1.0f}, {0.01f, 0.01f, 0.01f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, 0.078125f * 128},
    // 2nd sphere on 4th column, cyan
    {{0.0f, 0.05f, 0.05f, 1.0f}, {0.4f, 0.5f, 0.5f, 1.0f}, {0.04f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128},
    // 3rd sphere on 4th column, green
    {{0.0f, 0.05f, 0.0f, 1.0f}, {0.4f, 0.5f, 0.4f, 1.0f}, {0.04f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128},
    // 4th sphere on 4th column, red
    {{0.05f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.4f, 0.4f, 1.0f}, {0.7f, 0.04f, 0.04f, 1.0f}, 0.078125f * 128},
    // 5th sphere on 4th column, white
    {{0.05f, 0.05f, 0.05f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}, {0.7f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128},
    // 6th sphere on 4th column, yellow
    {{0.05f, 0.05f, 0.0f, 1.0f}, {0.5f, 0.5f, 0.4f, 1.0f}, {0.7f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128}
};

@implementation GLESView
{
    EAGLContext *eaglContext;

    GLuint framebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;

    CADisplayLink *displayLink;
    CAFrameRateRange frameRateRange;
    BOOL isDisplayLink;
    
    // ============== CUBE SHADER VARIABLES ==============
    GLuint shaderProgramObject_cube;
    GLuint vao_cube;
    GLuint vbo_position_cube;
    GLuint vbo_texcoord_cube;
    GLint mvpMatrixUniform_cube;
    GLint textureSamplerUniform;
    vmath::mat4 perspectiveProjectionMatrix_cube;
    float angleCube;
    
    // ============== SPHERE SHADER VARIABLES (PV) ==============
    GLuint pv_shaderProgramObject_sphere;
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;
    
    GLint pv_modelMatrixUniform_sphere;
    GLint pv_viewMatrixUniform_sphere;
    GLint pv_projectionMatrixUniform_sphere;
    GLint pv_laUniform_sphere[NO_OF_LIGHTS];
    GLint pv_ldUniform_sphere[NO_OF_LIGHTS];
    GLint pv_lsUniform_sphere[NO_OF_LIGHTS];
    GLint pv_lightPositionUniform_sphere[NO_OF_LIGHTS];
    GLint pv_kaUniform_sphere;
    GLint pv_kdUniform_sphere;
    GLint pv_ksUniform_sphere;
    GLint pv_materialShininessUniform_sphere;
    GLint pv_lKeyPressedUniform_sphere;
    
    // ============== SPHERE SHADER VARIABLES (PF) ==============
    GLuint pf_shaderProgramObject_sphere;
    
    GLint pf_modelMatrixUniform_sphere;
    GLint pf_viewMatrixUniform_sphere;
    GLint pf_projectionMatrixUniform_sphere;
    GLint pf_laUniform_sphere[NO_OF_LIGHTS];
    GLint pf_ldUniform_sphere[NO_OF_LIGHTS];
    GLint pf_lsUniform_sphere[NO_OF_LIGHTS];
    GLint pf_lightPositionUniform_sphere[NO_OF_LIGHTS];
    GLint pf_kaUniform_sphere;
    GLint pf_kdUniform_sphere;
    GLint pf_ksUniform_sphere;
    GLint pf_materialShininessUniform_sphere;
    GLint pf_lKeyPressedUniform_sphere;
    
    vmath::mat4 perspectiveProjectionMatrix_sphere;
    
    // ============== LIGHT PROPERTIES ==============
    float lightAmbient[NO_OF_LIGHTS][4];
    float lightDiffuse[NO_OF_LIGHTS][4];
    float lightSpecular[NO_OF_LIGHTS][4];
    float lightPosition[NO_OF_LIGHTS][4];
    float lightAngle[NO_OF_LIGHTS];
    
    // Material properties
    float materialAmbient[4];
    float materialDiffuse[4];
    float materialSpecular[4];
    float materialShininess;
    
    BOOL bLight;
    BOOL bPerVertexPerFragmentToggle;
    
    // ============== FBO VARIABLES ==============
    GLuint fbo;
    GLuint rbo;
    GLuint fboTexture;
    BOOL fboResult;
    
    int winWidth;
    int winHeight;
    
    // Material choice (0-23)
    int materialChoice;
    
    // Light rotation axis: 1=X, 2=Y, 3=Z
    int keyPressed;
    float lightAngleSingle;
    
    // Sphere object
    Sphere *sphere;
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
        
        // Initialize single white light
        lightAmbient[0][0] = 0.0f; lightAmbient[0][1] = 0.0f; lightAmbient[0][2] = 0.0f; lightAmbient[0][3] = 1.0f;
        lightDiffuse[0][0] = 1.0f; lightDiffuse[0][1] = 1.0f; lightDiffuse[0][2] = 1.0f; lightDiffuse[0][3] = 1.0f;
        lightSpecular[0][0] = 1.0f; lightSpecular[0][1] = 1.0f; lightSpecular[0][2] = 1.0f; lightSpecular[0][3] = 1.0f;
        lightPosition[0][0] = 0.0f; lightPosition[0][1] = 0.0f; lightPosition[0][2] = 0.0f; lightPosition[0][3] = 1.0f;
        lightAngle[0] = 0.0f;
        lightAngleSingle = 0.0f;
        keyPressed = 1; // Default: X axis rotation
        
        // Material properties - Start with first material (emerald)
        materialChoice = 0;
        materialAmbient[0] = materials[materialChoice].ambient[0];
        materialAmbient[1] = materials[materialChoice].ambient[1];
        materialAmbient[2] = materials[materialChoice].ambient[2];
        materialAmbient[3] = materials[materialChoice].ambient[3];
        materialDiffuse[0] = materials[materialChoice].diffuse[0];
        materialDiffuse[1] = materials[materialChoice].diffuse[1];
        materialDiffuse[2] = materials[materialChoice].diffuse[2];
        materialDiffuse[3] = materials[materialChoice].diffuse[3];
        materialSpecular[0] = materials[materialChoice].specular[0];
        materialSpecular[1] = materials[materialChoice].specular[1];
        materialSpecular[2] = materials[materialChoice].specular[2];
        materialSpecular[3] = materials[materialChoice].specular[3];
        materialShininess = materials[materialChoice].shininess;
        
        bLight = YES;
        bPerVertexPerFragmentToggle = NO;
        angleCube = 0.0f;
        winWidth = width;
        winHeight = height;
        
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
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)[self layer]];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

    GLint width;
    GLint height;

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Delete old depth renderbuffer if it exists
    if(depthRenderbuffer){
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
    
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

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
    // Cycle through materials (0-23)
    materialChoice = (materialChoice + 1) % NO_OF_MATERIALS;
    
    // Update material properties from materials array
    materialAmbient[0] = materials[materialChoice].ambient[0];
    materialAmbient[1] = materials[materialChoice].ambient[1];
    materialAmbient[2] = materials[materialChoice].ambient[2];
    materialAmbient[3] = materials[materialChoice].ambient[3];
    
    materialDiffuse[0] = materials[materialChoice].diffuse[0];
    materialDiffuse[1] = materials[materialChoice].diffuse[1];
    materialDiffuse[2] = materials[materialChoice].diffuse[2];
    materialDiffuse[3] = materials[materialChoice].diffuse[3];
    
    materialSpecular[0] = materials[materialChoice].specular[0];
    materialSpecular[1] = materials[materialChoice].specular[1];
    materialSpecular[2] = materials[materialChoice].specular[2];
    materialSpecular[3] = materials[materialChoice].specular[3];
    
    materialShininess = materials[materialChoice].shininess;
    
    printf("Material changed to: %d\n", materialChoice);
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gestureRecognizer {
    // Cycle through X, Y, Z axis rotation (1 -> 2 -> 3 -> 1)
    keyPressed = (keyPressed % 3) + 1;
    lightAngleSingle = 0.0f; // Reset angle when switching axis
    
    if(keyPressed == 1)
        printf("Light rotating on X axis\n");
    else if(keyPressed == 2)
        printf("Light rotating on Y axis\n");
    else
        printf("Light rotating on Z axis\n");
}

- (void)onSwipe:(UITapGestureRecognizer *)gestureRecognizer {
    // code
    [self uninitialize];
    [self release];
    exit(0);
}

- (void)onLongPress:(UITapGestureRecognizer *)gestureRecognizer {
    // Toggle between Per-Vertex and Per-Fragment lighting
    if(gestureRecognizer.state == UIGestureRecognizerStateBegan){
        bPerVertexPerFragmentToggle = !bPerVertexPerFragmentToggle;
        if(bPerVertexPerFragmentToggle == NO)
            printf("Per-Vertex Lighting\n");
        else
            printf("Per-Fragment Lighting\n");
    }
}

-(int)initialize
{
    [self printGLESInfo];
    
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // ============== CUBE SHADER ==============
    GLuint vertexShaderObject_cube = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode_cube =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 out_texCoord;\n"
        "uniform mat4 uMVPMatrix;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = uMVPMatrix * aPosition;\n"
        "   out_texCoord = aTexCoord;\n"
        "}\n";
    glShaderSource(vertexShaderObject_cube, 1, &vertexShaderSourceCode_cube, NULL);
    glCompileShader(vertexShaderObject_cube);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(vertexShaderObject_cube, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(vertexShaderObject_cube, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(vertexShaderObject_cube, iInfoLogLength, NULL, szInfoLog);
                printf("Cube Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -1;
    }

    GLuint fragmentShaderObject_cube = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode_cube =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 FragColor;\n"
        "in vec2 out_texCoord;\n"
        "uniform sampler2D uTextureSampler;\n"
        "void main(void)\n"
        "{\n"
        "   FragColor = texture(uTextureSampler, out_texCoord);\n"
        "}\n";
    glShaderSource(fragmentShaderObject_cube, 1, &fragmentShaderSourceCode_cube, NULL);
    glCompileShader(fragmentShaderObject_cube);
    glGetShaderiv(fragmentShaderObject_cube, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(fragmentShaderObject_cube, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(fragmentShaderObject_cube, iInfoLogLength, NULL, szInfoLog);
                printf("Cube Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -2;
    }

    shaderProgramObject_cube = glCreateProgram();
    glAttachShader(shaderProgramObject_cube, vertexShaderObject_cube);
    glAttachShader(shaderProgramObject_cube, fragmentShaderObject_cube);
    glBindAttribLocation(shaderProgramObject_cube, 0, "aPosition");
    glBindAttribLocation(shaderProgramObject_cube, 1, "aTexCoord");
    glLinkProgram(shaderProgramObject_cube);
    glGetProgramiv(shaderProgramObject_cube, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(shaderProgramObject_cube, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(shaderProgramObject_cube, iInfoLogLength, NULL, szInfoLog);
                printf("Cube Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -3;
    }

    mvpMatrixUniform_cube = glGetUniformLocation(shaderProgramObject_cube, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject_cube, "uTextureSampler");

    // Cube VAO
    const GLfloat cube_position[] = {
        // front
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        // right
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        // back
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        // left
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        // top
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        // bottom
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
    };
    const GLfloat cube_texcoord[] = {
        // front
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // right
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // back
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // left
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // top
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // bottom
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);
    {
        glGenBuffers(1, &vbo_position_cube);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_texcoord_cube);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_cube);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoord), cube_texcoord, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    perspectiveProjectionMatrix_cube = vmath::mat4::identity();

    // Create FBO
    fboResult = [self createAndPrepareFBOForDrawing:FBO_WIDTH :FBO_HEIGHT];
    if(fboResult == YES){
        printf("FBO created successfully\n");
        [self initializeSphere];
    }
    else{
        printf("FBO creation failed\n");
    }
    
    return (0);
}

-(BOOL)createAndPrepareFBOForDrawing:(GLint)textureWidth :(GLint)textureHeight
{
    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    if (textureWidth > maxTexSize || textureHeight > maxTexSize) {
        printf("Requested texture size (%d x %d) exceeds max texture size (%d).\n",
                textureWidth, textureHeight, maxTexSize);
        return NO;
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is not complete: 0x%X\n", status);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        return NO;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    return YES;
}

-(int)initializeSphere
{
    // ============== PER-VERTEX SPHERE SHADER ==============
    GLuint pv_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pv_vertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "uniform mat4 uModelMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "uniform vec3 uLa;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uLs;\n"
        "uniform vec4 uLightPosition;\n"
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
        "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n"
        "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n"
        "       vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n"
        "       vec3 ambient = uLa * uKa;\n"
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n"
        "       out_phong_ads_Light = vec4(ambient + diffuse + specular, 1.0);\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       out_phong_ads_Light = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "   }\n"
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"
        "}\n";
    glShaderSource(pv_vertexShaderObject, 1, &pv_vertexShaderSourceCode, NULL);
    glCompileShader(pv_vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iStatus = 0;
    GLchar *szInfoLog = NULL;
    glGetShaderiv(pv_vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pv_vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pv_vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PV Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -1;
    }

    GLuint pv_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pv_fragmentShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 out_phong_ads_Light;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   FragColor = out_phong_ads_Light;\n"
        "}\n";
    glShaderSource(pv_fragmentShaderObject, 1, &pv_fragmentShaderSourceCode, NULL);
    glCompileShader(pv_fragmentShaderObject);
    glGetShaderiv(pv_fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pv_fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pv_fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PV Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -2;
    }

    pv_shaderProgramObject_sphere = glCreateProgram();
    glAttachShader(pv_shaderProgramObject_sphere, pv_vertexShaderObject);
    glAttachShader(pv_shaderProgramObject_sphere, pv_fragmentShaderObject);
    glBindAttribLocation(pv_shaderProgramObject_sphere, 0, "aPosition");
    glBindAttribLocation(pv_shaderProgramObject_sphere, 1, "aNormal");
    glLinkProgram(pv_shaderProgramObject_sphere);
    glGetProgramiv(pv_shaderProgramObject_sphere, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pv_shaderProgramObject_sphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pv_shaderProgramObject_sphere, iInfoLogLength, NULL, szInfoLog);
                printf("PV Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -3;
    }

    pv_modelMatrixUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uModelMatrix");
    pv_viewMatrixUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uViewMatrix");
    pv_projectionMatrixUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uProjectionMatrix");
    pv_laUniform_sphere[0] = glGetUniformLocation(pv_shaderProgramObject_sphere, "uLa");
    pv_ldUniform_sphere[0] = glGetUniformLocation(pv_shaderProgramObject_sphere, "uLd");
    pv_lsUniform_sphere[0] = glGetUniformLocation(pv_shaderProgramObject_sphere, "uLs");
    pv_lightPositionUniform_sphere[0] = glGetUniformLocation(pv_shaderProgramObject_sphere, "uLightPosition");
    pv_kaUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uKa");
    pv_kdUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uKd");
    pv_ksUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uKs");
    pv_materialShininessUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uMaterialShininess");
    pv_lKeyPressedUniform_sphere = glGetUniformLocation(pv_shaderProgramObject_sphere, "uLKeyPressed");

    // ============== PER-FRAGMENT SPHERE SHADER ==============
    GLuint pf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *pf_vertexShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 aPosition;\n"
        "in vec3 aNormal;\n"
        "out vec4 eyeCoordinates;\n"
        "out vec3 transformedNormal;\n"
        "out vec3 lightSource;\n"
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
        "}\n";
    glShaderSource(pf_vertexShaderObject, 1, &pf_vertexShaderSourceCode, NULL);
    glCompileShader(pf_vertexShaderObject);
    glGetShaderiv(pf_vertexShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pf_vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pf_vertexShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PF Vertex Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -4;
    }

    GLuint pf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *pf_fragmentShaderSourceCode =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec3 transformedNormal;\n"
        "in vec4 eyeCoordinates;\n"
        "in vec3 lightSource;\n"
        "uniform vec3 uLa;\n"
        "uniform vec3 uLd;\n"
        "uniform vec3 uLs;\n"
        "uniform vec3 uKa;\n"
        "uniform vec3 uKd;\n"
        "uniform vec3 uKs;\n"
        "uniform float uMaterialShininess;\n"
        "uniform int uLKeyPressed;\n"
        "out vec4 FragColor;\n"
        "void main(void)\n"
        "{\n"
        "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n"
        "   vec3 normalizedViewerVector = normalize(-eyeCoordinates.xyz);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       vec3 normalizedLightSource = normalize(lightSource);\n"
        "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "       vec3 ambient = uLa * uKa;\n"
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, normalizedViewerVector), 0.0), uMaterialShininess);\n"
        "       FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "   }\n"
        "}\n";
    glShaderSource(pf_fragmentShaderObject, 1, &pf_fragmentShaderSourceCode, NULL);
    glCompileShader(pf_fragmentShaderObject);
    glGetShaderiv(pf_fragmentShaderObject, GL_COMPILE_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetShaderiv(pf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetShaderInfoLog(pf_fragmentShaderObject, iInfoLogLength, NULL, szInfoLog);
                printf("PF Fragment Shader Compilation Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -5;
    }

    pf_shaderProgramObject_sphere = glCreateProgram();
    glAttachShader(pf_shaderProgramObject_sphere, pf_vertexShaderObject);
    glAttachShader(pf_shaderProgramObject_sphere, pf_fragmentShaderObject);
    glBindAttribLocation(pf_shaderProgramObject_sphere, 0, "aPosition");
    glBindAttribLocation(pf_shaderProgramObject_sphere, 1, "aNormal");
    glLinkProgram(pf_shaderProgramObject_sphere);
    glGetProgramiv(pf_shaderProgramObject_sphere, GL_LINK_STATUS, &iStatus);
    if(iStatus == GL_FALSE){
        glGetProgramiv(pf_shaderProgramObject_sphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if(iInfoLogLength > 0){
            szInfoLog = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
            if(szInfoLog != NULL){
                glGetProgramInfoLog(pf_shaderProgramObject_sphere, iInfoLogLength, NULL, szInfoLog);
                printf("PF Shader Program Linking Log: %s\n", szInfoLog);
                free(szInfoLog);
            }
        }
        return -6;
    }

    pf_modelMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uModelMatrix");
    pf_viewMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uViewMatrix");
    pf_projectionMatrixUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uProjectionMatrix");
    pf_laUniform_sphere[0] = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLa");
    pf_ldUniform_sphere[0] = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLd");
    pf_lsUniform_sphere[0] = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLs");
    pf_lightPositionUniform_sphere[0] = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLightPosition");
    pf_kaUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKa");
    pf_kdUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKd");
    pf_ksUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uKs");
    pf_materialShininessUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uMaterialShininess");
    pf_lKeyPressedUniform_sphere = glGetUniformLocation(pf_shaderProgramObject_sphere, "uLKeyPressed");

    // Initialize sphere
    sphere = [[Sphere alloc] init];
    
    float spherePositionCoords[1146];
    float sphereNormalCoords[1146];
    float sphereTexCoords[764];
    unsigned short sphereElements[2280];

    [sphere getSphereVertexData:spherePositionCoords :sphereNormalCoords :sphereTexCoords :sphereElements];

    int numSphereVertices = [sphere getNumberOfSphereVertices];
    int numSphereElements = [sphere getNumberOfSphereElements];

    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
    {
        glGenBuffers(1, &vbo_position_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), spherePositionCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_normal_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
        glBufferData(GL_ARRAY_BUFFER, numSphereVertices * 3 * sizeof(float), sphereNormalCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numSphereElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);
    }
    glBindVertexArray(0);

    [self resize_sphere:FBO_WIDTH :FBO_HEIGHT];

    return 0;
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
    
    winWidth = width;
    winHeight = height;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix_cube = vmath::perspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);
}

-(void)resize_sphere:(int)width :(int)height
{
    if(height <= 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix_sphere = vmath::perspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);
}

-(void)displaySphere
{
    if(fbo){
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    [self resize_sphere:FBO_WIDTH :FBO_HEIGHT];

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate light position based on keyPressed axis
    float radians = lightAngleSingle * M_PI / 180.0f;
    float lightPos[4];
    
    if(keyPressed == 1){
        // Rotate around X axis
        lightPos[0] = 0.0f;
        lightPos[1] = 20.0f * sinf(radians);
        lightPos[2] = 20.0f * cosf(radians);
        lightPos[3] = 1.0f;
    }
    else if(keyPressed == 2){
        // Rotate around Y axis
        lightPos[0] = 20.0f * sinf(radians);
        lightPos[1] = 0.0f;
        lightPos[2] = 20.0f * cosf(radians);
        lightPos[3] = 1.0f;
    }
    else{
        // Rotate around Z axis
        lightPos[0] = 20.0f * sinf(radians);
        lightPos[1] = 20.0f * cosf(radians);
        lightPos[2] = 0.0f;
        lightPos[3] = 1.0f;
    }

    // Transformations
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::lookat(
        vmath::vec3(0.0f, 0.0f, 2.0f),
        vmath::vec3(0.0f, 0.0f, 0.0f),
        vmath::vec3(0.0f, 1.0f, 0.0f)
    );

    if(bPerVertexPerFragmentToggle == NO){
        // Per-Vertex Lighting
        glUseProgram(pv_shaderProgramObject_sphere);

        glUniformMatrix4fv(pv_modelMatrixUniform_sphere, 1, GL_FALSE, (const float*)modelMatrix);
        glUniformMatrix4fv(pv_viewMatrixUniform_sphere, 1, GL_FALSE, (const float*)viewMatrix);
        glUniformMatrix4fv(pv_projectionMatrixUniform_sphere, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix_sphere);

        glUniform3fv(pv_laUniform_sphere[0], 1, lightAmbient[0]);
        glUniform3fv(pv_ldUniform_sphere[0], 1, lightDiffuse[0]);
        glUniform3fv(pv_lsUniform_sphere[0], 1, lightSpecular[0]);
        glUniform4fv(pv_lightPositionUniform_sphere[0], 1, lightPos);

        glUniform3fv(pv_kaUniform_sphere, 1, materialAmbient);
        glUniform3fv(pv_kdUniform_sphere, 1, materialDiffuse);
        glUniform3fv(pv_ksUniform_sphere, 1, materialSpecular);
        glUniform1f(pv_materialShininessUniform_sphere, materialShininess);
        glUniform1i(pv_lKeyPressedUniform_sphere, bLight ? 1 : 0);
    }
    else{
        // Per-Fragment Lighting
        glUseProgram(pf_shaderProgramObject_sphere);

        glUniformMatrix4fv(pf_modelMatrixUniform_sphere, 1, GL_FALSE, (const float*)modelMatrix);
        glUniformMatrix4fv(pf_viewMatrixUniform_sphere, 1, GL_FALSE, (const float*)viewMatrix);
        glUniformMatrix4fv(pf_projectionMatrixUniform_sphere, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix_sphere);

        glUniform3fv(pf_laUniform_sphere[0], 1, lightAmbient[0]);
        glUniform3fv(pf_ldUniform_sphere[0], 1, lightDiffuse[0]);
        glUniform3fv(pf_lsUniform_sphere[0], 1, lightSpecular[0]);
        glUniform4fv(pf_lightPositionUniform_sphere[0], 1, lightPos);

        glUniform3fv(pf_kaUniform_sphere, 1, materialAmbient);
        glUniform3fv(pf_kdUniform_sphere, 1, materialDiffuse);
        glUniform3fv(pf_ksUniform_sphere, 1, materialSpecular);
        glUniform1f(pf_materialShininessUniform_sphere, materialShininess);
        glUniform1i(pf_lKeyPressedUniform_sphere, bLight ? 1 : 0);
    }

    // Draw sphere
    glBindVertexArray(vao_sphere);
    {
        int numElements = [sphere getNumberOfSphereElements];
        glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
    }
    glBindVertexArray(0);

    glUseProgram(0);

    // Unbind FBO and restore main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
}

-(void)display
{
    // First render sphere to FBO
    if(fboResult == YES){
        [self displaySphere];
    }

    // Restore main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // Restore cube's resize
    [self resize:winWidth :winHeight];

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw cube with FBO texture
    glUseProgram(shaderProgramObject_cube);
    {
        vmath::mat4 modelViewMatrix = vmath::mat4::identity();
        vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
        vmath::mat4 translationMatrix = vmath::mat4::identity();
        vmath::mat4 rotationMatrix = vmath::mat4::identity();

        translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
        
        rotationMatrix = vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f);
        rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f);
        rotationMatrix = rotationMatrix * vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f);

        modelViewMatrix = translationMatrix * rotationMatrix;
        modelViewProjectionMatrix = perspectiveProjectionMatrix_cube * modelViewMatrix;

        glUniformMatrix4fv(mvpMatrixUniform_cube, 1, GL_FALSE, (const float*)modelViewProjectionMatrix);

        // Bind FBO texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glUniform1i(textureSamplerUniform, 0);

        // Draw cube
        glBindVertexArray(vao_cube);
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   // front
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);   // right
            glDrawArrays(GL_TRIANGLE_FAN, 8, 4);   // back
            glDrawArrays(GL_TRIANGLE_FAN, 12, 4);  // left
            glDrawArrays(GL_TRIANGLE_FAN, 16, 4);  // top
            glDrawArrays(GL_TRIANGLE_FAN, 20, 4);  // bottom
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}

-(void)myupdate
{
    angleCube = angleCube - 0.5f;

    if(bLight){
        lightAngleSingle = lightAngleSingle + 2.0f;
        if(lightAngleSingle >= 360.0f)
            lightAngleSingle = 0.0f;
    }
}

-(void)uninitialize
{
    // Cube cleanup
    if(vbo_texcoord_cube){
        glDeleteBuffers(1, &vbo_texcoord_cube);
        vbo_texcoord_cube = 0;
    }
    if(vbo_position_cube){
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }
    if(vao_cube){
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }
    
    // Sphere cleanup
    if(vbo_position_sphere){
        glDeleteBuffers(1, &vbo_position_sphere);
        vbo_position_sphere = 0;
    }
    if(vbo_normal_sphere){
        glDeleteBuffers(1, &vbo_normal_sphere);
        vbo_normal_sphere = 0;
    }
    if(vbo_element_sphere){
        glDeleteBuffers(1, &vbo_element_sphere);
        vbo_element_sphere = 0;
    }
    if(vao_sphere){
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }
    
    if(sphere){
        [sphere release];
        sphere = nil;
    }
    
    // FBO cleanup
    if(fboTexture){
        glDeleteTextures(1, &fboTexture);
        fboTexture = 0;
    }
    if(rbo){
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
    if(fbo){
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    
    // Shader cleanup - Cube
    if(shaderProgramObject_cube){
        glUseProgram(shaderProgramObject_cube);
        GLint numShaders;
        glGetProgramiv(shaderProgramObject_cube, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(shaderProgramObject_cube, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(shaderProgramObject_cube, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
            }
        }
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject_cube);
        shaderProgramObject_cube = 0;
    }
    
    // Shader cleanup - PV Sphere
    if(pv_shaderProgramObject_sphere){
        glUseProgram(pv_shaderProgramObject_sphere);
        GLint numShaders;
        glGetProgramiv(pv_shaderProgramObject_sphere, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pv_shaderProgramObject_sphere, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pv_shaderProgramObject_sphere, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
            }
        }
        glUseProgram(0);
        glDeleteProgram(pv_shaderProgramObject_sphere);
        pv_shaderProgramObject_sphere = 0;
    }
    
    // Shader cleanup - PF Sphere
    if(pf_shaderProgramObject_sphere){
        glUseProgram(pf_shaderProgramObject_sphere);
        GLint numShaders;
        glGetProgramiv(pf_shaderProgramObject_sphere, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0){
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if(pShaders){
                glGetAttachedShaders(pf_shaderProgramObject_sphere, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++){
                    glDetachShader(pf_shaderProgramObject_sphere, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                }
                free(pShaders);
            }
        }
        glUseProgram(0);
        glDeleteProgram(pf_shaderProgramObject_sphere);
        pf_shaderProgramObject_sphere = 0;
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

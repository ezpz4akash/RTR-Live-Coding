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
    GLint pvLaUniform;
    GLint pvLdUniform;
    GLint pvLsUniform;
    GLint pvLightPositionUniform;
    GLint pvKaUniform;
    GLint pvKdUniform;
    GLint pvKsUniform;
    GLint pvMaterialShininessUniform;
    GLint pvLKeyPressedUniform;
    
    // Per-Fragment uniforms
    GLint pfModelMatrixUniform;
    GLint pfViewMatrixUniform;
    GLint pfProjectionMatrixUniform;
    GLint pfLaUniform;
    GLint pfLdUniform;
    GLint pfLsUniform;
    GLint pfLightPositionUniform;
    GLint pfKaUniform;
    GLint pfKdUniform;
    GLint pfKsUniform;
    GLint pfMaterialShininessUniform;
    GLint pfLKeyPressedUniform;
    
    Sphere *sphere;
    vmath::mat4 perspectiveProjectionMatrix;
    
    // Light properties
    float lightAmbient[4];
    float lightDiffuse[4];
    float lightSpecular[4];
    float lightPosition[4];
    
    // Material properties for 24 spheres
    float materialAmbient[24][4];
    float materialDiffuse[24][4];
    float materialSpecular[24][4];
    float materialShininess[24];
    
    // Sphere translations for 24 spheres (4 columns x 6 rows)
    float sphereTranslation[24][3];
    
    // Rotation angles for light
    float angleForXRotation;
    float angleForYRotation;
    float angleForZRotation;
    
    int onLongPress;
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
        
        // Initialize light properties - white light
        lightAmbient[0] = 1.0f; lightAmbient[1] = 1.0f; lightAmbient[2] = 1.0f; lightAmbient[3] = 1.0f;
        lightDiffuse[0] = 1.0f; lightDiffuse[1] = 1.0f; lightDiffuse[2] = 1.0f; lightDiffuse[3] = 1.0f;
        lightSpecular[0] = 1.0f; lightSpecular[1] = 1.0f; lightSpecular[2] = 1.0f; lightSpecular[3] = 1.0f;
        lightPosition[0] = 0.0f; lightPosition[1] = 0.0f; lightPosition[2] = 0.0f; lightPosition[3] = 1.0f;
        
        // Initialize 24 materials (4 columns x 6 rows)
        // Column 1
        // Emerald
        materialAmbient[0][0] = 0.0215f; materialAmbient[0][1] = 0.1745f; materialAmbient[0][2] = 0.0215f; materialAmbient[0][3] = 1.0f;
        materialDiffuse[0][0] = 0.07568f; materialDiffuse[0][1] = 0.61424f; materialDiffuse[0][2] = 0.07568f; materialDiffuse[0][3] = 1.0f;
        materialSpecular[0][0] = 0.633f; materialSpecular[0][1] = 0.727811f; materialSpecular[0][2] = 0.633f; materialSpecular[0][3] = 1.0f;
        materialShininess[0] = 0.6f * 128.0f;
        
        // Jade
        materialAmbient[1][0] = 0.135f; materialAmbient[1][1] = 0.2225f; materialAmbient[1][2] = 0.1575f; materialAmbient[1][3] = 1.0f;
        materialDiffuse[1][0] = 0.54f; materialDiffuse[1][1] = 0.89f; materialDiffuse[1][2] = 0.63f; materialDiffuse[1][3] = 1.0f;
        materialSpecular[1][0] = 0.316228f; materialSpecular[1][1] = 0.316228f; materialSpecular[1][2] = 0.316228f; materialSpecular[1][3] = 1.0f;
        materialShininess[1] = 0.1f * 128.0f;
        
        // Obsidian
        materialAmbient[2][0] = 0.05375f; materialAmbient[2][1] = 0.05f; materialAmbient[2][2] = 0.06625f; materialAmbient[2][3] = 1.0f;
        materialDiffuse[2][0] = 0.18275f; materialDiffuse[2][1] = 0.17f; materialDiffuse[2][2] = 0.22525f; materialDiffuse[2][3] = 1.0f;
        materialSpecular[2][0] = 0.332741f; materialSpecular[2][1] = 0.328634f; materialSpecular[2][2] = 0.346435f; materialSpecular[2][3] = 1.0f;
        materialShininess[2] = 0.3f * 128.0f;
        
        // Pearl
        materialAmbient[3][0] = 0.25f; materialAmbient[3][1] = 0.20725f; materialAmbient[3][2] = 0.20725f; materialAmbient[3][3] = 1.0f;
        materialDiffuse[3][0] = 1.0f; materialDiffuse[3][1] = 0.829f; materialDiffuse[3][2] = 0.829f; materialDiffuse[3][3] = 1.0f;
        materialSpecular[3][0] = 0.296648f; materialSpecular[3][1] = 0.296648f; materialSpecular[3][2] = 0.296648f; materialSpecular[3][3] = 1.0f;
        materialShininess[3] = 0.088f * 128.0f;
        
        // Ruby
        materialAmbient[4][0] = 0.1745f; materialAmbient[4][1] = 0.01175f; materialAmbient[4][2] = 0.01175f; materialAmbient[4][3] = 1.0f;
        materialDiffuse[4][0] = 0.61424f; materialDiffuse[4][1] = 0.04136f; materialDiffuse[4][2] = 0.04136f; materialDiffuse[4][3] = 1.0f;
        materialSpecular[4][0] = 0.727811f; materialSpecular[4][1] = 0.626959f; materialSpecular[4][2] = 0.626959f; materialSpecular[4][3] = 1.0f;
        materialShininess[4] = 0.6f * 128.0f;
        
        // Turquoise
        materialAmbient[5][0] = 0.1f; materialAmbient[5][1] = 0.18725f; materialAmbient[5][2] = 0.1745f; materialAmbient[5][3] = 1.0f;
        materialDiffuse[5][0] = 0.396f; materialDiffuse[5][1] = 0.74151f; materialDiffuse[5][2] = 0.69102f; materialDiffuse[5][3] = 1.0f;
        materialSpecular[5][0] = 0.297254f; materialSpecular[5][1] = 0.30829f; materialSpecular[5][2] = 0.306678f; materialSpecular[5][3] = 1.0f;
        materialShininess[5] = 0.1f * 128.0f;
        
        // Column 2
        // Brass
        materialAmbient[6][0] = 0.329412f; materialAmbient[6][1] = 0.223529f; materialAmbient[6][2] = 0.027451f; materialAmbient[6][3] = 1.0f;
        materialDiffuse[6][0] = 0.780392f; materialDiffuse[6][1] = 0.568627f; materialDiffuse[6][2] = 0.113725f; materialDiffuse[6][3] = 1.0f;
        materialSpecular[6][0] = 0.992157f; materialSpecular[6][1] = 0.941176f; materialSpecular[6][2] = 0.807843f; materialSpecular[6][3] = 1.0f;
        materialShininess[6] = 0.21794872f * 128.0f;
        
        // Bronze
        materialAmbient[7][0] = 0.2125f; materialAmbient[7][1] = 0.1275f; materialAmbient[7][2] = 0.054f; materialAmbient[7][3] = 1.0f;
        materialDiffuse[7][0] = 0.714f; materialDiffuse[7][1] = 0.4284f; materialDiffuse[7][2] = 0.18144f; materialDiffuse[7][3] = 1.0f;
        materialSpecular[7][0] = 0.393548f; materialSpecular[7][1] = 0.271906f; materialSpecular[7][2] = 0.166721f; materialSpecular[7][3] = 1.0f;
        materialShininess[7] = 0.2f * 128.0f;
        
        // Chrome
        materialAmbient[8][0] = 0.25f; materialAmbient[8][1] = 0.25f; materialAmbient[8][2] = 0.25f; materialAmbient[8][3] = 1.0f;
        materialDiffuse[8][0] = 0.4f; materialDiffuse[8][1] = 0.4f; materialDiffuse[8][2] = 0.4f; materialDiffuse[8][3] = 1.0f;
        materialSpecular[8][0] = 0.774597f; materialSpecular[8][1] = 0.774597f; materialSpecular[8][2] = 0.774597f; materialSpecular[8][3] = 1.0f;
        materialShininess[8] = 0.6f * 128.0f;
        
        // Copper
        materialAmbient[9][0] = 0.19125f; materialAmbient[9][1] = 0.0735f; materialAmbient[9][2] = 0.0225f; materialAmbient[9][3] = 1.0f;
        materialDiffuse[9][0] = 0.7038f; materialDiffuse[9][1] = 0.27048f; materialDiffuse[9][2] = 0.0828f; materialDiffuse[9][3] = 1.0f;
        materialSpecular[9][0] = 0.256777f; materialSpecular[9][1] = 0.137622f; materialSpecular[9][2] = 0.086014f; materialSpecular[9][3] = 1.0f;
        materialShininess[9] = 0.1f * 128.0f;
        
        // Gold
        materialAmbient[10][0] = 0.24725f; materialAmbient[10][1] = 0.1995f; materialAmbient[10][2] = 0.0745f; materialAmbient[10][3] = 1.0f;
        materialDiffuse[10][0] = 0.75164f; materialDiffuse[10][1] = 0.60648f; materialDiffuse[10][2] = 0.22648f; materialDiffuse[10][3] = 1.0f;
        materialSpecular[10][0] = 0.628281f; materialSpecular[10][1] = 0.555802f; materialSpecular[10][2] = 0.366065f; materialSpecular[10][3] = 1.0f;
        materialShininess[10] = 0.4f * 128.0f;
        
        // Silver
        materialAmbient[11][0] = 0.19225f; materialAmbient[11][1] = 0.19225f; materialAmbient[11][2] = 0.19225f; materialAmbient[11][3] = 1.0f;
        materialDiffuse[11][0] = 0.50754f; materialDiffuse[11][1] = 0.50754f; materialDiffuse[11][2] = 0.50754f; materialDiffuse[11][3] = 1.0f;
        materialSpecular[11][0] = 0.508273f; materialSpecular[11][1] = 0.508273f; materialSpecular[11][2] = 0.508273f; materialSpecular[11][3] = 1.0f;
        materialShininess[11] = 0.4f * 128.0f;
        
        // Column 3
        // Black
        materialAmbient[12][0] = 0.0f; materialAmbient[12][1] = 0.0f; materialAmbient[12][2] = 0.0f; materialAmbient[12][3] = 1.0f;
        materialDiffuse[12][0] = 0.01f; materialDiffuse[12][1] = 0.01f; materialDiffuse[12][2] = 0.01f; materialDiffuse[12][3] = 1.0f;
        materialSpecular[12][0] = 0.5f; materialSpecular[12][1] = 0.5f; materialSpecular[12][2] = 0.5f; materialSpecular[12][3] = 1.0f;
        materialShininess[12] = 0.25f * 128.0f;
        
        // Cyan
        materialAmbient[13][0] = 0.0f; materialAmbient[13][1] = 0.1f; materialAmbient[13][2] = 0.06f; materialAmbient[13][3] = 1.0f;
        materialDiffuse[13][0] = 0.0f; materialDiffuse[13][1] = 0.50980392f; materialDiffuse[13][2] = 0.50980392f; materialDiffuse[13][3] = 1.0f;
        materialSpecular[13][0] = 0.50980392f; materialSpecular[13][1] = 0.50980392f; materialSpecular[13][2] = 0.50980392f; materialSpecular[13][3] = 1.0f;
        materialShininess[13] = 0.25f * 128.0f;
        
        // Green
        materialAmbient[14][0] = 0.0f; materialAmbient[14][1] = 0.0f; materialAmbient[14][2] = 0.0f; materialAmbient[14][3] = 1.0f;
        materialDiffuse[14][0] = 0.1f; materialDiffuse[14][1] = 0.35f; materialDiffuse[14][2] = 0.1f; materialDiffuse[14][3] = 1.0f;
        materialSpecular[14][0] = 0.45f; materialSpecular[14][1] = 0.55f; materialSpecular[14][2] = 0.45f; materialSpecular[14][3] = 1.0f;
        materialShininess[14] = 0.25f * 128.0f;
        
        // Red
        materialAmbient[15][0] = 0.0f; materialAmbient[15][1] = 0.0f; materialAmbient[15][2] = 0.0f; materialAmbient[15][3] = 1.0f;
        materialDiffuse[15][0] = 0.5f; materialDiffuse[15][1] = 0.0f; materialDiffuse[15][2] = 0.0f; materialDiffuse[15][3] = 1.0f;
        materialSpecular[15][0] = 0.7f; materialSpecular[15][1] = 0.6f; materialSpecular[15][2] = 0.6f; materialSpecular[15][3] = 1.0f;
        materialShininess[15] = 0.25f * 128.0f;
        
        // White
        materialAmbient[16][0] = 0.0f; materialAmbient[16][1] = 0.0f; materialAmbient[16][2] = 0.0f; materialAmbient[16][3] = 1.0f;
        materialDiffuse[16][0] = 0.55f; materialDiffuse[16][1] = 0.55f; materialDiffuse[16][2] = 0.55f; materialDiffuse[16][3] = 1.0f;
        materialSpecular[16][0] = 0.7f; materialSpecular[16][1] = 0.7f; materialSpecular[16][2] = 0.7f; materialSpecular[16][3] = 1.0f;
        materialShininess[16] = 0.25f * 128.0f;
        
        // Yellow
        materialAmbient[17][0] = 0.0f; materialAmbient[17][1] = 0.0f; materialAmbient[17][2] = 0.0f; materialAmbient[17][3] = 1.0f;
        materialDiffuse[17][0] = 0.5f; materialDiffuse[17][1] = 0.5f; materialDiffuse[17][2] = 0.0f; materialDiffuse[17][3] = 1.0f;
        materialSpecular[17][0] = 0.6f; materialSpecular[17][1] = 0.6f; materialSpecular[17][2] = 0.5f; materialSpecular[17][3] = 1.0f;
        materialShininess[17] = 0.25f * 128.0f;
        
        // Column 4
        // Black Plastic
        materialAmbient[18][0] = 0.02f; materialAmbient[18][1] = 0.02f; materialAmbient[18][2] = 0.02f; materialAmbient[18][3] = 1.0f;
        materialDiffuse[18][0] = 0.01f; materialDiffuse[18][1] = 0.01f; materialDiffuse[18][2] = 0.01f; materialDiffuse[18][3] = 1.0f;
        materialSpecular[18][0] = 0.4f; materialSpecular[18][1] = 0.4f; materialSpecular[18][2] = 0.4f; materialSpecular[18][3] = 1.0f;
        materialShininess[18] = 0.078125f * 128.0f;
        
        // Cyan Plastic
        materialAmbient[19][0] = 0.0f; materialAmbient[19][1] = 0.05f; materialAmbient[19][2] = 0.05f; materialAmbient[19][3] = 1.0f;
        materialDiffuse[19][0] = 0.4f; materialDiffuse[19][1] = 0.5f; materialDiffuse[19][2] = 0.5f; materialDiffuse[19][3] = 1.0f;
        materialSpecular[19][0] = 0.04f; materialSpecular[19][1] = 0.7f; materialSpecular[19][2] = 0.7f; materialSpecular[19][3] = 1.0f;
        materialShininess[19] = 0.078125f * 128.0f;
        
        // Green Plastic
        materialAmbient[20][0] = 0.0f; materialAmbient[20][1] = 0.05f; materialAmbient[20][2] = 0.0f; materialAmbient[20][3] = 1.0f;
        materialDiffuse[20][0] = 0.4f; materialDiffuse[20][1] = 0.5f; materialDiffuse[20][2] = 0.4f; materialDiffuse[20][3] = 1.0f;
        materialSpecular[20][0] = 0.04f; materialSpecular[20][1] = 0.7f; materialSpecular[20][2] = 0.04f; materialSpecular[20][3] = 1.0f;
        materialShininess[20] = 0.078125f * 128.0f;
        
        // Red Plastic
        materialAmbient[21][0] = 0.05f; materialAmbient[21][1] = 0.0f; materialAmbient[21][2] = 0.0f; materialAmbient[21][3] = 1.0f;
        materialDiffuse[21][0] = 0.5f; materialDiffuse[21][1] = 0.4f; materialDiffuse[21][2] = 0.4f; materialDiffuse[21][3] = 1.0f;
        materialSpecular[21][0] = 0.7f; materialSpecular[21][1] = 0.04f; materialSpecular[21][2] = 0.04f; materialSpecular[21][3] = 1.0f;
        materialShininess[21] = 0.078125f * 128.0f;
        
        // White Plastic
        materialAmbient[22][0] = 0.05f; materialAmbient[22][1] = 0.05f; materialAmbient[22][2] = 0.05f; materialAmbient[22][3] = 1.0f;
        materialDiffuse[22][0] = 0.5f; materialDiffuse[22][1] = 0.5f; materialDiffuse[22][2] = 0.5f; materialDiffuse[22][3] = 1.0f;
        materialSpecular[22][0] = 0.7f; materialSpecular[22][1] = 0.7f; materialSpecular[22][2] = 0.7f; materialSpecular[22][3] = 1.0f;
        materialShininess[22] = 0.078125f * 128.0f;
        
        // Yellow Plastic
        materialAmbient[23][0] = 0.05f; materialAmbient[23][1] = 0.05f; materialAmbient[23][2] = 0.0f; materialAmbient[23][3] = 1.0f;
        materialDiffuse[23][0] = 0.5f; materialDiffuse[23][1] = 0.5f; materialDiffuse[23][2] = 0.4f; materialDiffuse[23][3] = 1.0f;
        materialSpecular[23][0] = 0.7f; materialSpecular[23][1] = 0.7f; materialSpecular[23][2] = 0.04f; materialSpecular[23][3] = 1.0f;
        materialShininess[23] = 0.078125f * 128.0f;
        
        // Initialize sphere translations (4 columns x 6 rows)
        // Column 1
        sphereTranslation[0][0] = 1.5f; sphereTranslation[0][1] = 14.0f; sphereTranslation[0][2] = -20.0f;
        sphereTranslation[1][0] = 1.5f; sphereTranslation[1][1] = 11.5f; sphereTranslation[1][2] = -20.0f;
        sphereTranslation[2][0] = 1.5f; sphereTranslation[2][1] = 9.0f; sphereTranslation[2][2] = -20.0f;
        sphereTranslation[3][0] = 1.5f; sphereTranslation[3][1] = 6.5f; sphereTranslation[3][2] = -20.0f;
        sphereTranslation[4][0] = 1.5f; sphereTranslation[4][1] = 4.0f; sphereTranslation[4][2] = -20.0f;
        sphereTranslation[5][0] = 1.5f; sphereTranslation[5][1] = 1.5f; sphereTranslation[5][2] = -20.0f;
        // Column 2
        sphereTranslation[6][0] = 7.5f; sphereTranslation[6][1] = 14.0f; sphereTranslation[6][2] = -20.0f;
        sphereTranslation[7][0] = 7.5f; sphereTranslation[7][1] = 11.5f; sphereTranslation[7][2] = -20.0f;
        sphereTranslation[8][0] = 7.5f; sphereTranslation[8][1] = 9.0f; sphereTranslation[8][2] = -20.0f;
        sphereTranslation[9][0] = 7.5f; sphereTranslation[9][1] = 6.5f; sphereTranslation[9][2] = -20.0f;
        sphereTranslation[10][0] = 7.5f; sphereTranslation[10][1] = 4.0f; sphereTranslation[10][2] = -20.0f;
        sphereTranslation[11][0] = 7.5f; sphereTranslation[11][1] = 1.5f; sphereTranslation[11][2] = -20.0f;
        // Column 3
        sphereTranslation[12][0] = 13.5f; sphereTranslation[12][1] = 14.0f; sphereTranslation[12][2] = -20.0f;
        sphereTranslation[13][0] = 13.5f; sphereTranslation[13][1] = 11.5f; sphereTranslation[13][2] = -20.0f;
        sphereTranslation[14][0] = 13.5f; sphereTranslation[14][1] = 9.0f; sphereTranslation[14][2] = -20.0f;
        sphereTranslation[15][0] = 13.5f; sphereTranslation[15][1] = 6.5f; sphereTranslation[15][2] = -20.0f;
        sphereTranslation[16][0] = 13.5f; sphereTranslation[16][1] = 4.0f; sphereTranslation[16][2] = -20.0f;
        sphereTranslation[17][0] = 13.5f; sphereTranslation[17][1] = 1.5f; sphereTranslation[17][2] = -20.0f;
        // Column 4
        sphereTranslation[18][0] = 19.5f; sphereTranslation[18][1] = 14.0f; sphereTranslation[18][2] = -20.0f;
        sphereTranslation[19][0] = 19.5f; sphereTranslation[19][1] = 11.5f; sphereTranslation[19][2] = -20.0f;
        sphereTranslation[20][0] = 19.5f; sphereTranslation[20][1] = 9.0f; sphereTranslation[20][2] = -20.0f;
        sphereTranslation[21][0] = 19.5f; sphereTranslation[21][1] = 6.5f; sphereTranslation[21][2] = -20.0f;
        sphereTranslation[22][0] = 19.5f; sphereTranslation[22][1] = 4.0f; sphereTranslation[22][2] = -20.0f;
        sphereTranslation[23][0] = 19.5f; sphereTranslation[23][1] = 1.5f; sphereTranslation[23][2] = -20.0f;
        
        // Initialize rotation angles
        angleForXRotation = 0.0f;
        angleForYRotation = 0.0f;
        angleForZRotation = 0.0f;
        
        onLongPress = 0;
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
    // Cycle through X, Y, Z rotation axes
    onLongPress = onLongPress + 1;
    if(onLongPress > 2) {
        onLongPress = 0;
    }
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
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n"
        "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n"
        "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n"
        "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n"
        "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n"
        "       vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n"
        "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n"
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
    pvLaUniform = glGetUniformLocation(pvShaderProgramObject, "uLa");
    pvLdUniform = glGetUniformLocation(pvShaderProgramObject, "uLd");
    pvLsUniform = glGetUniformLocation(pvShaderProgramObject, "uLs");
    pvLightPositionUniform = glGetUniformLocation(pvShaderProgramObject, "uLightPosition");
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
        "   vec3 normalizedLightSource = normalize(lightSource);\n"
        "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n"
        "   if(uLKeyPressed == 1)\n"
        "   {\n"
        "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n"
        "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n"
        "       vec3 viewerVector = -normalizedViewerVector.xyz;\n"
        "       vec3 ambient = uLa * uKa;\n"
        "       vec3 diffuse = uLd * uKd * tnDotLd;\n"
        "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n"
        "       FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
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
    pfLaUniform = glGetUniformLocation(pfShaderProgramObject, "uLa");
    pfLdUniform = glGetUniformLocation(pfShaderProgramObject, "uLd");
    pfLsUniform = glGetUniformLocation(pfShaderProgramObject, "uLs");
    pfLightPositionUniform = glGetUniformLocation(pfShaderProgramObject, "uLightPosition");
    pfKaUniform = glGetUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = glGetUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = glGetUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = glGetUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLKeyPressedUniform = glGetUniformLocation(pfShaderProgramObject, "uLKeyPressed");

    // Initialize sphere
    sphere = [[Sphere alloc]init];
    
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

        // element VBO
        glGenBuffers(1, &vbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numSphereElements * sizeof(unsigned short), sphereElements, GL_STATIC_DRAW);

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
    
    // Base translation to center the grid
    modelMatrix = vmath::translate(-10.0f, -8.0f, 0.0f);
    
    // Calculate light position based on rotation axis
    float lightRadius = 20.0f;
    float lightPosX = 0.0f, lightPosY = 0.0f, lightPosZ = 0.0f;
    float rotationAngle = 0.0f;
    
    if(onLongPress == 0) {
        // X-axis rotation
        rotationAngle = angleForXRotation * M_PI / 180.0f;
        lightPosY = lightRadius * sinf(rotationAngle);
        lightPosZ = lightRadius * cosf(rotationAngle);
    } else if(onLongPress == 1) {
        // Y-axis rotation
        rotationAngle = angleForYRotation * M_PI / 180.0f;
        lightPosX = lightRadius * sinf(rotationAngle);
        lightPosZ = lightRadius * cosf(rotationAngle);
    } else if(onLongPress == 2) {
        // Z-axis rotation
        rotationAngle = angleForZRotation * M_PI / 180.0f;
        lightPosX = lightRadius * cosf(rotationAngle);
        lightPosY = lightRadius * sinf(rotationAngle);
    }
    
    // Get number of elements for drawing
    int numElements = [sphere getNumberOfSphereElements];
    
    // Draw 24 spheres with different materials
    for(int i = 0; i < 24; i++) {
        // Create sphere-specific model matrix
        vmath::mat4 sphereModelMatrix = vmath::translate(sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]) * modelMatrix;
        
        // Calculate light position relative to this sphere
        float sphereLightPos[4] = {
            sphereTranslation[i][0] - 10.0f + lightPosX,
            sphereTranslation[i][1] - 8.0f + lightPosY,
            sphereTranslation[i][2] + lightPosZ,
            1.0f
        };
        
        if(bPerVertexPerFragmentToggle) {
            // Per-Fragment rendering
            glUniformMatrix4fv(pfModelMatrixUniform, 1, GL_FALSE, (const float*)sphereModelMatrix);
            glUniformMatrix4fv(pfViewMatrixUniform, 1, GL_FALSE, (const float*)viewMatrix);
            glUniformMatrix4fv(pfProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
            
            glUniform3fv(pfLaUniform, 1, lightAmbient);
            glUniform3fv(pfLdUniform, 1, lightDiffuse);
            glUniform3fv(pfLsUniform, 1, lightSpecular);
            glUniform4fv(pfLightPositionUniform, 1, sphereLightPos);
            
            glUniform3fv(pfKaUniform, 1, materialAmbient[i]);
            glUniform3fv(pfKdUniform, 1, materialDiffuse[i]);
            glUniform3fv(pfKsUniform, 1, materialSpecular[i]);
            glUniform1f(pfMaterialShininessUniform, materialShininess[i]);
            glUniform1i(pfLKeyPressedUniform, bLight ? 1 : 0);
        } else {
            // Per-Vertex rendering
            glUniformMatrix4fv(pvModelMatrixUniform, 1, GL_FALSE, (const float*)sphereModelMatrix);
            glUniformMatrix4fv(pvViewMatrixUniform, 1, GL_FALSE, (const float*)viewMatrix);
            glUniformMatrix4fv(pvProjectionMatrixUniform, 1, GL_FALSE, (const float*)perspectiveProjectionMatrix);
            
            glUniform3fv(pvLaUniform, 1, lightAmbient);
            glUniform3fv(pvLdUniform, 1, lightDiffuse);
            glUniform3fv(pvLsUniform, 1, lightSpecular);
            glUniform4fv(pvLightPositionUniform, 1, sphereLightPos);
            
            glUniform3fv(pvKaUniform, 1, materialAmbient[i]);
            glUniform3fv(pvKdUniform, 1, materialDiffuse[i]);
            glUniform3fv(pvKsUniform, 1, materialSpecular[i]);
            glUniform1f(pvMaterialShininessUniform, materialShininess[i]);
            glUniform1i(pvLKeyPressedUniform, bLight ? 1 : 0);
        }
        
        // Draw sphere
        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
    }
    
    glUseProgram(0);
}

-(void)myupdate
{
    // Update all rotation angles
    angleForXRotation += 5.0f;
    angleForYRotation += 5.0f;
    angleForZRotation += 5.0f;
    
    if(angleForXRotation >= 360.0f) angleForXRotation = 0.0f;
    if(angleForYRotation >= 360.0f) angleForYRotation = 0.0f;
    if(angleForZRotation >= 360.0f) angleForZRotation = 0.0f;
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

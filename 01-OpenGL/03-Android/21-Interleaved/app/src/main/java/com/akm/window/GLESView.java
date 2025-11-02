package com.akm.window;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;

//Event related packages
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

//OpenGL related packages
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;
import android.opengl.Matrix;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

//Texture related packages
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    //Event related variables
    private GestureDetector gestureDetector;
    private Context context;

    //OpenGL related variables
    private int shaderProgramObject;
    private float perspectiveProjectionMatrix[] = new float[16];
    private int modelMatrixUniform;
    private int viewMatrixUniform;
    private int projectionMatrixUniform;

    // Light Settings
    private int laUniform = 0;
    private int ldUniform = 0;
    private int lsUniform = 0;
    private int lightPositionUniform = 0;

    private int kaUniform = 0;
    private int kdUniform = 0;
    private int ksUniform = 0;
    private int materialShininessUniform = 0;

    private int singleTapUniform = 0;

    private boolean bLight = false;

    private float lightAmbient[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
    private float lightDiffuse[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightSpecular[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightPosition[] = new float[] {100.0f, 100.0f, 100.0f, 1.0f};

    private float materialAmbient[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
    private float materialDiffuse[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialSpecular[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 50.0f;

    //Cube
    private int vaoCube[] = new int[1];
    private int vboCube[] = new int[1];
    /* private int vboCubePosition[] = new int[1];
    private int vboCubeColor[] = new int[1];
    private int vboCubeNormal[] = new int[1];
    private int vboTexCoords[] = new int[1]; */
    private float angleCube = 0.0f;

    //Texture variables
    private int marbleTexture;
    private int textureSamplerUniform;

    public GLESView(Context _context)
    {
        super(_context);

        context = _context;

        gestureDetector = new GestureDetector(context, this, null, false);

        //Set this class as double tap listener
        gestureDetector.setOnDoubleTapListener(this);

        //OpenGL ES version 3.0
        setEGLContextClientVersion(3);

        //Set Renderer for drawing on the GLSurfaceView
        setRenderer(this);

        //Render the view only when there is a change in the drawing data
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
    
    //Event related methods

    // Implement one method from view
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);
        return true;
    }

    //Renderer methods
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        //Code
        int iResult = initialize(gl);
        if(iResult != 0)
        {
            //Log error
            System.out.println("AKM: Error in initialize()");
            System.exit(0);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        //Code
        display();
        update();
    }
    
    //OnGestureListener
    @Override
    public boolean onDown(MotionEvent e)
    {
        return true;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return true;
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) //Will be used as swipe/unitialize
    {
        uninitialize();
        System.exit(0);
        return true;
    }

    @Override
    public void onShowPress(MotionEvent e)  
    {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return true;
    }

    //OnDoubleTapListener
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        bLight = !bLight;
        return true;
    }

    //Custom method
    private int initialize(GL10 gl)
    {
        printGLESInfo(gl);

        // Vertex Shader
        final String vertexShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec4 aPosition;\n" + 
            "in vec4 aColor;\n" + 
            "in vec3 aNormal;\n" + 
            "in vec2 aTexCoord;\n" + 
            "out vec2 out_texCoord;\n" + 
            "out vec4 eyeCoordinates;\n" + 
            "out vec3 transformedNormal;\n" + 
            "out vec3 lightSource;\n" + 
            "uniform mat4 uModelMatrix;\n" + 
            "uniform mat4 uViewMatrix;\n" + 
            "uniform mat4 uProjectionMatrix;\n" + 
            "uniform vec4 uLightPosition;\n" + 
            "out vec4 out_color;\n" +
            "void main(void)\n" + 
            "{\n" + 
            "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "   transformedNormal = (normalMatrix * aNormal);\n" + 
            "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" + 
            "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" + 
            "   out_color = aColor;\n" + 
            "   out_texCoord = aTexCoord;\n" + 
            "}\n"
        );
        int vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
        GLES32.glCompileShader(vertexShaderObject);
        int[] iStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;
        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("AKM: Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }


        //Fragment Shader
        final String fragmentShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec3 transformedNormal;\n" + 
            "in vec4 eyeCoordinates;\n" + 
            "in vec3 lightSource;\n" + 
            "in vec4 out_color;\n" +
            "in vec2 out_texCoord;\n" + 
            "uniform vec3 uLa;\n" + 
            "uniform vec3 uLd;\n" + 
            "uniform vec3 uLs;\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "out vec4 FragColor;\n" + 
            "uniform sampler2D uTextureSampler;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
            "   vec3 normalizedLightSource = normalize(lightSource);\n" + 
            "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
            "   if(uLKeyPressed == 1)\n" + 
            "   {\n" + 
            "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n" + 
            "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n" + 
            "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
            "       vec3 ambient = uLa * uKa;\n" + 
            "       vec3 diffuse = uLd * uKd * tnDotLd;\n" + 
            "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" + 
            "       FragColor = vec4(ambient + diffuse + specular, 1.0) * out_color * texture(uTextureSampler, out_texCoord);\n" + 
            "   }\n" + 
            "   else\n" + 
            "   {\n" + 
            "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" + 
            "   }\n" + 
            "}\n"
        );
        int fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
        GLES32.glCompileShader(fragmentShaderObject);
        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("AKM: Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program
        shaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_COLOR, "aColor");
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_TEXCOORD, "aTexCoord");
        GLES32.glLinkProgram(shaderProgramObject);
        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        modelMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uModelMatrix");
        viewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uViewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

        laUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLa");
        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLd");
        lsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLs");
        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKa");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKs");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLightPosition");
        singleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLKeyPressed");
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uTextureSampler");

        //Cube vertices
        {
            /* final float cubeVertices[] = new float[]
            {
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

            final float cubeColor[] = new float[]
            {
                // front
                1.0f, 0.0f, 0.0f, // top-right of front
                1.0f, 0.0f, 0.0f, // top-left of front
                1.0f, 0.0f, 0.0f, // bottom-left of front
                1.0f, 0.0f, 0.0f, // bottom-right of front

                // right
                0.0f, 0.0f, 1.0f, // top-right of right
                0.0f, 0.0f, 1.0f, // top-left of right
                0.0f, 0.0f, 1.0f, // bottom-left of right
                0.0f, 0.0f, 1.0f, // bottom-right of right

                // back
                1.0f, 1.0f, 0.0f, // top-right of back
                1.0f, 1.0f, 0.0f, // top-left of back
                1.0f, 1.0f, 0.0f, // bottom-left of back
                1.0f, 1.0f, 0.0f, // bottom-right of back

                // left
                1.0f, 0.0f, 1.0f, // top-right of left
                1.0f, 0.0f, 1.0f, // top-left of left
                1.0f, 0.0f, 1.0f, // bottom-left of left
                1.0f, 0.0f, 1.0f, // bottom-right of left

                // top
                0.0f, 1.0f, 0.0f, // top-right of top
                0.0f, 1.0f, 0.0f, // top-left of top
                0.0f, 1.0f, 0.0f, // bottom-left of top
                0.0f, 1.0f, 0.0f, // bottom-right of top

                // bottom
                1.0f, 0.5f, 0.0f, // top-right of bottom
                1.0f, 0.5f, 0.0f, // top-left of bottom
                1.0f, 0.5f, 0.0f, // bottom-left of bottom
                1.0f, 0.5f, 0.0f, // bottom-right of bottom
            };

            final float cube_normal[] = new float[]
            {
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

            final float cube_texcoord[] = new float[]
            {
                //Front face
                0.0f,  1.0f,  0.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,

                //Right face
                0.0f,  1.0f,  0.0f,
                1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,

                //Back face
                0.0f,  1.0f,  0.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,

                //Left face
                0.0f,  1.0f,  0.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f
            };
            */

            final float cube_PCNT[] = new float[]
            {
                // front
                // position				// color			 // normals				// texcoords
                1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 0.0f,
                1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 0.0f,
                // right			 
                // position				// color			 // normals				// texcoords
                1.0f,  1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
                1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
                1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
                1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
                // back				 
                // position				// color			 // normals				// texcoords
                1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 0.0f,
                1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 0.0f,
                // left				 
                // position				// color			 // normals				// texcoords
                -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
                -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
                // top				 
                // position				// color			 // normals				// texcoords
                1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 0.0f,
                1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
                // bottom			 
                // position				// color			 // normals				// texcoords
                1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
                1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
            };

            GLES32.glGenVertexArrays(1, vaoCube, 0);
            GLES32.glBindVertexArray(vaoCube[0]);
            {   
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cube_PCNT.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubePCNT = byteBuffer.asFloatBuffer();
                cubePCNT.put(cube_PCNT);
                cubePCNT.position(0);
                GLES32.glGenBuffers(1, vboCube, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCube[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubePCNT.capacity() * 4, cubePCNT, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 11 * 4, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 11 * 4, 3 * 4);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 11 * 4, 6 * 4);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 11 * 4, 9 * 4);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                /* ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeVerticesBuffer = byteBuffer.asFloatBuffer();
                cubeVerticesBuffer.put(cubeVertices);
                cubeVerticesBuffer.position(0);
                GLES32.glGenBuffers(1, vboCubePosition, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCubePosition[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVerticesBuffer.capacity() * 4, cubeVerticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                //Normal
                byteBuffer = ByteBuffer.allocateDirect(cubeColor.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeColorBuffer = byteBuffer.asFloatBuffer();
                cubeColorBuffer.put(cubeColor);
                cubeColorBuffer.position(0);
                GLES32.glGenBuffers(1, vboCubeColor, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCubeColor[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeColorBuffer.capacity() * 4, cubeColorBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                byteBuffer = ByteBuffer.allocateDirect(cube_normal.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeNormalBuffer = byteBuffer.asFloatBuffer();
                cubeNormalBuffer.put(cube_normal);
                cubeNormalBuffer.position(0);
                GLES32.glGenBuffers(1, vboCubeNormal, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCubeNormal[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeNormalBuffer.capacity() * 4, cubeNormalBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                byteBuffer = ByteBuffer.allocateDirect(cube_texcoord.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeTexCoordBuffer = byteBuffer.asFloatBuffer();
                cubeTexCoordBuffer.put(cube_texcoord);
                cubeTexCoordBuffer.position(0);
                GLES32.glGenBuffers(1, vboTexCoords, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTexCoords[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeTexCoordBuffer.capacity() * 4, cubeTexCoordBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); */
            }
            GLES32.glBindVertexArray(0);
        }


        //Set the background frame color
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Blue Screen

        marbleTexture = loadGLTexture(R.raw.marble);

        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
        return 0;
    }

    public int loadGLTexture(int imageFileResourceID){
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;   // No pre-scaling
        Bitmap bitmap = BitmapFactory.decodeResource(
            context.getResources(),
            imageFileResourceID,
            options
        );
        //Generate texture ID
        int[] textureIDs = new int[1];
        GLES32.glGenTextures(1, textureIDs, 0);
        //Bind with texture ID
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, textureIDs[0]);

        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);

        //Set up texture parameters
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);
        
        //Load the bitmap into the bound texture
        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);

        //Generate MipMap
        GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);

        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

        //Releasing bitmap memory
        bitmap.recycle();

        return textureIDs[0];
    }

    private void resize(int width, int height)
    {
        //Adjust the viewport based on geometry changes, such as screen rotation
        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

    private void display()
    {
        //Redraw background color
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        float modelMatrix[] = new float[16];
        float viewMatrix[] = new float[16];
        float translationMatrix[] = new float[16];
        float rotationMatrix[] = new float[16];
        float scaleMatrix[] = new float[16];

        //Use shader program object
        GLES32.glUseProgram(shaderProgramObject);
        {
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);
            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setIdentityM(scaleMatrix, 0);
            
            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -6.0f);
            Matrix.scaleM(scaleMatrix, 0, 0.75f, 0.75f, 0.75f);
            Matrix.multiplyMM(translationMatrix, 0, translationMatrix, 0, scaleMatrix, 0);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 0.0f, 1.0f, 0.0f);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 1.0f, 0.0f, 0.0f);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 0.0f, 0.0f, 1.0f);
            Matrix.multiplyMM(modelMatrix, 0, translationMatrix, 0, rotationMatrix, 0);

            GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            GLES32.glUniform3fv(laUniform, 1, lightAmbient, 0);
            GLES32.glUniform3fv(ldUniform, 1, lightDiffuse, 0);
            GLES32.glUniform3fv(lsUniform, 1, lightSpecular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, lightPosition, 0);
            GLES32.glUniform3fv(kaUniform, 1, materialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, materialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, materialSpecular, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess);
            GLES32.glUniform1i(singleTapUniform, bLight ? 1 : 0);

            GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, marbleTexture);
            GLES32.glUniform1i(textureSamplerUniform, 0);

            GLES32.glBindVertexArray(vaoCube[0]);
            {
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
            }
            GLES32.glBindVertexArray(0);
        }
        
        //Unuse program
        GLES32.glUseProgram(0);

        //Render the view
        requestRender(); //equivalent to swapBuffers()
    }

    private void update()
    {
        //Code
        angleCube += 2.0f;
    }

    private void uninitialize()
    {
        if(shaderProgramObject != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(shaderProgramObject);
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(shaderProgramObject, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(shaderProgramObject, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
        }
    }

    private void printGLESInfo(GL10 gl)
    {
        String glesVersion = gl.glGetString(GL10.GL_VERSION);
        System.out.println("AKM: OpenGL-ES Version = " + glesVersion);

        String glslVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        System.out.println("AKM: GLSL Version = " + glslVersion);

        String vendor = gl.glGetString(GL10.GL_VENDOR);
        System.out.println("AKM: Vendor = " + vendor);

        String renderer = gl.glGetString(GL10.GL_RENDERER);
        System.out.println("AKM: Renderer = " + renderer);
    }
}
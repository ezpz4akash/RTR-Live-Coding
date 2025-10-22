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

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    //Event related variables
    private GestureDetector gestureDetector;
    private Context context;

    //OpenGL related variables : Cube
    private int shaderProgramObjectCube;
    private int mvpMatrixUniformCube;
    private float perspectiveProjectionMatrixCube[] = new float[16];

    private int vaoCube[] = new int[1];
    private int vboCubePosition[] = new int[1];
    private int vboCubeTexcoord[] = new int[1];

    private int textureSamplerUniform;

    private float angleCube = 0.0f;

    // FBO related variables
    private int fboWidth = 512;
    private int fboHeight = 512;

    private int winWidth;
    private int winHeight;

    private int fbo[] = new int[1];
    private int rbo[] = new int[1];

    private int fboTexture[] = new int[1];
    private int fboResult = -1;

    private int pvShaderProgramObjectSphere;
    private int pfShaderProgramObjectSphere;

    private float perspectiveProjectionMatrixSphere[] = new float[16];

    private int pvModelMatrixUniformSphere;
    private int pvViewMatrixUniformSphere;
    private int pvProjectionMatrixUniformSphere;

    private int pfModelMatrixUniformSphere;
    private int pfViewMatrixUniformSphere;
    private int pfProjectionMatrixUniformSphere;

    //Sphere related variables
    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];
    private int numVerticesSphere;
    private int numElementsSphere;

    // Light Settings
    private int pvLaUniformSphere[] = new int[3];
    private int pvLdUniformSphere[] = new int[3];
    private int pvLsUniformSphere[] = new int[3];
    private int pvLightPositionUniformSphere[] = new int[3];

    private int pvKaUniformSphere = 0;
    private int pvKdUniformSphere = 0;
    private int pvKsUniformSphere = 0;
    private int pvMaterialShininessUniformSphere = 0;

    private int pvSingleTapUniformSphere = 0;

    private int pfLaUniformSphere[] = new int[3];
    private int pfLdUniformSphere[] = new int[3];
    private int pfLsUniformSphere[] = new int[3];
    private int pfLightPositionUniformSphere[] = new int[3];

    private int pfKaUniformSphere = 0;
    private int pfKdUniformSphere = 0;
    private int pfKsUniformSphere = 0;
    private int pfMaterialShininessUniformSphere = 0;

    private int pfSingleTapUniformSphere = 0;

    private int onLongPress = 0;
    private int onSingleTap = 0;

    private boolean bLightSphere = true;
    private boolean perVertexperFragmentToggleSphere = true;

    private float lightAmbientSphere[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightDiffuseSphere[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightSpecularSphere[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightPositionSphere[]  = {10.0f, 10.0f, 10.0f, 1.0f};

    private float angleForXRotationSphere = 0.0f;
    private float angleForYRotationSphere = 0.0f;
    private float angleForZRotationSphere = 0.0f;

    /* private float materialAmbientSphere[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
    private float materialDiffuseSphere[] = new float[] {1.0f, 0.0f, 1.0f, 1.0f};
    private float materialSpecularSphere[] = new float[] {1.0f, 0.0f, 1.0f, 1.0f};
    private float materialShininessSphere = 128.0f; */

    //Material
    Material[] materials = new Material[] {
        // 1st column
        new Material(new float[]{0.0215f, 0.1745f, 0.0215f, 1.0f}, new float[]{0.07568f, 0.61424f, 0.07568f, 1.0f}, new float[]{0.633f, 0.727811f, 0.633f, 1.0f}, 0.6f * 128),
        new Material(new float[]{0.135f, 0.2225f, 0.1575f, 1.0f}, new float[]{0.54f, 0.89f, 0.63f, 1.0f}, new float[]{0.316228f, 0.316228f, 0.316228f, 1.0f}, 0.1f * 128),
        new Material(new float[]{0.05375f, 0.05f, 0.06625f, 1.0f}, new float[]{0.18275f, 0.17f, 0.22525f, 1.0f}, new float[]{0.332741f, 0.328634f, 0.346435f, 1.0f}, 0.3f * 128),
        new Material(new float[]{0.25f, 0.20725f, 0.20725f, 1.0f}, new float[]{1.0f, 0.829f, 0.829f, 1.0f}, new float[]{0.296648f, 0.296648f, 0.296648f, 1.0f}, 0.088f * 128),
        new Material(new float[]{0.1745f, 0.01175f, 0.01175f, 1.0f}, new float[]{0.61424f, 0.04136f, 0.04136f, 1.0f}, new float[]{0.727811f, 0.626959f, 0.626959f, 1.0f}, 0.6f * 128),
        new Material(new float[]{0.1f, 0.18725f, 0.1745f, 1.0f}, new float[]{0.396f, 0.74151f, 0.69102f, 1.0f}, new float[]{0.297254f, 0.30829f, 0.306678f, 1.0f}, 0.1f * 128),

        // 2nd column
        new Material(new float[]{0.329412f, 0.223529f, 0.027451f, 1.0f}, new float[]{0.780392f, 0.568627f, 0.113725f, 1.0f}, new float[]{0.992157f, 0.941176f, 0.807843f, 1.0f}, 0.21794872f * 128),
        new Material(new float[]{0.2125f, 0.1275f, 0.054f, 1.0f}, new float[]{0.714f, 0.4284f, 0.18144f, 1.0f}, new float[]{0.393548f, 0.271906f, 0.166721f, 1.0f}, 0.2f * 128),
        new Material(new float[]{0.25f, 0.25f, 0.25f, 1.0f}, new float[]{0.4f, 0.4f, 0.4f, 1.0f}, new float[]{0.774597f, 0.774597f, 0.774597f, 1.0f}, 0.6f * 128),
        new Material(new float[]{0.19125f, 0.0735f, 0.0225f, 1.0f}, new float[]{0.7038f, 0.27048f, 0.0828f, 1.0f}, new float[]{0.256777f, 0.137622f, 0.086014f, 1.0f}, 0.1f * 128),
        new Material(new float[]{0.24725f, 0.1995f, 0.0745f, 1.0f}, new float[]{0.75164f, 0.60648f, 0.22648f, 1.0f}, new float[]{0.628281f, 0.555802f, 0.366065f, 1.0f}, 0.4f * 128),
        new Material(new float[]{0.19225f, 0.19225f, 0.19225f, 1.0f}, new float[]{0.50754f, 0.50754f, 0.50754f, 1.0f}, new float[]{0.508273f, 0.508273f, 0.508273f, 1.0f}, 0.4f * 128),

        // 3rd column
        new Material(new float[]{0.0f, 0.0f, 0.0f, 1.0f}, new float[]{0.01f, 0.01f, 0.01f, 1.0f}, new float[]{0.5f, 0.5f, 0.5f, 1.0f}, 0.25f * 128),
        new Material(new float[]{0.0f, 0.1f, 0.06f, 1.0f}, new float[]{0.0f, 0.50980392f, 0.50980392f, 1.0f}, new float[]{0.50980392f, 0.50980392f, 0.50980392f, 1.0f}, 0.25f * 128),
        new Material(new float[]{0.0f, 0.0f, 0.0f, 1.0f}, new float[]{0.1f, 0.35f, 0.1f, 1.0f}, new float[]{0.45f, 0.55f, 0.45f, 1.0f}, 0.25f * 128),
        new Material(new float[]{0.0f, 0.0f, 0.0f, 1.0f}, new float[]{0.5f, 0.0f, 0.0f, 1.0f}, new float[]{0.7f, 0.6f, 0.6f, 1.0f}, 0.25f * 128),
        new Material(new float[]{0.0f, 0.0f, 0.0f, 1.0f}, new float[]{0.55f, 0.55f, 0.55f, 1.0f}, new float[]{0.7f, 0.7f, 0.7f, 1.0f}, 0.25f * 128),
        new Material(new float[]{0.0f, 0.0f, 0.0f, 1.0f}, new float[]{0.5f, 0.5f, 0.0f, 1.0f}, new float[]{0.6f, 0.6f, 0.5f, 1.0f}, 0.25f * 128),

        // 4th column
        new Material(new float[]{0.02f, 0.02f, 0.02f, 1.0f}, new float[]{0.01f, 0.01f, 0.01f, 1.0f}, new float[]{0.4f, 0.4f, 0.4f, 1.0f}, 0.078125f * 128),
        new Material(new float[]{0.0f, 0.05f, 0.05f, 1.0f}, new float[]{0.4f, 0.5f, 0.5f, 1.0f}, new float[]{0.04f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128),
        new Material(new float[]{0.0f, 0.05f, 0.0f, 1.0f}, new float[]{0.4f, 0.5f, 0.4f, 1.0f}, new float[]{0.04f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128),
        new Material(new float[]{0.05f, 0.0f, 0.0f, 1.0f}, new float[]{0.5f, 0.4f, 0.4f, 1.0f}, new float[]{0.7f, 0.04f, 0.04f, 1.0f}, 0.078125f * 128),
        new Material(new float[]{0.05f, 0.05f, 0.05f, 1.0f}, new float[]{0.5f, 0.5f, 0.5f, 1.0f}, new float[]{0.7f, 0.7f, 0.7f, 1.0f}, 0.078125f * 128),
        new Material(new float[]{0.05f, 0.05f, 0.0f, 1.0f}, new float[]{0.5f, 0.5f, 0.4f, 1.0f}, new float[]{0.7f, 0.7f, 0.04f, 1.0f}, 0.078125f * 128),
    };

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
        onLongPress++;
        if(onLongPress > 2){
            onLongPress = 0;
        }
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
        perVertexperFragmentToggleSphere = !perVertexperFragmentToggleSphere;
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
        onSingleTap = onSingleTap + 1;
        if(onSingleTap > 23){
            onSingleTap = 0;
        }
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
            "in vec4 aPosition;\n" + 
            "in vec2 aTexCoord;\n" + 
            "out vec2 out_texCoord;\n" + 
            "uniform mat4 uMVPMatrix;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "gl_Position = uMVPMatrix * aPosition;\n" + 
            "out_texCoord = aTexCoord;\n" + 
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
            "out vec4 FragColor;\n" + 
            "in vec2 out_texCoord;\n" + 
            "uniform sampler2D uTextureSampler;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "FragColor = texture(uTextureSampler, out_texCoord);\n" + 
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
        shaderProgramObjectCube = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObjectCube, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObjectCube, fragmentShaderObject);
        GLES32.glBindAttribLocation(shaderProgramObjectCube, MyAttributes.AMC_ATTRIBUTE_POSITION_CUBE, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObjectCube, MyAttributes.AMC_ATTRIBUTE_TEXCOORD_CUBE, "aTexCoord");
        GLES32.glLinkProgram(shaderProgramObjectCube);
        GLES32.glGetProgramiv(shaderProgramObjectCube, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObjectCube, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObjectCube);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        mvpMatrixUniformCube = GLES32.glGetUniformLocation(shaderProgramObjectCube, "uMVPMatrix");
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObjectCube, "uTextureSampler");

        //Cube vertices
        //Cube vertices
        {
            final float cubeVertices[] = new float[]
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

            final float cubeTexcoords[] = new float[]
            {
                 // front
                1.0f, 1.0f, // top-right of front
                0.0f, 1.0f, // top-left of front
                0.0f, 0.0f, // bottom-left of front
                1.0f, 0.0f, // bottom-right of front

                // right
                1.0f, 1.0f, // top-right of right
                0.0f, 1.0f, // top-left of right
                0.0f, 0.0f, // bottom-left of right
                1.0f, 0.0f, // bottom-right of right

                // back
                1.0f, 1.0f, // top-right of back
                0.0f, 1.0f, // top-left of back
                0.0f, 0.0f, // bottom-left of back
                1.0f, 0.0f, // bottom-right of back

                // left
                1.0f, 1.0f, // top-right of left
                0.0f, 1.0f, // top-left of left
                0.0f, 0.0f, // bottom-left of left
                1.0f, 0.0f, // bottom-right of left

                // top
                1.0f, 1.0f, // top-right of top
                0.0f, 1.0f, // top-left of top
                0.0f, 0.0f, // bottom-left of top
                1.0f, 0.0f, // bottom-right of top

                // bottom
                1.0f, 1.0f, // top-right of bottom
                0.0f, 1.0f, // top-left of bottom
                0.0f, 0.0f, // bottom-left of bottom
                1.0f, 0.0f, // bottom-right of bottom
            };

            GLES32.glGenVertexArrays(1, vaoCube, 0);
            GLES32.glBindVertexArray(vaoCube[0]);
            {
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeVerticesBuffer = byteBuffer.asFloatBuffer();
                cubeVerticesBuffer.put(cubeVertices);
                cubeVerticesBuffer.position(0);
                GLES32.glGenBuffers(1, vboCubePosition, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCubePosition[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVerticesBuffer.capacity() * 4, cubeVerticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION_CUBE, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION_CUBE);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                byteBuffer = ByteBuffer.allocateDirect(cubeTexcoords.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer cubeTexcoordsBuffer = byteBuffer.asFloatBuffer();
                cubeTexcoordsBuffer.put(cubeTexcoords);
                cubeTexcoordsBuffer.position(0);
                GLES32.glGenBuffers(1, vboCubeTexcoord, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCubeTexcoord[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeTexcoordsBuffer.capacity() * 4, cubeTexcoordsBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD_CUBE, 2, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD_CUBE);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //Set the background frame color
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        GLES32.glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //Blue Screen

        Matrix.setIdentityM(perspectiveProjectionMatrixCube, 0);

        //Setup FBO
        if(createAndPrepareFBOForDrawing(fboWidth, fboHeight) == true){
            System.out.println("AKM: FBO created successfully");
            fboResult = initializeSphere(gl);

            if(fboResult != 0){
                System.out.println("AKM: InitializeSphere() failed with error code");
                return -1;
            }
            else{
                System.out.println("AKM: InitializeSphere() succeeded.");
            }
        }
        else{
            System.out.println("AKM: CreateAndPrepareFBOForDrawing() failed");
            return -1;
        }

        return 0;
    }

    private int initializeSphere(GL10 gl){
        // Vertex Shader
        final String pvVertexShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec4 aPosition;\n" + 
            "in vec3 aNormal;\n" + 
            "uniform mat4 uModelMatrix;\n" + 
            "uniform mat4 uViewMatrix;\n" + 
            "uniform mat4 uProjectionMatrix;\n" + 
            "uniform vec3 uLa[3];\n" + 
            "uniform vec3 uLd[3];\n" + 
            "uniform vec3 uLs[3];\n" + 
            "uniform vec4 uLightPosition[3];\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "out vec4 out_phong_ads_Light;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
            "   if(uLKeyPressed == 1)\n" + 
            "   {\n" + 
            "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n" + 
            "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" + 
            "       vec3 lightSource[3];\n" + 
            "       float tnDotLd[3];\n" + 
            "       vec3 reflectedVector[3];\n" + 
            "       vec3 ambient[3];\n" + 
            "       vec3 diffuse[3];\n" + 
            "       vec3 specular[3];\n" + 
            "       for(int i = 0; i < 3; i++)\n" + 
            "       {\n" + 
            "           lightSource[i] = normalize(vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n" + 
            "           tnDotLd[i] = max(dot(lightSource[i], transformedNormal), 0.0);\n" + 
            "           reflectedVector[i] = reflect(-lightSource[i], transformedNormal);\n" + 
            "           ambient[i] = uLa[i] * uKa;\n" + 
            "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n" + 
            "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], viewerVector), 0.0), uMaterialShininess);\n" + 
            "           out_phong_ads_Light = out_phong_ads_Light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n" + 
            "       }\n" + 
            "   }\n" + 
            "   else\n" + 
            "   {\n" + 
            "       out_phong_ads_Light = vec4(1.0, 1.0, 1.0, 1.0);\n" + 
            "   }\n" + 
            "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" + 
            "}\n"
        );
        int pvVertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(pvVertexShaderObject, pvVertexShaderSourceCode);
        GLES32.glCompileShader(pvVertexShaderObject);
        int[] iStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;
        GLES32.glGetShaderiv(pvVertexShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pvVertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pvVertexShaderObject);
                System.out.println("AKM: Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }


        //Fragment Shader
        final String pvFragmentShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec4 out_phong_ads_Light;\n" + 
            "out vec4 FragColor;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "FragColor = out_phong_ads_Light;\n" + 
            "}\n"
        );
        int pvFragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(pvFragmentShaderObject, pvFragmentShaderSourceCode);
        GLES32.glCompileShader(pvFragmentShaderObject);
        GLES32.glGetShaderiv(pvFragmentShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pvFragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pvFragmentShaderObject);
                System.out.println("AKM: Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program
        pvShaderProgramObjectSphere = GLES32.glCreateProgram();
        GLES32.glAttachShader(pvShaderProgramObjectSphere, pvVertexShaderObject);
        GLES32.glAttachShader(pvShaderProgramObjectSphere, pvFragmentShaderObject);
        GLES32.glBindAttribLocation(pvShaderProgramObjectSphere, MyAttributes.AMC_ATTRIBUTE_POSITION_SPHERE, "aPosition");
        GLES32.glBindAttribLocation(pvShaderProgramObjectSphere, MyAttributes.AMC_ATTRIBUTE_NORMAL_SPHERE, "aNormal");
        GLES32.glLinkProgram(pvShaderProgramObjectSphere);
        GLES32.glGetProgramiv(pvShaderProgramObjectSphere, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pvShaderProgramObjectSphere, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pvShaderProgramObjectSphere);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        pvModelMatrixUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uModelMatrix");
        pvViewMatrixUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uViewMatrix");
        pvProjectionMatrixUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uProjectionMatrix");

        pvLaUniformSphere[0] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLa[0]");
        pvLdUniformSphere[0] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLd[0]");
        pvLsUniformSphere[0] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLs[0]");
        pvLightPositionUniformSphere[0] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLightPosition[0]");

        pvLaUniformSphere[1] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLa[1]");
        pvLdUniformSphere[1] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLd[1]");
        pvLsUniformSphere[1] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLs[1]");
        pvLightPositionUniformSphere[1] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLightPosition[1]");

        pvLaUniformSphere[2] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLa[2]");
        pvLdUniformSphere[2] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLd[2]");
        pvLsUniformSphere[2] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLs[2]");
        pvLightPositionUniformSphere[2] = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLightPosition[2]");

        pvKaUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uKa");
        pvKdUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uKd");
        pvKsUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uKs");
        pvMaterialShininessUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uMaterialShininess");
        pvSingleTapUniformSphere = GLES32.glGetUniformLocation(pvShaderProgramObjectSphere, "uLKeyPressed");


        //Per fragment shaders
        final String pfVertexShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec4 aPosition;\n" + 
            "in vec3 aNormal;\n" + 
            "out vec4 eyeCoordinates;\n" + 
            "out vec3 transformedNormal;\n" + 
            "out vec3 lightSource[3];\n" + 
            "uniform mat4 uModelMatrix;\n" + 
            "uniform mat4 uViewMatrix;\n" + 
            "uniform mat4 uProjectionMatrix;\n" + 
            "uniform vec4 uLightPosition[3];\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "   transformedNormal = (normalMatrix * aNormal);\n" + 
            "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "   for(int i = 0; i < 3; i++)\n" + 
            "   {\n" + 
            "       lightSource[i] = (vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n" + 
            "   }\n" + 
            "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" + 
            "}\n"
        );
        int pfVertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(pfVertexShaderObject, pfVertexShaderSourceCode);
        GLES32.glCompileShader(pfVertexShaderObject);
        iStatus = new int[1];
        iInfoLogLength = new int[1];
        szInfoLog = null;
        GLES32.glGetShaderiv(pfVertexShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pfVertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pfVertexShaderObject);
                System.out.println("AKM: Vertex Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }


        //Fragment Shader
        final String pfFragmentShaderSourceCode = String.format
        (
            "#version 320 es\n" + 
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec3 transformedNormal;\n" + 
            "in vec4 eyeCoordinates;\n" + 
            "in vec3 lightSource[3];\n" + 
            "uniform vec3 uLa[3];\n" + 
            "uniform vec3 uLd[3];\n" + 
            "uniform vec3 uLs[3];\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "out vec4 FragColor;\n" + 
            "vec4 out_phong_ads_Light;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
            "   vec3 normalizedLightSource[3];\n" + 
            "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
            "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
            "   if(uLKeyPressed == 1)\n" + 
            "   {\n" + 
            "       float tnDotLd[3];\n" + 
            "       vec3 reflectedVector[3];\n" + 
            "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
            "       vec3 ambient[3];\n" + 
            "       vec3 diffuse[3];\n" + 
            "       vec3 specular[3];\n" + 
            "       for(int i = 0; i < 3; i++)\n" + 
            "       {\n" + 
            "           normalizedLightSource[i] = normalize(lightSource[i]);\n" + 
            "           tnDotLd[i] = max(dot(normalizedLightSource[i], normalizedTransformNormal), 0.0);\n" + 
            "           reflectedVector[i] = reflect(-normalizedLightSource[i], normalizedTransformNormal);\n" + 
            "           ambient[i] = uLa[i] * uKa;\n" + 
            "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n" + 
            "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], viewerVector), 0.0), uMaterialShininess);\n" + 
            "           out_phong_ads_Light = out_phong_ads_Light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n" + 
            "       }\n" + 
            "       FragColor = out_phong_ads_Light;\n" + 
            "   }\n" + 
            "   else\n" + 
            "   {\n" + 
            "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" + 
            "   }\n" + 
            "}\n"
        );
        int pfFragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(pfFragmentShaderObject, pfFragmentShaderSourceCode);
        GLES32.glCompileShader(pfFragmentShaderObject);
        GLES32.glGetShaderiv(pfFragmentShaderObject, GLES32.GL_COMPILE_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(pfFragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(pfFragmentShaderObject);
                System.out.println("AKM: Fragment Shader Compilation Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //Shader Program
        pfShaderProgramObjectSphere = GLES32.glCreateProgram();
        GLES32.glAttachShader(pfShaderProgramObjectSphere, pfVertexShaderObject);
        GLES32.glAttachShader(pfShaderProgramObjectSphere, pfFragmentShaderObject);
        GLES32.glBindAttribLocation(pfShaderProgramObjectSphere, MyAttributes.AMC_ATTRIBUTE_POSITION_SPHERE, "aPosition");
        GLES32.glBindAttribLocation(pfShaderProgramObjectSphere, MyAttributes.AMC_ATTRIBUTE_NORMAL_SPHERE, "aNormal");
        GLES32.glLinkProgram(pfShaderProgramObjectSphere);
        GLES32.glGetProgramiv(pfShaderProgramObjectSphere, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pfShaderProgramObjectSphere, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pfShaderProgramObjectSphere);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        pfModelMatrixUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uModelMatrix");
        pfViewMatrixUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uViewMatrix");
        pfProjectionMatrixUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uProjectionMatrix");

        pfLaUniformSphere[0] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLa[0]");
        pfLdUniformSphere[0] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLd[0]");
        pfLsUniformSphere[0] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLs[0]");
        pfLightPositionUniformSphere[0] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLightPosition[0]");

        pfLaUniformSphere[1] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLa[1]");
        pfLdUniformSphere[1] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLd[1]");
        pfLsUniformSphere[1] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLs[1]");
        pfLightPositionUniformSphere[1] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLightPosition[1]");

        pfLaUniformSphere[2] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLa[2]");
        pfLdUniformSphere[2] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLd[2]");
        pfLsUniformSphere[2] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLs[2]");
        pfLightPositionUniformSphere[2] = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLightPosition[2]");

        pfKaUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uKa");
        pfKdUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uKd");
        pfKsUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uKs");
        pfMaterialShininessUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uMaterialShininess");
        pfSingleTapUniformSphere = GLES32.glGetUniformLocation(pfShaderProgramObjectSphere, "uLKeyPressed");

        //Sphere data
        Sphere sphere = new Sphere();
        float sphere_vertices[] = new float[1146];
        float sphere_normals[]  = new float[1146];
        float sphere_textures[] = new float[764];
        short sphere_elements[] = new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVerticesSphere = sphere.getNumberOfSphereVertices();
        numElementsSphere = sphere.getNumberOfSphereElements();

        //Setup vao and vbo for sphere
        GLES32.glGenVertexArrays(1, vao_sphere, 0);
        GLES32.glBindVertexArray(vao_sphere[0]);
        {
            // position vbo
            GLES32.glGenBuffers(1, vbo_sphere_position,0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphere_position[0]);
            
            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphere_vertices.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
            verticesBuffer.put(sphere_vertices);
            verticesBuffer.position(0);
            
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphere_vertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION_SPHERE, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION_SPHERE);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
            
            // normal vbo
            GLES32.glGenBuffers(1,vbo_sphere_normal,0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);
            
            byteBuffer = ByteBuffer.allocateDirect(sphere_normals.length * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            verticesBuffer=byteBuffer.asFloatBuffer();
            verticesBuffer.put(sphere_normals);
            verticesBuffer.position(0);
            
            GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphere_normals.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL_SPHERE, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL_SPHERE);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
            
            // element vbo
            GLES32.glGenBuffers(1,vbo_sphere_element,0);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);
            
            byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
            byteBuffer.order(ByteOrder.nativeOrder());
            ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
            elementsBuffer.put(sphere_elements);
            elementsBuffer.position(0);
            
            GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER, sphere_elements.length * 2, elementsBuffer, GLES32.GL_STATIC_DRAW);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);
        }
        GLES32.glBindVertexArray(0);

        //Set the background frame color
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        resizeSphere(fboWidth, fboHeight);

        return 0;
    }

    private void resize(int width, int height)
    {
        //Adjust the viewport based on geometry changes, such as screen rotation

        winWidth = width;
        winHeight = height;

        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(perspectiveProjectionMatrixCube, 0, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

    private void resizeSphere(int width, int height)
    {
        //Adjust the viewport based on geometry changes, such as screen rotation
        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(perspectiveProjectionMatrixSphere, 0, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

    private boolean createAndPrepareFBOForDrawing(int textureWidth, int textureHeight) {
        int maxTexSize[] = new int[1];
        GLES32.glGetIntegerv(GLES32.GL_MAX_TEXTURE_SIZE, maxTexSize, 0);
        if (textureWidth > maxTexSize[0] || textureHeight > maxTexSize[0]) {
            System.out.println("AKM: Requested texture size" + textureWidth + "X" + textureHeight + "exceeds max texture size" + maxTexSize[0]);
            return false;
        }

        GLES32.glGenFramebuffers(1, fbo, 0);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo[0]);

        GLES32.glGenRenderbuffers(1, rbo, 0);
        GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER, rbo[0]);
        GLES32.glRenderbufferStorage(GLES32.GL_RENDERBUFFER, GLES32.GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

        GLES32.glGenTextures(1, fboTexture, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, fboTexture[0]);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_S, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_T, GLES32.GL_CLAMP_TO_EDGE);

        GLES32.glTexImage2D(
            GLES32.GL_TEXTURE_2D,   // target
            0,                      // level
            GLES32.GL_RGBA,         // internal format
            textureWidth,           // width
            textureHeight,          // height
            0,                      // border (must be 0 in ES)
            GLES32.GL_RGBA,         // format
            GLES32.GL_UNSIGNED_BYTE,// type
            null                    // data pointer -> no data (allocate only)
        );

        GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER, GLES32.GL_COLOR_ATTACHMENT0, GLES32.GL_TEXTURE_2D, fboTexture[0], 0);
        GLES32.glFramebufferRenderbuffer(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, GLES32.GL_RENDERBUFFER, rbo[0]);

        int status = GLES32.glCheckFramebufferStatus(GLES32.GL_FRAMEBUFFER);
        if (status != GLES32.GL_FRAMEBUFFER_COMPLETE) {
            System.out.println("AKM: Framebuffer is not complete: " +  status);
            // optionally query glGetError() here
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
            GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER, 0);
            GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
            return false;
        }

        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
        GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER, 0);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);

        return true;
    }

    private void display()
    {

        if(fboResult == 0){
            displaySphere();
        }

        resize(winWidth, winHeight);
        GLES32.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        //Redraw background color
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];
        float translationMatrix[] = new float[16];
        float rotationMatrix[] = new float[16];
        float scaleMatrix[] = new float[16];

        //Use shader program object
        GLES32.glUseProgram(shaderProgramObjectCube);
        {
            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setIdentityM(scaleMatrix, 0);

            //Cube
            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -6.0f);
            Matrix.multiplyMM(translationMatrix, 0, translationMatrix, 0, scaleMatrix, 0);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 1.0f, 0.0f, 0.0f);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 0.0f, 1.0f, 0.0f);
            Matrix.rotateM(rotationMatrix, 0, angleCube, 0.0f, 0.0f, 1.0f);
            Matrix.multiplyMM(modelViewMatrix, 0, translationMatrix, 0, rotationMatrix, 0);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrixCube, 0, modelViewMatrix, 0);

            GLES32.glUniformMatrix4fv(mvpMatrixUniformCube, 1, false, modelViewProjectionMatrix, 0);
            GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, fbo[0]);
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

    private void displaySphere(){
        if(fbo[0] != 0){
            GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo[0]);
        }

        resizeSphere(fboWidth, fboHeight);
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        //Redraw background color
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        float modelMatrix[] = new float[16];
        float viewMatrix[] = new float[16];

        //Use shader program object
        if(perVertexperFragmentToggleSphere){
            GLES32.glUseProgram(pfShaderProgramObjectSphere);
        }
        else{
            GLES32.glUseProgram(pvShaderProgramObjectSphere);
        }
        {
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);
            {
                if(perVertexperFragmentToggleSphere){
                    float lightRotationMatrix[] = new float[16];
                    float lightTranslationMatrix[] = new float[16];
                    float lightTransformMatrix[] = new float[16];
                    float[] result = new float[4];

                    Matrix.setIdentityM(lightRotationMatrix, 0);
                    Matrix.setIdentityM(lightTranslationMatrix, 0);
                    Matrix.setIdentityM(lightTransformMatrix, 0);

                    Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);
                    
                    GLES32.glUniformMatrix4fv(pfModelMatrixUniformSphere, 1, false, modelMatrix, 0);
                    GLES32.glUniformMatrix4fv(pfViewMatrixUniformSphere, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pfProjectionMatrixUniformSphere, 1, false, perspectiveProjectionMatrixSphere, 0);

                    GLES32.glUniform3fv(pfLaUniformSphere[0], 1, lightAmbientSphere, 0);
                    GLES32.glUniform3fv(pfLdUniformSphere[0], 1, lightDiffuseSphere, 0);
                    GLES32.glUniform3fv(pfLsUniformSphere[0], 1, lightSpecularSphere, 0);

                    if(onLongPress == 0){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForXRotationSphere, 1.0f, 0.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    }
                    else if(onLongPress == 1){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForYRotationSphere, 0.0f, 1.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 20.0f, 0.0f, 0.0f);
                    }
                    else if(onLongPress == 2){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForZRotationSphere, 0.0f, 0.0f, 1.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 20.0f, 0.0f);
                    }
                    
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lightPositionSphere, 0);

                    GLES32.glUniform4fv(pfLightPositionUniformSphere[0], 1, result, 0);

                    GLES32.glUniform3fv(pfKaUniformSphere, 1, materials[onSingleTap].ambient, 0);
                    GLES32.glUniform3fv(pfKdUniformSphere, 1, materials[onSingleTap].diffuse, 0);
                    GLES32.glUniform3fv(pfKsUniformSphere, 1, materials[onSingleTap].specular, 0);

                    GLES32.glUniform1f(pfMaterialShininessUniformSphere, materials[onSingleTap].shininess);
                    GLES32.glUniform1i(pfSingleTapUniformSphere, bLightSphere ? 1 : 0);

                    GLES32.glBindVertexArray(vao_sphere[0]);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElementsSphere, GLES32.GL_UNSIGNED_SHORT, 0);
                    GLES32.glBindVertexArray(0);
                }
                else{
                    float lightRotationMatrix[] = new float[16];
                    float lightTranslationMatrix[] = new float[16];
                    float lightTransformMatrix[] = new float[16];
                    float[] result = new float[4];

                    Matrix.setIdentityM(lightRotationMatrix, 0);
                    Matrix.setIdentityM(lightTranslationMatrix, 0);
                    Matrix.setIdentityM(lightTransformMatrix, 0);

                    Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);
                    
                    GLES32.glUniformMatrix4fv(pvModelMatrixUniformSphere, 1, false, modelMatrix, 0);
                    GLES32.glUniformMatrix4fv(pvViewMatrixUniformSphere, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pvProjectionMatrixUniformSphere, 1, false, perspectiveProjectionMatrixSphere, 0);

                    GLES32.glUniform3fv(pvLaUniformSphere[0], 1, lightAmbientSphere, 0);
                    GLES32.glUniform3fv(pvLdUniformSphere[0], 1, lightDiffuseSphere, 0);
                    GLES32.glUniform3fv(pvLsUniformSphere[0], 1, lightSpecularSphere, 0);

                    if(onLongPress == 0){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForXRotationSphere, 1.0f, 0.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    }
                    else if(onLongPress == 1){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForYRotationSphere, 0.0f, 1.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 20.0f, 0.0f, 0.0f);
                    }
                    else if(onLongPress == 2){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForZRotationSphere, 0.0f, 0.0f, 1.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 20.0f, 0.0f);
                    }
                    
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lightPositionSphere, 0);

                    GLES32.glUniform4fv(pvLightPositionUniformSphere[0], 1, result, 0);
                    
                    GLES32.glUniform3fv(pvKaUniformSphere, 1, materials[onSingleTap].ambient, 0);
                    GLES32.glUniform3fv(pvKdUniformSphere, 1, materials[onSingleTap].diffuse, 0);
                    GLES32.glUniform3fv(pvKsUniformSphere, 1, materials[onSingleTap].specular, 0);

                    GLES32.glUniform1f(pvMaterialShininessUniformSphere, materials[onSingleTap].shininess);
                    GLES32.glUniform1i(pvSingleTapUniformSphere, bLightSphere ? 1 : 0);

                    GLES32.glBindVertexArray(vao_sphere[0]);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElementsSphere, GLES32.GL_UNSIGNED_SHORT, 0);
                    GLES32.glBindVertexArray(0);
                }
            }
        }
        //Unuse program
        GLES32.glUseProgram(0);

        if(fbo[0] != 0){
            GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
        }
    }

    private void update()
    {
        //Code
        angleCube += 2.0f;
        angleForXRotationSphere = angleForXRotationSphere + 3.0f;
        angleForYRotationSphere = angleForYRotationSphere + 3.0f;
        angleForZRotationSphere = angleForZRotationSphere + 3.0f;
    }

    private void uninitialize()
    {

        if(fboResult == 0){
            uninitializeSphere();
        }
        //Code
        if(vboCubePosition[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboCubePosition, 0);
            vboCubePosition[0] = 0;
        }
        if(vaoCube[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoCube, 0);
            vaoCube[0] = 0;
        }
        if(shaderProgramObjectCube != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(shaderProgramObjectCube);
            GLES32.glGetProgramiv(shaderProgramObjectCube, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(shaderProgramObjectCube, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(shaderProgramObjectCube, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(shaderProgramObjectCube);
            shaderProgramObjectCube = 0;
        }
    }

    private void uninitializeSphere(){
        //Code
        if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }
        if(vbo_sphere_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }
        if(vbo_sphere_normal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }
        if(vbo_sphere_element[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
        }
        if(pvShaderProgramObjectSphere != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(pvShaderProgramObjectSphere);
            GLES32.glGetProgramiv(pvShaderProgramObjectSphere, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(pvShaderProgramObjectSphere, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(pvShaderProgramObjectSphere, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(pvShaderProgramObjectSphere);
            pvShaderProgramObjectSphere = 0;
        }
        if(pfShaderProgramObjectSphere != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(pfShaderProgramObjectSphere);
            GLES32.glGetProgramiv(pfShaderProgramObjectSphere, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(pfShaderProgramObjectSphere, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(pfShaderProgramObjectSphere, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(pfShaderProgramObjectSphere);
            pfShaderProgramObjectSphere = 0;
        }

        if(rbo[0] != 0){
            GLES32.glDeleteRenderbuffers(1, rbo, 0);
            rbo[0] = 0;
        }

        if(fbo[0] != 0){
            GLES32.glDeleteFramebuffers(1, fbo, 0);
            fbo[0] = 0;
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
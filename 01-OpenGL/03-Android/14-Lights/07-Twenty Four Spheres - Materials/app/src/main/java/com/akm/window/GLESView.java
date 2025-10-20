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

    //OpenGL related variables
    private int pvShaderProgramObject;
    private int pfShaderProgramObject;

    private float perspectiveProjectionMatrix[] = new float[16];

    private int pvModelMatrixUniform;
    private int pvViewMatrixUniform;
    private int pvProjectionMatrixUniform;

    private int pfModelMatrixUniform;
    private int pfViewMatrixUniform;
    private int pfProjectionMatrixUniform;

    //Sphere related variables
    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];
    private int numVertices;
    private int numElements;

    // Light Settings
    private int pvLaUniform = 0;
    private int pvLdUniform = 0;
    private int pvLsUniform = 0;
    private int pvLightPositionUniform = 0;

    private int pvKaUniform = 0;
    private int pvKdUniform = 0;
    private int pvKsUniform = 0;
    private int pvMaterialShininessUniform = 0;

    private int pvSingleTapUniform = 0;

    private int pfLaUniform = 0;
    private int pfLdUniform = 0;
    private int pfLsUniform = 0;
    private int pfLightPositionUniform = 0;

    private int pfKaUniform = 0;
    private int pfKdUniform = 0;
    private int pfKsUniform = 0;
    private int pfMaterialShininessUniform = 0;

    private int pfSingleTapUniform = 0;

    private boolean bLight = false;
    private boolean perVertexperFragmentToggle = false;

    private float lightAmbient[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightDiffuse[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightSpecular[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float lightPosition[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};

    private float angleForXRotation = 0.0f;
    private float angleForYRotation = 0.0f;
    private float angleForZRotation = 0.0f;

    private int onLongPress = 0;

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

    float[][] sphereTranslation = new float[][] {
        {1.5f, 14.0f, -20.0f},
        {1.5f, 11.5f, -20.0f},
        {1.5f, 9.0f, -20.0f},
        {1.5f, 6.5f, -20.0f},
        {1.5f, 4.0f, -20.0f},
        {1.5f, 1.5f, -20.0f},

        {7.5f, 14.0f, -20.0f},
        {7.5f, 11.5f, -20.0f},
        {7.5f, 9.0f, -20.0f},
        {7.5f, 6.5f, -20.0f},
        {7.5f, 4.0f, -20.0f},
        {7.5f, 1.5f, -20.0f},

        {13.5f, 14.0f, -20.0f},
        {13.5f, 11.5f, -20.0f},
        {13.5f, 9.0f, -20.0f},
        {13.5f, 6.5f, -20.0f},
        {13.5f, 4.0f, -20.0f},
        {13.5f, 1.5f, -20.0f},

        {19.5f, 14.0f, -20.0f},
        {19.5f, 11.5f, -20.0f},
        {19.5f, 9.0f, -20.0f},
        {19.5f, 6.5f, -20.0f},
        {19.5f, 4.0f, -20.0f},
        {19.5f, 1.5f, -20.0f},
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
        onLongPress = onLongPress + 1;
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
        perVertexperFragmentToggle = !perVertexperFragmentToggle;
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
            "uniform vec3 uLa;\n" + 
            "uniform vec3 uLd;\n" + 
            "uniform vec3 uLs;\n" + 
            "uniform vec4 uLightPosition;\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "out vec4 out_phong_ads_Light;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   if(uLKeyPressed == 1)\n" + 
            "   {\n" + 
            "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n" + 
            "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n" + 
            "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n" + 
            "       vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n" + 
            "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" + 
            "       vec3 ambient = uLa * uKa;\n" + 
            "       vec3 diffuse = uLd * uKd * tnDotLd;\n" + 
            "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" + 
            "       out_phong_ads_Light = vec4(ambient + diffuse + specular, 1.0);\n" + 
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
        pvShaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(pvShaderProgramObject, pvVertexShaderObject);
        GLES32.glAttachShader(pvShaderProgramObject, pvFragmentShaderObject);
        GLES32.glBindAttribLocation(pvShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(pvShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glLinkProgram(pvShaderProgramObject);
        GLES32.glGetProgramiv(pvShaderProgramObject, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pvShaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pvShaderProgramObject);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        pvModelMatrixUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uModelMatrix");
        pvViewMatrixUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uViewMatrix");
        pvProjectionMatrixUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uProjectionMatrix");

        pvLaUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLa");
        pvLdUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLd");
        pvLsUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLs");
        pvKaUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKa");
        pvKdUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKd");
        pvKsUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKs");
        pvMaterialShininessUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uMaterialShininess");
        pvLightPositionUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLightPosition");
        pvSingleTapUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLKeyPressed");


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
            "out vec3 lightSource;\n" + 
            "uniform mat4 uModelMatrix;\n" + 
            "uniform mat4 uViewMatrix;\n" + 
            "uniform mat4 uProjectionMatrix;\n" + 
            "uniform vec4 uLightPosition;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "   transformedNormal = (normalMatrix * aNormal);\n" + 
            "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" + 
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
            "in vec3 lightSource;\n" + 
            "uniform vec3 uLa;\n" + 
            "uniform vec3 uLd;\n" + 
            "uniform vec3 uLs;\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "out vec4 FragColor;\n" + 
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
            "       FragColor = vec4(ambient + diffuse + specular, 1.0);\n" + 
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
        pfShaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(pfShaderProgramObject, pfVertexShaderObject);
        GLES32.glAttachShader(pfShaderProgramObject, pfFragmentShaderObject);
        GLES32.glBindAttribLocation(pfShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(pfShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glLinkProgram(pfShaderProgramObject);
        GLES32.glGetProgramiv(pfShaderProgramObject, GLES32.GL_LINK_STATUS, iStatus, 0);
        if(iStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(pfShaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(pfShaderProgramObject);
                System.out.println("AKM: Shader Program Linking Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        pfModelMatrixUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uModelMatrix");
        pfViewMatrixUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uViewMatrix");
        pfProjectionMatrixUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uProjectionMatrix");

        pfLaUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLa");
        pfLdUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLd");
        pfLsUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLs");
        pfKaUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKa");
        pfKdUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKd");
        pfKsUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKs");
        pfMaterialShininessUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uMaterialShininess");
        pfLightPositionUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLightPosition");
        pfSingleTapUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLKeyPressed");

        //Sphere data
        Sphere sphere = new Sphere();
        float sphere_vertices[] = new float[1146];
        float sphere_normals[]  = new float[1146];
        float sphere_textures[] = new float[764];
        short sphere_elements[] = new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

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
            GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
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
            GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
            GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);
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

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Blue Screen

        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
        return 0;
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

        //Use shader program object
        if(perVertexperFragmentToggle){
            GLES32.glUseProgram(pfShaderProgramObject);
        }
        else{
            GLES32.glUseProgram(pvShaderProgramObject);
        }
        {
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);


            float lightRotationMatrix[] = new float[16];
            float lightTranslationMatrix[] = new float[16];
            float lightTransformMatrix[] = new float[16];
            float finalLightTransformMatrix[] = new float[16];
            float[] result = new float[]{0.0f, 0.0f, 0.0f, 1.0f};

            Matrix.setIdentityM(lightRotationMatrix, 0);
            Matrix.setIdentityM(lightTranslationMatrix, 0);
            Matrix.setIdentityM(lightTransformMatrix, 0);
            Matrix.setIdentityM(finalLightTransformMatrix, 0);

            
            {
                if(perVertexperFragmentToggle){
                    Matrix.translateM(modelMatrix, 0, -10.0f, -8.0f, 0.0f);
                    GLES32.glUniformMatrix4fv(pfViewMatrixUniform, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pfProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

                    GLES32.glUniform3fv(pfLaUniform, 1, lightAmbient, 0);
                    GLES32.glUniform3fv(pfLdUniform, 1, lightDiffuse, 0);
                    GLES32.glUniform3fv(pfLsUniform, 1, lightSpecular, 0);
                    GLES32.glUniform1i(pfSingleTapUniform, bLight ? 1 : 0);

                    if(onLongPress == 0){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForXRotation, 1.0f, 0.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    }
                    else if(onLongPress == 1){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForYRotation, 0.0f, 1.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 20f, 0.0f, 0.0f);
                    }
                    else if(onLongPress == 2){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForZRotation, 0.0f, 0.0f, 1.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0f, 20.0f, 0.0f);
                    }

                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);

                    for(int i = 0; i < 24; i++){
                        float eachSphereTranslation[] = new float[16];
                        Matrix.setIdentityM(eachSphereTranslation, 0);
                        Matrix.translateM(eachSphereTranslation, 0, sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]);
                        Matrix.multiplyMM(eachSphereTranslation, 0, eachSphereTranslation, 0, modelMatrix, 0);
                        GLES32.glUniformMatrix4fv(pfModelMatrixUniform, 1, false, eachSphereTranslation, 0);
                        
                        Matrix.multiplyMM(finalLightTransformMatrix, 0, eachSphereTranslation, 0, lightTransformMatrix, 0);
                        Matrix.multiplyMV(result, 0, finalLightTransformMatrix, 0, lightPosition, 0);

                        GLES32.glUniform4fv(pfLightPositionUniform, 1, result, 0);
                        GLES32.glUniform3fv(pfKaUniform, 1, materials[i].ambient, 0);
                        GLES32.glUniform3fv(pfKdUniform, 1, materials[i].diffuse, 0);
                        GLES32.glUniform3fv(pfKsUniform, 1, materials[i].specular, 0);
                        GLES32.glUniform1f(pfMaterialShininessUniform, materials[i].shininess);

                        GLES32.glBindVertexArray(vao_sphere[0]);
                            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                            GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                        GLES32.glBindVertexArray(0);
                    }
                }
                else{
                    Matrix.translateM(modelMatrix, 0, -10.0f, -8.0f, 0.0f);
                    GLES32.glUniformMatrix4fv(pvViewMatrixUniform, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pvProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

                    GLES32.glUniform3fv(pvLaUniform, 1, lightAmbient, 0);
                    GLES32.glUniform3fv(pvLdUniform, 1, lightDiffuse, 0);
                    GLES32.glUniform3fv(pvLsUniform, 1, lightSpecular, 0);
                    GLES32.glUniform1i(pvSingleTapUniform, bLight ? 1 : 0);

                    if(onLongPress == 0){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForXRotation, 1.0f, 0.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    }
                    else if(onLongPress == 1){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForYRotation, 0.0f, 1.0f, 0.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 20f, 0.0f, 0.0f);
                    }
                    else if(onLongPress == 2){
                        Matrix.rotateM(lightRotationMatrix, 0, angleForZRotation, 0.0f, 0.0f, 1.0f);
                        Matrix.translateM(lightTranslationMatrix, 0, 0f, 20.0f, 0.0f);
                    }

                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);

                    for(int i = 0; i < 24; i++){
                        float eachSphereTranslation[] = new float[16];
                        Matrix.setIdentityM(eachSphereTranslation, 0);
                        Matrix.translateM(eachSphereTranslation, 0, sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]);
                        Matrix.multiplyMM(eachSphereTranslation, 0, eachSphereTranslation, 0, modelMatrix, 0);
                        GLES32.glUniformMatrix4fv(pvModelMatrixUniform, 1, false, eachSphereTranslation, 0);
                        
                        Matrix.multiplyMM(finalLightTransformMatrix, 0, eachSphereTranslation, 0, lightTransformMatrix, 0);
                        Matrix.multiplyMV(result, 0, finalLightTransformMatrix, 0, lightPosition, 0);

                        GLES32.glUniform4fv(pvLightPositionUniform, 1, result, 0);
                        GLES32.glUniform3fv(pvKaUniform, 1, materials[i].ambient, 0);
                        GLES32.glUniform3fv(pvKdUniform, 1, materials[i].diffuse, 0);
                        GLES32.glUniform3fv(pvKsUniform, 1, materials[i].specular, 0);
                        GLES32.glUniform1f(pvMaterialShininessUniform, materials[i].shininess);

                        GLES32.glBindVertexArray(vao_sphere[0]);
                            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                            GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                        GLES32.glBindVertexArray(0);
                    }
                }
            }
        }
        
        //Unuse program
        GLES32.glUseProgram(0);

        //Render the view
        requestRender(); //equivalent to swapBuffers()
    }

    private void update()
    {
        //Code
        angleForXRotation = angleForXRotation + 5.0f;
        angleForYRotation = angleForYRotation + 5.0f;
        angleForZRotation = angleForZRotation + 5.0f;
    }

    private void uninitialize()
    {
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
        if(pvShaderProgramObject != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(pvShaderProgramObject);
            GLES32.glGetProgramiv(pvShaderProgramObject, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(pvShaderProgramObject, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(pvShaderProgramObject, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(pvShaderProgramObject);
            pvShaderProgramObject = 0;
        }
        if(pfShaderProgramObject != 0)
        {
            int[] shaderCount = new int[1];
            GLES32.glUseProgram(pfShaderProgramObject);
            GLES32.glGetProgramiv(pfShaderProgramObject, GLES32.GL_ATTACHED_SHADERS, shaderCount, 0);
            int[] shaders = new int[shaderCount[0]];
            GLES32.glGetAttachedShaders(pfShaderProgramObject, shaderCount[0], shaderCount, 0, shaders, 0);
            for(int i = 0; i < shaderCount[0]; i++)
            {
                GLES32.glDetachShader(pfShaderProgramObject, shaders[i]);
                GLES32.glDeleteShader(shaders[i]);
                shaders[i] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(pfShaderProgramObject);
            pfShaderProgramObject = 0;
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
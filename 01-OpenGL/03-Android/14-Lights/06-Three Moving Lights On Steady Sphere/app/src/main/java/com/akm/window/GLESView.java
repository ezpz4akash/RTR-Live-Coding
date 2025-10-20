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
    private int pvLaUniform[] = new int[3];
    private int pvLdUniform[] = new int[3];
    private int pvLsUniform[] = new int[3];
    private int pvLightPositionUniform[] = new int[3];

    private int pvKaUniform = 0;
    private int pvKdUniform = 0;
    private int pvKsUniform = 0;
    private int pvMaterialShininessUniform = 0;

    private int pvSingleTapUniform = 0;

    private int pfLaUniform[] = new int[3];
    private int pfLdUniform[] = new int[3];
    private int pfLsUniform [] = new int[3];
    private int pfLightPositionUniform[] = new int[3];

    private int pfKaUniform = 0;
    private int pfKdUniform = 0;
    private int pfKsUniform = 0;
    private int pfMaterialShininessUniform = 0;

    private int pfSingleTapUniform = 0;

    private boolean bLight = true;
    private boolean perVertexperFragmentToggle = false;

    Light lights[] = new Light[3];

    private float materialAmbient[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
    private float materialDiffuse[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialSpecular[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;

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

        pvLaUniform[0] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLa[0]");
        pvLdUniform[0] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLd[0]");
        pvLsUniform[0] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLs[0]");
        pvLightPositionUniform[0] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLightPosition[0]");

        pvLaUniform[1] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLa[1]");
        pvLdUniform[1] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLd[1]");
        pvLsUniform[1] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLs[1]");
        pvLightPositionUniform[1] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLightPosition[1]");

        pvLaUniform[2] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLa[2]");
        pvLdUniform[2] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLd[2]");
        pvLsUniform[2] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLs[2]");
        pvLightPositionUniform[2] = GLES32.glGetUniformLocation(pvShaderProgramObject, "uLightPosition[2]");

        pvKaUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKa");
        pvKdUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKd");
        pvKsUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uKs");
        pvMaterialShininessUniform = GLES32.glGetUniformLocation(pvShaderProgramObject, "uMaterialShininess");
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
            "out vec4 out_phong_ads_Light;\n" + 
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

        pfLaUniform[0] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLa[0]");
        pfLdUniform[0] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLd[0]");
        pfLsUniform[0] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLs[0]");
        pfLightPositionUniform[0] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLightPosition[0]");

        pfLaUniform[1] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLa[1]");
        pfLdUniform[1] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLd[1]");
        pfLsUniform[1] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLs[1]");
        pfLightPositionUniform[1] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLightPosition[1]");

        pfLaUniform[2] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLa[2]");
        pfLdUniform[2] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLd[2]");
        pfLsUniform[2] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLs[2]");
        pfLightPositionUniform[2] = GLES32.glGetUniformLocation(pfShaderProgramObject, "uLightPosition[2]");

        pfKaUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKa");
        pfKdUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKd");
        pfKsUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uKs");
        pfMaterialShininessUniform = GLES32.glGetUniformLocation(pfShaderProgramObject, "uMaterialShininess");
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


        //Lights
        lights[0] = new Light();
        lights[0].ambient       = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[0].diffuse       = new float[] {1.0f, 0.0f, 0.0f, 1.0f};
        lights[0].specular      = new float[] {1.0f, 0.0f, 0.0f, 1.0f};
        lights[0].position      = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[0].lightAngle    = 0.0f;

        lights[1] = new Light();
        lights[1].ambient       = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[1].diffuse       = new float[] {0.0f, 1.0f, 0.0f, 1.0f};
        lights[1].specular      = new float[] {0.0f, 1.0f, 0.0f, 1.0f};
        lights[1].position      = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[1].lightAngle    = 0.0f;

        lights[2] = new Light();
        lights[2].ambient       = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[2].diffuse       = new float[] {0.0f, 0.0f, 1.0f, 1.0f};
        lights[2].specular      = new float[] {0.0f, 0.0f, 1.0f, 1.0f};
        lights[2].position      = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[2].lightAngle    = 0.0f;

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

            GLES32.glBindVertexArray(vao_sphere[0]);
            {
                if(perVertexperFragmentToggle){
                    float lightRotationMatrix[] = new float[16];
                    float lightTranslationMatrix[] = new float[16];
                    float lightTransformMatrix[] = new float[16];
                    float[] result = new float[4];

                    Matrix.setIdentityM(lightRotationMatrix, 0);
                    Matrix.setIdentityM(lightTranslationMatrix, 0);
                    Matrix.setIdentityM(lightTransformMatrix, 0);

                    Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);
                    
                    GLES32.glUniformMatrix4fv(pfModelMatrixUniform, 1, false, modelMatrix, 0);
                    GLES32.glUniformMatrix4fv(pfViewMatrixUniform, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pfProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

                    // Light[0]
                    GLES32.glUniform3fv(pfLaUniform[0], 1, lights[0].ambient, 0);
                    GLES32.glUniform3fv(pfLdUniform[0], 1, lights[0].diffuse, 0);
                    GLES32.glUniform3fv(pfLsUniform[0], 1, lights[0].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[0].lightAngle, 0.0f, 1.0f, 0.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[0].position, 0);
                    GLES32.glUniform4fv(pfLightPositionUniform[0], 1, result, 0);

                    // Light[1]
                    GLES32.glUniform3fv(pfLaUniform[1], 1, lights[1].ambient, 0);
                    GLES32.glUniform3fv(pfLdUniform[1], 1, lights[1].diffuse, 0);
                    GLES32.glUniform3fv(pfLsUniform[1], 1, lights[1].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[1].lightAngle, 1.0f, 0.0f, 0.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 20.0f, 0.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[1].position, 0);
                    GLES32.glUniform4fv(pfLightPositionUniform[1], 1, result, 0);

                    // Light[2]
                    GLES32.glUniform3fv(pfLaUniform[2], 1, lights[2].ambient, 0);
                    GLES32.glUniform3fv(pfLdUniform[2], 1, lights[2].diffuse, 0);
                    GLES32.glUniform3fv(pfLsUniform[2], 1, lights[2].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[2].lightAngle, 0.0f, 0.0f, 1.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 20.0f, 0.0f, 0.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[1].position, 0);
                    GLES32.glUniform4fv(pfLightPositionUniform[2], 1, result, 0);

                    GLES32.glUniform3fv(pfKaUniform, 1, materialAmbient, 0);
                    GLES32.glUniform3fv(pfKdUniform, 1, materialDiffuse, 0);
                    GLES32.glUniform3fv(pfKsUniform, 1, materialSpecular, 0);

                    GLES32.glUniform1f(pfMaterialShininessUniform, materialShininess);
                    GLES32.glUniform1i(pfSingleTapUniform, bLight ? 1 : 0);

                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
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
                    
                    GLES32.glUniformMatrix4fv(pvModelMatrixUniform, 1, false, modelMatrix, 0);
                    GLES32.glUniformMatrix4fv(pvViewMatrixUniform, 1, false, viewMatrix, 0);
                    GLES32.glUniformMatrix4fv(pvProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

                    // Light[0]
                    GLES32.glUniform3fv(pvLaUniform[0], 1, lights[0].ambient, 0);
                    GLES32.glUniform3fv(pvLdUniform[0], 1, lights[0].diffuse, 0);
                    GLES32.glUniform3fv(pvLsUniform[0], 1, lights[0].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[0].lightAngle, 0.0f, 1.0f, 0.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 0.0f, 20.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[0].position, 0);
                    GLES32.glUniform4fv(pvLightPositionUniform[0], 1, result, 0);

                    // Light[1]
                    GLES32.glUniform3fv(pvLaUniform[1], 1, lights[1].ambient, 0);
                    GLES32.glUniform3fv(pvLdUniform[1], 1, lights[1].diffuse, 0);
                    GLES32.glUniform3fv(pvLsUniform[1], 1, lights[1].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[1].lightAngle, 1.0f, 0.0f, 0.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 0.0f, 20.0f, 0.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[1].position, 0);
                    GLES32.glUniform4fv(pvLightPositionUniform[1], 1, result, 0);

                    // Light[2]
                    GLES32.glUniform3fv(pvLaUniform[2], 1, lights[2].ambient, 0);
                    GLES32.glUniform3fv(pvLdUniform[2], 1, lights[2].diffuse, 0);
                    GLES32.glUniform3fv(pvLsUniform[2], 1, lights[2].specular, 0);

                    Matrix.rotateM(lightRotationMatrix, 0, lights[2].lightAngle, 0.0f, 0.0f, 1.0f);
                    Matrix.translateM(lightTranslationMatrix, 0, 20.0f, 0.0f, 0.0f);
                    Matrix.multiplyMM(lightTransformMatrix, 0, lightRotationMatrix, 0, lightTranslationMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, modelMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMM(lightTransformMatrix, 0, viewMatrix, 0, lightTransformMatrix, 0);
                    Matrix.multiplyMV(result, 0, lightTransformMatrix, 0, lights[1].position, 0);
                    GLES32.glUniform4fv(pvLightPositionUniform[2], 1, result, 0);

                    GLES32.glUniform3fv(pvKaUniform, 1, materialAmbient, 0);
                    GLES32.glUniform3fv(pvKdUniform, 1, materialDiffuse, 0);
                    GLES32.glUniform3fv(pvKsUniform, 1, materialSpecular, 0);

                    GLES32.glUniform1f(pvMaterialShininessUniform, materialShininess);
                    GLES32.glUniform1i(pvSingleTapUniform, bLight ? 1 : 0);

                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                }
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
        for(int i = 0; i < 3; i++){
            lights[i].lightAngle = lights[i].lightAngle + 3.0f;
        }
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
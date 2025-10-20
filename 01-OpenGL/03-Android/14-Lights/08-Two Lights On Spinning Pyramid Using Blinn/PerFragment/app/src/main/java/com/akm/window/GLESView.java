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

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    //Event related variables
    private GestureDetector gestureDetector;
    private Context context;

    //OpenGL related variables
    private int shaderProgramObject;
    private float perspectiveProjectionMatrix[] = new float[16];
    private int modelMatrixUniform = 0;
    private int viewMatrixUniform = 0;
    private int projectionMatrixUniform = 0;

    private int vaoPyramid[] = new int[1];
    private int vboPyramidPosition[] = new int[1];
    private int vboPyramidNormal[] = new int[1];

    private float anglePyramid = 0.0f;

    //Light parameters
    private int laUniform[] = new int[2];
    private int ldUniform[] = new int[2];
    private int lsUniform[] = new int[2];
    private int lightPositionUniform[] = new int[2];

    private int kaUniform = 0;
    private int kdUniform = 0;
    private int ksUniform = 0;
    private int materialShininessUniform = 0;

    Light lights[] = new Light[2];

    private float materialAmbient[] = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
    private float materialDiffuse[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialSpecular[] = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;

    private int pfSingleTapUniform = 0;
    private boolean bLight = false;

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
        bLight = !bLight;
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
            "in vec3 aNormal;\n" + 
            "out vec4 eyeCoordinates;\n" + 
            "out vec3 transformedNormal;\n" + 
            "out vec3 lightSource[2];\n" + 
            "uniform mat4 uModelMatrix;\n" + 
            "uniform mat4 uViewMatrix;\n" + 
            "uniform mat4 uProjectionMatrix;\n" + 
            "uniform vec4 uLightPosition[2];\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
            "   transformedNormal = (normalMatrix * aNormal);\n" + 
            "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
            "   for(int i = 0; i < 2; i++)\n" + 
            "   {\n" + 
            "       lightSource[i] = (vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n" + 
            "   }\n" + 
            "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" + 
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
            "in vec3 transformedNormal;\n" + 
            "in vec4 eyeCoordinates;\n" + 
            "in vec3 lightSource[2];\n" + 
            "uniform vec3 uLa[2];\n" + 
            "uniform vec3 uLd[2];\n" + 
            "uniform vec3 uLs[2];\n" + 
            "uniform vec3 uKa;\n" + 
            "uniform vec3 uKd;\n" + 
            "uniform vec3 uKs;\n" + 
            "uniform float uMaterialShininess;\n" + 
            "uniform int  uLKeyPressed;\n" + 
            "vec4 out_phong_ads_Light_blinn;\n" + 
            "out vec4 FragColor;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "   out_phong_ads_Light_blinn = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
            "   vec3 normalizedLightSource[3];\n" + 
            "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
            "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
            "   if(uLKeyPressed == 1)\n" + 
            "   {\n" + 
            "       float tnDotLd[2];\n" + 
            "       vec3 halfVector[2];\n" + 
            "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
            "       vec3 ambient[2];\n" + 
            "       vec3 diffuse[2];\n" + 
            "       vec3 specular[2];\n" + 
            "       for(int i = 0; i < 2; i++)\n" + 
            "       {\n" + 
            "           normalizedLightSource[i] = normalize(lightSource[i]);\n" + 
            "           tnDotLd[i] = max(dot(normalizedLightSource[i], normalizedTransformNormal), 0.0);\n" + 
            "           halfVector[i] = normalize(lightSource[i] + viewerVector[i]) / length(lightSource[i] + viewerVector[i]);\n" + 
            "           ambient[i] = uLa[i] * uKa;\n" + 
            "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n" + 
            "           specular[i] = uLs[i] * uKs * pow(max(dot(halfVector[i], transformedNormal), 0.0), uMaterialShininess);\n" + 
            "           out_phong_ads_Light_blinn = out_phong_ads_Light_blinn + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n" + 
            "       }\n" + 
            "       FragColor = out_phong_ads_Light_blinn;\n" + 
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
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
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
        laUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLa[0]");
        ldUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLd[0]");
        lsUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLs[0]");
        laUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLa[1]");
        ldUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLd[1]");
        lsUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLs[1]");
        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKa");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKs");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
        lightPositionUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLightPosition[0]");
        lightPositionUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLightPosition[1]");
        pfSingleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLKeyPressed");

        // Light parameters
        lights[0] = new Light();
        lights[0].ambient   = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[0].diffuse   = new float[] {1.0f, 0.0f, 0.0f, 1.0f};
        lights[0].specular  = new float[] {1.0f, 0.0f, 0.0f, 1.0f};
        lights[0].position  = new float[] {-2.0f, 0.0f, 0.0f, 1.0f};

        lights[1] = new Light();
        lights[1].ambient   = new float[] {0.0f, 0.0f, 0.0f, 1.0f};
        lights[1].diffuse   = new float[] {0.0f, 0.0f, 1.0f, 1.0f};
        lights[1].specular  = new float[] {0.0f, 0.0f, 1.0f, 1.0f};
        lights[1].position  = new float[] {2.0f, 0.0f, 0.0f, 1.0f};

        //Pyramid vertices
        {
            final float pyramidVertices[] = new float[]
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

            final float pyramidNormals[] = new float[]
            {
                // front
                0.000000f, 0.447214f,  0.894427f, // front-top
                0.000000f, 0.447214f,  0.894427f, // front-left
                0.000000f, 0.447214f,  0.894427f, // front-right
                                        
                // right			    
                0.894427f, 0.447214f,  0.000000f, // right-top
                0.894427f, 0.447214f,  0.000000f, // right-left
                0.894427f, 0.447214f,  0.000000f, // right-right

                // back
                0.000000f, 0.447214f, -0.894427f, // back-top
                0.000000f, 0.447214f, -0.894427f, // back-left
                0.000000f, 0.447214f, -0.894427f, // back-right

                // left
                -0.894427f, 0.447214f,  0.000000f, // left-top
                -0.894427f, 0.447214f,  0.000000f, // left-left
                -0.894427f, 0.447214f,  0.000000f, // left-right
            };

            GLES32.glGenVertexArrays(1, vaoPyramid, 0);
            GLES32.glBindVertexArray(vaoPyramid[0]);
            {   
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(pyramidVertices.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer pyramidVerticesBuffer = byteBuffer.asFloatBuffer();
                pyramidVerticesBuffer.put(pyramidVertices);
                pyramidVerticesBuffer.position(0);
                GLES32.glGenBuffers(1, vboPyramidPosition, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPyramidPosition[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramidVerticesBuffer.capacity() * 4, pyramidVerticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                //Normal
                byteBuffer = ByteBuffer.allocateDirect(pyramidNormals.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());
                FloatBuffer pyramidNormalBuffer = byteBuffer.asFloatBuffer();
                pyramidNormalBuffer.put(pyramidNormals);
                pyramidNormalBuffer.position(0);
                GLES32.glGenBuffers(1, vboPyramidNormal, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPyramidNormal[0]);
                {
                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramidNormalBuffer.capacity() * 4, pyramidNormalBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

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

            //Pyramid
            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -6.0f);
            Matrix.rotateM(rotationMatrix, 0, anglePyramid, 0.0f, 1.0f, 0.0f);
            Matrix.multiplyMM(modelMatrix, 0, translationMatrix, 0, rotationMatrix, 0);

            GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            for(int i = 0; i < 2; i++){
                GLES32.glUniform3fv(laUniform[i], 1, lights[i].ambient, 0);
                GLES32.glUniform3fv(ldUniform[i], 1, lights[i].diffuse, 0);
                GLES32.glUniform3fv(lsUniform[i], 1, lights[i].specular, 0);
                GLES32.glUniform4fv(lightPositionUniform[i], 1, lights[i].position, 0);
            }

            GLES32.glUniform3f(kaUniform, materialAmbient[0], materialAmbient[1], materialAmbient[2]);
            GLES32.glUniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
            GLES32.glUniform3f(ksUniform, materialSpecular[0], materialSpecular[1], materialSpecular[2]);
            GLES32.glUniform1f(materialShininessUniform, materialShininess);
            GLES32.glUniform1i(pfSingleTapUniform, bLight ? 1 : 0);

            GLES32.glBindVertexArray(vaoPyramid[0]);
            {
                GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
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
        anglePyramid += 1.0f;
    }

    private void uninitialize()
    {
        //Code
        if(vboPyramidPosition[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPyramidPosition, 0);
            vboPyramidPosition[0] = 0;
        }
        if(vboPyramidNormal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPyramidNormal, 0);
            vboPyramidNormal[0] = 0;
        }
        if(vaoPyramid[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoPyramid, 0);
            vaoPyramid[0] = 0;
        }
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
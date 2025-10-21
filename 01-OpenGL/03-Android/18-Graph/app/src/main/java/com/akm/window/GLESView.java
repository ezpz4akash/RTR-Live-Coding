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
    private int mvpMatrixUniform;
    private float perspectiveProjectionMatrix[] = new float[16];

    private int vaoAxisLines[] = new int[1];
    private int vboPositionAxisLines[] = new int[1];
    private int vboPositionAxisColors[] = new int[1];

    private int vaoThinBlueLine[] = new int[1];
    private int vboPositionThinBlueLines[] = new int[1];

    private int vaoThickBlueLine[] = new int[1];
    private int vboPositionThickBlueLines[] = new int[1];

    private int vaoTriangle[] = new int[1];
    private int vboPositionTriangle[] = new int[1];

    private int vaoSquare[] = new int[1];
    private int vboPositionSquare[] = new int[1];

    private int vaoCircle[] = new int[1];
    private int vboPositioncircle[] = new int[1];

    private int singleTapCount = 0;

    private float angle = 0.0f;


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
        singleTapCount = singleTapCount + 1;
        if(singleTapCount > 3){
            singleTapCount = 0;
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
            "in vec4 aColor;\n" + 
            "out vec4 out_color;\n" + 
            "uniform mat4 uMVPMatrix;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "gl_Position = uMVPMatrix * aPosition;\n" + 
            "out_color = aColor;\n" + 
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
            "in vec4 out_color;\n" + 
            "void main(void)\n" + 
            "{\n" + 
            "FragColor = out_color;\n" + 
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

        mvpMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

        //VAO Setup Thin Blue Lines
        {
            float thinBlueLinePosition[] = new float[32 * 3 * 2 * 2];
            int lineCounter = 0;
            float spacing = (1.0f / 20.0f);

            int iVertex = 0;
            for(float y = -1.0f; y < 1.0f + spacing; y = y + spacing){
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

            lineCounter = 0;
            for(float x = -1.0f; x < 1.0f + spacing; x = x + spacing){
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

            //Setup vao and vbo
            GLES32.glGenVertexArrays(1, vaoThinBlueLine, 0);
            GLES32.glBindVertexArray(vaoThinBlueLine[0]);
            {
                GLES32.glGenBuffers(1, vboPositionThinBlueLines, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionThinBlueLines[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(thinBlueLinePosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(thinBlueLinePosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, thinBlueLinePosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //VAO Setup Thick Blue Lines
        {
            float thickBlueLinePosition[] = new float[32 * 3 * 2 * 2];
            int lineCounter = 0;
            float spacing = (1.0f / 20.0f);

            int iVertex = 0;
            for(float y = -1.0f; y < 1.0f + spacing; y = y + spacing){
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

            lineCounter = 0;
            for(float x = -1.0f; x < 1.0f + spacing; x = x + spacing){
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

            //Setup vao and vbo
            GLES32.glGenVertexArrays(1, vaoThickBlueLine, 0);
            GLES32.glBindVertexArray(vaoThickBlueLine[0]);
            {
                GLES32.glGenBuffers(1, vboPositionThickBlueLines, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionThickBlueLines[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(thickBlueLinePosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(thickBlueLinePosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, thickBlueLinePosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //VAO Setup Axis Lines
        {
            float axisPosition[] = {
                -1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,

                0.0f, -1.0f, 0.0f,
                0.0f, 1.0f, 0.0f
            };

            float axisColor[] = {
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,

                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f
            };
            GLES32.glGenVertexArrays(1, vaoAxisLines, 0);
            GLES32.glBindVertexArray(vaoAxisLines[0]);
            {
                GLES32.glGenBuffers(1, vboPositionAxisLines, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionAxisLines[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(axisPosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(axisPosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, axisPosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                GLES32.glGenBuffers(1, vboPositionAxisColors, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionAxisColors[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(axisColor.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(axisColor);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, axisColor.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //VAO Setup Triangle
        {
            float trianglePosition[] = {
                0.0f, 0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f
            };

            GLES32.glGenVertexArrays(1, vaoTriangle, 0);
            GLES32.glBindVertexArray(vaoTriangle[0]);
            {
                GLES32.glGenBuffers(1, vboPositionTriangle, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionTriangle[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(trianglePosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(trianglePosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, trianglePosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //VAO Setup Square
        {
            float squarePosition[] = {
                -0.5f, 0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.5f, 0.5f, 0.0f
            };

            GLES32.glGenVertexArrays(1, vaoSquare, 0);
            GLES32.glBindVertexArray(vaoSquare[0]);
            {
                GLES32.glGenBuffers(1, vboPositionSquare, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositionSquare[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(squarePosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(squarePosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squarePosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }

        //VAO Setup Circle
        {
            final int NO_OF_CIRCLE_POINTS = 360;
            final int NO_OF_FLOATS_IN_3D = 3;

            // Convert degrees to radians
            

            // Main logic
            float[] circlePosition = new float[NO_OF_CIRCLE_POINTS * NO_OF_FLOATS_IN_3D];
            float radius = 0.5f;
            int i = 0;

            // Generate circle points
            for (float angle = 0.0f; angle < 360.0f; angle += 1.0f, i += 3) {
                circlePosition[i]     = (float)(radius * Math.cos(degToRad(angle)));
                circlePosition[i + 1] = (float)(radius * Math.sin(degToRad(angle)));
                circlePosition[i + 2] = 0.0f;
            }

            GLES32.glGenVertexArrays(1, vaoCircle, 0);
            GLES32.glBindVertexArray(vaoCircle[0]);
            {
                GLES32.glGenBuffers(1, vboPositioncircle, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositioncircle[0]);
                {
                    java.nio.ByteBuffer byteBuffer = java.nio.ByteBuffer.allocateDirect(circlePosition.length * 4);
                    byteBuffer.order(java.nio.ByteOrder.nativeOrder());
                    java.nio.FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();

                    verticesBuffer.put(circlePosition);
                    verticesBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, circlePosition.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
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

    private float degToRad(float deg) {
        final float GL_PI = 3.145f;
        return (float)(deg * (GL_PI / 180.0f));
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

        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];

        //Use shader program object
        GLES32.glUseProgram(shaderProgramObject);
        {
            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);
            Matrix.rotateM(modelViewMatrix, 0, angle, 0.0f, 1.0f, 0.0f);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);    
            
            if(singleTapCount >= 0){
                GLES32.glBindVertexArray(vaoThinBlueLine[0]);
                {
                    GLES32.glLineWidth(1.0f);
                    GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f, 1.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINES, 0, 32 * 2 * 2);
                }
                GLES32.glBindVertexArray(0);

                GLES32.glBindVertexArray(vaoThickBlueLine[0]);
                {
                    GLES32.glLineWidth(3.0f);
                    GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f, 1.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINES, 0, 9 * 2 * 2);
                }
                GLES32.glBindVertexArray(0);

                GLES32.glBindVertexArray(vaoAxisLines[0]);
                {
                    GLES32.glLineWidth(3.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINES, 0, 9 * 2 * 2);
                }
                GLES32.glBindVertexArray(0);
            }

            if(singleTapCount >= 1){
                GLES32.glBindVertexArray(vaoTriangle[0]);
                {
                    GLES32.glLineWidth(2.0f);
                    GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f, 1.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 3);
                }
                GLES32.glBindVertexArray(0);
            }

            if(singleTapCount >= 2){
                GLES32.glBindVertexArray(vaoSquare[0]);
                {
                    GLES32.glLineWidth(2.0f);
                    GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f, 1.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 4);
                }
                GLES32.glBindVertexArray(0);
            }

            if(singleTapCount >= 3){
                GLES32.glBindVertexArray(vaoCircle[0]);
                {
                    GLES32.glLineWidth(2.0f);
                    GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f, 1.0f);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 360);
                }
                GLES32.glBindVertexArray(0);
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
        angle += 0.0f;
    }

    private void uninitialize()
    {
        //Code
        if(vaoThinBlueLine[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoThinBlueLine, 0);
            vaoThinBlueLine[0] = 0;
        }
        if(vaoThickBlueLine[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoThickBlueLine, 0);
            vaoThickBlueLine[0] = 0;
        }
        if(vaoAxisLines[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoAxisLines, 0);
            vaoAxisLines[0] = 0;
        }
        if(vaoTriangle[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoTriangle, 0);
            vaoTriangle[0] = 0;
        }
        if(vaoSquare[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoSquare, 0);
            vaoSquare[0] = 0;
        }
        if(vaoCircle[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vaoCircle, 0);
            vaoCircle[0] = 0;
        }
        if(vboPositionAxisLines[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionAxisLines, 0);
            vboPositionAxisLines[0] = 0;
        }
        if(vboPositionAxisColors[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionAxisColors, 0);
            vboPositionAxisColors[0] = 0;
        }
        if(vboPositionThinBlueLines[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionThinBlueLines, 0);
            vboPositionThinBlueLines[0] = 0;
        }
        if(vboPositionThickBlueLines[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionThickBlueLines, 0);
            vboPositionThickBlueLines[0] = 0;
        }
        if(vboPositionTriangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionTriangle, 0);
            vboPositionTriangle[0] = 0;
        }
        if(vboPositionSquare[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositionSquare, 0);
            vboPositionSquare[0] = 0;
        }
        if(vboPositioncircle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPositioncircle, 0);
            vboPositioncircle[0] = 0;
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
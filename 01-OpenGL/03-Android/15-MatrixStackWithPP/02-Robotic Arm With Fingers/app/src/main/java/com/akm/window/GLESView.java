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

import java.util.ArrayDeque;
import java.util.Deque; // optional - good practice to program to the interface


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    //Event related variables
    private GestureDetector gestureDetector;
    private Context context;

    //OpenGL related variables
    private int shaderProgramObject;
    private float perspectiveProjectionMatrix[] = new float[16];
    private int mvpMatrixUniform;

    //Sphere related variables
    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];
    private int numVertices;
    private int numElements;

    private int shoulder    = 0;
    private int elbow       = 0;
    private int wrist       = 0;
    private int finger1     = 0;
    private int finger2     = 0;
    private int finger3     = 0;
    private int finger4     = 0;
    private int finger5     = 0;

    private int whatToRotate = 0;

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
        whatToRotate++;
        if(whatToRotate > 7){
            whatToRotate = 0;
        }
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
        if(whatToRotate == 0){
            shoulder = shoulder + 10;
        }
        if(whatToRotate == 1){
            elbow = elbow + 10;
        }
        if(whatToRotate == 2){
            wrist = wrist + 10;
        }
        if(whatToRotate == 3){
            finger1 = finger1 + 10;
        }
        if(whatToRotate == 4){
            finger2 = finger2 + 10;
        }
        if(whatToRotate == 5){
            finger3 = finger3 + 10;
        }
        if(whatToRotate == 6){
            finger4 = finger4 + 10;
        }
        if(whatToRotate == 7){
            finger5 = finger5 + 10;
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
            "precision highp float;\n" +
            "precision highp int;\n" +
            "in vec4 aPosition;\n" + 
            "in vec4 aNormal;\n" + 
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
            "precision highp int;\n" +
            "in vec4 out_color;\n" + 
            "out vec4 FragColor;\n" + 
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
        GLES32.glBindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
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

    private void display() {
        // Clear
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        // Matrices
        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];
        float scaleMatrix[] = new float[16];
        float rotationMatrix[] = new float[16];
        float translationMatrix[] = new float[16];

        ArrayDeque<float[]> stack = new ArrayDeque<>();

        // Use shader
        GLES32.glUseProgram(shaderProgramObject);

        // Identity
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(scaleMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(translationMatrix, 0);

        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -12.0f);

        multiplyMM(modelViewMatrix, translationMatrix);

        // push root
        stack.push(copyMat(modelViewMatrix));
            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setRotateM(rotationMatrix, 0, shoulder, 0.0f, 0.0f, 1.0f);

            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.translateM(translationMatrix, 0, 1.0f, 0.0f, 0.0f);

            multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);
            multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);

            stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 2.1f, 0.6f, 1.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);

                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);
            modelViewMatrix = stack.pop();

            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.translateM(translationMatrix, 0, 1.0f, 0.0f, 0.0f);

            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setRotateM(rotationMatrix, 0, elbow, 0.0f, 0.0f, 1.0f);

            multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);
            multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);

            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.translateM(translationMatrix, 0, 1.0f, 0.0f, 0.0f);
            multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

            stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 2.1f, 0.6f, 1.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);
            modelViewMatrix = stack.pop();

            // Wrist
            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.translateM(translationMatrix, 0, 1.0f, 0.0f, 0.0f);

            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setRotateM(rotationMatrix, 0, wrist, 0.0f, 0.0f, 1.0f);

            multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);
            multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);

            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
            multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

            stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 1.0f, 0.4f, 1.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);
            modelViewMatrix = stack.pop();

            // Finger 1
            stack.push(copyMat(modelViewMatrix));
            {
                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, 45.0f, 0.0f, 1.0f, 0.0f); 
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, finger1, 0.0f, 0.0f, 1.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.3f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 0.5f, 0.01f, 0.1f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);

                modelViewMatrix = stack.pop();
            }
            modelViewMatrix = stack.pop();

            // Finger 2
            stack.push(copyMat(modelViewMatrix));
            {
                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, 30.0f, 0.0f, 1.0f, 0.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, finger2, 0.0f, 0.0f, 1.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.3f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 0.5f, 0.01f, 0.1f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);

                modelViewMatrix = stack.pop();
            }
            modelViewMatrix = stack.pop();

            // Finger 3
            stack.push(copyMat(modelViewMatrix));
            {
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, finger3, 0.0f, 0.0f, 1.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.3f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 0.5f, 0.01f, 0.1f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);

                modelViewMatrix = stack.pop();
            }
            modelViewMatrix = stack.pop();

            // Finger
            stack.push(copyMat(modelViewMatrix));
            {
                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, -30.0f, 0.0f, 1.0f, 0.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, finger4, 0.0f, 0.0f, 1.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.3f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 0.5f, 0.01f, 0.1f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);

                modelViewMatrix = stack.pop();
            }
            modelViewMatrix = stack.pop();

            // Finger 5
            stack.push(copyMat(modelViewMatrix));
            {
                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, -45.0f, 0.0f, 1.0f, 0.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.5f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                Matrix.setIdentityM(rotationMatrix, 0);
                Matrix.setRotateM(rotationMatrix, 0, finger5, 0.0f, 0.0f, 1.0f);
                Matrix.setIdentityM(translationMatrix, 0);
                Matrix.translateM(translationMatrix, 0, 0.3f, 0.0f, 0.0f);
                multiplyMM(modelViewMatrix, modelViewMatrix, rotationMatrix);
                multiplyMM(modelViewMatrix, modelViewMatrix, translationMatrix);

                stack.push(copyMat(modelViewMatrix));
                Matrix.setIdentityM(scaleMatrix, 0);
                Matrix.scaleM(scaleMatrix, 0, 0.5f, 0.01f, 0.1f);
                multiplyMM(modelViewMatrix, modelViewMatrix, scaleMatrix);

                GLES32.glVertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.5f, 0.35f, 0.05f, 1.0f);
                multiplyVP(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(vao_sphere[0]);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindVertexArray(0);

                modelViewMatrix = stack.pop();
            }
        modelViewMatrix = stack.pop();

        if (!stack.isEmpty()) {
            modelViewMatrix = stack.pop();
        }

        GLES32.glUseProgram(0);

        requestRender();
    }

    private void multiplyMM(float[] dest, float[] rhs) {
        float tmp[] = new float[16];
        Matrix.multiplyMM(tmp, 0, dest, 0, rhs, 0);
        System.arraycopy(tmp, 0, dest, 0, 16);
    }

    private void multiplyMM(float[] out, float[] a, float[] b) {
        Matrix.multiplyMM(out, 0, a, 0, b, 0);
    }

    private void multiplyVP(float[] out, float[] proj, float[] mv) {
        Matrix.multiplyMM(out, 0, proj, 0, mv, 0);
    }

    private float[] copyMat(float[] m) {
        float[] c = new float[16];
        System.arraycopy(m, 0, c, 0, 16);
        return c;
    }


    private void update()
    {
        //Code
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
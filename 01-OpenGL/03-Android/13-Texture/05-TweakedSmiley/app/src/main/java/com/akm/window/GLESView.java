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

    //Uniform locations
    private int mvpMatrixUniform;
    private int keyPressedUniform;

    private int vao[] = new int[1];
    private int vboPosition[] = new int[1];
    private int vboTexcoord[] = new int[1];

    //Texture variables
    private int smileyTexture;
    private int textureSamplerUniform;

    //Single tap count
    private int singleTapCount = 0;

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
        singleTapCount = 0;
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        singleTapCount++;
        if(singleTapCount > 3)
            singleTapCount = 0;
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
            "uniform int uKeyPress;\n" +
            "in vec2 out_texCoord;\n" + 
            "uniform sampler2D uTextureSampler;\n" + 
            "void main(void)\n" + 
            "{\n" + 
                "if(uKeyPress != -1)\n" + 
                    "FragColor = texture(uTextureSampler, out_texCoord);\n" + 
                "else\n" + 
                    "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" + 
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

        mvpMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uTextureSampler");
        keyPressedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKeyPress");
        

        //Rectangle vertices
        {
            final float rectangle_position[] = new float[]
            {
                1.0f, 1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f
            };

            final float rectangle_texcoord[] = new float[]
            {
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f
            };

            GLES32.glGenVertexArrays(1, vao, 0);
            GLES32.glBindVertexArray(vao[0]);
            {
                //Position
                GLES32.glGenBuffers(1, vboPosition, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPosition[0]);
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(rectangle_position.length * 4);
                    byteBuffer.order(ByteOrder.nativeOrder());
                    FloatBuffer positionBuffer = byteBuffer.asFloatBuffer();
                    positionBuffer.put(rectangle_position);
                    positionBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, rectangle_position.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                //Texcoord
                GLES32.glGenBuffers(1, vboTexcoord, 0);
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTexcoord[0]);
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(rectangle_texcoord.length * 4);
                    byteBuffer.order(ByteOrder.nativeOrder());
                    FloatBuffer texcoordBuffer = byteBuffer.asFloatBuffer();
                    texcoordBuffer.put(rectangle_texcoord);
                    texcoordBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, rectangle_texcoord.length * 4, texcoordBuffer, GLES32.GL_STATIC_DRAW);
                    GLES32.glVertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 0, 0);
                    GLES32.glEnableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD);
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

        smileyTexture = loadGLTexture(R.raw.smiley);

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

        float modelViewMatrix[] = new float[16];
        float modelViewProjectionMatrix[] = new float[16];
        float translationMatrix[] = new float[16];
        float rotationMatrix[] = new float[16];
        float scaleMatrix[] = new float[16];

        //Use shader program object
        GLES32.glUseProgram(shaderProgramObject);
        {
            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setIdentityM(scaleMatrix, 0);

            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -4.0f);
            Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, translationMatrix, 0);
            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            //Bind with smiley texture
            GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smileyTexture);
            GLES32.glUniform1i(textureSamplerUniform, 0);

            //Bind with vao
            GLES32.glBindVertexArray(vao[0]);
            {
                final float rectangle_texcoord[] = new float[8];
                if(singleTapCount == 0){
                    GLES32.glEnable(GLES32.GL_TEXTURE_2D);
                    GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smileyTexture);
                    GLES32.glUniform1i(keyPressedUniform, singleTapCount);
                    rectangle_texcoord[0] = 0.5f; rectangle_texcoord[1] = 0.5f;
                    rectangle_texcoord[2] = 0.0f; rectangle_texcoord[3] = 0.5f;
                    rectangle_texcoord[4] = 0.0f; rectangle_texcoord[5] = 0.0f;
                    rectangle_texcoord[6] = 0.5f; rectangle_texcoord[7] = 0.0f;
                }
                else if(singleTapCount == 1){
                    GLES32.glEnable(GLES32.GL_TEXTURE_2D);
                    GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smileyTexture);
                    GLES32.glUniform1i(keyPressedUniform, singleTapCount);
                    rectangle_texcoord[0] = 1.0f; rectangle_texcoord[1] = 1.0f;
                    rectangle_texcoord[2] = 0.0f; rectangle_texcoord[3] = 1.0f;
                    rectangle_texcoord[4] = 0.0f; rectangle_texcoord[5] = 0.0f;
                    rectangle_texcoord[6] = 1.0f; rectangle_texcoord[7] = 0.0f;
                }
                else if(singleTapCount == 2){
                    GLES32.glEnable(GLES32.GL_TEXTURE_2D);
                    GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smileyTexture);
                    GLES32.glUniform1i(keyPressedUniform, singleTapCount);
                    rectangle_texcoord[0] = 2.0f; rectangle_texcoord[1] = 2.0f;
                    rectangle_texcoord[2] = 0.0f; rectangle_texcoord[3] = 2.0f;
                    rectangle_texcoord[4] = 0.0f; rectangle_texcoord[5] = 0.0f;
                    rectangle_texcoord[6] = 2.0f; rectangle_texcoord[7] = 0.0f;
                }
                else if(singleTapCount == 3){
                    GLES32.glEnable(GLES32.GL_TEXTURE_2D);
                    GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, smileyTexture);
                    GLES32.glUniform1i(keyPressedUniform, singleTapCount);
                    rectangle_texcoord[0] = 0.5f; rectangle_texcoord[1] = 0.5f;
                    rectangle_texcoord[2] = 0.5f; rectangle_texcoord[3] = 0.5f;
                    rectangle_texcoord[4] = 0.5f; rectangle_texcoord[5] = 0.5f;
                    rectangle_texcoord[6] = 0.5f; rectangle_texcoord[7] = 0.5f;
                }

                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTexcoord[0]);
                {
                    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(rectangle_texcoord.length * 4);
                    byteBuffer.order(ByteOrder.nativeOrder());
                    FloatBuffer texcoordBuffer = byteBuffer.asFloatBuffer();
                    texcoordBuffer.put(rectangle_texcoord);
                    texcoordBuffer.position(0);

                    GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, rectangle_texcoord.length * 4, texcoordBuffer, GLES32.GL_DYNAMIC_DRAW);
                }
                GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
                GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
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
    }

    private void uninitialize()
    {
        //Code
        if(vboPosition[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboPosition, 0);
            vboPosition[0] = 0;
        }
        if(vboTexcoord[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vboTexcoord, 0);
            vboTexcoord[0] = 0;
        }
        if(smileyTexture != 0)
        {
            GLES32.glDeleteTextures(1, new int[] {smileyTexture}, 0);
            smileyTexture = 0;
        }
        if(vao[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
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
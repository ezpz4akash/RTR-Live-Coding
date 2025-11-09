var canvas = null;
var gl = null;
var bFullScreen = null;
var canvas_original_width;
var canvas_original_height;

//WebGL related variables
const MyAttributes = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_COLOR: 1
};

var shaderProgramObject = null;
var mvpMatrixUniform = null;

var vaoTriangle = null;
var vboPositionTriangle = null;
var vboColorTriangle = null;

var vaoRectangle = null;
var vboPositionRectangle = null;
var vboColorRectangle = null;

var perspectiveProjectionMatrix;

var angleTriangle = 0.0;
var angleRectangle = 0.0;

var requestAnimationFrame =
    window.requestAnimationFrame ||
    window.webkitRequestAnimationFrame ||
    window.mozRequestAnimationFrame ||
    window.oRequestAnimationFrame ||
    window.msRequestAnimationFrame;


function main(){
    canvas = document.getElementById('glCanvas');

    if(canvas === null){
        console.log('Failed to retrieve the <canvas> element');
        return;
    }

    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    // Register event handlers
    window.addEventListener('keydown', keyDown, false);
    window.addEventListener('mousedown', mouseDown, false);
    window.addEventListener('resize', resize, false);

    initialize();
    resize();
    display();
}

function keyDown(event){
    console.log('Key Down: ' + event.key);
    console.log('Key Down: ' + event.keyCode);

    switch(event.keyCode){
        case 70:  // 'F'
        case 102: // 'f'
            bFullScreen = !bFullScreen;
            toggleFullscreen();
        break;

        case 27: // Escape
            uninitialize();
            window.close();
        break;
    }
}

function mouseDown(event){
    console.log('Mouse Button Pressed: ' + event.button);
}

function toggleFullscreen(){
    var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
    if(fullscreenElement == null){
        if(canvas.requestFullscreen){
            canvas.requestFullscreen();
        } else if(canvas.mozRequestFullScreen){
            canvas.mozRequestFullScreen();
        } else if(canvas.webkitRequestFullscreen){
            canvas.webkitRequestFullscreen();
        } else if(canvas.msRequestFullscreen){
            canvas.msRequestFullscreen();
        }
    } else {
        if(document.exitFullscreen){
            document.exitFullscreen();
        } else if(document.mozCancelFullScreen){
            document.mozCancelFullScreen();
        } else if(document.webkitExitFullscreen){
            document.webkitExitFullscreen();
        } else if(document.msExitFullscreen){
            document.msExitFullscreen();
        }   
    }
}

function initialize(){
    // code
    // Initialize the WebGL gl
    gl = canvas.getContext('webgl2');

    if(gl === null){
        console.log('Context cannot be obtained');
        return;
    }

    //set viewport width and height
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;


    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    var vertexShaderSourceCode =
    "#version 300 es\n" + 
    "in vec4 aPosition;\n" + 
    "in vec4 aColor;\n" +
    "out vec4 out_color;\n" +
    "uniform mat4 uMVPMatrix;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "gl_Position = uMVPMatrix * aPosition;\n" + 
    "out_color = aColor;\n" +
    "}\n";

    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);
    var compileStatus = gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(vertexShaderObject);
        console.log('Vertex Shader Compilation Error: ' + error);
        return;
    }

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    var fragmentShaderSourceCode =
    "#version 300 es\n" + 
    "precision highp float;\n" +
    "in vec4 out_color;\n" + 
    "out vec4 FragColor;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "FragColor = out_color;\n" + 
    "}\n";

    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    compileStatus = gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        console.log('Fragment Shader Compilation Error: ' + error);
        return;
    }

    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_COLOR, "aColor");

    gl.linkProgram(shaderProgramObject);

    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uMVPMatrix");

    {
        var triangleVertices = new Float32Array([
            0.0,  1.0, 0.0,
            -1.0, -1.0, 0.0,
            1.0, -1.0, 0.0
        ]);

        var triangleColors = new Float32Array([
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        ]);

        vaoTriangle = gl.createVertexArray();
        gl.bindVertexArray(vaoTriangle);
        {
            vboPositionTriangle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboPositionTriangle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);

            vboColorTriangle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboColorTriangle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, triangleColors, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }
    
    {
        var rectangleVertices = new Float32Array([
            1.0, 1.0, 0.0,
            -1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0,
            1.0, -1.0, 0.0
        ]);

        var rectangleColors = new Float32Array([
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0
        ]);

        vaoRectangle = gl.createVertexArray();
        gl.bindVertexArray(vaoRectangle);
        {
            vboPositionRectangle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboPositionRectangle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, rectangleVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);

            vboColorRectangle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboColorRectangle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, rectangleColors, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    // Initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // Set clear color to blue
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

}

function resize(){
    // code
    if(bFullScreen == true){
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else{
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function display(){
    //code

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(rotationMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -6.0]);
    mat4.rotateY(rotationMatrix, rotationMatrix, angleTriangle);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vaoTriangle);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    gl.bindVertexArray(null);

    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(rotationMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [1.5, 0.0, -6.0]);
    mat4.rotateX(rotationMatrix, rotationMatrix, angleRectangle);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vaoRectangle);
    {
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    }
    gl.bindVertexArray(null);

    update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
    angleTriangle += 0.01;
    angleRectangle += 0.01;
}

function uninitialize(){
    // code
    if(vaoTriangle){
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
    }
    if(vboPositionTriangle){
        gl.deleteBuffer(vboPositionTriangle);
        vboPositionTriangle = null;
    }
    if(vboColorTriangle){
        gl.deleteBuffer(vboColorTriangle);
        vboColorTriangle = null;
    }
    if(vaoRectangle){
        gl.deleteVertexArray(vaoRectangle);
        vaoRectangle = null;
    }
    if(vboPositionRectangle){
        gl.deleteBuffer(vboPositionRectangle);
        vboPositionRectangle = null;
    }
    if(vboColorRectangle){
        gl.deleteBuffer(vboColorRectangle);
        vboColorRectangle = null;
    }
    if(shaderProgramObject){
        gl.useProgram(shaderProgramObject);
        var attachedShaders = gl.getAttachedShaders(shaderProgramObject);
        for(var i = 0; i < attachedShaders.length; i++){
            gl.detachShader(shaderProgramObject, attachedShaders[i]);
            gl.deleteShader(attachedShaders[i]);
            attachedShaders[i] = null;
        }
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}
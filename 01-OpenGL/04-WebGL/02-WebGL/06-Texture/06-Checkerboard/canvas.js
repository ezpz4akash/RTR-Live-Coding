var canvas = null;
var gl = null;
var bFullScreen = null;
var canvas_original_width;
var canvas_original_height;

//WebGL related variables
const MyAttributes = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_TEXCOORD0: 1
};

var shaderProgramObject = null;
var mvpMatrixUniform = null;

var vao = null;
var vboPosition = null;
var vboTexCoord = null;

var perspectiveProjectionMatrix;

//Texture related variables
var checkerboardTexture = null;
var textureSamplerUniform = null;

//Checkerboard
var checkerWidth = 64;
var checkerHeight = 64;
var checkerImage = new Uint8Array(checkerWidth * checkerHeight * 4); // RGBA

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
    "in vec2 aTexCoord;\n" + 
    "out vec2 out_texCoord;\n" + 
    "uniform mat4 uMVPMatrix;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "gl_Position = uMVPMatrix * aPosition;\n" + 
    "out_texCoord = aTexCoord;\n" + 
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
    "out vec4 FragColor;\n" + 
    "in vec2 out_texCoord;\n" + 
    "uniform sampler2D uTextureSampler;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "FragColor = texture(uTextureSampler, out_texCoord);\n" + 
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
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_TEXCOORD0, "aTexCoord");

    gl.linkProgram(shaderProgramObject);

    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uMVPMatrix");

    {
        var rectangle_position = new Float32Array([
            1.0, 1.0, 0.0,
            -1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0,
            1.0, -1.0, 0.0
        ]);

        var rectangle_texcoord = new Float32Array([
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0
        ]);



        vao = gl.createVertexArray();
        gl.bindVertexArray(vao);
        {
            vboPosition = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboPosition);
            {
                gl.bufferData(gl.ARRAY_BUFFER, rectangle_position, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);

            vboTexCoord = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboTexCoord);
            {
                gl.bufferData(gl.ARRAY_BUFFER, rectangle_texcoord, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD0);
            }
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
    gl.clearColor(0.75, 0.75, 0.75, 1.0);

    // Load textures
    checkerboardTexture = loadGLTexture();

}

function loadGLTexture() {
    // Create checkerboard pattern
    for (var i = 0; i < checkerHeight; i++) {
        for (var j = 0; j < checkerWidth; j++) {
            var c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
            checkerImage[(i * checkerWidth + j) * 4 + 0] = c; // Red
            checkerImage[(i * checkerWidth + j) * 4 + 1] = c; // Green 
            checkerImage[(i * checkerWidth + j) * 4 + 2] = c; // Blue
            checkerImage[(i * checkerWidth + j) * 4 + 3] = 255; // Alpha
        }
    }

    var texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, checkerWidth, checkerHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, checkerImage);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.bindTexture(gl.TEXTURE_2D, null);

    return texture;
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

    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, checkerboardTexture);
    gl.uniform1i(gl.getUniformLocation(shaderProgramObject, "uTextureSampler"), 0);

    gl.bindVertexArray(vao);
    {
        {
            var rectangle_position = new Float32Array([
                0.0, 1.0, 0.0,
                -2.0, 1.0, 0.0,
                -2.0, -1.0, 0.0,
                0.0, -1.0, 0.0
            ]);

            {
                gl.bindBuffer(gl.ARRAY_BUFFER, vboPosition);
                {
                    gl.bufferData(gl.ARRAY_BUFFER, rectangle_position, gl.STATIC_DRAW);
                }
                gl.bindBuffer(gl.ARRAY_BUFFER, null);
            }
            
            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        }
    }
    gl.bindVertexArray(null);


    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, checkerboardTexture);
    gl.uniform1i(gl.getUniformLocation(shaderProgramObject, "uTextureSampler"), 0);

    gl.bindVertexArray(vao);
    {
        {
            var rectangle_position = new Float32Array([
                2.41421, 1.0, -1.41421,
                1.0, 1.0, 0.0,
                1.0, -1.0, 0.0,
                2.41421, -1.0, -1.41421
            ]);

            {
                gl.bindBuffer(gl.ARRAY_BUFFER, vboPosition);
                {
                    gl.bufferData(gl.ARRAY_BUFFER, rectangle_position, gl.STATIC_DRAW);
                }
                gl.bindBuffer(gl.ARRAY_BUFFER, null);
            }

            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        }
    }
    gl.bindVertexArray(null);

    gl.useProgram(null);

    update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
}

function uninitialize(){
    // code
    if(vao){
        gl.deleteVertexArray(vao);
        vao = null;
    }
    if(vboPosition){
        gl.deleteBuffer(vboPosition);
        vboPosition = null;
    }
    if(vboTexCoord){
        gl.deleteBuffer(vboTexCoord);
        vboTexCoord = null;
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
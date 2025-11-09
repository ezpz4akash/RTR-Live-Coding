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

var vaoPyramid = null;
var vboPositionPyramid = null;
var vboTexCoordPyramid = null;

var perspectiveProjectionMatrix;

var anglePyramid = 0.0;

//Texture related variables
var stoneTexture = null;
var textureSamplerUniform = null;

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
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "uTextureSampler");

    {
        var pyramidVertices = new Float32Array([
            //Front face
                0.0,  1.0,  0.0,
                -1.0, -1.0,  1.0,
                1.0, -1.0,  1.0,

                //Right ace
                0.0,  1.0,  0.0,
                1.0, -1.0,  1.0,
                1.0, -1.0, -1.0,

                //Back ace
                0.0,  1.0,  0.0,
                1.0, -1.0, -1.0,
                -1.0, -1.0, -1.0,

                //Let ace
                0.0,  1.0,  0.0,
                -1.0, -1.0, -1.0,
                -1.0, -1.0,  1.0
        ]);

        var pyramidTexCoords = new Float32Array([
                0.5, 1.0,
                0.0, 0.0,
                1.0, 0.0,

                0.5, 1.0,
                1.0, 0.0,
                0.0, 0.0,

                0.5, 1.0,
                1.0, 0.0,
                0.0, 0.0,

                0.5, 1.0,
                0.0, 0.0,
                1.0, 0.0
        ]);



        vaoPyramid = gl.createVertexArray();
        gl.bindVertexArray(vaoPyramid);
        {
            vboPositionPyramid = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboPositionPyramid);
            {
                gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);

            vboTexCoordPyramid = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboTexCoordPyramid);
            {
                gl.bufferData(gl.ARRAY_BUFFER, pyramidTexCoords, gl.STATIC_DRAW);
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
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // Load textures
    stoneTexture = loadGLTexture('stone.png');

}

function loadGLTexture(url) {
    var texture = gl.createTexture();
    texture.image = new Image();
    texture.image.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture.image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        gl.generateMipmap(gl.TEXTURE_2D);
        gl.bindTexture(gl.TEXTURE_2D, null);
    }
    texture.image.src = url;
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
    var rotationMatrix = mat4.create();
    var scaleMatrix = mat4.create();

    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(rotationMatrix);
    mat4.identity(scaleMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -6.0]);
    mat4.rotateY(rotationMatrix, rotationMatrix, anglePyramid);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    //bind stone texture
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, stoneTexture);
    gl.uniform1i(textureSamplerUniform, 0);

    gl.bindVertexArray(vaoPyramid);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);

    update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
    anglePyramid += 0.01;
}

function uninitialize(){
    // code
    if(vaoPyramid){
        gl.deleteVertexArray(vaoPyramid);
        vaoPyramid = null;
    }
    if(vboPositionPyramid){
        gl.deleteBuffer(vboPositionPyramid);
        vboPositionPyramid = null;
    }
    if(vboTexCoordPyramid){
        gl.deleteBuffer(vboTexCoordPyramid);
        vboTexCoordPyramid = null;
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
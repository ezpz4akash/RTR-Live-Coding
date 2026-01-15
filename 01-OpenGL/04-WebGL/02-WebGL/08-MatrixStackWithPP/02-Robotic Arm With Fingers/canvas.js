var canvas = null;
var gl = null;
var bFullScreen = null;
var canvas_original_width;
var canvas_original_height;

//WebGL related variables
const MyAttributes = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_COLOR: 1,
};

var shaderProgramObject = null;
var mvpMatrixUniform = null;
var colorUniform = null;

var perspectiveProjectionMatrix;

var sphere = null;

var shoulder = 0;
var elbow = 0;
var wrist = 0;
var finger1 = 0;
var finger2 = 0;
var finger3 = 0;
var finger4 = 0;
var finger5 = 0;

var transformationMatrixStack = [];

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

        case 83: // 'S'
            shoulder = (shoulder + 3) % 360;
        break;

        case 115: // 's'
            shoulder = (shoulder - 3) % 360;
        break;

        case 69: // 'E'
            elbow = (elbow + 3) % 360;
        break;

        case 101: // 'e'
            elbow = (elbow - 3) % 360;
        break;

        case 87: // 'W'
            wrist = (wrist + 3) % 360;
        break;

        case 119: // 'w'
            wrist = (wrist - 3) % 360;
        break;

        case 49: // '1'
            finger1 = (finger1 + 3) % 360;
        break;

        case 50: // '2'
            finger2 = (finger2 + 3) % 360;
        break;

        case 51: // '3'
            finger3 = (finger3 + 3) % 360;
        break;

        case 52: // '4'
            finger4 = (finger4 + 3) % 360;
        break;

        case 53: // '5'
            finger5 = (finger5 + 3) % 360;
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
    "uniform mat4 uMVPMatrix;\n" + 
    "uniform vec4 uColor;\n" +
    "out vec4 outColor;\n" +
    "void main(void)\n" + 
    "{\n" + 
    "gl_Position = uMVPMatrix * aPosition;\n" +
    "outColor = uColor;\n" +
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
    "in vec4 outColor;\n" +
    "out vec4 FragColor;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "FragColor = outColor;\n" + 
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

    gl.linkProgram(shaderProgramObject);

    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uMVPMatrix");
    colorUniform = gl.getUniformLocation(shaderProgramObject, "uColor");

    sphere = new Mesh();
	makeSphere(sphere, 2.0, 30, 30);


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

    // Clear the transformation stack
    transformationMatrixStack = [];

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var translationMatrix = mat4.create();
    var translationMatrix2 = mat4.create();
    var rotationMatrix = mat4.create();
    var rotationMatrix2 = mat4.create();
    var scaleMatrix = mat4.create();

    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(translationMatrix);
    mat4.identity(rotationMatrix);
    mat4.identity(scaleMatrix);

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -20.0]);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));

    // Shoulder
    mat4.rotate(rotationMatrix, mat4.create(), shoulder * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));

    // Upper arm
    mat4.scale(scaleMatrix, mat4.create(), [1.5, 0.4, 0.6]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Elbow
    mat4.translate(translationMatrix, mat4.create(), [2.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), elbow * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [3.5, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);

    // Forearm
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [1.2, 0.35, 0.5]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Wrist
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), wrist * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [2.75, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));

    // Hand
    mat4.scale(scaleMatrix, mat4.create(), [0.5, 0.3, 0.7]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Finger 1
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), 60.0 * Math.PI / 180, [0.0, 1.0, 0.0]);
    mat4.rotate(rotationMatrix2, mat4.create(), finger1 * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [0.15, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.05, 0.08]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Finger 2
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), 45.0 * Math.PI / 180, [0.0, 1.0, 0.0]);
    mat4.rotate(rotationMatrix2, mat4.create(), finger2 * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [0.15, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.05, 0.08]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Finger 3
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), finger3 * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [0.15, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.05, 0.08]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Finger 4
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), -45.0 * Math.PI / 180, [0.0, 1.0, 0.0]);
    mat4.rotate(rotationMatrix2, mat4.create(), finger4 * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [0.15, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.05, 0.08]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

    // Finger 5
    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.translate(translationMatrix, mat4.create(), [1.0, 0.0, 0.0]);
    mat4.rotate(rotationMatrix, mat4.create(), -60.0 * Math.PI / 180, [0.0, 1.0, 0.0]);
    mat4.rotate(rotationMatrix2, mat4.create(), finger5 * Math.PI / 180, [0.0, 0.0, 1.0]);
    mat4.translate(translationMatrix2, mat4.create(), [0.15, 0.0, 0.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix2);
    mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix2);

    transformationMatrixStack.push(mat4.clone(modelViewMatrix));
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.05, 0.08]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.uniform4f(colorUniform, 0.5, 0.35, 0.05, 1.0);

    sphere.draw();

    // Restore modelViewMatrix
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();
    modelViewMatrix = mat4.clone(transformationMatrixStack[transformationMatrixStack.length - 1]);
    transformationMatrixStack.pop();

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
    if (sphere){
        sphere.deallocate();
        sphere = null;
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
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvas_original_width;
var canvas_original_height;

//WebGL related variables
const MyAttributes = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL: 2,
    AMC_ATTRIBUTE_TEXTURE0: 3,
    AMC_ATTRIBUTE_TEXTURE1: 4,
};

var shaderProgramObject = null;
var modelMatrixUniform = null;
var viewMatrixUniform = null;
var projectionMatrixUniform = null;

var perspectiveProjectionMatrix;

//Light related variables
var laUniform = null;
var ldUniform = null;
var lsUniform = null;
var lightPositionUniform = null;

var kaUniform = null;
var kdUniform = null;
var ksUniform = null;
var materialShininessUniform = null;

var lKeyPressedUniform = null;

var bLight = false;
var bAnimation = false;

var lightAmbient        = new Float32Array([0.0, 0.0, 0.0, 1.0]);
var lightDiffuse        = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var lightSpecular       = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var lightPosition       = new Float32Array([100.0, 100.0, 100.0, 1.0]);

var materialAmbient     = new Float32Array([0.0, 0.0, 0.0, 1.0]);
var materialDiffuse     = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialSpecular    = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialShininess   = 128.0;


//Sphere
var sphere = null;

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

        case 76: // 'L'
        case 108: // 'l'
            bLight = !bLight;
        break;

        case 65: // 'A'
        case 97: // 'a'
            bAnimation = !bAnimation;
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
    "precision highp int;\n" +
    "in vec4 out_phong_ads_Light;\n" + 
    "out vec4 FragColor;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "FragColor = out_phong_ads_Light;\n" + 
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
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");

    gl.linkProgram(shaderProgramObject);

    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uProjectionMatrix");

    // Light uniforms
    laUniform = gl.getUniformLocation(shaderProgramObject, "uLa");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "uLd");
    lsUniform = gl.getUniformLocation(shaderProgramObject, "uLs");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "uLightPosition");
    kaUniform = gl.getUniformLocation(shaderProgramObject, "uKa");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "uKd");
    ksUniform = gl.getUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "uMaterialShininess");
    lKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uLKeyPressed");

    
    {
        sphere = new Mesh();
        makeSphere(sphere, 1.0, 120, 120);
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

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    // Set light uniforms
    gl.uniform3fv(laUniform, lightAmbient.subarray(0, 3));
    gl.uniform3fv(ldUniform, lightDiffuse.subarray(0, 3));
    gl.uniform3fv(lsUniform, lightSpecular.subarray(0, 3));
    gl.uniform4fv(lightPositionUniform, lightPosition);

    gl.uniform3fv(kaUniform, materialAmbient.subarray(0, 3));
    gl.uniform3fv(kdUniform, materialDiffuse.subarray(0, 3));
    gl.uniform3fv(ksUniform, materialSpecular.subarray(0, 3));
    gl.uniform1f(materialShininessUniform, materialShininess);

    gl.uniform1i(lKeyPressedUniform, bLight == true ? 1 : 0);

    sphere.draw();

    if(bAnimation)
        update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
}

function uninitialize(){
    // code
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
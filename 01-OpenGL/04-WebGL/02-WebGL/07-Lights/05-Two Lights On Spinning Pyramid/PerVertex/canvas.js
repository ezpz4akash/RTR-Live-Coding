var canvas = null;
var gl = null;
var bFullScreen = null;
var canvas_original_width;
var canvas_original_height;

//WebGL related variables
const MyAttributes = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_NORMAL: 1,
};

var shaderProgramObject = null;
var modelMatrixUniform = null;
var viewMatrixUniform = null;
var projectionMatrixUniform = null;

var vaoPyramid = null;
var vboPositionPyramid = null;
var vboNormalPyramid = null;

var perspectiveProjectionMatrix;

var anglePyramid = 0.0;

//Lights related variables
var laUniform = new Array(2);;               // Light Ambient
var ldUniform = new Array(2);;               // Light Diffuse
var lsUniform = new Array(2);;               // Material Diffuse
var lightPositionUniform = new Array(2);;    // Light Position

var kaUniform = null;               // Light Ambient
var kdUniform = null;               // Light Diffuse
var ksUniform = null;               // Material Diffuse
var materialShininessUniform = null;               // Material Diffuse

var lKeyPressedUniform = null;      // Light Key Pressed

let lights = new Array(2);

var materialAmbient     = new Float32Array([0.0, 0.0, 0.0, 1.0]);
var materialDiffuse      = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialSpecular    = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialShininess   = 128.0

var bLight = false; // Light On/Off
var bAnimate = false; // Animation On/Off

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
            bAnimate = !bAnimate;
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
    "in vec3 aNormal;\n" + 
    "uniform mat4 uModelMatrix;\n" + 
    "uniform mat4 uViewMatrix;\n" + 
    "uniform mat4 uProjectionMatrix;\n" + 
    "uniform vec3 uLa[2];\n" + 
    "uniform vec3 uLd[2];\n" + 
    "uniform vec3 uLs[2];\n" + 
    "uniform vec4 uLightPosition[2];\n" + 
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
    "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" + 
    "       vec3 lightSource[2];\n" + 
    "       float tnDotLd[2];\n" + 
    "       vec3 reflectedVector[2];\n" + 
    "       vec3 ambient[2];\n" + 
    "       vec3 diffuse[2];\n" + 
    "       vec3 specular[2];\n" + 
    "       out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
    "       for(int i = 0; i < 2; i++)\n" + 
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
    laUniform[0] = gl.getUniformLocation(shaderProgramObject, "uLa[0]");
    ldUniform[0] = gl.getUniformLocation(shaderProgramObject, "uLd[0]");
    lsUniform[0] = gl.getUniformLocation(shaderProgramObject, "uLs[0]");
    laUniform[1] = gl.getUniformLocation(shaderProgramObject, "uLa[1]");
    ldUniform[1] = gl.getUniformLocation(shaderProgramObject, "uLd[1]");
    lsUniform[1] = gl.getUniformLocation(shaderProgramObject, "uLs[1]");
    kaUniform = gl.getUniformLocation(shaderProgramObject, "uKa");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "uKd");
    ksUniform = gl.getUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform[0] = gl.getUniformLocation(shaderProgramObject, "uLightPosition[0]");
    lightPositionUniform[1] = gl.getUniformLocation(shaderProgramObject, "uLightPosition[1]");
    lKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uLKeyPressed");

    //Lights Seting
    lights[0] = new Light();
    lights[0].ambient = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[0].diffuse = new Float32Array([1.0, 0.0, 0.0, 1.0]);
    lights[0].specular = new Float32Array([1.0, 0.0, 0.0, 1.0]);
    lights[0].position = new Float32Array([-2.0, 0.0, 0.0, 1.0]);

    lights[1] = new Light();
    lights[1].ambient = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[1].diffuse = new Float32Array([0.0, 0.0, 1.0, 1.0]);
    lights[1].specular = new Float32Array([0.0, 0.0, 1.0, 1.0]);
    lights[1].position = new Float32Array([2.0, 0.0, 0.0, 1.0]);

    {
        var pyramidVertices = new Float32Array([
            // front
            0.0,  1.0,  0.0, // front-top
            -1.0, -1.0,  1.0, // front-left
            1.0, -1.0,  1.0, // front-right

            // right
            0.0,  1.0,  0.0, // right-top
            1.0, -1.0,  1.0, // right-left
            1.0, -1.0, -1.0, // right-right

            // back
            0.0,  1.0,  0.0, // back-top
            1.0, -1.0, -1.0, // back-left
            -1.0, -1.0, -1.0, // back-right

            // left
            0.0,  1.0,  0.0, // left-top
            -1.0, -1.0, -1.0, // left-left
            -1.0, -1.0,  1.0, // left-right
        ]);

        var pyramidNormals = new Float32Array([
            // front
            0.000000, 0.447214,  0.894427, // front-top
            0.000000, 0.447214,  0.894427, // front-left
            0.000000, 0.447214,  0.894427, // front-right
                                    
            // right			    
            0.894427, 0.447214,  0.000000, // right-top
            0.894427, 0.447214,  0.000000, // right-left
            0.894427, 0.447214,  0.000000, // right-right

            // back
            0.000000, 0.447214, -0.894427, // back-top
            0.000000, 0.447214, -0.894427, // back-left
            0.000000, 0.447214, -0.894427, // back-right

            // left
            -0.894427, 0.447214,  0.000000, // left-top
            -0.894427, 0.447214,  0.000000, // left-left
            -0.894427, 0.447214,  0.000000, // left-right
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

            vboNormalPyramid = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboNormalPyramid);
            {
                gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);
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

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateY(rotationMatrix, rotationMatrix, anglePyramid);
    mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    // Set light uniforms
    for(let i = 0; i < 2; i++){
        gl.uniform3fv(laUniform[i], lights[i].ambient.subarray(0, 3));
        gl.uniform3fv(ldUniform[i], lights[i].diffuse.subarray(0, 3));
        gl.uniform3fv(lsUniform[i], lights[i].specular.subarray(0, 3));
        gl.uniform4fv(lightPositionUniform[i], lights[i].position);
    }

    gl.uniform3fv(kaUniform, materialAmbient.subarray(0, 3));
    gl.uniform3fv(kdUniform, materialDiffuse.subarray(0, 3));
    gl.uniform3fv(ksUniform, materialSpecular.subarray(0, 3));
    gl.uniform1f(materialShininessUniform, materialShininess);

    gl.uniform1i(lKeyPressedUniform, bLight == true ? 1 : 0);

    gl.bindVertexArray(vaoPyramid);
        gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);

    if(bAnimate)
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
    if(vboNormalPyramid){
        gl.deleteBuffer(vboNormalPyramid);
        vboNormalPyramid = null;
    }
    if(shaderProgramObject){
        gl.useProgram(shaderProgramObject);
        var attachedShaders = gl.getAttachedShaders(shaderProgramObject);
        for(var i = null; i < attachedShaders.length; i++){
            gl.detachShader(shaderProgramObject, attachedShaders[i]);
            gl.deleteShader(attachedShaders[i]);
            attachedShaders[i] = null;
        }
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}
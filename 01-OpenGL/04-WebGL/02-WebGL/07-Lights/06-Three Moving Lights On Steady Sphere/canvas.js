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

var pvShaderProgramObject = null;
var pvModelMatrixUniform = null;
var pvViewMatrixUniform = null;
var pvProjectionMatrixUniform = null;

//Light related variables
var pvLaUniform = null;
var pvLdUniform = null;
var pvLsUniform = null;
var pvLightPositionUniform = null;

var pvKaUniform = null;
var pvKdUniform = null;
var pvKsUniform = null;
var pvMaterialShininessUniform = null;

var pvLKeyPressedUniform = null;

//PF
var pfShaderProgramObject = null;
var pfModelMatrixUniform = null;
var pfViewMatrixUniform = null;
var pfProjectionMatrixUniform = null;

//Light related variables
var pfLaUniform = null;
var pfLdUniform = null;
var pfLsUniform = null;
var pfLightPositionUniform = null;

var pfKaUniform = null;
var pfKdUniform = null;
var pfKsUniform = null;
var pfMaterialShininessUniform = null;

var pfLKeyPressedUniform = null;

let lights = new Array(3);

var materialAmbient     = new Float32Array([0.0, 0.0, 0.0, 1.0]);
var materialDiffuse     = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialSpecular    = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var materialShininess   = 128.0;

var perspectiveProjectionMatrix;
var bLight = false;
var bAnimation = false;
var perVertexperFragmentToggle = false;


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
        //home button
        case 36:
            bFullScreen = !bFullScreen;
            toggleFullscreen();  
        break;

        case 76: // 'L'
        case 108: // 'l'
            bLight = !bLight;
        break;

        case 65: // 'A'
        case 97: // 'a'
            bAnimation = !bAnimation;
        break;

        case 70:  // 'F'
        case 102: // 'f'
            perVertexperFragmentToggle = false;
        break;

        case 86:  // 'V'
        case 118: // 'v'
            perVertexperFragmentToggle = true;
        break;

        //for Q button
        case 81: // 'Q'
        case 113: // 'q'
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


    var pv_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    var pv_vertexShaderSourceCode =
    "#version 300 es\n" + 
    "precision highp float;\n" +
    "precision highp int;\n" +
    "in vec4 aPosition;\n" + 
    "in vec3 aNormal;\n" + 
    "uniform mat4 uModelMatrix;\n" + 
    "uniform mat4 uViewMatrix;\n" + 
    "uniform mat4 uProjectionMatrix;\n" + 
    "uniform vec3 uLa[3];\n" + 
    "uniform vec3 uLd[3];\n" + 
    "uniform vec3 uLs[3];\n" + 
    "uniform vec4 uLightPosition[3];\n" + 
    "uniform vec3 uKa;\n" + 
    "uniform vec3 uKd;\n" + 
    "uniform vec3 uKs;\n" + 
    "uniform float uMaterialShininess;\n" + 
    "uniform int  uLKeyPressed;\n" + 
    "out vec4 out_phong_ads_Light;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
    "   if(uLKeyPressed == 1)\n" + 
    "   {\n" + 
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
    "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
    "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n" + 
    "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" + 
    "       vec3 lightSource[3];\n" + 
    "       float tnDotLd[3];\n" + 
    "       vec3 reflectedVector[3];\n" + 
    "       vec3 ambient[3];\n" + 
    "       vec3 diffuse[3];\n" + 
    "       vec3 specular[3];\n" + 
    "       for(int i = 0; i < 3; i++)\n" + 
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

    gl.shaderSource(pv_vertexShaderObject, pv_vertexShaderSourceCode);
    gl.compileShader(pv_vertexShaderObject);
    var compileStatus = gl.getShaderParameter(pv_vertexShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(pv_vertexShaderObject);
        console.log('Vertex Shader Compilation Error: ' + error);
        return;
    }

    var pv_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    var pv_fragmentShaderSourceCode =
    "#version 300 es\n" + 
    "precision highp float;\n" +
    "precision highp int;\n" +
    "in vec4 out_phong_ads_Light;\n" + 
    "out vec4 FragColor;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "FragColor = out_phong_ads_Light;\n" + 
    "}\n";

    gl.shaderSource(pv_fragmentShaderObject, pv_fragmentShaderSourceCode);
    gl.compileShader(pv_fragmentShaderObject);
    compileStatus = gl.getShaderParameter(pv_fragmentShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(pv_fragmentShaderObject);
        console.log('Fragment Shader Compilation Error: ' + error);
        return;
    }

    pvShaderProgramObject = gl.createProgram();
    gl.attachShader(pvShaderProgramObject, pv_vertexShaderObject);
    gl.attachShader(pvShaderProgramObject, pv_fragmentShaderObject);

    gl.bindAttribLocation(pvShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(pvShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");

    gl.linkProgram(pvShaderProgramObject);

    pvModelMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uModelMatrix");
    pvViewMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uViewMatrix");
    pvProjectionMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uProjectionMatrix");

    pvLaUniform = new Array(3);
    pvLdUniform = new Array(3);
    pvLsUniform = new Array(3);
    pvLightPositionUniform = new Array(3);

    // Light uniforms
    pvLaUniform[0] = gl.getUniformLocation(pvShaderProgramObject, "uLa[0]");
    pvLdUniform[0] = gl.getUniformLocation(pvShaderProgramObject, "uLd[0]");
    pvLsUniform[0] = gl.getUniformLocation(pvShaderProgramObject, "uLs[0]");
    pvLightPositionUniform[0] = gl.getUniformLocation(pvShaderProgramObject, "uLightPosition[0]");

    pvLaUniform[1] = gl.getUniformLocation(pvShaderProgramObject, "uLa[1]");
    pvLdUniform[1] = gl.getUniformLocation(pvShaderProgramObject, "uLd[1]");
    pvLsUniform[1] = gl.getUniformLocation(pvShaderProgramObject, "uLs[1]");
    pvLightPositionUniform[1] = gl.getUniformLocation(pvShaderProgramObject, "uLightPosition[1]");

    pvLaUniform[2] = gl.getUniformLocation(pvShaderProgramObject, "uLa[2]");
    pvLdUniform[2] = gl.getUniformLocation(pvShaderProgramObject, "uLd[2]");
    pvLsUniform[2] = gl.getUniformLocation(pvShaderProgramObject, "uLs[2]");
    pvLightPositionUniform[2] = gl.getUniformLocation(pvShaderProgramObject, "uLightPosition[2]");

    pvKaUniform = gl.getUniformLocation(pvShaderProgramObject, "uKa");
    pvKdUniform = gl.getUniformLocation(pvShaderProgramObject, "uKd");
    pvKsUniform = gl.getUniformLocation(pvShaderProgramObject, "uKs");
    pvMaterialShininessUniform = gl.getUniformLocation(pvShaderProgramObject, "uMaterialShininess");
    pvLKeyPressedUniform = gl.getUniformLocation(pvShaderProgramObject, "uLKeyPressed");

    // Per fragment
    var pf_vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    var pf_vertexShaderSourceCode =
    "#version 300 es\n" + 
    "precision highp float;\n" +
    "precision highp int;\n" +
    "in vec4 aPosition;\n" + 
    "in vec3 aNormal;\n" + 
    "out vec4 eyeCoordinates;\n" + 
    "out vec3 transformedNormal;\n" + 
    "out vec3 lightSource[3];\n" + 
    "uniform mat4 uModelMatrix;\n" + 
    "uniform mat4 uViewMatrix;\n" + 
    "uniform mat4 uProjectionMatrix;\n" + 
    "uniform vec4 uLightPosition[3];\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
    "   transformedNormal = (normalMatrix * aNormal);\n" + 
    "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
    "   for(int i = 0; i < 3; i++)\n" + 
    "   {\n" + 
    "       lightSource[i] = (vec3(uLightPosition[i]) - eyeCoordinates.xyz);\n" + 
    "   }\n" + 
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" + 
    "}\n";

    gl.shaderSource(pf_vertexShaderObject, pf_vertexShaderSourceCode);
    gl.compileShader(pf_vertexShaderObject);
    var compileStatus = gl.getShaderParameter(pf_vertexShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(pf_vertexShaderObject);
        console.log('Vertex Shader Compilation Error: ' + error);
        return;
    }

    var pf_fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    var pf_fragmentShaderSourceCode =
    "#version 300 es\n" + 
    "precision highp float;\n" +
    "precision highp int;\n" +
    "in vec3 transformedNormal;\n" + 
    "in vec4 eyeCoordinates;\n" + 
    "in vec3 lightSource[3];\n" + 
    "uniform vec3 uLa[3];\n" + 
    "uniform vec3 uLd[3];\n" + 
    "uniform vec3 uLs[3];\n" + 
    "uniform vec3 uKa;\n" + 
    "uniform vec3 uKd;\n" + 
    "uniform vec3 uKs;\n" + 
    "uniform float uMaterialShininess;\n" + 
    "uniform int  uLKeyPressed;\n" + 
    "out vec4 FragColor;\n" + 
    "vec4 out_phong_ads_Light;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
    "   vec3 normalizedLightSource[3];\n" + 
    "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
    "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
    "   if(uLKeyPressed == 1)\n" + 
    "   {\n" + 
    "       float tnDotLd[3];\n" + 
    "       vec3 reflectedVector[3];\n" + 
    "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
    "       vec3 ambient[3];\n" + 
    "       vec3 diffuse[3];\n" + 
    "       vec3 specular[3];\n" + 
    "       for(int i = 0; i < 3; i++)\n" + 
    "       {\n" + 
    "           normalizedLightSource[i] = normalize(lightSource[i]);\n" + 
    "           tnDotLd[i] = max(dot(normalizedLightSource[i], normalizedTransformNormal), 0.0);\n" + 
    "           reflectedVector[i] = reflect(-normalizedLightSource[i], normalizedTransformNormal);\n" + 
    "           ambient[i] = uLa[i] * uKa;\n" + 
    "           diffuse[i] = uLd[i] * uKd * tnDotLd[i];\n" + 
    "           specular[i] = uLs[i] * uKs * pow(max(dot(reflectedVector[i], viewerVector), 0.0), uMaterialShininess);\n" + 
    "           out_phong_ads_Light = out_phong_ads_Light + vec4(ambient[i] + diffuse[i] + specular[i], 1.0);\n" + 
    "       }\n" + 
    "       FragColor = out_phong_ads_Light;\n" + 
    "   }\n" + 
    "   else\n" + 
    "   {\n" + 
    "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" + 
    "   }\n" + 
    "}\n";

    gl.shaderSource(pf_fragmentShaderObject, pf_fragmentShaderSourceCode);
    gl.compileShader(pf_fragmentShaderObject);
    compileStatus = gl.getShaderParameter(pf_fragmentShaderObject, gl.COMPILE_STATUS);
    if(compileStatus == false){
        var error = gl.getShaderInfoLog(pf_fragmentShaderObject);
        console.log('Fragment Shader Compilation Error: ' + error);
        return;
    }

    pfShaderProgramObject = gl.createProgram();
    gl.attachShader(pfShaderProgramObject, pf_vertexShaderObject);
    gl.attachShader(pfShaderProgramObject, pf_fragmentShaderObject);

    gl.bindAttribLocation(pfShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(pfShaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");

    gl.linkProgram(pfShaderProgramObject);

    pfModelMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uModelMatrix");
    pfViewMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uViewMatrix");
    pfProjectionMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uProjectionMatrix");

    pfLaUniform = new Array(3);
    pfLdUniform = new Array(3);
    pfLsUniform = new Array(3);
    pfLightPositionUniform = new Array(3);

    // Light uniforms
    pfLaUniform[0] = gl.getUniformLocation(pfShaderProgramObject, "uLa[0]");
    pfLdUniform[0] = gl.getUniformLocation(pfShaderProgramObject, "uLd[0]");
    pfLsUniform[0] = gl.getUniformLocation(pfShaderProgramObject, "uLs[0]");
    pfLightPositionUniform[0] = gl.getUniformLocation(pfShaderProgramObject, "uLightPosition[0]");

    pfLaUniform[1] = gl.getUniformLocation(pfShaderProgramObject, "uLa[1]");
    pfLdUniform[1] = gl.getUniformLocation(pfShaderProgramObject, "uLd[1]");
    pfLsUniform[1] = gl.getUniformLocation(pfShaderProgramObject, "uLs[1]");
    pfLightPositionUniform[1] = gl.getUniformLocation(pfShaderProgramObject, "uLightPosition[1]");

    pfLaUniform[2] = gl.getUniformLocation(pfShaderProgramObject, "uLa[2]");
    pfLdUniform[2] = gl.getUniformLocation(pfShaderProgramObject, "uLd[2]");
    pfLsUniform[2] = gl.getUniformLocation(pfShaderProgramObject, "uLs[2]");
    pfLightPositionUniform[2] = gl.getUniformLocation(pfShaderProgramObject, "uLightPosition[2]");

    pfKaUniform = gl.getUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = gl.getUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = gl.getUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = gl.getUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLKeyPressedUniform = gl.getUniformLocation(pfShaderProgramObject, "uLKeyPressed");

    lights[0] = new Light();
    lights[0].ambient = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[0].diffuse = new Float32Array([1.0, 0.0, 0.0, 1.0]);
    lights[0].specular = new Float32Array([1.0, 0.0, 0.0, 1.0]);
    lights[0].position = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[0].lightAngle = 0.0;

    lights[1] = new Light();
    lights[1].ambient = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[1].diffuse = new Float32Array([0.0, 1.0, 0.0, 1.0]);
    lights[1].specular = new Float32Array([0.0, 1.0, 0.0, 1.0]);
    lights[1].position = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[1].lightAngle = 0.0;

    lights[2] = new Light();
    lights[2].ambient = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[2].diffuse = new Float32Array([0.0, 0.0, 1.0, 1.0]);
    lights[2].specular = new Float32Array([0.0, 0.0, 1.0, 1.0]);
    lights[2].position = new Float32Array([0.0, 0.0, 0.0, 1.0]);
    lights[2].lightAngle = 0.0;

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

    

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    var lightTranslationMatrix = mat4.create();
    var lightRotationMatrix = mat4.create();
    var lightTransformMatrix = mat4.create();
    var result = vec4.create();

    if(perVertexperFragmentToggle == true){
        gl.useProgram(pvShaderProgramObject);

        mat4.identity(lightTranslationMatrix);
        mat4.identity(lightRotationMatrix);
        mat4.identity(lightTransformMatrix);

        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(pvModelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(pvViewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(pvProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(pvKaUniform, materialAmbient.subarray(0, 3));
        gl.uniform3fv(pvKdUniform, materialDiffuse.subarray(0, 3));
        gl.uniform3fv(pvKsUniform, materialSpecular.subarray(0, 3));
        gl.uniform1f(pvMaterialShininessUniform, materialShininess);

        gl.uniform1i(pvLKeyPressedUniform, bLight == true ? 1 : 0);

        //Light[0]
        gl.uniform3fv(pvLaUniform[0], lights[0].ambient.subarray(0, 3));
        gl.uniform3fv(pvLdUniform[0], lights[0].diffuse.subarray(0, 3));
        gl.uniform3fv(pvLsUniform[0], lights[0].specular.subarray(0, 3));

        mat4.rotateY(lightRotationMatrix, lightRotationMatrix, lights[0].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[0].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pvLightPositionUniform[0], result);

        //Light[1]
        gl.uniform3fv(pvLaUniform[1], lights[1].ambient.subarray(0, 3));
        gl.uniform3fv(pvLdUniform[1], lights[1].diffuse.subarray(0, 3));
        gl.uniform3fv(pvLsUniform[1], lights[1].specular.subarray(0, 3));

        mat4.rotateX(lightRotationMatrix, lightRotationMatrix, lights[1].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[1].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pvLightPositionUniform[1], result);

        //Light[2]
        gl.uniform3fv(pvLaUniform[2], lights[2].ambient.subarray(0, 3));
        gl.uniform3fv(pvLdUniform[2], lights[2].diffuse.subarray(0, 3));
        gl.uniform3fv(pvLsUniform[2], lights[2].specular.subarray(0, 3));

        mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, lights[2].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[2].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pvLightPositionUniform[2], result);
    }
    else{
        gl.useProgram(pfShaderProgramObject);

        mat4.identity(lightTranslationMatrix);
        mat4.identity(lightRotationMatrix);
        mat4.identity(lightTransformMatrix);

        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(pfModelMatrixUniform, false, modelMatrix);
        gl.uniformMatrix4fv(pfViewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(pfProjectionMatrixUniform, false, perspectiveProjectionMatrix);

        gl.uniform3fv(pfKaUniform, materialAmbient.subarray(0, 3));
        gl.uniform3fv(pfKdUniform, materialDiffuse.subarray(0, 3));
        gl.uniform3fv(pfKsUniform, materialSpecular.subarray(0, 3));
        gl.uniform1f(pfMaterialShininessUniform, materialShininess);

        gl.uniform1i(pfLKeyPressedUniform, bLight == true ? 1 : 0);

        //Light[0]
        gl.uniform3fv(pfLaUniform[0], lights[0].ambient.subarray(0, 3));
        gl.uniform3fv(pfLdUniform[0], lights[0].diffuse.subarray(0, 3));
        gl.uniform3fv(pfLsUniform[0], lights[0].specular.subarray(0, 3));

        mat4.rotateY(lightRotationMatrix, lightRotationMatrix, lights[0].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[0].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pfLightPositionUniform[0], result);

        //Light[1]
        gl.uniform3fv(pfLaUniform[1], lights[1].ambient.subarray(0, 3));
        gl.uniform3fv(pfLdUniform[1], lights[1].diffuse.subarray(0, 3));
        gl.uniform3fv(pfLsUniform[1], lights[1].specular.subarray(0, 3));

        mat4.rotateX(lightRotationMatrix, lightRotationMatrix, lights[1].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[1].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pfLightPositionUniform[1], result);

        //Light[2]
        gl.uniform3fv(pfLaUniform[2], lights[2].ambient.subarray(0, 3));
        gl.uniform3fv(pfLdUniform[2], lights[2].diffuse.subarray(0, 3));
        gl.uniform3fv(pfLsUniform[2], lights[2].specular.subarray(0, 3));

        mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, lights[2].lightAngle);
        mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, modelMatrix);

        vec4.transformMat4(result, lights[2].position, lightTransformMatrix);
        //result = vec3.fromValues(100.0, 100.0, 100.0, 1.0);
        gl.uniform4fv(pfLightPositionUniform[2], result);
    }

    sphere.draw();

    update();

    

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
    for(var i = 0; i < 3; i++){
        lights[i].lightAngle = (lights[i].lightAngle + 0.01) ;
    }
}

function uninitialize(){
    // code
    if(pvShaderProgramObject){
        gl.useProgram(pvShaderProgramObject);
        var attachedShaders = gl.getAttachedShaders(pvShaderProgramObject);
        for(var i = 0; i < attachedShaders.length; i++){
            gl.detachShader(pvShaderProgramObject, attachedShaders[i]);
            gl.deleteShader(attachedShaders[i]);
            attachedShaders[i] = null;
        }
        gl.deleteProgram(pvShaderProgramObject);
        pvShaderProgramObject = null;
    }

    if(pfShaderProgramObject){
        gl.useProgram(pfShaderProgramObject);
        var attachedShaders = gl.getAttachedShaders(pfShaderProgramObject);
        for(var i = 0; i < attachedShaders.length; i++){
            gl.detachShader(pfShaderProgramObject, attachedShaders[i]);
            gl.deleteShader(attachedShaders[i]);
            attachedShaders[i] = null;
        }
        gl.deleteProgram(pfShaderProgramObject);
        pfShaderProgramObject = null;
    }
}
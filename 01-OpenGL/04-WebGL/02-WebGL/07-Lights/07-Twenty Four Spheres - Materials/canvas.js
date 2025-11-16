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

var lightAmbient        = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var lightDiffuse        = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var lightSpecular       = new Float32Array([1.0, 1.0, 1.0, 1.0]);
var lightPosition       = new Float32Array([0.0, 0.0, 0.0, 1.0]);

var perspectiveProjectionMatrix;
var bLight = false;
var bAnimation = false;
var perVertexperFragmentToggle = false;

var angleForXRotation = 0.0;
var angleForYRotation = 0.0;
var angleForZRotation = 0.0;
var keyPressed = -1;

const materials = [
    // 1st column
    new Material(new Float32Array([0.0215, 0.1745, 0.0215, 1.0]), new Float32Array([0.07568, 0.61424, 0.07568, 1.0]), new Float32Array([0.633, 0.727811, 0.633, 1.0]), 0.6 * 128),
    new Material(new Float32Array([0.135, 0.2225, 0.1575, 1.0]), new Float32Array([0.54, 0.89, 0.63, 1.0]), new Float32Array([0.316228, 0.316228, 0.316228, 1.0]), 0.1 * 128),
    new Material(new Float32Array([0.05375, 0.05, 0.06625, 1.0]), new Float32Array([0.18275, 0.17, 0.22525, 1.0]), new Float32Array([0.332741, 0.328634, 0.346435, 1.0]), 0.3 * 128),
    new Material(new Float32Array([0.25, 0.20725, 0.20725, 1.0]), new Float32Array([1.0, 0.829, 0.829, 1.0]), new Float32Array([0.296648, 0.296648, 0.296648, 1.0]), 0.088 * 128),
    new Material(new Float32Array([0.1745, 0.01175, 0.01175, 1.0]), new Float32Array([0.61424, 0.04136, 0.04136, 1.0]), new Float32Array([0.727811, 0.626959, 0.626959, 1.0]), 0.6 * 128),
    new Material(new Float32Array([0.1, 0.18725, 0.1745, 1.0]), new Float32Array([0.396, 0.74151, 0.69102, 1.0]), new Float32Array([0.297254, 0.30829, 0.306678, 1.0]), 0.1 * 128),

    // 2nd column
    new Material(new Float32Array([0.329412, 0.223529, 0.027451, 1.0]), new Float32Array([0.780392, 0.568627, 0.113725, 1.0]), new Float32Array([0.992157, 0.941176, 0.807843, 1.0]), 0.21794872 * 128),
    new Material(new Float32Array([0.2125, 0.1275, 0.054, 1.0]), new Float32Array([0.714, 0.4284, 0.18144, 1.0]), new Float32Array([0.393548, 0.271906, 0.166721, 1.0]), 0.2 * 128),
    new Material(new Float32Array([0.25, 0.25, 0.25, 1.0]), new Float32Array([0.4, 0.4, 0.4, 1.0]), new Float32Array([0.774597, 0.774597, 0.774597, 1.0]), 0.6 * 128),
    new Material(new Float32Array([0.19125, 0.0735, 0.0225, 1.0]), new Float32Array([0.7038, 0.27048, 0.0828, 1.0]), new Float32Array([0.256777, 0.137622, 0.086014, 1.0]), 0.1 * 128),
    new Material(new Float32Array([0.24725, 0.1995, 0.0745, 1.0]), new Float32Array([0.75164, 0.60648, 0.22648, 1.0]), new Float32Array([0.628281, 0.555802, 0.366065, 1.0]), 0.4 * 128),
    new Material(new Float32Array([0.19225, 0.19225, 0.19225, 1.0]), new Float32Array([0.50754, 0.50754, 0.50754, 1.0]), new Float32Array([0.508273, 0.508273, 0.508273, 1.0]), 0.4 * 128),

    // 3rd column
    new Material(new Float32Array([0.0, 0.0, 0.0, 1.0]), new Float32Array([0.01, 0.01, 0.01, 1.0]), new Float32Array([0.5, 0.5, 0.5, 1.0]), 0.25 * 128),
    new Material(new Float32Array([0.0, 0.1, 0.06, 1.0]), new Float32Array([0.0, 0.50980392, 0.50980392, 1.0]), new Float32Array([0.50980392, 0.50980392, 0.50980392, 1.0]), 0.25 * 128),
    new Material(new Float32Array([0.0, 0.0, 0.0, 1.0]), new Float32Array([0.1, 0.35, 0.1, 1.0]), new Float32Array([0.45, 0.55, 0.45, 1.0]), 0.25 * 128),
    new Material(new Float32Array([0.0, 0.0, 0.0, 1.0]), new Float32Array([0.5, 0.0, 0.0, 1.0]), new Float32Array([0.7, 0.6, 0.6, 1.0]), 0.25 * 128),
    new Material(new Float32Array([0.0, 0.0, 0.0, 1.0]), new Float32Array([0.55, 0.55, 0.55, 1.0]), new Float32Array([0.7, 0.7, 0.7, 1.0]), 0.25 * 128),
    new Material(new Float32Array([0.0, 0.0, 0.0, 1.0]), new Float32Array([0.5, 0.5, 0.0, 1.0]), new Float32Array([0.6, 0.6, 0.5, 1.0]), 0.25 * 128),

    // 4th column
    new Material(new Float32Array([0.02, 0.02, 0.02, 1.0]), new Float32Array([0.01, 0.01, 0.01, 1.0]), new Float32Array([0.4, 0.4, 0.4, 1.0]), 0.078125 * 128),
    new Material(new Float32Array([0.0, 0.05, 0.05, 1.0]), new Float32Array([0.4, 0.5, 0.5, 1.0]), new Float32Array([0.04, 0.7, 0.7, 1.0]), 0.078125 * 128),
    new Material(new Float32Array([0.0, 0.05, 0.0, 1.0]), new Float32Array([0.4, 0.5, 0.4, 1.0]), new Float32Array([0.04, 0.7, 0.04, 1.0]), 0.078125 * 128),
    new Material(new Float32Array([0.05, 0.0, 0.0, 1.0]), new Float32Array([0.5, 0.4, 0.4, 1.0]), new Float32Array([0.7, 0.04, 0.04, 1.0]), 0.078125 * 128),
    new Material(new Float32Array([0.05, 0.05, 0.05, 1.0]), new Float32Array([0.5, 0.5, 0.5, 1.0]), new Float32Array([0.7, 0.7, 0.7, 1.0]), 0.078125 * 128),
    new Material(new Float32Array([0.05, 0.05, 0.0, 1.0]), new Float32Array([0.5, 0.5, 0.4, 1.0]), new Float32Array([0.7, 0.7, 0.04, 1.0]), 0.078125 * 128),
];

const sphereTranslation = [
    [1.5, 14.0, -20.0],
    [1.5, 11.5, -20.0],
    [1.5, 9.0, -20.0],
    [1.5, 6.5, -20.0],
    [1.5, 4.0, -20.0],
    [1.5, 1.5, -20.0],

    [7.5, 14.0, -20.0],
    [7.5, 11.5, -20.0],
    [7.5, 9.0, -20.0],
    [7.5, 6.5, -20.0],
    [7.5, 4.0, -20.0],
    [7.5, 1.5, -20.0],

    [13.5, 14.0, -20.0],
    [13.5, 11.5, -20.0],
    [13.5, 9.0, -20.0],
    [13.5, 6.5, -20.0],
    [13.5, 4.0, -20.0],
    [13.5, 1.5, -20.0],

    [19.5, 14.0, -20.0],
    [19.5, 11.5, -20.0],
    [19.5, 9.0, -20.0],
    [19.5, 6.5, -20.0],
    [19.5, 4.0, -20.0],
    [19.5, 1.5, -20.0],
];


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

        case 88:
            angleForXRotation = 0.0;
            keyPressed = 1;
        break;

        case 89:
            angleForYRotation = 0.0;
            keyPressed = 2;
        break;

        case 90:
            angleForZRotation = 0.0;
            keyPressed = 3;
        break;

        //for Q button
        case 81: // 'Q'
        case 113: // 'q'
            uninitialize();
            window.close();
        break;

        default : 
            keyPressed = -1;
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

    // Light uniforms
    pvLaUniform = gl.getUniformLocation(pvShaderProgramObject, "uLa");
    pvLdUniform = gl.getUniformLocation(pvShaderProgramObject, "uLd");
    pvLsUniform = gl.getUniformLocation(pvShaderProgramObject, "uLs");
    pvLightPositionUniform = gl.getUniformLocation(pvShaderProgramObject, "uLightPosition");
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
    "out vec3 lightSource;\n" + 
    "uniform mat4 uModelMatrix;\n" + 
    "uniform mat4 uViewMatrix;\n" + 
    "uniform mat4 uProjectionMatrix;\n" + 
    "uniform vec4 uLightPosition;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
    "   transformedNormal = (normalMatrix * aNormal);\n" + 
    "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
    "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" + 
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
    "in vec3 lightSource;\n" + 
    "uniform vec3 uLa;\n" + 
    "uniform vec3 uLd;\n" + 
    "uniform vec3 uLs;\n" + 
    "uniform vec3 uKa;\n" + 
    "uniform vec3 uKd;\n" + 
    "uniform vec3 uKs;\n" + 
    "uniform float uMaterialShininess;\n" + 
    "uniform int  uLKeyPressed;\n" + 
    "out vec4 FragColor;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
    "   vec3 normalizedLightSource = normalize(lightSource);\n" + 
    "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
    "   if(uLKeyPressed == 1)\n" + 
    "   {\n" + 
    "       float tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n" + 
    "       vec3 reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n" + 
    "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
    "       vec3 ambient = uLa * uKa;\n" + 
    "       vec3 diffuse = uLd * uKd * tnDotLd;\n" + 
    "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" + 
    "       FragColor = vec4(ambient + diffuse + specular, 1.0);\n" + 
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

    // Light uniforms
    pfLaUniform = gl.getUniformLocation(pfShaderProgramObject, "uLa");
    pfLdUniform = gl.getUniformLocation(pfShaderProgramObject, "uLd");
    pfLsUniform = gl.getUniformLocation(pfShaderProgramObject, "uLs");
    pfLightPositionUniform = gl.getUniformLocation(pfShaderProgramObject, "uLightPosition");
    pfKaUniform = gl.getUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = gl.getUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = gl.getUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = gl.getUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLKeyPressedUniform = gl.getUniformLocation(pfShaderProgramObject, "uLKeyPressed");

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
    var result = vec4.create();

    var lightRotationMatrix = mat4.create();
    var lightTranslationMatrix = mat4.create();
    var lightTransformMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(lightRotationMatrix);
    mat4.identity(lightTranslationMatrix);
    mat4.identity(lightTransformMatrix);

    if(perVertexperFragmentToggle == true){
        gl.useProgram(pvShaderProgramObject);

        // Set light uniforms
        gl.uniform3fv(pvLaUniform, lightAmbient.subarray(0, 3));
        gl.uniform3fv(pvLdUniform, lightDiffuse.subarray(0, 3));
        gl.uniform3fv(pvLsUniform, lightSpecular.subarray(0, 3));
        gl.uniform1i(pvLKeyPressedUniform, bLight == true ? 1 : 0);

        if(keyPressed == 1){
            mat4.rotateX(lightRotationMatrix, lightRotationMatrix, angleForXRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        }
        else if(keyPressed == 2){
            mat4.rotateY(lightRotationMatrix, lightRotationMatrix, angleForYRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        }
        else if(keyPressed == 3){
            mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, angleForZRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        }
        else{
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [10.0, 10.0, 10.0]);
        }

        for(var i = 0; i < 24; i++){
            mat4.identity(modelMatrix);
            mat4.identity(viewMatrix);
            mat4.identity(modelViewProjectionMatrix);
            mat4.identity(lightTransformMatrix);

            mat4.translate(modelMatrix, modelMatrix, [-10.0, -8.0, 0.0]);
            mat4.translate(modelMatrix, modelMatrix, [sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]]);
            mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
            mat4.multiply(lightTransformMatrix, modelMatrix, lightTransformMatrix);
            vec4.transformMat4(result, lightPosition, lightTransformMatrix);
            gl.uniform4fv(pvLightPositionUniform, result);

            gl.uniform3fv(pvKaUniform, materials[i].ambient.subarray(0, 3));
            gl.uniform3fv(pvKdUniform, materials[i].diffuse.subarray(0, 3));
            gl.uniform3fv(pvKsUniform, materials[i].specular.subarray(0, 3));
            gl.uniform1f(pvMaterialShininessUniform, materials[i].shininess);

            gl.uniformMatrix4fv(pvModelMatrixUniform, false, modelMatrix);
            gl.uniformMatrix4fv(pvViewMatrixUniform, false, viewMatrix);
            gl.uniformMatrix4fv(pvProjectionMatrixUniform, false, perspectiveProjectionMatrix);

            sphere.draw();
        }
    }
    else{
        gl.useProgram(pfShaderProgramObject);

        // Set light uniforms
        gl.uniform3fv(pfLaUniform, lightAmbient.subarray(0, 3));
        gl.uniform3fv(pfLdUniform, lightDiffuse.subarray(0, 3));
        gl.uniform3fv(pfLsUniform, lightSpecular.subarray(0, 3));
        gl.uniform1i(pfLKeyPressedUniform, bLight == true ? 1 : 0);

        if(keyPressed == 1){
            mat4.rotateX(lightRotationMatrix, lightRotationMatrix, angleForXRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        }
        else if(keyPressed == 2){
            mat4.rotateY(lightRotationMatrix, lightRotationMatrix, angleForYRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        }
        else if(keyPressed == 3){
            mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, angleForZRotation);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        }
        else{
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [10.0, 10.0, 10.0]);
        }

        for(var i = 0; i < 24; i++){
            mat4.identity(modelMatrix);
            mat4.identity(viewMatrix);
            mat4.identity(modelViewProjectionMatrix);
            mat4.identity(lightTransformMatrix);

            mat4.translate(modelMatrix, modelMatrix, [-10.0, -8.0, 0.0]);
            mat4.translate(modelMatrix, modelMatrix, [sphereTranslation[i][0], sphereTranslation[i][1], sphereTranslation[i][2]]);
            mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            mat4.multiply(lightTransformMatrix, lightRotationMatrix, lightTranslationMatrix);
            mat4.multiply(lightTransformMatrix, modelMatrix, lightTransformMatrix);
            vec4.transformMat4(result, lightPosition, lightTransformMatrix);
            gl.uniform4fv(pfLightPositionUniform, result);

            gl.uniform3fv(pfKaUniform, materials[i].ambient.subarray(0, 3));
            gl.uniform3fv(pfKdUniform, materials[i].diffuse.subarray(0, 3));
            gl.uniform3fv(pfKsUniform, materials[i].specular.subarray(0, 3));
            gl.uniform1f(pfMaterialShininessUniform, materials[i].shininess);

            gl.uniformMatrix4fv(pfModelMatrixUniform, false, modelMatrix);
            gl.uniformMatrix4fv(pfViewMatrixUniform, false, viewMatrix);
            gl.uniformMatrix4fv(pfProjectionMatrixUniform, false, perspectiveProjectionMatrix);

            sphere.draw();
        }
    }

    //if(bAnimation)
        update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    if(bLight){
        angleForXRotation = angleForXRotation + 0.05;
        angleForYRotation = angleForYRotation + 0.05;
        angleForZRotation = angleForZRotation + 0.05;
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
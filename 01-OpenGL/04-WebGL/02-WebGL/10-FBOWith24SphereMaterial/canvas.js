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

const MyAttributesSphere = {
    AMC_ATTRIBUTE_POSITION_SPHERE: 0,
    AMC_ATTRIBUTE_COLOR_SPHERE: 1,
    AMC_ATTRIBUTE_NORMAL_SPHERE: 2,
    AMC_ATTRIBUTE_TEXTURE0_SPHERE: 3,
    AMC_ATTRIBUTE_TEXTURE1_SPHERE: 4,
};

var shaderProgramObject = null;
var mvpMatrixUniform = null;

var vaoCube = null;
var vboPositionCube = null;

var perspectiveProjectionMatrixCube;

var angleCube = 0.0;

//Texture related variables
var textureSamplerUniform = null;

/* Sphere related variables */
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

// Materials array
var materials = [
    // 1st sphere on 1st column, emerald
    {ambient: [0.0215, 0.1745, 0.0215, 1.0], diffuse: [0.07568, 0.61424, 0.07568, 1.0], specular: [0.633, 0.727811, 0.633, 1.0], shininess: 0.6 * 128},
    // 2nd sphere on 1st column, jade
    {ambient: [0.135, 0.2225, 0.1575, 1.0], diffuse: [0.54, 0.89, 0.63, 1.0], specular: [0.316228, 0.316228, 0.316228, 1.0], shininess: 0.1 * 128},
    // 3rd sphere on 1st column, obsidian
    {ambient: [0.05375, 0.05, 0.06625, 1.0], diffuse: [0.18275, 0.17, 0.22525, 1.0], specular: [0.332741, 0.328634, 0.346435, 1.0], shininess: 0.3 * 128},
    // 4th sphere on 1st column, pearl
    {ambient: [0.25, 0.20725, 0.20725, 1.0], diffuse: [1.0, 0.829, 0.829, 1.0], specular: [0.296648, 0.296648, 0.296648, 1.0], shininess: 0.088 * 128},
    // 5th sphere on 1st column, ruby
    {ambient: [0.1745, 0.01175, 0.01175, 1.0], diffuse: [0.61424, 0.04136, 0.04136, 1.0], specular: [0.727811, 0.626959, 0.626959, 1.0], shininess: 0.6 * 128},
    // 6th sphere on 1st column, turquoise
    {ambient: [0.1, 0.18725, 0.1745, 1.0], diffuse: [0.396, 0.74151, 0.69102, 1.0], specular: [0.297254, 0.30829, 0.306678, 1.0], shininess: 0.1 * 128},
    // 1st sphere on 2nd column, brass
    {ambient: [0.329412, 0.223529, 0.027451, 1.0], diffuse: [0.780392, 0.568627, 0.113725, 1.0], specular: [0.992157, 0.941176, 0.807843, 1.0], shininess: 0.21794872 * 128},
    // 2nd sphere on 2nd column, bronze
    {ambient: [0.2125, 0.1275, 0.054, 1.0], diffuse: [0.714, 0.4284, 0.18144, 1.0], specular: [0.393548, 0.271906, 0.166721, 1.0], shininess: 0.2 * 128},
    // 3rd sphere on 2nd column, chrome
    {ambient: [0.25, 0.25, 0.25, 1.0], diffuse: [0.4, 0.4, 0.4, 1.0], specular: [0.774597, 0.774597, 0.774597, 1.0], shininess: 0.6 * 128},
    // 4th sphere on 2nd column, copper
    {ambient: [0.19125, 0.0735, 0.0225, 1.0], diffuse: [0.7038, 0.27048, 0.0828, 1.0], specular: [0.256777, 0.137622, 0.086014, 1.0], shininess: 0.1 * 128},
    // 5th sphere on 2nd column, gold
    {ambient: [0.24725, 0.1995, 0.0745, 1.0], diffuse: [0.75164, 0.60648, 0.22648, 1.0], specular: [0.628281, 0.555802, 0.366065, 1.0], shininess: 0.4 * 128},
    // 6th sphere on 2nd column, silver
    {ambient: [0.19225, 0.19225, 0.19225, 1.0], diffuse: [0.50754, 0.50754, 0.50754, 1.0], specular: [0.508273, 0.508273, 0.508273, 1.0], shininess: 0.4 * 128},
    // 1st sphere on 3rd column, black
    {ambient: [0.0, 0.0, 0.0, 1.0], diffuse: [0.01, 0.01, 0.01, 1.0], specular: [0.50, 0.50, 0.50, 1.0], shininess: 0.25 * 128},
    // 2nd sphere on 3rd column, cyan
    {ambient: [0.0, 0.1, 0.06, 1.0], diffuse: [0.0, 0.50980392, 0.50980392, 1.0], specular: [0.50980392, 0.50980392, 0.50980392, 1.0], shininess: 0.25 * 128},
    // 3rd sphere on 3rd column, green
    {ambient: [0.0, 0.0, 0.0, 1.0], diffuse: [0.1, 0.35, 0.1, 1.0], specular: [0.45, 0.55, 0.45, 1.0], shininess: 0.25 * 128},
    // 4th sphere on 3rd column, red
    {ambient: [0.0, 0.0, 0.0, 1.0], diffuse: [0.5, 0.0, 0.0, 1.0], specular: [0.7, 0.6, 0.6, 1.0], shininess: 0.25 * 128},
    // 5th sphere on 3rd column, white
    {ambient: [0.0, 0.0, 0.0, 1.0], diffuse: [0.55, 0.55, 0.55, 1.0], specular: [0.70, 0.70, 0.70, 1.0], shininess: 0.25 * 128},
    // 6th sphere on 3rd column, yellow
    {ambient: [0.0, 0.0, 0.0, 1.0], diffuse: [0.5, 0.5, 0.0, 1.0], specular: [0.60, 0.60, 0.50, 1.0], shininess: 0.25 * 128},
    // 1st sphere on 4th column, black
    {ambient: [0.02, 0.02, 0.02, 1.0], diffuse: [0.01, 0.01, 0.01, 1.0], specular: [0.4, 0.4, 0.4, 1.0], shininess: 0.078125 * 128},
    // 2nd sphere on 4th column, cyan
    {ambient: [0.0, 0.05, 0.05, 1.0], diffuse: [0.4, 0.5, 0.5, 1.0], specular: [0.04, 0.7, 0.7, 1.0], shininess: 0.078125 * 128},
    // 3rd sphere on 4th column, green
    {ambient: [0.0, 0.05, 0.0, 1.0], diffuse: [0.4, 0.5, 0.4, 1.0], specular: [0.04, 0.7, 0.04, 1.0], shininess: 0.078125 * 128},
    // 4th sphere on 4th column, red
    {ambient: [0.05, 0.0, 0.0, 1.0], diffuse: [0.5, 0.4, 0.4, 1.0], specular: [0.7, 0.04, 0.04, 1.0], shininess: 0.078125 * 128},
    // 5th sphere on 4th column, white
    {ambient: [0.05, 0.05, 0.05, 1.0], diffuse: [0.5, 0.5, 0.5, 1.0], specular: [0.7, 0.7, 0.7, 1.0], shininess: 0.078125 * 128},
    // 6th sphere on 4th column, yellow
    {ambient: [0.05, 0.05, 0.0, 1.0], diffuse: [0.5, 0.5, 0.4, 1.0], specular: [0.7, 0.7, 0.04, 1.0], shininess: 0.078125 * 128}
];

var perspectiveProjectionMatrixSphere;
var bLight = false;
var bAnimation = false;
var perVertexperFragmentToggle = false;

// Sphere rotation variables
var angleForXRotation_sphere = 0.0;
var angleForYRotation_sphere = 0.0;
var angleForZRotation_sphere = 0.0;
var keyPressed_sphere = -1;
var materialChoice = 0;

//Sphere
var sphere = null;

//FBO
var winWidth;
var winHeight;
var fboWidth = 512;
var fboHeight = 512;
var fbo; // Frame Buffer Object
var rbo; // Render Buffer Object
var fboTexture; // Texture attached to FBO
var fboResult = -1; // To check FBO completeness

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

        case 88: // 'X'
        case 120: // 'x'
            angleForXRotation_sphere = 0.0;
            keyPressed_sphere = 1;
        break;

        case 89: // 'Y'
        case 121: // 'y'
            angleForYRotation_sphere = 0.0;
            keyPressed_sphere = 2;
        break;

        case 90: // 'Z'
        case 122: // 'z'
            angleForZRotation_sphere = 0.0;
            keyPressed_sphere = 3;
        break;

        case 78: // 'N'
        case 110: // 'n'
            materialChoice = (materialChoice + 1) % 24;
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
    console.log(shaderProgramObject);
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_TEXCOORD0, "aTexCoord");

    gl.linkProgram(shaderProgramObject);

    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "uTextureSampler");

    
    {
        var cubeVertices = new Float32Array([
                1.0, 1.0, -1.0,
                -1.0, 1.0, -1.0,
                -1.0, 1.0, 1.0,
                1.0, 1.0, 1.0,

                
                1.0, -1.0, 1.0,
                -1.0, -1.0, 1.0,
                -1.0, -1.0, -1.0,
                1.0, -1.0, -1.0,

                
                1.0, 1.0, 1.0,
                -1.0, 1.0, 1.0,
                -1.0, -1.0, 1.0,
                1.0, -1.0, 1.0,

                
                1.0, -1.0, -1.0,
                -1.0, -1.0, -1.0,
                -1.0, 1.0, -1.0,
                1.0, 1.0, -1.0,

                
                1.0, 1.0, -1.0,
                1.0, 1.0, 1.0,
                1.0, -1.0, 1.0,
                1.0, -1.0, -1.0,

                
                -1.0, 1.0, 1.0,
                -1.0, 1.0, -1.0,
                -1.0, -1.0, -1.0,
                -1.0, -1.0, 1.0
        ]);

        var cubeTexcoords = new Float32Array([
            // front
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,

            // right
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,

            // back
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,

            // left
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,

            // top
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,

            // bottom
            1.0, 1.0,
            0.0, 1.0,
            0.0, 0.0,
            1.0, 0.0,
        ]);

        vaoCube = gl.createVertexArray();
        gl.bindVertexArray(vaoCube);
        {
            vboPositionCube = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboPositionCube);
            {
                gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);

            var vboTexCoordCube = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vboTexCoordCube);
            {
                gl.bufferData(gl.ARRAY_BUFFER, cubeTexcoords, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD0);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    // Initialize projection matrix
    perspectiveProjectionMatrixCube = mat4.create();
    mat4.identity(perspectiveProjectionMatrixCube);

    // Set clear color to blue
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clearColor(1.0, 1.0, 1.0, 1.0);

    if(createAndPrepareFBOForDrawing(fboWidth, fboHeight)){
        console.log('AKM: FBO created successfully');
        fboResult = initializeSphere();
        if(fboResult != 0){
            console.log('AKM: InitializeSphere() failed with error code');
        }
        else{
            console.log('AKM: InitializeSphere() succeeded');
        }
    }
}

function initializeSphere(){
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
    "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
    "   if(uLKeyPressed == 1)\n" + 
    "   {\n" + 
    "       vec4 eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" + 
    "       mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" + 
    "       vec3 transformedNormal = normalize(normalMatrix * aNormal);\n" + 
    "       vec3 viewerVector = normalize(-eyeCoordinates.xyz);\n" + 
    "       vec3 lightSource = normalize(vec3(uLightPosition) - eyeCoordinates.xyz);\n" + 
    "       float tnDotLd = max(dot(lightSource, transformedNormal), 0.0);\n" + 
    "       vec3 reflectedVector = reflect(-lightSource, transformedNormal);\n" + 
    "       vec3 ambient = uLa * uKa;\n" + 
    "       vec3 diffuse = uLd * uKd * tnDotLd;\n" + 
    "       vec3 specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" + 
    "       out_phong_ads_Light = out_phong_ads_Light + vec4(ambient + diffuse + specular, 1.0);\n" + 
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

    gl.bindAttribLocation(pvShaderProgramObject, MyAttributesSphere.AMC_ATTRIBUTE_POSITION_SPHERE, "aPosition");
    gl.bindAttribLocation(pvShaderProgramObject, MyAttributesSphere.AMC_ATTRIBUTE_NORMAL_SPHERE, "aNormal");

    gl.linkProgram(pvShaderProgramObject);

    pvModelMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uModelMatrix");
    pvViewMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uViewMatrix");
    pvProjectionMatrixUniform = gl.getUniformLocation(pvShaderProgramObject, "uProjectionMatrix");

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
    "vec4 out_phong_ads_Light;\n" + 
    "void main(void)\n" + 
    "{\n" + 
    "   out_phong_ads_Light = vec4(0.0, 0.0, 0.0, 1.0);\n" + 
    "   vec3 normalizedLightSource;\n" + 
    "   vec3 normalizedTransformNormal = normalize(transformedNormal);\n" + 
    "   vec3 normalizedViewerVector = normalize(eyeCoordinates.xyz);\n" + 
    "   if(uLKeyPressed == 1)\n" + 
    "   {\n" + 
    "       float tnDotLd;\n" + 
    "       vec3 reflectedVector;\n" + 
    "       vec3 viewerVector = (-normalizedViewerVector.xyz);\n" + 
    "       vec3 ambient;\n" + 
    "       vec3 diffuse;\n" + 
    "       vec3 specular;\n" + 
    "       normalizedLightSource = normalize(lightSource);\n" + 
    "       tnDotLd = max(dot(normalizedLightSource, normalizedTransformNormal), 0.0);\n" + 
    "       reflectedVector = reflect(-normalizedLightSource, normalizedTransformNormal);\n" + 
    "       ambient = uLa * uKa;\n" + 
    "       diffuse = uLd * uKd * tnDotLd;\n" + 
    "       specular = uLs * uKs * pow(max(dot(reflectedVector, viewerVector), 0.0), uMaterialShininess);\n" + 
    "       out_phong_ads_Light = out_phong_ads_Light + vec4(ambient + diffuse + specular, 1.0);\n" + 
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

    gl.bindAttribLocation(pfShaderProgramObject, MyAttributesSphere.AMC_ATTRIBUTE_POSITION_SPHERE, "aPosition");
    gl.bindAttribLocation(pfShaderProgramObject, MyAttributesSphere.AMC_ATTRIBUTE_NORMAL_SPHERE, "aNormal");

    gl.linkProgram(pfShaderProgramObject);

    pfModelMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uModelMatrix");
    pfViewMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uViewMatrix");
    pfProjectionMatrixUniform = gl.getUniformLocation(pfShaderProgramObject, "uProjectionMatrix");

    pfLaUniform = gl.getUniformLocation(pfShaderProgramObject, "uLa");
    pfLdUniform = gl.getUniformLocation(pfShaderProgramObject, "uLd");
    pfLsUniform = gl.getUniformLocation(pfShaderProgramObject, "uLs");
    pfLightPositionUniform = gl.getUniformLocation(pfShaderProgramObject, "uLightPosition");

    pfKaUniform = gl.getUniformLocation(pfShaderProgramObject, "uKa");
    pfKdUniform = gl.getUniformLocation(pfShaderProgramObject, "uKd");
    pfKsUniform = gl.getUniformLocation(pfShaderProgramObject, "uKs");
    pfMaterialShininessUniform = gl.getUniformLocation(pfShaderProgramObject, "uMaterialShininess");
    pfLKeyPressedUniform = gl.getUniformLocation(pfShaderProgramObject, "uLKeyPressed");

    lights[0] = new Light();
    lights[0].ambient = new Float32Array([1.0, 1.0, 1.0, 1.0]);
    lights[0].diffuse = new Float32Array([1.0, 1.0, 1.0, 1.0]);
    lights[0].specular = new Float32Array([1.0, 1.0, 1.0, 1.0]);
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
    perspectiveProjectionMatrixSphere = mat4.create();
    mat4.identity(perspectiveProjectionMatrixSphere);

    // Set clear color to blue
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    resizeSphere(fboWidth, fboHeight);

    return 0;
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

    mat4.perspective(perspectiveProjectionMatrixCube, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function resizeSphere(width, height){
    gl.viewport(0, 0, width, height);
    mat4.perspective(perspectiveProjectionMatrixSphere, 45.0, width / height, 0.1, 100.0);
}

function createAndPrepareFBOForDrawing(textureWidth, textureHeight) {
    // Query max texture size
    const maxTexSize = gl.getParameter(gl.MAX_TEXTURE_SIZE);
    if (textureWidth > maxTexSize || textureHeight > maxTexSize) {
        console.error(`Requested texture size (${textureWidth} x ${textureHeight}) exceeds max texture size (${maxTexSize}).`);
        return false;
    }

    // Create framebuffer
    fbo = gl.createFramebuffer();
    if (!fbo) {
        console.error('Failed to create framebuffer.');
        return false;
    }
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

    // Create renderbuffer for depth
    rbo = gl.createRenderbuffer();
    if (!rbo) {
        console.error('Failed to create renderbuffer.');
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        return false;
    }
    gl.bindRenderbuffer(gl.RENDERBUFFER, rbo);

    // Allocate depth storage
    // DEPTH_COMPONENT16 is available in both WebGL1 and WebGL2 for renderbuffers
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, textureWidth, textureHeight);

    // Create texture to attach as color target
    fboTexture = gl.createTexture();
    if (!fboTexture) {
        console.error('Failed to create texture for FBO.');
        // cleanup
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        return false;
    }
    gl.bindTexture(gl.TEXTURE_2D, fboTexture);

    // Texture params
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    // Allocate texture storage (use same internalformat and format: RGBA + UNSIGNED_BYTE)
    // Note: in WebGL1 internalFormat === format, which is gl.RGBA
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, textureWidth, textureHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // Attach texture and renderbuffer to framebuffer
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fboTexture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, rbo);

    // Check completeness
    const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
    if (status !== gl.FRAMEBUFFER_COMPLETE) {
        console.error(`Framebuffer is not complete: 0x${status.toString(16)}`);
        // Cleanup / unbind
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);

        // Delete created objects if desired
        try { gl.deleteTexture(fboTexture); } catch (e) {}
        try { gl.deleteRenderbuffer(rbo); } catch (e) {}
        try { gl.deleteFramebuffer(fbo); } catch (e) {}

        return false;
    }

    // Unbind to leave GL state clean
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    return true;
}


function display(){
    //code
    if(fboResult == 0){
        displaySphere();
    }

    resize();

    gl.clearColor(1.0, 1.0, 1.0, 1.0);
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

    mat4.rotateX(rotationMatrix, rotationMatrix, angleCube);
    mat4.rotateY(rotationMatrix, rotationMatrix, angleCube);
    mat4.rotateZ(rotationMatrix, rotationMatrix, angleCube);
    mat4.scale(scaleMatrix, scaleMatrix, [0.75, 0.75, 0.75]);
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -6.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, modelViewMatrix, scaleMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrixCube, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    //bind kundali texture
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, fboTexture);
    gl.uniform1i(textureSamplerUniform, 0);

    gl.bindVertexArray(vaoCube);
    {
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
    }
    gl.bindVertexArray(null);

    update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function displaySphere(){
    if(fbo){
        gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    }

    resizeSphere(fboWidth, fboHeight);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.scale(modelMatrix, modelMatrix, [0.75, 0.75, 0.75]);
    mat4.lookAt(viewMatrix, [0.0, 0.0, 2.0], [0.0, 0.0, 0.0], [0.0, 1.0, 0.0]);

    var lightRotationMatrix = mat4.create();
    var lightTranslationMatrix = mat4.create();
    var lightTransformMatrix = mat4.create();
    var result = vec4.create();

    if(perVertexperFragmentToggle == true){
        gl.useProgram(pvShaderProgramObject);

        gl.uniformMatrix4fv(pvViewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(pvProjectionMatrixUniform, false, perspectiveProjectionMatrixSphere);
        gl.uniform3fv(pvLaUniform, lights[0].ambient.subarray(0, 3));
        gl.uniform3fv(pvLdUniform, lights[0].diffuse.subarray(0, 3));
        gl.uniform3fv(pvLsUniform, lights[0].specular.subarray(0, 3));
        gl.uniform1i(pvLKeyPressedUniform, bLight == true ? 1 : 0);

        if(keyPressed_sphere == 1){
            mat4.rotateX(lightRotationMatrix, lightRotationMatrix, angleForXRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        }
        else if(keyPressed_sphere == 2){
            mat4.rotateY(lightRotationMatrix, lightRotationMatrix, angleForYRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        }
        else if(keyPressed_sphere == 3){
            mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, angleForZRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        }
        else{
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [10.0, 10.0, 10.0]);
        }

        mat4.multiply(lightTransformMatrix, modelMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, lightRotationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, lightTranslationMatrix);

        vec4.transformMat4(result, lights[0].position, lightTransformMatrix);
        gl.uniform4fv(pvLightPositionUniform, result);

        gl.uniformMatrix4fv(pvModelMatrixUniform, false, modelMatrix);
        gl.uniform3fv(pvKaUniform, materials[materialChoice].ambient.slice(0, 3));
        gl.uniform3fv(pvKdUniform, materials[materialChoice].diffuse.slice(0, 3));
        gl.uniform3fv(pvKsUniform, materials[materialChoice].specular.slice(0, 3));
        gl.uniform1f(pvMaterialShininessUniform, materials[materialChoice].shininess);
    }
    else{
        gl.useProgram(pfShaderProgramObject);

        gl.uniformMatrix4fv(pfViewMatrixUniform, false, viewMatrix);
        gl.uniformMatrix4fv(pfProjectionMatrixUniform, false, perspectiveProjectionMatrixSphere);
        gl.uniform3fv(pfLaUniform, lights[0].ambient.subarray(0, 3));
        gl.uniform3fv(pfLdUniform, lights[0].diffuse.subarray(0, 3));
        gl.uniform3fv(pfLsUniform, lights[0].specular.subarray(0, 3));
        gl.uniform1i(pfLKeyPressedUniform, bLight == true ? 1 : 0);

        if(keyPressed_sphere == 1){
            mat4.rotateX(lightRotationMatrix, lightRotationMatrix, angleForXRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 0.0, 20.0]);
        }
        else if(keyPressed_sphere == 2){
            mat4.rotateY(lightRotationMatrix, lightRotationMatrix, angleForYRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [20.0, 0.0, 0.0]);
        }
        else if(keyPressed_sphere == 3){
            mat4.rotateZ(lightRotationMatrix, lightRotationMatrix, angleForZRotation_sphere);
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [0.0, 20.0, 0.0]);
        }
        else{
            mat4.translate(lightTranslationMatrix, lightTranslationMatrix, [10.0, 10.0, 10.0]);
        }

        mat4.multiply(lightTransformMatrix, modelMatrix, viewMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, lightRotationMatrix);
        mat4.multiply(lightTransformMatrix, lightTransformMatrix, lightTranslationMatrix);

        vec4.transformMat4(result, lights[0].position, lightTransformMatrix);
        gl.uniform4fv(pfLightPositionUniform, result);

        gl.uniformMatrix4fv(pfModelMatrixUniform, false, modelMatrix);
        gl.uniform3fv(pfKaUniform, materials[materialChoice].ambient.slice(0, 3));
        gl.uniform3fv(pfKdUniform, materials[materialChoice].diffuse.slice(0, 3));
        gl.uniform3fv(pfKsUniform, materials[materialChoice].specular.slice(0, 3));
        gl.uniform1f(pfMaterialShininessUniform, materials[materialChoice].shininess);
    }

    sphere.draw();

    if(fbo){
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
}

function update(){
    // code
    angleCube -= 0.005;

    if(bLight){
        angleForXRotation_sphere += 0.05;
        angleForYRotation_sphere += 0.05;
        angleForZRotation_sphere += 0.05;
    }
}

function updateSphere(){
    // code
    for(var i = 0; i < 3; i++){
        lights[i].lightAngle = (lights[i].lightAngle + 0.01) ;
    }
}

function uninitialize(){
    // code
    if(vaoCube){
        gl.deleteVertexArray(vaoCube);
        vaoCube = null;
    }
    if(vboPositionCube){
        gl.deleteBuffer(vboPositionCube);
        vboPositionCube = null;
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

function uninitializeSphere(){
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
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
    AMC_ATTRIBUTE_TEXCOORD: 3
};

var shaderProgramObject = null;

var modelMatrixUniform = null;
var viewMatrixUniform = null;
var projectionMatrixUniform = null;

var vaoCube = null;
var vbo = null;

var perspectiveProjectionMatrix;

var angleCube = 0.0;

//Texture related variables
var marbleTexture = null;
var textureSamplerUniform = null;

// Light and material uniforms
var laUniform = null;               // Light Ambient
var ldUniform = null;               // Light Diffuse
var lsUniform = null;               // Light Specular
var lightPositionUniform = null;    // Light Position

var kaUniform = null;               // Material Ambient
var ksUniform = null;               // Material Specular
var kdUniform = null;               // Material Diffuse
var materialShininessUniform = null; // Material Shininess

var lKeyPressedUniform = null;      // Light Key Pressed

var lightAmbient = [0.0, 0.0, 0.0, 1.0];
var lightDiffuse = [1.0, 1.0, 1.0, 1.0];
var lightSpecular = [1.0, 1.0, 1.0, 1.0];
var lightPosition = [100.0, 100.0, 100.0, 1.0];

var materialAmbient = [0.25, 0.25, 0.25, 1.0];
var materialDiffuse = [1.0, 1.0, 1.0, 1.0];
var materialSpecular = [1.0, 1.0, 1.0, 1.0];
var materialShininess = 128.0;

var bLight = false; // Light On/Off

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

        case 76:  // 'L'
        case 108: // 'l'
            bLight = !bLight;
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
    "in vec3 aNormal;\n" +
    "in vec2 aTexCoord;\n" +
    "out vec2 out_texCoord;\n" +
    "out vec4 eyeCoordinates;\n" +
    "out vec3 transformedNormal;\n" +
    "out vec3 lightSource;\n" +
    "uniform mat4 uModelMatrix;\n" +
    "uniform mat4 uViewMatrix;\n" +
    "uniform mat4 uProjectionMatrix;\n" +
    "uniform vec4 uLightPosition;\n" +
    "out vec4 out_color;\n" +
    "void main(void)\n" +
    "{\n" +
    "   mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));\n" +
    "   transformedNormal = (normalMatrix * aNormal);\n" +
    "   eyeCoordinates = uViewMatrix * uModelMatrix * aPosition;\n" +
    "   lightSource = (vec3(uLightPosition) - eyeCoordinates.xyz);\n" +
    "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" +
    "   out_color = aColor;\n" +
    "   out_texCoord = aTexCoord;\n" +
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
    "in vec3 transformedNormal;\n" +
    "in vec4 eyeCoordinates;\n" +
    "in vec3 lightSource;\n" +
    "in vec4 out_color;\n" +
    "in vec2 out_texCoord;\n" +
    "uniform vec3 uLa;\n" +
    "uniform vec3 uLd;\n" +
    "uniform vec3 uLs;\n" +
    "uniform vec3 uKa;\n" +
    "uniform vec3 uKd;\n" +
    "uniform vec3 uKs;\n" +
    "uniform float uMaterialShininess;\n" +
    "uniform int  uLKeyPressed;\n" +
    "out vec4 FragColor;\n" +
    "uniform sampler2D uTextureSampler;\n" +
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
    "       FragColor = vec4(ambient + diffuse + specular, 1.0) * out_color * texture(uTextureSampler, out_texCoord);\n" +
    "   }\n" +
    "   else\n" +
    "   {\n" +
    "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" +
    "   }\n" +
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
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_NORMAL, "aNormal");
    gl.bindAttribLocation(shaderProgramObject, MyAttributes.AMC_ATTRIBUTE_TEXCOORD, "aTexCoord");

    gl.linkProgram(shaderProgramObject);

    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uProjectionMatrix");
    laUniform = gl.getUniformLocation(shaderProgramObject, "uLa");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "uLd");
    lsUniform = gl.getUniformLocation(shaderProgramObject, "uLs");
    kaUniform = gl.getUniformLocation(shaderProgramObject, "uKa");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "uKd");
    ksUniform = gl.getUniformLocation(shaderProgramObject, "uKs");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "uMaterialShininess");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "uLightPosition");
    lKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uLKeyPressed");
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "uTextureSampler");

    
    {
        var cube_PCNT = new Float32Array([
            // front
            // position				// color			 // normals				// texcoords
            1.0,  1.0,  1.0,	1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	1.0, 1.0,
            -1.0,  1.0,  1.0,	1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	0.0, 1.0,
            -1.0, -1.0,  1.0,	1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	0.0, 0.0,
            1.0, -1.0,  1.0,	1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	1.0, 0.0,
            // right			 
            // position				// color			 // normals				// texcoords
            1.0,  1.0, -1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	1.0, 1.0,
            1.0,  1.0,  1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	0.0, 1.0,
            1.0, -1.0,  1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	0.0, 0.0,
            1.0, -1.0, -1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	1.0, 0.0,
            // back				 
            // position				// color			 // normals				// texcoords
            1.0,  1.0, -1.0,	1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	1.0, 1.0,
            -1.0,  1.0, -1.0,	1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	0.0, 1.0,
            -1.0, -1.0, -1.0,	1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	0.0, 0.0,
            1.0, -1.0, -1.0,	1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	1.0, 0.0,
            // left				 
            // position				// color			 // normals				// texcoords
            -1.0,  1.0,  1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	1.0, 1.0,
            -1.0,  1.0, -1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	0.0, 1.0,
            -1.0, -1.0, -1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	0.0, 0.0,
            -1.0, -1.0,  1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	1.0, 0.0,
            // top				 
            // position				// color			 // normals				// texcoords
            1.0,  1.0, -1.0,	0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	1.0, 1.0,
            -1.0,  1.0, -1.0,	0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	0.0, 1.0,
            -1.0,  1.0,  1.0,	0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	0.0, 0.0,
            1.0,  1.0,  1.0,	0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	1.0, 0.0,
            // bottom			 
            // position				// color			 // normals				// texcoords
            1.0, -1.0,  1.0,	1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	1.0, 1.0,
            -1.0, -1.0,  1.0,	1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	0.0, 1.0,
            -1.0, -1.0, -1.0,	1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	0.0, 0.0,
            1.0, -1.0, -1.0,	1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	1.0, 0.0,
        ]);

        vaoCube = gl.createVertexArray();
        gl.bindVertexArray(vaoCube);
        {
            // Common VBO for P,C,N,T
            {
                vbo = gl.createBuffer();
                gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
                {
                    //24 * 11 * 4
                    gl.bufferData(gl.ARRAY_BUFFER, cube_PCNT, gl.STATIC_DRAW);
                    gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 11 * 4, 0 * 4);
                    gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);

                    gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 11 * 4, 3 * 4);
                    gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);

                    gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 11 * 4, 6 * 4);
                    gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_NORMAL);

                    gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 11 * 4, 9 * 4);
                    gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_TEXCOORD);
                }
                gl.bindBuffer(gl.ARRAY_BUFFER, null);
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
    marbleTexture = loadGLTexture('marble.bmp');
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

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var projectionMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(projectionMatrix);
    mat4.identity(rotationMatrix);

    mat4.rotateX(rotationMatrix, rotationMatrix, angleCube * Math.PI / 180.0);
    mat4.rotateY(rotationMatrix, rotationMatrix, angleCube * Math.PI / 180.0);
    mat4.rotateZ(rotationMatrix, rotationMatrix, angleCube * Math.PI / 180.0);
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
    mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    gl.uniform3f(laUniform, lightAmbient[0], lightAmbient[1], lightAmbient[2]);
    gl.uniform3f(ldUniform, lightDiffuse[0], lightDiffuse[1], lightDiffuse[2]);
    gl.uniform3f(lsUniform, lightSpecular[0], lightSpecular[1], lightSpecular[2]);
    gl.uniform3f(kaUniform, materialAmbient[0], materialAmbient[1], materialAmbient[2]);
    gl.uniform3f(kdUniform, materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);    
    gl.uniform3f(ksUniform, materialSpecular[0], materialSpecular[1], materialSpecular[2]);
    gl.uniform1f(materialShininessUniform, materialShininess);
    gl.uniform4fv(lightPositionUniform, lightPosition);
    gl.uniform1i(lKeyPressedUniform, bLight ? 1 : 0);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, marbleTexture);
    gl.uniform1i(textureSamplerUniform, 0);

    // Bind the VAO
    gl.bindVertexArray(vaoCube);
    {
        // Draw the Cube
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4); // front face
        gl.drawArrays(gl.TRIANGLE_FAN, 4, 4); // right face
        gl.drawArrays(gl.TRIANGLE_FAN, 8, 4); // back face
        gl.drawArrays(gl.TRIANGLE_FAN, 12, 4); // left face
        gl.drawArrays(gl.TRIANGLE_FAN, 16, 4); // top face
        gl.drawArrays(gl.TRIANGLE_FAN, 20, 4); // bottom face
    }
    gl.bindVertexArray(null);

    update();

    // Animation loop
    requestAnimationFrame(display, canvas);
}

function update(){
    // code
    angleCube += 1.0;
}

function uninitialize(){
    // code
    if(vaoCube){
        gl.deleteVertexArray(vaoCube);
        vaoCube = null;
    }
    if(vbo){
        gl.deleteBuffer(vbo);
        vbo = null;
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
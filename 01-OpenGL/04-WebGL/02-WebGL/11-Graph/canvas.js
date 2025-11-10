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

var vao_AxisLines = null;
var vbo_position_AxisLines = null;
var vbo_position_AxisColors = null;

var vao_thinBlueLine = null;
var vbo_position_thinBlueLines = null;

var vao_thickBlueLine = null;
var vbo_position_thickBlueLines = null;

var vao_triangle = null;
var vbo_position_triangle = null;

var vao_square = null;
var vbo_position_square = null;

var vao_circle = null;
var vbo_position_circle = null;

var perspectiveProjectionMatrix;

var drawTriangle = false, drawCircle = false, drawSquare = false, drawGraph = false, drawCircleUsingPoints = false;

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

        case 84: // 'T'
        case 116: // 't'
            drawTriangle = !drawTriangle;
        break;

        case 67: // 'C'
        case 99: // 'c'
            drawCircle = !drawCircle;
        break;

        case 83: // 'S'
        case 115: // 's'
            drawSquare = !drawSquare;
        break;

        case 71: // 'G'
        case 103: // 'g'
            drawGraph = !drawGraph;
        break;

        case 80: // 'P'
        case 112: // 'p'
            drawCircleUsingPoints = true; 
        break;

        case 76: // 'L'
        case 108: // 'l'
            drawCircleUsingPoints = false;
        break;

        case 48: // '0'
            drawTriangle = false;
            drawCircle = false;
            drawSquare = false;
            drawGraph = false;
            drawCircleUsingPoints = false;
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
    "gl_PointSize = 1.0;\n" +
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

    //VAO Setup Thin Blue Lines
    {
        let thinBlueLinePosition = new Float32Array(32 * 3 * 2 * 2);

        let lineCounter = 0;
        let spacing = (1.0 / 20.0);

        let iVertex = 0;
        for(let y = -1.0; y < 1.0 + spacing; y = y + spacing){
            if(lineCounter % 5 != 0){
                thinBlueLinePosition[iVertex]       = -1.0;
                thinBlueLinePosition[iVertex + 1]   = y;
                thinBlueLinePosition[iVertex + 2]   = 0.0;

                thinBlueLinePosition[iVertex + 3]   = 1.0;
                thinBlueLinePosition[iVertex + 4]   = y;
                thinBlueLinePosition[iVertex + 5]   = 0.0;

                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }

        lineCounter = 0;
        for(let x = -1.0; x < 1.0 + spacing; x = x + spacing){
            if(lineCounter % 5 != 0){
                thinBlueLinePosition[iVertex]       = x;
                thinBlueLinePosition[iVertex + 1]   = -1.0;
                thinBlueLinePosition[iVertex + 2]   = 0.0;

                thinBlueLinePosition[iVertex + 3]   = x;
                thinBlueLinePosition[iVertex + 4]   = 1.0;
                thinBlueLinePosition[iVertex + 5]   = 0.0;

                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }

        vao_thinBlueLine = gl.createVertexArray();
        gl.bindVertexArray(vao_thinBlueLine);
        {
            vbo_position_thinBlueLines = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_thinBlueLines);
            {
                gl.bufferData(gl.ARRAY_BUFFER, thinBlueLinePosition, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    //VAO Setup Thick Blue Lines
    {
        let thickBlueLinePosition = new Float32Array(9 * 3 * 2 * 2);

        let lineCounter = 0;
        let spacing = (1.0 / 20.0);

        let iVertex = 0;
        for(let y = -1.0; y < 1.0 + spacing; y = y + spacing){
            if(lineCounter % 5 == 0){
                thickBlueLinePosition[iVertex]       = -1.0;
                thickBlueLinePosition[iVertex + 1]   = y;
                thickBlueLinePosition[iVertex + 2]   = 0.0;

                thickBlueLinePosition[iVertex + 3]   = 1.0;
                thickBlueLinePosition[iVertex + 4]   = y;
                thickBlueLinePosition[iVertex + 5]   = 0.0;

                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }

        lineCounter = 0;
        for(let x = -1.0; x < 1.0 + spacing; x = x + spacing){
            if(lineCounter % 5 == 0){
                thickBlueLinePosition[iVertex]       = x;
                thickBlueLinePosition[iVertex + 1]   = -1.0;
                thickBlueLinePosition[iVertex + 2]   = 0.0;

                thickBlueLinePosition[iVertex + 3]   = x;
                thickBlueLinePosition[iVertex + 4]   = 1.0;
                thickBlueLinePosition[iVertex + 5]   = 0.0;

                iVertex = iVertex + 6;
            }
            lineCounter = lineCounter + 1;
        }

        vao_thickBlueLine = gl.createVertexArray();
        gl.bindVertexArray(vao_thickBlueLine);
        {
            vbo_position_thickBlueLines = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_thickBlueLines);
            {
                gl.bufferData(gl.ARRAY_BUFFER, thickBlueLinePosition, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }   
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    //VAO Setup Axis Lines
    {
        let axisLinesPosition = new Float32Array([
            -1.0, 0.0, 0.0,
            1.0, 0.0, 0.0,
            0.0, -1.0, 0.0,
            0.0, 1.0, 0.0
        ]);
        let axisLinesColors = new Float32Array([
            1.0, 0.0, 0.0,
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 1.0, 0.0
        ]);

        vao_AxisLines = gl.createVertexArray();
        gl.bindVertexArray(vao_AxisLines);
        {
            vbo_position_AxisLines = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_AxisLines);
            {
                gl.bufferData(gl.ARRAY_BUFFER, axisLinesPosition, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            vbo_position_AxisColors = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_AxisColors);
            {
                gl.bufferData(gl.ARRAY_BUFFER, axisLinesColors, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_COLOR);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    //VAO Setup Triangle
    {
        let triangleVertices = new Float32Array([
            0.0,  0.5, 0.0,
            -0.5, -0.5, 0.0,
            0.5, -0.5, 0.0
        ]);
        vao_triangle = gl.createVertexArray();
        gl.bindVertexArray(vao_triangle);
        {
            vbo_position_triangle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_triangle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    //VAO Setup Square
    {
        let squareVertices = new Float32Array([
            0.5,  0.5, 0.0,
            -0.5,  0.5, 0.0,
            -0.5, -0.5, 0.0,
            0.5, -0.5, 0.0
        ]);
        vao_square = gl.createVertexArray();
        gl.bindVertexArray(vao_square);
        {
            vbo_position_square = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_square);
            {
                gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
            }
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }
        gl.bindVertexArray(null);
    }

    //VAO Setup Circle
    {
        let circleVertices = [];
        let numSegments = 360;
        let angleStep = (2.0 * Math.PI) / numSegments;
        for(let angle = 0.0; angle < 2.0 * Math.PI; angle += angleStep){
            let x = Math.cos(angle) * 0.5;
            let y = Math.sin(angle) * 0.5;
            circleVertices.push(x);
            circleVertices.push(y);
            circleVertices.push(0.0);
        }
        vao_circle = gl.createVertexArray();
        gl.bindVertexArray(vao_circle);
        {
            vbo_position_circle = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_circle);
            {
                gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleVertices), gl.STATIC_DRAW);
                gl.vertexAttribPointer(MyAttributes.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
                gl.enableVertexAttribArray(MyAttributes.AMC_ATTRIBUTE_POSITION);
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

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);
    mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    if(drawGraph == true){
        // Draw Thin Blue Lines
        gl.bindVertexArray(vao_thinBlueLine);
        gl.lineWidth(1.0);
        gl.vertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0, 1.0);
        gl.drawArrays(gl.LINES, 0, 128);
        gl.bindVertexArray(null);

        // Draw Thick Blue Lines
        gl.bindVertexArray(vao_thickBlueLine);
        gl.lineWidth(5.0); //Not supported in WebGL
        gl.vertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0, 1.0);
        gl.drawArrays(gl.LINES, 0, 36);
        gl.bindVertexArray(null);

        // Draw Axis Lines
        gl.bindVertexArray(vao_AxisLines);
        gl.lineWidth(3.0);
        gl.drawArrays(gl.LINES, 0, 4);
        gl.bindVertexArray(null);
    }

    if(drawTriangle == true){
        // Draw Triangle
        gl.bindVertexArray(vao_triangle);
        gl.vertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0, 1.0);
        gl.drawArrays(gl.LINE_LOOP, 0, 3);
        gl.bindVertexArray(null);
    }

    if(drawSquare == true){
        // Draw Square
        gl.bindVertexArray(vao_square);
        gl.vertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0, 1.0);
        gl.drawArrays(gl.LINE_LOOP, 0, 4);
        gl.bindVertexArray(null);
    }

    if(drawCircle == true){
        // Draw Circle
        gl.bindVertexArray(vao_circle);
        gl.vertexAttrib4f(MyAttributes.AMC_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0, 1.0);
        //gl.pointSize(2.0);
        if(drawCircleUsingPoints == false)
            gl.drawArrays(gl.LINE_LOOP, 0, 360);
        else
            gl.drawArrays(gl.POINTS, 0, 360);
        gl.bindVertexArray(null);
    }

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
    if(vboColor){
        gl.deleteBuffer(vboColor);
        vboColor = null;
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
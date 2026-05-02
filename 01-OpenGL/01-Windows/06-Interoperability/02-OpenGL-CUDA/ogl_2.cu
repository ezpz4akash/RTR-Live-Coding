// Win32 Headers files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmath.h"
using namespace vmath;

// OpenGl related header files
#include <gl/glew.h> // this header file must me inlcude before gl.h
#include <gl/GL.h>

// User defined header file
#include "OGL.h"

// OpenGl related libraries
#pragma comment(lib, "glew32.lib") // this is to maintin consistecy for above seq
#pragma comment(lib, "opengl32.lib")

//Cuda related headers and lib
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#pragma comment(lib, "cudart.lib")



// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
// variables related to FullScreen
BOOL gbFullScreen = FALSE;
HWND ghwnd = NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

// shader related global variable
GLuint shaderProgramObject = 0;

enum
{
	AMC_ATTRIBUTE_POSITION = 0
};


// variables related to attributes
GLuint vao = 0;
GLuint vbo_position = 0;

GLuint mvpMatrixUniform = 0; // model view projection

mat4 perspectivePorjectionMatrix; // matrix of 4

// variables related with file I/O
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// Active window related variables
BOOL gbActiveWindow = FALSE;

// Exit Keypress related
BOOL gbEscapeKeyIsPressed = FALSE;

// OpenGl related Global variables
HDC ghdc = NULL;
HGLRC ghrc = NULL; // global handle to rendering context

//SIGN WAVE RELATED VARIABLES
const unsigned int gMeshWidth = 1024; // 64
const unsigned int gMeshHeight = 1024; // 64
const unsigned int gMeshDepth = 4;

#define MESHARRAYSIZE (gMeshWidth * gMeshHeight * gMeshDepth)

float position[gMeshWidth][gMeshHeight][gMeshDepth]; // in home work start with 64

GLuint vbo_gpu = 0;
GLuint vbo_cpu = 0;

float animationTime = 0.0f;

// CUDA related variables
cudaError_t cudaResult;
struct cudaGraphicsResource *cuGraphicsResource = NULL;
BOOL bOnGPU = FALSE;
 
__global__ void sineWaveKernal(float4 * pos, unsigned int width, unsigned int height, float time)
{
	// code
	unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int j = blockIdx.y + threadIdx.y;

	float u = (float)i / (float)width;
	float v = (float)j / (float)height;

	u = u * 2.0f - 1.0f;
	v = v * 2.0f - 1.0f;

	float frequency = 4.0f;
	float w = sinf(u * frequency + time) * cosf(v * frequency + time) * 0.5;

	pos[j * width + i] = make_float4(u, w, v, 1.0f);

} 

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function Declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// Variable Declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("RTR6");
	BOOL bDone = FALSE;

	// Code
	// Create Log File
	gpFile = fopen(gszLogFileName, "w");

	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("LOG FILE CREATION FAILED !!!"), TEXT("FILE I/O ERROR"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Program started successfully\n");
	}

	// Window Class Intialization
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Registration of Window Class
	RegisterClassEx(&wndclass);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);

	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	// Create Window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName,
						  TEXT("Ashish Palekar"),
						  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
						  screenWidth / 2 - WIN_WIDTH / 2, screenHeight / 2 - WIN_HEIGHT / 2,
						  WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

	ghwnd = hwnd;

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	// Paint Background of the Window
	UpdateWindow(hwnd);

	// Message loop
	/*while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	*/

	// initialize
	int result = initialize();

	if (result != 0)
	{
		fprintf(gpFile, "Initialise() Failed !!!\n");
		DestroyWindow(hwnd);
		hwnd = NULL;
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(gpFile, "Initialise() Completed Successfully\n");
	}
   
	// set this window as foreground and active window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				if (gbEscapeKeyIsPressed == TRUE)
				{
					bDone = TRUE;
				}
				// Render
				display();

				// update
				update();
			}
		}
	}

	uninitialize();
	return ((int)msg.wParam);
}

// Callback Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declarations
	void toggleFullScreen(void);
	void resize(int, int);
	void uninitialize(void);

	// Code
	switch (iMsg)
	{
	case WM_CREATE:
		ZeroMemory((void *)&wpPrev, sizeof(WINDOWPLACEMENT));
		wpPrev.length = sizeof(WINDOWPLACEMENT);
		break;
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;
	case WM_ERASEBKGND:
		return (0); // flikker free rendering
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE: // virtual key code
			gbEscapeKeyIsPressed = TRUE;
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'f':
		case 'F':
			if (gbFullScreen == FALSE)
			{
				toggleFullScreen();
				gbFullScreen = TRUE;
			}
			else
			{
				toggleFullScreen();
				gbFullScreen = FALSE;
			}
			break;
		
		case 'C':
		case 'c':
			bOnGPU = FALSE;
			break;

		case 'G':
		case 'g':
			bOnGPU = TRUE;
			break;
		}
		break;
	case WM_CLOSE:
		uninitialize();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
	// variable declarations
	MONITORINFO mi;

	// code
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			ZeroMemory((void *)&mi, sizeof(MONITORINFO));
			mi.cbSize = sizeof(MONITORINFO);
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE); // optional for full screen
	}
	else
	{
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

int initialize(void)
{
	// Function declarations
	void printGLInfo(void);
	void resize(int, int);
	void uninitialize(void);
	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	GLenum glewResult;
	// code
	// PIXELFORMATDESCRIPTOR initialization
	ZeroMemory((void *)&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	// getDC
	ghdc = GetDC(ghwnd);
	if (ghdc == NULL)
	{
		fprintf(gpFile, "GetDC() failed !!!\n");
		return (-1);
	}

	// Get Matching Pixel format index using HDC and PFD
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormatIndex == 0)
	{
		fprintf(gpFile, "ChoosePixelFormat() failed !!!\n");
		return (-2);
	}

	// select the pixel format of found index
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		fprintf(gpFile, "SetPixelFormat() failed !!!\n");
		return (-3);
	}

	// create rendering context using hdc, pfd and chosen iPixelFormatIndex
	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL)
	{
		fprintf(gpFile, "wglCreateContext() failed !!!\n");
		return (-4);
	}

	// Make this rendering context as current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		fprintf(gpFile, "wglMakeCurrent() failed !!!\n");
		return (-5);
	}

	// initiaize glew variable

	glewResult = glewInit();

	if (glewResult != GLEW_OK)
	{
		fprintf(gpFile, "glewinit() failed !!!\n");
		return (-6);
	}

	printGLInfo();

	// CUDA related device selection
	int devCount = 0;
	cudaResult = cudaGetDeviceCount(&devCount);
	if(cudaResult != cudaSuccess)
	{
		fprintf(gpFile, "cuda device get count failed");
		uninitialize();
		exit(EXIT_FAILURE);
	}
	else if(devCount == 0)
	{
		fprintf(gpFile, "initialise cuda device get count successed but no cuda supported device found\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}

	cudaSetDevice(0);

	// VERTEX shader

	// 1 write the shader source code
	// 2 create the shader object
	// 3 give source code to shader object
	// 4 compile shader
	// 5 do shader  compilation error checking
	// in vec4 aat yeto vec4 a attribute
	// uniform mat4 is different from vmath
	const GLchar *vertexShaderSourceCode = "#version 460 core\n"
		"in vec4 aPosition ;\n"
		"uniform mat4 uMVPMatrix ; \n"
		"void main(void)\n"
		"{\n"
		"gl_Position = uMVPMatrix * aPosition ;\n"
		"}";

	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	// 	multiple shader lenght as null parameter
	glShaderSource(vertexShaderObject, 1,
				   (const GLchar **)&vertexShaderSourceCode, NULL);

	glCompileShader(vertexShaderObject);

	GLint status = 0;
	GLint infoLogLength = 0;
	GLchar *szInfoLog = NULL;
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) // error which may not have lenght
		{
			szInfoLog = (GLchar *)malloc((infoLogLength +1) * sizeof(GLchar));
			if (szInfoLog != NULL)
			{
				glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog);
				fprintf(gpFile, "vertex shader compilation log = %s\n", szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
			}
		}
		uninitialize();
	}

	// fragment shader

	const GLchar *fragmentShaderSourceCode = "#version 460 core" \
	   "\n" \
		"out vec4 FragColor ;" \
		"void main(void)" \
		"{" \
		"FragColor=vec4(1.0f,0.5f,0.0f,1.0f) ;" \
		"}";

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(fragmentShaderObject);
	status = 0;
	infoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	// Depth related code
	if (status == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
			if (szInfoLog != NULL)
			{
				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL, szInfoLog);
				fprintf(gpFile, "fragmet shader compilation log = %s \n", szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
			}
		}
		uninitialize();
	}

	// create shader program object attach shader object  to this shader program object
	// tell to link the shader object to shader program object
	// check for link error log

	// creaate attache and link shader program object
	shaderProgramObject = glCreateProgram();
	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	// bind shader attribute at certain index in shader to same index in post program
	 glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");

	glLinkProgram(shaderProgramObject);
	status = 0;
	infoLogLength = 0;
	szInfoLog = NULL;
	glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == FALSE)
	{
		glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar *));
			if (szInfoLog != NULL)
			{
				glGetProgramInfoLog(shaderProgramObject,
									infoLogLength,
									NULL,
									szInfoLog);
				fprintf(gpFile, "shader program link log = %s \n", szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
			}
		}
		uninitialize();
	}

	// get req uniform location from shader
	// attribute no chane after sending no run time change
	// jo data par frame badalto to uniformla de
	mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
	// what to do shader data
	// provide verteex position ,color , normal,texcord etc;

	for (int i = 0; i < (int)gMeshWidth;i++)
	{
		for (int j = 0; j < (int)gMeshHeight;j++)
		{
			for (int k = 0; k < (int)gMeshDepth;k++)
			{
				position[i][j][k] = 0.0f;
			}
		}
	}

	// vertx array object for arrays of vertex attribute
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// position
	// step 1
	glGenBuffers(1, &vbo_cpu); // chotosa ghol vao kuth hi karu shkto tas buffer ch nhi ahe
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cpu);
	glBufferData(GL_ARRAY_BUFFER, 
				MESHARRAYSIZE * sizeof(float),
				NULL, 	
				GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// for gpu
	glGenBuffers(1, &vbo_gpu); // chotosa ghol vao kuth hi karu shkto tas buffer ch nhi ahe
	glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);
	glBufferData(GL_ARRAY_BUFFER, 
				MESHARRAYSIZE * sizeof(float),
				NULL, 	
				GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); //unbind to vao

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	// From here onwards OpenGL code starts
	// Tell OpenGL to choose the color to clear the screen
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	//cuda related initailisation
	cudaResult = cudaGraphicsGLRegisterBuffer(&cuGraphicsResource, vbo_gpu, cudaGraphicsMapFlagsWriteDiscard);
	if(cudaResult != cudaSuccess)
	{
		fprintf(gpFile, "initialise cuda graphics gl register buffer failed \n");
		uninitialize();
		exit(EXIT_FAILURE);
	}

	// analogous to gl_load identity in resize madhe karto ahe
	perspectivePorjectionMatrix = mat4::identity();

	// Warmup resize
	resize(WIN_WIDTH, WIN_HEIGHT);
	return (0);
}

void printGLInfo(void)
{
	// varibale declaration
	// GLint numExtensions, i;
	// code
	// print OpenGL Information
	fprintf(gpFile, "OPENGL INFORMATION\n");
	fprintf(gpFile, "******************\n");
	fprintf(gpFile, "OpenGL Vendor   : %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version  : %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version  : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	fprintf(gpFile, "******************\n");
	// // get number of extension
	// glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	// // print opengl extention
	// for (i = 0; i < numExtensions; i = i + 1)
	// {
	// 	fprintf(gpFile, " %s : \n", glGetStringi(GL_EXTENSIONS, i));
	// }
}

void resize(int width, int height)
{
	// code
	// If height by accident become less or equal to zero then set it to 1
	if (height <= 0)
	{
		height = 1;
	}
	// set the view port
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectivePorjectionMatrix = vmath ::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	// code
	void uninitialize(void);
	void sineWave(unsigned int, unsigned int, float);

	// Clear OpenGl Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// use shader program object
	glUseProgram(shaderProgramObject);
    
	// transformations 
	mat4 modelViewMatrix = mat4 :: identity(); // anlogous to gl_load identity in display for model view matrix
	//mat4 tranlationMatrix = mat4 ::identity();
	//tranlationMatrix = vmath ::translate(0.0f, 0.0f, -3.0f);
	//modelViewMatrix = tranlationMatrix;
	mat4 modelViewProjectionMatrix = perspectivePorjectionMatrix * modelViewMatrix; // order is important

	// send this matrix to shader in uniform 
	glUniformMatrix4fv (mvpMatrixUniform,
	1, GL_FALSE,modelViewProjectionMatrix);

	// bind with vao
	glBindVertexArray(vao);

	if(bOnGPU == TRUE)
	{
		//Step 1- map the resource
		cudaResult = cudaGraphicsMapResources(1, &cuGraphicsResource, 0);
		if(cudaResult != cudaSuccess)
		{
			fprintf(gpFile, "cuda graphics map resources failed\n");
			uninitialize();
			exit(EXIT_FAILURE);
		}

		/// step 2- Get host compatible mapped pointer for graphics reosurce
		float4 *pPosition = NULL;
		cudaResult = cudaGraphicsResourceGetMappedPointer((void **)&pPosition, NULL, cuGraphicsResource);
		if(cudaResult != cudaSuccess)
		{
			fprintf(gpFile, "cuda graphics getmappointer failed resources failed\n");
			uninitialize();
			exit(EXIT_FAILURE);
		}

		// step 3- Run Cuda Kernel
		dim3 block(512 * 512 * 1);
		dim3 grid(gMeshWidth / block.x, gMeshHeight / block.y, 1);
		sineWaveKernal<<<grid, block>>>(pPosition, gMeshWidth, gMeshHeight, animationTime);

		//step4 -unmap the mapped the pointer
		cudaResult = cudaGraphicsUnmapResources(1, &cuGraphicsResource, 0);

		// step 5 - Remember that our p position is nothing but vbo_gpu hence bind with it
		glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);

	}
	else
	{
		sineWave(gMeshWidth, gMeshHeight, animationTime);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_cpu);
	}



	//draw sinewave
	sineWave(gMeshWidth, gMeshHeight, animationTime);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_cpu);
	glBufferData(GL_ARRAY_BUFFER, MESHARRAYSIZE * sizeof(float), position, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(
				AMC_ATTRIBUTE_POSITION,
				4, 
				GL_FLOAT, GL_FALSE, 0,
				NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	// DRAW vertext aray
	glDrawArrays(GL_POINTS,0,gMeshWidth * gMeshHeight);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // unbind with vao

	// unuse shader program object
	glUseProgram(0);
	
	// Swap the Buffers
	SwapBuffers(ghdc);
}

void sineWave(unsigned int mesh_width, unsigned int mesh_height, float time)
{
	// code
	for (int i = 0; i < (int)gMeshWidth;i++)
	{
		for (int j = 0; j < (int)gMeshHeight;j++)
		{
			for (int k = 0; k < (int)gMeshDepth;k++)
			{
				float u = (float)i / (float)mesh_width;
				float v = (float)j / (float)mesh_height;

				u = u * 2.0f - 1.0f;
				v = v * 2.0f - 1.0f;

				float frequency = 4.0f;
				float w = sin(u * frequency + time) * cos(v * frequency + time) * 0.5;

				if(k == 0)
				{
					position[i][j][k] = u;
				}

				if(k == 1)
				{
					position[i][j][k] = w;
				}
				
				if(k == 2)
				{
					position[i][j][k] = v;
				}

				if(k == 3)
				{
					position[i][j][k] = 1.0f;
				}
			}
		}
	}
}

void update(void)
{
	// code
	animationTime = animationTime + 0.01;
}

void uninitialize(void)
{
	// function declarations
	void toggleFullScreen(void);


	// code
	// If user is exiting in FullScreen, then restore it to normal window
	if (gbFullScreen == TRUE)
	{
		toggleFullScreen();
		gbFullScreen = FALSE;
	}

	if(vbo_gpu)
	{
		if(cuGraphicsResource)
		{
			cudaGraphicsUnregisterResource(cuGraphicsResource);
			cuGraphicsResource = NULL;
		}

		glDeleteBuffers(1, &vbo_gpu);
		vbo_gpu = 0;
	}
	// free vbo_posisiton 
	if(vbo_cpu)
	{
		glDeleteBuffers(1, &vbo_cpu);
		vbo_cpu = 0;
	}
	// fre vao
	if(vao)
	{
		glDeleteVertexArrays(1,&vao);
		vao = 0;
	}
	
	// detach delete shader objects and delete shader program object
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		GLint NumShaders;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &NumShaders);
		if (NumShaders > 0)
		{
			GLuint *pShaders = (GLuint *)malloc(NumShaders * sizeof(GLuint));
			if (pShaders != NULL)
			{
				glGetAttachedShaders(shaderProgramObject, NumShaders, NULL, pShaders);
				for (GLint i = 0; i < NumShaders; i++)
				{
					glDetachShader(shaderProgramObject, pShaders[i]);
					glDeleteShader(pShaders[i]);
					pShaders[i] = 0;
				}
			}
			free(pShaders);
			pShaders = 0;
		}
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
	}

	// Make HDC as current context by releasing rendering context as current context
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	// delete the rendering context
	if (ghrc != NULL)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	// release the DC
	if (ghdc != NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// Destroy Window
	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	// close the file
	if (gpFile != NULL)
	{
		fprintf(gpFile, "Program terminated successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

// WM_NCCALCSIZE
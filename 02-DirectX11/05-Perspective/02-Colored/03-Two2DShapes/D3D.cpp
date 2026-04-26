// Win32 headers
#include <Windows.h>
#include <stdio.h>          // For File IO
#include <stdlib.h>         // For exit()
#include <math.h>           // for ceil
#include "D3D.h"

// D3D 11 related header
#include <dxgi.h>
#include <d3d11.h>  
#include <d3dcompiler.h>

// for D3D11 Math
#pragma warning(disable:4838)
#include "XNAMath/xnamath.h"

// dxgi - direct x graphics interface 
#pragma comment(lib, "dxgi.lib")        
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Macros
#define WIN_WIDTH       800
#define WIN_HEIGHT      600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
// variable related to FullScreen
BOOL gbFullScreen = FALSE;
HWND ghwnd = NULL;
DWORD dwStyle;                  // Local static asala tri chalel
WINDOWPLACEMENT wpPrev;

// variable related to FileIO
char gszLogFileName[] = "Log.txt";
FILE* gpFile = NULL;

// Active window related variable
BOOL gbActiveWindow = FALSE;

// exit key pressed related variable
BOOL gbEscapeKeyIsPressed = FALSE;

const char szLogFileName[] = "D3DLog.txt";

// D3D related global variables
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
float clearColor[4];       // RGBA   

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_TrianglePositionBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_RectanglePositionBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_TriangleColorBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_RectangleColorBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;

struct CBUFFER
{
    XMMATRIX worldViewProjectionMatrix;
}; 

XMMATRIX perspectiveProjectionMatrix;


// Entry-Point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // function declarations
    HRESULT initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);
    
    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR6");
    BOOL bDone = FALSE;

    HRESULT hr = S_OK;

    // Window center related variable
    int windowWidth;
    int windowHeight;
    int xPosWindow;
    int yPosWindow;
    int screenWidth;
    int screenHeight;
    
    // code

    // create log file
    gpFile = fopen(szLogFileName, "a+");
    // file open modes
    // r = read 
    // w = write 
    // a = append 

    // fopen_s(gszLogFileName, "w");    --windows recommendation
    // fprintf_s()                      --windows recommendation

    if(gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log File Creation Failed"), TEXT("File IO Error"), MB_OK | MB_TOPMOST);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program Started Successfully\n");
    }


    // window class initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;    // CS_OWNDC => class style own device context
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    // wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    // wndclass.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(MYCURSOR));
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    // wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // Registration of window class
    if(!RegisterClassEx(&wndclass))
    {
        MessageBox(NULL, TEXT("Failed to Register Class"), TEXT("RegisterClassEx"), MB_OK | MB_TOPMOST);
        ExitProcess(EXIT_FAILURE);
    }

    // Screen dimensions
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

    windowWidth = WIN_WIDTH;
    windowHeight = WIN_HEIGHT;

    xPosWindow = (screenWidth - windowWidth) / 2;
    yPosWindow = (screenHeight - windowHeight) / 2;

    // Create Window in Memory
    hwnd = CreateWindowEx(
            WS_EX_APPWINDOW,
            szAppName,              // class name
            TEXT("RTR 6 - Akash Musale: D3D11 Two 2D Shapes Color"),  // window name Caption Bar Name
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,    // window style
            // WS_OVERLAPPEDWINDOW,     // window style
            xPosWindow,                 // x co-ordinate
            yPosWindow,                 // y co-ordinate
            WIN_WIDTH,                  // window width
            WIN_HEIGHT,                 // window height
            NULL,                       // parent window handle    
            NULL,                       // Menu Bar
            hInstance,                  // Instance of current Handle window
            NULL                        // window creation parameter
        );
    
    ghwnd = hwnd;
    
    // Show window
    ShowWindow(hwnd, iCmdShow);

    // Paint background of window
    UpdateWindow(hwnd);

    // initialize
    hr = initialize();
    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "initialize() failed\n");
        fclose(gpFile);
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "initialize() Completed Successfully\n");
        fclose(gpFile);
    }

    // Set This window as foreground and active window
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // message loop
    /* 
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    */

    // Game loop
    while(bDone == FALSE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
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
            if(gbActiveWindow == TRUE)
            {
                if(gbEscapeKeyIsPressed == TRUE)
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

    // uninitialize
    uninitialize();

    return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // function declarations
    void toggleFullScreen(void);
    HRESULT resize(int width, int height);
    void uninitialize(void);

    // variable declarations
    HRESULT hr = S_OK;

    // code
    switch(iMsg)
    {
        case WM_CREATE:
            ZeroMemory((void*)&wpPrev, sizeof(WINDOWPLACEMENT)); 
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS:
            gbActiveWindow = TRUE;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = FALSE;
            break;
        
        case WM_SIZE:
            if(gpID3D11DeviceContext)
            {
                hr = resize(LOWORD(lParam), HIWORD(lParam));
                if(FAILED(hr))
                {
                    gpFile = fopen(szLogFileName, "a+");
                    fprintf(gpFile, "resize() failed\n");
                    fclose(gpFile);
                    return(hr);
                }
            }
            break;
        
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:                     // virtual key code @, $, Esc 
                    gbEscapeKeyIsPressed = TRUE;
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'F':                
                case 'f':
                    if(gbFullScreen == FALSE)
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

                default:
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

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    // variable declarations
    MONITORINFO mi;

    // code
    if(gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)   // & => contains
        {
            ZeroMemory((void*)&mi, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, 
                            HWND_TOP, 
                            mi.rcMonitor.left, 
                            mi.rcMonitor.top, 
                            mi.rcMonitor.right - mi.rcMonitor.left, 
                            mi.rcMonitor.bottom - mi.rcMonitor.top,
                            SWP_NOZORDER | SWP_FRAMECHANGED
                    );

                
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, 
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
                );
        ShowCursor(TRUE);
    }
}

HRESULT initialize(void)
{
    // function declarations
    void printDXInfo(void);
    HRESULT resize(int width, int height);

    // variable declarations
    HRESULT hr = S_OK;


    // code
    printDXInfo();

    // swap chain descriptor initialization
    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
    dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferCount = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.SampleDesc.Count = 1;               // No MultiSampling
    dxgiSwapChainDesc.SampleDesc.Quality = 0;             // No MultiSampling
    dxgiSwapChainDesc.OutputWindow = ghwnd;
    dxgiSwapChainDesc.Windowed = TRUE;                    // start with windowed mode

    // get dxgi swap chain, d3d11 device and d3d11 device context supported driver and supported feature level 5 things at a once
    D3D_DRIVER_TYPE d3dDriverType;
    D3D_DRIVER_TYPE d3dDriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,           // windows advanced rasterization platform
        D3D_DRIVER_TYPE_SOFTWARE,
        D3D_DRIVER_TYPE_REFERENCE
    };

    D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL d3dFeatureLevel_aquired = D3D_FEATURE_LEVEL_10_0;
    UINT numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

    for(UINT i = 0; i < numDriverTypes; i++)
    {
        d3dDriverType = d3dDriverTypes[i];
        hr = D3D11CreateDeviceAndSwapChain(
                NULL,                           // Adapter
                d3dDriverType,
                NULL,                           // Software
                0,                              // Flags
                &d3dFeatureLevel_required,      // Feature Levels
                1,                              // count of feature levels
                D3D11_SDK_VERSION,
                &dxgiSwapChainDesc,
                &gpIDXGISwapChain,
                &gpID3D11Device,
                &d3dFeatureLevel_aquired,
                &gpID3D11DeviceContext
            );

        if(SUCCEEDED(hr))
        {
            break;
        }

    }

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11CreateDeviceAndSwapChain() failed\n");
        fclose(gpFile);
        return (hr);
    }

    // check which driver is usage
    gpFile = fopen(szLogFileName, "a+");
    if(d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_HARDWARE\n");
    }
    else if(d3dDriverType == D3D_DRIVER_TYPE_WARP)
    {
        fprintf(gpFile, "chosen driver is  D3D_DRIVER_TYPE_WARP\n");
    }
    else if(d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_REFERENCE\n");
    }
    else if(d3dDriverType == D3D_DRIVER_TYPE_SOFTWARE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_SOFTWARE\n");
    }
    else
    {
        fprintf(gpFile, "chosen driver is unknown\n");
    }

    // check which feature level is acquired
    if(d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_11_0)
    {
        fprintf(gpFile, "acquired feature level is D3D_FEATURE_LEVEL_11_0\n");
    }
    else if(d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_1)
    {
        fprintf(gpFile, "acquired feature level is D3D_FEATURE_LEVEL_10_1\n");
    }
    else if(d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_0)
    {
        fprintf(gpFile, "acquired feature level is D3D_FEATURE_LEVEL_10_0\n");
    }
    else
    {
        fprintf(gpFile, "very old acquired feature level\n");
    }

    fclose(gpFile);

    printDXInfo();

    /////////////////////////////////////// Vertex Shader ///////////////////////////////////////
    const char  *vertexShaderSourceCode = 
    "cbuffer constantBuffer" \
    "{" \
    "   float4x4 worldViewProjectionMatrix;" \
    "}" \
    "struct vertex_Output" \
    "{" \
    "   float4 position : SV_POSITION;" \
    "   float4 color : COLOR;" \
    "};" \
    "vertex_Output main(float4 pos : POSITION, float4 col : COLOR)" \
    "{" \
    "   vertex_Output vertex_output;"
    "   vertex_output.position = mul(worldViewProjectionMatrix, pos);" \
    "   vertex_output.color = col;" \
    "   return (vertex_output);" \
    "}";

    ID3DBlob *pID3DBlob_vertexShaderCode = NULL;
    ID3DBlob *pID3DBlob_error = NULL;

    // compile above shader
    hr = D3DCompile(
            vertexShaderSourceCode,
            lstrlenA(vertexShaderSourceCode) + 1,
            "VS",                                   // Source code string
            NULL,                                   // D3D_SHADER_MACRO
            D3D_COMPILE_STANDARD_FILE_INCLUDE, 
            "main",                                 // Entry point function name  
            "vs_5_0",                               // Shader model
            0,                                      // Flags
            0,                                      // Effect Constant
            &pID3DBlob_vertexShaderCode,            // compiled code thevnya sathi
            &pID3DBlob_error                        // Error for vertex shader
        );

    if(FAILED(hr))
    {
        if(pID3DBlob_error != NULL)                 
        {
            gpFile = fopen(szLogFileName, "a+");
            fprintf(gpFile, "D3DCompile() failed for vertex shader:%s\n", (char*)pID3DBlob_error->GetBufferPointer());
            fclose(gpFile);
            pID3DBlob_error->Release();
            pID3DBlob_error = NULL;
            return (hr);
        }
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3DCompile() succeeded for vertex shader\n");
        fclose(gpFile);
    }

    // create vertex shader source code
    hr = gpID3D11Device->CreateVertexShader(
                            pID3DBlob_vertexShaderCode->GetBufferPointer(),
                            pID3DBlob_vertexShaderCode->GetBufferSize(),
                            NULL,       // class linkage parameter across shader variable sathi
                            &gpID3D11VertexShader
                        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateVertexShader() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateVertexShader() Succeeded\n");
        fclose(gpFile);
    }

    // set this vertex shader in pipeline
    gpID3D11DeviceContext->VSSetShader(
            gpID3D11VertexShader,
            NULL,   
            0
        );



    /////////////////////////////////////// Pixel Shader ///////////////////////////////////////
    const char  *pixelShaderSourceCode = 
    "struct vertex_Output" \
    "{" \
    "   float4 position : SV_POSITION;" \
    "   float4 color : COLOR;" \
    "};" \
    "float4 main(vertex_Output input) : SV_TARGET" \
    "{" \
    "   float4 color = input.color;" \
    "   return (color);" \
    "}";

    ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
    pID3DBlob_error = NULL;

    // compile pixel shader
    hr = D3DCompile(
            pixelShaderSourceCode,
            lstrlenA(pixelShaderSourceCode) + 1,
            "PS",                                   // Source code string
            NULL,                                   // D3D_SHADER_MACRO
            D3D_COMPILE_STANDARD_FILE_INCLUDE, 
            "main",                                 // Entry point function name  
            "ps_5_0",                               // Shader model
            0,                                      // Flags
            0,                                      // Effect Constant
            &pID3DBlob_pixelShaderCode,             // compiled code thevnya sathi
            &pID3DBlob_error                        // Error for vertex shader
        );

    if(FAILED(hr))
    {
        if(pID3DBlob_error != NULL)                 
        {
            gpFile = fopen(szLogFileName, "a+");
            fprintf(gpFile, "D3DCompile() failed for pixel shader:%s\n", (char*)pID3DBlob_error->GetBufferPointer());
            fclose(gpFile);
            pID3DBlob_error->Release();
            pID3DBlob_error = NULL;
            return (hr);
        }
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3DCompile() succeeded for pixel shader\n");
        fclose(gpFile);
    }

    // create vertex shader source code
    hr = gpID3D11Device->CreatePixelShader(
                            pID3DBlob_pixelShaderCode->GetBufferPointer(),
                            pID3DBlob_pixelShaderCode->GetBufferSize(),
                            NULL,       // class linkage parameter across shader variable sathi
                            &gpID3D11PixelShader
                        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreatePixelShader() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreatePixelShader() Succeeded\n");
        fclose(gpFile);
    }

    // set this vertex shader in pipeline
    gpID3D11DeviceContext->PSSetShader(
            gpID3D11PixelShader,
            NULL,   
            0
        );

    // initialize input layout
    D3D11_INPUT_ELEMENT_DESC d3d11InputElementDesc[2];
    ZeroMemory((void*)d3d11InputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC) * _ARRAYSIZE(d3d11InputElementDesc));

    // position
    d3d11InputElementDesc[0].SemanticName = "POSITION";    
    d3d11InputElementDesc[0].SemanticIndex = 0;
    d3d11InputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    d3d11InputElementDesc[0].InputSlot = 0;
    d3d11InputElementDesc[0].AlignedByteOffset = 0;
    d3d11InputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3d11InputElementDesc[0].InstanceDataStepRate = 0;

    // color
    d3d11InputElementDesc[1].SemanticName = "COLOR";    
    d3d11InputElementDesc[1].SemanticIndex = 0;
    d3d11InputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    d3d11InputElementDesc[1].InputSlot = 1;
    d3d11InputElementDesc[1].AlignedByteOffset = 0;
    d3d11InputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3d11InputElementDesc[1].InstanceDataStepRate = 0;

    // create input layout based on above structrue
    hr = gpID3D11Device->CreateInputLayout(
            d3d11InputElementDesc,
            _ARRAYSIZE(d3d11InputElementDesc),
            pID3DBlob_vertexShaderCode->GetBufferPointer(),
            pID3DBlob_vertexShaderCode->GetBufferSize(),
            &gpID3D11InputLayout
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateInputLayout() failed\n");
        fclose(gpFile);

        if(pID3DBlob_vertexShaderCode)
        {
            pID3DBlob_vertexShaderCode->Release();
            pID3DBlob_vertexShaderCode = NULL;
        }

        if(pID3DBlob_pixelShaderCode)   
        {
            pID3DBlob_pixelShaderCode->Release();
            pID3DBlob_pixelShaderCode = NULL;
        }

        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateInputLayout() Succeeded\n");
        fclose(gpFile);
    }

    // set this Input layout in pipeline
    gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);

    if(pID3DBlob_vertexShaderCode)
    {
        pID3DBlob_vertexShaderCode->Release();
        pID3DBlob_vertexShaderCode = NULL;
    }

    if(pID3DBlob_pixelShaderCode)   
    {
        pID3DBlob_pixelShaderCode->Release();
        pID3DBlob_pixelShaderCode = NULL;
    }

    if(pID3DBlob_error)
    {
        pID3DBlob_error->Release();
        pID3DBlob_error = NULL;
    }

    // provide vertex position, color, normal, texcords etc
    // create buffer for vertex data
    const float triangle_position[] = {
                                            0.0f, 1.0f, 0.0f,
                                            1.0f, -1.0f, 0.0f,
                                            -1.0f, -1.0f, 0.0f
                                        }; 

    const float triangle_color[] = {
                                            1.0f, 0.0f, 0.0f,   // red
                                            0.0f, 0.0f, 1.0f,   // blue
                                            0.0f, 1.0f, 0.0f,   // green
                                        }; 

    const float rectangle_position[] = {
                                            // first triangle
                                            -1.0f, 1.0f, 0.0f,
                                            1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, 0.0f,

                                            // second triangle
                                            -1.0f, -1.0f, 0.0f,
                                            1.0f, 1.0f, 0.0f,
                                            1.0f, -1.0f, 0.0f,
                                        }; 

    const float rectangle_color[] = {
                                            // first triangle
                                            0.0f, 0.0f, 1.0f,
                                            0.0f, 0.0f, 1.0f,
                                            0.0f, 0.0f, 1.0f,

                                            // second triangle
                                            0.0f, 0.0f, 1.0f,
                                            0.0f, 0.0f, 1.0f,
                                            0.0f, 0.0f, 1.0f,
                                        }; 

    // Position Triangle
    D3D11_BUFFER_DESC d3d11BufferDesc;
    ZeroMemory((void*)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;            // gl_STATIC_DRAW 
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(triangle_position);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer 
    D3D11_SUBRESOURCE_DATA d3d11SubResourceData;
    ZeroMemory((void*)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = triangle_position;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(
            &d3d11BufferDesc,
            &d3d11SubResourceData,
            &gpID3D11Buffer_TrianglePositionBuffer
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for triangle Position\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for triangle Position\n");
        fclose(gpFile);
    }

    // Color Triangle
    ZeroMemory((void*)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;            // gl_STATIC_DRAW 
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(triangle_color);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer 
    ZeroMemory((void*)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = triangle_color;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(
            &d3d11BufferDesc,
            &d3d11SubResourceData,
            &gpID3D11Buffer_TriangleColorBuffer
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for triangle Color\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for triangle Color\n");
        fclose(gpFile);
    }

    // Position Rectangle
    ZeroMemory((void*)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;            // gl_STATIC_DRAW 
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(rectangle_position);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer 
    ZeroMemory((void*)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = rectangle_position;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(
            &d3d11BufferDesc,
            &d3d11SubResourceData,
            &gpID3D11Buffer_RectanglePositionBuffer
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for rectangle Position\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for rectangle Position\n");
        fclose(gpFile);
    }

    // Color Rectangle
    ZeroMemory((void*)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;            // gl_STATIC_DRAW 
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(rectangle_color);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer 
    ZeroMemory((void*)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = rectangle_color;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(
            &d3d11BufferDesc,
            &d3d11SubResourceData,
            &gpID3D11Buffer_RectangleColorBuffer
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for rectangle Color\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for rectangle Color\n");
        fclose(gpFile);
    }

    // now create constant buffer
    ZeroMemory((void*)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    d3d11BufferDesc.ByteWidth = sizeof(CBUFFER);
    d3d11BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    d3d11BufferDesc.CPUAccessFlags = 0;

    // create constant buffer 
    hr = gpID3D11Device->CreateBuffer(
            &d3d11BufferDesc,
            NULL,
            &gpID3D11Buffer_ConstantBuffer
        );

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded\n");
        fclose(gpFile);
    }

    // set this empty constant buffer 
    // data will given in display
    gpID3D11DeviceContext->VSSetConstantBuffers(
            0,
            1,
            &gpID3D11Buffer_ConstantBuffer
        );

    // set clear color
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;  
    clearColor[3] = 1.0f;

    // initialize perspective projection matrix
    perspectiveProjectionMatrix = XMMatrixIdentity();

    // warm up resize
    hr = resize(WIN_WIDTH, WIN_HEIGHT);

    if(FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "resize() failed in initialize()\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "resize() succeeded in initialize()\n");
        fclose(gpFile);
    }
    
    return (hr);

    return (0);
}

void printDXInfo(void)
{
    // variable declarations
    IDXGIFactory *pIDXGIFactory = NULL;
    IDXGIAdapter *pIDXGIAdapter = NULL;
    DXGI_ADAPTER_DESC  dxgiAdapterDesc;
    HRESULT hr = S_OK;
    char str[255];

    // get DXGI Factory
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
    if(FAILED(hr))
    {
        fprintf(gpFile, "CreateDXGIFactory() failed for %u\n", hr);
        goto cleanup;
    }

    // From Factory get Adapater
    if(pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        ZeroMemory((void*)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
        pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);

        // to convert WCHAR type of name of graphic card into char type
        WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, 255, str, 255, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, -1, str, 255, NULL, NULL);
        fprintf(gpFile,"Graphic Device Name = %s\n", str);

        fprintf(gpFile, "VRAM(in bytes) = %I64d\n", (__int64)dxgiAdapterDesc.DedicatedVideoMemory);
        fprintf(gpFile, "VRAM(in GB's) = %d\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0 / 1024.0));
    }
    else
    {
        fprintf(gpFile, "IDXGIFactory::EnumAdapters() failed\n");
        goto cleanup;
    }

    cleanup:
    if(pIDXGIAdapter)
    {
        pIDXGIAdapter->Release();
        pIDXGIAdapter = NULL;
    }

    if(pIDXGIFactory)
    {
        pIDXGIFactory->Release();
        pIDXGIFactory = NULL;
    }
}

HRESULT resize(int width, int height)
{
	// code
    HRESULT hr = S_OK;

    // Step 1: release existing render target view
    if(gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    // Step 2: resize swap chain buffers according to the new width and height
    gpIDXGISwapChain->ResizeBuffers(
            1,                  // buffer count
            width,
            height,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0                   // flags
        );

    // Step 3-A: get the buffer from swap chain for render target view
    ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;
    gpIDXGISwapChain->GetBuffer(
            0,                                      // buffer index
            __uuidof(ID3D11Texture2D),              // riid
            (LPVOID*)&pID3D11Texture2D_BackBuffer   // ppBuffer
        );

    // Step 3-B: create render target view using above textured swap chain buffer
    hr = gpID3D11Device->CreateRenderTargetView(
            pID3D11Texture2D_BackBuffer,     // pResource
            NULL,                           // pDesc
            &gpID3D11RenderTargetView       // ppRTView
        );

    if(FAILED(hr))
    {
        if(pID3D11Texture2D_BackBuffer)
        {
            pID3D11Texture2D_BackBuffer->Release();
            pID3D11Texture2D_BackBuffer = NULL;
        }

        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "ID3D11Device::CreateRenderTargetView() failed for %u\n", hr);
        fclose(gpFile);
        return (hr);
    }

    if(pID3D11Texture2D_BackBuffer)
    {
        pID3D11Texture2D_BackBuffer->Release();
        pID3D11Texture2D_BackBuffer = NULL;
    }

    // Step 4: set render target view as render target in pipeline
    gpID3D11DeviceContext->OMSetRenderTargets(      // OM = Output Merger
            1,                                      // NumViews
            &gpID3D11RenderTargetView,              // ppRTViews
            NULL                                    // pDepthStencilView
        );

    // Step 5: initialie viewport structure
    D3D11_VIEWPORT d3dViewPort;
    ZeroMemory((void*)&d3dViewPort, sizeof(D3D11_VIEWPORT));

    d3dViewPort.TopLeftX = 0.0f;
    d3dViewPort.TopLeftY = 0.0f;
    d3dViewPort.Width = (float)width;
    d3dViewPort.Height = (float)height;
    d3dViewPort.MinDepth = 0.0f;
    d3dViewPort.MaxDepth = 1.0f;

    // Step 6: set viewport in pipeline
    gpID3D11DeviceContext->RSSetViewports(      // RS = Rasterizer Stage
            1,                                  // NumViewports
            &d3dViewPort                        // pViewports
        );
    
    // set perspective Projection matrix
    perspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(45.0f),
            (float)width / (float)height,
            0.1f,
            1000.0f
        );

    return (hr);
}

void display(void)
{
    // code
    // Clear color
    gpID3D11DeviceContext->ClearRenderTargetView(
            gpID3D11RenderTargetView,
            clearColor
        );

    // Triangle Position
    // set vertex buffer 
    UINT stride = sizeof(float) * 3;
    UINT offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(
            0,
            1,
            &gpID3D11Buffer_TrianglePositionBuffer,
            &stride,
            &offset
        );

    // Triangle Color
    // set vertex buffer 
    stride = sizeof(float) * 3;
    offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(
            1,
            1,
            &gpID3D11Buffer_TriangleColorBuffer,
            &stride,
            &offset
        );

    // set primitive Topology
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Transformations
    XMMATRIX worldMatrix = XMMatrixIdentity();
    XMMATRIX viewMatrix = XMMatrixIdentity();
    XMMATRIX translationMatrix = XMMatrixIdentity();
    translationMatrix = XMMatrixTranslation(-1.5f, 0.0f, 6.0f);
    XMMATRIX wvpMatrix = XMMatrixIdentity();

    worldMatrix = translationMatrix;

    wvpMatrix = worldMatrix * viewMatrix * perspectiveProjectionMatrix;

    // Now push this wvp matrix into vertex shader
    CBUFFER constantBuffer;
    ZeroMemory((void*)&constantBuffer, sizeof(CBUFFER));
    constantBuffer.worldViewProjectionMatrix = wvpMatrix;

    gpID3D11DeviceContext->UpdateSubresource(
            gpID3D11Buffer_ConstantBuffer,
            0,
            NULL,
            &constantBuffer,
            0,
            0
        );

    gpID3D11DeviceContext->Draw(
            3,
            0
        );

    // Rectangle Position
    // set vertex buffer 
    stride = sizeof(float) * 3;
    offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(
            0,
            1,
            &gpID3D11Buffer_RectanglePositionBuffer,
            &stride,
            &offset
        );

    // Rectangle Color
    // set vertex buffer 
    stride = sizeof(float) * 3;
    offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(
            1,
            1,
            &gpID3D11Buffer_RectangleColorBuffer,
            &stride,
            &offset
        );

    // set primitive Topology
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Transformations
    worldMatrix     = XMMatrixIdentity();
    viewMatrix      = XMMatrixIdentity();
    translationMatrix     = XMMatrixTranslation(1.5f, 0.0f, 6.0f);
    wvpMatrix       = XMMatrixIdentity();

    worldMatrix  = translationMatrix;
    wvpMatrix       = worldMatrix * viewMatrix * perspectiveProjectionMatrix;

    // Now push this wvp matrix into vertex shader
    constantBuffer;
    ZeroMemory((void*)&constantBuffer, sizeof(CBUFFER));
    constantBuffer.worldViewProjectionMatrix = wvpMatrix;

    gpID3D11DeviceContext->UpdateSubresource(
            gpID3D11Buffer_ConstantBuffer,
            0,
            NULL,
            &constantBuffer,
            0,
            0
        );

    gpID3D11DeviceContext->Draw(
            6,
            0
        );

    // present swap chain buffers switch between front and back buffers
    gpIDXGISwapChain->Present(0, 0);
}

void update(void)
{
    // code
}

void uninitialize(void)
{
	// code
    if(gpID3D11Buffer_ConstantBuffer)
    {
        gpID3D11Buffer_ConstantBuffer->Release();
        gpID3D11Buffer_ConstantBuffer = NULL;
    }

    if(gpID3D11VertexShader)
    {
        gpID3D11VertexShader->Release();
        gpID3D11VertexShader = NULL;
    }

    if(gpID3D11PixelShader)
    {
        gpID3D11PixelShader->Release();
        gpID3D11PixelShader = NULL;
    }

    if(gpID3D11Buffer_RectangleColorBuffer)
    {
        gpID3D11Buffer_RectangleColorBuffer->Release();
        gpID3D11Buffer_RectangleColorBuffer = NULL;
    }

    if(gpID3D11Buffer_TriangleColorBuffer)
    {
        gpID3D11Buffer_TriangleColorBuffer->Release();
        gpID3D11Buffer_TriangleColorBuffer = NULL;
    }

    if(gpID3D11Buffer_RectanglePositionBuffer)
    {
        gpID3D11Buffer_RectanglePositionBuffer->Release();
        gpID3D11Buffer_RectanglePositionBuffer = NULL;
    }

    if(gpID3D11Buffer_TrianglePositionBuffer)
    {
        gpID3D11Buffer_TrianglePositionBuffer->Release();
        gpID3D11Buffer_TrianglePositionBuffer = NULL;
    }

    if(gpID3D11InputLayout)
    {
        gpID3D11InputLayout->Release();
        gpID3D11InputLayout = NULL;
    }

    if(gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    if(gpID3D11DeviceContext)
    {
        gpID3D11DeviceContext->Release();
        gpID3D11DeviceContext = NULL;
    }

    if(gpIDXGISwapChain)
    {
        gpIDXGISwapChain->Release();
        gpIDXGISwapChain = NULL;
    }

    if(gpID3D11Device)
    {
        gpID3D11Device->Release();  
        gpID3D11Device = NULL;
    }

    // close the file
    if(gpFile)
    {
        fprintf(gpFile, "Program terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

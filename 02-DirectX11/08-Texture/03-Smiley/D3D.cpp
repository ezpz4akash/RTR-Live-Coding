// Win32 headers
#include <Windows.h>
#include <stdio.h>  // For File IO
#include <stdlib.h> // For exit()
#include <math.h>   // for ceil
#include "D3D.h"

// D3D 11 related header
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

// for D3D11 Math
#pragma warning(disable : 4838)
#include "XNAMath/xnamath.h"

// for texture loading
#include "WICTextureLoader.h"

// dxgi - direct x graphics interface
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
struct WindowPlacementConfig
{
    int x;
    int y;
    int width;
    int height;
};

static WindowPlacementConfig getCenteredWindowPlacement(int width, int height)
{
    WindowPlacementConfig placement;
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    placement.width = width;
    placement.height = height;
    placement.x = (screenWidth - width) / 2;
    placement.y = (screenHeight - height) / 2;

    return placement;
}

// global variable declarations
// variable related to FullScreen
BOOL gbFullScreen = FALSE;
HWND ghwnd = NULL;
DWORD dwStyle; // Local static asala tri chalel
WINDOWPLACEMENT wpPrev;

// variable related to FileIO
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

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
float clearColor[4]; // RGBA

ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_PositionBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_TexCoordBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;

ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

// Texture related variables
ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture = NULL;
ID3D11SamplerState *gpID3D11SamplerState_Texture = NULL;

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

    // Window placement is calculated once so WinMain stays focused on setup flow.
    WindowPlacementConfig windowPlacement = getCenteredWindowPlacement(WIN_WIDTH, WIN_HEIGHT);

    // code

    // create log file
    gpFile = fopen(szLogFileName, "a+");
    // file open modes
    // r = read
    // w = write
    // a = append

    // fopen_s(gszLogFileName, "w");    --windows recommendation
    // fprintf_s()                      --windows recommendation

    if (gpFile == NULL)
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
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // CS_OWNDC => class style own device context
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
    if (!RegisterClassEx(&wndclass))
    {
        MessageBox(NULL, TEXT("Failed to Register Class"), TEXT("RegisterClassEx"), MB_OK | MB_TOPMOST);
        ExitProcess(EXIT_FAILURE);
    }

    // Create Window in Memory
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          szAppName,                                                            // class name
                          TEXT("RTR 6 - Akash Musale: D3D11 Smiley Texture"),                   // window name Caption Bar Name
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, // window style
                          // WS_OVERLAPPEDWINDOW,     // window style
                          windowPlacement.x,      // x co-ordinate
                          windowPlacement.y,      // y co-ordinate
                          windowPlacement.width,  // window width
                          windowPlacement.height, // window height
                          NULL,                   // parent window handle
                          NULL,                   // Menu Bar
                          hInstance,              // Instance of current Handle window
                          NULL                    // window creation parameter
    );

    ghwnd = hwnd;

    // Show window
    ShowWindow(hwnd, iCmdShow);

    // Paint background of window
    UpdateWindow(hwnd);

    // initialize
    hr = initialize();
    if (FAILED(hr))
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

    // uninitialize
    uninitialize();

    return ((int)msg.wParam);
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

    case WM_SIZE:
        if (gpID3D11DeviceContext)
        {
            hr = resize(LOWORD(lParam), HIWORD(lParam));
            if (FAILED(hr))
            {
                gpFile = fopen(szLogFileName, "a+");
                fprintf(gpFile, "resize() failed\n");
                fclose(gpFile);
                return (hr);
            }
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE: // virtual key code @, $, Esc
            gbEscapeKeyIsPressed = TRUE;
            break;

        default:
            break;
        }
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case 'F':
        case 'f':
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
        if (dwStyle & WS_OVERLAPPEDWINDOW) // & => contains
        {
            ZeroMemory((void *)&mi, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

HRESULT initialize(void)
{
    // function declarations
    void printDXInfo(void);
    HRESULT resize(int width, int height);
    HRESULT loadD3DTexture(const wchar_t *, ID3D11ShaderResourceView **);

    // variable declarations
    HRESULT hr = S_OK;

    // code
    printDXInfo();

    // swap chain descriptor initialization
    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
    dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferCount = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.SampleDesc.Count = 1;   // No MultiSampling
    dxgiSwapChainDesc.SampleDesc.Quality = 0; // No MultiSampling
    dxgiSwapChainDesc.OutputWindow = ghwnd;
    dxgiSwapChainDesc.Windowed = TRUE; // start with windowed mode

    // get dxgi swap chain, d3d11 device and d3d11 device context supported driver and supported feature level 5 things at a once
    D3D_DRIVER_TYPE d3dDriverType;
    D3D_DRIVER_TYPE d3dDriverTypes[] = {D3D_DRIVER_TYPE_HARDWARE,
                                        D3D_DRIVER_TYPE_WARP, // windows advanced rasterization platform
                                        D3D_DRIVER_TYPE_SOFTWARE, D3D_DRIVER_TYPE_REFERENCE};

    D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL d3dFeatureLevel_aquired = D3D_FEATURE_LEVEL_10_0;
    UINT numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

    for (UINT i = 0; i < numDriverTypes; i++)
    {
        d3dDriverType = d3dDriverTypes[i];
        hr = D3D11CreateDeviceAndSwapChain(NULL, // Adapter
                                           d3dDriverType,
                                           NULL,                      // Software
                                           0,                         // Flags
                                           &d3dFeatureLevel_required, // Feature Levels
                                           1,                         // count of feature levels
                                           D3D11_SDK_VERSION, &dxgiSwapChainDesc, &gpIDXGISwapChain, &gpID3D11Device,
                                           &d3dFeatureLevel_aquired, &gpID3D11DeviceContext);

        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11CreateDeviceAndSwapChain() failed\n");
        fclose(gpFile);
        return (hr);
    }

    // check which driver is usage
    gpFile = fopen(szLogFileName, "a+");
    if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_HARDWARE\n");
    }
    else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
    {
        fprintf(gpFile, "chosen driver is  D3D_DRIVER_TYPE_WARP\n");
    }
    else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_REFERENCE\n");
    }
    else if (d3dDriverType == D3D_DRIVER_TYPE_SOFTWARE)
    {
        fprintf(gpFile, "chosen driver is D3D_DRIVER_TYPE_SOFTWARE\n");
    }
    else
    {
        fprintf(gpFile, "chosen driver is unknown\n");
    }

    // check which feature level is acquired
    if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_11_0)
    {
        fprintf(gpFile, "acquired feature level is D3D_FEATURE_LEVEL_11_0\n");
    }
    else if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_1)
    {
        fprintf(gpFile, "acquired feature level is D3D_FEATURE_LEVEL_10_1\n");
    }
    else if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_0)
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
    const char *vertexShaderSourceCode = "cbuffer constantBuffer"
                                         "{"
                                         "   float4x4 worldViewProjectionMatrix;"
                                         "}"
                                         "struct vertex_Output"
                                         "{"
                                         "   float4 position : SV_POSITION;"
                                         "   float2 texcoord : TEXCOORD;"
                                         "};"
                                         "vertex_Output main(float4 pos : POSITION, float2 tex : TEXCOORD)"
                                         "{"
                                         "   vertex_Output vertex_output;"
                                         "   vertex_output.position = mul(worldViewProjectionMatrix, pos);"
                                         "   vertex_output.texcoord = tex;"
                                         "   return (vertex_output);"
                                         "}";

    ID3DBlob *pID3DBlob_vertexShaderCode = NULL;
    ID3DBlob *pID3DBlob_error = NULL;

    // compile above shader
    hr = D3DCompile(vertexShaderSourceCode, lstrlenA(vertexShaderSourceCode) + 1,
                    "VS", // Source code string
                    NULL, // D3D_SHADER_MACRO
                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main",                      // Entry point function name
                    "vs_5_0",                    // Shader model
                    0,                           // Flags
                    0,                           // Effect Constant
                    &pID3DBlob_vertexShaderCode, // compiled code thevnya sathi
                    &pID3DBlob_error             // Error for vertex shader
    );

    if (FAILED(hr))
    {
        if (pID3DBlob_error != NULL)
        {
            gpFile = fopen(szLogFileName, "a+");
            fprintf(gpFile, "D3DCompile() failed for vertex shader:%s\n", (char *)pID3DBlob_error->GetBufferPointer());
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
    hr = gpID3D11Device->CreateVertexShader(pID3DBlob_vertexShaderCode->GetBufferPointer(), pID3DBlob_vertexShaderCode->GetBufferSize(),
                                            NULL, // class linkage parameter across shader variable sathi
                                            &gpID3D11VertexShader);

    if (FAILED(hr))
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
    gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, NULL, 0);

    /////////////////////////////////////// Pixel Shader ///////////////////////////////////////
    const char *pixelShaderSourceCode = "struct vertex_Output"
                                        "{"
                                        "   float4 position : SV_POSITION;"
                                        "   float2 texcoord : TEXCOORD;"
                                        "};"
                                        "Texture2D myTexture2D;"
                                        "SamplerState mySamplerState;"
                                        "float4 main(vertex_Output input) : SV_TARGET"
                                        "{"
                                        "   float4 color = myTexture2D.Sample(mySamplerState, input.texcoord);"
                                        "   return (color);"
                                        "}";

    ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
    pID3DBlob_error = NULL;

    // compile pixel shader
    hr = D3DCompile(pixelShaderSourceCode, lstrlenA(pixelShaderSourceCode) + 1,
                    "PS", // Source code string
                    NULL, // D3D_SHADER_MACRO
                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main",                     // Entry point function name
                    "ps_5_0",                   // Shader model
                    0,                          // Flags
                    0,                          // Effect Constant
                    &pID3DBlob_pixelShaderCode, // compiled code thevnya sathi
                    &pID3DBlob_error            // Error for vertex shader
    );

    if (FAILED(hr))
    {
        if (pID3DBlob_error != NULL)
        {
            gpFile = fopen(szLogFileName, "a+");
            fprintf(gpFile, "D3DCompile() failed for pixel shader:%s\n", (char *)pID3DBlob_error->GetBufferPointer());
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
    hr = gpID3D11Device->CreatePixelShader(pID3DBlob_pixelShaderCode->GetBufferPointer(), pID3DBlob_pixelShaderCode->GetBufferSize(),
                                           NULL, // class linkage parameter across shader variable sathi
                                           &gpID3D11PixelShader);

    if (FAILED(hr))
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
    gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, NULL, 0);

    // initialize input layout
    D3D11_INPUT_ELEMENT_DESC d3d11InputElementDesc[2];
    ZeroMemory((void *)d3d11InputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC) * _ARRAYSIZE(d3d11InputElementDesc));

    // position
    d3d11InputElementDesc[0].SemanticName = "POSITION";
    d3d11InputElementDesc[0].SemanticIndex = 0;
    d3d11InputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    d3d11InputElementDesc[0].InputSlot = 0;
    d3d11InputElementDesc[0].AlignedByteOffset = 0;
    d3d11InputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3d11InputElementDesc[0].InstanceDataStepRate = 0;

    // texture
    d3d11InputElementDesc[1].SemanticName = "TEXCOORD";
    d3d11InputElementDesc[1].SemanticIndex = 0;
    d3d11InputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    d3d11InputElementDesc[1].InputSlot = 1;
    d3d11InputElementDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    d3d11InputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3d11InputElementDesc[1].InstanceDataStepRate = 0;

    // create input layout based on above structrue
    hr = gpID3D11Device->CreateInputLayout(d3d11InputElementDesc, _ARRAYSIZE(d3d11InputElementDesc),
                                           pID3DBlob_vertexShaderCode->GetBufferPointer(), pID3DBlob_vertexShaderCode->GetBufferSize(),
                                           &gpID3D11InputLayout);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateInputLayout() failed\n");
        fclose(gpFile);

        if (pID3DBlob_vertexShaderCode)
        {
            pID3DBlob_vertexShaderCode->Release();
            pID3DBlob_vertexShaderCode = NULL;
        }

        if (pID3DBlob_pixelShaderCode)
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

    if (pID3DBlob_vertexShaderCode)
    {
        pID3DBlob_vertexShaderCode->Release();
        pID3DBlob_vertexShaderCode = NULL;
    }

    if (pID3DBlob_pixelShaderCode)
    {
        pID3DBlob_pixelShaderCode->Release();
        pID3DBlob_pixelShaderCode = NULL;
    }

    if (pID3DBlob_error)
    {
        pID3DBlob_error->Release();
        pID3DBlob_error = NULL;
    }

    // provide vertex position, color, normal, texcords etc
    // create buffer for vertex data
    const float rectangle_position[] = {
        -1.0f, +1.0f, -1.0f, // top-left of front
        +1.0f, +1.0f, -1.0f, // top-right of front
        -1.0f, -1.0f, -1.0f, // bottom-left of front

        -1.0f, -1.0f, -1.0f, // bottom-left of front
        +1.0f, +1.0f, -1.0f, // top-right of front
        +1.0f, -1.0f, -1.0f, // bottom-right of front
    };

    const float rectangle_Texcoords[] = {
        0.0f, 1.0f, // top-left of front
        1.0f, 1.0f, // top-right of front
        0.0f, 0.0f, // bottom-left of front

        0.0f, 0.0f, // bottom-left of front
        1.0f, 1.0f, // top-right of front
        1.0f, 0.0f, // bottom-right of front
    };

    // Position
    D3D11_BUFFER_DESC d3d11BufferDesc;
    ZeroMemory((void *)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT; // gl_STATIC_DRAW
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(rectangle_position);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer
    D3D11_SUBRESOURCE_DATA d3d11SubResourceData;
    ZeroMemory((void *)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = rectangle_position;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(&d3d11BufferDesc, &d3d11SubResourceData, &gpID3D11Buffer_PositionBuffer);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for position buffer\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for position buffer\n");
        fclose(gpFile);
    }

    // Texture
    ZeroMemory((void *)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));

    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT; // gl_STATIC_DRAW
    d3d11BufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(rectangle_Texcoords);
    d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // d3d11BufferDesc.CPUAccessFlags = 0;

    // initialize sub resource of buffer
    ZeroMemory((void *)&d3d11SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    d3d11SubResourceData.pSysMem = rectangle_Texcoords;

    // now create actual buffer
    hr = gpID3D11Device->CreateBuffer(&d3d11BufferDesc, &d3d11SubResourceData, &gpID3D11Buffer_TexCoordBuffer);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() failed for texture buffer\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateBuffer() Succeeded for texture buffer\n");
        fclose(gpFile);
    }

    // now create constant buffer
    ZeroMemory((void *)&d3d11BufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    d3d11BufferDesc.ByteWidth = sizeof(CBUFFER);
    d3d11BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    d3d11BufferDesc.CPUAccessFlags = 0;

    // create constant buffer
    hr = gpID3D11Device->CreateBuffer(&d3d11BufferDesc, NULL, &gpID3D11Buffer_ConstantBuffer);

    if (FAILED(hr))
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
    gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

    // set Rasterizer state to disable backface culling so that back of culling face also render during animation
    D3D11_RASTERIZER_DESC d3d11RasterizerDesc;
    ZeroMemory((void *)&d3d11RasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    d3d11RasterizerDesc.AntialiasedLineEnable = FALSE;
    d3d11RasterizerDesc.CullMode = D3D11_CULL_NONE;
    d3d11RasterizerDesc.DepthBias = 0;
    d3d11RasterizerDesc.DepthBiasClamp = 0.0f;
    d3d11RasterizerDesc.DepthClipEnable = TRUE;
    d3d11RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    d3d11RasterizerDesc.FrontCounterClockwise = FALSE; // D3D11 follows clockwise Rule
    d3d11RasterizerDesc.MultisampleEnable = FALSE;
    d3d11RasterizerDesc.ScissorEnable = FALSE;
    d3d11RasterizerDesc.SlopeScaledDepthBias = 0.0f;

    hr = gpID3D11Device->CreateRasterizerState(&d3d11RasterizerDesc, &gpID3D11RasterizerState);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateRasterizerState() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateRasterizerState() Succeeded\n");
        fclose(gpFile);
    }

    // Set this Rasterizer State into this pipeline
    gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

    // create shader resource view
    // In other words load D3D texture to get this shader resource view
    hr = loadD3DTexture(L"Smiley.bmp", &gpID3D11ShaderResourceView_Texture);
    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "loadD3DTexture() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "loadD3DTexture() Succeeded\n");
        fclose(gpFile);
    }

    // create sampler state
    D3D11_SAMPLER_DESC d3d11SamplerDesc;
    ZeroMemory((void *)&d3d11SamplerDesc, sizeof(D3D11_SAMPLER_DESC));

    d3d11SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    d3d11SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    d3d11SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    d3d11SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    hr = gpID3D11Device->CreateSamplerState(&d3d11SamplerDesc, &gpID3D11SamplerState_Texture);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateSamplerState() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "D3D11Device::CreateSamplerState() Succeeded\n");
        fclose(gpFile);
    }

    // set clear color
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    clearColor[3] = 1.0f;

    // initialize perspective projection matrix
    perspectiveProjectionMatrix = XMMatrixIdentity();

    // warm up resize
    hr = resize(WIN_WIDTH, WIN_HEIGHT);

    if (FAILED(hr))
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
}

void printDXInfo(void)
{
    // variable declarations
    IDXGIFactory *pIDXGIFactory = NULL;
    IDXGIAdapter *pIDXGIAdapter = NULL;
    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    HRESULT hr = S_OK;
    char str[255];

    // get DXGI Factory
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);
    if (FAILED(hr))
    {
        fprintf(gpFile, "CreateDXGIFactory() failed for %u\n", hr);
        goto cleanup;
    }

    // From Factory get Adapater
    if (pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        ZeroMemory((void *)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
        pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);

        // to convert WCHAR type of name of graphic card into char type
        WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, 255, str, 255, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, -1, str, 255, NULL, NULL);
        fprintf(gpFile, "Graphic Device Name = %s\n", str);

        fprintf(gpFile, "VRAM(in bytes) = %I64d\n", (__int64)dxgiAdapterDesc.DedicatedVideoMemory);
        fprintf(gpFile, "VRAM(in GB's) = %d\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0 / 1024.0));
    }
    else
    {
        fprintf(gpFile, "IDXGIFactory::EnumAdapters() failed\n");
        goto cleanup;
    }

cleanup:
    if (pIDXGIAdapter)
    {
        pIDXGIAdapter->Release();
        pIDXGIAdapter = NULL;
    }

    if (pIDXGIFactory)
    {
        pIDXGIFactory->Release();
        pIDXGIFactory = NULL;
    }
}

HRESULT loadD3DTexture(const wchar_t *textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
    // variable declarations
    HRESULT hr = S_OK;

    // code
    // create texture and return shader resource view SRV
    hr = DirectX::CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);

    if (FAILED(hr))
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "DirectX::CreateWICTextureFromFile() failed\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "DirectX::CreateWICTextureFromFile() Succeeded\n");
        fclose(gpFile);
    }

    return (hr);
}

HRESULT resize(int width, int height)
{
    // code
    HRESULT hr = S_OK;

    // release the depth stencil view if already present
    if (gpID3D11DepthStencilView)
    {
        gpID3D11DepthStencilView->Release();
        gpID3D11DepthStencilView = NULL;
    }

    // Step 1: release existing render target view
    if (gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    // Step 2: resize swap chain buffers according to the new width and height
    gpIDXGISwapChain->ResizeBuffers(1, // buffer count
                                    width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                    0 // flags
    );

    // Step 3-A: get the buffer from swap chain for render target view
    ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;
    gpIDXGISwapChain->GetBuffer(0,                                     // buffer index
                                __uuidof(ID3D11Texture2D),             // riid
                                (LPVOID *)&pID3D11Texture2D_BackBuffer // ppBuffer
    );

    // Step 3-B: create render target view using above textured swap chain buffer
    hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, // pResource
                                                NULL,                        // pDesc
                                                &gpID3D11RenderTargetView    // ppRTView
    );

    if (FAILED(hr))
    {
        if (pID3D11Texture2D_BackBuffer)
        {
            pID3D11Texture2D_BackBuffer->Release();
            pID3D11Texture2D_BackBuffer = NULL;
        }

        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "ID3D11Device::CreateRenderTargetView() failed for %u\n", hr);
        fclose(gpFile);
        return (hr);
    }

    if (pID3D11Texture2D_BackBuffer)
    {
        pID3D11Texture2D_BackBuffer->Release();
        pID3D11Texture2D_BackBuffer = NULL;
    }

    // texture property of color buffer of RTV are already set by WSI by system, our job is to just get it, into texturing interface. that
    // is what we did in above GetBuffer() and CreateRenderTargetView()

    // This is not case in depth stencil buffer, it cannot be get from WSI, it has to be created by new own your own, and hence its texture
    // property has to be set by us, not by WSI

    D3D11_TEXTURE2D_DESC d3d11Texture2DDesc;
    ZeroMemory((void *)&d3d11Texture2DDesc, sizeof(D3D11_TEXTURE2D_DESC));

    d3d11Texture2DDesc.Width = (UINT)width;
    d3d11Texture2DDesc.Height = (UINT)height;
    d3d11Texture2DDesc.ArraySize = 1;
    d3d11Texture2DDesc.MipLevels = 1;
    d3d11Texture2DDesc.SampleDesc.Count = 1;
    d3d11Texture2DDesc.SampleDesc.Quality = 0;
    d3d11Texture2DDesc.Format = DXGI_FORMAT_D32_FLOAT;
    d3d11Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
    d3d11Texture2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    d3d11Texture2DDesc.CPUAccessFlags = 0;
    d3d11Texture2DDesc.MiscFlags = 0;

    // Now create 2D texture from above description
    ID3D11Texture2D *pID3D11Texture2D_DepthStencilBuffer = NULL;

    hr = gpID3D11Device->CreateTexture2D(&d3d11Texture2DDesc, NULL, &pID3D11Texture2D_DepthStencilBuffer);

    if (FAILED(hr))
    {
        if (pID3D11Texture2D_DepthStencilBuffer)
        {
            pID3D11Texture2D_DepthStencilBuffer->Release();
            pID3D11Texture2D_DepthStencilBuffer = NULL;
        }

        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "ID3D11Device::CreateTexture2D() failed for %u\n", hr);
        fclose(gpFile);
        return (hr);
    }

    // Now create depth stencil view from above created depth stencil buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC d3d11DepthStencilViewDesc;
    ZeroMemory((void *)&d3d11DepthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    d3d11DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    d3d11DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; // multisampled depth stencil view

    hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthStencilBuffer, &d3d11DepthStencilViewDesc, &gpID3D11DepthStencilView);

    if (FAILED(hr))
    {
        if (pID3D11Texture2D_DepthStencilBuffer)
        {
            pID3D11Texture2D_DepthStencilBuffer->Release();
            pID3D11Texture2D_DepthStencilBuffer = NULL;
        }

        gpFile = fopen(szLogFileName, "a+");
        fprintf(gpFile, "ID3D11Device::CreateDepthStencilView() failed for %u\n", hr);
        fclose(gpFile);
        return (hr);
    }

    if (pID3D11Texture2D_DepthStencilBuffer)
    {
        pID3D11Texture2D_DepthStencilBuffer->Release();
        pID3D11Texture2D_DepthStencilBuffer = NULL;
    }

    // Step 4: set render target view and depth stencil view in pipeline
    gpID3D11DeviceContext->OMSetRenderTargets( // OM = Output Merger
        1,                                     // NumViews
        &gpID3D11RenderTargetView,             // ppRTViews
        gpID3D11DepthStencilView               // pDepthStencilView
    );

    // Step 5: initialie viewport structure
    D3D11_VIEWPORT d3dViewPort;
    ZeroMemory((void *)&d3dViewPort, sizeof(D3D11_VIEWPORT));

    d3dViewPort.TopLeftX = 0.0f;
    d3dViewPort.TopLeftY = 0.0f;
    d3dViewPort.Width = (float)width;
    d3dViewPort.Height = (float)height;
    d3dViewPort.MinDepth = 0.0f;
    d3dViewPort.MaxDepth = 1.0f;

    // Step 6: set viewport in pipeline
    gpID3D11DeviceContext->RSSetViewports( // RS = Rasterizer Stage
        1,                                 // NumViewports
        &d3dViewPort                       // pViewports
    );

    // set perspective Projection matrix
    perspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    return (hr);
}

void display(void)
{
    // code
    // Clear color
    gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, clearColor);

    // Clear depth
    gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Position
    // set vertex buffer
    UINT stride = sizeof(float) * 3;
    UINT offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_PositionBuffer, &stride, &offset);

    // Texture
    // set vertex buffer
    stride = sizeof(float) * 2;
    offset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_TexCoordBuffer, &stride, &offset);

    // Set shader resource view to pipeline
    gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture);

    // Set sampler state to pipeline
    gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture);

    // set primitive Topology
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Transformations
    XMMATRIX worldMatrix = XMMatrixIdentity();
    XMMATRIX translationMatrix = XMMatrixIdentity();
    XMMATRIX viewMatrix = XMMatrixIdentity();
    XMMATRIX wvpMatrix = XMMatrixIdentity();
    translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 6.0f);

    worldMatrix = translationMatrix; // order is important

    wvpMatrix = worldMatrix * viewMatrix * perspectiveProjectionMatrix;

    // Now push this wvp matrix into vertex shader
    CBUFFER constantBuffer;
    ZeroMemory((void *)&constantBuffer, sizeof(CBUFFER));
    constantBuffer.worldViewProjectionMatrix = wvpMatrix;

    gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

    gpID3D11DeviceContext->Draw(6, 0);

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
    if (gpID3D11SamplerState_Texture)
    {
        gpID3D11SamplerState_Texture->Release();
        gpID3D11SamplerState_Texture = NULL;
    }

    if (gpID3D11ShaderResourceView_Texture)
    {
        gpID3D11ShaderResourceView_Texture->Release();
        gpID3D11ShaderResourceView_Texture = NULL;
    }

    if (gpID3D11Buffer_ConstantBuffer)
    {
        gpID3D11Buffer_ConstantBuffer->Release();
        gpID3D11Buffer_ConstantBuffer = NULL;
    }

    if (gpID3D11VertexShader)
    {
        gpID3D11VertexShader->Release();
        gpID3D11VertexShader = NULL;
    }

    if (gpID3D11PixelShader)
    {
        gpID3D11PixelShader->Release();
        gpID3D11PixelShader = NULL;
    }

    if (gpID3D11Buffer_TexCoordBuffer)
    {
        gpID3D11Buffer_TexCoordBuffer->Release();
        gpID3D11Buffer_TexCoordBuffer = NULL;
    }

    if (gpID3D11Buffer_PositionBuffer)
    {
        gpID3D11Buffer_PositionBuffer->Release();
        gpID3D11Buffer_PositionBuffer = NULL;
    }

    if (gpID3D11RasterizerState)
    {
        gpID3D11RasterizerState->Release();
        gpID3D11RasterizerState = NULL;
    }

    if (gpID3D11InputLayout)
    {
        gpID3D11InputLayout->Release();
        gpID3D11InputLayout = NULL;
    }

    if (gpID3D11DepthStencilView)
    {
        gpID3D11DepthStencilView->Release();
        gpID3D11DepthStencilView = NULL;
    }

    if (gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    if (gpID3D11DeviceContext)
    {
        gpID3D11DeviceContext->Release();
        gpID3D11DeviceContext = NULL;
    }

    if (gpIDXGISwapChain)
    {
        gpIDXGISwapChain->Release();
        gpIDXGISwapChain = NULL;
    }

    if (gpID3D11Device)
    {
        gpID3D11Device->Release();
        gpID3D11Device = NULL;
    }

    // close the file
    if (gpFile)
    {
        fprintf(gpFile, "Program terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

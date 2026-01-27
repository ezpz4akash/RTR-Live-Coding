// headers
#include <math.h>
#include <stdio.h>
#include <windows.h>

#include <d3d11.h>

#include "D3D11.h"

// libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variables
bool gbFullscreen = false;
bool gbActiveWindow = false;

HWND ghwnd = NULL;
FILE *gpFile = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

// log macro
#define LogD(...)                                                              \
  fopen_s(&gpFile, "RMCLog.txt", "a");                                         \
  fprintf(gpFile, __VA_ARGS__);                                                \
  fclose(gpFile);

// D3D variables
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

float gClearColor[4];

// WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int iCmdShow) {
  // function declarations
  HRESULT initialize(void);
  void display(void);
  void update(void);
  void uninitialize(void);

  // variable declarations
  bool bDone = false;
  WNDCLASSEX wndclass;
  HWND hwnd;
  MSG msg;
  TCHAR szAppName[] = TEXT("MyApp");
  HRESULT hr;

  // code
  LogD("==== Application Started ====\n");

  // initialization of WNDCLASSEX
  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.lpfnWndProc = WndProc;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MIDORIA_ICON));
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszClassName = szAppName;
  wndclass.lpszMenuName = NULL;
  wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MIDORIA_ICON));

  // register above class
  RegisterClassEx(&wndclass);

  // get the screen size
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  // create window
  hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("D3D11 | Blue Screen"),
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
                            WS_CLIPSIBLINGS | WS_VISIBLE,
                        (width / 2) - 400, (height / 2) - 300, WIN_WIDTH,
                        WIN_HEIGHT, NULL, NULL, hInstance, NULL);

  ghwnd = hwnd;

  ShowWindow(hwnd, iCmdShow);
  SetForegroundWindow(hwnd);
  SetFocus(hwnd);

  hr = initialize();
  if (FAILED(hr)) {
    LogD("initialize() failed..\n");
    uninitialize();
    DestroyWindow(ghwnd);
    ghwnd = NULL;
  }
  LogD("initialize() successful..\n");

  // Game Loop!
  while (bDone == false) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        bDone = true;
      else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    } else {
      if (gbActiveWindow == true) {
        update();
        display();
      }
    }
  }

  return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  // function declaration
  HRESULT resize(int, int);
  void uninitialize();
  void ToggleFullscreen(void);

  HRESULT hr;

  // code
  switch (iMsg) {

  case WM_SETFOCUS:
    gbActiveWindow = true;
    break;

  case WM_KILLFOCUS:
    gbActiveWindow = false;
    break;

  case WM_SIZE:
    if (gpID3D11DeviceContext) {

      hr = resize(LOWORD(lParam), HIWORD(lParam));
      if (FAILED(hr)) {
        LogD("resize() failed..\n");
        return (hr);
      }
      LogD("resize() successful..\n");
    }
    break;

  case WM_KEYDOWN:
    switch (wParam) {
    case VK_ESCAPE:
      DestroyWindow(hwnd);
      break;

    case 0x46:
    case 0x66:
      ToggleFullscreen();
      break;

    default:
      break;
    }
    break;

  case WM_ERASEBKGND:
    return (0);

  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    uninitialize();
    PostQuitMessage(0);
    break;

  default:
    break;
  }

  return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void) {
  // local variables
  MONITORINFO mi = {sizeof(MONITORINFO)};

  // code
  if (gbFullscreen == false) {
    dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
      if (GetWindowPlacement(ghwnd, &wpPrev) &&
          GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOZORDER | SWP_FRAMECHANGED);
      }
    }
    ShowCursor(FALSE);
    gbFullscreen = true;
  } else {
    SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(ghwnd, &wpPrev);
    SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOOWNERZORDER);
    ShowCursor(TRUE);
    gbFullscreen = false;
  }
}

HRESULT D3DDeviceInfo(void) {
  // local variable declarations
  IDXGIFactory *pIDXGIFactory = NULL;
  IDXGIAdapter *pIDXGIAdapter = NULL; // ~ physical graphics device

  DXGI_ADAPTER_DESC dxgiAdapterDesc;
  HRESULT hr;
  char str[255];

  // code

  // coInitialize() and coCreateInstance() is internally called by
  // CreateDXGIFactory() method
  hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);
  if (FAILED(hr)) {
    LogD("CreateDXGIFactory() failed..\n");
    goto CLEANUP;
  }

  // enumerate all compatible adapters
  if (pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter) == DXGI_ERROR_NOT_FOUND) {
    LogD("Cannot find DXGIAdapter..\n");
    goto CLEANUP;
  }

  // fetch adapter descre
  ZeroMemory((void *)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
  hr = pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);
  if (FAILED(hr)) {
    LogD("GetDesc() failed..\n");
    goto CLEANUP;
  }

  // convert wide char string to ansi string
  WideCharToMultiByte(CP_ACP, // ANSI CODE PAGE
                      0,
                      dxgiAdapterDesc.Description, // source string
                      255,                         // size of input string
                      str,                         // destination buffer
                      255,                         // size of destination buffer
                      NULL, NULL);

  LogD("GPU      : %s\n", str);
  LogD("GPU VRAM : %d GB\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory /
                                       1024.0 / 1024.0 / 1024.0));

CLEANUP:
  if (pIDXGIAdapter) {
    pIDXGIAdapter->Release();
    pIDXGIAdapter = NULL;
  }

  if (pIDXGIFactory) {
    pIDXGIFactory->Release();
    pIDXGIFactory = NULL;
  }

  return (hr);
}

HRESULT initialize(void) {
  // function declarations
  HRESULT resize(int, int);
  void uninitialize(void);
  HRESULT D3DDeviceInfo(void);

  // variables
  HRESULT hr;

  D3D_DRIVER_TYPE d3dDriverType;
  D3D_DRIVER_TYPE d3dDriverTypes[] = {D3D_DRIVER_TYPE_HARDWARE,
                                      D3D_DRIVER_TYPE_WARP,
                                      D3D_DRIVER_TYPE_REFERENCE};

  D3D_FEATURE_LEVEL d3dFeatureLevelRequired = D3D_FEATURE_LEVEL_11_0;
  D3D_FEATURE_LEVEL d3dFeatureLevelAcquired = D3D_FEATURE_LEVEL_10_0;

  UINT createDeviceFlags = 0;
  UINT numDriverTypes = 0;
  UINT numFeatureLevels = 1;

  // code
  numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

  // 0. D3D11 device log
  hr = D3DDeviceInfo();
  if (FAILED(hr)) {
    LogD("D3DDeviceInfo() failed..\n");
    return (hr);
  }

  // 1. create Swap Chain desc
  DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
  ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
  dxgiSwapChainDesc.BufferCount = 1;
  dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
  dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
  dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
  dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  dxgiSwapChainDesc.OutputWindow = ghwnd;
  dxgiSwapChainDesc.SampleDesc.Count = 1;
  dxgiSwapChainDesc.SampleDesc.Quality = 0;
  dxgiSwapChainDesc.Windowed = TRUE;

  // 2. create Swap chain and device context using swap chain desc
  for (UINT driverType = 0; driverType < numDriverTypes; driverType++) {
    d3dDriverType = d3dDriverTypes[driverType];
    hr = D3D11CreateDeviceAndSwapChain(
        NULL,                     // Adapter
        d3dDriverType,            // Driver Type
        NULL,                     // Software Rasterizer
        createDeviceFlags,        // Flags (Debug, Multithreaded etc)
        &d3dFeatureLevelRequired, // Required Feature Levels
        numFeatureLevels,         // Number of Feature Levels
        D3D11_SDK_VERSION,        // Direct3D SDK version
        &dxgiSwapChainDesc,       // Swap Chain Desc
        &gpIDXGISwapChain,        // Swap Chain
        &gpID3D11Device,          // Device
        &d3dFeatureLevelAcquired, // Acquired Feature Level
        &gpID3D11DeviceContext    // Device Context
    );
    if (SUCCEEDED(hr))
      break;
  }

  if (FAILED(hr)) {
    LogD("D3D11CreateDeviceAndSwapChain() failed..\n");
    return (hr);
  }

  LogD("D3D11CreateDeviceAndSwapChain() successful..\n");

  LogD("The Chosen Driver is of ");
  if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE) {
    LogD("Hardware Type. \n");
  } else if (d3dDriverType == D3D_DRIVER_TYPE_WARP) {
    LogD("Warp Type. \n");
  } else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE) {
    LogD("Reference Type. \n");
  } else {
    LogD("Unknown Type. \n");
  }

  LogD("The Supported Highest Feature Level is ");
  if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_11_0) {
    LogD("11.0. \n");
  } else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_1) {
    LogD("10.1. \n");
  } else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_0) {
    LogD("10.0. \n");
  } else {
    LogD("Unknown. \n");
  }

  // set clear color
  gClearColor[0] = 0.0f;
  gClearColor[1] = 0.0f;
  gClearColor[2] = 1.0f;
  gClearColor[3] = 0.0f;

  // warm-up resize call
  hr = resize(WIN_WIDTH, WIN_HEIGHT);
  if (FAILED(hr)) {
    LogD("resize() failed..\n");
    return (hr);
  } else {
    LogD("resize() successful..\n");
  }

  return (S_OK);
}

HRESULT resize(int width, int height) {
  // code
  HRESULT hr = S_OK;

  // free all size dependent resources
  if (gpID3D11RenderTargetView) {
    gpID3D11RenderTargetView->Release();
    gpID3D11RenderTargetView = NULL;
  }

  // resize swap chain buffers
  gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                  0);

  // dummy texture buffer for render target
  ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;
  gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                              (LPVOID *)&pID3D11Texture2D_BackBuffer);

  // create render target view using above back buffer
  hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL,
                                              &gpID3D11RenderTargetView);
  if (FAILED(hr)) {
    LogD("CreateRenderTargetView() failed..\n");
    return (hr);
  } else {
    LogD("CreateRenderTargetView() successful..\n");
  }

  pID3D11Texture2D_BackBuffer->Release();
  pID3D11Texture2D_BackBuffer = NULL;

  // set render target view as render output
  gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

  // set viewport
  D3D11_VIEWPORT d3d11Viewport;
  ZeroMemory((void *)&d3d11Viewport, sizeof(D3D11_VIEWPORT));
  d3d11Viewport.TopLeftX = 0;
  d3d11Viewport.TopLeftY = 0;
  d3d11Viewport.Width = (float)width;
  d3d11Viewport.Height = (float)height;
  d3d11Viewport.MinDepth = 0.0f;
  d3d11Viewport.MaxDepth = 1.0f;

  gpID3D11DeviceContext->RSSetViewports(1, &d3d11Viewport);

  return (hr);
}

void display(void) {
  // code

  // clear render target view
  gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView,
                                               gClearColor);

  // Swap Buffers!
  gpIDXGISwapChain->Present(1, 0);
}

void update(void) {
  // code
}

void uninitialize(void) {
  // code
  if (gpID3D11RenderTargetView) {
    gpID3D11RenderTargetView->Release();
    gpID3D11RenderTargetView = NULL;
  }

  if (gpID3D11DeviceContext) {
    gpID3D11DeviceContext->Release();
    gpID3D11DeviceContext = NULL;
  }

  if (gpID3D11Device) {
    gpID3D11Device->Release();
    gpID3D11Device = NULL;
  }

  if (gpIDXGISwapChain) {
    gpIDXGISwapChain->Release();
    gpIDXGISwapChain = NULL;
  }

  LogD("==== Application Terminated ====\n");
}
// Win32 headers
#include <Windows.h>
#include <stdio.h>          // For File IO
#include <stdlib.h>         // For exit()
#include <math.h>           // for ceil
#include "Window.h"

// D3D 11 related header
#include <dxgi.h>
#include <d3d11.h>  

// dxgi - direct x graphics interface 
#pragma comment(lib, "dxgi.lib")        
#pragma comment(lib, "d3d11.lib")
// #pragma comment(linker, "/SUBSYSTEM:CONSOLE")

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

// Entry-Point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // function declarations
    int initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);
    
    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR6");
    BOOL bDone = FALSE;

    // Window center related variable
    int windowWidth;
    int windowHeight;
    int xPosWindow;
    int yPosWindow;
    int screenWidth;
    int screenHeight;
    
    // code

    // create log file
    gpFile = fopen(gszLogFileName, "w");
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
            TEXT("RTR 6 - Akash Musale"),  // window name Caption Bar Name
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
    int result = initialize();
    if(result != 0)
    {
        fprintf(gpFile, "initialize() failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "initialize() Completed Successfully\n");
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
    void resize(int width, int height);
    void uninitialize(void);

    // variable declarations

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
            resize(LOWORD(lParam), HIWORD(lParam));
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

int initialize(void)
{
    // function declarations
    void printDXInfo(void);

    // code
    printDXInfo();

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

void resize(int width, int height)
{
	// code
}

void display(void)
{
    // code
}

void update(void)
{
    // code
}

void uninitialize(void)
{
	// code
    // close the file
    if(gpFile)
    {
        fprintf(gpFile, "Program terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

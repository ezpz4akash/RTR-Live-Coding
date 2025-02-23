// Win32 Headers
#include <Windows.h>

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Entry Point Functions
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow){
    //variable declarations
    WNDCLASSEX wndClass;
    HWND hWnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR 6");

    // WindowClass Initialization
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style  = CS_HREDRAW | CS_VREDRAW;
    wndClass.cbClsExtra = 0;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.lpszClassName = szAppName;
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Registration Of WindowClass
    RegisterClassEx(&wndClass);

    // Create Window
    
    hWnd = CreateWindow(szAppName, TEXT("RTR 6 - Akash Musale"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    // Show Windows
    ShowWindow(hWnd, iCmdShow);

    // Paint Background of the Window
    UpdateWindow(hWnd);

    // Message Loop
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    TCHAR szBuffer[10];
    switch(uMsg){
        case WM_CREATE:
            OutputDebugString(TEXT("This is the first message\n"));
        break;
        case WM_SIZE:
            OutputDebugString(TEXT("Window size is changed\n"));
        break;
        case WM_MOVE:
            OutputDebugString(TEXT("Window is moved\n"));
        break;
        case WM_CHAR:
            switch(wParam){
                case 'F':
                    OutputDebugString(TEXT("F Key Is Pressed (WM_CHAR)\n"));
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(wParam){
                case VK_ESCAPE:
                    OutputDebugString(TEXT("Escape Key Is Pressed (WM_KEYDOWN)\n"));
                break;
                case 0x46:
                    OutputDebugString(TEXT("F VK Key Is Pressed (WM_KEYDOWN)\n"));
                break;
                default:
                break;
            }
        break;
        case WM_KEYUP:
            switch(wParam){
                case 0x46:
                    OutputDebugString(TEXT("F VK Key Is Up (WM_KEYUP)\n"));
                break;
                default:
                break;
            }
        break;
        case WM_LBUTTONDOWN:
            OutputDebugString(TEXT("Mouse Left Button Is Pressed\n"));
        break;
        case WM_CLOSE:
            OutputDebugString(TEXT("Window is closed (WM_CLOSE)\n"));
            DestroyWindow(hWnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


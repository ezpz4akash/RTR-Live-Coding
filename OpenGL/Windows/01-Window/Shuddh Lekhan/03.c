// Win32 Headers
#include <Windows.h>

// Global Function Declarations
HRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow){
    // Variable declarations
    WNDCLASSEX wndClass;
    HWND hWnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR6");

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;

    wndClass.lpfnWndProc = WndProc;
    wndClass.lpszClassName = szAppName;
    wndClass.lpszMenuName = NULL;
    
    wndClass.hInstance = hInstance;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    wndClass.style = CS_HREDRAW | CS_VREDRAW;

    // Class Registrations
    RegisterClassEx(&wndClass);

    // Create Window
    hWnd = CreateWindow(szAppName, TEXT("Day 03"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    // Show Window
    ShowWindow(hWnd, iCmdShow);

    // Paint Window
    UpdateWindow(hWnd);

    // Message Loop
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return ((int)msg.wParam);
}

HRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
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
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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
    HDC hdc;
    RECT rc;
    TCHAR str[] = TEXT("Hello World!!!");
    PAINTSTRUCT ps;
    static int iPaintFlag = -1;
    switch(uMsg){
        case WM_PAINT:
            GetClientRect(hWnd, &rc);
            hdc = BeginPaint(hWnd, &ps);
            SetBkColor(hdc, RGB(0, 0, 0));
            if(iPaintFlag == 1)
                SetTextColor(hdc, RGB(255, 0, 0));
            else if(iPaintFlag == 2)
                SetTextColor(hdc, RGB(0, 255, 0));
            else if(iPaintFlag == 3)
                SetTextColor(hdc, RGB(0, 0, 255));
            else if(iPaintFlag == 4)
                SetTextColor(hdc, RGB(255, 255, 0));
            else if(iPaintFlag == 5)
                SetTextColor(hdc, RGB(0, 255, 255));
            else if(iPaintFlag == 6)
                SetTextColor(hdc, RGB(255, 0, 255));
            else if(iPaintFlag == 7)
                SetTextColor(hdc, RGB(255, 255, 255));
            DrawText(hdc, str, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            EndPaint(hWnd, &ps);
        break;
        case WM_CHAR:
            switch(wParam){
                case 'R':
                case 'r':
                    iPaintFlag = 1;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'G':
                case 'g':
                    iPaintFlag = 2;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'B':
                case 'b':
                    iPaintFlag = 3;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'Y':
                case 'y':
                    iPaintFlag = 4;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'C':
                case 'c':
                    iPaintFlag = 5;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'M':
                case 'm':
                    iPaintFlag = 6;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
                case 'W':
                case 'w':
                    iPaintFlag = 7;
                    InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


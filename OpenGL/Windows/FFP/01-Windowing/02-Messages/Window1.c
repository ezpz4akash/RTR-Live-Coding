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
            MessageBox(hWnd, TEXT("This is the first message"), TEXT("WM_CREATE"), MB_OK);
        break;
        case WM_SIZE:
            MessageBox(hWnd, TEXT("Window size is changed"), TEXT("WM_SIZE"), MB_OK);
        break;
        case WM_MOVE:
            MessageBox(hWnd, TEXT("Window is moved"), TEXT("WM_MOVE"), MB_OK);
        break;
        case WM_CHAR:
            switch(wParam){
                case 'F':
                    MessageBox(hWnd, TEXT("F Key Is Pressed"), TEXT("WM_CHAR"), MB_OK);
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(wParam){
                case VK_ESCAPE:
                    MessageBox(hWnd, TEXT("Escape Key Is Pressed"), TEXT("WM_KEYDOWN"), MB_OK);
                break;
                case 0x46:
                    MessageBox(hWnd, TEXT("F VK Key Is Pressed"), TEXT("WM_KEYDOWN"), MB_OK);
                break;
                default:
                break;
            }
        break;
        case WM_KEYUP:
            switch(wParam){
                case 0x46:
                    MessageBox(hWnd, TEXT("F VK Key Is Up"), TEXT("WM_KEYUP"), MB_OK);
                break;
                default:
                break;
            }
        break;
        case WM_LBUTTONDOWN:
            MessageBox(hWnd, TEXT("Mouse Left Button Is Pressed"), TEXT("WM_LBUTTONDOWN"), MB_OK);
        break;
        case WM_CLOSE:
            MessageBox(hWnd, TEXT("Window is closed"), TEXT("WM_CLOSE"), MB_OK);
            DestroyWindow(hWnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


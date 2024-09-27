#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <new>
#include <vector>
#include <windowsx.h>
#include <cwchar>
#include "resource.h"

#define WM_MOUSEMOVE_UPDATE (WM_USER + 1)


/****************** Global variables ********************/

HWND hTargetWindow;  
// Khai báo hook
HHOOK hMouseHook;

static TCHAR xPos[10]; static TCHAR yPos[10];
int x, y; // coordinates mouse (x,y) (int, int)
COLORREF color;
TCHAR colorBuffer[10];

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("Mouse Logger");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) { // Nếu hook đang hoạt động
        MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

        switch (wParam) {
			case WM_MOUSEMOVE:
            {
                x = pMouseStruct->pt.x;
                y = pMouseStruct->pt.y;

                // Convert them to TCHAR strings and store them
                wsprintf(xPos, TEXT("%d"), x);
                wsprintf(yPos, TEXT("%d"), y);

                PostMessage(hTargetWindow, WM_MOUSEMOVE_UPDATE, 0, 0);
			}

            break;
        }
			
        return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
    }
}



// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
)
{

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 100,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }
    // set target widow is current window
    hTargetWindow = hWnd;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    if (hMouseHook == NULL)
    {
        MessageBox(NULL,
            _T("Failed to install mouse hook!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int cxClientMax, cyClientMax, cxClient, cyClient, cxChar, cyChar;
    static int cLinesMax, cLines;
    static PMSG pmsg;
 
    static TCHAR header[] = TEXT("    x            y          color(RGB)     ");

    TEXTMETRIC   tm;
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch (message)
    {
    case WM_CREATE:
    {
        // Get maximum size of client area
        cxClientMax = GetSystemMetrics(SM_CXMAXIMIZED);
        cyClientMax = GetSystemMetrics(SM_CYMAXIMIZED);

        // Get character size for fixed−pitch font
        hdc = GetDC(hWnd);
        SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
        GetTextMetrics(hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight;
        ReleaseDC(hWnd, hdc);
    }
    break;

    case WM_SIZE:
    {
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
    }
    break;

    case WM_MOUSEMOVE_UPDATE:
    {
        // get color of specified pixel
        HDC hdc = GetDC(NULL);
        color = GetPixel(hdc, x, y);  
        BYTE red = GetRValue(color);
        BYTE green = GetGValue(color);
        BYTE blue = GetBValue(color);
        // format into hexa string
        swprintf(colorBuffer, 10, _T("#%02X%02X%02X"), red, green, blue);
        ReleaseDC(hWnd, hdc);

        InvalidateRect(hWnd, NULL, TRUE);  // Invalidate the window to trigger WM_PAINT
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hWnd, &ps);
        // Draw header
        TextOut(hdc, 0, 0, header, lstrlen(header));
        // Draw the pressed character in the window
        TextOut(hdc, cxChar, cyChar, xPos, lstrlen(xPos));
        TextOut(hdc, cxChar*8, cyChar, yPos, lstrlen(yPos));
        TextOut(hdc, cxChar*15, cyChar, colorBuffer, lstrlen(colorBuffer));

        // Draw retangle with specified color
        HBRUSH hBrush = CreateSolidBrush(color);
        HBRUSH hNewBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, cxChar * 24, cyChar, cxChar * 26, cyChar * 2);
        // clean created brush and restore old one
        SelectObject(hdc, hNewBrush);
        DeleteObject(hBrush);

        EndPaint(hWnd, &ps);
    }
	break;

    // This message is sent after the window is removed from the screen
    case WM_DESTROY:
    {
        if (hMouseHook)
        {
            UnhookWindowsHookEx(hMouseHook); // Unhook the mouse hook to release resources
        }
        PostQuitMessage(0);
    }
	break;

    case WM_CLOSE:
    {
		DestroyWindow(hWnd);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
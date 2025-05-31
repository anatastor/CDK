
#include "platform/platform.h"
#include "core/input.h"

#if defined(CDK_PLATFORM_WIN)

#include <time.h>
#include <windows.h>
#include <windowsx.h> // parameter input extraction 

#define REGISTER_NAME   "cdkWindow"


static uint8 running = CDK_TRUE;

typedef struct InternalState
{
    HINSTANCE hInstance;
    HWND hwnd;
} InternalState;


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  { 
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        uint8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        uint16 key = (uint16)wParam;
        input_process_keyboard (key, pressed);
	break;
    }

    case WM_MOUSEMOVE:
    {
      uint32 x = GET_X_LPARAM (lParam);
      uint32 y = GET_Y_LPARAM (lParam);
      input_process_mouse_move (x, y);
      break;
    }


    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    {   
        uint8 pressed = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN);
        switch (msg)
        {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                input_process_mouse_button (CDK_MOUSEBUTTON_LEFT, pressed);
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                input_process_mouse_button (CDK_MOUSEBUTTON_RIGHT, pressed);
                break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                input_process_mouse_button (CDK_MOUSEBUTTON_MIDDLE, pressed);
                break;
        }

        break;
    }

    case WM_DESTROY: // handle window closing
      PostQuitMessage(0);
      running = CDK_FALSE;
      break;
  }
  
  return DefWindowProc(hwnd, msg, wParam, lParam); // defined by preprocessor if ...A or ...W
}


uint8 cdk_platform_create (PlatformState* pltState,
        const char* name,
        uint32 x, uint32 y,
        uint32 width, uint32 height)
{
    pltState->iState = malloc (sizeof (InternalState));
    InternalState* iState = pltState->iState;

    uint8 ret = GetModuleHandleExA (0, NULL, &iState->hInstance); // instead fo GetModuleHandleA because of possible race conditions
    if (ret == 0)
    {
        uint64 errorCode = GetLastError ();
        cdk_log_error ("win32 error: %lu", errorCode);
        return CDK_FALSE;
    }


    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof (WNDCLASSEX);
    // windowClass.style = CS_SAVEBITS | CS_DBLCLKS; // enable double clicks
    windowClass.style = CS_HREDRAW | CS_VREDRAW; // enable double clicks
    windowClass.lpfnWndProc = WndProc; // callback function
    windowClass.cbClsExtra = 0; // number of extra bytes
    windowClass.cbWndExtra = 0; // number of extra bytes following window instance
    windowClass.hInstance = iState->hInstance;
    windowClass.hIcon = NULL; // default Icon
    windowClass.hCursor = NULL; // default Cursor
    windowClass.hbrBackground = NULL;
    // windowClass.hbrBackground = CreateSolidBrush (0x000000ff); // red --> -lgdi32
    // windowClass.hbrBackground = GetSysColorBrush (COLOR_HIGHLIGHT);
    // windowClass.hbrBackground = (HBRUSH) GetSysColorBrush (0x000000FF);
    // windowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    windowClass.lpszMenuName = NULL; // no default menu
    windowClass.lpszClassName = REGISTER_NAME;
    windowClass.hIconSm = NULL; // small Icon; NULL = based on hIcon member

    if (RegisterClassExA (&windowClass) == 0)
    {
        uint64 errorCode = GetLastError ();
        cdk_log_error ("win32 error: %lu", errorCode);
        return CDK_FALSE;
    }
  
    // window style
    unsigned windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE; // window style
    unsigned windowExStyle = WS_EX_APPWINDOW;

    
    // resize window to accomodate for border
    /* 
    * resize window to create client area of desired width and height
    * position does change if you need absolute position of the client area
    * without change you get the absolute position of the window
    */
    RECT borderRect = {0, 0, width, height};
    AdjustWindowRectEx (&borderRect, windowStyle, 0, windowExStyle);

    int32 winWidth = borderRect.right - borderRect.left;
    int32 winHeight = borderRect.bottom - borderRect.top;


    // create window
    iState->hwnd = CreateWindowExA (windowExStyle, // extended window style ???
        REGISTER_NAME, // class name --> tell the function wich registered window to use
        name, // window name
        windowStyle, // window style
        x, y, // x and y position
        winWidth, winHeight, // width and height
        NULL, // hwndParent --> for sub windows
        NULL, // hMenu
        iState->hInstance,
        NULL // lParam --> additional parameters/arguments
        );
    
    if (iState->hwnd)
    {
        uint64 errorCode = GetLastError ();
        cdk_log_error ("win32 error: %lu", errorCode);
        return CDK_FALSE;
    }


    // ShowWindow (hwnd, SW_SHOW); // only necessary if WS_VISIBLE is not set in window styles
    UpdateWindow (iState->hwnd);

    return CDK_TRUE;
}


uint8
cdk_platform_update (PlatformState* pltState)
{
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) // peek message for non blocking msg
        DispatchMessage(&msg); // call callback

    return running;
}


void
cdk_platform_shutdown (PlatformState* pltState)
{   
    InternalState* iState = pltState->iState;
    DestroyWindow (iState->hwnd);
    UnregisterClass (REGISTER_NAME, iState->hInstance);
}


float64
cdk_platform_time (void)
{
    struct timespec time;
    clock_gettime (CLOCK_MONOTONIC, &time);
    return time.tv_sec + time.tv_nsec * 0.000000001;
}


#endif

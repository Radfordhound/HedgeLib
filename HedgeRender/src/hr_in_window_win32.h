/*
   NOTE: This file is included only by hr_window.c, therefore
   it needs no include guard. Please do not include this file
   anywhere else.
*/
#include "../../HedgeLib/src/hl_in_assert.h"
#include "../../HedgeLib/src/hl_in_win32.h"

#include "imgui/imgui.h"
#include <float.h>

#ifdef HL_WIN32_FORCE_ANSI
/* TODO: Is this ok? Does this even work?? */
#undef UNICODE
#endif

#include <windows.h>

extern "C" {
#ifdef HR_IS_DLL
__declspec(dllexport)
#endif
int HrINWin32NCmdShow = SW_SHOWNA;
}

static const LPCTSTR wcName = TEXT("HedgeRender");
static WNDCLASSEX wc;
static HlBool initializing = HL_FALSE, initialized = HL_FALSE;

static LRESULT CALLBACK hrINWin32WndProc(HWND hwnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    HrWindow* hrWindow = (HrWindow*)GetProp(hwnd, wcName);
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] = true;
        hrWindow->mouseDown[HR_INPUT_MOUSE_LEFT] = HL_TRUE;
        goto set_mouse_capture;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Right] = true;
        hrWindow->mouseDown[HR_INPUT_MOUSE_RIGHT] = HL_TRUE;
        goto set_mouse_capture;

    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Middle] = true;
        hrWindow->mouseDown[HR_INPUT_MOUSE_MIDDLE] = HL_TRUE;
        goto set_mouse_capture;

    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
        ImGui::GetIO().MouseDown[(GET_XBUTTON_WPARAM(
            wParam) == XBUTTON1) ? 3 : 4] = true;

    set_mouse_capture:
        if (!ImGui::IsAnyMouseDown() && !GetCapture())
        {
            SetCapture(hwnd);
        }

        return 0;

    case WM_LBUTTONUP:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] = false;
        hrWindow->mouseDown[HR_INPUT_MOUSE_LEFT] = HL_FALSE;
        goto release_mouse_capture;

    case WM_RBUTTONUP:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Right] = false;
        hrWindow->mouseDown[HR_INPUT_MOUSE_RIGHT] = HL_FALSE;
        goto release_mouse_capture;

    case WM_MBUTTONUP:
        ImGui::GetIO().MouseDown[ImGuiMouseButton_Middle] = false;
        hrWindow->mouseDown[HR_INPUT_MOUSE_MIDDLE] = HL_FALSE;
        goto release_mouse_capture;

    case WM_XBUTTONUP:
        ImGui::GetIO().MouseDown[(GET_XBUTTON_WPARAM(
            wParam) == XBUTTON1) ? 3 : 4] = false;

    release_mouse_capture:
        if (!ImGui::IsAnyMouseDown() && GetCapture() != hwnd)
        {
            ReleaseCapture();
        }
        return 0;

    case WM_MOUSEWHEEL:
        ImGui::GetIO().MouseWheel += ((float)GET_WHEEL_DELTA_WPARAM(
            wParam) / (float)WHEEL_DELTA);
        return 0;

    case WM_MOUSEHWHEEL:
        ImGui::GetIO().MouseWheelH += ((float)GET_WHEEL_DELTA_WPARAM(
            wParam) / (float)WHEEL_DELTA);
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < 256)
        {
            ImGui::GetIO().KeysDown[wParam] = true;
            hrWindow->nativeKeysDown[wParam] = HL_TRUE;
        }
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256)
        {
            ImGui::GetIO().KeysDown[wParam] = 0;
            hrWindow->nativeKeysDown[wParam] = HL_FALSE;
        }
        return 0;

    case WM_CHAR:
        /* You can also use ToAscii()+GetKeyboardState() to retrieve characters. */
        if (wParam > 0 && wParam < 0x10000)
        {
            ImGui::GetIO().AddInputCharacterUTF16((ImWchar16)wParam);
        }

        return 0;

    /* TODO */
    /*case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;*/

    case WM_CLOSE:
    {
        hrWindow->shouldClose = HL_TRUE;
        return 0;
    }
    }
    
    /* Handle messages not handled by the switch statement. */
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static HlResult hrINWin32Init(void)
{
    /* If we're already initializing on another thread, block until we're done, then return. */
    if (initializing)
    {
        while (initializing) {}
        return HL_RESULT_SUCCESS;
    }

    /* State that we're now initializing. */
    initializing = HL_TRUE;

    /* Create Window Class. */
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &hrINWin32WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);

    /* TODO: Load user-specified icon. */
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wcName;
    wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    
    /* Register Window Class. */
    if (!RegisterClassEx(&wc)) return HL_ERROR_UNKNOWN; /* TODO: Return actual error. */

    /* State that we're done initializing. */
    initialized = HL_TRUE;
    initializing = HL_FALSE;

    /* Return success. */
    return HL_RESULT_SUCCESS;
}

HlResult hrWindowCreate(const HlNChar* HL_RESTRICT title,
    unsigned int width, unsigned int height, unsigned int x, unsigned int y,
    HrWindow* HL_RESTRICT window)
{
    HlResult result;

    /* Initialize Win32 window class if necessary. */
    if (!initialized)
    {
        result = hrINWin32Init();
        if (HL_FAILED(result)) return result;
    }

    /* Create window. */
    {
        /* Get Client Size Rectangle. */
        RECT clientRect =
        {
            0L, 0L, (LONG)width, (LONG)height
        };

        /* Get style. */
        /* TODO: Fullscreen windows. */
        /* TODO: Borderless windows. */
        /* TODO: Support non-resizable windows. */
        /* NOTE: Both fullscreen windows and borderless windows use WS_POPUP. */
        DWORD style = WS_OVERLAPPEDWINDOW;

        /*DWORD style = (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPED |
           WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);*/

        /* Get extended style. */
        DWORD exStyle = WS_EX_APPWINDOW;

        /* TODO: Windows 10 Anniversary update DPI support. */
        /*
        #if (WINVER >= 0x0605)
            HL_ASSERT(AdjustWindowRectExForDpi(&clientRect, style, FALSE,
                exStyle, USER_DEFAULT_SCREEN_DPI));
        #else
        HL_ASSERT(AdjustWindowRectEx(&clientRect, style, FALSE, exStyle));
        #endif
        */

        /* Create the window and get a handle. */
        HWND handle = CreateWindowEx(
            exStyle, wc.lpszClassName, title, style,
            (x == HR_WINDOW_DEF_POS) ? CW_USEDEFAULT : (int)x,
            (y == HR_WINDOW_DEF_POS) ? CW_USEDEFAULT : (int)y,
            (int)(clientRect.right - clientRect.left),
            (int)(clientRect.bottom - clientRect.top),
            NULL, NULL, wc.hInstance, NULL);

        /* Add HrWindow pointer to native window handle. */
        if (!SetProp(handle, wcName, window))
        {
            DestroyWindow(handle);
            return HL_ERROR_UNKNOWN; /* TODO: Return actual error. */
        }

        /* Setup HrWindow. */
        window->handle = handle;
        window->ticksPerSecond = 0;
        window->time = 0;
        window->shouldClose = HL_FALSE;
        memset(window->nativeKeysDown, 0, sizeof(window->nativeKeysDown));
        memset(window->nativePrevKeysDown, 0, sizeof(window->nativePrevKeysDown));
        memset(window->mouseDown, 0, sizeof(window->mouseDown));
        memset(window->prevMouseDown, 0, sizeof(window->prevMouseDown));
        memset(&window->mousePos, 0, sizeof(window->mousePos));
        memset(&window->prevMousePos, 0, sizeof(window->prevMousePos));

        /* Return success. */
        return HL_RESULT_SUCCESS;
    }
}

void hrWindowImguiSetupIO(HrWindow* HL_RESTRICT window,
    struct ImGuiIO* HL_RESTRICT io)
{
    {
        BOOL r = QueryPerformanceFrequency((LARGE_INTEGER*)&window->ticksPerSecond);
        HL_ASSERT(r);

        r = QueryPerformanceCounter((LARGE_INTEGER*)&window->time);
        HL_ASSERT(r);
    }

    /* Setup back-end capabilities flags. */
    io->BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io->BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io->BackendPlatformName = "Win32";
    io->ImeWindowHandle = window->handle;

    /*
       Keyboard mapping. ImGui will use those indices to peek
       into the io.KeysDown[] array that we will update during
       the application lifetime.
    */
    io->KeyMap[ImGuiKey_Tab] = VK_TAB;
    io->KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io->KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io->KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io->KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io->KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io->KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io->KeyMap[ImGuiKey_Home] = VK_HOME;
    io->KeyMap[ImGuiKey_End] = VK_END;
    io->KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io->KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io->KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io->KeyMap[ImGuiKey_Space] = VK_SPACE;
    io->KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io->KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io->KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io->KeyMap[ImGuiKey_A] = 'A';
    io->KeyMap[ImGuiKey_C] = 'C';
    io->KeyMap[ImGuiKey_V] = 'V';
    io->KeyMap[ImGuiKey_X] = 'X';
    io->KeyMap[ImGuiKey_Y] = 'Y';
    io->KeyMap[ImGuiKey_Z] = 'Z';
}

HlResult hrWindowShow(HrWindow* window)
{
    /* Display the window. */
    /* TODO: Make this thread-safe. */
    ShowWindow((HWND)window->handle, HrINWin32NCmdShow);

    /*
       Set HrINWin32NCmdShow back to default value for subsequent calls.
       (Necessary as HrINWin32NCmdShow is modified by hr_win32_main.h)
    */
    /* TODO: Make this thread-safe. */
    HrINWin32NCmdShow = SW_SHOWNA;

    /* Return success (ShowWindow's return value is not a result code). */
    return HL_RESULT_SUCCESS;
}

static const HlU8 hrINWin32KeyMap[HR_INPUT_KEY_COUNT] =
{
    VK_BACK,
    VK_TAB,
    VK_CLEAR,
    VK_RETURN,
    VK_SHIFT,
    VK_CONTROL,
    VK_MENU,
    VK_PAUSE,
    VK_CAPITAL,
    VK_ESCAPE,
    VK_SPACE,
    VK_PRIOR,
    VK_NEXT,
    VK_END,
    VK_HOME,
    VK_LEFT,
    VK_UP,
    VK_RIGHT,
    VK_DOWN,
    VK_SELECT,
    VK_PRINT,
    VK_EXECUTE,
    VK_SNAPSHOT,
    VK_INSERT,
    VK_DELETE,
    VK_HELP,
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    VK_NUMPAD0,
    VK_NUMPAD1,
    VK_NUMPAD2,
    VK_NUMPAD3,
    VK_NUMPAD4,
    VK_NUMPAD5,
    VK_NUMPAD6,
    VK_NUMPAD7,
    VK_NUMPAD8,
    VK_NUMPAD9,
    VK_MULTIPLY,
    VK_ADD,
    VK_SEPARATOR,
    VK_SUBTRACT,
    VK_DECIMAL,
    VK_DIVIDE,
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
    VK_F13,
    VK_F14,
    VK_F15,
    VK_F16,
    VK_F17,
    VK_F18,
    VK_F19,
    VK_F20,
    VK_F21,
    VK_F22,
    VK_F23,
    VK_F24,
    VK_NUMLOCK,
    VK_SCROLL,
    VK_LSHIFT,
    VK_RSHIFT,
    VK_LCONTROL,
    VK_RCONTROL,
    VK_LMENU,
    VK_RMENU
};

HlBool hrWindowIsKeyPressed(const HrWindow* window, HrInputKey key)
{
    /* If the key is down this frame and was up last frame, it's pressed. */
    const HlU8 nativeIndex = hrINWin32KeyMap[key];
    return (window->nativeKeysDown[nativeIndex] &&
        !window->nativePrevKeysDown[nativeIndex]);
}

HlBool hrWindowIsKeyDown(const HrWindow* window, HrInputKey key)
{
    return window->nativeKeysDown[hrINWin32KeyMap[key]];
}

HlBool hrWindowIsKeyReleased(const HrWindow* window, HrInputKey key)
{
    /* If the key is up this frame and was down last frame, it's released. */
    const HlU8 nativeIndex = hrINWin32KeyMap[key];
    return (!window->nativeKeysDown[nativeIndex] &&
        window->nativePrevKeysDown[nativeIndex]);
}

HlResult hrWindowGetPos(const HrWindow* HL_RESTRICT window,
    HlVector2* HL_RESTRICT pos)
{
    RECT rect;
    if (GetWindowRect((HWND)window->handle, &rect))
    {
        pos->x = (float)rect.left;
        pos->y = (float)rect.top;
        return HL_RESULT_SUCCESS;
    }
    else
    {
        pos->x = 0.0f;
        pos->y = 0.0f;
        return hlINWin32GetResultLastError();
    }
}

HlBool hrWindowContains(const HrWindow* window,
    HlVector2 pos)
{
    RECT rect;
    if (GetClientRect((HWND)window->handle, &rect))
    {
        return ((pos.x > rect.left && pos.x < rect.right) &&
            (pos.y > rect.top && pos.y < rect.bottom));
    }
    else
    {
        return HL_FALSE;
    }
}

HlVector2 hrWindowGetMouseDelta(const HrWindow* window)
{
    const HlVector2 mouseDelta =
    {
        (window->mousePos.x - window->prevMousePos.x),
        (window->mousePos.y - window->prevMousePos.y)
    };

    return mouseDelta;
}

HlResult hrWindowSetMousePos(HrWindow* window, HlVector2 pos)
{
    POINT point = { (LONG)pos.x, (LONG)pos.y };
    if (ClientToScreen((HWND)window->handle, &point) &&
        SetCursorPos((int)point.x, (int)point.y))
    {
        window->mousePos = pos;
        return HL_RESULT_SUCCESS;
    }

    return hlINWin32GetResultLastError();
}

HlResult hrWindowCenterMousePos(HrWindow* window)
{
    RECT rect;
    if (GetClientRect((HWND)window->handle, &rect))
    {
        const HlVector2 pos =
        {
            ((rect.left / 2.0f) + (rect.right / 2.0f)),
            ((rect.top / 2.0f) + (rect.bottom / 2.0f))
        };

        return hrWindowSetMousePos(window, pos);
    }
    else
    {
        return hlINWin32GetResultLastError();
    }
}

void hrWindowImguiNewFrame(HrWindow* window)
{
    /* Get imgui IO. */
    ImGuiIO& io = ImGui::GetIO();

    /* Setup display size (every frame to accommodate for window resizing). */
    {
        RECT rect;
        GetClientRect((HWND)window->handle, &rect);
        io.DisplaySize.x = (float)(rect.right - rect.left);
        io.DisplaySize.y = (float)(rect.bottom - rect.top);
    }

    /* Setup time step. */
    {
        HlS64 curTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
        io.DeltaTime = ((float)(curTime - window->time) / window->ticksPerSecond);
        window->time = curTime;
    }

    /* Read keyboard modifiers inputs. */
    io.KeyCtrl = ((GetKeyState(VK_CONTROL) & 0x8000) != 0);
    io.KeyShift = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
    io.KeyAlt = ((GetKeyState(VK_MENU) & 0x8000) != 0);
    io.KeySuper = false;

    /*
       Set OS mouse position if requested (rarely used, only when
       ImGuiConfigFlags_NavEnableSetMousePos is enabled by user).
    */
    if (io.WantSetMousePos)
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        ClientToScreen((HWND)window->handle, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    /* Set mouse position. */
    io.MousePos.x = window->mousePos.x;
    io.MousePos.y = window->mousePos.y;

    /* Update OS mouse cursor with the cursor requested by imgui. */
    /*{
        ImGuiMouseCursor mouse_cursor = (io->MouseDrawCursor) ?
            ImGuiMouseCursor_None : ImGui::GetMouseCursor();

        if (g_LastMouseCursor != mouse_cursor)
        {
            g_LastMouseCursor = mouse_cursor;
            ImGui_ImplWin32_UpdateMouseCursor();
        }
    }*/

    /* Update game controllers (if enabled and available). */
    /* TODO: ImGui_ImplWin32_UpdateGamepads();*/
}

void hrWindowUpdate(HrWindow* window)
{
    MSG msg;

    /* Set previous key down values. */
    memcpy(window->nativePrevKeysDown, window->nativeKeysDown,
        sizeof(window->nativeKeysDown));

    /* Set previous mouse down values. */
    memcpy(window->prevMouseDown, window->mouseDown,
        sizeof(window->mouseDown));

    /* Set previous mouse position value. */
    window->prevMousePos = window->mousePos;

    /* Get mouse position. */
    {
        POINT pos;
        HWND activeWindow = GetForegroundWindow();

        if ((activeWindow == window->handle || IsChild(activeWindow,
            (HWND)window->handle)) && (GetCursorPos(&pos) && ScreenToClient(
            (HWND)window->handle, &pos)))
        {
            window->mousePos.x = (float)pos.x;
            window->mousePos.y = (float)pos.y;
        }
    }

    /* Check for any window messages in the queue. */
    if (PeekMessage(&msg, (HWND)window->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void hrWindowDestroy(HrWindow* window)
{
    /* Destroy the window. */
    if (window)
    {
        HWND hwnd = (HWND)window->handle;
        RemoveProp(hwnd, wcName);
        DestroyWindow(hwnd);
    }
}

#ifndef HR_WINDOW_H_INCLUDED
#define HR_WINDOW_H_INCLUDED
#include "hr_internal.h"
#include "hedgelib/hl_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Move this to another header? */
typedef enum HrInputKey
{
    HR_INPUT_KEY_BACK,
    HR_INPUT_KEY_TAB,
    HR_INPUT_KEY_CLEAR,
    HR_INPUT_KEY_RETURN,
    HR_INPUT_KEY_SHIFT,
    HR_INPUT_KEY_CONTROL,
    HR_INPUT_KEY_ALT,
    HR_INPUT_KEY_PAUSE,
    HR_INPUT_KEY_CAPSLOCK,
    HR_INPUT_KEY_ESCAPE,
    HR_INPUT_KEY_SPACE,
    HR_INPUT_KEY_PAGEUP,
    HR_INPUT_KEY_PAGEDOWN,
    HR_INPUT_KEY_END,
    HR_INPUT_KEY_HOME,
    HR_INPUT_KEY_LEFT,
    HR_INPUT_KEY_UP,
    HR_INPUT_KEY_RIGHT,
    HR_INPUT_KEY_DOWN,
    HR_INPUT_KEY_SELECT,
    HR_INPUT_KEY_PRINT,
    HR_INPUT_KEY_EXECUTE,
    HR_INPUT_KEY_PRINTSCREEN,
    HR_INPUT_KEY_INSERT,
    HR_INPUT_KEY_DELETE,
    HR_INPUT_KEY_HELP,
    HR_INPUT_KEY_ZERO,
    HR_INPUT_KEY_ONE,
    HR_INPUT_KEY_TWO,
    HR_INPUT_KEY_THREE,
    HR_INPUT_KEY_FOUR,
    HR_INPUT_KEY_FIVE,
    HR_INPUT_KEY_SIX,
    HR_INPUT_KEY_SEVEN,
    HR_INPUT_KEY_EIGHT,
    HR_INPUT_KEY_NINE,
    HR_INPUT_KEY_A,
    HR_INPUT_KEY_B,
    HR_INPUT_KEY_C,
    HR_INPUT_KEY_D,
    HR_INPUT_KEY_E,
    HR_INPUT_KEY_F,
    HR_INPUT_KEY_G,
    HR_INPUT_KEY_H,
    HR_INPUT_KEY_I,
    HR_INPUT_KEY_J,
    HR_INPUT_KEY_K,
    HR_INPUT_KEY_L,
    HR_INPUT_KEY_M,
    HR_INPUT_KEY_N,
    HR_INPUT_KEY_O,
    HR_INPUT_KEY_P,
    HR_INPUT_KEY_Q,
    HR_INPUT_KEY_R,
    HR_INPUT_KEY_S,
    HR_INPUT_KEY_T,
    HR_INPUT_KEY_U,
    HR_INPUT_KEY_V,
    HR_INPUT_KEY_W,
    HR_INPUT_KEY_X,
    HR_INPUT_KEY_Y,
    HR_INPUT_KEY_Z,
    HR_INPUT_KEY_NUMPAD0,
    HR_INPUT_KEY_NUMPAD1,
    HR_INPUT_KEY_NUMPAD2,
    HR_INPUT_KEY_NUMPAD3,
    HR_INPUT_KEY_NUMPAD4,
    HR_INPUT_KEY_NUMPAD5,
    HR_INPUT_KEY_NUMPAD6,
    HR_INPUT_KEY_NUMPAD7,
    HR_INPUT_KEY_NUMPAD8,
    HR_INPUT_KEY_NUMPAD9,
    HR_INPUT_KEY_MULTIPLY,
    HR_INPUT_KEY_ADD,
    HR_INPUT_KEY_SEPARATOR,
    HR_INPUT_KEY_SUBTRACT,
    HR_INPUT_KEY_DECIMAL,
    HR_INPUT_KEY_DIVIDE,
    HR_INPUT_KEY_F1,
    HR_INPUT_KEY_F2,
    HR_INPUT_KEY_F3,
    HR_INPUT_KEY_F4,
    HR_INPUT_KEY_F5,
    HR_INPUT_KEY_F6,
    HR_INPUT_KEY_F7,
    HR_INPUT_KEY_F8,
    HR_INPUT_KEY_F9,
    HR_INPUT_KEY_F10,
    HR_INPUT_KEY_F11,
    HR_INPUT_KEY_F12,
    HR_INPUT_KEY_F13,
    HR_INPUT_KEY_F14,
    HR_INPUT_KEY_F15,
    HR_INPUT_KEY_F16,
    HR_INPUT_KEY_F17,
    HR_INPUT_KEY_F18,
    HR_INPUT_KEY_F19,
    HR_INPUT_KEY_F20,
    HR_INPUT_KEY_F21,
    HR_INPUT_KEY_F22,
    HR_INPUT_KEY_F23,
    HR_INPUT_KEY_F24,
    HR_INPUT_KEY_NUMLOCK,
    HR_INPUT_KEY_SCROLLLOCK,
    HR_INPUT_KEY_LSHIFT,
    HR_INPUT_KEY_RSHIFT,
    HR_INPUT_KEY_LCONTROL,
    HR_INPUT_KEY_RCONTROL,
    HR_INPUT_KEY_LALT,
    HR_INPUT_KEY_RALT,
    HR_INPUT_KEY_COUNT
}
HrInputKey;

/* TODO: Move this to another header? */
typedef enum HrInputMouse
{
    HR_INPUT_MOUSE_LEFT,
    HR_INPUT_MOUSE_RIGHT,
    HR_INPUT_MOUSE_MIDDLE,
    HR_INPUT_MOUSE_COUNT
}
HrInputMouse;

typedef struct HrWindow
{
    void* handle;
    HlS64 ticksPerSecond;
    HlS64 time;
    HlVector2 mousePos;
    HlVector2 prevMousePos;
    HlBool nativeKeysDown[256];
    HlBool nativePrevKeysDown[256];
    HlBool mouseDown[HR_INPUT_MOUSE_COUNT];
    HlBool prevMouseDown[HR_INPUT_MOUSE_COUNT];
    HlBool shouldClose;
}
HrWindow;

#define HR_WINDOW_DEF_POS (unsigned int)(-1)

HR_API HlResult hrWindowCreate(const HlNChar* HL_RESTRICT title,
    unsigned int width, unsigned int height, unsigned int x, unsigned int y,
    HrWindow* HL_RESTRICT window);

HR_API void hrWindowImguiSetupIO(HrWindow* HL_RESTRICT window,
    struct ImGuiIO* HL_RESTRICT io);

HR_API HlResult hrWindowShow(HrWindow* window);

HR_API HlBool hrWindowIsKeyPressed(const HrWindow* window, HrInputKey key);
HR_API HlBool hrWindowIsKeyDown(const HrWindow* window, HrInputKey key);
HR_API HlBool hrWindowIsKeyReleased(const HrWindow* window, HrInputKey key);

HR_API HlResult hrWindowGetPos(const HrWindow* HL_RESTRICT window,
    HlVector2* HL_RESTRICT pos);

HR_API HlBool hrWindowContains(const HrWindow* window,
    HlVector2 pos);

HR_API HlVector2 hrWindowGetMouseDelta(const HrWindow* window);
HR_API HlResult hrWindowSetMousePos(HrWindow* window, HlVector2 pos);
HR_API HlResult hrWindowCenterMousePos(HrWindow* window);

HR_API void hrWindowImguiNewFrame(HrWindow* window);
HR_API void hrWindowUpdate(HrWindow* window);
HR_API void hrWindowDestroy(HrWindow* window);

#ifdef __cplusplus
}
#endif
#endif

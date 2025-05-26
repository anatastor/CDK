
#include "core/input.h"
#include "core/logger.h"

#include <string.h>


typedef struct CDKKeyboardState
{
    uint8 keys[CDK_MAX_KEYS];
} CDKKeyboardState;


typedef struct CDKMouseState
{
    uint32 x;
    uint32 y;
    uint8 buttons[CDK_MOUSEBUTTON_MAX];
} CDKMouseState;


typedef struct CDKInputState
{
    CDKKeyboardState keyboard;
    CDKKeyboardState keyboardPrev;

    CDKMouseState mouse;
    CDKMouseState mousePrev;
} CDKInputState;


static uint8 _initialized = CDK_FALSE;
static CDKInputState state = {};


void
input_init (void)
{
    if (!_initialized)
        return;
    
    memset (&state, 0, sizeof (state));
    _initialized = CDK_TRUE;
}


void
input_update (float64 delta)
{
    if (!_initialized)
        return;

    memcpy (&state.keyboard, &state.keyboardPrev, sizeof (CDKKeyboardState));
    memcpy (&state.mouse, &state.mousePrev, sizeof (CDKMouseState));
}


void
input_process_keyboard (CDKKeyCodes key, uint8 pressed)
{
    if (state.keyboard.keys[key] != pressed)
    {
        state.keyboard.keys[key] = pressed;
        cdk_log_debug ("key %c 0x%x %s", key, key, pressed ? "pres" : "rel");
    }
}


void
input_process_mouse_move (uint32 x, uint32 y)
{
    if (state.mouse.x == x && state.mouse.y == y)
        return;
    
    state.mouse.x = x;
    state.mouse.y = y;
}


void
input_process_mouse_button (CDKMouseButtons button, uint8 pressed)
{
    if (state.mouse.buttons[button] != pressed)
    {
        state.mouse.buttons[button] = pressed;
        cdk_log_debug ("Mouse Button %i %s", button, pressed ? "pressed" : "released");
    }
}


void
input_get_mouse_pos (uint32* x, uint32* y)
{
    *x = state.mouse.x;
    *y = state.mouse.y;
}



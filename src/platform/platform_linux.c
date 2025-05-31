
#include "platform/platform.h"
#include "core/input.h"


#if defined(CDK_PLATFORM_LINUX)

#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h> // for receiving keycodes from input
#include <X11/keysym.h> // for mapping keycodes; list in /usr/include/X11/keysymdef.h


typedef struct InternalState
{
    Display* display;
    Window window;
    Atom wmDeleteMsg;
} InternalState;


CDKKeyCodes _translate_keys (uint32 key);



uint8
cdk_platform_create (PlatformState* pltState,
        const char* name,
        uint32 x, uint32 y,
        uint32 width, uint32 height)
{

    pltState->iState = malloc (sizeof (InternalState));
    InternalState* iState = pltState->iState;

    iState->display = XOpenDisplay (NULL); 
    iState->window = XCreateSimpleWindow (iState->display,
            DefaultRootWindow (iState->display),
            x, y, width, height, 0, 0, 0);
            //0, 0, 200, 100, 0, colorBlack, colorWhite);

    XStoreName (iState->display, iState->window, 
            name); // set the window title
                   //
    XSelectInput (iState->display, iState->window,
            KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow (iState->display, iState->window);

    XkbSetDetectableAutoRepeat (iState->display, 1, NULL); // works, but still sends an KeyPressEvent every time
    
    // override window manager settings and send a "WM_DELETE_WINDOW" ClientMessage instead of doing a KillClient() by the wm
    iState->wmDeleteMsg = XInternAtom (iState->display, 
            "WM_DELETE_WINDOW", False);
    XSetWMProtocols (iState->display,
            iState->window, &iState->wmDeleteMsg, 1);


    return CDK_TRUE;
}


uint8
cdk_platform_update (PlatformState* pltState)
{   
    InternalState* iState = pltState->iState;
    XEvent event;
    
    while (XPending (iState->display))
    {
        XNextEvent (iState->display, &event);

        if (event.type == ClientMessage)
        {   
            if (event.xclient.data.l[0] == iState->wmDeleteMsg)
            {
                cdk_log_debug ("closing window detected");
                return CDK_FALSE; 
            }
        }

        if (event.type == KeyPress || event.type == KeyRelease)
        {   
            int type = event.xkey.type;
            unsigned keycode = event.xkey.keycode;
            KeySym keysym = XkbKeycodeToKeysym (iState->display, keycode, 0, event.xkey.state);
            CDKKeyCodes key = _translate_keys (keysym);

            input_process_keyboard (key, event.xkey.type == 2 ? 1 : 0);
        }

        if (event.type == MotionNotify) // mouse move
        {
            uint32 x = event.xmotion.x;
            uint32 y = event.xmotion.y;
            input_process_mouse_move (x, y);
        }

        if (event.type == ButtonPress || event.type == ButtonRelease) // mouse click
        {   
            /*
             * can retreive pointer position by button click event
            printf ("Button pressed: %i/%i\n", x, y);
            */
            int button = event.xbutton.button;
            uint8 pressed = event.xbutton.type == ButtonPress ? 1 : 0;

            switch (button)
            {
                case 1: // left
                    input_process_mouse_button (CDK_MOUSEBUTTON_LEFT, pressed);
                    break;

                case 2: // right
                    input_process_mouse_button (CDK_MOUSEBUTTON_LEFT, pressed);
                    break;

                case 3: // middle
                    input_process_mouse_button (CDK_MOUSEBUTTON_MIDDLE, pressed);
                    break;

                case 4: // wheel up
                    break;

                case 5: // wheel down
                    break;
            }

        }
    }

    return CDK_TRUE;
}


void
cdk_platform_shutdown (PlatformState* pltState)
{   
    InternalState* iState = pltState->iState;
    XCloseDisplay (iState->display);
    free (pltState->iState);
}


float64
cdk_platform_time (void)
{
    struct timespec time;
    clock_gettime (CLOCK_MONOTONIC, &time);
    return time.tv_sec + time.tv_nsec * 0.000000001;
}



CDKKeyCodes
_translate_keys (uint32 xKey)
{   
    // TODO SYSKEYS
    switch (xKey)
    {
        case XK_Left: return CDK_KEY_ARROW_LEFT;
        case XK_Up: return CDK_KEY_ARROW_UP;
        case XK_Right: return CDK_KEY_ARROW_RIGHT;
        case XK_Down: return CDK_KEY_ARROW_DOWN;
    
        case XK_BackSpace: return CDK_KEY_BACKSPACE;
        case XK_Tab: return CDK_KEY_TAB;
        case XK_Return: return CDK_KEY_RETURN;
        case XK_Shift_L: case XK_Shift_R: return CDK_KEY_SHIFT;
        case XK_Escape: return CDK_KEY_ESC;
        case XK_Delete: return CDK_KEY_DELETE;
    }

    if (xKey >= 0x0061 && xKey <= 0x007a) return xKey ^ 0x20; // lower a - z 
    if (xKey >= 0x0020 && xKey <= 0x007e) return xKey; // A - Z, 0 - 9

    return 0;
}


#endif

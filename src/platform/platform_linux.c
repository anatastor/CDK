
#include "platform/platform.h"


#if defined(CDK_PLATFORM_LINUX)

#include <stdlib.h>
#include <X11/Xlib.h>


typedef struct InternalState
{
    Display* display;
    Window window;
    Atom wmDeleteMsg;
} InternalState;



uint8
cdk_window_create (PlatformState* pltState,
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
    XMapWindow (iState->display, iState->window);
    
    // override window manager settings and send a "WM_DELETE_WINDOW" ClientMessage instead of doing a KillClient() by the wm
    iState->wmDeleteMsg = XInternAtom (iState->display, 
            "WM_DELETE_WINDOW", False);
    XSetWMProtocols (iState->display,
            iState->window, &iState->wmDeleteMsg, 1);


    return CDK_TRUE;
}


uint8
cdk_window_update (PlatformState* pltState)
{   
    InternalState* iState = pltState->iState;
    XEvent event;
    
    while (XPending (iState->display))
    {
        cdk_log_debug ("event!!!");
        XNextEvent (iState->display, &event);

        if (event.type == ClientMessage)
        {   
            if (event.xclient.data.l[0] == iState->wmDeleteMsg)
            {
                cdk_log_debug ("closing window detected");
                return CDK_FALSE; 
            }
        }
    }


    return CDK_TRUE;
}


void
cdk_shutdown (PlatformState* pltState)
{   
    InternalState* iState = pltState->iState;
    XCloseDisplay (iState->display);
    free (pltState->iState);
}


#endif

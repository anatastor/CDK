
#pragma once

#include "def.h"
#include "core/inputkeys.h"


typedef enum CDKMouseButtons
{
    CDK_MOUSEBUTTON_LEFT,
    CDK_MOUSEBUTTON_RIGHT,
    CDK_MOUSEBUTTON_MIDDLE,

    CDK_MOUSEBUTTON_MAX
} CDKMouseButtons;



void input_init (void);
void input_shutdown (void);

void input_update (float64 delta);


// keyboard
void input_process_keyboard (CDKKeyCodes key, uint8 pressed);


// mouse 
void input_process_mouse_move (uint32 x, uint32 y);
void input_process_mouse_button (CDKMouseButtons button, uint8 pressed);

void input_get_mouse_pos (uint32* x, uint32* y);


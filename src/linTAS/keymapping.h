#ifndef KEYMAPPING_H_INCLUDED
#define KEYMAPPING_H_INCLUDED

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/inputs.h"

enum 
{
    HOTKEY_PLAYPAUSE, // Switch from play to pause of the game
    HOTKEY_FRAMEADVANCE, // Advance one frame, also pause the game if playing
    HOTKEY_FASTFORWARD, // Enable fastforward when pressed
    HOTKEY_READWRITE, // Switch from read-only recording to write
    HOTKEY_SAVESTATE, // Save the entire state of the game
    HOTKEY_LOADSTATE, // Load the entire state of the game
    HOTKEY_LEN
};

void default_hotkeys(KeySym *hotkeys);
void format_keyboard(struct AllInputs* ai, Display *display, char keyboard_state[], KeySym hotkeys[]);

#endif // KEYMAPPING_H_INCLUDED

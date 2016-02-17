#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include "SDL.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

void X11_InitKeymap(void);
void xkeyboardToSDLkeyboard(Display *display, char Xkeyboard[], Uint8* SDLkeyboard);
void xkeycodeToSDL(Display *display, SDL_Keysym *keysym, char xkeycode);

#endif // KEYBOARD_H_INCLUDED

#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include "../external/SDL.h"
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

SDL_Keycode X11_TranslateKeysym(KeySym kc);
SDL_Scancode GetScanFromKey(SDL_Keycode);
void X11_InitKeymap(void);
void xkeyboardToSDLkeyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard);
void xkeysymToSDL(SDL_Keysym *keysym, KeySym xkeysym);

#endif // KEYBOARD_H_INCLUDED

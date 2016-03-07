#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include "../external/SDL.h"
#include <X11/keysym.h>

SDL_Keycode X11_TranslateKeysym(KeySym kc);
SDLKey X11_Translate1Keysym(KeySym xsym);

SDL_Scancode GetScanFromKey(SDL_Keycode);
SDL_Scancode GetScanFromKey1(SDLKey key);

void X11_InitKeymap(void);
void xkeyboardToSDLkeyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard);
void xkeyboardToSDL1keyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard);

void xkeysymToSDL(SDL_Keysym *keysym, KeySym xkeysym);
void xkeysymToSDL1(SDL_keysym *keysym, KeySym xkeysym);

#endif // KEYBOARD_H_INCLUDED

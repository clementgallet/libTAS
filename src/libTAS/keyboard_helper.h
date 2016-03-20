/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include "../external/SDL.h"
#include <X11/keysym.h>

SDL_Keycode X11_TranslateKeysym(KeySym kc);
SDL1::SDLKey X11_Translate1Keysym(KeySym xsym);

SDL_Scancode GetScanFromKey(SDL_Keycode);
SDL_Scancode GetScanFromKey1(SDL1::SDLKey key);

void X11_InitKeymap(void);
void xkeyboardToSDLkeyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard);
void xkeyboardToSDL1keyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard);

void xkeysymToSDL(SDL_Keysym *keysym, KeySym xkeysym);
void xkeysymToSDL1(SDL1::SDL_keysym *keysym, KeySym xkeysym);

#endif // KEYBOARD_H_INCLUDED

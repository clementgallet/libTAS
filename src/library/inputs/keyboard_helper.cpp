/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "keyboard_helper.h"
#include "sdlkeyboardlayout.h"

#include "../external/keysymdef.h"

#include <string.h>
#include <array>

namespace libtas {

/* The translation tables from an X11 keysym to a SDL keysym */
static SDL_Keycode MISC_keymap[256];
static SDL1::SDLKey MISC1_keymap[256];

void X11_InitKeymap(void)
{
	int i;

	/* Map the miscellaneous keys */
	for ( i=0; i<256; ++i )
		MISC_keymap[i] = SDLK_UNKNOWN;

	/* These X keysyms have 0xFF as the high byte */
	MISC_keymap[XK_BackSpace&0xFF] = SDLK_BACKSPACE;
	MISC_keymap[XK_Tab&0xFF] = SDLK_TAB;
	MISC_keymap[XK_Clear&0xFF] = SDLK_CLEAR;
	MISC_keymap[XK_Return&0xFF] = SDLK_RETURN;
	MISC_keymap[XK_Pause&0xFF] = SDLK_PAUSE;
	MISC_keymap[XK_Escape&0xFF] = SDLK_ESCAPE;
	MISC_keymap[XK_Delete&0xFF] = SDLK_DELETE;

	MISC_keymap[XK_KP_0&0xFF] = SDLK_KP_0;		/* Keypad 0-9 */
	MISC_keymap[XK_KP_1&0xFF] = SDLK_KP_1;
	MISC_keymap[XK_KP_2&0xFF] = SDLK_KP_2;
	MISC_keymap[XK_KP_3&0xFF] = SDLK_KP_3;
	MISC_keymap[XK_KP_4&0xFF] = SDLK_KP_4;
	MISC_keymap[XK_KP_5&0xFF] = SDLK_KP_5;
	MISC_keymap[XK_KP_6&0xFF] = SDLK_KP_6;
	MISC_keymap[XK_KP_7&0xFF] = SDLK_KP_7;
	MISC_keymap[XK_KP_8&0xFF] = SDLK_KP_8;
	MISC_keymap[XK_KP_9&0xFF] = SDLK_KP_9;
	MISC_keymap[XK_KP_Insert&0xFF] = SDLK_KP_0;
	MISC_keymap[XK_KP_End&0xFF] = SDLK_KP_1;
	MISC_keymap[XK_KP_Down&0xFF] = SDLK_KP_2;
	MISC_keymap[XK_KP_Page_Down&0xFF] = SDLK_KP_3;
	MISC_keymap[XK_KP_Left&0xFF] = SDLK_KP_4;
	MISC_keymap[XK_KP_Begin&0xFF] = SDLK_KP_5;
	MISC_keymap[XK_KP_Right&0xFF] = SDLK_KP_6;
	MISC_keymap[XK_KP_Home&0xFF] = SDLK_KP_7;
	MISC_keymap[XK_KP_Up&0xFF] = SDLK_KP_8;
	MISC_keymap[XK_KP_Page_Up&0xFF] = SDLK_KP_9;
	MISC_keymap[XK_KP_Delete&0xFF] = SDLK_KP_PERIOD;
	MISC_keymap[XK_KP_Decimal&0xFF] = SDLK_KP_PERIOD;
	MISC_keymap[XK_KP_Divide&0xFF] = SDLK_KP_DIVIDE;
	MISC_keymap[XK_KP_Multiply&0xFF] = SDLK_KP_MULTIPLY;
	MISC_keymap[XK_KP_Subtract&0xFF] = SDLK_KP_MINUS;
	MISC_keymap[XK_KP_Add&0xFF] = SDLK_KP_PLUS;
	MISC_keymap[XK_KP_Enter&0xFF] = SDLK_KP_ENTER;
	MISC_keymap[XK_KP_Equal&0xFF] = SDLK_KP_EQUALS;

	MISC_keymap[XK_Up&0xFF] = SDLK_UP;
	MISC_keymap[XK_Down&0xFF] = SDLK_DOWN;
	MISC_keymap[XK_Right&0xFF] = SDLK_RIGHT;
	MISC_keymap[XK_Left&0xFF] = SDLK_LEFT;
	MISC_keymap[XK_Insert&0xFF] = SDLK_INSERT;
	MISC_keymap[XK_Home&0xFF] = SDLK_HOME;
	MISC_keymap[XK_End&0xFF] = SDLK_END;
	MISC_keymap[XK_Page_Up&0xFF] = SDLK_PAGEUP;
	MISC_keymap[XK_Page_Down&0xFF] = SDLK_PAGEDOWN;

	MISC_keymap[XK_F1&0xFF] = SDLK_F1;
	MISC_keymap[XK_F2&0xFF] = SDLK_F2;
	MISC_keymap[XK_F3&0xFF] = SDLK_F3;
	MISC_keymap[XK_F4&0xFF] = SDLK_F4;
	MISC_keymap[XK_F5&0xFF] = SDLK_F5;
	MISC_keymap[XK_F6&0xFF] = SDLK_F6;
	MISC_keymap[XK_F7&0xFF] = SDLK_F7;
	MISC_keymap[XK_F8&0xFF] = SDLK_F8;
	MISC_keymap[XK_F9&0xFF] = SDLK_F9;
	MISC_keymap[XK_F10&0xFF] = SDLK_F10;
	MISC_keymap[XK_F11&0xFF] = SDLK_F11;
	MISC_keymap[XK_F12&0xFF] = SDLK_F12;
	MISC_keymap[XK_F13&0xFF] = SDLK_F13;
	MISC_keymap[XK_F14&0xFF] = SDLK_F14;
	MISC_keymap[XK_F15&0xFF] = SDLK_F15;

	MISC_keymap[XK_Num_Lock&0xFF] = SDLK_NUMLOCKCLEAR;
	MISC_keymap[XK_Caps_Lock&0xFF] = SDLK_CAPSLOCK;
	MISC_keymap[XK_Scroll_Lock&0xFF] = SDLK_SCROLLLOCK;
	MISC_keymap[XK_Shift_R&0xFF] = SDLK_RSHIFT;
	MISC_keymap[XK_Shift_L&0xFF] = SDLK_LSHIFT;
	MISC_keymap[XK_Control_R&0xFF] = SDLK_RCTRL;
	MISC_keymap[XK_Control_L&0xFF] = SDLK_LCTRL;
	MISC_keymap[XK_Alt_R&0xFF] = SDLK_RALT;
	MISC_keymap[XK_Alt_L&0xFF] = SDLK_LALT;
	MISC_keymap[XK_Meta_R&0xFF] = SDLK_RGUI;
	MISC_keymap[XK_Meta_L&0xFF] = SDLK_LGUI;
	MISC_keymap[XK_Super_L&0xFF] = SDLK_LGUI; /* Left "Windows" */
	MISC_keymap[XK_Super_R&0xFF] = SDLK_RGUI; /* Right "Windows */
	MISC_keymap[XK_Mode_switch&0xFF] = SDLK_MODE; /* "Alt Gr" key */
	//MISC_keymap[XK_Multi_key&0xFF] = SDLK_COMPOSE; /* Multi-key compose */

	MISC_keymap[XK_Help&0xFF] = SDLK_HELP;
	MISC_keymap[XK_Print&0xFF] = SDLK_PRINTSCREEN;
	MISC_keymap[XK_Sys_Req&0xFF] = SDLK_SYSREQ;
	MISC_keymap[XK_Break&0xFF] = SDLK_PAUSE;
	MISC_keymap[XK_Menu&0xFF] = SDLK_MENU;
	MISC_keymap[XK_Hyper_R&0xFF] = SDLK_MENU;   /* Windows "Menu" key */

    /* SDL 1.2 */

	/* Map the miscellaneous keys */
	for ( i=0; i<256; ++i )
		MISC1_keymap[i] = SDL1::SDLK_UNKNOWN;

	/* These X keysyms have 0xFF as the high byte */
	MISC1_keymap[XK_BackSpace&0xFF] = SDL1::SDLK_BACKSPACE;
	MISC1_keymap[XK_Tab&0xFF] = SDL1::SDLK_TAB;
	MISC1_keymap[XK_Clear&0xFF] = SDL1::SDLK_CLEAR;
	MISC1_keymap[XK_Return&0xFF] = SDL1::SDLK_RETURN;
	MISC1_keymap[XK_Pause&0xFF] = SDL1::SDLK_PAUSE;
	MISC1_keymap[XK_Escape&0xFF] = SDL1::SDLK_ESCAPE;
	MISC1_keymap[XK_Delete&0xFF] = SDL1::SDLK_DELETE;

	MISC1_keymap[XK_KP_0&0xFF] = SDL1::SDLK_KP0;		/* Keypad 0-9 */
	MISC1_keymap[XK_KP_1&0xFF] = SDL1::SDLK_KP1;
	MISC1_keymap[XK_KP_2&0xFF] = SDL1::SDLK_KP2;
	MISC1_keymap[XK_KP_3&0xFF] = SDL1::SDLK_KP3;
	MISC1_keymap[XK_KP_4&0xFF] = SDL1::SDLK_KP4;
	MISC1_keymap[XK_KP_5&0xFF] = SDL1::SDLK_KP5;
	MISC1_keymap[XK_KP_6&0xFF] = SDL1::SDLK_KP6;
	MISC1_keymap[XK_KP_7&0xFF] = SDL1::SDLK_KP7;
	MISC1_keymap[XK_KP_8&0xFF] = SDL1::SDLK_KP8;
	MISC1_keymap[XK_KP_9&0xFF] = SDL1::SDLK_KP9;
	MISC1_keymap[XK_KP_Insert&0xFF] = SDL1::SDLK_KP0;
	MISC1_keymap[XK_KP_End&0xFF] = SDL1::SDLK_KP1;
	MISC1_keymap[XK_KP_Down&0xFF] = SDL1::SDLK_KP2;
	MISC1_keymap[XK_KP_Page_Down&0xFF] = SDL1::SDLK_KP3;
	MISC1_keymap[XK_KP_Left&0xFF] = SDL1::SDLK_KP4;
	MISC1_keymap[XK_KP_Begin&0xFF] = SDL1::SDLK_KP5;
	MISC1_keymap[XK_KP_Right&0xFF] = SDL1::SDLK_KP6;
	MISC1_keymap[XK_KP_Home&0xFF] = SDL1::SDLK_KP7;
	MISC1_keymap[XK_KP_Up&0xFF] = SDL1::SDLK_KP8;
	MISC1_keymap[XK_KP_Page_Up&0xFF] = SDL1::SDLK_KP9;
	MISC1_keymap[XK_KP_Delete&0xFF] = SDL1::SDLK_KP_PERIOD;
	MISC1_keymap[XK_KP_Decimal&0xFF] = SDL1::SDLK_KP_PERIOD;
	MISC1_keymap[XK_KP_Divide&0xFF] = SDL1::SDLK_KP_DIVIDE;
	MISC1_keymap[XK_KP_Multiply&0xFF] = SDL1::SDLK_KP_MULTIPLY;
	MISC1_keymap[XK_KP_Subtract&0xFF] = SDL1::SDLK_KP_MINUS;
	MISC1_keymap[XK_KP_Add&0xFF] = SDL1::SDLK_KP_PLUS;
	MISC1_keymap[XK_KP_Enter&0xFF] = SDL1::SDLK_KP_ENTER;
	MISC1_keymap[XK_KP_Equal&0xFF] = SDL1::SDLK_KP_EQUALS;

	MISC1_keymap[XK_Up&0xFF] = SDL1::SDLK_UP;
	MISC1_keymap[XK_Down&0xFF] = SDL1::SDLK_DOWN;
	MISC1_keymap[XK_Right&0xFF] = SDL1::SDLK_RIGHT;
	MISC1_keymap[XK_Left&0xFF] = SDL1::SDLK_LEFT;
	MISC1_keymap[XK_Insert&0xFF] = SDL1::SDLK_INSERT;
	MISC1_keymap[XK_Home&0xFF] = SDL1::SDLK_HOME;
	MISC1_keymap[XK_End&0xFF] = SDL1::SDLK_END;
	MISC1_keymap[XK_Page_Up&0xFF] = SDL1::SDLK_PAGEUP;
	MISC1_keymap[XK_Page_Down&0xFF] = SDL1::SDLK_PAGEDOWN;

	MISC1_keymap[XK_F1&0xFF] = SDL1::SDLK_F1;
	MISC1_keymap[XK_F2&0xFF] = SDL1::SDLK_F2;
	MISC1_keymap[XK_F3&0xFF] = SDL1::SDLK_F3;
	MISC1_keymap[XK_F4&0xFF] = SDL1::SDLK_F4;
	MISC1_keymap[XK_F5&0xFF] = SDL1::SDLK_F5;
	MISC1_keymap[XK_F6&0xFF] = SDL1::SDLK_F6;
	MISC1_keymap[XK_F7&0xFF] = SDL1::SDLK_F7;
	MISC1_keymap[XK_F8&0xFF] = SDL1::SDLK_F8;
	MISC1_keymap[XK_F9&0xFF] = SDL1::SDLK_F9;
	MISC1_keymap[XK_F10&0xFF] = SDL1::SDLK_F10;
	MISC1_keymap[XK_F11&0xFF] = SDL1::SDLK_F11;
	MISC1_keymap[XK_F12&0xFF] = SDL1::SDLK_F12;
	MISC1_keymap[XK_F13&0xFF] = SDL1::SDLK_F13;
	MISC1_keymap[XK_F14&0xFF] = SDL1::SDLK_F14;
	MISC1_keymap[XK_F15&0xFF] = SDL1::SDLK_F15;

	MISC1_keymap[XK_Num_Lock&0xFF] = SDL1::SDLK_NUMLOCK;
	MISC1_keymap[XK_Caps_Lock&0xFF] = SDL1::SDLK_CAPSLOCK;
	MISC1_keymap[XK_Scroll_Lock&0xFF] = SDL1::SDLK_SCROLLOCK;
	MISC1_keymap[XK_Shift_R&0xFF] = SDL1::SDLK_RSHIFT;
	MISC1_keymap[XK_Shift_L&0xFF] = SDL1::SDLK_LSHIFT;
	MISC1_keymap[XK_Control_R&0xFF] = SDL1::SDLK_RCTRL;
	MISC1_keymap[XK_Control_L&0xFF] = SDL1::SDLK_LCTRL;
	MISC1_keymap[XK_Alt_R&0xFF] = SDL1::SDLK_RALT;
	MISC1_keymap[XK_Alt_L&0xFF] = SDL1::SDLK_LALT;
	MISC1_keymap[XK_Meta_R&0xFF] = SDL1::SDLK_RMETA;
	MISC1_keymap[XK_Meta_L&0xFF] = SDL1::SDLK_LMETA;
	MISC1_keymap[XK_Super_L&0xFF] = SDL1::SDLK_LSUPER; /* Left "Windows" */
	MISC1_keymap[XK_Super_R&0xFF] = SDL1::SDLK_RSUPER; /* Right "Windows */
	MISC1_keymap[XK_Mode_switch&0xFF] = SDL1::SDLK_MODE; /* "Alt Gr" key */
	MISC1_keymap[XK_Multi_key&0xFF] = SDL1::SDLK_COMPOSE; /* Multi-key compose */

	MISC1_keymap[XK_Help&0xFF] = SDL1::SDLK_HELP;
	MISC1_keymap[XK_Print&0xFF] = SDL1::SDLK_PRINT;
	MISC1_keymap[XK_Sys_Req&0xFF] = SDL1::SDLK_SYSREQ;
	MISC1_keymap[XK_Break&0xFF] = SDL1::SDLK_BREAK;
	MISC1_keymap[XK_Menu&0xFF] = SDL1::SDLK_MENU;
	MISC1_keymap[XK_Hyper_R&0xFF] = SDL1::SDLK_MENU;   /* Windows "Menu" key */
}

/* Get the translated SDL virtual keysym */
SDL_Keycode X11_TranslateKeysym(unsigned int xsym)
{
    static int keymap_inited = 0;
    if (! keymap_inited) {
        X11_InitKeymap();
        keymap_inited = 1;
    }

	SDL_Keycode key;

	key = SDLK_UNKNOWN;
	if ( xsym ) {
		switch (xsym>>8) {
		    case 0x00:	/* Latin 1 */
			    key = static_cast<SDL_Keycode>(xsym & 0xFF);
			    break;
		    case 0x01:	/* Latin 2 */
		    case 0x02:	/* Latin 3 */
		    case 0x03:	/* Latin 4 */
		    case 0x04:	/* Katakana */
		    case 0x05:	/* Arabic */
		    case 0x06:	/* Cyrillic */
		    case 0x07:	/* Greek */
		    case 0x08:	/* Technical */
		    case 0x0A:	/* Publishing */
		    case 0x0C:	/* Hebrew */
		    case 0x0D:	/* Thai */
			    /* These are wrong, but it's better than nothing */
			    key = static_cast<SDL_Keycode>(xsym & 0xFF);
			    break;
		    case 0xFE:
                /* Odd keys used in international keyboards */
			    break;
		    case 0xFF:
			    key = MISC_keymap[xsym&0xFF];
			    break;
		    default:
			    break;
		}
	}
	return key;
}

/* Get the translated SDL 1.2 virtual keysym */
SDL1::SDLKey X11_Translate1Keysym(unsigned int xsym)
{
    static int keymap_inited = 0;
    if (! keymap_inited) {
        X11_InitKeymap();
        keymap_inited = 1;
    }

	SDL1::SDLKey key;

	key = SDL1::SDLK_UNKNOWN;
	if ( xsym ) {
		switch (xsym>>8) {
		    case 0x00:	/* Latin 1 */
			key = static_cast<SDL1::SDLKey>(xsym & 0xFF);
			break;
		    case 0x01:	/* Latin 2 */
		    case 0x02:	/* Latin 3 */
		    case 0x03:	/* Latin 4 */
		    case 0x04:	/* Katakana */
		    case 0x05:	/* Arabic */
		    case 0x06:	/* Cyrillic */
		    case 0x07:	/* Greek */
		    case 0x08:	/* Technical */
		    case 0x0A:	/* Publishing */
		    case 0x0C:	/* Hebrew */
		    case 0x0D:	/* Thai */
			/* These are wrong, but it's better than nothing */
			key = static_cast<SDL1::SDLKey>(xsym & 0xFF);
			break;
		    case 0xFE:
			break;
		    case 0xFF:
			key = MISC1_keymap[xsym&0xFF];
			break;
		    default:
			break;
		}
	}
	return key;
}

void xkeyboardToSDLkeyboard(const std::array<unsigned int,AllInputsFlat::MAXKEYS>& Xkeyboard, Uint8* SDLkeyboard) {
    memset(SDLkeyboard, 0, SDL_NUM_SCANCODES);
    for (int i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (Xkeyboard[i]) {
            SDL_Scancode sc = GetScanFromKey(X11_TranslateKeysym(Xkeyboard[i]));
            SDLkeyboard[sc] = 1;
        }
    }
}

void xkeyboardToSDL1keyboard(const std::array<unsigned int,AllInputsFlat::MAXKEYS>& Xkeyboard, Uint8* SDLkeyboard) {
    memset(SDLkeyboard, 0, SDL1::SDLK_LAST);
    for (int i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (Xkeyboard[i]) {
            SDL1::SDLKey key = X11_Translate1Keysym(Xkeyboard[i]);
            SDLkeyboard[key] = 1;
        }
    }
}

void xkeysymToSDL(SDL_Keysym *keysym, unsigned int xkeysym) {
    keysym->sym = X11_TranslateKeysym(xkeysym);
    keysym->scancode = GetScanFromKey(keysym->sym);
    keysym->unused = 0;
}

void xkeysymToSDL1(SDL1::SDL_keysym *keysym, unsigned int xkeysym) {
    keysym->sym = X11_Translate1Keysym(xkeysym);
    keysym->scancode = GetScanFromKey1(keysym->sym);
    keysym->unicode = 0;
}

struct ModTranslate {
    unsigned int keysym;
    unsigned int xmod;
    SDL_Keymod sdlmod;
};

static std::array<ModTranslate, 10> mod_translate {{
    {XK_Shift_L, 1<<0 /* ShiftMask */, KMOD_LSHIFT},
    {XK_Shift_R, 1<<0 /* ShiftMask */, KMOD_RSHIFT},
    {XK_Control_L, 1<<2 /* ControlMask */, KMOD_LCTRL},
    {XK_Control_R, 1<<2 /* ControlMask */, KMOD_RCTRL},
    {XK_Meta_L, 1<<6 /* Mod4Mask */, KMOD_LGUI},
    {XK_Meta_R, 1<<6 /* Mod4Mask */, KMOD_RGUI},
    {XK_Alt_L, 1<<3 /* Mod1Mask */, KMOD_LALT},
    {XK_Alt_R, 1<<3 /* Mod1Mask */, KMOD_RALT},
    {XK_Caps_Lock, 1<<1 /* LockMask */, KMOD_CAPS},
    {XK_Shift_Lock, 1<<4 /* Mod2Mask */, KMOD_NUM},
}};

unsigned int xkeyboardToXMod(const std::array<unsigned int,AllInputsFlat::MAXKEYS>& Xkeyboard) {
    unsigned int mod = 0;
    for (int i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (Xkeyboard[i]) {
            for (int j=0; j<10; j++) {
                if (Xkeyboard[i] == mod_translate[j].keysym) {
                    mod |= mod_translate[j].xmod;
                    break;
                }
            }
        }
    }

    return mod;
}

SDL_Keymod xkeyboardToSDLMod(const std::array<unsigned int,AllInputsFlat::MAXKEYS>& Xkeyboard) {
    unsigned int mod = KMOD_NONE; // use int because "bitwise or" promotes to int
    for (int i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (Xkeyboard[i]) {
            for (int j=0; j<10; j++) {
                if (Xkeyboard[i] == mod_translate[j].keysym) {
                    mod |= mod_translate[j].sdlmod;
                    break;
                }
            }
        }
    }

    return static_cast<SDL_Keymod>(mod);
}

}

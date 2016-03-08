#include "keyboard_helper.h"
#include <string.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include "stdio.h"
/* The translation tables from an X11 keysym to a SDL keysym */
static SDL_Keycode MISC_keymap[256];
static SDLKey MISC1_keymap[256];

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
		MISC1_keymap[i] = SDLK1_UNKNOWN;

	/* These X keysyms have 0xFF as the high byte */
	MISC1_keymap[XK_BackSpace&0xFF] = SDLK1_BACKSPACE;
	MISC1_keymap[XK_Tab&0xFF] = SDLK1_TAB;
	MISC1_keymap[XK_Clear&0xFF] = SDLK1_CLEAR;
	MISC1_keymap[XK_Return&0xFF] = SDLK1_RETURN;
	MISC1_keymap[XK_Pause&0xFF] = SDLK1_PAUSE;
	MISC1_keymap[XK_Escape&0xFF] = SDLK1_ESCAPE;
	MISC1_keymap[XK_Delete&0xFF] = SDLK1_DELETE;

	MISC1_keymap[XK_KP_0&0xFF] = SDLK1_KP0;		/* Keypad 0-9 */
	MISC1_keymap[XK_KP_1&0xFF] = SDLK1_KP1;
	MISC1_keymap[XK_KP_2&0xFF] = SDLK1_KP2;
	MISC1_keymap[XK_KP_3&0xFF] = SDLK1_KP3;
	MISC1_keymap[XK_KP_4&0xFF] = SDLK1_KP4;
	MISC1_keymap[XK_KP_5&0xFF] = SDLK1_KP5;
	MISC1_keymap[XK_KP_6&0xFF] = SDLK1_KP6;
	MISC1_keymap[XK_KP_7&0xFF] = SDLK1_KP7;
	MISC1_keymap[XK_KP_8&0xFF] = SDLK1_KP8;
	MISC1_keymap[XK_KP_9&0xFF] = SDLK1_KP9;
	MISC1_keymap[XK_KP_Insert&0xFF] = SDLK1_KP0;
	MISC1_keymap[XK_KP_End&0xFF] = SDLK1_KP1;	
	MISC1_keymap[XK_KP_Down&0xFF] = SDLK1_KP2;
	MISC1_keymap[XK_KP_Page_Down&0xFF] = SDLK1_KP3;
	MISC1_keymap[XK_KP_Left&0xFF] = SDLK1_KP4;
	MISC1_keymap[XK_KP_Begin&0xFF] = SDLK1_KP5;
	MISC1_keymap[XK_KP_Right&0xFF] = SDLK1_KP6;
	MISC1_keymap[XK_KP_Home&0xFF] = SDLK1_KP7;
	MISC1_keymap[XK_KP_Up&0xFF] = SDLK1_KP8;
	MISC1_keymap[XK_KP_Page_Up&0xFF] = SDLK1_KP9;
	MISC1_keymap[XK_KP_Delete&0xFF] = SDLK1_KP_PERIOD;
	MISC1_keymap[XK_KP_Decimal&0xFF] = SDLK1_KP_PERIOD;
	MISC1_keymap[XK_KP_Divide&0xFF] = SDLK1_KP_DIVIDE;
	MISC1_keymap[XK_KP_Multiply&0xFF] = SDLK1_KP_MULTIPLY;
	MISC1_keymap[XK_KP_Subtract&0xFF] = SDLK1_KP_MINUS;
	MISC1_keymap[XK_KP_Add&0xFF] = SDLK1_KP_PLUS;
	MISC1_keymap[XK_KP_Enter&0xFF] = SDLK1_KP_ENTER;
	MISC1_keymap[XK_KP_Equal&0xFF] = SDLK1_KP_EQUALS;

	MISC1_keymap[XK_Up&0xFF] = SDLK1_UP;
	MISC1_keymap[XK_Down&0xFF] = SDLK1_DOWN;
	MISC1_keymap[XK_Right&0xFF] = SDLK1_RIGHT;
	MISC1_keymap[XK_Left&0xFF] = SDLK1_LEFT;
	MISC1_keymap[XK_Insert&0xFF] = SDLK1_INSERT;
	MISC1_keymap[XK_Home&0xFF] = SDLK1_HOME;
	MISC1_keymap[XK_End&0xFF] = SDLK1_END;
	MISC1_keymap[XK_Page_Up&0xFF] = SDLK1_PAGEUP;
	MISC1_keymap[XK_Page_Down&0xFF] = SDLK1_PAGEDOWN;

	MISC1_keymap[XK_F1&0xFF] = SDLK1_F1;
	MISC1_keymap[XK_F2&0xFF] = SDLK1_F2;
	MISC1_keymap[XK_F3&0xFF] = SDLK1_F3;
	MISC1_keymap[XK_F4&0xFF] = SDLK1_F4;
	MISC1_keymap[XK_F5&0xFF] = SDLK1_F5;
	MISC1_keymap[XK_F6&0xFF] = SDLK1_F6;
	MISC1_keymap[XK_F7&0xFF] = SDLK1_F7;
	MISC1_keymap[XK_F8&0xFF] = SDLK1_F8;
	MISC1_keymap[XK_F9&0xFF] = SDLK1_F9;
	MISC1_keymap[XK_F10&0xFF] = SDLK1_F10;
	MISC1_keymap[XK_F11&0xFF] = SDLK1_F11;
	MISC1_keymap[XK_F12&0xFF] = SDLK1_F12;
	MISC1_keymap[XK_F13&0xFF] = SDLK1_F13;
	MISC1_keymap[XK_F14&0xFF] = SDLK1_F14;
	MISC1_keymap[XK_F15&0xFF] = SDLK1_F15;

	MISC1_keymap[XK_Num_Lock&0xFF] = SDLK1_NUMLOCK;
	MISC1_keymap[XK_Caps_Lock&0xFF] = SDLK1_CAPSLOCK;
	MISC1_keymap[XK_Scroll_Lock&0xFF] = SDLK1_SCROLLOCK;
	MISC1_keymap[XK_Shift_R&0xFF] = SDLK1_RSHIFT;
	MISC1_keymap[XK_Shift_L&0xFF] = SDLK1_LSHIFT;
	MISC1_keymap[XK_Control_R&0xFF] = SDLK1_RCTRL;
	MISC1_keymap[XK_Control_L&0xFF] = SDLK1_LCTRL;
	MISC1_keymap[XK_Alt_R&0xFF] = SDLK1_RALT;
	MISC1_keymap[XK_Alt_L&0xFF] = SDLK1_LALT;
	MISC1_keymap[XK_Meta_R&0xFF] = SDLK1_RMETA;
	MISC1_keymap[XK_Meta_L&0xFF] = SDLK1_LMETA;
	MISC1_keymap[XK_Super_L&0xFF] = SDLK1_LSUPER; /* Left "Windows" */
	MISC1_keymap[XK_Super_R&0xFF] = SDLK1_RSUPER; /* Right "Windows */
	MISC1_keymap[XK_Mode_switch&0xFF] = SDLK1_MODE; /* "Alt Gr" key */
	MISC1_keymap[XK_Multi_key&0xFF] = SDLK1_COMPOSE; /* Multi-key compose */

	MISC1_keymap[XK_Help&0xFF] = SDLK1_HELP;
	MISC1_keymap[XK_Print&0xFF] = SDLK1_PRINT;
	MISC1_keymap[XK_Sys_Req&0xFF] = SDLK1_SYSREQ;
	MISC1_keymap[XK_Break&0xFF] = SDLK1_BREAK;
	MISC1_keymap[XK_Menu&0xFF] = SDLK1_MENU;
	MISC1_keymap[XK_Hyper_R&0xFF] = SDLK1_MENU;   /* Windows "Menu" key */
}

/* Get the translated SDL virtual keysym */
SDL_Keycode X11_TranslateKeysym(KeySym xsym)
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
			    key = (SDL_Keycode)(xsym & 0xFF);
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
			    key = (SDL_Keycode)(xsym & 0xFF);
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
SDLKey X11_Translate1Keysym(KeySym xsym)
{
    static int keymap_inited = 0;
    if (! keymap_inited) {
        X11_InitKeymap();
        keymap_inited = 1;
    }

	SDLKey key;

	key = SDLK1_UNKNOWN;
	if ( xsym ) {
		switch (xsym>>8) {
		    case 0x00:	/* Latin 1 */
			key = (SDLKey)(xsym & 0xFF);
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
			key = (SDLKey)(xsym & 0xFF);
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

SDL_Scancode GetScanFromKey(SDL_Keycode keycode){
    for (SDL_Scancode i=0; i<SDL_NUM_SCANCODES; i++)
        if (SDL_default_keymap[i] == keycode)
            return i;
    return SDL_SCANCODE_UNKNOWN;
}

SDL_Scancode GetScanFromKey1(SDLKey key){
    for (SDL_Scancode i=0; i<SDL_NUM_SCANCODES; i++)
        if (SDL1_default_keymap[i] == key)
            return i;
    return SDL_SCANCODE_UNKNOWN;
}


void xkeyboardToSDLkeyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard) {
    memset(SDLkeyboard, 0, SDL_NUM_SCANCODES);
    for (int i=0; i<16; i++) {
        if (Xkeyboard[i] != XK_VoidSymbol) {
            SDL_Scancode sc = GetScanFromKey(X11_TranslateKeysym(Xkeyboard[i]));
            SDLkeyboard[sc] = 1;
        }
    }
}

void xkeyboardToSDL1keyboard(KeySym Xkeyboard[], Uint8* SDLkeyboard) {
    memset(SDLkeyboard, 0, SDLK1_LAST);
    for (int i=0; i<16; i++) {
        if (Xkeyboard[i] != XK_VoidSymbol) {
            SDLKey key = X11_Translate1Keysym(Xkeyboard[i]);
            SDLkeyboard[key] = 1;
        }
    }
}

void xkeysymToSDL(SDL_Keysym *keysym, KeySym xkeysym) {
    keysym->sym = X11_TranslateKeysym(xkeysym);
    keysym->scancode = GetScanFromKey(keysym->sym);
    keysym->mod = KMOD_NONE; /* TODO: Add the modifier */
    keysym->unused = 0;
}

void xkeysymToSDL1(SDL_keysym *keysym, KeySym xkeysym) {
    keysym->sym = X11_Translate1Keysym(xkeysym);
    keysym->scancode = GetScanFromKey1(keysym->sym);
    keysym->mod = KMOD_NONE; /* TODO: Add the modifier */
    keysym->unicode = 0;
}


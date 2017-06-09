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

#include "keyboard_helper.h"
#include <string.h>
#include <X11/Xlib.h>

static const SDL_Keycode SDL_default_keymap[SDL_NUM_SCANCODES] = {
    0, 0, 0, 0,
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_SPACE,
    '-',
    '=',
    '[',
    ']',
    '\\',
    '#',
    ';',
    '\'',
    '`',
    ',',
    '.',
    '/',
    SDLK_CAPSLOCK,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PRINTSCREEN,
    SDLK_SCROLLLOCK,
    SDLK_PAUSE,
    SDLK_INSERT,
    SDLK_HOME,
    SDLK_PAGEUP,
    SDLK_DELETE,
    SDLK_END,
    SDLK_PAGEDOWN,
    SDLK_RIGHT,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_NUMLOCKCLEAR,
    SDLK_KP_DIVIDE,
    SDLK_KP_MULTIPLY,
    SDLK_KP_MINUS,
    SDLK_KP_PLUS,
    SDLK_KP_ENTER,
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_0,
    SDLK_KP_PERIOD,
    0,
    SDLK_APPLICATION,
    SDLK_POWER,
    SDLK_KP_EQUALS,
    SDLK_F13,
    SDLK_F14,
    SDLK_F15,
    SDLK_F16,
    SDLK_F17,
    SDLK_F18,
    SDLK_F19,
    SDLK_F20,
    SDLK_F21,
    SDLK_F22,
    SDLK_F23,
    SDLK_F24,
    SDLK_EXECUTE,
    SDLK_HELP,
    SDLK_MENU,
    SDLK_SELECT,
    SDLK_STOP,
    SDLK_AGAIN,
    SDLK_UNDO,
    SDLK_CUT,
    SDLK_COPY,
    SDLK_PASTE,
    SDLK_FIND,
    SDLK_MUTE,
    SDLK_VOLUMEUP,
    SDLK_VOLUMEDOWN,
    0, 0, 0,
    SDLK_KP_COMMA,
    SDLK_KP_EQUALSAS400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_ALTERASE,
    SDLK_SYSREQ,
    SDLK_CANCEL,
    SDLK_CLEAR,
    SDLK_PRIOR,
    SDLK_RETURN2,
    SDLK_SEPARATOR,
    SDLK_OUT,
    SDLK_OPER,
    SDLK_CLEARAGAIN,
    SDLK_CRSEL,
    SDLK_EXSEL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_KP_00,
    SDLK_KP_000,
    SDLK_THOUSANDSSEPARATOR,
    SDLK_DECIMALSEPARATOR,
    SDLK_CURRENCYUNIT,
    SDLK_CURRENCYSUBUNIT,
    SDLK_KP_LEFTPAREN,
    SDLK_KP_RIGHTPAREN,
    SDLK_KP_LEFTBRACE,
    SDLK_KP_RIGHTBRACE,
    SDLK_KP_TAB,
    SDLK_KP_BACKSPACE,
    SDLK_KP_A,
    SDLK_KP_B,
    SDLK_KP_C,
    SDLK_KP_D,
    SDLK_KP_E,
    SDLK_KP_F,
    SDLK_KP_XOR,
    SDLK_KP_POWER,
    SDLK_KP_PERCENT,
    SDLK_KP_LESS,
    SDLK_KP_GREATER,
    SDLK_KP_AMPERSAND,
    SDLK_KP_DBLAMPERSAND,
    SDLK_KP_VERTICALBAR,
    SDLK_KP_DBLVERTICALBAR,
    SDLK_KP_COLON,
    SDLK_KP_HASH,
    SDLK_KP_SPACE,
    SDLK_KP_AT,
    SDLK_KP_EXCLAM,
    SDLK_KP_MEMSTORE,
    SDLK_KP_MEMRECALL,
    SDLK_KP_MEMCLEAR,
    SDLK_KP_MEMADD,
    SDLK_KP_MEMSUBTRACT,
    SDLK_KP_MEMMULTIPLY,
    SDLK_KP_MEMDIVIDE,
    SDLK_KP_PLUSMINUS,
    SDLK_KP_CLEAR,
    SDLK_KP_CLEARENTRY,
    SDLK_KP_BINARY,
    SDLK_KP_OCTAL,
    SDLK_KP_DECIMAL,
    SDLK_KP_HEXADECIMAL,
    0, 0,
    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT,
    SDLK_RGUI,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_MODE,
    SDLK_AUDIONEXT,
    SDLK_AUDIOPREV,
    SDLK_AUDIOSTOP,
    SDLK_AUDIOPLAY,
    SDLK_AUDIOMUTE,
    SDLK_MEDIASELECT,
    SDLK_WWW,
    SDLK_MAIL,
    SDLK_CALCULATOR,
    SDLK_COMPUTER,
    SDLK_AC_SEARCH,
    SDLK_AC_HOME,
    SDLK_AC_BACK,
    SDLK_AC_FORWARD,
    SDLK_AC_STOP,
    SDLK_AC_REFRESH,
    SDLK_AC_BOOKMARKS,
    SDLK_BRIGHTNESSDOWN,
    SDLK_BRIGHTNESSUP,
    SDLK_DISPLAYSWITCH,
    SDLK_KBDILLUMTOGGLE,
    SDLK_KBDILLUMDOWN,
    SDLK_KBDILLUMUP,
    SDLK_EJECT,
    SDLK_SLEEP,
};

static const SDL1::SDLKey SDL1_default_keymap[SDL_NUM_SCANCODES] = {
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_a,
    SDL1::SDLK_b,
    SDL1::SDLK_c,
    SDL1::SDLK_d,
    SDL1::SDLK_e,
    SDL1::SDLK_f,
    SDL1::SDLK_g,
    SDL1::SDLK_h,
    SDL1::SDLK_i,
    SDL1::SDLK_j,
    SDL1::SDLK_k,
    SDL1::SDLK_l,
    SDL1::SDLK_m,
    SDL1::SDLK_n,
    SDL1::SDLK_o,
    SDL1::SDLK_p,
    SDL1::SDLK_q,
    SDL1::SDLK_r,
    SDL1::SDLK_s,
    SDL1::SDLK_t,
    SDL1::SDLK_u,
    SDL1::SDLK_v,
    SDL1::SDLK_w,
    SDL1::SDLK_x,
    SDL1::SDLK_y,
    SDL1::SDLK_z,
    SDL1::SDLK_1,
    SDL1::SDLK_2,
    SDL1::SDLK_3,
    SDL1::SDLK_4,
    SDL1::SDLK_5,
    SDL1::SDLK_6,
    SDL1::SDLK_7,
    SDL1::SDLK_8,
    SDL1::SDLK_9,
    SDL1::SDLK_0,
    SDL1::SDLK_RETURN,
    SDL1::SDLK_ESCAPE,
    SDL1::SDLK_BACKSPACE,
    SDL1::SDLK_TAB,
    SDL1::SDLK_SPACE,
	SDL1::SDLK_MINUS,
	SDL1::SDLK_EQUALS,
	SDL1::SDLK_LEFTBRACKET,
	SDL1::SDLK_RIGHTBRACKET,
	SDL1::SDLK_BACKSLASH,
	SDL1::SDLK_HASH,
	SDL1::SDLK_SEMICOLON,
	SDL1::SDLK_QUOTE,
	SDL1::SDLK_BACKQUOTE,
	SDL1::SDLK_COMMA,
	SDL1::SDLK_PERIOD,
	SDL1::SDLK_SLASH,
    SDL1::SDLK_CAPSLOCK,
    SDL1::SDLK_F1,
    SDL1::SDLK_F2,
    SDL1::SDLK_F3,
    SDL1::SDLK_F4,
    SDL1::SDLK_F5,
    SDL1::SDLK_F6,
    SDL1::SDLK_F7,
    SDL1::SDLK_F8,
    SDL1::SDLK_F9,
    SDL1::SDLK_F10,
    SDL1::SDLK_F11,
    SDL1::SDLK_F12,
    SDL1::SDLK_PRINT,
	SDL1::SDLK_SCROLLOCK,
    SDL1::SDLK_PAUSE,
    SDL1::SDLK_INSERT,
    SDL1::SDLK_HOME,
    SDL1::SDLK_PAGEUP,
    SDL1::SDLK_DELETE,
    SDL1::SDLK_END,
    SDL1::SDLK_PAGEDOWN,
    SDL1::SDLK_RIGHT,
    SDL1::SDLK_LEFT,
    SDL1::SDLK_DOWN,
    SDL1::SDLK_UP,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_KP_DIVIDE,
    SDL1::SDLK_KP_MULTIPLY,
    SDL1::SDLK_KP_MINUS,
    SDL1::SDLK_KP_PLUS,
    SDL1::SDLK_KP_ENTER,
    SDL1::SDLK_KP1,
    SDL1::SDLK_KP2,
    SDL1::SDLK_KP3,
    SDL1::SDLK_KP4,
    SDL1::SDLK_KP5,
    SDL1::SDLK_KP6,
    SDL1::SDLK_KP7,
    SDL1::SDLK_KP8,
    SDL1::SDLK_KP9,
    SDL1::SDLK_KP0,
    SDL1::SDLK_KP_PERIOD,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_POWER,
    SDL1::SDLK_KP_EQUALS,
    SDL1::SDLK_F13,
    SDL1::SDLK_F14,
    SDL1::SDLK_F15,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_HELP,
    SDL1::SDLK_MENU,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_SYSREQ,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_CLEAR,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_LCTRL,
    SDL1::SDLK_LSHIFT,
    SDL1::SDLK_LALT,
    SDL1::SDLK_LMETA,
    SDL1::SDLK_RCTRL,
    SDL1::SDLK_RSHIFT,
    SDL1::SDLK_RALT,
    SDL1::SDLK_RMETA,
    SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN, SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_MODE,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
    SDL1::SDLK_UNKNOWN,
};

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
SDL1::SDLKey X11_Translate1Keysym(KeySym xsym)
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

SDL_Scancode GetScanFromKey(SDL_Keycode keycode){
    for (int i=0; i<static_cast<int>(SDL_NUM_SCANCODES); i++)
        if (SDL_default_keymap[i] == keycode)
            return static_cast<SDL_Scancode>(i);
    return SDL_SCANCODE_UNKNOWN;
}

SDL_Scancode GetScanFromKey1(SDL1::SDLKey key){
    for (int i=0; i<static_cast<int>(SDL_NUM_SCANCODES); i++)
        if (SDL1_default_keymap[i] == key)
            return static_cast<SDL_Scancode>(i);
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
    memset(SDLkeyboard, 0, SDL1::SDLK_LAST);
    for (int i=0; i<16; i++) {
        if (Xkeyboard[i] != XK_VoidSymbol) {
            SDL1::SDLKey key = X11_Translate1Keysym(Xkeyboard[i]);
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

void xkeysymToSDL1(SDL1::SDL_keysym *keysym, KeySym xkeysym) {
    keysym->sym = X11_Translate1Keysym(xkeysym);
    keysym->scancode = GetScanFromKey1(keysym->sym);
    keysym->mod = KMOD_NONE; /* TODO: Add the modifier */
    keysym->unicode = 0;
}

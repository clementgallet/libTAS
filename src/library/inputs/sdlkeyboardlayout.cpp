/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdlkeyboardlayout.h"

#include "logging.h"

namespace libtas {

static const sdl2::SDL_Keycode SDL2_default_keymap[sdl2::SDL_NUM_SCANCODES] = {
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
    sdl2::SDLK_RETURN,
    sdl2::SDLK_ESCAPE,
    sdl2::SDLK_BACKSPACE,
    sdl2::SDLK_TAB,
    sdl2::SDLK_SPACE,
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
    sdl2::SDLK_CAPSLOCK,
    sdl2::SDLK_F1,
    sdl2::SDLK_F2,
    sdl2::SDLK_F3,
    sdl2::SDLK_F4,
    sdl2::SDLK_F5,
    sdl2::SDLK_F6,
    sdl2::SDLK_F7,
    sdl2::SDLK_F8,
    sdl2::SDLK_F9,
    sdl2::SDLK_F10,
    sdl2::SDLK_F11,
    sdl2::SDLK_F12,
    sdl2::SDLK_PRINTSCREEN,
    sdl2::SDLK_SCROLLLOCK,
    sdl2::SDLK_PAUSE,
    sdl2::SDLK_INSERT,
    sdl2::SDLK_HOME,
    sdl2::SDLK_PAGEUP,
    sdl2::SDLK_DELETE,
    sdl2::SDLK_END,
    sdl2::SDLK_PAGEDOWN,
    sdl2::SDLK_RIGHT,
    sdl2::SDLK_LEFT,
    sdl2::SDLK_DOWN,
    sdl2::SDLK_UP,
    sdl2::SDLK_NUMLOCKCLEAR,
    sdl2::SDLK_KP_DIVIDE,
    sdl2::SDLK_KP_MULTIPLY,
    sdl2::SDLK_KP_MINUS,
    sdl2::SDLK_KP_PLUS,
    sdl2::SDLK_KP_ENTER,
    sdl2::SDLK_KP_1,
    sdl2::SDLK_KP_2,
    sdl2::SDLK_KP_3,
    sdl2::SDLK_KP_4,
    sdl2::SDLK_KP_5,
    sdl2::SDLK_KP_6,
    sdl2::SDLK_KP_7,
    sdl2::SDLK_KP_8,
    sdl2::SDLK_KP_9,
    sdl2::SDLK_KP_0,
    sdl2::SDLK_KP_PERIOD,
    0,
    sdl2::SDLK_APPLICATION,
    sdl2::SDLK_POWER,
    sdl2::SDLK_KP_EQUALS,
    sdl2::SDLK_F13,
    sdl2::SDLK_F14,
    sdl2::SDLK_F15,
    sdl2::SDLK_F16,
    sdl2::SDLK_F17,
    sdl2::SDLK_F18,
    sdl2::SDLK_F19,
    sdl2::SDLK_F20,
    sdl2::SDLK_F21,
    sdl2::SDLK_F22,
    sdl2::SDLK_F23,
    sdl2::SDLK_F24,
    sdl2::SDLK_EXECUTE,
    sdl2::SDLK_HELP,
    sdl2::SDLK_MENU,
    sdl2::SDLK_SELECT,
    sdl2::SDLK_STOP,
    sdl2::SDLK_AGAIN,
    sdl2::SDLK_UNDO,
    sdl2::SDLK_CUT,
    sdl2::SDLK_COPY,
    sdl2::SDLK_PASTE,
    sdl2::SDLK_FIND,
    sdl2::SDLK_MUTE,
    sdl2::SDLK_VOLUMEUP,
    sdl2::SDLK_VOLUMEDOWN,
    0, 0, 0,
    sdl2::SDLK_KP_COMMA,
    sdl2::SDLK_KP_EQUALSAS400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    sdl2::SDLK_ALTERASE,
    sdl2::SDLK_SYSREQ,
    sdl2::SDLK_CANCEL,
    sdl2::SDLK_CLEAR,
    sdl2::SDLK_PRIOR,
    sdl2::SDLK_RETURN2,
    sdl2::SDLK_SEPARATOR,
    sdl2::SDLK_OUT,
    sdl2::SDLK_OPER,
    sdl2::SDLK_CLEARAGAIN,
    sdl2::SDLK_CRSEL,
    sdl2::SDLK_EXSEL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    sdl2::SDLK_KP_00,
    sdl2::SDLK_KP_000,
    sdl2::SDLK_THOUSANDSSEPARATOR,
    sdl2::SDLK_DECIMALSEPARATOR,
    sdl2::SDLK_CURRENCYUNIT,
    sdl2::SDLK_CURRENCYSUBUNIT,
    sdl2::SDLK_KP_LEFTPAREN,
    sdl2::SDLK_KP_RIGHTPAREN,
    sdl2::SDLK_KP_LEFTBRACE,
    sdl2::SDLK_KP_RIGHTBRACE,
    sdl2::SDLK_KP_TAB,
    sdl2::SDLK_KP_BACKSPACE,
    sdl2::SDLK_KP_A,
    sdl2::SDLK_KP_B,
    sdl2::SDLK_KP_C,
    sdl2::SDLK_KP_D,
    sdl2::SDLK_KP_E,
    sdl2::SDLK_KP_F,
    sdl2::SDLK_KP_XOR,
    sdl2::SDLK_KP_POWER,
    sdl2::SDLK_KP_PERCENT,
    sdl2::SDLK_KP_LESS,
    sdl2::SDLK_KP_GREATER,
    sdl2::SDLK_KP_AMPERSAND,
    sdl2::SDLK_KP_DBLAMPERSAND,
    sdl2::SDLK_KP_VERTICALBAR,
    sdl2::SDLK_KP_DBLVERTICALBAR,
    sdl2::SDLK_KP_COLON,
    sdl2::SDLK_KP_HASH,
    sdl2::SDLK_KP_SPACE,
    sdl2::SDLK_KP_AT,
    sdl2::SDLK_KP_EXCLAM,
    sdl2::SDLK_KP_MEMSTORE,
    sdl2::SDLK_KP_MEMRECALL,
    sdl2::SDLK_KP_MEMCLEAR,
    sdl2::SDLK_KP_MEMADD,
    sdl2::SDLK_KP_MEMSUBTRACT,
    sdl2::SDLK_KP_MEMMULTIPLY,
    sdl2::SDLK_KP_MEMDIVIDE,
    sdl2::SDLK_KP_PLUSMINUS,
    sdl2::SDLK_KP_CLEAR,
    sdl2::SDLK_KP_CLEARENTRY,
    sdl2::SDLK_KP_BINARY,
    sdl2::SDLK_KP_OCTAL,
    sdl2::SDLK_KP_DECIMAL,
    sdl2::SDLK_KP_HEXADECIMAL,
    0, 0,
    sdl2::SDLK_LCTRL,
    sdl2::SDLK_LSHIFT,
    sdl2::SDLK_LALT,
    sdl2::SDLK_LGUI,
    sdl2::SDLK_RCTRL,
    sdl2::SDLK_RSHIFT,
    sdl2::SDLK_RALT,
    sdl2::SDLK_RGUI,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    sdl2::SDLK_MODE,
    sdl2::SDLK_AUDIONEXT,
    sdl2::SDLK_AUDIOPREV,
    sdl2::SDLK_AUDIOSTOP,
    sdl2::SDLK_AUDIOPLAY,
    sdl2::SDLK_AUDIOMUTE,
    sdl2::SDLK_MEDIASELECT,
    sdl2::SDLK_WWW,
    sdl2::SDLK_MAIL,
    sdl2::SDLK_CALCULATOR,
    sdl2::SDLK_COMPUTER,
    sdl2::SDLK_AC_SEARCH,
    sdl2::SDLK_AC_HOME,
    sdl2::SDLK_AC_BACK,
    sdl2::SDLK_AC_FORWARD,
    sdl2::SDLK_AC_STOP,
    sdl2::SDLK_AC_REFRESH,
    sdl2::SDLK_AC_BOOKMARKS,
    sdl2::SDLK_BRIGHTNESSDOWN,
    sdl2::SDLK_BRIGHTNESSUP,
    sdl2::SDLK_DISPLAYSWITCH,
    sdl2::SDLK_KBDILLUMTOGGLE,
    sdl2::SDLK_KBDILLUMDOWN,
    sdl2::SDLK_KBDILLUMUP,
    sdl2::SDLK_EJECT,
    sdl2::SDLK_SLEEP,
};

static const int SDL1_NUM_SCANCODES = 220;

static const sdl1::SDLKey SDL1_default_keymap[SDL1_NUM_SCANCODES] = {
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_ESCAPE,
	sdl1::SDLK_1, sdl1::SDLK_2, sdl1::SDLK_3, sdl1::SDLK_4, sdl1::SDLK_5,
    sdl1::SDLK_6, sdl1::SDLK_7, sdl1::SDLK_8, sdl1::SDLK_9, sdl1::SDLK_0,
	/* 0x0c: */
	sdl1::SDLK_MINUS, sdl1::SDLK_EQUALS, sdl1::SDLK_BACKSPACE, sdl1::SDLK_TAB,
	sdl1::SDLK_q, sdl1::SDLK_w, sdl1::SDLK_e, sdl1::SDLK_r, sdl1::SDLK_t,
    sdl1::SDLK_y, sdl1::SDLK_u, sdl1::SDLK_i, sdl1::SDLK_o, sdl1::SDLK_p,
	sdl1::SDLK_LEFTBRACKET, sdl1::SDLK_RIGHTBRACKET,
    sdl1::SDLK_RETURN, sdl1::SDLK_LCTRL,
	sdl1::SDLK_a, sdl1::SDLK_s, sdl1::SDLK_d, sdl1::SDLK_f, sdl1::SDLK_g,
    sdl1::SDLK_h, sdl1::SDLK_j, sdl1::SDLK_k, sdl1::SDLK_l,
	sdl1::SDLK_SEMICOLON, sdl1::SDLK_QUOTE, sdl1::SDLK_BACKQUOTE,
    sdl1::SDLK_LSHIFT, sdl1::SDLK_BACKSLASH,
	sdl1::SDLK_z, sdl1::SDLK_x, sdl1::SDLK_c, sdl1::SDLK_v,
    sdl1::SDLK_b, sdl1::SDLK_n, sdl1::SDLK_m,
	/* 0x33: */
	sdl1::SDLK_COMMA, sdl1::SDLK_PERIOD, sdl1::SDLK_SLASH,
    sdl1::SDLK_RSHIFT, sdl1::SDLK_KP_MULTIPLY,
	sdl1::SDLK_LALT, sdl1::SDLK_SPACE, sdl1::SDLK_CAPSLOCK,
	sdl1::SDLK_F1, sdl1::SDLK_F2, sdl1::SDLK_F3, sdl1::SDLK_F4, sdl1::SDLK_F5,
    sdl1::SDLK_F6, sdl1::SDLK_F7, sdl1::SDLK_F8, sdl1::SDLK_F9, sdl1::SDLK_F10,
	/* 0x45: */
	sdl1::SDLK_NUMLOCK, sdl1::SDLK_SCROLLOCK,
	sdl1::SDLK_KP7, sdl1::SDLK_KP8, sdl1::SDLK_KP9, sdl1::SDLK_KP_MINUS,
    sdl1::SDLK_KP4, sdl1::SDLK_KP5, sdl1::SDLK_KP6, sdl1::SDLK_KP_PLUS,
	sdl1::SDLK_KP1, sdl1::SDLK_KP2, sdl1::SDLK_KP3, sdl1::SDLK_KP0, sdl1::SDLK_KP_PERIOD,
	sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
	sdl1::SDLK_LESS, sdl1::SDLK_F11, sdl1::SDLK_F12,
	sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,	sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,	sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
    sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN, sdl1::SDLK_UNKNOWN,
};

sdl2::SDL_Scancode GetScanFromKey(sdl2::SDL_Keycode keycode){
    for (int i=0; i<static_cast<int>(sdl2::SDL_NUM_SCANCODES); i++)
        if (SDL2_default_keymap[i] == keycode)
            return static_cast<sdl2::SDL_Scancode>(i);
    return sdl2::SDL_SCANCODE_UNKNOWN;
}

unsigned char GetScanFromKey1(sdl1::SDLKey key){
    for (int i=0; i<static_cast<int>(SDL1_NUM_SCANCODES); i++)
        if (SDL1_default_keymap[i] == key)
            return static_cast<unsigned char>(i);
    return 0;
}

/* Override */ sdl2::SDL_Keycode sdl2::SDL_GetKeyFromScancode(sdl2::SDL_Scancode scancode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_KEYBOARD, "%s called with scancode %d", __func__, (int)scancode);
    sdl2::SDL_Keycode keycode = SDL2_default_keymap[scancode];
    LOG(LL_DEBUG, LCF_SDL | LCF_KEYBOARD, "   returning %d", keycode);
    return keycode;
}

/* Override */ sdl2::SDL_Scancode sdl2::SDL_GetScancodeFromKey(sdl2::SDL_Keycode key)
{
    LOG(LL_TRACE, LCF_SDL | LCF_KEYBOARD, "%s called with key %d", __func__, key);
    sdl2::SDL_Scancode scancode = GetScanFromKey(key);
    LOG(LL_DEBUG, LCF_SDL | LCF_KEYBOARD, "   returning %d", scancode);
    return scancode;
}

sdl3::SDL_Keycode sdl3::SDL_GetKeyFromScancode(sdl3::SDL_Scancode scancode, sdl3::SDL_Keymod modstate, bool key_event)
{
    LOG(LL_TRACE, LCF_SDL | LCF_KEYBOARD, "%s called with scancode %d, modstate %d, key_event %d", __func__, (int)scancode, modstate, key_event);
    /* TODO: Implement proper key code retrieval based on modifier state and key event flag */
    sdl3::SDL_Keycode keycode = SDL2_default_keymap[scancode];
    LOG(LL_DEBUG, LCF_SDL | LCF_KEYBOARD, "   returning %d", keycode);
    return keycode;
}

sdl3::SDL_Scancode sdl3::SDL_GetScancodeFromKey(sdl3::SDL_Keycode key, sdl3::SDL_Keymod *modstate)
{
    LOG(LL_TRACE, LCF_SDL | LCF_KEYBOARD, "%s called with key %d", __func__, key);
    sdl3::SDL_Scancode scancode = static_cast<sdl3::SDL_Scancode>(GetScanFromKey(static_cast<sdl2::SDL_Keycode>(key)));
    if (modstate) {
        *modstate = 0; // Initialize modifier state
    }
    return scancode;
}

}

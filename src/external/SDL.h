#ifndef _SDL_h
#define _SDL_h

#include <inttypes.h> 
#include <X11/Xlib.h>

#define SDL_INIT_TIMER          0x00000001
#define SDL_INIT_AUDIO          0x00000010
#define SDL_INIT_VIDEO          0x00000020  /**< SDL_INIT_VIDEO implies SDL_INIT_EVENTS */
#define SDL_INIT_CDROM          0x00000100  /* SDL 1.2 only */
#define SDL_INIT_JOYSTICK       0x00000200  /**< SDL_INIT_JOYSTICK implies SDL_INIT_EVENTS */
#define SDL_INIT_HAPTIC         0x00001000
#define SDL_INIT_GAMECONTROLLER 0x00002000  /**< SDL_INIT_GAMECONTROLLER implies SDL_INIT_JOYSTICK */
#define SDL_INIT_EVENTS         0x00004000
#define SDL_INIT_NOPARACHUTE    0x00100000  /**< Don't catch fatal signals */
#define SDL_INIT_EVENTTHREAD    0x01000000  /**< SDL 1.2 only Not supported on all OS's */


typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;
typedef int32_t Sint32;
typedef int16_t Sint16;
typedef int64_t Sint64;

typedef enum
{
    SDL_FALSE = 0,
    SDL_TRUE = 1
} SDL_bool;

typedef Sint32 SDL_JoystickID;

typedef struct SDL_Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} SDL_Color;

typedef struct SDL_Palette
{
    int ncolors;
    SDL_Color *colors;
    Uint32 version;
    int refcount;
} SDL_Palette;


#define SDL_RELEASED	0
#define SDL_PRESSED	1

typedef enum
{
    SDL_SCANCODE_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    SDL_SCANCODE_A = 4,
    SDL_SCANCODE_B = 5,
    SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7,
    SDL_SCANCODE_E = 8,
    SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10,
    SDL_SCANCODE_H = 11,
    SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13,
    SDL_SCANCODE_K = 14,
    SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16,
    SDL_SCANCODE_N = 17,
    SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19,
    SDL_SCANCODE_Q = 20,
    SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22,
    SDL_SCANCODE_T = 23,
    SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25,
    SDL_SCANCODE_W = 26,
    SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28,
    SDL_SCANCODE_Z = 29,

    SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31,
    SDL_SCANCODE_3 = 32,
    SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34,
    SDL_SCANCODE_6 = 35,
    SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37,
    SDL_SCANCODE_9 = 38,
    SDL_SCANCODE_0 = 39,

    SDL_SCANCODE_RETURN = 40,
    SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44,

    SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    SDL_SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate SDL_SCANCODE_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    SDL_SCANCODE_SEMICOLON = 51,
    SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    SDL_SCANCODE_COMMA = 54,
    SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56,

    SDL_SCANCODE_CAPSLOCK = 57,

    SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59,
    SDL_SCANCODE_F3 = 60,
    SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62,
    SDL_SCANCODE_F6 = 63,
    SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65,
    SDL_SCANCODE_F9 = 66,
    SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68,
    SDL_SCANCODE_F12 = 69,

    SDL_SCANCODE_PRINTSCREEN = 70,
    SDL_SCANCODE_SCROLLLOCK = 71,
    SDL_SCANCODE_PAUSE = 72,
    SDL_SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    SDL_SCANCODE_HOME = 74,
    SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76,
    SDL_SCANCODE_END = 77,
    SDL_SCANCODE_PAGEDOWN = 78,
    SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81,
    SDL_SCANCODE_UP = 82,

    SDL_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    SDL_SCANCODE_KP_DIVIDE = 84,
    SDL_SCANCODE_KP_MULTIPLY = 85,
    SDL_SCANCODE_KP_MINUS = 86,
    SDL_SCANCODE_KP_PLUS = 87,
    SDL_SCANCODE_KP_ENTER = 88,
    SDL_SCANCODE_KP_1 = 89,
    SDL_SCANCODE_KP_2 = 90,
    SDL_SCANCODE_KP_3 = 91,
    SDL_SCANCODE_KP_4 = 92,
    SDL_SCANCODE_KP_5 = 93,
    SDL_SCANCODE_KP_6 = 94,
    SDL_SCANCODE_KP_7 = 95,
    SDL_SCANCODE_KP_8 = 96,
    SDL_SCANCODE_KP_9 = 97,
    SDL_SCANCODE_KP_0 = 98,
    SDL_SCANCODE_KP_PERIOD = 99,

    SDL_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    SDL_SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
    SDL_SCANCODE_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    SDL_SCANCODE_KP_EQUALS = 103,
    SDL_SCANCODE_F13 = 104,
    SDL_SCANCODE_F14 = 105,
    SDL_SCANCODE_F15 = 106,
    SDL_SCANCODE_F16 = 107,
    SDL_SCANCODE_F17 = 108,
    SDL_SCANCODE_F18 = 109,
    SDL_SCANCODE_F19 = 110,
    SDL_SCANCODE_F20 = 111,
    SDL_SCANCODE_F21 = 112,
    SDL_SCANCODE_F22 = 113,
    SDL_SCANCODE_F23 = 114,
    SDL_SCANCODE_F24 = 115,
    SDL_SCANCODE_EXECUTE = 116,
    SDL_SCANCODE_HELP = 117,
    SDL_SCANCODE_MENU = 118,
    SDL_SCANCODE_SELECT = 119,
    SDL_SCANCODE_STOP = 120,
    SDL_SCANCODE_AGAIN = 121,   /**< redo */
    SDL_SCANCODE_UNDO = 122,
    SDL_SCANCODE_CUT = 123,
    SDL_SCANCODE_COPY = 124,
    SDL_SCANCODE_PASTE = 125,
    SDL_SCANCODE_FIND = 126,
    SDL_SCANCODE_MUTE = 127,
    SDL_SCANCODE_VOLUMEUP = 128,
    SDL_SCANCODE_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     SDL_SCANCODE_LOCKINGCAPSLOCK = 130,  */
/*     SDL_SCANCODE_LOCKINGNUMLOCK = 131, */
/*     SDL_SCANCODE_LOCKINGSCROLLLOCK = 132, */
    SDL_SCANCODE_KP_COMMA = 133,
    SDL_SCANCODE_KP_EQUALSAS400 = 134,

    SDL_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    SDL_SCANCODE_INTERNATIONAL2 = 136,
    SDL_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
    SDL_SCANCODE_INTERNATIONAL4 = 138,
    SDL_SCANCODE_INTERNATIONAL5 = 139,
    SDL_SCANCODE_INTERNATIONAL6 = 140,
    SDL_SCANCODE_INTERNATIONAL7 = 141,
    SDL_SCANCODE_INTERNATIONAL8 = 142,
    SDL_SCANCODE_INTERNATIONAL9 = 143,
    SDL_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
    SDL_SCANCODE_LANG2 = 145, /**< Hanja conversion */
    SDL_SCANCODE_LANG3 = 146, /**< Katakana */
    SDL_SCANCODE_LANG4 = 147, /**< Hiragana */
    SDL_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
    SDL_SCANCODE_LANG6 = 149, /**< reserved */
    SDL_SCANCODE_LANG7 = 150, /**< reserved */
    SDL_SCANCODE_LANG8 = 151, /**< reserved */
    SDL_SCANCODE_LANG9 = 152, /**< reserved */

    SDL_SCANCODE_ALTERASE = 153, /**< Erase-Eaze */
    SDL_SCANCODE_SYSREQ = 154,
    SDL_SCANCODE_CANCEL = 155,
    SDL_SCANCODE_CLEAR = 156,
    SDL_SCANCODE_PRIOR = 157,
    SDL_SCANCODE_RETURN2 = 158,
    SDL_SCANCODE_SEPARATOR = 159,
    SDL_SCANCODE_OUT = 160,
    SDL_SCANCODE_OPER = 161,
    SDL_SCANCODE_CLEARAGAIN = 162,
    SDL_SCANCODE_CRSEL = 163,
    SDL_SCANCODE_EXSEL = 164,

    SDL_SCANCODE_KP_00 = 176,
    SDL_SCANCODE_KP_000 = 177,
    SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL_SCANCODE_DECIMALSEPARATOR = 179,
    SDL_SCANCODE_CURRENCYUNIT = 180,
    SDL_SCANCODE_CURRENCYSUBUNIT = 181,
    SDL_SCANCODE_KP_LEFTPAREN = 182,
    SDL_SCANCODE_KP_RIGHTPAREN = 183,
    SDL_SCANCODE_KP_LEFTBRACE = 184,
    SDL_SCANCODE_KP_RIGHTBRACE = 185,
    SDL_SCANCODE_KP_TAB = 186,
    SDL_SCANCODE_KP_BACKSPACE = 187,
    SDL_SCANCODE_KP_A = 188,
    SDL_SCANCODE_KP_B = 189,
    SDL_SCANCODE_KP_C = 190,
    SDL_SCANCODE_KP_D = 191,
    SDL_SCANCODE_KP_E = 192,
    SDL_SCANCODE_KP_F = 193,
    SDL_SCANCODE_KP_XOR = 194,
    SDL_SCANCODE_KP_POWER = 195,
    SDL_SCANCODE_KP_PERCENT = 196,
    SDL_SCANCODE_KP_LESS = 197,
    SDL_SCANCODE_KP_GREATER = 198,
    SDL_SCANCODE_KP_AMPERSAND = 199,
    SDL_SCANCODE_KP_DBLAMPERSAND = 200,
    SDL_SCANCODE_KP_VERTICALBAR = 201,
    SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
    SDL_SCANCODE_KP_COLON = 203,
    SDL_SCANCODE_KP_HASH = 204,
    SDL_SCANCODE_KP_SPACE = 205,
    SDL_SCANCODE_KP_AT = 206,
    SDL_SCANCODE_KP_EXCLAM = 207,
    SDL_SCANCODE_KP_MEMSTORE = 208,
    SDL_SCANCODE_KP_MEMRECALL = 209,
    SDL_SCANCODE_KP_MEMCLEAR = 210,
    SDL_SCANCODE_KP_MEMADD = 211,
    SDL_SCANCODE_KP_MEMSUBTRACT = 212,
    SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL_SCANCODE_KP_MEMDIVIDE = 214,
    SDL_SCANCODE_KP_PLUSMINUS = 215,
    SDL_SCANCODE_KP_CLEAR = 216,
    SDL_SCANCODE_KP_CLEARENTRY = 217,
    SDL_SCANCODE_KP_BINARY = 218,
    SDL_SCANCODE_KP_OCTAL = 219,
    SDL_SCANCODE_KP_DECIMAL = 220,
    SDL_SCANCODE_KP_HEXADECIMAL = 221,

    SDL_SCANCODE_LCTRL = 224,
    SDL_SCANCODE_LSHIFT = 225,
    SDL_SCANCODE_LALT = 226, /**< alt, option */
    SDL_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
    SDL_SCANCODE_RCTRL = 228,
    SDL_SCANCODE_RSHIFT = 229,
    SDL_SCANCODE_RALT = 230, /**< alt gr, option */
    SDL_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

    SDL_SCANCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    SDL_SCANCODE_AUDIONEXT = 258,
    SDL_SCANCODE_AUDIOPREV = 259,
    SDL_SCANCODE_AUDIOSTOP = 260,
    SDL_SCANCODE_AUDIOPLAY = 261,
    SDL_SCANCODE_AUDIOMUTE = 262,
    SDL_SCANCODE_MEDIASELECT = 263,
    SDL_SCANCODE_WWW = 264,
    SDL_SCANCODE_MAIL = 265,
    SDL_SCANCODE_CALCULATOR = 266,
    SDL_SCANCODE_COMPUTER = 267,
    SDL_SCANCODE_AC_SEARCH = 268,
    SDL_SCANCODE_AC_HOME = 269,
    SDL_SCANCODE_AC_BACK = 270,
    SDL_SCANCODE_AC_FORWARD = 271,
    SDL_SCANCODE_AC_STOP = 272,
    SDL_SCANCODE_AC_REFRESH = 273,
    SDL_SCANCODE_AC_BOOKMARKS = 274,

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    SDL_SCANCODE_BRIGHTNESSDOWN = 275,
    SDL_SCANCODE_BRIGHTNESSUP = 276,
    SDL_SCANCODE_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    SDL_SCANCODE_KBDILLUMTOGGLE = 278,
    SDL_SCANCODE_KBDILLUMDOWN = 279,
    SDL_SCANCODE_KBDILLUMUP = 280,
    SDL_SCANCODE_EJECT = 281,
    SDL_SCANCODE_SLEEP = 282,

    SDL_SCANCODE_APP1 = 283,
    SDL_SCANCODE_APP2 = 284,

    /* @} *//* Walther keys */

    /* Add any other keys here. */

    SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
} SDL_Scancode;

typedef Sint32 SDL_Keycode;

#define SDLK_SCANCODE_MASK (1<<30)
#define SDL_SCANCODE_TO_KEYCODE(X)  (X | SDLK_SCANCODE_MASK)

enum
{
    SDLK_UNKNOWN = 0,

    SDLK_RETURN = '\r',
    SDLK_ESCAPE = '\033',
    SDLK_BACKSPACE = '\b',
    SDLK_TAB = '\t',
    SDLK_SPACE = ' ',
    SDLK_EXCLAIM = '!',
    SDLK_QUOTEDBL = '"',
    SDLK_HASH = '#',
    SDLK_PERCENT = '%',
    SDLK_DOLLAR = '$',
    SDLK_AMPERSAND = '&',
    SDLK_QUOTE = '\'',
    SDLK_LEFTPAREN = '(',
    SDLK_RIGHTPAREN = ')',
    SDLK_ASTERISK = '*',
    SDLK_PLUS = '+',
    SDLK_COMMA = ',',
    SDLK_MINUS = '-',
    SDLK_PERIOD = '.',
    SDLK_SLASH = '/',
    SDLK_0 = '0',
    SDLK_1 = '1',
    SDLK_2 = '2',
    SDLK_3 = '3',
    SDLK_4 = '4',
    SDLK_5 = '5',
    SDLK_6 = '6',
    SDLK_7 = '7',
    SDLK_8 = '8',
    SDLK_9 = '9',
    SDLK_COLON = ':',
    SDLK_SEMICOLON = ';',
    SDLK_LESS = '<',
    SDLK_EQUALS = '=',
    SDLK_GREATER = '>',
    SDLK_QUESTION = '?',
    SDLK_AT = '@',
    /*
       Skip uppercase letters
     */
    SDLK_LEFTBRACKET = '[',
    SDLK_BACKSLASH = '\\',
    SDLK_RIGHTBRACKET = ']',
    SDLK_CARET = '^',
    SDLK_UNDERSCORE = '_',
    SDLK_BACKQUOTE = '`',
    SDLK_a = 'a',
    SDLK_b = 'b',
    SDLK_c = 'c',
    SDLK_d = 'd',
    SDLK_e = 'e',
    SDLK_f = 'f',
    SDLK_g = 'g',
    SDLK_h = 'h',
    SDLK_i = 'i',
    SDLK_j = 'j',
    SDLK_k = 'k',
    SDLK_l = 'l',
    SDLK_m = 'm',
    SDLK_n = 'n',
    SDLK_o = 'o',
    SDLK_p = 'p',
    SDLK_q = 'q',
    SDLK_r = 'r',
    SDLK_s = 's',
    SDLK_t = 't',
    SDLK_u = 'u',
    SDLK_v = 'v',
    SDLK_w = 'w',
    SDLK_x = 'x',
    SDLK_y = 'y',
    SDLK_z = 'z',

    SDLK_CAPSLOCK = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),

    SDLK_F1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
    SDLK_F2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
    SDLK_F3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
    SDLK_F4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
    SDLK_F5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
    SDLK_F6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
    SDLK_F7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
    SDLK_F8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
    SDLK_F9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
    SDLK_F10 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
    SDLK_F11 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
    SDLK_F12 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),

    SDLK_PRINTSCREEN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
    SDLK_SCROLLLOCK = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
    SDLK_PAUSE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
    SDLK_INSERT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
    SDLK_HOME = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
    SDLK_PAGEUP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
    SDLK_DELETE = '\177',
    SDLK_END = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
    SDLK_PAGEDOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
    SDLK_RIGHT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
    SDLK_LEFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
    SDLK_DOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
    SDLK_UP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),

    SDLK_NUMLOCKCLEAR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
    SDLK_KP_DIVIDE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
    SDLK_KP_MULTIPLY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
    SDLK_KP_MINUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
    SDLK_KP_PLUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
    SDLK_KP_ENTER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
    SDLK_KP_1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
    SDLK_KP_2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
    SDLK_KP_3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
    SDLK_KP_4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
    SDLK_KP_5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
    SDLK_KP_6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
    SDLK_KP_7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
    SDLK_KP_8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
    SDLK_KP_9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
    SDLK_KP_0 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
    SDLK_KP_PERIOD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),

    SDLK_APPLICATION = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION),
    SDLK_POWER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER),
    SDLK_KP_EQUALS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS),
    SDLK_F13 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13),
    SDLK_F14 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14),
    SDLK_F15 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15),
    SDLK_F16 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16),
    SDLK_F17 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17),
    SDLK_F18 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18),
    SDLK_F19 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19),
    SDLK_F20 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20),
    SDLK_F21 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21),
    SDLK_F22 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22),
    SDLK_F23 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23),
    SDLK_F24 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24),
    SDLK_EXECUTE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE),
    SDLK_HELP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP),
    SDLK_MENU = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU),
    SDLK_SELECT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT),
    SDLK_STOP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP),
    SDLK_AGAIN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN),
    SDLK_UNDO = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO),
    SDLK_CUT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT),
    SDLK_COPY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY),
    SDLK_PASTE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE),
    SDLK_FIND = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND),
    SDLK_MUTE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE),
    SDLK_VOLUMEUP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP),
    SDLK_VOLUMEDOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN),
    SDLK_KP_COMMA = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA),
    SDLK_KP_EQUALSAS400 =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400),

    SDLK_ALTERASE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE),
    SDLK_SYSREQ = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ),
    SDLK_CANCEL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL),
    SDLK_CLEAR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR),
    SDLK_PRIOR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR),
    SDLK_RETURN2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2),
    SDLK_SEPARATOR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR),
    SDLK_OUT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT),
    SDLK_OPER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER),
    SDLK_CLEARAGAIN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN),
    SDLK_CRSEL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL),
    SDLK_EXSEL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL),

    SDLK_KP_00 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00),
    SDLK_KP_000 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000),
    SDLK_THOUSANDSSEPARATOR =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR),
    SDLK_DECIMALSEPARATOR =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR),
    SDLK_CURRENCYUNIT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT),
    SDLK_CURRENCYSUBUNIT =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT),
    SDLK_KP_LEFTPAREN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN),
    SDLK_KP_RIGHTPAREN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN),
    SDLK_KP_LEFTBRACE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE),
    SDLK_KP_RIGHTBRACE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE),
    SDLK_KP_TAB = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB),
    SDLK_KP_BACKSPACE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE),
    SDLK_KP_A = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A),
    SDLK_KP_B = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B),
    SDLK_KP_C = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C),
    SDLK_KP_D = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D),
    SDLK_KP_E = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E),
    SDLK_KP_F = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F),
    SDLK_KP_XOR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR),
    SDLK_KP_POWER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER),
    SDLK_KP_PERCENT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT),
    SDLK_KP_LESS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS),
    SDLK_KP_GREATER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER),
    SDLK_KP_AMPERSAND = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND),
    SDLK_KP_DBLAMPERSAND =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND),
    SDLK_KP_VERTICALBAR =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR),
    SDLK_KP_DBLVERTICALBAR =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR),
    SDLK_KP_COLON = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON),
    SDLK_KP_HASH = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH),
    SDLK_KP_SPACE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE),
    SDLK_KP_AT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT),
    SDLK_KP_EXCLAM = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM),
    SDLK_KP_MEMSTORE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE),
    SDLK_KP_MEMRECALL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL),
    SDLK_KP_MEMCLEAR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR),
    SDLK_KP_MEMADD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD),
    SDLK_KP_MEMSUBTRACT =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT),
    SDLK_KP_MEMMULTIPLY =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY),
    SDLK_KP_MEMDIVIDE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE),
    SDLK_KP_PLUSMINUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS),
    SDLK_KP_CLEAR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR),
    SDLK_KP_CLEARENTRY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY),
    SDLK_KP_BINARY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY),
    SDLK_KP_OCTAL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL),
    SDLK_KP_DECIMAL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL),
    SDLK_KP_HEXADECIMAL =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL),

    SDLK_LCTRL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
    SDLK_LSHIFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
    SDLK_LALT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
    SDLK_LGUI = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
    SDLK_RCTRL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
    SDLK_RSHIFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
    SDLK_RALT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
    SDLK_RGUI = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

    SDLK_MODE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),

    SDLK_AUDIONEXT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT),
    SDLK_AUDIOPREV = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV),
    SDLK_AUDIOSTOP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP),
    SDLK_AUDIOPLAY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY),
    SDLK_AUDIOMUTE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE),
    SDLK_MEDIASELECT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT),
    SDLK_WWW = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW),
    SDLK_MAIL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL),
    SDLK_CALCULATOR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR),
    SDLK_COMPUTER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER),
    SDLK_AC_SEARCH = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH),
    SDLK_AC_HOME = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME),
    SDLK_AC_BACK = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK),
    SDLK_AC_FORWARD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD),
    SDLK_AC_STOP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP),
    SDLK_AC_REFRESH = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH),
    SDLK_AC_BOOKMARKS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS),

    SDLK_BRIGHTNESSDOWN =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN),
    SDLK_BRIGHTNESSUP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP),
    SDLK_DISPLAYSWITCH = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH),
    SDLK_KBDILLUMTOGGLE =
        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE),
    SDLK_KBDILLUMDOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN),
    SDLK_KBDILLUMUP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP),
    SDLK_EJECT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT),
    SDLK_SLEEP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP)
};

typedef enum {
        /** @name ASCII mapped keysyms
         *  The keyboard syms have been cleverly chosen to map to ASCII
         */
        /*@{*/
	SDLK1_UNKNOWN		= 0,
	SDLK1_FIRST		= 0,
	SDLK1_BACKSPACE		= 8,
	SDLK1_TAB		= 9,
	SDLK1_CLEAR		= 12,
	SDLK1_RETURN		= 13,
	SDLK1_PAUSE		= 19,
	SDLK1_ESCAPE		= 27,
	SDLK1_SPACE		= 32,
	SDLK1_EXCLAIM		= 33,
	SDLK1_QUOTEDBL		= 34,
	SDLK1_HASH		= 35,
	SDLK1_DOLLAR		= 36,
	SDLK1_AMPERSAND		= 38,
	SDLK1_QUOTE		= 39,
	SDLK1_LEFTPAREN		= 40,
	SDLK1_RIGHTPAREN		= 41,
	SDLK1_ASTERISK		= 42,
	SDLK1_PLUS		= 43,
	SDLK1_COMMA		= 44,
	SDLK1_MINUS		= 45,
	SDLK1_PERIOD		= 46,
	SDLK1_SLASH		= 47,
	SDLK1_0			= 48,
	SDLK1_1			= 49,
	SDLK1_2			= 50,
	SDLK1_3			= 51,
	SDLK1_4			= 52,
	SDLK1_5			= 53,
	SDLK1_6			= 54,
	SDLK1_7			= 55,
	SDLK1_8			= 56,
	SDLK1_9			= 57,
	SDLK1_COLON		= 58,
	SDLK1_SEMICOLON		= 59,
	SDLK1_LESS		= 60,
	SDLK1_EQUALS		= 61,
	SDLK1_GREATER		= 62,
	SDLK1_QUESTION		= 63,
	SDLK1_AT			= 64,
	/* 
	   Skip uppercase letters
	 */
	SDLK1_LEFTBRACKET	= 91,
	SDLK1_BACKSLASH		= 92,
	SDLK1_RIGHTBRACKET	= 93,
	SDLK1_CARET		= 94,
	SDLK1_UNDERSCORE		= 95,
	SDLK1_BACKQUOTE		= 96,
	SDLK1_a			= 97,
	SDLK1_b			= 98,
	SDLK1_c			= 99,
	SDLK1_d			= 100,
	SDLK1_e			= 101,
	SDLK1_f			= 102,
	SDLK1_g			= 103,
	SDLK1_h			= 104,
	SDLK1_i			= 105,
	SDLK1_j			= 106,
	SDLK1_k			= 107,
	SDLK1_l			= 108,
	SDLK1_m			= 109,
	SDLK1_n			= 110,
	SDLK1_o			= 111,
	SDLK1_p			= 112,
	SDLK1_q			= 113,
	SDLK1_r			= 114,
	SDLK1_s			= 115,
	SDLK1_t			= 116,
	SDLK1_u			= 117,
	SDLK1_v			= 118,
	SDLK1_w			= 119,
	SDLK1_x			= 120,
	SDLK1_y			= 121,
	SDLK1_z			= 122,
	SDLK1_DELETE		= 127,
	/* End of ASCII mapped keysyms */
        /*@}*/

	/** @name International keyboard syms */
        /*@{*/
	SDLK1_WORLD_0		= 160,		/* 0xA0 */
	SDLK1_WORLD_1		= 161,
	SDLK1_WORLD_2		= 162,
	SDLK1_WORLD_3		= 163,
	SDLK1_WORLD_4		= 164,
	SDLK1_WORLD_5		= 165,
	SDLK1_WORLD_6		= 166,
	SDLK1_WORLD_7		= 167,
	SDLK1_WORLD_8		= 168,
	SDLK1_WORLD_9		= 169,
	SDLK1_WORLD_10		= 170,
	SDLK1_WORLD_11		= 171,
	SDLK1_WORLD_12		= 172,
	SDLK1_WORLD_13		= 173,
	SDLK1_WORLD_14		= 174,
	SDLK1_WORLD_15		= 175,
	SDLK1_WORLD_16		= 176,
	SDLK1_WORLD_17		= 177,
	SDLK1_WORLD_18		= 178,
	SDLK1_WORLD_19		= 179,
	SDLK1_WORLD_20		= 180,
	SDLK1_WORLD_21		= 181,
	SDLK1_WORLD_22		= 182,
	SDLK1_WORLD_23		= 183,
	SDLK1_WORLD_24		= 184,
	SDLK1_WORLD_25		= 185,
	SDLK1_WORLD_26		= 186,
	SDLK1_WORLD_27		= 187,
	SDLK1_WORLD_28		= 188,
	SDLK1_WORLD_29		= 189,
	SDLK1_WORLD_30		= 190,
	SDLK1_WORLD_31		= 191,
	SDLK1_WORLD_32		= 192,
	SDLK1_WORLD_33		= 193,
	SDLK1_WORLD_34		= 194,
	SDLK1_WORLD_35		= 195,
	SDLK1_WORLD_36		= 196,
	SDLK1_WORLD_37		= 197,
	SDLK1_WORLD_38		= 198,
	SDLK1_WORLD_39		= 199,
	SDLK1_WORLD_40		= 200,
	SDLK1_WORLD_41		= 201,
	SDLK1_WORLD_42		= 202,
	SDLK1_WORLD_43		= 203,
	SDLK1_WORLD_44		= 204,
	SDLK1_WORLD_45		= 205,
	SDLK1_WORLD_46		= 206,
	SDLK1_WORLD_47		= 207,
	SDLK1_WORLD_48		= 208,
	SDLK1_WORLD_49		= 209,
	SDLK1_WORLD_50		= 210,
	SDLK1_WORLD_51		= 211,
	SDLK1_WORLD_52		= 212,
	SDLK1_WORLD_53		= 213,
	SDLK1_WORLD_54		= 214,
	SDLK1_WORLD_55		= 215,
	SDLK1_WORLD_56		= 216,
	SDLK1_WORLD_57		= 217,
	SDLK1_WORLD_58		= 218,
	SDLK1_WORLD_59		= 219,
	SDLK1_WORLD_60		= 220,
	SDLK1_WORLD_61		= 221,
	SDLK1_WORLD_62		= 222,
	SDLK1_WORLD_63		= 223,
	SDLK1_WORLD_64		= 224,
	SDLK1_WORLD_65		= 225,
	SDLK1_WORLD_66		= 226,
	SDLK1_WORLD_67		= 227,
	SDLK1_WORLD_68		= 228,
	SDLK1_WORLD_69		= 229,
	SDLK1_WORLD_70		= 230,
	SDLK1_WORLD_71		= 231,
	SDLK1_WORLD_72		= 232,
	SDLK1_WORLD_73		= 233,
	SDLK1_WORLD_74		= 234,
	SDLK1_WORLD_75		= 235,
	SDLK1_WORLD_76		= 236,
	SDLK1_WORLD_77		= 237,
	SDLK1_WORLD_78		= 238,
	SDLK1_WORLD_79		= 239,
	SDLK1_WORLD_80		= 240,
	SDLK1_WORLD_81		= 241,
	SDLK1_WORLD_82		= 242,
	SDLK1_WORLD_83		= 243,
	SDLK1_WORLD_84		= 244,
	SDLK1_WORLD_85		= 245,
	SDLK1_WORLD_86		= 246,
	SDLK1_WORLD_87		= 247,
	SDLK1_WORLD_88		= 248,
	SDLK1_WORLD_89		= 249,
	SDLK1_WORLD_90		= 250,
	SDLK1_WORLD_91		= 251,
	SDLK1_WORLD_92		= 252,
	SDLK1_WORLD_93		= 253,
	SDLK1_WORLD_94		= 254,
	SDLK1_WORLD_95		= 255,		/* 0xFF */
        /*@}*/

	/** @name Numeric keypad */
        /*@{*/
	SDLK1_KP0		= 256,
	SDLK1_KP1		= 257,
	SDLK1_KP2		= 258,
	SDLK1_KP3		= 259,
	SDLK1_KP4		= 260,
	SDLK1_KP5		= 261,
	SDLK1_KP6		= 262,
	SDLK1_KP7		= 263,
	SDLK1_KP8		= 264,
	SDLK1_KP9		= 265,
	SDLK1_KP_PERIOD		= 266,
	SDLK1_KP_DIVIDE		= 267,
	SDLK1_KP_MULTIPLY	= 268,
	SDLK1_KP_MINUS		= 269,
	SDLK1_KP_PLUS		= 270,
	SDLK1_KP_ENTER		= 271,
	SDLK1_KP_EQUALS		= 272,
        /*@}*/

	/** @name Arrows + Home/End pad */
        /*@{*/
	SDLK1_UP			= 273,
	SDLK1_DOWN		= 274,
	SDLK1_RIGHT		= 275,
	SDLK1_LEFT		= 276,
	SDLK1_INSERT		= 277,
	SDLK1_HOME		= 278,
	SDLK1_END		= 279,
	SDLK1_PAGEUP		= 280,
	SDLK1_PAGEDOWN		= 281,
        /*@}*/

	/** @name Function keys */
        /*@{*/
	SDLK1_F1			= 282,
	SDLK1_F2			= 283,
	SDLK1_F3			= 284,
	SDLK1_F4			= 285,
	SDLK1_F5			= 286,
	SDLK1_F6			= 287,
	SDLK1_F7			= 288,
	SDLK1_F8			= 289,
	SDLK1_F9			= 290,
	SDLK1_F10		= 291,
	SDLK1_F11		= 292,
	SDLK1_F12		= 293,
	SDLK1_F13		= 294,
	SDLK1_F14		= 295,
	SDLK1_F15		= 296,
        /*@}*/

	/** @name Key state modifier keys */
        /*@{*/
	SDLK1_NUMLOCK		= 300,
	SDLK1_CAPSLOCK		= 301,
	SDLK1_SCROLLOCK		= 302,
	SDLK1_RSHIFT		= 303,
	SDLK1_LSHIFT		= 304,
	SDLK1_RCTRL		= 305,
	SDLK1_LCTRL		= 306,
	SDLK1_RALT		= 307,
	SDLK1_LALT		= 308,
	SDLK1_RMETA		= 309,
	SDLK1_LMETA		= 310,
	SDLK1_LSUPER		= 311,		/**< Left "Windows" key */
	SDLK1_RSUPER		= 312,		/**< Right "Windows" key */
	SDLK1_MODE		= 313,		/**< "Alt Gr" key */
	SDLK1_COMPOSE		= 314,		/**< Multi-key compose key */
        /*@}*/

	/** @name Miscellaneous function keys */
        /*@{*/
	SDLK1_HELP		= 315,
	SDLK1_PRINT		= 316,
	SDLK1_SYSREQ		= 317,
	SDLK1_BREAK		= 318,
	SDLK1_MENU		= 319,
	SDLK1_POWER		= 320,		/**< Power Macintosh power key */
	SDLK1_EURO		= 321,		/**< Some european keyboards */
	SDLK1_UNDO		= 322,		/**< Atari keyboard has Undo */
        /*@}*/

	/* Add any other keys here */

	SDLK1_LAST
} SDLKey;




/**
 * \brief Enumeration of valid key mods (possibly OR'd together).
 */
typedef enum
{
    KMOD_NONE = 0x0000,
    KMOD_LSHIFT = 0x0001,
    KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL = 0x0040,
    KMOD_RCTRL = 0x0080,
    KMOD_LALT = 0x0100,
    KMOD_RALT = 0x0200,
    KMOD_LGUI = 0x0400,
    KMOD_RGUI = 0x0800,
    KMOD_NUM = 0x1000,
    KMOD_CAPS = 0x2000,
    KMOD_MODE = 0x4000,
    KMOD_RESERVED = 0x8000
} SDL_Keymod;

#define KMOD_CTRL   (KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT  (KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT    (KMOD_LALT|KMOD_RALT)
#define KMOD_GUI    (KMOD_LGUI|KMOD_RGUI)


typedef struct SDL_Keysym
{
    SDL_Scancode scancode;      /**< SDL physical key code - see ::SDL_Scancode for details */
    SDL_Keycode sym;            /**< SDL virtual key code - see ::SDL_Keycode for details */
    Uint16 mod;                 /**< current key modifiers */
    Uint32 unused;
} SDL_Keysym;


typedef struct SDL_KeyboardEvent
{
    Uint32 type;        /**< ::SDL_KEYDOWN or ::SDL_KEYUP */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with keyboard focus, if any */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 repeat;       /**< Non-zero if this is a key repeat */
    Uint8 padding2;
    Uint8 padding3;
    SDL_Keysym keysym;  /**< The key that was pressed or released */
} SDL_KeyboardEvent;

/**
 *  \brief Window state change event data (event.window.*)
 */
typedef struct SDL_WindowEvent
{
    Uint32 type;        /**< ::SDL_WINDOWEVENT */
    Uint32 timestamp;
    Uint32 windowID;    /**< The associated window */
    Uint8 event;        /**< ::SDL_WindowEventID */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;       /**< event dependent data */
    Sint32 data2;       /**< event dependent data */
} SDL_WindowEvent;

/**
 *  \brief Event subtype for window events
 */
typedef enum
{
    SDL_WINDOWEVENT_NONE,           /**< Never used */
    SDL_WINDOWEVENT_SHOWN,          /**< Window has been shown */
    SDL_WINDOWEVENT_HIDDEN,         /**< Window has been hidden */
    SDL_WINDOWEVENT_EXPOSED,        /**< Window has been exposed and should be
                                         redrawn */
    SDL_WINDOWEVENT_MOVED,          /**< Window has been moved to data1, data2
                                     */
    SDL_WINDOWEVENT_RESIZED,        /**< Window has been resized to data1xdata2 */
    SDL_WINDOWEVENT_SIZE_CHANGED,   /**< The window size has changed, either as a result of an API call or through the system or user changing the window size. */
    SDL_WINDOWEVENT_MINIMIZED,      /**< Window has been minimized */
    SDL_WINDOWEVENT_MAXIMIZED,      /**< Window has been maximized */
    SDL_WINDOWEVENT_RESTORED,       /**< Window has been restored to normal size
                                         and position */
    SDL_WINDOWEVENT_ENTER,          /**< Window has gained mouse focus */
    SDL_WINDOWEVENT_LEAVE,          /**< Window has lost mouse focus */
    SDL_WINDOWEVENT_FOCUS_GAINED,   /**< Window has gained keyboard focus */
    SDL_WINDOWEVENT_FOCUS_LOST,     /**< Window has lost keyboard focus */
    SDL_WINDOWEVENT_CLOSE           /**< The window manager requests that the
                                         window be closed */
} SDL_WindowEventID;

/**
 *  \brief Mouse motion event structure (event.motion.*)
 */
typedef struct SDL_MouseMotionEvent
{
    Uint32 type;        /**< ::SDL_MOUSEMOTION */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint32 state;       /**< The current button state */
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
    Sint32 xrel;        /**< The relative motion in the X direction */
    Sint32 yrel;        /**< The relative motion in the Y direction */
} SDL_MouseMotionEvent;

/**
 *  \brief Mouse button event structure (event.button.*)
 */
typedef struct SDL_MouseButtonEvent
{
    Uint32 type;        /**< ::SDL_MOUSEBUTTONDOWN or ::SDL_MOUSEBUTTONUP */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint8 button;       /**< The mouse button index */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 clicks;       /**< 1 for single-click, 2 for double-click, etc. */
    Uint8 padding1;
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
} SDL_MouseButtonEvent;

/**
 *  \brief Mouse wheel event structure (event.wheel.*)
 */
typedef struct SDL_MouseWheelEvent
{
    Uint32 type;        /**< ::SDL_MOUSEWHEEL */
    Uint32 timestamp;
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Sint32 x;           /**< The amount scrolled horizontally, positive to the right and negative to the left */
    Sint32 y;           /**< The amount scrolled vertically, positive away from the user and negative toward the user */
} SDL_MouseWheelEvent;

/**
 *  \brief Joystick axis motion event structure (event.jaxis.*)
 */
typedef struct SDL_JoyAxisEvent
{
    Uint32 type;        /**< ::SDL_JOYAXISMOTION */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 axis;         /**< The joystick axis index */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;       /**< The axis value (range: -32768 to 32767) */
    Uint16 padding4;
} SDL_JoyAxisEvent;

/**
 *  \brief Joystick trackball motion event structure (event.jball.*)
 */
typedef struct SDL_JoyBallEvent
{
    Uint32 type;        /**< ::SDL_JOYBALLMOTION */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 ball;         /**< The joystick trackball index */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 xrel;        /**< The relative motion in the X direction */
    Sint16 yrel;        /**< The relative motion in the Y direction */
} SDL_JoyBallEvent;

/**
 *  \brief Joystick hat position change event structure (event.jhat.*)
 */
typedef struct SDL_JoyHatEvent
{
    Uint32 type;        /**< ::SDL_JOYHATMOTION */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 hat;          /**< The joystick hat index */
    Uint8 value;        /**< The hat position value.
                         *   \sa ::SDL_HAT_LEFTUP ::SDL_HAT_UP ::SDL_HAT_RIGHTUP
                         *   \sa ::SDL_HAT_LEFT ::SDL_HAT_CENTERED ::SDL_HAT_RIGHT
                         *   \sa ::SDL_HAT_LEFTDOWN ::SDL_HAT_DOWN ::SDL_HAT_RIGHTDOWN
                         *
                         *   Note that zero means the POV is centered.
                         */
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyHatEvent;

/**
 *  \brief Joystick button event structure (event.jbutton.*)
 */
typedef struct SDL_JoyButtonEvent
{
    Uint32 type;        /**< ::SDL_JOYBUTTONDOWN or ::SDL_JOYBUTTONUP */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 button;       /**< The joystick button index */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyButtonEvent;

/**
 *  \brief Joystick device event structure (event.jdevice.*)
 */
typedef struct SDL_JoyDeviceEvent
{
    Uint32 type;        /**< ::SDL_JOYDEVICEADDED or ::SDL_JOYDEVICEREMOVED */
    Uint32 timestamp;
    Sint32 which;       /**< The joystick device index for the ADDED event, instance id for the REMOVED event */
} SDL_JoyDeviceEvent;


/**
 *  \brief Game controller axis motion event structure (event.caxis.*)
 */
typedef struct SDL_ControllerAxisEvent
{
    Uint32 type;        /**< ::SDL_CONTROLLERAXISMOTION */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 axis;         /**< The controller axis (SDL_GameControllerAxis) */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;       /**< The axis value (range: -32768 to 32767) */
    Uint16 padding4;
} SDL_ControllerAxisEvent;


/**
 *  \brief Game controller button event structure (event.cbutton.*)
 */
typedef struct SDL_ControllerButtonEvent
{
    Uint32 type;        /**< ::SDL_CONTROLLERBUTTONDOWN or ::SDL_CONTROLLERBUTTONUP */
    Uint32 timestamp;
    SDL_JoystickID which; /**< The joystick instance id */
    Uint8 button;       /**< The controller button (SDL_GameControllerButton) */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 padding1;
    Uint8 padding2;
} SDL_ControllerButtonEvent;


/**
 *  \brief Controller device event structure (event.cdevice.*)
 */
typedef struct SDL_ControllerDeviceEvent
{
    Uint32 type;        /**< ::SDL_CONTROLLERDEVICEADDED, ::SDL_CONTROLLERDEVICEREMOVED, or ::SDL_CONTROLLERDEVICEREMAPPED */
    Uint32 timestamp;
    Sint32 which;       /**< The joystick device index for the ADDED event, instance id for the REMOVED or REMAPPED event */
} SDL_ControllerDeviceEvent;

/**
 *  These are the various supported windowing subsystems
 */
typedef enum
{
    SDL_SYSWM_UNKNOWN,
    SDL_SYSWM_WINDOWS,
    SDL_SYSWM_X11,
    SDL_SYSWM_DIRECTFB,
    SDL_SYSWM_COCOA,
    SDL_SYSWM_UIKIT,
    SDL_SYSWM_WAYLAND,
    SDL_SYSWM_MIR,
} SDL_SYSWM_TYPE;


typedef struct SDL_version
{
    Uint8 major;        /**< major version */
    Uint8 minor;        /**< minor version */
    Uint8 patch;        /**< update version */
} SDL_version;

/**
 *  The custom event structure.
 */
struct SDL_SysWMmsg
{
    SDL_version version;
    SDL_SYSWM_TYPE subsystem;
    union
    {
        struct {
            XEvent event;
        } x11;
        /* Can't have an empty union */
        int dummy;
    } msg;
};

typedef struct SDL_SysWMmsg SDL_SysWMmsg;


/**
 *  The custom window manager information structure.
 *
 *  When this structure is returned, it holds information about which
 *  low level system it is using, and will be one of SDL_SYSWM_TYPE.
 */
struct SDL_SysWMinfo
{
    SDL_version version;
    SDL_SYSWM_TYPE subsystem;
    union
    {
        struct
        {
            Display *display;           /**< The X11 display */
            Window window;              /**< The X11 window */
        } x11;
   } info;
};

typedef struct SDL_SysWMinfo SDL_SysWMinfo;

/**
 *  \brief A video driver dependent system event (event.syswm.*)
 *         This event is disabled by default, you can enable it with SDL_EventState()
 *
 *  \note If you want to use this event, you should include SDL_syswm.h.
 */
typedef struct SDL_SysWMEvent
{
    Uint32 type;        /**< ::SDL_SYSWMEVENT */
    Uint32 timestamp;
    SDL_SysWMmsg *msg;  /**< driver dependent data, defined in SDL_syswm.h */
} SDL_SysWMEvent;


typedef union SDL_Event
{
    Uint32 type;                    /**< Event type, shared with all events */
    SDL_WindowEvent window;         /**< Window event data */
    SDL_KeyboardEvent key;          /**< Keyboard event data */
    SDL_MouseMotionEvent motion;    /**< Mouse motion event data */
    SDL_MouseButtonEvent button;    /**< Mouse button event data */
    SDL_MouseWheelEvent wheel;      /**< Mouse wheel event data */
    SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
    SDL_JoyBallEvent jball;         /**< Joystick ball event data */
    SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
    SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
    SDL_JoyDeviceEvent jdevice;     /**< Joystick device change event data */
    SDL_ControllerAxisEvent caxis;      /**< Game Controller axis event data */
    SDL_ControllerButtonEvent cbutton;  /**< Game Controller button event data */
    SDL_ControllerDeviceEvent cdevice;  /**< Game Controller device event data */
    SDL_SysWMEvent syswm;           /**< System dependent window event data */
    Uint8 padding[56];
} SDL_Event;

/**
 * \brief The types of events that can be delivered.
 */
typedef enum
{
    SDL_FIRSTEVENT     = 0,     /**< Unused (do not remove) */

    /* Application events */
    SDL_QUIT           = 0x100, /**< User-requested quit */

    /* These application events have special meaning on iOS, see README-ios.txt for details */
    SDL_APP_TERMINATING,        /**< The application is being terminated by the OS
                                     Called on iOS in applicationWillTerminate()
                                     Called on Android in onDestroy()
                                */
    SDL_APP_LOWMEMORY,          /**< The application is low on memory, free memory if possible.
                                     Called on iOS in applicationDidReceiveMemoryWarning()
                                     Called on Android in onLowMemory()
                                */
    SDL_APP_WILLENTERBACKGROUND, /**< The application is about to enter the background
                                     Called on iOS in applicationWillResignActive()
                                     Called on Android in onPause()
                                */
    SDL_APP_DIDENTERBACKGROUND, /**< The application did enter the background and may not get CPU for some time
                                     Called on iOS in applicationDidEnterBackground()
                                     Called on Android in onPause()
                                */
    SDL_APP_WILLENTERFOREGROUND, /**< The application is about to enter the foreground
                                     Called on iOS in applicationWillEnterForeground()
                                     Called on Android in onResume()
                                */
    SDL_APP_DIDENTERFOREGROUND, /**< The application is now interactive
                                     Called on iOS in applicationDidBecomeActive()
                                     Called on Android in onResume()
                                */

    /* Window events */
    SDL_WINDOWEVENT    = 0x200, /**< Window state change */
    SDL_SYSWMEVENT,             /**< System specific event */

    /* Keyboard events */
    SDL_KEYDOWN        = 0x300, /**< Key pressed */
    SDL_KEYUP,                  /**< Key released */
    SDL_TEXTEDITING,            /**< Keyboard text editing (composition) */
    SDL_TEXTINPUT,              /**< Keyboard text input */

    /* Mouse events */
    SDL_MOUSEMOTION    = 0x400, /**< Mouse moved */
    SDL_MOUSEBUTTONDOWN,        /**< Mouse button pressed */
    SDL_MOUSEBUTTONUP,          /**< Mouse button released */
    SDL_MOUSEWHEEL,             /**< Mouse wheel motion */

    /* Joystick events */
    SDL_JOYAXISMOTION  = 0x600, /**< Joystick axis motion */
    SDL_JOYBALLMOTION,          /**< Joystick trackball motion */
    SDL_JOYHATMOTION,           /**< Joystick hat position change */
    SDL_JOYBUTTONDOWN,          /**< Joystick button pressed */
    SDL_JOYBUTTONUP,            /**< Joystick button released */
    SDL_JOYDEVICEADDED,         /**< A new joystick has been inserted into the system */
    SDL_JOYDEVICEREMOVED,       /**< An opened joystick has been removed */

    /* Game controller events */
    SDL_CONTROLLERAXISMOTION  = 0x650, /**< Game controller axis motion */
    SDL_CONTROLLERBUTTONDOWN,          /**< Game controller button pressed */
    SDL_CONTROLLERBUTTONUP,            /**< Game controller button released */
    SDL_CONTROLLERDEVICEADDED,         /**< A new Game controller has been inserted into the system */
    SDL_CONTROLLERDEVICEREMOVED,       /**< An opened Game controller has been removed */
    SDL_CONTROLLERDEVICEREMAPPED,      /**< The controller mapping was updated */

    /* Touch events */
    SDL_FINGERDOWN      = 0x700,
    SDL_FINGERUP,
    SDL_FINGERMOTION,

    /* Gesture events */
    SDL_DOLLARGESTURE   = 0x800,
    SDL_DOLLARRECORD,
    SDL_MULTIGESTURE,

    /* Clipboard events */
    SDL_CLIPBOARDUPDATE = 0x900, /**< The clipboard changed */

    /* Drag and drop events */
    SDL_DROPFILE        = 0x1000, /**< The system requests a file open */

    /* Render events */
    SDL_RENDER_TARGETS_RESET = 0x2000, /**< The render targets have been reset */

    /** Events ::SDL_USEREVENT through ::SDL_LASTEVENT are for your use,
     *  and should be allocated with SDL_RegisterEvents()
     */
    SDL_USEREVENT    = 0x8000,

    /**
     *  This last event is only for bounding internal arrays
     */
    SDL_LASTEVENT    = 0xFFFF
} SDL_EventType;

typedef enum
{
    SDL_ADDEVENT,
    SDL_PEEKEVENT,
    SDL_GETEVENT
} SDL_eventaction;


typedef struct SDL_PixelFormat
{
    Uint32 format;
    void *palette;
    Uint8 BitsPerPixel;
    Uint8 BytesPerPixel;
    Uint8 padding[2];
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rloss;
    Uint8 Gloss;
    Uint8 Bloss;
    Uint8 Aloss;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;
    int refcount;
    struct SDL_PixelFormat *next;
} SDL_PixelFormat;

typedef struct SDL_Rect
{
    int x, y;
    int w, h;
} SDL_Rect;

typedef struct SDL_Surface
{
    Uint32 flags;               /**< Read-only */
    SDL_PixelFormat *format;    /**< Read-only */
    int w, h;                   /**< Read-only */
    int pitch;                  /**< Read-only */
    void *pixels;               /**< Read-write */

    /** Application data associated with the surface */
    void *userdata;             /**< Read-write */

    /** information needed for surfaces requiring locks */
    int locked;                 /**< Read-only */
    void *lock_data;            /**< Read-only */

    /** clipping information */
    SDL_Rect clip_rect;         /**< Read-only */

    /** info for fast blit mapping to other surfaces */
    void *map;    /**< Private */

    /** Reference count -- used when freeing surface */
    int refcount;               /**< Read-mostly */
} SDL_Surface;


typedef enum
{
    SDL_CONTROLLER_AXIS_INVALID = -1,
    SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_AXIS_MAX
} SDL_GameControllerAxis;

/**
 *  The list of buttons available from a controller
 */
typedef enum
{
    SDL_CONTROLLER_BUTTON_INVALID = -1,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_MAX
} SDL_GameControllerButton;

#define SDL_SWSURFACE       0           /**< Just here for compatibility */




/*** SDL 1.2 event structures ***/

/** Event enumerations */
typedef enum {
       SDL1_NOEVENT = 0,			/**< Unused (do not remove) */
       SDL1_ACTIVEEVENT,			/**< Application loses/gains visibility */
       SDL1_KEYDOWN,			/**< Keys pressed */
       SDL1_KEYUP,			/**< Keys released */
       SDL1_MOUSEMOTION,			/**< Mouse moved */
       SDL1_MOUSEBUTTONDOWN,		/**< Mouse button pressed */
       SDL1_MOUSEBUTTONUP,		/**< Mouse button released */
       SDL1_JOYAXISMOTION,		/**< Joystick axis motion */
       SDL1_JOYBALLMOTION,		/**< Joystick trackball motion */
       SDL1_JOYHATMOTION,		/**< Joystick hat position change */
       SDL1_JOYBUTTONDOWN,		/**< Joystick button pressed */
       SDL1_JOYBUTTONUP,			/**< Joystick button released */
       SDL1_QUIT,			/**< User-requested quit */
       SDL1_SYSWMEVENT,			/**< System specific event */
       SDL1_EVENT_RESERVEDA,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVEDB,		/**< Reserved for future use.. */
       SDL1_VIDEORESIZE,			/**< User resized video mode */
       SDL1_VIDEOEXPOSE,			/**< Screen needs to be redrawn */
       SDL1_EVENT_RESERVED2,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVED3,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVED4,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVED5,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVED6,		/**< Reserved for future use.. */
       SDL1_EVENT_RESERVED7,		/**< Reserved for future use.. */
       /** Events SDL_USEREVENT through SDL_MAXEVENTS-1 are for your use */
       SDL1_USEREVENT = 24,
       /** This last event is only for bounding internal arrays
	*  It is the number of bits in the event mask datatype -- Uint32
        */
       SDL1_NUMEVENTS = 32
} SDL1_EventType;

/** @name Predefined event masks */
/*@{*/
#define SDL1_EVENTMASK(X)	(1<<(X))
typedef enum {
	SDL1_ACTIVEEVENTMASK	= SDL1_EVENTMASK(SDL1_ACTIVEEVENT),
	SDL1_KEYDOWNMASK		= SDL1_EVENTMASK(SDL1_KEYDOWN),
	SDL1_KEYUPMASK	        = SDL1_EVENTMASK(SDL1_KEYUP),
	SDL1_KEYEVENTMASK	    = SDL1_EVENTMASK(SDL1_KEYDOWN)|
	                          SDL1_EVENTMASK(SDL1_KEYUP),
	SDL1_MOUSEMOTIONMASK	= SDL1_EVENTMASK(SDL1_MOUSEMOTION),
	SDL1_MOUSEBUTTONDOWNMASK	= SDL1_EVENTMASK(SDL1_MOUSEBUTTONDOWN),
	SDL1_MOUSEBUTTONUPMASK	= SDL1_EVENTMASK(SDL1_MOUSEBUTTONUP),
	SDL1_MOUSEEVENTMASK	= SDL1_EVENTMASK(SDL1_MOUSEMOTION)|
	                          SDL1_EVENTMASK(SDL1_MOUSEBUTTONDOWN)|
	                          SDL1_EVENTMASK(SDL1_MOUSEBUTTONUP),
	SDL1_JOYAXISMOTIONMASK	= SDL1_EVENTMASK(SDL1_JOYAXISMOTION),
	SDL1_JOYBALLMOTIONMASK	= SDL1_EVENTMASK(SDL1_JOYBALLMOTION),
	SDL1_JOYHATMOTIONMASK	= SDL1_EVENTMASK(SDL1_JOYHATMOTION),
	SDL1_JOYBUTTONDOWNMASK	= SDL1_EVENTMASK(SDL1_JOYBUTTONDOWN),
	SDL1_JOYBUTTONUPMASK	= SDL1_EVENTMASK(SDL1_JOYBUTTONUP),
	SDL1_JOYEVENTMASK	= SDL1_EVENTMASK(SDL1_JOYAXISMOTION)|
	                          SDL1_EVENTMASK(SDL1_JOYBALLMOTION)|
	                          SDL1_EVENTMASK(SDL1_JOYHATMOTION)|
	                          SDL1_EVENTMASK(SDL1_JOYBUTTONDOWN)|
	                          SDL1_EVENTMASK(SDL1_JOYBUTTONUP),
	SDL1_VIDEORESIZEMASK	= SDL1_EVENTMASK(SDL1_VIDEORESIZE),
	SDL1_VIDEOEXPOSEMASK	= SDL1_EVENTMASK(SDL1_VIDEOEXPOSE),
	SDL1_QUITMASK		= SDL1_EVENTMASK(SDL1_QUIT),
	SDL1_SYSWMEVENTMASK	= SDL1_EVENTMASK(SDL1_SYSWMEVENT)
} SDL1_EventMask ;
#define SDL1_ALLEVENTS		0xFFFFFFFF
/*@}*/

/** Application visibility event structure */
typedef struct SDL1_ActiveEvent {
	Uint8 type;	/**< SDL_ACTIVEEVENT */
	Uint8 gain;	/**< Whether given states were gained or lost (1/0) */
	Uint8 state;	/**< A mask of the focus states */
} SDL1_ActiveEvent;

typedef int SDLMod;

typedef struct SDL_keysym {
    Uint8 scancode;         /**< hardware specific scancode */
    SDLKey sym;         /**< SDL virtual keysym */
    SDLMod mod;         /**< current key modifiers */
    Uint16 unicode;         /**< translated character */
} SDL_keysym;

/** Keyboard event structure */
typedef struct SDL1_KeyboardEvent {
	Uint8 type;	/**< SDL_KEYDOWN or SDL_KEYUP */
	Uint8 which;	/**< The keyboard device index */
	Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
	SDL_keysym keysym;
} SDL1_KeyboardEvent;

/** Mouse motion event structure */
typedef struct SDL1_MouseMotionEvent {
	Uint8 type;	/**< SDL_MOUSEMOTION */
	Uint8 which;	/**< The mouse device index */
	Uint8 state;	/**< The current button state */
	Uint16 x, y;	/**< The X/Y coordinates of the mouse */
	Sint16 xrel;	/**< The relative motion in the X direction */
	Sint16 yrel;	/**< The relative motion in the Y direction */
} SDL1_MouseMotionEvent;

/** Mouse button event structure */
typedef struct SDL1_MouseButtonEvent {
	Uint8 type;	/**< SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP */
	Uint8 which;	/**< The mouse device index */
	Uint8 button;	/**< The mouse button index */
	Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
	Uint16 x, y;	/**< The X/Y coordinates of the mouse at press time */
} SDL1_MouseButtonEvent;

/** Joystick axis motion event structure */
typedef struct SDL1_JoyAxisEvent {
	Uint8 type;	/**< SDL_JOYAXISMOTION */
	Uint8 which;	/**< The joystick device index */
	Uint8 axis;	/**< The joystick axis index */
	Sint16 value;	/**< The axis value (range: -32768 to 32767) */
} SDL1_JoyAxisEvent;

/** Joystick trackball motion event structure */
typedef struct SDL1_JoyBallEvent {
	Uint8 type;	/**< SDL_JOYBALLMOTION */
	Uint8 which;	/**< The joystick device index */
	Uint8 ball;	/**< The joystick trackball index */
	Sint16 xrel;	/**< The relative motion in the X direction */
	Sint16 yrel;	/**< The relative motion in the Y direction */
} SDL1_JoyBallEvent;

/** Joystick hat position change event structure */
typedef struct SDL1_JoyHatEvent {
	Uint8 type;	/**< SDL_JOYHATMOTION */
	Uint8 which;	/**< The joystick device index */
	Uint8 hat;	/**< The joystick hat index */
	Uint8 value;	/**< The hat position value:
			 *   SDL_HAT_LEFTUP   SDL_HAT_UP       SDL_HAT_RIGHTUP
			 *   SDL_HAT_LEFT     SDL_HAT_CENTERED SDL_HAT_RIGHT
			 *   SDL_HAT_LEFTDOWN SDL_HAT_DOWN     SDL_HAT_RIGHTDOWN
			 *  Note that zero means the POV is centered.
			 */
} SDL1_JoyHatEvent;

/** Joystick button event structure */
typedef struct SDL1_JoyButtonEvent {
	Uint8 type;	/**< SDL_JOYBUTTONDOWN or SDL_JOYBUTTONUP */
	Uint8 which;	/**< The joystick device index */
	Uint8 button;	/**< The joystick button index */
	Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
} SDL1_JoyButtonEvent;

/** The "window resized" event
 *  When you get this event, you are responsible for setting a new video
 *  mode with the new width and height.
 */
typedef struct SDL1_ResizeEvent {
	Uint8 type;	/**< SDL_VIDEORESIZE */
	int w;		/**< New width */
	int h;		/**< New height */
} SDL1_ResizeEvent;

/** The "screen redraw" event */
typedef struct SDL1_ExposeEvent {
	Uint8 type;	/**< SDL_VIDEOEXPOSE */
} SDL1_ExposeEvent;

/** The "quit requested" event */
typedef struct SDL1_QuitEvent {
	Uint8 type;	/**< SDL_QUIT */
} SDL1_QuitEvent;

/** A user-defined event type */
typedef struct SDL1_UserEvent {
	Uint8 type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;	/**< User defined event code */
	void *data1;	/**< User defined data pointer */
	void *data2;	/**< User defined data pointer */
} SDL1_UserEvent;

/** If you want to use this event, you should include SDL_syswm.h */
struct SDL1_SysWMmsg;
typedef struct SDL1_SysWMmsg SDL1_SysWMmsg;
typedef struct SDL1_SysWMEvent {
	Uint8 type;
	SDL_SysWMmsg *msg;
} SDL1_SysWMEvent;

/** General event structure */
typedef union SDL1_Event {
	Uint8 type;
	SDL1_ActiveEvent active;
	SDL1_KeyboardEvent key;
	SDL1_MouseMotionEvent motion;
	SDL1_MouseButtonEvent button;
	SDL1_JoyAxisEvent jaxis;
	SDL1_JoyBallEvent jball;
	SDL1_JoyHatEvent jhat;
	SDL1_JoyButtonEvent jbutton;
	SDL1_ResizeEvent resize;
	SDL1_ExposeEvent expose;
	SDL1_QuitEvent quit;
	SDL1_UserEvent user;
	SDL1_SysWMEvent syswm;
} SDL1_Event;


#endif /* _SDL_h */

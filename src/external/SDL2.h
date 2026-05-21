/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2025 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* Modified for libTAS to avoid conflicts with SDL3.
 * All types are in the SDL2 namespace.
 * Macros are converted to enums where possible.
 */

#ifndef _SDL2_h
#define _SDL2_h

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include "SDL_common.h"

namespace libtas {

namespace sdl2 {

    // Pixel format macros
#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
    ((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
     ((bits) << 8) | ((bytes) << 0))

    enum {

        SDL_PIXELTYPE_ARRAYU8 = 4,
        SDL_PIXELTYPE_PACKED32 = 8,

        SDL_ARRAYORDER_RGB = 1,
        SDL_ARRAYORDER_BGR = 2,

        SDL_PACKEDORDER_XRGB = 1,
        SDL_PACKEDORDER_RGBX = 2,
        SDL_PACKEDORDER_ARGB = 3,
        SDL_PACKEDORDER_RGBA = 4,
        SDL_PACKEDORDER_XBGR = 5,
        SDL_PACKEDORDER_BGRX = 6,
        SDL_PACKEDORDER_ABGR = 7,
        SDL_PACKEDORDER_BGRA = 8,
        SDL_PACKEDLAYOUT_8888 = 1,

        SDL_PIXELFORMAT_RGB24 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0, 24, 3),
        SDL_PIXELFORMAT_BGR24 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0, 24, 3),
        SDL_PIXELFORMAT_XRGB8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_8888, 24, 4),
        SDL_PIXELFORMAT_RGB888 = SDL_PIXELFORMAT_XRGB8888,
        SDL_PIXELFORMAT_RGBX8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX, SDL_PACKEDLAYOUT_8888, 24, 4),
        SDL_PIXELFORMAT_XBGR8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_8888, 24, 4),
        SDL_PIXELFORMAT_BGR888 = SDL_PIXELFORMAT_XBGR8888,
        SDL_PIXELFORMAT_BGRX8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX, SDL_PACKEDLAYOUT_8888, 24, 4),
        SDL_PIXELFORMAT_ARGB8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_8888, 32, 4),
        SDL_PIXELFORMAT_BGRA8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_8888, 32, 4),
        SDL_PIXELFORMAT_RGBA8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_8888, 32, 4),
        SDL_PIXELFORMAT_ABGR8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_8888, 32, 4),

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_RGBA8888,
        SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_BGRA8888,
        SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_ABGR8888,
#else
        SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
        SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
        SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
#endif

    };

    typedef struct SDL_Rect
    {
        int x, y;
        int w, h;
    } SDL_Rect;

    typedef struct SDL_Color
    {
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 a;
    } SDL_Color;

    typedef enum SDL_Scancode
    {
        SDL_SCANCODE_UNKNOWN = 0,
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
        SDL_SCANCODE_BACKSLASH = 49,
        SDL_SCANCODE_NONUSHASH = 50,
        SDL_SCANCODE_SEMICOLON = 51,
        SDL_SCANCODE_APOSTROPHE = 52,
        SDL_SCANCODE_GRAVE = 53,
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
        SDL_SCANCODE_INSERT = 73,
        SDL_SCANCODE_HOME = 74,
        SDL_SCANCODE_PAGEUP = 75,
        SDL_SCANCODE_DELETE = 76,
        SDL_SCANCODE_END = 77,
        SDL_SCANCODE_PAGEDOWN = 78,
        SDL_SCANCODE_RIGHT = 79,
        SDL_SCANCODE_LEFT = 80,
        SDL_SCANCODE_DOWN = 81,
        SDL_SCANCODE_UP = 82,
        SDL_SCANCODE_NUMLOCKCLEAR = 83,
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
        SDL_SCANCODE_NONUSBACKSLASH = 100,
        SDL_SCANCODE_APPLICATION = 101,
        SDL_SCANCODE_POWER = 102,
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
        SDL_SCANCODE_AGAIN = 121,
        SDL_SCANCODE_UNDO = 122,
        SDL_SCANCODE_CUT = 123,
        SDL_SCANCODE_COPY = 124,
        SDL_SCANCODE_PASTE = 125,
        SDL_SCANCODE_FIND = 126,
        SDL_SCANCODE_MUTE = 127,
        SDL_SCANCODE_VOLUMEUP = 128,
        SDL_SCANCODE_VOLUMEDOWN = 129,
        SDL_SCANCODE_KP_COMMA = 133,
        SDL_SCANCODE_KP_EQUALSAS400 = 134,
        SDL_SCANCODE_INTERNATIONAL1 = 135,
        SDL_SCANCODE_INTERNATIONAL2 = 136,
        SDL_SCANCODE_INTERNATIONAL3 = 137,
        SDL_SCANCODE_INTERNATIONAL4 = 138,
        SDL_SCANCODE_INTERNATIONAL5 = 139,
        SDL_SCANCODE_INTERNATIONAL6 = 140,
        SDL_SCANCODE_INTERNATIONAL7 = 141,
        SDL_SCANCODE_INTERNATIONAL8 = 142,
        SDL_SCANCODE_INTERNATIONAL9 = 143,
        SDL_SCANCODE_LANG1 = 144,
        SDL_SCANCODE_LANG2 = 145,
        SDL_SCANCODE_LANG3 = 146,
        SDL_SCANCODE_LANG4 = 147,
        SDL_SCANCODE_LANG5 = 148,
        SDL_SCANCODE_LANG6 = 149,
        SDL_SCANCODE_LANG7 = 150,
        SDL_SCANCODE_LANG8 = 151,
        SDL_SCANCODE_LANG9 = 152,
        SDL_SCANCODE_ALTERASE = 153,
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
        SDL_SCANCODE_LALT = 226,
        SDL_SCANCODE_LGUI = 227,
        SDL_SCANCODE_RCTRL = 228,
        SDL_SCANCODE_RSHIFT = 229,
        SDL_SCANCODE_RALT = 230,
        SDL_SCANCODE_RGUI = 231,
        SDL_SCANCODE_MODE = 257,
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
        SDL_SCANCODE_BRIGHTNESSDOWN = 275,
        SDL_SCANCODE_BRIGHTNESSUP = 276,
        SDL_SCANCODE_DISPLAYSWITCH = 277,
        SDL_SCANCODE_KBDILLUMTOGGLE = 278,
        SDL_SCANCODE_KBDILLUMDOWN = 279,
        SDL_SCANCODE_KBDILLUMUP = 280,
        SDL_SCANCODE_EJECT = 281,
        SDL_SCANCODE_SLEEP = 282,
        SDL_SCANCODE_APP1 = 283,
        SDL_SCANCODE_APP2 = 284,
        SDL_SCANCODE_AUDIOREWIND = 285,
        SDL_SCANCODE_AUDIOFASTFORWARD = 286,
        SDL_NUM_SCANCODES = 512
    } SDL_Scancode;

    typedef enum SDL_GLattr
    {
        SDL_GL_RED_SIZE,
        SDL_GL_GREEN_SIZE,
        SDL_GL_BLUE_SIZE,
        SDL_GL_ALPHA_SIZE,
        SDL_GL_BUFFER_SIZE,
        SDL_GL_DOUBLEBUFFER,
        SDL_GL_DEPTH_SIZE,
        SDL_GL_STENCIL_SIZE,
        SDL_GL_ACCUM_RED_SIZE,
        SDL_GL_ACCUM_GREEN_SIZE,
        SDL_GL_ACCUM_BLUE_SIZE,
        SDL_GL_ACCUM_ALPHA_SIZE,
        SDL_GL_STEREO,
        SDL_GL_MULTISAMPLEBUFFERS,
        SDL_GL_MULTISAMPLESAMPLES,
        SDL_GL_ACCELERATED_VISUAL,
        SDL_GL_RETAINED_BACKING,
        SDL_GL_CONTEXT_MAJOR_VERSION,
        SDL_GL_CONTEXT_MINOR_VERSION,
        SDL_GL_CONTEXT_EGL,
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
        SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
        SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
        SDL_GL_CONTEXT_RESET_NOTIFICATION,
        SDL_GL_CONTEXT_NO_ERROR
    } SDL_GLattr;

    enum {
        SDL_GL_CONTEXT_PROFILE_CORE = 0x0001,
        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY = 0x0002,
        SDL_GL_CONTEXT_PROFILE_ES = 0x0004
    };

    typedef enum SDL_BlendMode
    {
        SDL_BLENDMODE_NONE = 0x00000000,     /**< no blending
                                                  dstRGBA = srcRGBA */
        SDL_BLENDMODE_BLEND = 0x00000001,    /**< alpha blending
                                                  dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                                                  dstA = srcA + (dstA * (1-srcA)) */
        SDL_BLENDMODE_ADD = 0x00000002,      /**< additive blending
                                                  dstRGB = (srcRGB * srcA) + dstRGB
                                                  dstA = dstA */
        SDL_BLENDMODE_MOD = 0x00000004,      /**< color modulate
                                                  dstRGB = srcRGB * dstRGB
                                                  dstA = dstA */
        SDL_BLENDMODE_MUL = 0x00000008       /**< color multiply
                                                  dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                                                  dstA = (srcA * dstA) + (dstA * (1-srcA)) */
    } SDL_BlendMode;

    typedef enum SDL_TextureAccess
    {
        SDL_TEXTUREACCESS_STATIC,    /**< changes rarely, not lockable */
        SDL_TEXTUREACCESS_STREAMING, /**< changes frequently, lockable */
        SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
    } SDL_TextureAccess;

    typedef Sint32 SDL_Keycode;

    // Typedefs
    typedef int SDL_JoystickID;
    typedef Sint64 SDL_TouchID;
    typedef Sint64 SDL_FingerID;
    typedef Uint32 SDL_threadID;
    typedef Sint64 SDL_GestureID;
    typedef Uint32 SDL_AudioDeviceID;
    typedef Uint32 SDL_TimerID;

    typedef Uint32 (*SDL_TimerCallback)(Uint32 interval, void *param);

    // Audio formats
    typedef enum SDL_AudioFormat
    {
        AUDIO_U8 = 0x0008,
        AUDIO_S8 = 0x8008,
        AUDIO_U16LSB = 0x0010,
        AUDIO_S16LSB = 0x8010,
        AUDIO_U16MSB = 0x1010,
        AUDIO_S16MSB = 0x9010,
        AUDIO_U16 = AUDIO_U16LSB,
        AUDIO_S16 = AUDIO_S16LSB,
        AUDIO_S32LSB = 0x8020,
        AUDIO_S32MSB = 0x9020,
        AUDIO_S32 = AUDIO_S32LSB,
        AUDIO_F32LSB = 0x8120,
        AUDIO_F32MSB = 0x9120,
        AUDIO_F32 = AUDIO_F32LSB
    } SDL_AudioFormat;

    typedef enum SDL_AudioStatus
    {
        SDL_AUDIO_STOPPED = 0,
        SDL_AUDIO_PLAYING,
        SDL_AUDIO_PAUSED
    } SDL_AudioStatus;

    typedef void (* SDL_AudioCallback) (void *userdata, Uint8 * stream, int len);

    typedef struct SDL_AudioSpec
    {
        int freq;                   /**< DSP frequency -- samples per second */
        SDL_AudioFormat format;     /**< Audio data format */
        Uint8 channels;             /**< Number of channels: 1 mono, 2 stereo */
        Uint8 silence;              /**< Audio buffer silence value (calculated) */
        Uint16 samples;             /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
        Uint16 padding;             /**< Necessary for some compile environments */
        Uint32 size;                /**< Audio buffer size in bytes (calculated) */
        SDL_AudioCallback callback; /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
        void *userdata;             /**< Userdata passed to callback (ignored for NULL callbacks). */
    } SDL_AudioSpec;

    typedef struct SDL_Keysym
    {
        SDL_Scancode scancode;      /**< SDL physical key code - see ::SDL_Scancode for details */
        SDL_Keycode sym;            /**< SDL virtual key code - see ::SDL_Keycode for details */
        Uint16 mod;                 /**< current key modifiers */
        Uint32 unused;
    } SDL_Keysym;

    typedef enum SDL_EventType
    {
        SDL_FIRSTEVENT     = 0,     /**< Unused (do not remove) */

        /* Application events */
        SDL_QUIT           = 0x100, /**< User-requested quit */
        SDL_APP_TERMINATING,        /**< The application is being terminated by the OS */
        SDL_APP_LOWMEMORY,          /**< The application is low on memory, free memory if possible. */
        SDL_APP_WILLENTERBACKGROUND, /**< The application is about to enter the background */
        SDL_APP_DIDENTERBACKGROUND, /**< The application did enter the background and may not get CPU for some time */
        SDL_APP_WILLENTERFOREGROUND, /**< The application is about to enter the foreground */
        SDL_APP_DIDENTERFOREGROUND, /**< The application is now interactive */

        SDL_LOCALECHANGED,  /**< The user's locale preferences have changed. */

        /* Display events */
        SDL_DISPLAYEVENT   = 0x150,  /**< Display state change */

        /* Window events */
        SDL_WINDOWEVENT    = 0x200, /**< Window state change */
        SDL_SYSWMEVENT,             /**< System specific event */

        /* Keyboard events */
        SDL_KEYDOWN        = 0x300, /**< Key pressed */
        SDL_KEYUP,                  /**< Key released */
        SDL_TEXTEDITING,            /**< Keyboard text editing (composition) */
        SDL_TEXTINPUT,              /**< Keyboard text input */
        SDL_KEYMAPCHANGED,          /**< Keymap changed due to a system event such as an input language or keyboard layout change. */
        SDL_TEXTEDITING_EXT,       /**< Extended keyboard text editing (composition) */

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
        SDL_JOYBATTERYUPDATED,      /**< Joystick battery level change */

        /* Game controller events */
        SDL_CONTROLLERAXISMOTION  = 0x650, /**< Game controller axis motion */
        SDL_CONTROLLERBUTTONDOWN,          /**< Game controller button pressed */
        SDL_CONTROLLERBUTTONUP,            /**< Game controller button released */
        SDL_CONTROLLERDEVICEADDED,         /**< A new Game controller has been inserted into the system */
        SDL_CONTROLLERDEVICEREMOVED,       /**< An opened Game controller has been removed */
        SDL_CONTROLLERDEVICEREMAPPED,      /**< The controller mapping was updated */
        SDL_CONTROLLERTOUCHPADDOWN,        /**< Game controller touchpad was touched */
        SDL_CONTROLLERTOUCHPADMOTION,      /**< Game controller touchpad finger was moved */
        SDL_CONTROLLERTOUCHPADUP,          /**< Game controller touchpad finger was lifted */
        SDL_CONTROLLERSENSORUPDATE,        /**< Game controller sensor was updated */
        SDL_CONTROLLERUPDATECOMPLETE_RESERVED_FOR_SDL3,
        SDL_CONTROLLERSTEAMHANDLEUPDATED,  /**< Game controller Steam handle has changed */

        /* Touch events */
        SDL_FINGERDOWN      = 0x700,
        SDL_FINGERUP,
        SDL_FINGERMOTION,

        /* Gesture events */
        SDL_DOLLARGESTURE   = 0x800,
        SDL_DOLLARRECORD,
        SDL_MULTIGESTURE,

        /* Clipboard events */
        SDL_CLIPBOARDUPDATE = 0x900, /**< The clipboard or primary selection changed */

        /* Drag and drop events */
        SDL_DROPFILE        = 0x1000, /**< The system requests a file open */
        SDL_DROPTEXT,                 /**< text/plain drag-and-drop event */
        SDL_DROPBEGIN,                /**< A new set of drops is beginning (NULL filename) */
        SDL_DROPCOMPLETE,             /**< Current set of drops is now complete (NULL filename) */

        /* Audio hotplug events */
        SDL_AUDIODEVICEADDED = 0x1100, /**< A new audio device is available */
        SDL_AUDIODEVICEREMOVED,        /**< An audio device has been removed. */

        /* Sensor events */
        SDL_SENSORUPDATE = 0x1200,     /**< A sensor was updated */

        /* Render events */
        SDL_RENDER_TARGETS_RESET = 0x2000, /**< The render targets have been reset and their contents need to be updated */
        SDL_RENDER_DEVICE_RESET, /**< The device has been reset and all textures need to be recreated */

        /* Internal events */
        SDL_POLLSENTINEL = 0x7F00, /**< Signals the end of an event poll cycle */

        /** Events SDL_USEREVENT through SDL_LASTEVENT are for your use,
         *  and should be allocated with SDL_RegisterEvents()
         */
        SDL_USEREVENT    = 0x8000,

        /**
         *  This last event is only for bounding internal arrays
         */
        SDL_LASTEVENT    = 0xFFFF
    } SDL_EventType;

    typedef struct SDL_CommonEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    } SDL_CommonEvent;

    typedef struct SDL_DisplayEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 display;     /**< The associated display index */
        Uint8 event;        /**< ::SDL_DisplayEventID */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
        Sint32 data1;       /**< event dependent data */
    } SDL_DisplayEvent;

    typedef struct SDL_WindowEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The associated window */
        Uint8 event;        /**< ::SDL_WindowEventID */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
        Sint32 data1;       /**< event dependent data */
        Sint32 data2;       /**< event dependent data */
    } SDL_WindowEvent;

    typedef enum SDL_WindowEventID
    {
        SDL_WINDOWEVENT_NONE,           /**< Never used */
        SDL_WINDOWEVENT_SHOWN,          /**< Window has been shown */
        SDL_WINDOWEVENT_HIDDEN,         /**< Window has been hidden */
        SDL_WINDOWEVENT_EXPOSED,        /**< Window has been exposed and should be redrawn */
        SDL_WINDOWEVENT_MOVED,          /**< Window has been moved to data1, data2 */
        SDL_WINDOWEVENT_RESIZED,        /**< Window has been resized to data1xdata2 */
        SDL_WINDOWEVENT_SIZE_CHANGED,   /**< The window size has changed, either as a result of an API call or through the system or user changing the window size. */
        SDL_WINDOWEVENT_MINIMIZED,      /**< Window has been minimized */
        SDL_WINDOWEVENT_MAXIMIZED,      /**< Window has been maximized */
        SDL_WINDOWEVENT_RESTORED,       /**< Window has been restored to normal size and position */
        SDL_WINDOWEVENT_ENTER,          /**< Window has gained mouse focus */
        SDL_WINDOWEVENT_LEAVE,          /**< Window has lost mouse focus */
        SDL_WINDOWEVENT_FOCUS_GAINED,   /**< Window has gained keyboard focus */
        SDL_WINDOWEVENT_FOCUS_LOST,     /**< Window has lost keyboard focus */
        SDL_WINDOWEVENT_CLOSE,          /**< The window manager requests that the window be closed */
        SDL_WINDOWEVENT_TAKE_FOCUS,     /**< Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) */
        SDL_WINDOWEVENT_HIT_TEST        /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL. */
    } SDL_WindowEventID;

    typedef struct SDL_KeyboardEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with keyboard focus, if any */
        Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
        Uint8 repeat;       /**< Non-zero if this is a key repeat */
        Uint8 padding2;
        Uint8 padding3;
        SDL_Keysym keysym;  /**< The key that was pressed or released */
    } SDL_KeyboardEvent;

    typedef struct SDL_TextEditingEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with keyboard focus, if any */
        char text[32];      /**< The editing text */
        Sint32 start;       /**< The start cursor of selected editing text */
        Sint32 length;      /**< The length of selected editing text */
    } SDL_TextEditingEvent;

    typedef struct SDL_TextEditingExtEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with keyboard focus, if any */
        char* text;         /**< The editing text, which should be freed with SDL_free(), and will not be NULL */
        Sint32 start;       /**< The start cursor of selected editing text */
        Sint32 length;      /**< The length of selected editing text */
    } SDL_TextEditingExtEvent;

    typedef struct SDL_TextInputEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with keyboard focus, if any */
        char text[32];      /**< The input text */
    } SDL_TextInputEvent;

    typedef struct SDL_MouseMotionEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with mouse focus, if any */
        Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
        Uint32 state;       /**< The current button state */
        Sint32 x;           /**< X coordinate, relative to window */
        Sint32 y;           /**< Y coordinate, relative to window */
        Sint32 xrel;        /**< The relative motion in the X direction */
        Sint32 yrel;        /**< The relative motion in the Y direction */
    } SDL_MouseMotionEvent;

    typedef struct SDL_MouseButtonEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with mouse focus, if any */
        Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
        Uint8 button;       /**< The mouse button index */
        Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
        Uint8 clicks;       /**< 1 for single-click, 2 for double-click, etc. */
        Uint8 padding1;
        Sint32 x;           /**< X coordinate, relative to window */
        Sint32 y;           /**< Y coordinate, relative to window */
    } SDL_MouseButtonEvent;

    typedef struct SDL_MouseWheelEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The window with mouse focus, if any */
        Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
        Sint32 x;           /**< The amount scrolled horizontally, positive to the right */
        Sint32 y;           /**< The amount scrolled vertically, positive away from the user */
        Uint32 direction;   /**< Set to one of the SDL_MOUSEWHEEL_* defines. When FLIPPED the values in X and Y will be opposite. Multiply by -1 to change them back */
        float preciseX;     /**< The amount scrolled horizontally, positive to the right */
        float preciseY;     /**< The amount scrolled vertically, positive away from the user */
    } SDL_MouseWheelEvent;

    typedef struct SDL_JoyAxisEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 axis;         /**< The joystick axis index */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
        Sint16 value;       /**< The axis value (range: -32768 to 32767) */
        Uint16 padding4;
    } SDL_JoyAxisEvent;

    typedef struct SDL_JoyBallEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 ball;         /**< The joystick trackball index */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
        Sint16 xrel;        /**< The relative motion in the X direction */
        Sint16 yrel;        /**< The relative motion in the Y direction */
    } SDL_JoyBallEvent;

    typedef struct SDL_JoyHatEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 hat;          /**< The joystick hat index */
        Uint8 value;        /**< The hat position value:
                             *   SDL_HAT_LEFTUP   SDL_HAT_UP       SDL_HAT_RIGHTUP
                             *   SDL_HAT_LEFT     SDL_HAT_CENTERED SDL_HAT_RIGHT
                             *   SDL_HAT_LEFTDOWN SDL_HAT_DOWN     SDL_HAT_RIGHTDOWN
                             *
                             *   Note that zero means the POV is centered.
                             */
        Uint8 padding1;
        Uint8 padding2;
    } SDL_JoyHatEvent;

    typedef struct SDL_JoyButtonEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 button;       /**< The joystick button index */
        Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
        Uint8 padding1;
        Uint8 padding2;
    } SDL_JoyButtonEvent;

    typedef struct SDL_JoyDeviceEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Sint32 which;       /**< The joystick device index for the ADDED event, instance id for the REMOVED event */
    } SDL_JoyDeviceEvent;

    typedef enum SDL_JoystickPowerLevel
    {
        SDL_JOYSTICK_POWER_UNKNOWN = -1,
        SDL_JOYSTICK_POWER_EMPTY,   /* <= 5% */
        SDL_JOYSTICK_POWER_LOW,     /* <= 20% */
        SDL_JOYSTICK_POWER_MEDIUM,  /* <= 70% */
        SDL_JOYSTICK_POWER_FULL,    /* <= 100% */
        SDL_JOYSTICK_POWER_WIRED,
        SDL_JOYSTICK_POWER_MAX
    } SDL_JoystickPowerLevel;

    typedef struct SDL_JoyBatteryEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        SDL_JoystickPowerLevel level; /**< The joystick battery level */
    } SDL_JoyBatteryEvent;

    typedef struct SDL_ControllerAxisEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 axis;         /**< The controller axis (SDL_GameControllerAxis) */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
        Sint16 value;       /**< The axis value (range: -32768 to 32767) */
        Uint16 padding4;
    } SDL_ControllerAxisEvent;

    typedef struct SDL_ControllerButtonEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Uint8 button;       /**< The controller button (SDL_GameControllerButton) */
        Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
        Uint8 padding1;
        Uint8 padding2;
    } SDL_ControllerButtonEvent;

    typedef struct SDL_ControllerDeviceEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Sint32 which;       /**< The joystick device index for the ADDED event, instance id for the REMOVED or REMAPPED event */
    } SDL_ControllerDeviceEvent;

    typedef struct SDL_ControllerTouchpadEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Sint32 touchpad;    /**< The index of the touchpad */
        Sint32 finger;      /**< The index of the finger on the touchpad */
        float x;            /**< Normalized in the range 0...1 with 0 being on the left */
        float y;            /**< Normalized in the range 0...1 with 0 being at the top */
        float pressure;     /**< Normalized in the range 0...1 */
    } SDL_ControllerTouchpadEvent;

    typedef struct SDL_ControllerSensorEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_JoystickID which; /**< The joystick instance id */
        Sint32 sensor;      /**< The type of the sensor, one of the values of ::SDL_SensorType */
        float data[3];      /**< Up to 3 values from the sensor - additional data will be ignored */
    } SDL_ControllerSensorEvent;

    typedef struct SDL_AudioDeviceEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 which;       /**< The audio device index for the ADDED event (valid until next SDL_GetNumAudioDevices() call), SDL_AudioDeviceID for the REMOVED event */
        Uint8 iscapture;    /**< zero if an output device, non-zero if a capture device. */
        Uint8 padding1;
        Uint8 padding2;
        Uint8 padding3;
    } SDL_AudioDeviceEvent;

    typedef struct SDL_TouchFingerEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_TouchID touchId; /**< The touch device id */
        SDL_FingerID fingerId;
        float x;            /**< Normalized in the range 0...1 */
        float y;            /**< Normalized in the range 0...1 */
        float dx;           /**< Normalized in the range -1...1 */
        float dy;           /**< Normalized in the range -1...1 */
        float pressure;     /**< Normalized in the range 0...1 */
        Uint32 windowID;    /**< The window underneath the finger, if any */
    } SDL_TouchFingerEvent;

    typedef struct SDL_MultiGestureEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_TouchID touchId; /**< The touch device id */
        float dTheta;
        float dDist;
        float x;
        float y;
        Uint16 numFingers;
        Uint16 padding;
    } SDL_MultiGestureEvent;

    typedef struct SDL_DollarGestureEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_TouchID touchId; /**< The touch device id */
        SDL_GestureID gestureId;
        Uint32 numFingers;
        float error;
        float x;            /**< Normalized center of gesture */
        float y;            /**< Normalized center of gesture */
    } SDL_DollarGestureEvent;

    typedef struct SDL_DropEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        char *file;         /**< The file name, which should be freed with SDL_free(), is NULL on begin/complete */
        Uint32 windowID;    /**< The window that was dropped on, if any */
    } SDL_DropEvent;

    typedef struct SDL_SensorEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Sint32 which;       /**< The instance ID of the sensor */
        float data[6];      /**< Up to 6 values from the sensor - additional data will be ignored */
    } SDL_SensorEvent;

    typedef struct SDL_QuitEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    } SDL_QuitEvent;

    typedef struct SDL_OSEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    } SDL_OSEvent;

    typedef struct SDL_UserEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        Uint32 windowID;    /**< The associated window if any */
        Sint32 code;        /**< User defined event code */
        void *data1;        /**< User defined data pointer */
        void *data2;        /**< User defined data pointer */
    } SDL_UserEvent;

    typedef struct SDL_SysWMmsg SDL_SysWMmsg;

    typedef struct SDL_SysWMEvent
    {
        Uint32 type;
        Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
        SDL_SysWMmsg *msg;  /**< driver dependent data, defined in SDL_syswm.h */
    } SDL_SysWMEvent;

    typedef union SDL_Event
    {
        Uint32 type;                            /**< Event type, shared with all events */
        SDL_CommonEvent common;                 /**< Common event data */
        SDL_DisplayEvent display;               /**< Display event data */
        SDL_WindowEvent window;                 /**< Window event data */
        SDL_KeyboardEvent key;                  /**< Keyboard event data */
        SDL_TextEditingEvent edit;              /**< Text editing event data */
        SDL_TextEditingExtEvent editExt;        /**< Extended text editing event data */
        SDL_TextInputEvent text;                /**< Text input event data */
        SDL_MouseMotionEvent motion;            /**< Mouse motion event data */
        SDL_MouseButtonEvent button;            /**< Mouse button event data */
        SDL_MouseWheelEvent wheel;              /**< Mouse wheel event data */
        SDL_JoyAxisEvent jaxis;                 /**< Joystick axis event data */
        SDL_JoyBallEvent jball;                 /**< Joystick ball event data */
        SDL_JoyHatEvent jhat;                   /**< Joystick hat event data */
        SDL_JoyButtonEvent jbutton;             /**< Joystick button event data */
        SDL_JoyDeviceEvent jdevice;             /**< Joystick device change event data */
        SDL_JoyBatteryEvent jbattery;           /**< Joystick battery event data */
        SDL_ControllerAxisEvent caxis;          /**< Game Controller axis event data */
        SDL_ControllerButtonEvent cbutton;      /**< Game Controller button event data */
        SDL_ControllerDeviceEvent cdevice;      /**< Game Controller device event data */
        SDL_ControllerTouchpadEvent ctouchpad;  /**< Game Controller touchpad event data */
        SDL_ControllerSensorEvent csensor;      /**< Game Controller sensor event data */
        SDL_AudioDeviceEvent adevice;           /**< Audio device event data */
        SDL_SensorEvent sensor;                 /**< Sensor event data */
        SDL_QuitEvent quit;                     /**< Quit request event data */
        SDL_UserEvent user;                     /**< Custom event data */
        SDL_SysWMEvent syswm;                   /**< System dependent window event data */
        SDL_TouchFingerEvent tfinger;           /**< Touch finger event data */
        SDL_MultiGestureEvent mgesture;         /**< Gesture event data */
        SDL_DollarGestureEvent dgesture;        /**< Gesture event data */
        SDL_DropEvent drop;                     /**< Drag and drop event data */

        /* This is necessary for ABI compatibility between Visual C++ and GCC.
           Visual C++ will respect the push pack pragma and use 52 bytes (size of
           SDL_TextEditingEvent, the largest structure for 32-bit and 64-bit
           architectures) for this union, and GCC will use the alignment of the
           largest datatype within the union, which is 8 bytes on 64-bit
           architectures.

           So... we'll add padding to force the size to be 56 bytes for both.

           On architectures where pointers are 16 bytes, this needs rounding up to
           the next multiple of 16, 64, and on architectures where pointers are
           even larger the size of SDL_UserEvent will dominate as being 3 pointers.
        */
        Uint8 padding[sizeof(void *) <= 8 ? 56 : sizeof(void *) == 16 ? 64 : 3 * sizeof(void *)];
    } SDL_Event;

    typedef int SDL_errorcode;
    typedef int SDL_HintPriority;
    typedef void (*SDL_HintCallback)(void *userdata, const char *name, const char *oldValue, const char *newValue);
    typedef int SDL_LogPriority;
    typedef int SDL_PowerState;
    typedef void *SDL_GLContext;
    typedef struct SDL_AssertData SDL_AssertData;
    typedef int SDL_AssertState;
    typedef SDL_AssertState (*SDL_AssertionHandler)(const SDL_AssertData *data, void *userdata);
    typedef void (*SDL_LogOutputFunction)(void *userdata, int category, SDL_LogPriority priority, const char *message);
    typedef int SDL_HitTest;
    typedef int SDL_SensorID;
    typedef int SDL_TLSID;
    typedef enum SDL_DisplayOrientation
    {
        SDL_ORIENTATION_UNKNOWN = 0,
        SDL_ORIENTATION_LANDSCAPE,
        SDL_ORIENTATION_LANDSCAPE_FLIPPED,
        SDL_ORIENTATION_PORTRAIT,
        SDL_ORIENTATION_PORTRAIT_FLIPPED
    } SDL_DisplayOrientation;
    typedef enum SDL_TouchDeviceType
    {
        SDL_TOUCH_DEVICE_INVALID = 0,
        SDL_TOUCH_DEVICE_DIRECT,
        SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE,
        SDL_TOUCH_DEVICE_INDIRECT_RELATIVE
    } SDL_TouchDeviceType;
    typedef int SDL_ThreadPriority;
    typedef struct SDL_WinRT_Path SDL_WinRT_Path;
    typedef void (*SDL_WindowsMessageHook)(void *userdata, void *window, void *msg, void *wParam, void *lParam);
    typedef int SDL_YUV_CONVERSION_MODE;
    typedef void *SDL_iconv_t;
    typedef int (*SDL_main_func)(int argc, char *argv[]);
    typedef int SDL_SpinLock;

    #define SDL_assert_state SDL_AssertState
    #define SDL_assert_data SDL_AssertData

    typedef enum SDL_BlendFactor
    {
        SDL_BLENDFACTOR_ZERO,
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDFACTOR_SRC_COLOR,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
        SDL_BLENDFACTOR_SRC_ALPHA,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDFACTOR_DST_COLOR,
        SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR,
        SDL_BLENDFACTOR_DST_ALPHA,
        SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
        SDL_BLENDFACTOR_SRC_ALPHA_SATURATE
    } SDL_BlendFactor;

    typedef enum SDL_BlendOperation
    {
        SDL_BLENDOPERATION_ADD,
        SDL_BLENDOPERATION_SUBTRACT,
        SDL_BLENDOPERATION_REV_SUBTRACT,
        SDL_BLENDOPERATION_MINIMUM,
        SDL_BLENDOPERATION_MAXIMUM
    } SDL_BlendOperation;

    typedef enum SDL_FlashOperation
    {
        SDL_FLASH_BRIEFLY = 1,
        SDL_FLASH_UNTIL_FOCUSED = 2
    } SDL_FlashOperation;

    typedef enum SDL_RendererFlip
    {
        SDL_FLIP_NONE = 0x00000000,
        SDL_FLIP_HORIZONTAL = 0x00000001,
        SDL_FLIP_VERTICAL = 0x00000002
    } SDL_RendererFlip;

    typedef struct SDL_FPoint
    {
        float x;
        float y;
    } SDL_FPoint;

    typedef struct SDL_FRect
    {
        float x;
        float y;
        float w;
        float h;
    } SDL_FRect;

    typedef void *(*SDL_malloc_func)(size_t a);
    typedef void *(*SDL_calloc_func)(size_t a, size_t b);
    typedef void *(*SDL_realloc_func)(void *a, size_t b);
    typedef void (*SDL_free_func)(void *a);

    typedef struct VkInstance_T VkInstance_T;
    typedef struct VkSurfaceKHR_T VkSurfaceKHR_T;
    typedef VkInstance_T *VkInstance;
    typedef VkSurfaceKHR_T *VkSurfaceKHR;

    typedef struct SDL_Surface
    {
        Uint32 flags;               /**< Read-only */
        SDL_PixelFormat *format;    /**< Read-only */
        int w, h;                   /**< Read-only */
        int pitch;                  /**< Read-only */
        void *pixels;               /**< Read-write */

        void *userdata;             /**< Read-write */
        int locked;                 /**< Read-only */
        void *list_blitmap;         /**< Private */
        SDL_Rect clip_rect;         /**< Read-only */
        SDL_BlitMap *map;           /**< Private */
        int refcount;               /**< Read-mostly */
    } SDL_Surface;

    #define SDL_RLEACCEL 0x00000002
    #define SDL_MUSTLOCK(surface) ((surface)->flags & SDL_RLEACCEL)

    // Function pointer types
    typedef int (*SDL_ThreadFunction)(void *data);
    typedef Uint32 (*SDL_NewTimerCallback)(Uint32 interval, void *param);
    typedef int (*SDL_EventFilter)(void *userdata, SDL_Event *event);

    // Enums
    typedef enum SDL_eventaction
    {
        SDL_ADDEVENT,
        SDL_PEEKEVENT,
        SDL_GETEVENT
    } SDL_eventaction;



    // Keycode constants
    enum {
        SDLK_UNKNOWN = 0,
        SDLK_RETURN = '\r',
        SDLK_ESCAPE = '\x1B',
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
        SDLK_CAPSLOCK = (127 | (1<<30)),
        SDLK_F1 = (58 | (1<<30)),
        SDLK_F2 = (59 | (1<<30)),
        SDLK_F3 = (60 | (1<<30)),
        SDLK_F4 = (61 | (1<<30)),
        SDLK_F5 = (62 | (1<<30)),
        SDLK_F6 = (63 | (1<<30)),
        SDLK_F7 = (64 | (1<<30)),
        SDLK_F8 = (65 | (1<<30)),
        SDLK_F9 = (66 | (1<<30)),
        SDLK_F10 = (67 | (1<<30)),
        SDLK_F11 = (68 | (1<<30)),
        SDLK_F12 = (69 | (1<<30)),
        SDLK_PRINTSCREEN = (70 | (1<<30)),
        SDLK_SCROLLLOCK = (71 | (1<<30)),
        SDLK_PAUSE = (72 | (1<<30)),
        SDLK_INSERT = (73 | (1<<30)),
        SDLK_HOME = (74 | (1<<30)),
        SDLK_PAGEUP = (75 | (1<<30)),
        SDLK_DELETE = '\x7F',
        SDLK_END = (77 | (1<<30)),
        SDLK_PAGEDOWN = (78 | (1<<30)),
        SDLK_RIGHT = (79 | (1<<30)),
        SDLK_LEFT = (80 | (1<<30)),
        SDLK_DOWN = (81 | (1<<30)),
        SDLK_UP = (82 | (1<<30)),
        SDLK_NUMLOCKCLEAR = (83 | (1<<30)),
        SDLK_KP_DIVIDE = (84 | (1<<30)),
        SDLK_KP_MULTIPLY = (85 | (1<<30)),
        SDLK_KP_MINUS = (86 | (1<<30)),
        SDLK_KP_PLUS = (87 | (1<<30)),
        SDLK_KP_ENTER = (88 | (1<<30)),
        SDLK_KP_1 = (89 | (1<<30)),
        SDLK_KP_2 = (90 | (1<<30)),
        SDLK_KP_3 = (91 | (1<<30)),
        SDLK_KP_4 = (92 | (1<<30)),
        SDLK_KP_5 = (93 | (1<<30)),
        SDLK_KP_6 = (94 | (1<<30)),
        SDLK_KP_7 = (95 | (1<<30)),
        SDLK_KP_8 = (96 | (1<<30)),
        SDLK_KP_9 = (97 | (1<<30)),
        SDLK_KP_0 = (98 | (1<<30)),
        SDLK_KP_PERIOD = (99 | (1<<30)),
        SDLK_APPLICATION = (101 | (1<<30)),
        SDLK_POWER = (102 | (1<<30)),
        SDLK_KP_EQUALS = (103 | (1<<30)),
        SDLK_F13 = (104 | (1<<30)),
        SDLK_F14 = (105 | (1<<30)),
        SDLK_F15 = (106 | (1<<30)),
        SDLK_F16 = (107 | (1<<30)),
        SDLK_F17 = (108 | (1<<30)),
        SDLK_F18 = (109 | (1<<30)),
        SDLK_F19 = (110 | (1<<30)),
        SDLK_F20 = (111 | (1<<30)),
        SDLK_F21 = (112 | (1<<30)),
        SDLK_F22 = (113 | (1<<30)),
        SDLK_F23 = (114 | (1<<30)),
        SDLK_F24 = (115 | (1<<30)),
        SDLK_EXECUTE = (116 | (1<<30)),
        SDLK_HELP = (117 | (1<<30)),
        SDLK_MENU = (118 | (1<<30)),
        SDLK_SELECT = (119 | (1<<30)),
        SDLK_STOP = (120 | (1<<30)),
        SDLK_AGAIN = (121 | (1<<30)),
        SDLK_UNDO = (122 | (1<<30)),
        SDLK_CUT = (123 | (1<<30)),
        SDLK_COPY = (124 | (1<<30)),
        SDLK_PASTE = (125 | (1<<30)),
        SDLK_FIND = (126 | (1<<30)),
        SDLK_MUTE = (127 | (1<<30)),
        SDLK_VOLUMEUP = (128 | (1<<30)),
        SDLK_VOLUMEDOWN = (129 | (1<<30)),
        SDLK_KP_COMMA = (133 | (1<<30)),
        SDLK_KP_EQUALSAS400 = (134 | (1<<30)),
        SDLK_ALTERASE = (153 | (1<<30)),
        SDLK_SYSREQ = (154 | (1<<30)),
        SDLK_CANCEL = (155 | (1<<30)),
        SDLK_CLEAR = (156 | (1<<30)),
        SDLK_PRIOR = (157 | (1<<30)),
        SDLK_RETURN2 = (158 | (1<<30)),
        SDLK_SEPARATOR = (159 | (1<<30)),
        SDLK_OUT = (160 | (1<<30)),
        SDLK_OPER = (161 | (1<<30)),
        SDLK_CLEARAGAIN = (162 | (1<<30)),
        SDLK_CRSEL = (163 | (1<<30)),
        SDLK_EXSEL = (164 | (1<<30)),
        SDLK_KP_00 = (176 | (1<<30)),
        SDLK_KP_000 = (177 | (1<<30)),
        SDLK_THOUSANDSSEPARATOR = (178 | (1<<30)),
        SDLK_DECIMALSEPARATOR = (179 | (1<<30)),
        SDLK_CURRENCYUNIT = (180 | (1<<30)),
        SDLK_CURRENCYSUBUNIT = (181 | (1<<30)),
        SDLK_KP_LEFTPAREN = (182 | (1<<30)),
        SDLK_KP_RIGHTPAREN = (183 | (1<<30)),
        SDLK_KP_LEFTBRACE = (184 | (1<<30)),
        SDLK_KP_RIGHTBRACE = (185 | (1<<30)),
        SDLK_KP_TAB = (186 | (1<<30)),
        SDLK_KP_BACKSPACE = (187 | (1<<30)),
        SDLK_KP_A = (188 | (1<<30)),
        SDLK_KP_B = (189 | (1<<30)),
        SDLK_KP_C = (190 | (1<<30)),
        SDLK_KP_D = (191 | (1<<30)),
        SDLK_KP_E = (192 | (1<<30)),
        SDLK_KP_F = (193 | (1<<30)),
        SDLK_KP_XOR = (194 | (1<<30)),
        SDLK_KP_POWER = (195 | (1<<30)),
        SDLK_KP_PERCENT = (196 | (1<<30)),
        SDLK_KP_LESS = (197 | (1<<30)),
        SDLK_KP_GREATER = (198 | (1<<30)),
        SDLK_KP_AMPERSAND = (199 | (1<<30)),
        SDLK_KP_DBLAMPERSAND = (200 | (1<<30)),
        SDLK_KP_VERTICALBAR = (201 | (1<<30)),
        SDLK_KP_DBLVERTICALBAR = (202 | (1<<30)),
        SDLK_KP_COLON = (203 | (1<<30)),
        SDLK_KP_HASH = (204 | (1<<30)),
        SDLK_KP_SPACE = (205 | (1<<30)),
        SDLK_KP_AT = (206 | (1<<30)),
        SDLK_KP_EXCLAM = (207 | (1<<30)),
        SDLK_KP_MEMSTORE = (208 | (1<<30)),
        SDLK_KP_MEMRECALL = (209 | (1<<30)),
        SDLK_KP_MEMCLEAR = (210 | (1<<30)),
        SDLK_KP_MEMADD = (211 | (1<<30)),
        SDLK_KP_MEMSUBTRACT = (212 | (1<<30)),
        SDLK_KP_MEMMULTIPLY = (213 | (1<<30)),
        SDLK_KP_MEMDIVIDE = (214 | (1<<30)),
        SDLK_KP_PLUSMINUS = (215 | (1<<30)),
        SDLK_KP_CLEAR = (216 | (1<<30)),
        SDLK_KP_CLEARENTRY = (217 | (1<<30)),
        SDLK_KP_BINARY = (218 | (1<<30)),
        SDLK_KP_OCTAL = (219 | (1<<30)),
        SDLK_KP_DECIMAL = (220 | (1<<30)),
        SDLK_KP_HEXADECIMAL = (221 | (1<<30)),
        SDLK_LCTRL = (224 | (1<<30)),
        SDLK_LSHIFT = (225 | (1<<30)),
        SDLK_LALT = (226 | (1<<30)),
        SDLK_LGUI = (227 | (1<<30)),
        SDLK_RCTRL = (228 | (1<<30)),
        SDLK_RSHIFT = (229 | (1<<30)),
        SDLK_RALT = (230 | (1<<30)),
        SDLK_RGUI = (231 | (1<<30)),
        SDLK_MODE = (257 | (1<<30)),
        SDLK_AUDIONEXT = (258 | (1<<30)),
        SDLK_AUDIOPREV = (259 | (1<<30)),
        SDLK_AUDIOSTOP = (260 | (1<<30)),
        SDLK_AUDIOPLAY = (261 | (1<<30)),
        SDLK_AUDIOMUTE = (262 | (1<<30)),
        SDLK_MEDIASELECT = (263 | (1<<30)),
        SDLK_WWW = (264 | (1<<30)),
        SDLK_MAIL = (265 | (1<<30)),
        SDLK_CALCULATOR = (266 | (1<<30)),
        SDLK_COMPUTER = (267 | (1<<30)),
        SDLK_AC_SEARCH = (268 | (1<<30)),
        SDLK_AC_HOME = (269 | (1<<30)),
        SDLK_AC_BACK = (270 | (1<<30)),
        SDLK_AC_FORWARD = (271 | (1<<30)),
        SDLK_AC_STOP = (272 | (1<<30)),
        SDLK_AC_REFRESH = (273 | (1<<30)),
        SDLK_AC_BOOKMARKS = (274 | (1<<30)),
        SDLK_BRIGHTNESSDOWN = (275 | (1<<30)),
        SDLK_BRIGHTNESSUP = (276 | (1<<30)),
        SDLK_DISPLAYSWITCH = (277 | (1<<30)),
        SDLK_KBDILLUMTOGGLE = (278 | (1<<30)),
        SDLK_KBDILLUMDOWN = (279 | (1<<30)),
        SDLK_KBDILLUMUP = (280 | (1<<30)),
        SDLK_EJECT = (281 | (1<<30)),
        SDLK_SLEEP = (282 | (1<<30)),
        SDLK_APP1 = (283 | (1<<30)),
        SDLK_APP2 = (284 | (1<<30)),
        SDLK_AUDIOREWIND = (285 | (1<<30)),
        SDLK_AUDIOFASTFORWARD = (286 | (1<<30))
    };

    typedef Uint16 SDL_Keymod;

    // Keymod constants
    enum {
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
        KMOD_SCROLL = 0x8000,
        KMOD_CTRL = KMOD_LCTRL | KMOD_RCTRL,
        KMOD_SHIFT = KMOD_LSHIFT | KMOD_RSHIFT,
        KMOD_ALT = KMOD_LALT | KMOD_RALT,
        KMOD_GUI = KMOD_LGUI | KMOD_RGUI,
        KMOD_RESERVED = KMOD_SCROLL
    };

    // Constants
    enum {
        SDL_PRESSED = 1,
        SDL_RELEASED = 0
    };

    // Window position constants
#define SDL_WINDOWPOS_CENTERED    0x2FFF0000u
#define SDL_WINDOWPOS_UNDEFINED   0x1FFF0000u

    // Initialization flags
    enum {
        SDL_INIT_VIDEO = 0x00000020u,
        SDL_INIT_AUDIO = 0x00000010u,
        SDL_INIT_TIMER = 0x00000001u,
        SDL_INIT_HAPTIC = 0x00001000u,
        SDL_INIT_EVENTS = 0x00004000u,
        SDL_INIT_JOYSTICK = 0x00000200u,
        SDL_INIT_GAMECONTROLLER = 0x00002000u
    };

    // Window flags
    enum {
        SDL_WINDOW_SHOWN = 0x00000004,
        SDL_WINDOW_HIDDEN = 0x00000008,
        SDL_WINDOW_FULLSCREEN = 0x00000001,
        SDL_WINDOW_RESIZABLE = 0x00000020,
        SDL_WINDOW_BORDERLESS = 0x00000010,
        SDL_WINDOW_OPENGL = 0x00000002,
        SDL_WINDOW_MINIMIZED = 0x00000040,
        SDL_WINDOW_INPUT_FOCUS = 0x00000200,
        SDL_WINDOW_MOUSE_FOCUS = 0x00000400,
        SDL_WINDOW_FULLSCREEN_DESKTOP = (SDL_WINDOW_FULLSCREEN | 0x00001000),
        SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000
    };

    // Renderer flags
    enum {
        SDL_RENDERER_SOFTWARE = 0x00000001,
        SDL_RENDERER_ACCELERATED = 0x00000002,
        SDL_RENDERER_PRESENTVSYNC = 0x00000004,
        SDL_RENDERER_TARGETTEXTURE = 0x00000008
    };

    // Mutex constants
    enum {
        SDL_MUTEX_TIMEDOUT = 1
    };

    // Hat positions
    enum {
        SDL_HAT_CENTERED = 0x00,
        SDL_HAT_UP = 0x01,
        SDL_HAT_RIGHT = 0x02,
        SDL_HAT_DOWN = 0x04,
        SDL_HAT_LEFT = 0x08,
        SDL_HAT_RIGHTUP = SDL_HAT_RIGHT | SDL_HAT_UP,
        SDL_HAT_RIGHTDOWN = SDL_HAT_RIGHT | SDL_HAT_DOWN,
        SDL_HAT_LEFTUP = SDL_HAT_LEFT | SDL_HAT_UP,
        SDL_HAT_LEFTDOWN = SDL_HAT_LEFT | SDL_HAT_DOWN
    };

    // Mouse wheel directions
    enum {
        SDL_MOUSEWHEEL_NORMAL,
        SDL_MOUSEWHEEL_FLIPPED
    };

    // Joystick power levels

    typedef enum SDL_GameControllerAxis
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

    // Game controller buttons
    typedef enum SDL_GameControllerButton
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
        SDL_CONTROLLER_BUTTON_MISC1,
        SDL_CONTROLLER_BUTTON_PADDLE1,
        SDL_CONTROLLER_BUTTON_PADDLE2,
        SDL_CONTROLLER_BUTTON_PADDLE3,
        SDL_CONTROLLER_BUTTON_PADDLE4,
        SDL_CONTROLLER_BUTTON_TOUCHPAD,
        SDL_CONTROLLER_BUTTON_MAX
    } SDL_GameControllerButton;

    // Sensor types
    typedef enum SDL_SensorType
    {
        SDL_SENSOR_INVALID = -1,
        SDL_SENSOR_UNKNOWN,
        SDL_SENSOR_ACCEL,
        SDL_SENSOR_GYRO,
        SDL_SENSOR_ACCEL_L,
        SDL_SENSOR_GYRO_L,
        SDL_SENSOR_ACCEL_R,
        SDL_SENSOR_GYRO_R
    } SDL_SensorType;

    // System cursor types
    typedef enum SDL_SystemCursor
    {
        SDL_SYSTEM_CURSOR_ARROW,
        SDL_SYSTEM_CURSOR_IBEAM,
        SDL_SYSTEM_CURSOR_WAIT,
        SDL_SYSTEM_CURSOR_CROSSHAIR,
        SDL_SYSTEM_CURSOR_WAITARROW,
        SDL_SYSTEM_CURSOR_SIZENWSE,
        SDL_SYSTEM_CURSOR_SIZENESW,
        SDL_SYSTEM_CURSOR_SIZEWE,
        SDL_SYSTEM_CURSOR_SIZENS,
        SDL_SYSTEM_CURSOR_SIZEALL,
        SDL_SYSTEM_CURSOR_NO,
        SDL_SYSTEM_CURSOR_HAND,
        SDL_NUM_SYSTEM_CURSORS
    } SDL_SystemCursor;

    // Joystick types
    typedef enum SDL_JoystickType
    {
        SDL_JOYSTICK_TYPE_UNKNOWN,
        SDL_JOYSTICK_TYPE_GAMECONTROLLER,
        SDL_JOYSTICK_TYPE_WHEEL,
        SDL_JOYSTICK_TYPE_ARCADE_STICK,
        SDL_JOYSTICK_TYPE_FLIGHT_STICK,
        SDL_JOYSTICK_TYPE_DANCE_PAD,
        SDL_JOYSTICK_TYPE_GUITAR,
        SDL_JOYSTICK_TYPE_DRUM_KIT,
        SDL_JOYSTICK_TYPE_ARCADE_PAD,
        SDL_JOYSTICK_TYPE_THROTTLE
    } SDL_JoystickType;

    // Game controller types
    typedef enum SDL_GameControllerType
    {
        SDL_CONTROLLER_TYPE_UNKNOWN = 0,
        SDL_CONTROLLER_TYPE_XBOX360,
        SDL_CONTROLLER_TYPE_XBOXONE,
        SDL_CONTROLLER_TYPE_PS3,
        SDL_CONTROLLER_TYPE_PS4,
        SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO,
        SDL_CONTROLLER_TYPE_VIRTUAL,
        SDL_CONTROLLER_TYPE_PS5,
        SDL_CONTROLLER_TYPE_AMAZON_LUNA,
        SDL_CONTROLLER_TYPE_GOOGLE_STADIA,
        SDL_CONTROLLER_TYPE_NVIDIA_SHIELD,
        SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_L,
        SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_R,
        SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
    } SDL_GameControllerType;

    // Scale modes
    typedef enum SDL_ScaleMode
    {
        SDL_ScaleModeNearest,
        SDL_ScaleModeLinear,
        SDL_ScaleModeBest
    } SDL_ScaleMode;

    // Audio allow change flags
    enum {
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE = 0x00000001,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE = 0x00000002,
        SDL_AUDIO_ALLOW_CHANNELS_CHANGE = 0x00000004,
        SDL_AUDIO_ALLOW_SAMPLES_CHANGE = 0x00000008,
        SDL_AUDIO_ALLOW_ANY_CHANGE = (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_CHANNELS_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE)
    };

    // Display mode
    typedef struct SDL_DisplayMode
    {
        Uint32 format;              /**< pixel format */
        int w;                      /**< width, in screen coordinates */
        int h;                      /**< height, in screen coordinates */
        int refresh_rate;           /**< refresh rate (or zero for unspecified) */
        void *driverdata;           /**< driver-specific data, initialize to 0 */
    } SDL_DisplayMode;

    // Game controller bind type
    typedef enum SDL_GameControllerBindType
    {
        SDL_CONTROLLER_BINDTYPE_NONE = 0,
        SDL_CONTROLLER_BINDTYPE_BUTTON,
        SDL_CONTROLLER_BINDTYPE_AXIS,
        SDL_CONTROLLER_BINDTYPE_HAT
    } SDL_GameControllerBindType;

    // Game controller button bind
    typedef struct SDL_GameControllerButtonBind
    {
        SDL_GameControllerBindType bindType;
        union
        {
            int button;
            int axis;
            struct {
                int hat;
                int hat_mask;
            } hat;
        } value;
    } SDL_GameControllerButtonBind;

    typedef SDL_GUID SDL_JoystickGUID;

    #define SDL_BUTTON(X)       (1 << ((X)-1))
    enum {
        SDL_BUTTON_LEFT = 1,
        SDL_BUTTON_MIDDLE = 2,
        SDL_BUTTON_RIGHT = 3,
        SDL_BUTTON_X1 = 4,
        SDL_BUTTON_X2 = 5
    };

    enum {
        SDL_BUTTON_LMASK = SDL_BUTTON(SDL_BUTTON_LEFT),
        SDL_BUTTON_MMASK = SDL_BUTTON(SDL_BUTTON_MIDDLE),
        SDL_BUTTON_RMASK = SDL_BUTTON(SDL_BUTTON_RIGHT),
        SDL_BUTTON_X1MASK = SDL_BUTTON(SDL_BUTTON_X1),
        SDL_BUTTON_X2MASK = SDL_BUTTON(SDL_BUTTON_X2)
    };

// #ifdef __cplusplus
// extern "C" {
// #endif

#ifndef SDL_DYNAPI_PROC
#define SDL_DYNAPI_PROC(rc, fn, params, args, ret) extern __attribute__((weak)) rc fn params;
#endif

#ifndef SDL_PRINTF_FORMAT_STRING
#define SDL_PRINTF_FORMAT_STRING
#endif

#ifndef SDL_SCANF_FORMAT_STRING
#define SDL_SCANF_FORMAT_STRING
#endif

#ifndef SDL_OUT_Z_CAP
#define SDL_OUT_Z_CAP(x)
#endif

#ifndef SDL_OUT_BYTECAP
#define SDL_OUT_BYTECAP(x)
#endif

#ifndef SDL_IN_BYTECAP
#define SDL_IN_BYTECAP(x)
#endif

#ifndef SDL_INOUT_Z_CAP
#define SDL_INOUT_Z_CAP(x)
#endif

#ifndef SDL_IN_Z_CAP
#define SDL_IN_Z_CAP(x)
#endif

#ifndef SDLCALL
#define SDLCALL
#endif

#include "SDL2_dynapi_procs.h"

#ifdef SDL_DYNAPI_PROC
#undef SDL_DYNAPI_PROC
#endif

// #ifdef __cplusplus
// }
// #endif

};

}

#endif
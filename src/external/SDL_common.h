/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

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

/* Modified a bit by using a namespace and excluding #define, so that
 * it does not break with including SDL2 as well.
 */

#ifndef _SDL_common_h
#define _SDL_common_h

#include <stdint.h>

#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321

#ifdef __linux__
#include <endian.h>
#define SDL_BYTEORDER  __BYTE_ORDER
#else /* __linux__ */
#if defined(__hppa__) ||     defined(__m68k__) || defined(mc68000) || defined(_M_M68K) ||     (defined(__MIPS__) && defined(__MISPEB__)) ||     defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) ||     defined(__sparc__)
#define SDL_BYTEORDER   SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER   SDL_LIL_ENDIAN
#endif
#endif /* __linux__ */

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;
typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;
typedef int64_t Sint64;

typedef int SDL_bool;

enum {
    SDL_FALSE = 0,
    SDL_TRUE = 1
};

// Event state constants
enum 
{
    SDL_RELEASED = 0,
    SDL_PRESSED = 1,
    SDL_QUERY = -1,
    SDL_IGNORE = 0,
    SDL_DISABLE = 0,
    SDL_ENABLE = 1
};

typedef struct SDL_version
{
    Uint8 major;        /**< major version */
    Uint8 minor;        /**< minor version */
    Uint8 patch;        /**< update version */
} SDL_version;

// Forward declarations for incomplete types
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_PixelFormat SDL_PixelFormat;
typedef struct SDL_BlitMap SDL_BlitMap;
typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;
typedef struct SDL_sem SDL_sem;
typedef struct SDL_cond SDL_cond;
typedef struct SDL_Joystick SDL_Joystick;
typedef struct SDL_GameController SDL_GameController;
typedef struct SDL_Haptic SDL_Haptic;
typedef struct SDL_Cursor SDL_Cursor;
typedef struct SDL_RWops SDL_RWops;
typedef struct SDL_Finger SDL_Finger;
typedef void *SDL_MetalView;
typedef struct SDL_Locale SDL_Locale;
typedef struct SDL_hid_device_info SDL_hid_device_info;
typedef struct SDL_hid_device SDL_hid_device;
typedef struct SDL_AudioStream SDL_AudioStream;
typedef struct SDL_Vertex SDL_Vertex;
typedef struct SDL_Palette SDL_Palette;
typedef struct SDL_Point SDL_Point;
typedef struct SDL_RendererInfo SDL_RendererInfo;
typedef struct SDL_WindowShapeMode SDL_WindowShapeMode;
typedef struct SDL_atomic_t SDL_atomic_t;
typedef struct SDL_AudioCVT SDL_AudioCVT;
typedef struct SDL_HapticEffect SDL_HapticEffect;
typedef struct SDL_Sensor SDL_Sensor;
typedef struct SDL_SysWMinfo SDL_SysWMinfo;
typedef struct SDL_VirtualJoystickDesc SDL_VirtualJoystickDesc;
typedef struct SDL_GUID SDL_GUID;
typedef struct SDL_MessageBoxData SDL_MessageBoxData;

#endif /* _SDL_h */

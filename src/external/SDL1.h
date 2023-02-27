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

#ifndef _SDL1_h
#define _SDL1_h

#include <stdint.h>

namespace SDL1 {

    typedef uint8_t Uint8;
    typedef uint16_t Uint16;
    typedef uint32_t Uint32;
    typedef uint64_t Uint64;
    typedef int32_t Sint32;
    typedef int16_t Sint16;
    typedef int64_t Sint64;


    /** Event enumerations */
    typedef enum {
        SDL_NOEVENT = 0,			/**< Unused (do not remove) */
        SDL_ACTIVEEVENT,			/**< Application loses/gains visibility */
        SDL_KEYDOWN,			/**< Keys pressed */
        SDL_KEYUP,			/**< Keys released */
        SDL_MOUSEMOTION,			/**< Mouse moved */
        SDL_MOUSEBUTTONDOWN,		/**< Mouse button pressed */
        SDL_MOUSEBUTTONUP,		/**< Mouse button released */
        SDL_JOYAXISMOTION,		/**< Joystick axis motion */
        SDL_JOYBALLMOTION,		/**< Joystick trackball motion */
        SDL_JOYHATMOTION,		/**< Joystick hat position change */
        SDL_JOYBUTTONDOWN,		/**< Joystick button pressed */
        SDL_JOYBUTTONUP,			/**< Joystick button released */
        SDL_QUIT,			/**< User-requested quit */
        SDL_SYSWMEVENT,			/**< System specific event */
        SDL_EVENT_RESERVEDA,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVEDB,		/**< Reserved for future use.. */
        SDL_VIDEORESIZE,			/**< User resized video mode */
        SDL_VIDEOEXPOSE,			/**< Screen needs to be redrawn */
        SDL_EVENT_RESERVED2,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVED3,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVED4,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVED5,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVED6,		/**< Reserved for future use.. */
        SDL_EVENT_RESERVED7,		/**< Reserved for future use.. */
        /** Events SDL_USEREVENT through SDL_MAXEVENTS-1 are for your use */
        SDL_USEREVENT = 24,
        /** This last event is only for bounding internal arrays
         *  It is the number of bits in the event mask datatype -- Uint32
         */
        SDL_NUMEVENTS = 32
    } SDL_EventType;

    /** @name Predefined event masks */
    /*@{*/
#define SDL1_EVENTMASK(X)	(1<<(X))
    typedef enum {
        SDL_ACTIVEEVENTMASK	= SDL1_EVENTMASK(SDL_ACTIVEEVENT),
        SDL_KEYDOWNMASK		= SDL1_EVENTMASK(SDL_KEYDOWN),
        SDL_KEYUPMASK	        = SDL1_EVENTMASK(SDL_KEYUP),
        SDL_KEYEVENTMASK	    = SDL1_EVENTMASK(SDL_KEYDOWN)|
            SDL1_EVENTMASK(SDL_KEYUP),
        SDL_MOUSEMOTIONMASK	= SDL1_EVENTMASK(SDL_MOUSEMOTION),
        SDL_MOUSEBUTTONDOWNMASK	= SDL1_EVENTMASK(SDL_MOUSEBUTTONDOWN),
        SDL_MOUSEBUTTONUPMASK	= SDL1_EVENTMASK(SDL_MOUSEBUTTONUP),
        SDL_MOUSEEVENTMASK	= SDL1_EVENTMASK(SDL_MOUSEMOTION)|
            SDL1_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
            SDL1_EVENTMASK(SDL_MOUSEBUTTONUP),
        SDL_JOYAXISMOTIONMASK	= SDL1_EVENTMASK(SDL_JOYAXISMOTION),
        SDL_JOYBALLMOTIONMASK	= SDL1_EVENTMASK(SDL_JOYBALLMOTION),
        SDL_JOYHATMOTIONMASK	= SDL1_EVENTMASK(SDL_JOYHATMOTION),
        SDL_JOYBUTTONDOWNMASK	= SDL1_EVENTMASK(SDL_JOYBUTTONDOWN),
        SDL_JOYBUTTONUPMASK	= SDL1_EVENTMASK(SDL_JOYBUTTONUP),
        SDL_JOYEVENTMASK	= SDL1_EVENTMASK(SDL_JOYAXISMOTION)|
            SDL1_EVENTMASK(SDL_JOYBALLMOTION)|
            SDL1_EVENTMASK(SDL_JOYHATMOTION)|
            SDL1_EVENTMASK(SDL_JOYBUTTONDOWN)|
            SDL1_EVENTMASK(SDL_JOYBUTTONUP),
        SDL_VIDEORESIZEMASK	= SDL1_EVENTMASK(SDL_VIDEORESIZE),
        SDL_VIDEOEXPOSEMASK	= SDL1_EVENTMASK(SDL_VIDEOEXPOSE),
        SDL_QUITMASK		= SDL1_EVENTMASK(SDL_QUIT),
        SDL_SYSWMEVENTMASK	= SDL1_EVENTMASK(SDL_SYSWMEVENT),
        SDL_ALLEVENTS       = 0xFFFFFFFF
    } SDL_EventMask;
    /*@}*/

    typedef enum {
        /** @name ASCII mapped keysyms
         *  The keyboard syms have been cleverly chosen to map to ASCII
         */
        /*@{*/
        SDLK_UNKNOWN		= 0,
        SDLK_FIRST		= 0,
        SDLK_BACKSPACE		= 8,
        SDLK_TAB		= 9,
        SDLK_CLEAR		= 12,
        SDLK_RETURN		= 13,
        SDLK_PAUSE		= 19,
        SDLK_ESCAPE		= 27,
        SDLK_SPACE		= 32,
        SDLK_EXCLAIM		= 33,
        SDLK_QUOTEDBL		= 34,
        SDLK_HASH		= 35,
        SDLK_DOLLAR		= 36,
        SDLK_AMPERSAND		= 38,
        SDLK_QUOTE		= 39,
        SDLK_LEFTPAREN		= 40,
        SDLK_RIGHTPAREN		= 41,
        SDLK_ASTERISK		= 42,
        SDLK_PLUS		= 43,
        SDLK_COMMA		= 44,
        SDLK_MINUS		= 45,
        SDLK_PERIOD		= 46,
        SDLK_SLASH		= 47,
        SDLK_0			= 48,
        SDLK_1			= 49,
        SDLK_2			= 50,
        SDLK_3			= 51,
        SDLK_4			= 52,
        SDLK_5			= 53,
        SDLK_6			= 54,
        SDLK_7			= 55,
        SDLK_8			= 56,
        SDLK_9			= 57,
        SDLK_COLON		= 58,
        SDLK_SEMICOLON		= 59,
        SDLK_LESS		= 60,
        SDLK_EQUALS		= 61,
        SDLK_GREATER		= 62,
        SDLK_QUESTION		= 63,
        SDLK_AT			= 64,
        /*
           Skip uppercase letters
           */
        SDLK_LEFTBRACKET	= 91,
        SDLK_BACKSLASH		= 92,
        SDLK_RIGHTBRACKET	= 93,
        SDLK_CARET		= 94,
        SDLK_UNDERSCORE		= 95,
        SDLK_BACKQUOTE		= 96,
        SDLK_a			= 97,
        SDLK_b			= 98,
        SDLK_c			= 99,
        SDLK_d			= 100,
        SDLK_e			= 101,
        SDLK_f			= 102,
        SDLK_g			= 103,
        SDLK_h			= 104,
        SDLK_i			= 105,
        SDLK_j			= 106,
        SDLK_k			= 107,
        SDLK_l			= 108,
        SDLK_m			= 109,
        SDLK_n			= 110,
        SDLK_o			= 111,
        SDLK_p			= 112,
        SDLK_q			= 113,
        SDLK_r			= 114,
        SDLK_s			= 115,
        SDLK_t			= 116,
        SDLK_u			= 117,
        SDLK_v			= 118,
        SDLK_w			= 119,
        SDLK_x			= 120,
        SDLK_y			= 121,
        SDLK_z			= 122,
        SDLK_DELETE		= 127,
        /* End of ASCII mapped keysyms */
        /*@}*/

        /** @name International keyboard syms */
        /*@{*/
        SDLK_WORLD_0		= 160,		/* 0xA0 */
        SDLK_WORLD_1		= 161,
        SDLK_WORLD_2		= 162,
        SDLK_WORLD_3		= 163,
        SDLK_WORLD_4		= 164,
        SDLK_WORLD_5		= 165,
        SDLK_WORLD_6		= 166,
        SDLK_WORLD_7		= 167,
        SDLK_WORLD_8		= 168,
        SDLK_WORLD_9		= 169,
        SDLK_WORLD_10		= 170,
        SDLK_WORLD_11		= 171,
        SDLK_WORLD_12		= 172,
        SDLK_WORLD_13		= 173,
        SDLK_WORLD_14		= 174,
        SDLK_WORLD_15		= 175,
        SDLK_WORLD_16		= 176,
        SDLK_WORLD_17		= 177,
        SDLK_WORLD_18		= 178,
        SDLK_WORLD_19		= 179,
        SDLK_WORLD_20		= 180,
        SDLK_WORLD_21		= 181,
        SDLK_WORLD_22		= 182,
        SDLK_WORLD_23		= 183,
        SDLK_WORLD_24		= 184,
        SDLK_WORLD_25		= 185,
        SDLK_WORLD_26		= 186,
        SDLK_WORLD_27		= 187,
        SDLK_WORLD_28		= 188,
        SDLK_WORLD_29		= 189,
        SDLK_WORLD_30		= 190,
        SDLK_WORLD_31		= 191,
        SDLK_WORLD_32		= 192,
        SDLK_WORLD_33		= 193,
        SDLK_WORLD_34		= 194,
        SDLK_WORLD_35		= 195,
        SDLK_WORLD_36		= 196,
        SDLK_WORLD_37		= 197,
        SDLK_WORLD_38		= 198,
        SDLK_WORLD_39		= 199,
        SDLK_WORLD_40		= 200,
        SDLK_WORLD_41		= 201,
        SDLK_WORLD_42		= 202,
        SDLK_WORLD_43		= 203,
        SDLK_WORLD_44		= 204,
        SDLK_WORLD_45		= 205,
        SDLK_WORLD_46		= 206,
        SDLK_WORLD_47		= 207,
        SDLK_WORLD_48		= 208,
        SDLK_WORLD_49		= 209,
        SDLK_WORLD_50		= 210,
        SDLK_WORLD_51		= 211,
        SDLK_WORLD_52		= 212,
        SDLK_WORLD_53		= 213,
        SDLK_WORLD_54		= 214,
        SDLK_WORLD_55		= 215,
        SDLK_WORLD_56		= 216,
        SDLK_WORLD_57		= 217,
        SDLK_WORLD_58		= 218,
        SDLK_WORLD_59		= 219,
        SDLK_WORLD_60		= 220,
        SDLK_WORLD_61		= 221,
        SDLK_WORLD_62		= 222,
        SDLK_WORLD_63		= 223,
        SDLK_WORLD_64		= 224,
        SDLK_WORLD_65		= 225,
        SDLK_WORLD_66		= 226,
        SDLK_WORLD_67		= 227,
        SDLK_WORLD_68		= 228,
        SDLK_WORLD_69		= 229,
        SDLK_WORLD_70		= 230,
        SDLK_WORLD_71		= 231,
        SDLK_WORLD_72		= 232,
        SDLK_WORLD_73		= 233,
        SDLK_WORLD_74		= 234,
        SDLK_WORLD_75		= 235,
        SDLK_WORLD_76		= 236,
        SDLK_WORLD_77		= 237,
        SDLK_WORLD_78		= 238,
        SDLK_WORLD_79		= 239,
        SDLK_WORLD_80		= 240,
        SDLK_WORLD_81		= 241,
        SDLK_WORLD_82		= 242,
        SDLK_WORLD_83		= 243,
        SDLK_WORLD_84		= 244,
        SDLK_WORLD_85		= 245,
        SDLK_WORLD_86		= 246,
        SDLK_WORLD_87		= 247,
        SDLK_WORLD_88		= 248,
        SDLK_WORLD_89		= 249,
        SDLK_WORLD_90		= 250,
        SDLK_WORLD_91		= 251,
        SDLK_WORLD_92		= 252,
        SDLK_WORLD_93		= 253,
        SDLK_WORLD_94		= 254,
        SDLK_WORLD_95		= 255,		/* 0xFF */
        /*@}*/

        /** @name Numeric keypad */
        /*@{*/
        SDLK_KP0		= 256,
        SDLK_KP1		= 257,
        SDLK_KP2		= 258,
        SDLK_KP3		= 259,
        SDLK_KP4		= 260,
        SDLK_KP5		= 261,
        SDLK_KP6		= 262,
        SDLK_KP7		= 263,
        SDLK_KP8		= 264,
        SDLK_KP9		= 265,
        SDLK_KP_PERIOD		= 266,
        SDLK_KP_DIVIDE		= 267,
        SDLK_KP_MULTIPLY	= 268,
        SDLK_KP_MINUS		= 269,
        SDLK_KP_PLUS		= 270,
        SDLK_KP_ENTER		= 271,
        SDLK_KP_EQUALS		= 272,
        /*@}*/

        /** @name Arrows + Home/End pad */
        /*@{*/
        SDLK_UP			= 273,
        SDLK_DOWN		= 274,
        SDLK_RIGHT		= 275,
        SDLK_LEFT		= 276,
        SDLK_INSERT		= 277,
        SDLK_HOME		= 278,
        SDLK_END		= 279,
        SDLK_PAGEUP		= 280,
        SDLK_PAGEDOWN		= 281,
        /*@}*/

        /** @name Function keys */
        /*@{*/
        SDLK_F1			= 282,
        SDLK_F2			= 283,
        SDLK_F3			= 284,
        SDLK_F4			= 285,
        SDLK_F5			= 286,
        SDLK_F6			= 287,
        SDLK_F7			= 288,
        SDLK_F8			= 289,
        SDLK_F9			= 290,
        SDLK_F10		= 291,
        SDLK_F11		= 292,
        SDLK_F12		= 293,
        SDLK_F13		= 294,
        SDLK_F14		= 295,
        SDLK_F15		= 296,
        /*@}*/

        /** @name Key state modifier keys */
        /*@{*/
        SDLK_NUMLOCK		= 300,
        SDLK_CAPSLOCK		= 301,
        SDLK_SCROLLOCK		= 302,
        SDLK_RSHIFT		= 303,
        SDLK_LSHIFT		= 304,
        SDLK_RCTRL		= 305,
        SDLK_LCTRL		= 306,
        SDLK_RALT		= 307,
        SDLK_LALT		= 308,
        SDLK_RMETA		= 309,
        SDLK_LMETA		= 310,
        SDLK_LSUPER		= 311,		/**< Left "Windows" key */
        SDLK_RSUPER		= 312,		/**< Right "Windows" key */
        SDLK_MODE		= 313,		/**< "Alt Gr" key */
        SDLK_COMPOSE		= 314,		/**< Multi-key compose key */
        /*@}*/

        /** @name Miscellaneous function keys */
        /*@{*/
        SDLK_HELP		= 315,
        SDLK_PRINT		= 316,
        SDLK_SYSREQ		= 317,
        SDLK_BREAK		= 318,
        SDLK_MENU		= 319,
        SDLK_POWER		= 320,		/**< Power Macintosh power key */
        SDLK_EURO		= 321,		/**< Some european keyboards */
        SDLK_UNDO		= 322,		/**< Atari keyboard has Undo */
        /*@}*/

        /* Add any other keys here */

        SDLK_LAST
    } SDLKey;

    enum {
        SDL1_BUTTON_LEFT = 1,
        SDL1_BUTTON_MIDDLE = 2,
        SDL1_BUTTON_RIGHT = 3,
        SDL1_BUTTON_WHEELUP = 4,
        SDL1_BUTTON_WHEELDOWN = 5,
        SDL1_BUTTON_X1 = 6,
        SDL1_BUTTON_X2 = 7,
        SDL1_BUTTON_LMASK = (1 << ((1)-1)),
        SDL1_BUTTON_MMASK = (1 << ((2)-1)),
        SDL1_BUTTON_RMASK = (1 << ((3)-1)),
        SDL1_BUTTON_X1MASK = (1 << ((6)-1)),
        SDL1_BUTTON_X2MASK = (1 << ((7)-1)),
    };

    /** Application visibility event structure */
    typedef struct SDL_ActiveEvent {
        Uint8 type;	/**< SDL_ACTIVEEVENT */
        Uint8 gain;	/**< Whether given states were gained or lost (1/0) */
        Uint8 state;	/**< A mask of the focus states */
    } SDL_ActiveEvent;

    typedef int SDLMod;

    typedef struct SDL_keysym {
        Uint8 scancode;         /**< hardware specific scancode */
        SDLKey sym;         /**< SDL virtual keysym */
        SDLMod mod;         /**< current key modifiers */
        Uint16 unicode;         /**< translated character */
    } SDL_keysym;

    /** Keyboard event structure */
    typedef struct SDL_KeyboardEvent {
        Uint8 type;	/**< SDL_KEYDOWN or SDL_KEYUP */
        Uint8 which;	/**< The keyboard device index */
        Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
        SDL_keysym keysym;
    } SDL_KeyboardEvent;

    /** Mouse motion event structure */
    typedef struct SDL_MouseMotionEvent {
        Uint8 type;	/**< SDL_MOUSEMOTION */
        Uint8 which;	/**< The mouse device index */
        Uint8 state;	/**< The current button state */
        Uint16 x, y;	/**< The X/Y coordinates of the mouse */
        Sint16 xrel;	/**< The relative motion in the X direction */
        Sint16 yrel;	/**< The relative motion in the Y direction */
    } SDL_MouseMotionEvent;

    /** Mouse button event structure */
    typedef struct SDL_MouseButtonEvent {
        Uint8 type;	/**< SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP */
        Uint8 which;	/**< The mouse device index */
        Uint8 button;	/**< The mouse button index */
        Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
        Uint16 x, y;	/**< The X/Y coordinates of the mouse at press time */
    } SDL_MouseButtonEvent;

    /** Joystick axis motion event structure */
    typedef struct SDL_JoyAxisEvent {
        Uint8 type;	/**< SDL_JOYAXISMOTION */
        Uint8 which;	/**< The joystick device index */
        Uint8 axis;	/**< The joystick axis index */
        Sint16 value;	/**< The axis value (range: -32768 to 32767) */
    } SDL_JoyAxisEvent;

    /** Joystick trackball motion event structure */
    typedef struct SDL_JoyBallEvent {
        Uint8 type;	/**< SDL_JOYBALLMOTION */
        Uint8 which;	/**< The joystick device index */
        Uint8 ball;	/**< The joystick trackball index */
        Sint16 xrel;	/**< The relative motion in the X direction */
        Sint16 yrel;	/**< The relative motion in the Y direction */
    } SDL_JoyBallEvent;

    /** Joystick hat position change event structure */
    typedef struct SDL_JoyHatEvent {
        Uint8 type;	/**< SDL_JOYHATMOTION */
        Uint8 which;	/**< The joystick device index */
        Uint8 hat;	/**< The joystick hat index */
        Uint8 value;	/**< The hat position value:
                         *   SDL_HAT_LEFTUP   SDL_HAT_UP       SDL_HAT_RIGHTUP
                         *   SDL_HAT_LEFT     SDL_HAT_CENTERED SDL_HAT_RIGHT
                         *   SDL_HAT_LEFTDOWN SDL_HAT_DOWN     SDL_HAT_RIGHTDOWN
                         *  Note that zero means the POV is centered.
                         */
    } SDL_JoyHatEvent;

    /** Joystick button event structure */
    typedef struct SDL_JoyButtonEvent {
        Uint8 type;	/**< SDL_JOYBUTTONDOWN or SDL_JOYBUTTONUP */
        Uint8 which;	/**< The joystick device index */
        Uint8 button;	/**< The joystick button index */
        Uint8 state;	/**< SDL_PRESSED or SDL_RELEASED */
    } SDL_JoyButtonEvent;

    /** The "window resized" event
     *  When you get this event, you are responsible for setting a new video
     *  mode with the new width and height.
     */
    typedef struct SDL_ResizeEvent {
        Uint8 type;	/**< SDL_VIDEORESIZE */
        int w;		/**< New width */
        int h;		/**< New height */
    } SDL_ResizeEvent;

    /** The "screen redraw" event */
    typedef struct SDL_ExposeEvent {
        Uint8 type;	/**< SDL_VIDEOEXPOSE */
    } SDL_ExposeEvent;

    /** The "quit requested" event */
    typedef struct SDL_QuitEvent {
        Uint8 type;	/**< SDL_QUIT */
    } SDL_QuitEvent;

    /** A user-defined event type */
    typedef struct SDL_UserEvent {
        Uint8 type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
        int code;	/**< User defined event code */
        void *data1;	/**< User defined data pointer */
        void *data2;	/**< User defined data pointer */
    } SDL_UserEvent;

    /** If you want to use this event, you should include SDL_syswm.h */
    struct SDL_SysWMmsg;
    typedef struct SDL_SysWMmsg SDL_SysWMmsg;
    typedef struct SDL_SysWMEvent {
        Uint8 type;
        SDL_SysWMmsg *msg;
    } SDL_SysWMEvent;

    /** General event structure */
    typedef union SDL_Event {
        Uint8 type;
        SDL_ActiveEvent active;
        SDL_KeyboardEvent key;
        SDL_MouseMotionEvent motion;
        SDL_MouseButtonEvent button;
        SDL_JoyAxisEvent jaxis;
        SDL_JoyBallEvent jball;
        SDL_JoyHatEvent jhat;
        SDL_JoyButtonEvent jbutton;
        SDL_ResizeEvent resize;
        SDL_ExposeEvent expose;
        SDL_QuitEvent quit;
        SDL_UserEvent user;
        SDL_SysWMEvent syswm;
    } SDL_Event;

    typedef struct SDL_Rect {
        Sint16 x, y;
        Uint16 w, h;
    } SDL_Rect;

    typedef struct SDL_Color {
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 unused;
    } SDL_Color;

    typedef struct SDL_Palette {
        int       ncolors;
        SDL_Color *colors;
    } SDL_Palette;

    /** Everything in the pixel format structure is read-only */
    typedef struct SDL_PixelFormat {
        SDL_Palette *palette;
        Uint8  BitsPerPixel;
        Uint8  BytesPerPixel;
        Uint8  Rloss;
        Uint8  Gloss;
        Uint8  Bloss;
        Uint8  Aloss;
        Uint8  Rshift;
        Uint8  Gshift;
        Uint8  Bshift;
        Uint8  Ashift;
        Uint32 Rmask;
        Uint32 Gmask;
        Uint32 Bmask;
        Uint32 Amask;

        /** RGB color key information */
        Uint32 colorkey;
        /** Alpha value information (per-surface alpha) */
        Uint8  alpha;
    } SDL_PixelFormat;

    /** This structure should be treated as read-only, except for 'pixels',
     *  which, if not NULL, contains the raw pixel data for the surface.
     */
    typedef struct SDL_Surface {
        Uint32 flags;				/**< Read-only */
        SDL_PixelFormat *format;		/**< Read-only */
        int w, h;				/**< Read-only */
        Uint16 pitch;				/**< Read-only */
        void *pixels;				/**< Read-write */
        int offset;				/**< Private */

        /** Hardware-specific surface info */
        void *hwdata;

        /** clipping information */
        SDL_Rect clip_rect;			/**< Read-only */
        Uint32 unused1;				/**< for binary compatibility */

        /** Allow recursive locks */
        Uint32 locked;				/**< Private */

        /** info for fast blit mapping to other surfaces */
        void *map;		/**< Private */

        /** format version, bumped at every change to invalidate blit maps */
        unsigned int format_version;		/**< Private */

        /** Reference count -- used when freeing surface */
        int refcount;				/**< Read-mostly */
    } SDL_Surface;

    enum {
    	SDL1_SWSURFACE = 0x00000000,  /**< Surface is in system memory */
    	SDL1_HWSURFACE = 0x00000001,  /**< Surface is in video memory */
        SDL1_ASYNCBLIT = 0x00000004,  /**< Use asynchronous blits if possible */
        SDL1_ANYFORMAT = 0x10000000,  /**< Allow any video depth/pixel-format */
        SDL1_HWPALETTE = 0x20000000,  /**< Surface has exclusive palette */
        SDL1_DOUBLEBUF = 0x40000000,  /**< Set up double-buffered video mode */
        SDL1_FULLSCREEN = 0x80000000,  /**< Surface is a full screen display */
        SDL1_OPENGL = 0x00000002,      /**< Create an OpenGL rendering context */
        SDL1_OPENGLBLIT = 0x0000000A,  /**< Create an OpenGL rendering context and use it for blitting */
        SDL1_RESIZABLE = 0x00000010,  /**< This video mode may be resized */
        SDL1_NOFRAME = 0x00000020,  /**< No window caption or edge frame */
        SDL1_HWACCEL = 0x00000100,  /**< Blit uses hardware acceleration */
        SDL1_SRCCOLORKEY = 0x00001000,  /**< Blit uses a source color key */
        SDL1_RLEACCELOK = 0x00002000,  /**< Private flag */
        SDL1_RLEACCEL = 0x00004000,  /**< Surface is RLE encoded */
        SDL1_SRCALPHA = 0x00010000,  /**< Blit uses source alpha blending */
        SDL1_PREALLOC = 0x01000000,  /**< Surface uses preallocated memory */
    };


    typedef int (*SDL_EventFilter)(const SDL_Event *event);

    typedef enum {
    	SDL_GRAB_QUERY = -1,
    	SDL_GRAB_OFF = 0,
    	SDL_GRAB_ON = 1,
    	SDL_GRAB_FULLSCREEN	/**< Used internally */
    } SDL_GrabMode;
    
    enum {
        SDL_APPMOUSEFOCUS = 0x01,        /**< The app has mouse coverage */
        SDL_APPINPUTFOCUS = 0x02,        /**< The app has input focus */
        SDL_APPACTIVE = 0x04,        /**< The application is active */
    };
}

#endif /* _SDL_h */

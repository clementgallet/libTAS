/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SINGLEINPUT_H_INCLUDED
#define LIBTAS_SINGLEINPUT_H_INCLUDED

#include <string>

class SingleInput {
public:
    /* Pointer mode */
    enum {
        POINTER_MODE_ABSOLUTE,
        POINTER_MODE_RELATIVE,
    };

    /* Pointer buttons */
    enum {
        POINTER_B1,
        POINTER_B2,
        POINTER_B3,
        POINTER_B4,
        POINTER_B5,
    };

    /* Controller buttons */
    enum {
        BUTTON_A,
        BUTTON_B,
        BUTTON_X,
        BUTTON_Y,
        BUTTON_BACK,
        BUTTON_GUIDE,
        BUTTON_START,
        BUTTON_LEFTSTICK,
        BUTTON_RIGHTSTICK,
        BUTTON_LEFTSHOULDER,
        BUTTON_RIGHTSHOULDER,
        BUTTON_DPAD_UP,
        BUTTON_DPAD_DOWN,
        BUTTON_DPAD_LEFT,
        BUTTON_DPAD_RIGHT,
        BUTTON_LAST
    };

    /* Controller axes */
    enum {
        AXIS_LEFTX,
        AXIS_LEFTY,
        AXIS_RIGHTX,
        AXIS_RIGHTY,
        AXIS_TRIGGERLEFT,
        AXIS_TRIGGERRIGHT,
        AXIS_LAST
    };

    /* Flags */
    enum {
        FLAG_RESTART,
        FLAG_CONTROLLER1_ADDED_REMOVED,
        FLAG_CONTROLLER2_ADDED_REMOVED,
        FLAG_CONTROLLER3_ADDED_REMOVED,
        FLAG_CONTROLLER4_ADDED_REMOVED,
        FLAG_FOCUS_UNFOCUS,
    };

    /* Input type */
    enum {
        IT_NONE = -1, /* No input */
        IT_KEYBOARD = 0, /* Keyboard */

        /* Mouse */
        IT_POINTER_X = 1,
        IT_POINTER_Y = 2,
        IT_POINTER_WHEEL = 3,
        IT_POINTER_MODE = 4,
        IT_POINTER_BUTTON = 5,

        /* Single flag */
        IT_FLAG = 10,

        /* Framerate */
        IT_FRAMERATE_NUM,
        IT_FRAMERATE_DEN,

        /* Realtime */
        IT_REALTIME_SEC,
        IT_REALTIME_NSEC,

        /* Controllers. Don't move the order because arithmetic is used on it */
        IT_CONTROLLER1_BUTTON,
        IT_CONTROLLER1_AXIS,
        IT_CONTROLLER2_BUTTON,
        IT_CONTROLLER2_AXIS,
        IT_CONTROLLER3_BUTTON,
        IT_CONTROLLER3_AXIS,
        IT_CONTROLLER4_BUTTON,
        IT_CONTROLLER4_AXIS,
    };

    int type;
    unsigned int which;
    std::string description;

    bool operator==( const SingleInput &si ) const {
        return (type == si.type) && (which == si.which);
    }

    bool operator<( const SingleInput &si ) const {
        return ((type < si.type) || ((type == si.type) && (which < si.which)));
    }

    /* Returns if the input is analog (non-boolean) */
    bool isAnalog() const;

    /* Check is input type is a controller */
    int inputTypeIsController() const;

    /* Extract a controller number from an input type */
    int inputTypeToControllerNumber() const;

    /* Extract a controller axis flag from an input type */
    bool inputTypeToAxisFlag() const;

    /* Extract a controller input number from an input type */
    int inputTypeToInputNumber() const;

#ifdef __unix__
    /* Convert a pointer button to a Xlib pointer button */
    static unsigned int toXlibPointerButton(int button);
#endif

    /* Convert a pointer button to a SDL1 pointer button */
    static unsigned int toSDL1PointerButton(int button);

    /* Convert a pointer button to a SDL2 pointer button */
    static unsigned int toSDL2PointerButton(int button);

#ifdef __unix__
    /* Convert a pointer mask to a Xlib pointer mask */
    static unsigned int toXlibPointerMask(int mask);
#endif

    /* Convert a pointer mask to a SDL1 pointer mask */
    static unsigned int toSDL1PointerMask(int mask);

    /* Convert a pointer mask to a SDL2 pointer mask */
    static unsigned int toSDL2PointerMask(int mask);

    /* Convert an axis number to a SDL2 axis number */
    static int toSDL2Axis(int axis);

    /* Convert a button number to a SDL2 button number */
    static int toSDL2Button(int button);

    /* Convert button flags to a SDL1/2 hat number */
    static int toSDLHat(int buttons);

#ifdef __linux__
    /* Convert an axis number to an jsdev axis number */
    static int toJsdevAxis(int axis);

    /* Convert a button number to an jsdev button number */
    static int toJsdevButton(int button);

    /* Convert an axis number to an evdev axis number */
    static int toEvdevAxis(int axis);

    /* Convert a button number to an evdev button number */
    static int toEvdevButton(int button);

    /* Convert button flags to an jsdev/evdev horizontal hat number */
    static int toDevHatX(int buttons);

    /* Convert button flags to an jsdev/evdev vertical hat number */
    static int toDevHatY(int buttons);
#endif

};

#endif

/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
    };

    static const int IT_CONTROLLER_TYPE_MASK = 0x7F;
    static const int IT_CONTROLLER_ID_SHIFT = 8;
    static const int IT_CONTROLLER_AXIS_MASK = 0x80;

    /* Input type */
    enum {
        IT_NONE = -1, /* No input */
        IT_KEYBOARD = 0, /* Keyboard */

        /* Mouse */
        IT_POINTER_X = 1,
        IT_POINTER_Y = 2,
        IT_POINTER_MODE = 3,
        IT_POINTER_B1 = 4 + POINTER_B1,
        IT_POINTER_B2 = 4 + POINTER_B2,
        IT_POINTER_B3 = 4 + POINTER_B3,
        IT_POINTER_B4 = 4 + POINTER_B4,
        IT_POINTER_B5 = 4 + POINTER_B5,

        /* Single flag */
        IT_FLAG = 10,

        /* Framerate */
        IT_FRAMERATE_NUM,
        IT_FRAMERATE_DEN,

        /* Realtime */
        IT_REALTIME_SEC,
        IT_REALTIME_NSEC,

        /* Controller 1 */
        IT_CONTROLLER1_BUTTON_A = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_A,
        IT_CONTROLLER1_BUTTON_B = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_B,
        IT_CONTROLLER1_BUTTON_X = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_X,
        IT_CONTROLLER1_BUTTON_Y = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_Y,
        IT_CONTROLLER1_BUTTON_BACK = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_BACK,
        IT_CONTROLLER1_BUTTON_GUIDE = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_GUIDE,
        IT_CONTROLLER1_BUTTON_START = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_START,
        IT_CONTROLLER1_BUTTON_LEFTSTICK = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSTICK,
        IT_CONTROLLER1_BUTTON_RIGHTSTICK = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSTICK,
        IT_CONTROLLER1_BUTTON_LEFTSHOULDER = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSHOULDER,
        IT_CONTROLLER1_BUTTON_RIGHTSHOULDER = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSHOULDER,
        IT_CONTROLLER1_BUTTON_DPAD_UP = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_UP,
        IT_CONTROLLER1_BUTTON_DPAD_DOWN = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_DOWN,
        IT_CONTROLLER1_BUTTON_DPAD_LEFT = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_LEFT,
        IT_CONTROLLER1_BUTTON_DPAD_RIGHT = (1 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_RIGHT,
        IT_CONTROLLER1_AXIS_LEFTX = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTX,
        IT_CONTROLLER1_AXIS_LEFTY = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTY,
        IT_CONTROLLER1_AXIS_RIGHTX = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTX,
        IT_CONTROLLER1_AXIS_RIGHTY = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTY,
        IT_CONTROLLER1_AXIS_TRIGGERLEFT = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERLEFT,
        IT_CONTROLLER1_AXIS_TRIGGERRIGHT = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERRIGHT,

        /* Controller 2 */
        IT_CONTROLLER2_BUTTON_A = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_A,
        IT_CONTROLLER2_BUTTON_B = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_B,
        IT_CONTROLLER2_BUTTON_X = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_X,
        IT_CONTROLLER2_BUTTON_Y = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_Y,
        IT_CONTROLLER2_BUTTON_BACK = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_BACK,
        IT_CONTROLLER2_BUTTON_GUIDE = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_GUIDE,
        IT_CONTROLLER2_BUTTON_START = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_START,
        IT_CONTROLLER2_BUTTON_LEFTSTICK = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSTICK,
        IT_CONTROLLER2_BUTTON_RIGHTSTICK = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSTICK,
        IT_CONTROLLER2_BUTTON_LEFTSHOULDER = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSHOULDER,
        IT_CONTROLLER2_BUTTON_RIGHTSHOULDER = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSHOULDER,
        IT_CONTROLLER2_BUTTON_DPAD_UP = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_UP,
        IT_CONTROLLER2_BUTTON_DPAD_DOWN = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_DOWN,
        IT_CONTROLLER2_BUTTON_DPAD_LEFT = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_LEFT,
        IT_CONTROLLER2_BUTTON_DPAD_RIGHT = (2 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_RIGHT,
        IT_CONTROLLER2_AXIS_LEFTX = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTX,
        IT_CONTROLLER2_AXIS_LEFTY = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTY,
        IT_CONTROLLER2_AXIS_RIGHTX = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTX,
        IT_CONTROLLER2_AXIS_RIGHTY = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTY,
        IT_CONTROLLER2_AXIS_TRIGGERLEFT = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERLEFT,
        IT_CONTROLLER2_AXIS_TRIGGERRIGHT = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERRIGHT,

        /* Controller 3 */
        IT_CONTROLLER3_BUTTON_A = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_A,
        IT_CONTROLLER3_BUTTON_B = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_B,
        IT_CONTROLLER3_BUTTON_X = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_X,
        IT_CONTROLLER3_BUTTON_Y = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_Y,
        IT_CONTROLLER3_BUTTON_BACK = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_BACK,
        IT_CONTROLLER3_BUTTON_GUIDE = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_GUIDE,
        IT_CONTROLLER3_BUTTON_START = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_START,
        IT_CONTROLLER3_BUTTON_LEFTSTICK = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSTICK,
        IT_CONTROLLER3_BUTTON_RIGHTSTICK = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSTICK,
        IT_CONTROLLER3_BUTTON_LEFTSHOULDER = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSHOULDER,
        IT_CONTROLLER3_BUTTON_RIGHTSHOULDER = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSHOULDER,
        IT_CONTROLLER3_BUTTON_DPAD_UP = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_UP,
        IT_CONTROLLER3_BUTTON_DPAD_DOWN = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_DOWN,
        IT_CONTROLLER3_BUTTON_DPAD_LEFT = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_LEFT,
        IT_CONTROLLER3_BUTTON_DPAD_RIGHT = (3 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_RIGHT,
        IT_CONTROLLER3_AXIS_LEFTX = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTX,
        IT_CONTROLLER3_AXIS_LEFTY = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTY,
        IT_CONTROLLER3_AXIS_RIGHTX = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTX,
        IT_CONTROLLER3_AXIS_RIGHTY = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTY,
        IT_CONTROLLER3_AXIS_TRIGGERLEFT = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERLEFT,
        IT_CONTROLLER3_AXIS_TRIGGERRIGHT = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERRIGHT,

        /* Controller 4 */
        IT_CONTROLLER4_BUTTON_A = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_A,
        IT_CONTROLLER4_BUTTON_B = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_B,
        IT_CONTROLLER4_BUTTON_X = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_X,
        IT_CONTROLLER4_BUTTON_Y = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_Y,
        IT_CONTROLLER4_BUTTON_BACK = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_BACK,
        IT_CONTROLLER4_BUTTON_GUIDE = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_GUIDE,
        IT_CONTROLLER4_BUTTON_START = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_START,
        IT_CONTROLLER4_BUTTON_LEFTSTICK = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSTICK,
        IT_CONTROLLER4_BUTTON_RIGHTSTICK = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSTICK,
        IT_CONTROLLER4_BUTTON_LEFTSHOULDER = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_LEFTSHOULDER,
        IT_CONTROLLER4_BUTTON_RIGHTSHOULDER = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_RIGHTSHOULDER,
        IT_CONTROLLER4_BUTTON_DPAD_UP = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_UP,
        IT_CONTROLLER4_BUTTON_DPAD_DOWN = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_DOWN,
        IT_CONTROLLER4_BUTTON_DPAD_LEFT = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_LEFT,
        IT_CONTROLLER4_BUTTON_DPAD_RIGHT = (4 << IT_CONTROLLER_ID_SHIFT) + BUTTON_DPAD_RIGHT,
        IT_CONTROLLER4_AXIS_LEFTX = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTX,
        IT_CONTROLLER4_AXIS_LEFTY = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_LEFTY,
        IT_CONTROLLER4_AXIS_RIGHTX = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTX,
        IT_CONTROLLER4_AXIS_RIGHTY = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_RIGHTY,
        IT_CONTROLLER4_AXIS_TRIGGERLEFT = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERLEFT,
        IT_CONTROLLER4_AXIS_TRIGGERRIGHT = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AXIS_TRIGGERRIGHT,
    };

    int type;
    unsigned int value;
    std::string description;

    bool operator==( const SingleInput &si ) const {
        return (type == si.type) && (value == si.value);
    }

    bool operator<( const SingleInput &si ) const {
        return ((type < si.type) || ((type == si.type) && (value < si.value)));
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

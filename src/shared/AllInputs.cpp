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

#include "AllInputs.h"
#include <linux/input.h>
#include <SDL2/SDL_gamecontroller.h>


void AllInputs::emptyInputs() {
    int i,j;
    for (i=0; i<MAXKEYS; i++)
        keyboard[i] = XK_VoidSymbol;

    pointer_x = 0;
    pointer_y = 0;
    pointer_mask = 0;

    for (i=0; i<MAXJOYS; i++) {
        for (j=0; j<MAXAXES; j++)
            controller_axes[i][j] = 0;
        controller_buttons[i] = 0;
    }
}

int AllInputs::toSDL2Axis(int axis)
{
    switch(axis) {
        case AllInputs::AXIS_LEFTX:
            return SDL_CONTROLLER_AXIS_LEFTX;
        case AllInputs::AXIS_LEFTY:
            return SDL_CONTROLLER_AXIS_LEFTY;
        case AllInputs::AXIS_RIGHTX:
            return SDL_CONTROLLER_AXIS_RIGHTX;
        case AllInputs::AXIS_RIGHTY:
            return SDL_CONTROLLER_AXIS_RIGHTY;
        case AllInputs::AXIS_TRIGGERLEFT:
            return SDL_CONTROLLER_AXIS_TRIGGERLEFT;
        case AllInputs::AXIS_TRIGGERRIGHT:
            return SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
        default:
            return SDL_CONTROLLER_AXIS_INVALID;
    }
}

int AllInputs::toSDL2Button(int button)
{
    switch(button) {
        case AllInputs::BUTTON_A:
            return SDL_CONTROLLER_BUTTON_A;
        case AllInputs::BUTTON_B:
            return SDL_CONTROLLER_BUTTON_B;
        case AllInputs::BUTTON_X:
            return SDL_CONTROLLER_BUTTON_X;
        case AllInputs::BUTTON_Y:
            return SDL_CONTROLLER_BUTTON_Y;
        case AllInputs::BUTTON_BACK:
            return SDL_CONTROLLER_BUTTON_BACK;
        case AllInputs::BUTTON_GUIDE:
            return SDL_CONTROLLER_BUTTON_GUIDE;
        case AllInputs::BUTTON_START:
            return SDL_CONTROLLER_BUTTON_START;
        case AllInputs::BUTTON_LEFTSTICK:
            return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case AllInputs::BUTTON_RIGHTSTICK:
            return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        case AllInputs::BUTTON_LEFTSHOULDER:
            return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case AllInputs::BUTTON_RIGHTSHOULDER:
            return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case AllInputs::BUTTON_DPAD_UP:
            return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case AllInputs::BUTTON_DPAD_DOWN:
            return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case AllInputs::BUTTON_DPAD_LEFT:
            return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case AllInputs::BUTTON_DPAD_RIGHT:
            return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        default:
            return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

int AllInputs::toJsdevAxis(int axis)
{
    /* Mapping between xbox360 controller and joydev is taken from
     * http://wiki.unity3d.com/index.php/Xbox360Controller
     */
    switch(axis) {
        case AllInputs::AXIS_LEFTX:
            return 0;
        case AllInputs::AXIS_LEFTY:
            return 1;
        case AllInputs::AXIS_RIGHTX:
            return 3;
        case AllInputs::AXIS_RIGHTY:
            return 4;
        case AllInputs::AXIS_TRIGGERLEFT:
            return 2;
        case AllInputs::AXIS_TRIGGERRIGHT:
            return 5;
        default:
            return -1;
    }
}

int AllInputs::toJsdevButton(int button)
{
    /* Mapping between xbox360 controller and joydev is taken from
     * http://wiki.unity3d.com/index.php/Xbox360Controller
     */
    switch(button) {
        case AllInputs::BUTTON_A:
            return 0;
        case AllInputs::BUTTON_B:
            return 1;
        case AllInputs::BUTTON_X:
            return 2;
        case AllInputs::BUTTON_Y:
            return 3;
        case AllInputs::BUTTON_BACK:
            return 6;
        case AllInputs::BUTTON_GUIDE:
            return 8;
        case AllInputs::BUTTON_START:
            return 7;
        case AllInputs::BUTTON_LEFTSTICK:
            return 9;
        case AllInputs::BUTTON_RIGHTSTICK:
            return 10;
        case AllInputs::BUTTON_LEFTSHOULDER:
            return 4;
        case AllInputs::BUTTON_RIGHTSHOULDER:
            return 5;
        default:
            return -1;
    }
}

int AllInputs::toEvdevAxis(int axis)
{
    switch(axis) {
        case AllInputs::AXIS_LEFTX:
            return ABS_X;
        case AllInputs::AXIS_LEFTY:
            return ABS_Y;
        case AllInputs::AXIS_RIGHTX:
            return ABS_RX;
        case AllInputs::AXIS_RIGHTY:
            return ABS_RY;
        case AllInputs::AXIS_TRIGGERLEFT:
            return ABS_Z;
        case AllInputs::AXIS_TRIGGERRIGHT:
            return ABS_RZ;
        default:
            return -1;
    }
}

int AllInputs::toEvdevButton(int button)
{
    switch(button) {
        case AllInputs::BUTTON_A:
            return BTN_A;
        case AllInputs::BUTTON_B:
            return BTN_B;
        case AllInputs::BUTTON_X:
            return BTN_X;
        case AllInputs::BUTTON_Y:
            return BTN_Y;
        case AllInputs::BUTTON_BACK:
            return BTN_SELECT;
        case AllInputs::BUTTON_GUIDE:
            return BTN_MODE;
        case AllInputs::BUTTON_START:
            return BTN_START;
        case AllInputs::BUTTON_LEFTSTICK:
            return BTN_THUMBL;
        case AllInputs::BUTTON_RIGHTSTICK:
            return BTN_THUMBR;
        case AllInputs::BUTTON_LEFTSHOULDER:
            return BTN_TL;
        case AllInputs::BUTTON_RIGHTSHOULDER:
            return BTN_TR;
        default:
            return -1;
    }
}

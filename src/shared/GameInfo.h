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

#ifndef LIBTAS_GAMEINFO_H_INCLUDED
#define LIBTAS_GAMEINFO_H_INCLUDED

#include <string>

/*
 * Structure that holds information about what the game uses for its engine,
 * rendering, input devices, audio, etc, so that it can be collected and
 * displayed in the UI.
 */
struct GameInfo {
    bool tosend = false;

    enum Flag {
        UNKNOWN = 0,
        SDL1 = 0x01,
        SDL2 = 0x02,
        OPENGL = 0x04,
        OPENAL = 0x10,
        PULSEAUDIO = 0x20,
        XEVENTS = 0x40,
        JSDEV = 0x80,
        EVDEV = 0x100,
        ALSA = 0x200,
    };

    int video = UNKNOWN;
    int audio = UNKNOWN;
    int keyboard = XEVENTS;
    int mouse = XEVENTS;
    int joystick = UNKNOWN;
};

#endif

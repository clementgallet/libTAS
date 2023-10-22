/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
        SDL2_SURFACE = 0x04,
        SDL2_RENDERER = 0x08,
        OPENGL = 0x10,
        EGL = 0x20,
        VDPAU = 0x40,
        XSHM = 0x80,
        OPENAL = 0x100,
        PULSEAUDIO = 0x200,
        XEVENTS = 0x400,
        XIEVENTS = 0x800,
        XIRAWEVENTS = 0x1000,
        EVDEV = 0x2000,
        JSDEV = 0x4000,
        ALSA = 0x8000,
        XCBEVENTS = 0x10000,
        VULKAN = 0x20000,
    };

    int video = UNKNOWN;
    int audio = UNKNOWN;
    int keyboard = XEVENTS;
    int mouse = XEVENTS;
    int joystick = UNKNOWN;

    int opengl_major = 0;
    int opengl_minor = 0;

    enum Profile {
        NONE,
        CORE,
        COMPATIBILITY,
        ES
    };
    Profile opengl_profile = NONE;

};

#endif

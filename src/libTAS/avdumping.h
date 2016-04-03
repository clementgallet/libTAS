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

#ifndef AVDUMPING_H_INCL
#define AVDUMPING_H_INCL

#ifdef LIBTAS_ENABLE_AVDUMPING

#include <string>

int openAVDumping(void* window, int video_opengl, char* filename, int start_frame);
int encodeOneFrame(unsigned long fcounter, void* window);
int closeAVDumping();

#endif
#endif

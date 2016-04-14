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

#ifndef LIBTAS_AVDUMPING_H_INCL
#define LIBTAS_AVDUMPING_H_INCL

#ifdef LIBTAS_ENABLE_AVDUMPING

#include <string>

/* Set up the AV dumping into a file.
 * This consists mainly of getting the dimensions of the screen,
 * then initialize all objetcs from ffmpeg libraries
 * (context, format, codecs, frames, files, etc.)
 * 
 * @param window        Pointer to the SDL_Window* struct that is captured
 * @param video_opengl  Flag indicating if display is done using openGL or
 *                      software SDL rendering
 * @param filename      File where dumping is outputed. File extension
 *                      is important, it is used to guess the file container
 * @param start_frame   Frame when init is done. Does matter if dumping is not
 *                      done from the beginning.
 * @return              1 if error, 0 if not
 */
int openAVDumping(void* window, bool video_opengl, char* filename, int start_frame);

/* Encode a video and audio frame.
 * @param fconter       Frame counter
 * @param window        SDL Window* (needed for software rendering)
 * @return              1 if error, 0 if not
 */
int encodeOneFrame(unsigned long fcounter, void* window);

/* Close all allocated objects at the end of a av dump
 * @return              1 if error, 0 if not
 */
int closeAVDumping(void);

#endif
#endif

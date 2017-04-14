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

#ifndef LIBTAS_MESSAGES_H_INCLUDED
#define LIBTAS_MESSAGES_H_INCLUDED

/* List of message identification values that is sent from/to the game */
enum {
    /*
     * The game notices the program that he reached a frame boundary.
     * Then he sends the frame number
     * Argument: unsigned long
     */
    MSGB_START_FRAMEBOUNDARY,

    /*
     * Send all inputs to the game
     * Argument: struct AllInputs
     */
    MSGN_ALL_INPUTS,

    /*
     * Send tasflags
     * Argument: struct TasFlags
     */
    MSGN_TASFLAGS,

    /*
     * Send config
     * Argument: struct Config
     */
    MSGN_CONFIG,

    /*
     * The programs tells the game to end the frame boundary
     * Argument: none
     */
    MSGN_END_FRAMEBOUNDARY,

    /*
     * The game tells the program that he has quit
     * Argument: none
     */
    MSGB_QUIT,

    /*
     * The program tells the game that the user has requested a quit
     * Argument: none
     */
    MSGN_USERQUIT,

    /*
     * Send the game pid so that the program can attach to it
     * Argument: pid_t
     */
    MSGB_PID,

    /*
     * Notice the program of the end of initialization messages
     * Argument: none
     */
    MSGB_END_INIT,

    /*
     * Notice the game of the end of initialization messages
     * Argument: none
     */
    MSGN_END_INIT,

    /*
     * Send the dump file to the game
     * Arguments: size_t (string length) then char[len]
     */
    MSGN_DUMP_FILE,

    /*
     * Send the name of a shared library used by the game
     * Arguments: size_t (string length) then char[len]
     */
    MSGN_LIB_FILE,

    /*
     * Send the X11 window identifier of the game to the program
     * Argument: int
     */
    MSGB_WINDOW_ID,
};

#endif

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
     * Argument: none
     */
    MSGB_START_FRAMEBOUNDARY,

    /*
     * The game sends the frame number and time
     * Argument: unsigned long, struct timespec
     */
    MSGB_FRAMECOUNT_TIME,

    /*
     * Send all inputs to the game
     * Argument: struct AllInputs
     */
    MSGN_ALL_INPUTS,

    /*
     * Send all inputs to the game during a frame boundary, so that it can
     * display the inputs in the HUD
     * Argument: struct AllInputs
     */
    MSGN_PREVIEW_INPUTS,

    /*
     * Send config
     * Argument: struct SharedConfig
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

    /*
     * Send an alert message to be prompted by the program
     * Arguments: size_t (string length) then char[len]
     */
    MSGB_ALERT_MSG,

    /*
     * Ask the game to make a savestate
     * Argument: none
     */
    MSGN_SAVESTATE,

    /*
     * Ask the game to load a savestate
     * Argument: none
     */
    MSGN_LOADSTATE,

    /*
     * Tells the program that the loading succeeded
     * Argument: none
     */
    MSGB_LOADING_SUCCEEDED,

    /*
     * Notify the program that encoding failed
     * Arguments: none
     */
    MSGB_ENCODE_FAILED,

    /*
     * Notify the game that we must stop encoding
     * Arguments: none
     */
    MSGN_STOP_ENCODE,

    /*
     * Send game information to the program
     * Arguments: 3 strings
     */
    MSGB_GAMEINFO,

    /*
     * Notify the game that it must redrawn the screen
     * Arguments: none
     */
    MSGN_EXPOSE,

    /*
     * Send fps and logical fps values to the program
     * Arguments: 2 floats
     */
    MSGB_FPS,
};

#endif

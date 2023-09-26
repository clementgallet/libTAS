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
     * The program sent all messages at the beginning of the frame boundary.
     * Argument: none
     */
    MSGN_START_FRAMEBOUNDARY,

    /*
     * The game sends the frame number, monotonic and realtime
     * Argument: 5 uint64_t
     */
    MSGB_FRAMECOUNT_TIME,

    /*
     * Send all inputs to the game
     * Argument: AllInputs
     */
    MSGN_ALL_INPUTS,

    /*
     * Send all inputs to the game during a frame boundary, so that it can
     * display the inputs in the HUD
     * Argument: AllInputs
     */
    MSGN_PREVIEW_INPUTS,

    /*
     * Send config struct size
     * Argument: int
     */
    MSGN_CONFIG_SIZE,

    /*
     * Send config
     * Argument: struct SharedConfig
     */
    MSGN_CONFIG,

    /*
     * Send initial framecount and time. Non-zero when restarting
     * Argument: 3 uint64_t
     */
    MSGN_INITIAL_FRAMECOUNT_TIME,

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
     * Send a message to be displayed on the game screen
     * Arguments: size_t (string length) then char[len]
     */
    MSGN_OSD_MSG,

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
     * Tells the program that the saving succeeded
     * Argument: none
     */
    MSGB_SAVING_SUCCEEDED,

    /*
     * Tells the program that the loading succeeded
     * Argument: none
     */
    MSGB_LOADING_SUCCEEDED,

    /*
     * Send to the game the path of the savestate
     * Argument: size_t (string length) then char[len]
     */
    MSGN_SAVESTATE_PATH,

    /*
     * Send to the game the path of the base savestate
     * Argument: size_t (string length) then char[len]
     */
    MSGN_BASE_SAVESTATE_PATH,

    /*
     * Send to the game the index of the savestate
     * Argument: int
     */
    MSGN_SAVESTATE_INDEX,

    /*
     * Send to the game the index of the base savestate
     * Argument: int
     */
    MSGN_BASE_SAVESTATE_INDEX,

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

    /*
     * Send ramwatch string to display on OSD
     * Argument: size_t (string length) then char[len]
     */
    MSGN_RAMWATCH,

    /*
     * Send the current segment of video encoding to the program.
     * Argument: int
     */
    MSGB_ENCODING_SEGMENT,

    /*
     * Send the current segment of video encoding to the game.
     * Argument: int
     */
    MSGN_ENCODING_SEGMENT,

    /*
     * Notify that previous savestates have been invalidated because of thread change.
     * Argument: none
     */
    MSGB_INVALIDATE_SAVESTATES,

    /*
     * Send to the game the location of the Steam user data folder.
     * Argument: size_t (string length) then char[len]
     */
    MSGN_STEAM_USER_DATA_PATH,

    /*
     * Send to the game the location of the game's Steam remote storage path.
     * Argument: size_t (string length) then char[len]
     */
    MSGN_STEAM_REMOTE_STORAGE,

    /*
     * Send the git commit hash.
     * Argument: size_t (string length) then char[len]
     */
    MSGB_GIT_COMMIT,

    /*
     * Send the hash and backtrace of a gettime function.
     * Argument: uint64_t then size_t (string length) then char[len]
     */
    MSGB_GETTIME_BACKTRACE,

    /*
     * Indicate that the current frame is a non-draw frame.
     * Argument: None
     */
    MSGB_NONDRAW_FRAME,

    /*
     * Send to the game a text to be displayed from a lua script.
     * Argument: int x, int y, string text, uint32_t fg, uint32_t bg
     */
    MSGN_LUA_TEXT,

    /*
     * Send to the game a pixel to be displayed from a lua script.
     * Argument: int x, int y, uint32_t color
     */
    MSGN_LUA_PIXEL,

    /*
     * Send to the game a rectangle to be displayed from a lua script.
     * Argument: int x, int y, int w, int h, int thickness, uint32_t outline, uint32_t fill
     */
    MSGN_LUA_RECT,

    /*
     * Send to the game a line to be displayed from a lua script.
     * Argument: int x0, int y0, int x1, int y1, uint32_t color
     */
    MSGN_LUA_LINE,

    /*
     * Send to the game a line to be displayed from a lua script.
     * Argument: int center_x, int center_y, int radius_x, int radius_y, uint32_t color
     */
    MSGN_LUA_ELLIPSE,

    /*
     * Ask the game to send the screen resolution.
     * Argument: None
     */
    MSGN_LUA_RESOLUTION,

    /*
     * Send screen resolution to the program
     * Argument: int w, int h
     */
    MSGB_LUA_RESOLUTION,

    /*
     * Send the name of a symbol, to retreive its address in executable
     * Argument: size_t (string length) then char[len]
     */
    MSGB_SYMBOL_ADDRESS,

    /*
     * Send marker string to display on OSD
     * Argument: size_t (string length) then char[len]
     */
    MSGN_MARKER,

};

#endif

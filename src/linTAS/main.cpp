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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/tasflags.h"
#include "../shared/messages.h"
#include "keymapping.h"
#include "recording.h"
#include "SaveState.h"
#include <vector>
#include <string>

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

SaveState savestate;

unsigned long int frame_counter = 0;

char keyboard_state[32];
KeySym hotkeys[HOTKEY_LEN];

char *moviefile = NULL;
FILE* fp;

pid_t game_pid;

std::vector<std::string> shared_libs;

static int MyErrorHandler(Display *display, XErrorEvent *theEvent)
{
    (void) fprintf(stderr,
		   "Ignoring Xlib error: error code %d request code %d\n",
		   theEvent->error_code,
		   theEvent->request_code);

    return 0;
}

int main(int argc, char **argv)
{
    int message;

    /* Parsing arguments */
    int c;
    std::string libname, dumpfile;
    while ((c = getopt (argc, argv, "r:w:d:l:")) != -1)
        switch (c) {
            case 'r':
                /* Playback movie file */
                tasflags.recording = 0;
                moviefile = optarg;
                break;
            case 'w':
                /* Record movie file */
                tasflags.recording = 1;
                moviefile = optarg;
                break;
            case 'd':
                /* Dump video to file */
                tasflags.av_dumping = 1;
                dumpfile = optarg;
                break;
            case 'l':
                /* Shared library */
                libname = optarg;
                shared_libs.push_back(libname);
                break;
            case '?':
                fprintf (stderr, "Unknown option character");
                break;
            default:
                return 1;
        }

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    Display *display;
    XEvent event;
    // Find the window which has the current keyboard focus
    Window gameWindow = 0;
    struct timespec tim;

    XSetErrorHandler(MyErrorHandler);

    /* open connection with the server */
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }


    printf("Connecting to libTAS...\n");

    if (connect(socket_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        printf("Couldn’t connect to socket.\n");
        return 1;
    }

    printf("Connected.\n");

    /* Receive informations from the game */

    recv(socket_fd, &message, sizeof(int), 0);
    while (message != MSGB_END_INIT) {

        switch (message) {
            /* Get the game process pid */
            case MSGB_PID:
                recv(socket_fd, &game_pid, sizeof(pid_t), 0);
                break;

            default:
                fprintf(stderr, "Message init: unknown message\n");
                exit(1);
        }
        recv(socket_fd, &message, sizeof(int), 0);
    }

    /* Send informations to the game */

    /* Send TAS flags */
    message = MSGN_TASFLAGS;
    send(socket_fd, &message, sizeof(int), 0);
    send(socket_fd, &tasflags, sizeof(struct TasFlags), 0);

    /* Send dump file */
    if (tasflags.av_dumping) {
        message = MSGN_DUMP_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t dumpfile_size = dumpfile.size();
        send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
        send(socket_fd, dumpfile.c_str(), dumpfile_size, 0);
    }

    /* Send shared library names */
    for (auto &name : shared_libs) {
        message = MSGN_LIB_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t name_size = name.size();
        send(socket_fd, &name_size, sizeof(size_t), 0);
        send(socket_fd, name.c_str(), name_size, 0);
    }

    /* End message */
    message = MSGN_END_INIT;
    send(socket_fd, &message, sizeof(int), 0);

    tim.tv_sec  = 1;
    tim.tv_nsec = 0L;

    nanosleep(&tim, NULL);

    default_hotkeys(hotkeys);

    if (tasflags.recording >= 0){
        fp = openRecording(moviefile, tasflags.recording);
    }

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every iteration of the do loop
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0 then trigger frame advance
     */
    int ar_ticks = -1;
    int ar_delay = 50;
    int ar_freq = tasflags.fastforward ? 8 : 2;

    while (1)
    {
        
        /* Wait for frame boundary */
        recv(socket_fd, &message, sizeof(int), 0);

        if (message == MSGB_QUIT) {
            printf("Game has quit. Exiting\n");
            break;
        }

        if (message == MSGB_WINDOW_ID) {
            recv(socket_fd, &gameWindow, sizeof(Window), 0);
            if (gameWindow == 0) {
                /* libTAS could not get the window id
                 * Let's get the active window */
                int revert;
                XGetInputFocus(display, &gameWindow, &revert);
            }
            XSelectInput(display, gameWindow, KeyPressMask | KeyReleaseMask | FocusChangeMask);
#if 0
            int iError = XGrabKeyboard(display, gameWindow, 0,
                    GrabModeAsync, GrabModeAsync, CurrentTime); 
            if (iError != GrabSuccess && iError == AlreadyGrabbed) {
                XUngrabPointer(display, CurrentTime);
                XFlush(display);
                fprintf(stderr, "Keyboard is already grabbed\n");    
            }
#endif
            recv(socket_fd, &message, sizeof(int), 0);
        }

        if (message != MSGB_START_FRAMEBOUNDARY) {
            printf("Error in msg socket, waiting for frame boundary\n");
            exit(1);
        }
                   
        recv(socket_fd, &frame_counter, sizeof(unsigned long), 0);


        int isidle = !tasflags.running;
        int tasflagsmod = 0; // register if tasflags have been modified on this frame

        /* If we did not yet receive the game window id, just make the game running */
        if (! gameWindow )
            isidle = 0;

        /* We are at a frame boundary */
        do {

            XQueryKeymap(display, keyboard_state);
           
            /* Implement frame-advance auto-repeat */
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    isidle = 0;
            }

            while( XPending( display ) ) {

                XNextEvent(display, &event);
#if 0
                if (event.type == FocusIn) {
                    fprintf(stderr, "Grabbing window\n");
                    int iError = XGrabKeyboard(display, gameWindow, 0,
                            GrabModeAsync, GrabModeAsync, CurrentTime); 
                    if (iError != GrabSuccess && iError == AlreadyGrabbed) {
                        XUngrabPointer(display, CurrentTime);
                        XFlush(display);
                        fprintf(stderr, "Keyboard is already grabbed\n");    
                    }
                }
                if (event.type == FocusOut) {
                    fprintf(stderr, "Ungrabbing window\n");
                    XUngrabKeyboard(display, CurrentTime);
                }
#endif
                if (event.type == KeyPress)
                {
                    KeyCode kc = ((XKeyPressedEvent*)&event)->keycode;
                    KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);

                    if (ks == hotkeys[HOTKEY_FRAMEADVANCE]){
                        isidle = 0;
                        tasflags.running = 0;
                        tasflagsmod = 1;
                        ar_ticks = 0; // Activate auto-repeat
                    }
                    if (ks == hotkeys[HOTKEY_PLAYPAUSE]){
                        tasflags.running = !tasflags.running;
                        tasflagsmod = 1;
                        isidle = !tasflags.running;
                    }
                    if (ks == hotkeys[HOTKEY_FASTFORWARD]){
                        tasflags.fastforward = 1;
                        tasflagsmod = 1;
                    }
                    if (ks == hotkeys[HOTKEY_SAVESTATE]){
                        savestate.save(game_pid);
                    }
                    if (ks == hotkeys[HOTKEY_LOADSTATE]){
                        savestate.load(game_pid);
                    }
                    if (ks == hotkeys[HOTKEY_READWRITE]){
                        /* TODO: Use enum instead of values */
                        if (tasflags.recording >= 0)
                            tasflags.recording = !tasflags.recording;
                        if (tasflags.recording == 1)
                            truncateRecording(fp);
                        tasflagsmod = 1;
                    }
                }
                if (event.type == KeyRelease)
                {
                    /* 
                     * Detect AutoRepeat key (KeyRelease followed by KeyPress) and skip both
                     * Taken from http://stackoverflow.com/questions/2100654/ignore-auto-repeat-in-x11-applications
                     */
#if 0
                    if (XEventsQueued(display, QueuedAfterReading))
                    {
                        XEvent nev;
                        XPeekEvent(display, &nev);

                        if ((nev.type == KeyPress) && (nev.xkey.time == event.xkey.time) &&
                                (nev.xkey.keycode == event.xkey.keycode))
                        {
                            /* delete retriggered KeyPress event */
                            XNextEvent (display, &event);
                            /* Skip current KeyRelease event */
                            continue;
                        }
                    }
#endif
                    KeyCode kc = ((XKeyPressedEvent*)&event)->keycode;
                    KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
                    if (ks == hotkeys[HOTKEY_FASTFORWARD]){
                        tasflags.fastforward = 0;
                        tasflagsmod = 1;
                    }
                    if (ks == hotkeys[HOTKEY_FRAMEADVANCE]){
                        ar_ticks = -1; // Deactivate auto-repeat
                    }
                }
            }

            /* Sleep a bit to not surcharge the processor */
            if (isidle) {
                tim.tv_sec  = 0;
                tim.tv_nsec = 10000000L;
                nanosleep(&tim, NULL);
            }

        } while (isidle);

        struct AllInputs ai;

        if (tasflags.recording == -1) {
            /* Get keyboard inputs */
            XQueryKeymap(display, keyboard_state);

            /* Format the keyboard state and save it in the AllInputs struct */
            buildAllInputs(&ai, display, keyboard_state, hotkeys);

            /* Get the pointer position and mask */
            if (gameWindow) {
                Window w;
                int i;
                Bool onScreen = XQueryPointer(display, gameWindow, &w, &w, &i, &i, &ai.pointer_x, &ai.pointer_y, &ai.pointer_mask);
                if (!onScreen) {
                    ai.pointer_x = -1;
                    ai.pointer_y = -1;
                }
            }
        }

        if (tasflags.recording == 1) {
            /* Get keyboard inputs */
            XQueryKeymap(display, keyboard_state);

            /* Format the keyboard state and save it in the AllInputs struct */
            buildAllInputs(&ai, display, keyboard_state, hotkeys);

            /* Get the pointer position and mask */
            if (gameWindow) {
                Window w;
                int i;
                Bool onScreen = XQueryPointer(display, gameWindow, &w, &w, &i, &i, &ai.pointer_x, &ai.pointer_y, &ai.pointer_mask);
                if (!onScreen) {
                    ai.pointer_x = -1;
                    ai.pointer_y = -1;
                }
            }

            /* Save inputs to file */
            if (!writeFrame(fp, frame_counter, ai)) {
                /* Writing failed, returning to no recording mode */
                tasflags.recording = -1;
            }
        }

        if (tasflags.recording == 0) {
            /* Save inputs to file */
            if (!readFrame(fp, frame_counter, &ai)) {
                /* Writing failed, returning to no recording mode */
                tasflags.recording = -1;
            }
        }

        /* Send tasflags if modified */
        if (tasflagsmod) {
            message = MSGN_TASFLAGS;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &tasflags, sizeof(struct TasFlags), 0);
        }

        /* Send inputs and end of frame */
        message = MSGN_ALL_INPUTS;
        send(socket_fd, &message, sizeof(int), 0);
        send(socket_fd, &ai, sizeof(struct AllInputs), 0);

        message = MSGN_END_FRAMEBOUNDARY; 
        send(socket_fd, &message, sizeof(int), 0);

    }

    if (tasflags.recording >= 0){
        closeRecording(fp);
    }
    close(socket_fd);
    return 0;
}


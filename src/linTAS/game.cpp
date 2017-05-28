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

#include "game.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"
#include "PseudoSaveState.h"
#include <string>
#include <sstream>
#include <iostream>
#include <signal.h>
#include "ui/MainWindow.h"
#include "MovieFile.h"
#include <cerrno>

#define SOCKET_FILENAME "/tmp/libTAS.socket"

PseudoSaveState pseudosavestate;

static int MyErrorHandler(Display *display, XErrorEvent *theEvent)
{
    // (void) ui_print("Ignoring Xlib error: error code %d request code %d\n",
    //         theEvent->error_code,
    //         theEvent->request_code);

    return 0;
}

static bool haveFocus(Context* context)
{
    if (context->inputs_focus & Context::FOCUS_ALL)
        return true;

    Window window;
    int revert;
    XGetInputFocus(context->display, &window, &revert);

    if ((context->inputs_focus & Context::FOCUS_GAME) &&
        (window == context->game_window))
        return true;

    if ((context->inputs_focus & Context::FOCUS_UI) &&
        (window == context->ui_window))
        return true;

    return false;
}

void launchGame(Context* context)
{
    context->status = Context::ACTIVE;
    MainWindow& ui = MainWindow::getInstance();
    ui.update_status();

    /* Remove the file socket */
    system("rm -f /tmp/libTAS.socket");

    /* Build the system command for calling the game */
    std::ostringstream cmd;

    if (!context->config.libdir.empty())
        cmd << "export LD_LIBRARY_PATH=\"" << context->config.libdir << ":$LD_LIBRARY_PATH\" && ";
    if (!context->config.rundir.empty())
        cmd << "cd " << context->config.rundir << " && ";

    std::string logstr = "";
    if (context->config.sc.logging_status == SharedConfig::NO_LOGGING)
        logstr += "2> /dev/null";
    else if (context->config.sc.logging_status == SharedConfig::LOGGING_TO_FILE) {
        logstr += "2>";
        logstr += context->gamepath;
        logstr += ".log";
    }

    cmd << "LD_PRELOAD=" << context->libtaspath << " " << context->gamepath << " " << context->config.gameargs << logstr << " &";

    // std::cout << "Execute: " << cmd.str() << std::endl;
    system(cmd.str().c_str());

    /* Get the shared libs of the game executable */
    std::vector<std::string> linked_libs;
    std::ostringstream libcmd;
    libcmd << "ldd " << context->gamepath << "  | awk '/=>/{print $(NF-1)}'";
    FILE *libstr;
    //std::cout << "Execute: " << libcmd.str() << std::endl;
    libstr = popen(libcmd.str().c_str(), "r");
    if (libstr != NULL) {
        char buf[1000];
        while (fgets(buf, sizeof buf, libstr) != 0) {
            linked_libs.push_back(std::string(buf));
        }
        pclose(libstr);
    }
    // linked_libs.insert(linked_libs.end(), shared_libs.begin(), shared_libs.end());

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct timespec tim = {1, 0L};

    XSetErrorHandler(MyErrorHandler);
    XAutoRepeatOff(context->display);

    const int MAX_RETRIES = 3;
    int retry = 0;
    // ui_print("Connecting to libTAS...\n");

    nanosleep(&tim, NULL);
    while (connect(socket_fd, reinterpret_cast<const struct sockaddr*>(&addr),
                sizeof(struct sockaddr_un))) {
        // ui_print("Attempt #%i: Couldn't connect to socket.\n", retry + 1);
        retry++;
        if (retry < MAX_RETRIES) {
            // ui_print("Retrying in 2s\n");
            nanosleep(&tim, NULL);
        } else {
            return;
        }
    }

    // ui_print("Attempt #%i: Connected.\n", retry + 1);

    /* Receive informations from the game */

    int message;
    pid_t game_pid;
    recv(socket_fd, &message, sizeof(int), 0);
    while (message != MSGB_END_INIT) {

        switch (message) {
            /* Get the game process pid */
            case MSGB_PID:
                recv(socket_fd, &game_pid, sizeof(pid_t), 0);
                break;

            default:
                // ui_print("Message init: unknown message\n");
                return;
        }
        recv(socket_fd, &message, sizeof(int), 0);
    }

    /* Send informations to the game */

    /* Send shared config */
    message = MSGN_CONFIG;
    send(socket_fd, &message, sizeof(int), 0);
    send(socket_fd, &context->config.sc, sizeof(SharedConfig), 0);

    /* Send dump file if dumping from the beginning */
    if (context->config.sc.av_dumping) {
        message = MSGN_DUMP_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t dumpfile_size = context->config.dumpfile.size();
        send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
        send(socket_fd, context->config.dumpfile.c_str(), dumpfile_size, 0);
    }

    /* Send shared library names */
    for (auto &name : linked_libs) {
        message = MSGN_LIB_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t name_size = name.size();
        send(socket_fd, &name_size, sizeof(size_t), 0);
        send(socket_fd, name.c_str(), name_size, 0);
    }

    /* End message */
    message = MSGN_END_INIT;
    send(socket_fd, &message, sizeof(int), 0);

    nanosleep(&tim, NULL);

    /* Opening a movie, which imports the inputs and parameters if in read mode,
     * or prepare a movie if in write mode. Even if we are in NO_RECORDING mode,
     * we still open a movie to store the input list.
     */
    MovieFile movie;
    movie.open(context);

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every
     * iteration of the do loop.
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0: trigger frame advance
     */
    int ar_ticks = -1;
    int ar_delay = 50;
    int ar_freq = 2;

    /* Unvalidate the game window id */
    context->game_window = 0;

    while (1)
    {
        /* Wait for frame boundary */
        ssize_t ret = recv(socket_fd, &message, sizeof(int), 0);

        while ((ret > 0) && (message != MSGB_QUIT) && (message != MSGB_START_FRAMEBOUNDARY)) {
            void* error_msg;
            switch (message) {
            case MSGB_WINDOW_ID:
                recv(socket_fd, &context->game_window, sizeof(Window), 0);
                if (context->game_window == 0) {
                    /* libTAS could not get the window id
                     * Let's get the active window */
                    int revert;
                    XGetInputFocus(context->display, &context->game_window, &revert);
                }
                /* FIXME: Don't do this if the ui option is unchecked  */
                XSelectInput(context->display, context->game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
                break;

            case MSGB_ERROR_MSG:
                size_t error_len;
                recv(socket_fd, &error_len, sizeof(size_t), 0);
                error_msg = malloc(error_len);
                recv(socket_fd, error_msg, error_len, 0);
                Fl::awake(error_dialog, error_msg);
                /* The error_dialog function frees error_msg */
                break;
            default:
                std::cerr << "Got unknown message!!!" << std::endl;
                return;
            }
            ret = recv(socket_fd, &message, sizeof(int), 0);
        }

        if (ret == -1) {
            std::cerr << "Got a socket error: " << strerror(errno) << std::endl;
            break;
        }

        if (message == MSGB_QUIT) {
            break;
        }

        /* message was MSGB_START_FRAMEBOUNDARY, gathering the frame count */
        recv(socket_fd, &context->framecount, sizeof(unsigned long), 0);
        /* Update frame count in the UI */
        ui.update(false);

        /* Check if we are loading a pseudo savestate */
        if (pseudosavestate.loading) {
            /* When we approach the frame to pause, we disable fastforward so
             * that we draw all the frames.
             */
            if (context->framecount > (pseudosavestate.framecount - 30)) {
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                ui.update(true);
            }

            if (pseudosavestate.framecount == context->framecount) {
                /* We are back to our pseudosavestate frame, we pause the game, disable
                 * fastforward and recover the movie recording mode.
                 */
                pseudosavestate.loading = false;
                context->config.sc.running = false;
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                context->recording = pseudosavestate.recording;
                ui.update(true);
            }
        }

        std::array<char, 32> keyboard_state;

        /* Flag to trigger a frame advance even if the game is on pause */
        bool advance_frame = false;

        /* We are at a frame boundary */
        do {

            /* If we did not yet receive the game window id, just make the game running */
            if (! context->game_window ) {
                break;
            }

            XQueryKeymap(context->display, keyboard_state.data());
            KeySym modifiers = build_modifiers(keyboard_state, context->display);

            /* Implement frame-advance auto-repeat */
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    advance_frame = true;
            }

            while( XPending( context->display ) ) {

                XEvent event;
                XNextEvent(context->display, &event);

                struct HotKey hk;

                if (event.type == FocusOut) {
                    ar_ticks = -1; // Deactivate auto-repeat
                }

                if ((event.type == KeyPress) || (event.type == KeyRelease)) {
                    /* Get the actual pressed/released key */
                    KeyCode kc = event.xkey.keycode;
                    KeySym ks = XkbKeycodeToKeysym(context->display, kc, 0, 0);

                    /* If the key is a modifier, skip it */
                    if (is_modifier(ks))
                        continue;

                    /* Check if this KeySym with or without modifiers is mapped to a hotkey */
                    if (context->config.km.hotkey_mapping.find(ks | modifiers) != context->config.km.hotkey_mapping.end())
                        hk = context->config.km.hotkey_mapping[ks | modifiers];
                    else if (context->config.km.hotkey_mapping.find(ks) != context->config.km.hotkey_mapping.end())
                        hk = context->config.km.hotkey_mapping[ks];
                    else
                        /* This input is not a hotkey, skipping to the next */
                        continue;
                }

                if (event.type == KeyPress)
                {
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        if (context->config.sc.running) {
                            context->config.sc.running = false;
                            ui.update(true);
                            context->config.sc_modified = true;
                        }
                        ar_ticks = 0; // Activate auto-repeat
                        advance_frame = true; // Advance one frame
                    }
                    if (hk.type == HOTKEY_PLAYPAUSE){
                        context->config.sc.running = !context->config.sc.running;
                        ui.update(true);
                        context->config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_FASTFORWARD){
                        context->config.sc.fastforward = true;
                        ui.update(true);
                        context->config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_SAVEPSEUDOSTATE){
                        pseudosavestate.framecount = context->framecount;
                    }
                    if (hk.type == HOTKEY_LOADPSEUDOSTATE){
                        if (pseudosavestate.framecount > 0 && (
                            context->recording == Context::RECORDING_READ_WRITE ||
                            context->recording == Context::RECORDING_WRITE)) {
                            pseudosavestate.loading = true;
                            context->config.sc.running = true;
                            context->config.sc.fastforward = true;
                            context->config.sc_modified = true;
                            pseudosavestate.recording = context->recording;
                            context->recording = Context::RECORDING_READ_WRITE;
                            context->status = Context::QUITTING;
                            ui.update(true);
                            ui.update_status();
                            break;
                        }
                    }
                    if (hk.type == HOTKEY_SAVESTATE){
                        message = MSGN_SAVESTATE;
                        send(socket_fd, &message, sizeof(int), 0);
                    }
                    if (hk.type == HOTKEY_LOADSTATE){
                        message = MSGN_LOADSTATE;
                        send(socket_fd, &message, sizeof(int), 0);
                    }
                    if (hk.type == HOTKEY_READWRITE){
                        switch (context->recording) {
                        case Context::RECORDING_WRITE:
                            context->recording = Context::RECORDING_READ_WRITE;
                            break;
                        case Context::RECORDING_READ_WRITE:
                            context->recording = Context::RECORDING_WRITE;
                            break;
                        default:
                            break;
                        }
                        ui.update(true);
                    }
                }
                if (event.type == KeyRelease)
                {
                    /*
                     * TODO: The following code was supposed to detect the Xlib
                     * AutoRepeat and remove the generated events. Actually,
                     * I can't use it because for some reason, when I press a
                     * key, a KeyRelease is generated at the same time as the
                     * KeyPress event. For this reason, I disable Xlib
                     * AutoRepeat and I use this code to delete the extra
                     * KeyRelease event...
                     * Taken from http://stackoverflow.com/questions/2100654/ignore-auto-repeat-in-x11-applications
                     */
                    if (XEventsQueued(context->display, QueuedAfterReading))
                    {
                        XEvent nev;
                        XPeekEvent(context->display, &nev);

                        if ((nev.type == KeyPress) && (nev.xkey.time == event.xkey.time) &&
                                (nev.xkey.keycode == event.xkey.keycode))
                        {
                            /* delete retriggered KeyPress event */
                            // XNextEvent (display, &event);
                            /* Skip current KeyRelease event */
                            continue;
                        }
                    }

                    if (hk.type == HOTKEY_FASTFORWARD){
                        context->config.sc.fastforward = false;
                        ui.update(true);
                        context->config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        ar_ticks = -1; // Deactivate auto-repeat
                    }
                }
            }

            /* Sleep a bit to not surcharge the processor */
            if (!context->config.sc.running && !advance_frame) {
                tim.tv_sec  = 0;
                tim.tv_nsec = 10000000L;
                nanosleep(&tim, NULL);
            }

        } while (!context->config.sc.running && !advance_frame);

        AllInputs ai;
        ai.emptyInputs();

        /* Record inputs or get inputs from movie file */
        switch (context->recording) {
            case Context::NO_RECORDING:
            case Context::RECORDING_WRITE:

                /* Get inputs if we have input focus */
                if (haveFocus(context)) {
                    /* Get keyboard inputs */
                    XQueryKeymap(context->display, keyboard_state.data());

                    /* Format the keyboard state and save it in the AllInputs struct */
                    context->config.km.buildAllInputs(ai, context->display, keyboard_state, context->config.sc);

                    /* Get the pointer position and mask */
                    if (context->config.sc.mouse_support && context->game_window && haveFocus(context)) {
                        Window w;
                        int i;
                        Bool onScreen = XQueryPointer(context->display, context->game_window, &w, &w, &i, &i, &ai.pointer_x, &ai.pointer_y, &ai.pointer_mask);
                        if (!onScreen) {
                            ai.pointer_x = -1;
                            ai.pointer_y = -1;
                        }
                    }
                }

                /* Save inputs to moviefile */
                movie.setInputs(ai);
                break;

            case Context::RECORDING_READ_WRITE:
            case Context::RECORDING_READ_ONLY:
                /* Read inputs from file */
                if (!movie.getInputs(ai)) {
                    /* TODO: Add an option to decide what to do when movie ends */
                    // movie.saveMovie();
                    // context->recording = Context::NO_RECORDING;
                    // ui.update(true);
                }
                break;
        }

        /* Send shared config if modified */
        if (context->config.sc_modified) {
            /* Send config */
            message = MSGN_CONFIG;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &context->config.sc, sizeof(SharedConfig), 0);
            context->config.sc_modified = false;
        }

        /* Send dump file if modified */
        if (context->config.dumpfile_modified) {
            message = MSGN_DUMP_FILE;
            send(socket_fd, &message, sizeof(int), 0);
            size_t dumpfile_size = context->config.dumpfile.size();
            send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
            send(socket_fd, context->config.dumpfile.c_str(), dumpfile_size, 0);
            context->config.dumpfile_modified = false;
        }

        /* Send inputs and end of frame */
        message = MSGN_ALL_INPUTS;
        send(socket_fd, &message, sizeof(int), 0);
        send(socket_fd, &ai, sizeof(AllInputs), 0);

        if (context->status == Context::QUITTING) {
            message = MSGN_USERQUIT;
            send(socket_fd, &message, sizeof(int), 0);
        }

        message = MSGN_END_FRAMEBOUNDARY;
        send(socket_fd, &message, sizeof(int), 0);

    }

    movie.close();
    close(socket_fd);

    if (pseudosavestate.loading) {
        /* We a loading a pseudo savestate, we need to restart the game */
        context->status = Context::RESTARTING;
        /* Ask the main (UI) thread to call launch_cb, restarting the game */
        Fl::awake(reinterpret_cast<Fl_Awake_Handler>(&launch_cb)); // FIXME: not a good cast
    }
    else {
        /* Unvalidate the pseudo savestate */
        pseudosavestate.framecount = 0;

        context->status = Context::INACTIVE;
        ui.update_status();
    }

    XAutoRepeatOn(context->display);
    XFlush(context->display);

    return;
}

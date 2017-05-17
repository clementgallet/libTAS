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

#include "main.h"
#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"
#include "SaveState.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <signal.h>
#include "Context.h"
// #include "ui.h"
#include "ui/MainWindow.h"
#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include "MovieFile.h"

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

SaveState savestate;

pid_t game_pid;

std::vector<std::string> shared_libs;
std::string libname;

Context context;
MovieFile movie;

static int MyErrorHandler(Display *display, XErrorEvent *theEvent)
{
    // (void) ui_print("Ignoring Xlib error: error code %d request code %d\n",
    //         theEvent->error_code,
    //         theEvent->request_code);

    return 0;
}

void print_usage(void)
{
    std::cout << "Usage: linTAS [options] game_executable_relative_path [game_cmdline_arguments]" << std::endl;
    std::cout << "Options are:" << std::endl;
    std::cout << "  -d, --dump FILE     Start a audio/video encode into the specified FILE" << std::endl;
    std::cout << "  -r, --read MOVIE    Play game inputs from MOVIE file" << std::endl;
    std::cout << "  -w, --write MOVIE   Record game inputs into the specified MOVIE file" << std::endl;
    std::cout << "  -l, --lib     PATH  Manually import a library" << std::endl;
    std::cout << "  -L, --libpath PATH  Indicate a path to additional libraries the game" << std::endl;
    std::cout << "                      will want to import." << std::endl;
    std::cout << "  -R, --runpath PATH  From which directory the game must be launched." << std::endl;
    std::cout << "                      Set to the executable directory by default." << std::endl;
    std::cout << "  -h, --help          Show this message" << std::endl;
}

int main(int argc, char **argv)
{

    /* Parsing arguments */
    int c;
    char buf[PATH_MAX];
    char* abspath;
    std::ofstream o;
    while ((c = getopt (argc, argv, "+r:w:d:l:L:R:h")) != -1)
        switch (c) {
            case 'r':
            case 'w':
                /* Record/Playback movie file */

                /* We must be sure that the file exists, otherwise the following call
                 * to realpath will fail. */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.moviefile = abspath;
                    context.recording = (c == 'r')?Context::RECORDING_READ_WRITE:Context::RECORDING_WRITE;
                }
                break;
            case 'd':
                /* Dump video to file */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.config.sc.av_dumping = true;
                    context.config.dumpfile = abspath;
                }
                break;
            case 'l':
                /* Shared library */
                abspath = realpath(optarg, buf);
                if (abspath) {
                    libname = abspath;
                    shared_libs.push_back(libname);
                }
                break;
            case 'R':
                /* run directory */
                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.rundir = abspath;
                }
                break;
            case 'L':
                /* Shared library directory */
                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.libdir = abspath;
                }
                break;
            case '?':
                std::cout << "Unknown option character" << std::endl;
            case 'h':
                print_usage();
                return 0;
            default:
                return 1;
        }

    /* libTAS.so path */
    /* TODO: Not portable! */
    ssize_t count = readlink( "/proc/self/exe", buf, PATH_MAX );
    std::string binpath = std::string( buf, (count > 0) ? count : 0 );
    char* binpathptr = const_cast<char*>(binpath.c_str());
    context.libtaspath = dirname(binpathptr);
    context.libtaspath += "/libTAS.so"; // Quite bad programming

    /* Game path */
    abspath = realpath(argv[optind], buf);
    if (abspath) {
        context.gamepath = abspath;
    }

    /* Init the a game-specific config, loading a pref file if any */
    if (!context.gamepath.empty())
        context.config.load(context.gamepath);

    /* Game arguments */
    for (int i = optind+1; i < argc; i++) {
        context.config.gameargs += argv[i];
        context.config.gameargs += " ";
    }

    /* Open connection with the server */
    context.display = XOpenDisplay(NULL);
    if (context.display == NULL)
    {
        // ui_print("Cannot open display\n");
        return 1;
    }

    /* Starts the user interface */
    MainWindow& ui = MainWindow::getInstance();
    ui.build(&context);

    /* Start the threaded environnment */
    Fl::lock();

    return Fl::run();
}

void launchGame()
{
    context.status = Context::ACTIVE;
    MainWindow& ui = MainWindow::getInstance();
    ui.update_status();

    /* Remove the file socket */
    system("rm -f /tmp/libTAS.socket");

    /* Build the system command for calling the game */
    std::ostringstream cmd;

    if (!context.libdir.empty())
        cmd << "export LD_LIBRARY_PATH=\"" << context.libdir << ":$LD_LIBRARY_PATH\" && ";
    if (!context.rundir.empty())
        cmd << "cd " << context.rundir << " && ";

    std::string logstr = "";
    if (context.config.sc.logging_status == SharedConfig::NO_LOGGING)
        logstr += "2> /dev/null";
    else if (context.config.sc.logging_status == SharedConfig::LOGGING_TO_FILE) {
        logstr += "2>";
        logstr += context.gamepath;
        logstr += ".log";
    }

    cmd << "LD_PRELOAD=" << context.libtaspath << " " << context.gamepath << " " << context.config.gameargs << logstr << " &";

    // std::cout << "Execute: " << cmd.str() << std::endl;
    system(cmd.str().c_str());

    /* Get the shared libs of the game executable */
    std::vector<std::string> linked_libs;
    std::ostringstream libcmd;
    libcmd << "ldd " << context.gamepath << "  | awk '/=>/{print $(NF-1)}'";
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
    linked_libs.insert(linked_libs.end(), shared_libs.begin(), shared_libs.end());

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    XEvent event;

    struct timespec tim;

    XSetErrorHandler(MyErrorHandler);
    XAutoRepeatOff(context.display);

    const int MAX_RETRIES = 3;
    int retry = 0;
    // ui_print("Connecting to libTAS...\n");

    while (connect(socket_fd, reinterpret_cast<const struct sockaddr*>(&addr),
                sizeof(struct sockaddr_un))) {
        // ui_print("Attempt #%i: Couldn't connect to socket.\n", retry + 1);
        retry++;
        if (retry < MAX_RETRIES) {
            // ui_print("Retrying in 2s\n");
            sleep(2);
        } else {
            return;
        }
    }

    // ui_print("Attempt #%i: Connected.\n", retry + 1);

    /* Receive informations from the game */

    int message;
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
    send(socket_fd, &context.config.sc, sizeof(SharedConfig), 0);

    /* Send dump file if dumping from the beginning */
    if (context.config.sc.av_dumping) {
        message = MSGN_DUMP_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t dumpfile_size = context.config.dumpfile.size();
        send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
        send(socket_fd, context.config.dumpfile.c_str(), dumpfile_size, 0);
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

    tim.tv_sec  = 1;
    tim.tv_nsec = 0L;

    nanosleep(&tim, NULL);

    if (context.recording != Context::NO_RECORDING) {
        movie.open(&context);
    }

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every iteration of the do loop
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0 then trigger frame advance
     */
    int ar_ticks = -1;
    int ar_delay = 50;
    int ar_freq = context.config.sc.fastforward ? 8 : 2;

    /* Unvalidate the game window id */
    context.game_window = 0;

    while (1)
    {

        /* Wait for frame boundary */
        recv(socket_fd, &message, sizeof(int), 0);

        while ((message != MSGB_QUIT) && (message != MSGB_START_FRAMEBOUNDARY)) {
            void* error_msg;
            switch (message) {
            case MSGB_WINDOW_ID:
                recv(socket_fd, &context.game_window, sizeof(Window), 0);
                if (context.game_window == 0) {
                    /* libTAS could not get the window id
                     * Let's get the active window */
                    int revert;
                    XGetInputFocus(context.display, &context.game_window, &revert);
                }
                /* FIXME: Don't do this if the ui option is unchecked  */
                XSelectInput(context.display, context.game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
                // XSelectInput(context.display, context.game_window, KeyPressMask);
    #if 0
                int iError = XGrabKeyboard(display, context.game_window, 0,
                        GrabModeAsync, GrabModeAsync, CurrentTime);
                if (iError != GrabSuccess && iError == AlreadyGrabbed) {
                    XUngrabPointer(display, CurrentTime);
                    XFlush(display);
                    fprintf(stderr, "Keyboard is already grabbed\n");
                }
    #endif
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
                std::cout << "Got unknown message!!!" << std::endl;
                return;
            }
            recv(socket_fd, &message, sizeof(int), 0);
        }

        if (message == MSGB_QUIT) {
            break;
        }

        recv(socket_fd, &context.framecount, sizeof(unsigned long), 0);
        /* Update frame count in the UI */
        ui.update(false);

        std::array<char, 32> keyboard_state;

        /* Flag to trigger a frame advance even if the game is on pause */
        bool advance_frame = false;

        /* We are at a frame boundary */
        do {

            /* If we did not yet receive the game window id, just make the game running */
            if (! context.game_window ) {
                break;
            }

            XQueryKeymap(context.display, keyboard_state.data());
            KeySym modifiers = build_modifiers(keyboard_state, context.display);

            /* Implement frame-advance auto-repeat */
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    advance_frame = true;
            }

            while( XPending( context.display ) ) {

                XNextEvent(context.display, &event);

                struct HotKey hk;

                if (event.type == FocusOut) {
                    ar_ticks = -1; // Deactivate auto-repeat
                }

                if ((event.type == KeyPress) || (event.type == KeyRelease)) {
                    /* Get the actual pressed/released key */
                    KeyCode kc = event.xkey.keycode;
                    KeySym ks = XkbKeycodeToKeysym(context.display, kc, 0, 0);

                    /* If the key is a modifier, skip it */
                    if (is_modifier(ks))
                        continue;

                    /* Check if this KeySym with or without modifiers is mapped to a hotkey */
                    if (context.config.km.hotkey_mapping.find(ks | modifiers) != context.config.km.hotkey_mapping.end())
                        hk = context.config.km.hotkey_mapping[ks | modifiers];
                    else if (context.config.km.hotkey_mapping.find(ks) != context.config.km.hotkey_mapping.end())
                        hk = context.config.km.hotkey_mapping[ks];
                    else
                        /* This input is not a hotkey, skipping to the next */
                        continue;
                }

                if (event.type == KeyPress)
                {
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        if (context.config.sc.running) {
                            context.config.sc.running = false;
                            ui.update(true);
                            context.config.sc_modified = true;
                        }
                        ar_ticks = 0; // Activate auto-repeat
                        advance_frame = true; // Advance one frame
                    }
                    if (hk.type == HOTKEY_PLAYPAUSE){
                        context.config.sc.running = !context.config.sc.running;
                        ui.update(true);
                        context.config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_FASTFORWARD){
                        context.config.sc.fastforward = true;
                        ui.update(true);
                        context.config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_SAVESTATE){
                        savestate.save(game_pid);
                    }
                    if (hk.type == HOTKEY_LOADSTATE){
                        savestate.load(game_pid);
                    }
                    if (hk.type == HOTKEY_READWRITE){
                        switch (context.recording) {
                        case Context::RECORDING_WRITE:
                            context.recording = Context::RECORDING_READ_WRITE;
                            break;
                        case Context::RECORDING_READ_WRITE:
                            context.recording = Context::RECORDING_WRITE;
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
                    if (XEventsQueued(context.display, QueuedAfterReading))
                    {
                        XEvent nev;
                        XPeekEvent(context.display, &nev);

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
                        context.config.sc.fastforward = false;
                        ui.update(true);
                        context.config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        ar_ticks = -1; // Deactivate auto-repeat
                    }
                }
            }

            /* Sleep a bit to not surcharge the processor */
            if (!context.config.sc.running && !advance_frame) {
                tim.tv_sec  = 0;
                tim.tv_nsec = 10000000L;
                nanosleep(&tim, NULL);
            }

        } while (!context.config.sc.running && !advance_frame);

        AllInputs ai;

        /* Record inputs or get inputs from movie file */
        switch (context.recording) {
            case Context::NO_RECORDING:
            case Context::RECORDING_WRITE:

                /* Get keyboard inputs */
                XQueryKeymap(context.display, keyboard_state.data());

                /* Format the keyboard state and save it in the AllInputs struct */
                context.config.km.buildAllInputs(ai, context.display, keyboard_state, context.config.sc);

                /* Get the pointer position and mask */
                if (context.config.sc.mouse_support && context.game_window) {
                    Window w;
                    int i;
                    Bool onScreen = XQueryPointer(context.display, context.game_window, &w, &w, &i, &i, &ai.pointer_x, &ai.pointer_y, &ai.pointer_mask);
                    if (!onScreen) {
                        ai.pointer_x = -1;
                        ai.pointer_y = -1;
                    }
                }

                if (context.recording == Context::NO_RECORDING)
                    break;

                /* Save inputs to file */
                movie.setInputs(ai);
                break;

            case Context::RECORDING_READ_WRITE:
            case Context::RECORDING_READ_ONLY:
                /* Read inputs from file */
                if (!movie.getInputs(ai)) {
                    /* Reading failed, returning to no recording mode */
                    std::cout << "Reading failed" << std::endl;
                    movie.close();
                    context.recording = Context::NO_RECORDING;
                    ui.update(true);
                }
                break;
        }

        /* Send shared config if modified */
        if (context.config.sc_modified) {
            /* Send config */
            message = MSGN_CONFIG;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &context.config.sc, sizeof(SharedConfig), 0);
            context.config.sc_modified = false;
        }

        /* Send dump file if modified */
        if (context.config.dumpfile_modified) {
            message = MSGN_DUMP_FILE;
            send(socket_fd, &message, sizeof(int), 0);
            size_t dumpfile_size = context.config.dumpfile.size();
            send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
            send(socket_fd, context.config.dumpfile.c_str(), dumpfile_size, 0);
            context.config.dumpfile_modified = false;
        }

        /* Send inputs and end of frame */
        message = MSGN_ALL_INPUTS;
        send(socket_fd, &message, sizeof(int), 0);
        send(socket_fd, &ai, sizeof(AllInputs), 0);

        if (context.status == Context::QUITTING) {
            message = MSGN_USERQUIT;
            send(socket_fd, &message, sizeof(int), 0);
        }

        message = MSGN_END_FRAMEBOUNDARY;
        send(socket_fd, &message, sizeof(int), 0);

    }

    if (context.recording != Context::NO_RECORDING){
        movie.close();
    }
    close(socket_fd);

    context.status = Context::INACTIVE;
    ui.update_status();

    XAutoRepeatOn(context.display);

    return;
}

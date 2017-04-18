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
#include "../shared/tasflags.h"
#include "../shared/messages.h"
#include "recording.h"
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

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

SaveState savestate;

pid_t game_pid;

std::vector<std::string> shared_libs;
std::string libname;

Context context;

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
    /* TODO: this is weird, remove the extern for default values */
    context.tasflags = tasflags; // To get the default values.
    context.config = config; // To get the default values.

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
                    context.moviefile = abspath;
                    context.tasflags.recording = (c == 'r')?TasFlags::RECORDING_READ_WRITE:TasFlags::RECORDING_WRITE;
                }
                break;
            case 'd':
                /* Dump video to file */
                o.open(optarg);
                o.close();

                abspath = realpath(optarg, buf);
                if (abspath) {
                    context.tasflags.av_dumping = 1;
                    context.dumpfile = abspath;
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

    /* Fill a movie name if empty */
    if ( (!context.gamepath.empty()) && context.moviefile.empty()) {
        context.moviefile = context.gamepath + ".ltm";
        context.tasflags.recording = TasFlags::NO_RECORDING;
    }

    /* Game arguments */
    for (int i = optind+1; i < argc; i++) {
        context.gameargs += argv[i];
        context.gameargs += " ";
    }

    /* Starts the user interface */
    MainWindow& ui = MainWindow::getInstance();
    ui.build(&context);

    /* Start the threaded environnment */
    Fl::lock();

    return Fl::run();
}

void* launchGame(void* arg)
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
    if (context.tasflags.logging_status == TasFlags::NO_LOGGING)
        logstr += "2> /dev/null";
    else if (context.tasflags.logging_status == TasFlags::LOGGING_TO_FILE) {
        logstr += "2>";
        logstr += context.gamepath;
        logstr += ".log";
    }

    cmd << "LD_PRELOAD=" << context.libtaspath << " " << context.gamepath << " " << context.gameargs << logstr << " &";

    // std::cout << "Execute: " << cmd.str() << std::endl;
    system(cmd.str().c_str());

    /* Get the shared libs of the game executable */
    std::ostringstream libcmd;
    libcmd << "ldd " << context.gamepath << "  | awk '/=>/{print $(NF-1)}'";
    FILE *libstr;
    //std::cout << "Execute: " << libcmd.str() << std::endl;
    libstr = popen(libcmd.str().c_str(), "r");
    if (libstr != NULL) {
        char buf[1000];
        while (fgets(buf, sizeof buf, libstr) != 0) {
            shared_libs.push_back(std::string(buf));
        }
        pclose(libstr);
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
        // ui_print("Cannot open display\n");
        return nullptr;
    }

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
            return nullptr;
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
                return nullptr;
        }
        recv(socket_fd, &message, sizeof(int), 0);
    }

    /* Send informations to the game */

    /* Send TAS flags */
    message = MSGN_TASFLAGS;
    send(socket_fd, &message, sizeof(int), 0);
    send(socket_fd, &context.tasflags, sizeof(struct TasFlags), 0);

    /* Send config */
    message = MSGN_CONFIG;
    send(socket_fd, &message, sizeof(int), 0);
    send(socket_fd, &context.config, sizeof(struct Config), 0);


    /* Send dump file if dumping from the beginning */
    if (context.tasflags.av_dumping) {
        message = MSGN_DUMP_FILE;
        send(socket_fd, &message, sizeof(int), 0);
        size_t dumpfile_size = context.dumpfile.size();
        send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
        send(socket_fd, context.dumpfile.c_str(), dumpfile_size, 0);
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

    FILE* fp;
    if (context.tasflags.recording != TasFlags::NO_RECORDING){
        fp = openRecording(context.moviefile.c_str(), context.tasflags.recording);
    }

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every iteration of the do loop
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0 then trigger frame advance
     */
    int ar_ticks = -1;
    int ar_delay = 50;
    int ar_freq = context.tasflags.fastforward ? 8 : 2;

    while (1)
    {

        /* Wait for frame boundary */
        recv(socket_fd, &message, sizeof(int), 0);

        if (message == MSGB_QUIT) {
            // ui_print("Game has quit. Exiting\n");
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
            std::cout << "Got unknown message!!!" << std::endl;
            // ui_print("Error in msg socket, waiting for frame boundary\n");
            return nullptr;
        }

        recv(socket_fd, &context.framecount, sizeof(unsigned long), 0);
        /* Update frame count in the UI */
        ui.update(false);

        //int tasflagsmod = 0; // register if tasflags have been modified on this frame

        char keyboard_state[32];

        /* Flag to trigger a frame advance even if the game is on pause */
        bool advance_frame = false;

        /* We are at a frame boundary */
        do {

            /* If we did not yet receive the game window id, just make the game running */
            if (! gameWindow ) {
                break;
            }

            XQueryKeymap(display, keyboard_state);

            /* Implement frame-advance auto-repeat */
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    advance_frame = true;
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
                struct HotKey hk;

                if ((event.type == KeyPress) || (event.type == KeyRelease)) {
                    /* Check if the key pressed/released is a hotkey */
                    KeyCode kc = event.xkey.keycode;
                    KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);

                    if (context.km.hotkey_mapping.find(ks) != context.km.hotkey_mapping.end())
                        hk = context.km.hotkey_mapping[ks];
                    else
                        /* This input is not a hotkey, skipping to the next */
                        continue;
                }

                if (event.type == KeyPress)
                {
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        if (context.tasflags.running == 1) {
                            context.tasflags.running = 0;
                            ui.update(true);
                            context.tasflags_modified = true;
                        }
                        //ar_ticks = 0; // Activate auto-repeat
                        advance_frame = true; // Advance one frame
                    }
                    if (hk.type == HOTKEY_PLAYPAUSE){
                        context.tasflags.running = !context.tasflags.running;
                        ui.update(true);
                        context.tasflags_modified = true;
                    }
                    if (hk.type == HOTKEY_FASTFORWARD){
                        context.tasflags.fastforward = 1;
                        ui.update(true);
                        context.tasflags_modified = true;
                    }
                    if (hk.type == HOTKEY_SAVESTATE){
                        savestate.save(game_pid);
                    }
                    if (hk.type == HOTKEY_LOADSTATE){
                        savestate.load(game_pid);
                    }
                    if (hk.type == HOTKEY_READWRITE){
                        switch (context.tasflags.recording) {
                            case TasFlags::RECORDING_WRITE:
                                context.tasflags.recording = TasFlags::RECORDING_READ_WRITE;
                                context.tasflags_modified = true;
                                ui.update(true);
                                break;
                            case TasFlags::RECORDING_READ_WRITE:
                                context.tasflags.recording = TasFlags::RECORDING_WRITE;
                                context.tasflags_modified = true;
                                ui.update(true);
                                truncateRecording(fp);
                                break;
                        }
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
                    if (hk.type == HOTKEY_FASTFORWARD){
                        context.tasflags.fastforward = 0;
                        ui.update(true);
                        context.tasflags_modified = true;
                    }
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        ar_ticks = -1; // Deactivate auto-repeat
                        /* FIXME: If the game window looses focus,
                         * the key release event is never sent,
                         * so the auto-repeat is still activated
                         */
                    }
                }
            }


//            if (context.tasflags_modified) {
                /* Show the new flags to the UI */
//            }

            /* Sleep a bit to not surcharge the processor */
            if (!context.tasflags.running && !advance_frame) {
                tim.tv_sec  = 0;
                tim.tv_nsec = 10000000L;
                nanosleep(&tim, NULL);
            }

        } while (!context.tasflags.running && !advance_frame);

        AllInputs ai;

        /* Record inputs or get inputs from movie file */
        switch (context.tasflags.recording) {
            case TasFlags::NO_RECORDING:
            case TasFlags::RECORDING_WRITE:

                /* Get keyboard inputs */
                XQueryKeymap(display, keyboard_state);

                /* Format the keyboard state and save it in the AllInputs struct */
                context.km.buildAllInputs(ai, display, keyboard_state);

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

                if (context.tasflags.recording == TasFlags::NO_RECORDING)
                    break;

                /* Save inputs to file */
                if (!writeFrame(fp, context.framecount, ai)) {
                    /* Writing failed, returning to no recording mode */
                    context.tasflags.recording = TasFlags::NO_RECORDING;
                    closeRecording(fp);
                    ui.update(true);
                }

                break;

            case TasFlags::RECORDING_READ_WRITE:
            case TasFlags::RECORDING_READ_ONLY:
                /* Read inputs from file */
                if (!readFrame(fp, context.framecount, &ai)) {
                    /* Reading failed, returning to no recording mode */
                    std::cout << "Reading failed" << std::endl;
                    closeRecording(fp);
                    context.tasflags.recording = TasFlags::NO_RECORDING;
                    ui.update(true);
                }
                break;
        }

        /* Send tasflags if modified */
        if (context.tasflags_modified) {
            message = MSGN_TASFLAGS;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &context.tasflags, sizeof(struct TasFlags), 0);
            context.tasflags_modified = false;
        }

        /* Send config if modified */
        if (context.config_modified) {
            /* Send config */
            message = MSGN_CONFIG;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &context.config, sizeof(struct Config), 0);
            context.config_modified = false;
        }

        /* Send dump file if modified */
        if (context.dumpfile_modified) {
            message = MSGN_DUMP_FILE;
            send(socket_fd, &message, sizeof(int), 0);
            size_t dumpfile_size = context.dumpfile.size();
            send(socket_fd, &dumpfile_size, sizeof(size_t), 0);
            send(socket_fd, context.dumpfile.c_str(), dumpfile_size, 0);
            context.dumpfile_modified = false;
        }

        /* Send inputs and end of frame */
        message = MSGN_ALL_INPUTS;
        send(socket_fd, &message, sizeof(int), 0);
        send(socket_fd, &ai, sizeof(struct AllInputs), 0);

        if (context.status == Context::QUITTING) {
            message = MSGN_USERQUIT;
            send(socket_fd, &message, sizeof(int), 0);
        }

        message = MSGN_END_FRAMEBOUNDARY;
        send(socket_fd, &message, sizeof(int), 0);

    }

    if (context.tasflags.recording != TasFlags::NO_RECORDING){
        closeRecording(fp);
    }
    close(socket_fd);

    context.status = Context::INACTIVE;
    ui.update_status();

    return nullptr;
}

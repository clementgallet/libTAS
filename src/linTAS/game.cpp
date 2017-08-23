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
#include "../shared/sockethelpers.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"
#include "PseudoSaveState.h"
#include <string>
#include <sstream>
#include <iostream>
#include "ui/MainWindow.h"
#include "MovieFile.h"
#include <cerrno>
#include "utils.h"
#include <unistd.h> // fork()
#include <fcntl.h> // O_RDWR, O_CREAT

static PseudoSaveState pseudosavestate;

/* Determine if we are allowed to send inputs to the game, based on which
 * window has focus and our settings
 */
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

/* Set the different environment variables, then start the game executable with
 * our library to be injected using the LD_PRELOAD trick.
 * Because this function eventually calls execl, it does not return.
 * So, it is called from a child process using fork().
 */
static void executeGame(Context* context)
{
    /* Update the LD_LIBRARY_PATH environment variable if the user set one */
    if (!context->config.libdir.empty()) {
        char* oldlibpath = getenv("LD_LIBRARY_PATH");
        std::string libpath = context->config.libdir;
        if (oldlibpath) {
            libpath.append(":");
            libpath.append(oldlibpath);
        }
        setenv("LD_LIBRARY_PATH", libpath.c_str(), 1);
    }

    /* Change the working directory if the user set one */
    if (!context->config.rundir.empty())
        chdir(context->config.rundir.c_str());

    /* Set where stderr of the game is redirected */
    int fd;
    std::string logfile = context->gamepath + ".log";
    switch(context->config.sc.logging_status) {
        case SharedConfig::NO_LOGGING:
            fd = open("/dev/null", O_RDWR, S_IRUSR | S_IWUSR);
            dup2(fd, 2);
            close(fd);
            break;
        case SharedConfig::LOGGING_TO_FILE:
            fd = open(logfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(fd, 2);
            close(fd);
            break;
        case SharedConfig::LOGGING_TO_CONSOLE:
        default:
            break;
    }

    /* Set additional environment variables regarding Mesa configuration */
    if (context->config.opengl_soft)
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    else
        unsetenv("LIBGL_ALWAYS_SOFTWARE");

    if (!context->config.llvm_perf.empty())
        setenv("LP_PERF", context->config.llvm_perf.c_str(), 1);
    else
        unsetenv("LP_PERF");

    /* Run the actual game */
    if (context->attach_gdb) {
        /* Call the game using gdb. The LD_PRELOAD must be set inside gdb,
         * so building a command to be executed at the start of gdb.
         */
        std::string ldpreloadstr = "set exec-wrapper env 'LD_PRELOAD=";
        ldpreloadstr += context->libtaspath;
        ldpreloadstr += "'";

        execl("/usr/bin/gdb", "/usr/bin/gdb", "-q",
            "-ex", ldpreloadstr.c_str(),
            /* We are using SIGUSR1 and SIGUSR2 for savestates, so don't
             * print and pause when one signal is sent */
            "-ex", "handle SIGUSR1 nostop noprint",
            "-ex", "handle SIGUSR2 nostop noprint",
            "-ex", "run",
            context->gamepath.c_str(),
            (char*) NULL);
    }
    else {
        /* Set the LD_PRELOAD environment variable to inject our lib to the game */
        setenv("LD_PRELOAD", context->libtaspath.c_str(), 1);

        execl(context->gamepath.c_str(), context->gamepath.c_str(), context->config.gameargs.c_str(), NULL);
    }
}

void launchGame(Context* context)
{
    /* Unvalidate the game window id */
    context->game_window = 0;

    /* Reset the rerecord count */
    context->rerecord_count = 0;

    /* Extract the game executable name from the game executable path */
    size_t sep = context->gamepath.find_last_of("/");
    if (sep != std::string::npos)
        context->gamename = context->gamepath.substr(sep + 1);
    else
        context->gamename = context->gamepath;

    context->status = Context::ACTIVE;
    MainWindow& ui = MainWindow::getInstance();
    ui.update_status();

    /* Remove savestates again in case we did not exist cleanly the previous time */
    remove_savestates(context);

    /* Remove the file socket */
    removeSocket();

    /* Get the shared libs of the game executable */
    std::vector<std::string> linked_libs;
    std::ostringstream libcmd;
    libcmd << "ldd " << context->gamepath << "  | awk '/=>/{print $(NF-1)}'";
    //std::cout << "Execute: " << libcmd.str() << std::endl;

    FILE *libstr = popen(libcmd.str().c_str(), "r");
    if (libstr != NULL) {
        std::array<char,1000> buf;
        while (fgets(buf.data(), buf.size(), libstr) != 0) {
            std::string lib(buf.data());
            /* Remove trailing newline if any */
            if (lib.back() == '\n')
                lib.pop_back();
            linked_libs.push_back(lib);
        }
        pclose(libstr);
    }
    // linked_libs.insert(linked_libs.end(), shared_libs.begin(), shared_libs.end());

    /* We fork here so that the child process calls the game */
    int pid = fork();
    if (pid == 0) {
        executeGame(context);
    }

    /* Connect to the socket between the program and the game */
    initSocketProgram();

    XAutoRepeatOff(context->display);

    /* Receive informations from the game */

    int message = receiveMessage();
    while (message != MSGB_END_INIT) {

        switch (message) {
            /* Get the game process pid */
            case MSGB_PID:
                receiveData(&context->game_pid, sizeof(pid_t));
                break;

            default:
                // ui_print("Message init: unknown message\n");
                return;
        }
        message = receiveMessage();
    }

    /* Opening a movie, which imports the inputs and parameters if in read mode,
     * or prepare a movie if in write mode.
     */
    MovieFile movie(context);
    if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
        movie.loadMovie();
        context->config.sc.movie_framecount = movie.nbFrames();
    }
    ui.update_rerecordcount();

    /* Send informations to the game */

    /* Send shared config */
    sendMessage(MSGN_CONFIG);
    sendData(&context->config.sc, sizeof(SharedConfig));

    /* Send dump file if dumping from the beginning */
    if (context->config.sc.av_dumping) {
        sendMessage(MSGN_DUMP_FILE);
        sendString(context->config.dumpfile);
    }

    /* Send shared library names */
    for (auto &name : linked_libs) {
        sendMessage(MSGN_LIB_FILE);
        sendString(name);
    }

    /* End message */
    sendMessage(MSGN_END_INIT);

    /* Keep track of the last savestate loaded. This will save us from loading
     * a moviefile if we don't have to.
     */
    int last_savestate_slot = -1;

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every
     * iteration of the do loop.
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0: trigger frame advance
     */
    int ar_ticks = -1;
    int ar_delay = 50;
    int ar_freq = 2;

    while (1)
    {
        /* Wait for frame boundary */
        message = receiveMessage();

        while ((message >= 0) && (message != MSGB_QUIT) && (message != MSGB_START_FRAMEBOUNDARY)) {
            // void* alert_msg;
            std::string* alert_str;
            switch (message) {
            case MSGB_WINDOW_ID:
                receiveData(&context->game_window, sizeof(Window));
                if (context->game_window == 0) {
                    /* libTAS could not get the window id
                     * Let's get the active window */
                    int revert;
                    XGetInputFocus(context->display, &context->game_window, &revert);
                }
                /* FIXME: Don't do this if the ui option is unchecked  */
                XSelectInput(context->display, context->game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
                break;

            case MSGB_ALERT_MSG:
                /* Ask the UI thread to display the alert. He is in charge of
                 * freeing the string.
                 */
                alert_str = new std::string;
                *alert_str = receiveString();
                Fl::awake(alert_dialog, alert_str);
                break;
            case MSGB_ENCODE_FAILED:
                context->config.sc.av_dumping = false;
                context->config.sc_modified = true;
                ui.update_ui();
                break;
            case MSGB_FRAMECOUNT_TIME:
                receiveData(&context->framecount, sizeof(unsigned long));
                receiveData(&context->current_time, sizeof(struct timespec));
                ui.update_framecount_time();
                break;
            default:
                std::cerr << "Got unknown message!!!" << std::endl;
                return;
            }
            message = receiveMessage();
        }

        if (message == -1) {
            std::cerr << "Got a socket error: " << strerror(errno) << std::endl;
            break;
        }

        if (message == MSGB_QUIT) {
            break;
        }

        /* Check if we are loading a pseudo savestate */
        if (pseudosavestate.loading) {
            /* When we approach the frame to pause, we disable fastforward so
             * that we draw all the frames.
             */
            if (context->framecount > (pseudosavestate.framecount - 30)) {
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                ui.update_ui();
            }

            if (pseudosavestate.framecount == context->framecount) {
                /* We are back to our pseudosavestate frame, we pause the game, disable
                 * fastforward and recover the movie recording mode.
                 */
                pseudosavestate.loading = false;
                context->config.sc.running = false;
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                context->config.sc.recording = pseudosavestate.recording;
                ui.update_ui();
            }
        }

        /* Flag to trigger a frame advance even if the game is on pause */
        bool advance_frame = false;

        /* We are at a frame boundary */
        do {
            /* If we did not yet receive the game window id, just make the game running */
            if (! context->game_window ) {
                break;
            }

            std::array<char, 32> keyboard_state;
            XQueryKeymap(context->display, keyboard_state.data());
            KeySym modifiers = build_modifiers(keyboard_state, context->display);

            /* Implement frame-advance auto-repeat */
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    advance_frame = true;
            }

            while( XPending( context->display ) || !context->hotkey_queue.empty() ) {

                XEvent event;
                struct HotKey hk;

                if (!context->hotkey_queue.empty()) {
                    /* Processing a hotkey pushed by the UI */
                    context->hotkey_queue.pop(hk.type);
                    event.type = KeyPress;
                }
                else {
                    /* Processing a hotkey pressed by the user */
                    XNextEvent(context->display, &event);

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
                }

                if (event.type == KeyPress)
                {

                    /* Advance a frame */
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        if (context->config.sc.running) {
                            context->config.sc.running = false;
                            ui.update_ui();
                            context->config.sc_modified = true;
                        }
                        ar_ticks = 0; // Activate auto-repeat
                        advance_frame = true; // Advance one frame
                    }

                    /* Toggle between play and pause */
                    if (hk.type == HOTKEY_PLAYPAUSE){
                        context->config.sc.running = !context->config.sc.running;
                        ui.update_ui();
                        context->config.sc_modified = true;
                    }

                    /* Enable fastforward */
                    if (hk.type == HOTKEY_FASTFORWARD){
                        context->config.sc.fastforward = true;
                        ui.update_ui();
                        context->config.sc_modified = true;
                    }

                    /* Save a pseudo-savestate (register position in the movie) */
                    if (hk.type == HOTKEY_SAVEPSEUDOSTATE){
                        pseudosavestate.framecount = context->framecount;
                    }

                    /* Load a pseudo-savestate:
                     * - trigger a game restart
                     * - playback the movie in fastforward
                     * - stop the movie at the registered frame
                     * FIXME: was not tested when real savestates start to work,
                     * so may not work anymore. I'm unsure if I should keep this
                     * or not.
                     */
                    if (hk.type == HOTKEY_LOADPSEUDOSTATE){
                        if (pseudosavestate.framecount > 0 && (
                            context->config.sc.recording == SharedConfig::RECORDING_READ ||
                            context->config.sc.recording == SharedConfig::RECORDING_WRITE)) {
                            pseudosavestate.loading = true;
                            context->config.sc.running = true;
                            context->config.sc.fastforward = true;
                            context->config.sc_modified = true;
                            pseudosavestate.recording = context->config.sc.recording;
                            context->config.sc.recording = SharedConfig::RECORDING_READ;
                            context->status = Context::QUITTING;
                            ui.update_ui();
                            ui.update_status();
                            break;
                        }
                    }

                    /* Perform a savestate:
                     * - save the moviefile if we are recording
                     * - tell the game to save its state
                     */
                    if (hk.type >= HOTKEY_SAVESTATE1 && hk.type <= HOTKEY_SAVESTATE9){
                        /* Saving is not allowed if currently encoding */
                        if (context->config.sc.av_dumping) {
                            std::string* alert_str = new std::string("Saving is not allowed when in the middle of video encoding");
                            Fl::awake(alert_dialog, alert_str);
                            continue;
                        }

                        /* Slot number */
                        int statei = hk.type - HOTKEY_SAVESTATE1 + 1;
                        last_savestate_slot = statei;

                        if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
                            /* Building the movie path */
                            std::string moviepath = context->config.savestatedir + '/';
                            moviepath += context->gamename;
                            moviepath += ".movie" + std::to_string(statei) + ".ltm";

                            /* Save the movie file */
                            movie.saveMovie(moviepath, context->framecount);
                        }

                        /* Building the savestate path */
                        std::string savestatepath = context->config.savestatedir + '/';
                        savestatepath += context->gamename;
                        savestatepath += ".state" + std::to_string(statei);

                        sendMessage(MSGN_SAVESTATE);
                        sendString(savestatepath);
                    }

                    /* Load a savestate:
                     * - check for an existing savestate in the slot
                     * - if in read-only move, we must check that the movie
                         associated with the savestate must be a prefix of the
                         current movie
                     * - tell the game to load its state
                     * - if loading succeeded:
                     * -- send the shared config
                     * -- increment the rerecord count
                     * -- receive the frame count and the current time
                     */
                    if (hk.type >= HOTKEY_LOADSTATE1 && hk.type <= HOTKEY_LOADSTATE9){
                        /* Loading is not allowed if currently encoding */
                        if (context->config.sc.av_dumping) {
                            std::string* alert_str = new std::string("Loading is not allowed when in the middle of video encoding");
                            Fl::awake(alert_dialog, alert_str);
                            continue;
                        }

                        /* Slot number */
                        int statei = hk.type - HOTKEY_LOADSTATE1 + 1;

                        /* Building the savestate path */
                        std::string savestatepath = context->config.savestatedir + '/';
                        savestatepath += context->gamename;
                        savestatepath += ".state" + std::to_string(statei);

                        /* Check that the savestate exists */
                        struct stat sb;
                        if (stat(savestatepath.c_str(), &sb) == -1) {
                            std::string* alert_str = new std::string("There is no savestate to load in this slot");
                            Fl::awake(alert_dialog, alert_str);
                            continue;
                        }

                        /* Building the movie path */
                        std::string moviepath = context->config.savestatedir + '/';
                        moviepath += context->gamename;
                        moviepath += ".movie" + std::to_string(statei) + ".ltm";

                        if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
                            /* When loading in read mode, we must check that
                             * the moviefile associated with the savestate is
                             * a prefix of our moviefile.
                             */
                            MovieFile savedmovie(context);
                            int ret = savedmovie.loadInputs(moviepath);
                            if (ret < 0) {
                                std::string* alert_str = new std::string("Could not load the moviefile associated with the savestate");
                                Fl::awake(alert_dialog, alert_str);
                                continue;
                            }

                            if (!movie.isPrefix(savedmovie)) {
                                /* Not a prefix, we don't allow loading */
                                std::string* alert_str = new std::string("Trying to load a state in read-only but the inputs mismatch");
                                Fl::awake(alert_dialog, alert_str);
                                continue;
                            }
                        }

                        sendMessage(MSGN_LOADSTATE);
                        sendString(savestatepath);

                        message = receiveMessage();
                        /* Loading is not assured to succeed, the following must
                         * only be done if it's the case.
                         */
                        if (message == MSGB_LOADING_SUCCEEDED) {
                            /* The copy of SharedConfig that the game stores may not
                             * be the same as this one due to memory loading, so we
                             * send it.
                             */
                            sendMessage(MSGN_CONFIG);
                            sendData(&context->config.sc, sizeof(SharedConfig));

                            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                                /* When in writing move, we load the movie associated
                                 * with the savestate.
                                 * Check if we are loading the same state we just saved.
                                 * If so, we can keep the same movie.
                                 */
                                if (last_savestate_slot != statei) {
                                    /* Load the movie file */
                                    movie.loadInputs(moviepath);
                                }

                                /* Increment rerecord count */
                                context->rerecord_count++;
                                ui.update_rerecordcount();
                            }

                            last_savestate_slot = statei;
                            message = receiveMessage();
                        }

                        /* The frame count has changed, we must get the new one */
                        if (message != MSGB_FRAMECOUNT_TIME) {
                            std::cerr << "Got wrong message after state loading" << std::endl;
                            return;
                        }
                        receiveData(&context->framecount, sizeof(unsigned long));
                        receiveData(&context->current_time, sizeof(struct timespec));
                        ui.update_framecount_time();
                    }

                    /* Switch between movie write and read-only */
                    if (hk.type == HOTKEY_READWRITE){
                        switch (context->config.sc.recording) {
                        case SharedConfig::RECORDING_WRITE:
                            context->config.sc.recording = SharedConfig::RECORDING_READ;
                            context->config.sc.movie_framecount = movie.nbFrames();
                            break;
                        case SharedConfig::RECORDING_READ:
                            /* Check if we reached the end of the movie already. */
                            if (context->framecount > context->config.sc.movie_framecount) {
                                std::string* alert_str = new std::string("Cannot write to a movie after its end");
                                Fl::awake(alert_dialog, alert_str);
                            }
                            else {
                                context->config.sc.recording = SharedConfig::RECORDING_WRITE;
                            }
                            break;
                        default:
                            break;
                        }
                        context->config.sc_modified = true;
                        ui.update_ui();
                    }

                    /* Start or stop a video encode */
                    if (hk.type == HOTKEY_TOGGLE_ENCODE) {
#ifdef LIBTAS_ENABLE_AVDUMPING
                        if (!context->config.sc.av_dumping) {

                            context->config.sc.av_dumping = true;
                            context->config.sc_modified = true;
                            context->config.dumpfile_modified = true;
                        }
                        else {
                            context->config.sc.av_dumping = false;
                            context->config.sc_modified = true;

                            /* Tells the game to immediately stop the encode,
                             * so we don't have to advance a frame. This also
                             * allows to start a new encode on the same frame
                             */
                            sendMessage(MSGN_STOP_ENCODE);
                        }
                        ui.update_ui();
#endif
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
                        ui.update_ui();
                        context->config.sc_modified = true;
                    }
                    if (hk.type == HOTKEY_FRAMEADVANCE){
                        ar_ticks = -1; // Deactivate auto-repeat
                    }
                }
            }

            if (!context->config.sc.running && !advance_frame) {

                /* Sleep a bit to not surcharge the processor */
                struct timespec tim = {0, 10L*1000L*1000L};
                nanosleep(&tim, NULL);

                /* Send a preview of inputs so that the game can display them
                 * on the HUD */
#ifdef LIBTAS_ENABLE_HUD
                if ((context->config.sc.recording == SharedConfig::NO_RECORDING) ||
                    (context->config.sc.recording == SharedConfig::RECORDING_WRITE)) {

                    /* Get inputs if we have input focus */
                    if (haveFocus(context)) {

                        /* Format the keyboard and mouse state and save it in the AllInputs struct */
                        AllInputs ai;
                        context->config.km.buildAllInputs(ai, context->display, context->game_window, context->config.sc);

                        /* Send inputs */
                        sendMessage(MSGN_PREVIEW_INPUTS);
                        sendData(&ai, sizeof(AllInputs));
                    }
                }
#endif
            }

        } while (!context->config.sc.running && !advance_frame);

        AllInputs ai;
        ai.emptyInputs();

        /* Record inputs or get inputs from movie file */
        switch (context->config.sc.recording) {
            case SharedConfig::NO_RECORDING:
            case SharedConfig::RECORDING_WRITE:

                /* Get inputs if we have input focus */
                if (haveFocus(context)) {
                    /* Format the keyboard and mouse state and save it in the AllInputs struct */
                    context->config.km.buildAllInputs(ai, context->display, context->game_window, context->config.sc);
                }

                if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                    /* Save inputs to moviefile */
                    movie.setInputs(ai);
                }

                /* Update the movie end time */
                context->movie_end_time = context->current_time;
                break;

            case SharedConfig::RECORDING_READ:
                /* Read inputs from file */
                if (movie.getInputs(ai) == 1) {
                    /* We are reading the last frame of the movie */
                    /* TODO: Add an option to decide what to do when movie ends */
                    context->config.sc.running = false;
                    ui.update_ui();
                    // context->config.sc_modified = true;
                }
                break;
        }

        /* Send shared config if modified */
        if (context->config.sc_modified) {
            /* Send config */
            sendMessage(MSGN_CONFIG);
            sendData(&context->config.sc, sizeof(SharedConfig));
            context->config.sc_modified = false;
        }

        /* Send dump file if modified */
        if (context->config.dumpfile_modified) {
            sendMessage(MSGN_DUMP_FILE);
            sendString(context->config.dumpfile);
            context->config.dumpfile_modified = false;
        }

        /* Send inputs and end of frame */
        sendMessage(MSGN_ALL_INPUTS);
        sendData(&ai, sizeof(AllInputs));

        if (context->status == Context::QUITTING) {
            sendMessage(MSGN_USERQUIT);
        }

        sendMessage(MSGN_END_FRAMEBOUNDARY);
    }

    movie.close();
    closeSocket();

    /* Remove savestates because they are invalid on future instances of the game */
    remove_savestates(context);

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

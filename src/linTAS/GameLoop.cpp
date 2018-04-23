/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

// #include <QApplication>

// #include "ui/MainWindow.h"

#include "GameLoop.h"
#include "utils.h"
#include "AutoSave.h"

#include "../shared/sockethelpers.h"
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <unistd.h> // fork()
#include <fcntl.h> // O_RDWR, O_CREAT
#include <future>
#include <csignal> // kill
#include <memory> // unique_ptr
#include <sys/stat.h> // stat
#include <sys/wait.h> // waitpid
#include <X11/X.h>

GameLoop::GameLoop(Context* c) : context(c), keysyms(xcb_key_symbols_alloc(c->conn), xcb_key_symbols_free) {}

void GameLoop::launchGameThread()
{
    // /* Update the LD_LIBRARY_PATH environment variable if the user set one */
    // if (!context->config.libdir.empty()) {
    //     char* oldlibpath = getenv("LD_LIBRARY_PATH");
    //     std::string libpath = context->config.libdir;
    //     if (oldlibpath) {
    //         libpath.append(":");
    //         libpath.append(oldlibpath);
    //     }
    //     setenv("LD_LIBRARY_PATH", libpath.c_str(), 1);
    // }

    /* Change the working directory if the user set one */
    if (!context->config.rundir.empty()) {
        if (0 != chdir(context->config.rundir.c_str())) {
            std::cerr << "Could not change the working directory to " << context->config.rundir << std::endl;
        }
    }

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
            "-ex", "handle SIGPWR nostop noprint", // used a lot in some games
            "-ex", "handle SIGXCPU nostop noprint", // used a lot in some games
            "-ex", "handle SIG35 nostop noprint", // used a lot in some games
            "-ex", "handle SIG36 nostop noprint", // used a lot in some games
            "-ex", "run",
            "--args", context->gamepath.c_str(), context->config.gameargs.c_str(),
            (char*) NULL);
    }
    else {
        /* Set the LD_PRELOAD environment variable to inject our lib to the game */
        setenv("LD_PRELOAD", context->libtaspath.c_str(), 1);

        execl(context->gamepath.c_str(), context->gamepath.c_str(), context->config.gameargs.c_str(), NULL);
    }
}

void GameLoop::start()
{
    init();
    initProcessMessages();

    while (1)
    {
        bool exitMsg = loopReceiveMessages();
        if (exitMsg) {
            loopExit();
            return;
        }

        emit startFrameBoundary();
        std::cerr << "start FB: " << std::endl;

        /* We are at a frame boundary */
        /* If we did not yet receive the game window id, just make the game running */
        bool endInnerLoop = false;
        if (context->game_window ) do {
            std::cerr << "game windo " << std::endl;

            /* Check if game is still running */
            int ret = waitpid(context->game_pid, nullptr, WNOHANG);
            if (ret == context->game_pid) {
                emit alertToShow(QString("Game did not exit normally..."));
                loopExit();
                return;
            }

            /* Implement frame-advance auto-repeat */
            bool ar_advance = false;
            if (ar_ticks >= 0) {
                ar_ticks++;
                if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
                    /* Trigger auto-repeat */
                    ar_advance = true;
            }

            struct HotKey hk;
            uint8_t eventType = nextEvent(hk);

            bool hasFrameAdvanced = false;
            if (eventType) {
                hasFrameAdvanced = processEvent(eventType, hk);
            }

            std::cerr << "running: " << context->config.sc.running << std::endl;
            std::cerr << "ar_advance: " << ar_advance << std::endl;
            std::cerr << "hasFrameAdvanced: " << hasFrameAdvanced << std::endl;

            endInnerLoop = context->config.sc.running || ar_advance || hasFrameAdvanced;

            if (!endInnerLoop) {
                sleepSendPreview();
            }
        } while (!endInnerLoop);

        AllInputs ai;
        processInputs(ai);

        /* Pause if needed */
        if ((context->pause_frame == (context->framecount + 1)) ||
            ((context->config.sc.movie_framecount + context->pause_frame) == (context->framecount + 1))) {

            /* Disable pause */
            context->pause_frame = 0;

            /* Pause and disable fast-forward */
            context->config.sc.running = false;
            context->config.sc.fastforward = false;
            context->config.sc_modified = true;
            emit sharedConfigChanged();
        }

        loopSendMessages(ai);

    }
}

void GameLoop::init()
{
    /* Unvalidate the game window id */
    context->game_window = 0;

    /* Reset the frame count */
    context->framecount = 0;

    /* Reset the rerecord count */
    context->rerecord_count = 0;

    /* Extract the game executable name from the game executable path */
    size_t sep = context->gamepath.find_last_of("/");
    if (sep != std::string::npos)
        context->gamename = context->gamepath.substr(sep + 1);
    else
        context->gamename = context->gamepath;

    context->status = Context::ACTIVE;

    emit statusChanged();

    /* Remove savestates again in case we did not exist cleanly the previous time */
    remove_savestates(context);

    /* Remove the file socket */
    removeSocket();

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


    /* We fork here so that the child process calls the game */
    int pid = fork();
    if (pid == 0) {
        launchGameThread();
    }

    /* Disable auto-repeat */
    uint32_t mask_aroff = XCB_KB_AUTO_REPEAT_MODE;
    uint32_t values_aroff[] = {XCB_AUTO_REPEAT_MODE_OFF, 0};

    xcb_change_keyboard_control(context->conn, mask_aroff, values_aroff);

    ar_ticks = -1;
    ar_delay = 50;
    ar_freq = 2;

    /* Opening a movie, which imports the inputs and parameters if in read mode,
     * or prepare a movie if in write mode.
     */
    movie = MovieFile(context);
    if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
        int ret = movie.loadMovie();
        if (ret < 0) {
            emit alertToShow(MovieFile::errorString(ret));
            context->config.sc.recording = SharedConfig::NO_RECORDING;
        }
        else {
            context->config.sc.movie_framecount = movie.nbFrames();

            /* Update the UI accordingly */
            emit configChanged();
        }
    }
    if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
        /* Add one blank frame in every movie corresponding to the input
         * between the game startup and the first screen draw, which is for now
         * impossible to set
         */
        AllInputs ai;
        ai.emptyInputs();
        movie.setInputs(ai, false);
    }

    emit rerecordChanged();
}

void GameLoop::initProcessMessages()
{
    /* Connect to the socket between the program and the game */
    initSocketProgram();

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
                loopExit();
                return;
        }
        message = receiveMessage();
    }

    /* Send informations to the game */

    /* Send shared config */
    sendMessage(MSGN_CONFIG);
    sendData(&context->config.sc, sizeof(SharedConfig));

    /* Send dump file if dumping from the beginning */
    if (context->config.sc.av_dumping) {
        sendMessage(MSGN_DUMP_FILE);
        sendString(context->config.dumpfile);
    }

    /* Get the shared libs of the game executable */
    std::vector<std::string> linked_libs;
    std::ostringstream libcmd;
    libcmd << "ldd '" << context->gamepath << "' | cut -d'>' -f2 -s | cut -d'(' -f1 | sed 's/^[ \t]*//;s/[ \t]*$//'";
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

    /* Send shared library names */
    for (auto &name : linked_libs) {
        sendMessage(MSGN_LIB_FILE);
        sendString(name);
    }

    /* End message */
    sendMessage(MSGN_END_INIT);
}

bool GameLoop::loopReceiveMessages()
{
    /* Wait for frame boundary */
    int message = receiveMessage();

    while (message != MSGB_START_FRAMEBOUNDARY) {
        float fps, lfps;
        switch (message) {
        case MSGB_WINDOW_ID:
            std::cerr << "window ID !!" << std::endl;
            receiveData(&context->game_window, sizeof(Window));
            /* FIXME: Don't do this if the ui option is unchecked  */
            {
                const static uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_EXPOSURE };
                xcb_void_cookie_t cwa_cookie = xcb_change_window_attributes (context->conn, context->game_window, XCB_CW_EVENT_MASK, values);
                xcb_generic_error_t *error = xcb_request_check(context->conn, cwa_cookie);
                if (error) {
                    std::cerr << "error in xcb_change_window_attributes: " << error->error_code << std::endl;
                }
            }
            // XSelectInput(context->display, context->game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
            // std::cout << "Received window id " << context->game_window << std::endl;
            break;

        case MSGB_ALERT_MSG:
            /* Ask the UI thread to display the alert. He is in charge of
             * freeing the string.
             */
            emit alertToShow(QString(receiveString().c_str()));
            break;
        case MSGB_ENCODE_FAILED:
            context->config.sc.av_dumping = false;
            context->config.sc_modified = true;
            emit sharedConfigChanged();
            break;
        case MSGB_FRAMECOUNT_TIME:
            receiveData(&context->framecount, sizeof(unsigned long));
            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                context->config.sc.movie_framecount = context->framecount;
            }
            receiveData(&context->current_time, sizeof(struct timespec));
            emit frameCountChanged();
            break;
        case MSGB_GAMEINFO:
            receiveData(&context->game_info, sizeof(context->game_info));
            emit gameInfoChanged(context->game_info);
            break;
        case MSGB_FPS:
            receiveData(&fps, sizeof(float));
            receiveData(&lfps, sizeof(float));
            emit fpsChanged(fps, lfps);
            break;
        case MSGB_QUIT:
            return true;
        default:
            std::cerr << "Got unknown message!!!" << std::endl;
            return true;
        }
        message = receiveMessage();
    }
    return false;
}

uint8_t GameLoop::nextEvent(struct HotKey &hk)
{
    static xcb_generic_event_t *next_event = nullptr;

    while (true) {
        xcb_generic_event_t *event = nullptr;

        if (next_event) {
            event = next_event;
            next_event = nullptr;
        }
        else {
            event = xcb_poll_for_event(context->conn);
        }

        if (!event) {
            if (context->hotkey_queue.empty()) {
                return 0;
            }
            else {
                /* Processing a hotkey pushed by the UI */
                context->hotkey_queue.pop(hk.type);
                return XCB_KEY_PRESS;
            }
        }
        else {
            /* Processing a hotkey pressed by the user */
            uint8_t response_type = (event->response_type & ~0x80);

            if ((response_type == XCB_KEY_PRESS) || (response_type == XCB_KEY_RELEASE)) {
                /* Get the actual pressed/released key */
                xcb_key_press_event_t* key_event = reinterpret_cast<xcb_key_press_event_t*>(event);
                xcb_keycode_t kc = key_event->detail;

                // if ((event->response_type & ~0x80) == XCB_KEY_PRESS) {
                //     std::cout << "Pressed key: " << kc << std::endl;
                //     std::cout << "   response_type: " << (int)event->response_type << std::endl;
                //     std::cout << "   sequence: " << key_event->sequence << std::endl;
                //     std::cout << "   timestamp: " << key_event->time << std::endl;
                //     std::cout << "   root: " << key_event->root << std::endl;
                //     std::cout << "   event: " << key_event->event << std::endl;
                //     std::cout << "   child: " << key_event->child << std::endl;
                //     std::cout << "   root_x: " << key_event->root_x << std::endl;
                //     std::cout << "   root_y: " << key_event->root_y << std::endl;
                //     std::cout << "   event_x: " << key_event->event_x << std::endl;
                //     std::cout << "   event_y: " << key_event->event_y << std::endl;
                //     std::cout << "   state: " << key_event->state << std::endl;
                //     std::cout << "   same_screen: " << key_event->same_screen << std::endl;
                // }
                // else {
                //     std::cout << "Released key: " << kc << std::endl;
                //     std::cout << "   response_type: " << (int)event->response_type << std::endl;
                //     std::cout << "   sequence: " << key_event->sequence << std::endl;
                //     std::cout << "   timestamp: " << key_event->time << std::endl;
                //     std::cout << "   root: " << key_event->root << std::endl;
                //     std::cout << "   event: " << key_event->event << std::endl;
                //     std::cout << "   child: " << key_event->child << std::endl;
                //     std::cout << "   root_x: " << key_event->root_x << std::endl;
                //     std::cout << "   root_y: " << key_event->root_y << std::endl;
                //     std::cout << "   event_x: " << key_event->event_x << std::endl;
                //     std::cout << "   event_y: " << key_event->event_y << std::endl;
                //     std::cout << "   state: " << key_event->state << std::endl;
                //     std::cout << "   same_screen: " << key_event->same_screen << std::endl;
                // }

                /*
                 * TODO: The following code was supposed to detect the X
                 * AutoRepeat and remove the generated events. Actually,
                 * I can't use it because for some reason, when I press a
                 * key, a KeyRelease is generated at the same time as the
                 * KeyPress event. For this reason, I disable X
                 * AutoRepeat and I use this code to delete the extra
                 * KeyRelease event...
                 */

                if (response_type == XCB_KEY_RELEASE) {
                    next_event = xcb_poll_for_event (context->conn);
                    xcb_key_press_event_t* next_key_event = reinterpret_cast<xcb_key_press_event_t*>(next_event);

                    if (next_event &&
                        (next_event->sequence == event->sequence) &&
                        ((next_event->response_type & ~0x80) == XCB_KEY_PRESS) &&
                        (next_key_event->detail == key_event->detail)) {
                        /* This event must be discarded */
                        free(event);
                        continue;
                    }
                }

                free(event);

                /* Get keysym from keycode */
                xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms.get(), kc, 0);

                /* If pressed a controller button, update the controller input window */
                notifyControllerEvent(ks, response_type == XCB_KEY_PRESS);

                /* If the key is a modifier, skip it */
                if (is_modifier(ks))
                    continue;

                /* Build the modifier value */
                xcb_generic_error_t* error;
                xcb_query_keymap_cookie_t keymap_cookie = xcb_query_keymap(context->conn);
                xcb_query_keymap_reply_t* keymap_reply = xcb_query_keymap_reply(context->conn, keymap_cookie, &error);

                xcb_keysym_t modifiers = 0;
                if (error) {
                    std::cerr << "Could not get xcb_query_keymap, X error" << error->error_code << std::endl;
                }
                else {
                    modifiers = build_modifiers(keymap_reply->keys, keysyms.get());
                }
                free(keymap_reply);

                /* Check if this KeySym with or without modifiers is mapped to a hotkey */
                if (context->config.km.hotkey_mapping.find(ks | modifiers) != context->config.km.hotkey_mapping.end()) {
                    hk = context->config.km.hotkey_mapping[ks | modifiers];
                    return response_type;
                }
                else if (context->config.km.hotkey_mapping.find(ks) != context->config.km.hotkey_mapping.end()) {
                    hk = context->config.km.hotkey_mapping[ks];
                    return response_type;
                }
                else
                    /* This input is not a hotkey, skipping to the next */
                    continue;
            }
            else {
                free(event);
                return response_type;
            }
        }
    }
    return 0;
}

void GameLoop::notifyControllerEvent(xcb_keysym_t ks, bool pressed)
{
    if (context->config.km.input_mapping.find(ks) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[ks];
        if (SingleInput::inputTypeIsController(si.type))
            emit controllerButtonToggled(SingleInput::inputTypeToControllerNumber(si.type), SingleInput::inputTypeToInputNumber(si.type), pressed);
    }
}

bool GameLoop::processEvent(uint8_t type, struct HotKey &hk)
{

    switch (type) {

    case XCB_FOCUS_OUT:
        ar_ticks = -1; // Deactivate auto-repeat
        return false;

    case XCB_EXPOSE:
        /* Send an expose message to the game so that he can redrawn the screen */
        if (!context->config.sc.running)
            sendMessage(MSGN_EXPOSE);
        return false;

    case XCB_KEY_PRESS:

        switch(hk.type) {

        case HOTKEY_FRAMEADVANCE:
            /* Advance a frame */
            if (context->config.sc.running) {
                context->config.sc.running = false;
                emit sharedConfigChanged();
                context->config.sc_modified = true;
            }
            ar_ticks = 0; // Activate auto-repeat
            return true;

        case HOTKEY_PLAYPAUSE:
            /* Toggle between play and pause */
            context->config.sc.running = !context->config.sc.running;
            emit sharedConfigChanged();
            context->config.sc_modified = true;
            return false;

        case HOTKEY_FASTFORWARD:
            /* Enable fastforward */
            context->config.sc.fastforward = true;
            emit sharedConfigChanged();
            context->config.sc_modified = true;

            /* Make frame advance auto-repeat faster */
            ar_freq = 1;

            return false;

        case HOTKEY_SAVESTATE1:
        case HOTKEY_SAVESTATE2:
        case HOTKEY_SAVESTATE3:
        case HOTKEY_SAVESTATE4:
        case HOTKEY_SAVESTATE5:
        case HOTKEY_SAVESTATE6:
        case HOTKEY_SAVESTATE7:
        case HOTKEY_SAVESTATE8:
        case HOTKEY_SAVESTATE9:
        {
            /* Perform a savestate:
             * - save the moviefile if we are recording
             * - tell the game to save its state
             */

            /* Saving is not allowed if currently encoding */
            if (context->config.sc.av_dumping) {
                emit alertToShow(QString("Saving is not allowed when in the middle of video encoding"));
                return false;
            }

            /* Slot number */
            int statei = hk.type - HOTKEY_SAVESTATE1 + 1;

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
            return false;
        }

        case HOTKEY_LOADSTATE1:
        case HOTKEY_LOADSTATE2:
        case HOTKEY_LOADSTATE3:
        case HOTKEY_LOADSTATE4:
        case HOTKEY_LOADSTATE5:
        case HOTKEY_LOADSTATE6:
        case HOTKEY_LOADSTATE7:
        case HOTKEY_LOADSTATE8:
        case HOTKEY_LOADSTATE9:

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
        {

            /* Loading is not allowed if currently encoding */
            if (context->config.sc.av_dumping) {
                emit alertToShow(QString("Loading is not allowed when in the middle of video encoding"));
                return false;
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
                emit alertToShow(QString("There is no savestate to load in this slot"));
                return false;
            }

            /* Building the movie path */
            std::string moviepath = context->config.savestatedir + '/';
            moviepath += context->gamename;
            moviepath += ".movie" + std::to_string(statei) + ".ltm";

            bool moviePrefix;

            if (context->config.sc.recording != SharedConfig::NO_RECORDING) {

                /* Checking if the savestate movie is a prefix of our movie */
                MovieFile savedmovie(context);
                int ret = savedmovie.loadInputs(moviepath);
                if (ret < 0) {
                    emit alertToShow(QString("Could not load the moviefile associated with the savestate"));
                    return false;
                }

                moviePrefix = movie.isPrefix(savedmovie);

                /* When loading in read mode, we don't allow loading a non-prefix movie */
                if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
                    if (!moviePrefix) {
                        /* Not a prefix, we don't allow loading */
                        emit alertToShow(QString("Trying to load a state in read-only but the inputs mismatch"));
                        return false;
                    }
                }
            }

            sendMessage(MSGN_LOADSTATE);
            sendString(savestatepath);

            emit inputsToBeChanged();

            int message = receiveMessage();
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
                     * with the savestate. We check if we are loading a movie prefix.
                     * If so, we can keep the same movie.
                     */
                    if (!moviePrefix) {
                        /* Load the movie file */
                        movie.loadInputs(moviepath);
                    }

                    /* Increment rerecord count */
                    context->rerecord_count++;
                    emit rerecordChanged();
                }

                message = receiveMessage();
            }

            /* The frame count has changed, we must get the new one */
            if (message != MSGB_FRAMECOUNT_TIME) {
                std::cerr << "Got wrong message after state loading" << std::endl;
                return false;
            }
            receiveData(&context->framecount, sizeof(unsigned long));
            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                context->config.sc.movie_framecount = context->framecount;
            }
            receiveData(&context->current_time, sizeof(struct timespec));

            emit inputsChanged();
            emit frameCountChanged();
            return false;
        }

        case HOTKEY_READWRITE:
            /* Switch between movie write and read-only */
            switch (context->config.sc.recording) {
            case SharedConfig::RECORDING_WRITE:
                context->config.sc.recording = SharedConfig::RECORDING_READ;
                context->config.sc.movie_framecount = movie.nbFrames();
                break;
            case SharedConfig::RECORDING_READ:
                /* Check if we reached the end of the movie already. */
                if (context->framecount > context->config.sc.movie_framecount) {
                    emit alertToShow(QString("Cannot write to a movie after its end"));
                }
                else {
                    emit inputsToBeChanged();
                    context->config.sc.recording = SharedConfig::RECORDING_WRITE;
                    emit inputsChanged();
                }
                break;
            default:
                break;
            }
            context->config.sc_modified = true;
            emit sharedConfigChanged();
            return false;

        /* Start or stop a video encode */
        case HOTKEY_TOGGLE_ENCODE:
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
            emit sharedConfigChanged();
#endif
            return false;

        } /* switch(hk.type) */

    case XCB_KEY_RELEASE:

        switch (hk.type) {
        case HOTKEY_FASTFORWARD:
            context->config.sc.fastforward = false;
            emit sharedConfigChanged();
            context->config.sc_modified = true;

            /* Recover normal frame-advance auto-repeat */
            ar_freq = 1;

            return false;
        case HOTKEY_FRAMEADVANCE:
            ar_ticks = -1; // Deactivate auto-repeat
            return false;
        }
    default:
        return false;
    } /* switch (type) */
}


void GameLoop::sleepSendPreview()
{
    /* Sleep a bit to not surcharge the processor */
    struct timespec tim = {0, 17L*1000L*1000L};
    nanosleep(&tim, NULL);

    /* Send a preview of inputs so that the game can display them
     * on the HUD */
#ifdef LIBTAS_ENABLE_HUD
    if ((context->config.sc.recording == SharedConfig::NO_RECORDING) ||
        (context->config.sc.recording == SharedConfig::RECORDING_WRITE)) {

        /* Get inputs if we have input focus */
        if (haveFocus()) {

            /* Format the keyboard and mouse state and save it in the AllInputs struct */
            static AllInputs preview_ai, last_preview_ai;
            context->config.km.buildAllInputs(preview_ai, context->conn, context->game_window, keysyms.get(), context->config.sc);
            emit inputsToBeSent(preview_ai);

            /* Send inputs if changed */
            if (!(preview_ai == last_preview_ai)) {
                sendMessage(MSGN_PREVIEW_INPUTS);
                sendData(&preview_ai, sizeof(AllInputs));
                last_preview_ai = preview_ai;
            }
        }
    }
#endif
}


void GameLoop::processInputs(AllInputs &ai)
{
    ai.emptyInputs();

    /* Don't record inputs if we are quitting */
    if (context->status == Context::QUITTING)
        return;

    /* Record inputs or get inputs from movie file */
    switch (context->config.sc.recording) {
        case SharedConfig::NO_RECORDING:
        case SharedConfig::RECORDING_WRITE:

            /* Get inputs if we have input focus */
            if (haveFocus()) {
                /* Format the keyboard and mouse state and save it in the AllInputs struct */
                context->config.km.buildAllInputs(ai, context->conn, context->game_window, keysyms.get(), context->config.sc);
                emit inputsToBeSent(ai);
            }

            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                /* If the input editor is visible, we should keep future inputs.
                 * If not, we truncate inputs if necessary.
                 */
                bool keep_inputs = false;
                bool past_inputs = context->framecount < movie.nbFrames();
                emit isInputEditorVisible(keep_inputs);

                /* Send signal before saving the input */
                if (past_inputs) {
                    if (keep_inputs) {
                        emit inputsToBeEdited();
                    }
                    else {
                        emit inputsToBeChanged();
                    }
                }
                else {
                    emit inputsToBeAdded();
                }

                /* Save inputs to moviefile */
                movie.setInputs(ai, keep_inputs);

                /* Send signal after saving the input */
                if (past_inputs) {
                    if (keep_inputs) {
                        emit inputsEdited();
                    }
                    else {
                        emit inputsChanged();
                    }
                }
                else {
                    emit inputsAdded();
                }

                AutoSave::update(context, movie);
            }

            /* Update the movie end time */
            context->movie_end_time = context->current_time;
            break;

        case SharedConfig::RECORDING_READ:
            /* Read inputs from file */
            if (movie.getInputs(ai) == 1) {
                /* We are reading the last frame of the movie */
                switch(context->config.on_movie_end) {
                    case Config::MOVIEEND_READ:
                        // context->config.sc.running = false;
                        //context->config.sc_modified = true;
                        // emit sharedConfigChanged();
                        break;
                    case Config::MOVIEEND_WRITE:
                        context->config.sc.recording = SharedConfig::RECORDING_WRITE;
                        context->config.sc_modified = true;
                        emit sharedConfigChanged();
                        break;
                    default:
                        break;
                }
            }
            break;
    }
}

void GameLoop::loopSendMessages(AllInputs &ai)
{
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


bool GameLoop::haveFocus()
{
    if (context->inputs_focus & Context::FOCUS_ALL)
        return true;

    xcb_window_t window;

    xcb_generic_error_t* error;
    xcb_get_input_focus_cookie_t focus_cookie = xcb_get_input_focus(context->conn);
    std::unique_ptr<xcb_get_input_focus_reply_t> focus_reply(xcb_get_input_focus_reply(context->conn, focus_cookie, &error));
    if (error) {
        std::cerr << "Could not get focussed window, X error" << error->error_code << std::endl;
    }

    window = focus_reply->focus;

    // XGetInputFocus(context->display, &window, &revert);

    if ((context->inputs_focus & Context::FOCUS_GAME) &&
        (window == context->game_window))
        return true;

    if ((context->inputs_focus & Context::FOCUS_UI) &&
        (window == context->ui_window))
        return true;

    return false;
}

void GameLoop::loopExit()
{
    if (movie.modifiedSinceLastSave) {

        /* Ask the user if he wants to save the movie, and get the answer.
         * Prompting a alert window must be done by the UI thread, so we are
         * using std::future/std::promise mechanism.
         */
        std::promise<bool> saveAnswer;
        std::future<bool> futureSave = saveAnswer.get_future();
        emit askMovieSaved(&saveAnswer);

        if (futureSave.get()) {
            /* User answered yes */
            movie.saveMovie();
        }
    }

    movie.close();
    closeSocket();

    /* Remove savestates because they are invalid on future instances of the game */
    remove_savestates(context);

    context->status = Context::INACTIVE;
    emit statusChanged();

    /* Disable auto-repeat */
    uint32_t mask_aron = XCB_KB_AUTO_REPEAT_MODE;
    uint32_t values_aron[] = {XCB_AUTO_REPEAT_MODE_ON, 0};

    xcb_change_keyboard_control(context->conn, mask_aron, values_aron);
    xcb_flush(context->conn);
}

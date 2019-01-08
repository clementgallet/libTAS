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
#include <sys/stat.h> // stat
#include <sys/wait.h> // waitpid
#include <X11/X.h>

#include <sys/personality.h>
#ifndef HAVE_PERSONALITY
# include <syscall.h>
# define personality(pers) ((long)syscall(SYS_personality, pers))
#endif

GameLoop::GameLoop(Context* c) : context(c), keysyms(xcb_key_symbols_alloc(c->conn), xcb_key_symbols_free) {
    movie = MovieFile(context);
}

void GameLoop::launchGameThread()
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

    /* Change the working directory to the user-defined one or game directory */
    std::string newdir = context->config.rundir;
    if (newdir.empty()) {
        /* Get the game directory from path */
        size_t sep = context->gamepath.find_last_of("/");
        if (sep != std::string::npos)
            newdir = context->gamepath.substr(0, sep);
        else
            newdir = ""; // Should not happen
    }

    if (0 != chdir(newdir.c_str())) {
        std::cerr << "Could not change the working directory to " << newdir << std::endl;
    }

    /* Set PWD environment variable because games may use it and chdir
     * does not update it.
     */
    setenv("PWD", newdir.c_str(), 1);

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
    if (context->config.sc.opengl_soft)
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    else
        unsetenv("LIBGL_ALWAYS_SOFTWARE");

    if (!context->config.llvm_perf.empty())
        setenv("LP_PERF", context->config.llvm_perf.c_str(), 1);
    else
        unsetenv("LP_PERF");

    /* Disable Address Space Layout Randomization for the game, so that ram
     * watch addresses do not change on game restart.
     * Source: https://stackoverflow.com/questions/5194666/disable-randomization-of-memory-addresses/30385370#30385370
     */
    personality(ADDR_NO_RANDOMIZE);

    /* Build the argument list to be fed to execv */
    std::list<std::string> arg_list;

    if (context->attach_gdb) {
        arg_list.push_back("/usr/bin/gdb");
        arg_list.push_back("-q");
        arg_list.push_back("-ex");

        /* LD_PRELOAD must be set inside a gdb command to be effective */
        std::string ldpreloadstr = "set exec-wrapper env 'LD_PRELOAD=";
        ldpreloadstr += context->libtaspath;
        ldpreloadstr += "'";
        arg_list.push_back(ldpreloadstr);

        /* We are using SIGUSR1 and SIGUSR2 for savestates, so don't
         * print and pause when one signal is sent */
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIGUSR1 nostop noprint");
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIGUSR2 nostop noprint");
        /* The following signals are used a lot in some games */
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIGPWR nostop noprint");
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIGXCPU nostop noprint");
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIG35 nostop noprint");
        arg_list.push_back("-ex");
        arg_list.push_back("handle SIG36 nostop noprint");
        arg_list.push_back("-ex");
        arg_list.push_back("run");
        arg_list.push_back("--args");
    }
    else {
        /* Set the LD_PRELOAD environment variable to inject our lib to the game */
        setenv("LD_PRELOAD", context->libtaspath.c_str(), 1);
    }
    arg_list.push_back(context->gamepath);

    /* Parse the game command-line arguments */
    std::istringstream iss(context->config.gameargs);
    std::string arg;
    while (iss >> arg) {
        arg_list.push_back(arg);
    }

    /* Build the char* array for execv */
    std::vector<char*> arg_vect;
    for (const std::string& elem : arg_list) {
        /* The const_cast is bad, but execv has no reason to modify it, no ? */
        arg_vect.push_back(const_cast<char*>(elem.c_str()));
    }
    arg_vect.push_back(NULL);

    /* Run the actual game */
    execv(arg_vect[0], &arg_vect[0]);
}

void GameLoop::start()
{
    init();
    initProcessMessages();

    while (1)
    {
        bool exitMsg = startFrameMessages();
        if (exitMsg) {
            loopExit();
            return;
        }

        emit startFrameBoundary();

        /* We are at a frame boundary */
        /* If we did not yet receive the game window id, just make the game running */
        bool endInnerLoop = false;
        if (context->game_window ) do {

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

            endInnerLoop = context->config.sc.running || ar_advance || hasFrameAdvanced;

            if (!endInnerLoop) {
                sleepSendPreview();
            }
        } while (!endInnerLoop);

        AllInputs ai;
        processInputs(ai);
        prev_ai = ai;

        /* Set the status to restart */
        if (ai.restart) {
            context->status = Context::RESTARTING;
        }

        bool shouldQuit = false;

        /* Pause if needed */
        if ((context->pause_frame == (context->framecount + 1)) ||
            ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
            ((context->config.sc.movie_framecount + context->pause_frame) == (context->framecount + 1)))) {

            if (context->config.dumping) {
                /* If we're dumping from the command line, we are done */
                shouldQuit = true;
            } else {
                /* Disable pause */
                context->pause_frame = 0;

                /* Pause and disable fast-forward */
                context->config.sc.running = false;
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                emit sharedConfigChanged();
            }
        }

        endFrameMessages(ai);

        if (shouldQuit) {
            context->status = Context::QUITTING;
        }
    }
}

void GameLoop::init()
{
    /* Unvalidate the game window id */
    context->game_window = 0;

    /* Reset the frame count if not restarting */
    if (context->status != Context::RESTARTING)
        context->framecount = 0;

    /* Set the initial frame count for the game */
    context->config.sc.initial_framecount = context->framecount;

    /* Reset the rerecord count if not restarting */
    if (context->status != Context::RESTARTING)
        context->rerecord_count = 0;

    /* Reset the encoding segment if not restarting */
    if (context->status != Context::RESTARTING)
        context->encoding_segment = 0;

    /* Extract the game executable name from the game executable path */
    size_t sep = context->gamepath.find_last_of("/");
    if (sep != std::string::npos)
        context->gamename = context->gamepath.substr(sep + 1);
    else
        context->gamename = context->gamepath;

    /* Remove savestates again in case we did not exist cleanly the previous time */
    remove_savestates(context);

    /* Remove the file socket */
    removeSocket();

    /* We fork here so that the child process calls the game */
    int pid = fork();
    if (pid == 0) {
        launchGameThread();
    }

    ar_ticks = -1;
    ar_delay = 50;
    ar_freq = 2;

    /* Compute the MD5 hash of the game binary */
    context->md5_game.clear();
    std::ostringstream cmd;
    cmd << "md5sum -b \"" << context->gamepath << "\" 2> /dev/null";

    FILE *md5str = popen(cmd.str().c_str(), "r");
    if (md5str != NULL) {
        std::array<char,33> buf;
        if (fgets(buf.data(), buf.size(), md5str) != nullptr) {
            context->md5_game.assign(buf.data());
        }
        pclose(md5str);
    }

    /* Only open the movie if we did not restart */
    if (context->status != Context::RESTARTING) {
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

            /* Check md5 match */
            if ((!context->md5_movie.empty()) && (context->md5_game.compare(context->md5_movie) != 0))
                emit alertToShow(QString("Game executable hash does not match with the hash stored in the movie!"));

        }
    }

    /* We must add a blank frame in most cases */
    if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
        /* Add one blank frame in every movie corresponding to the input
         * between the game startup and the first screen draw, which is for now
         * impossible to set. Exception is if we restarted.
         */
        if (context->framecount == movie.nbFrames()) {
            AllInputs ai;
            ai.emptyInputs();
            movie.setInputs(ai, false);
        }
    }

    /* Set the current time to the initial time, except when restarting */
    if (context->status != Context::RESTARTING)
        context->current_time = context->config.sc.initial_time;

    pointer_offset_x = 0;
    pointer_offset_y = 0;

    emit rerecordChanged();

    context->status = Context::ACTIVE;
    emit statusChanged();
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

    /* This is a bit hackish, change the initial time to the current time before
     * sending so that the game gets the correct time after restarting. */
    struct timespec it = context->config.sc.initial_time;
    context->config.sc.initial_time = context->current_time;
    sendMessage(MSGN_CONFIG);
    sendData(&context->config.sc, sizeof(SharedConfig));
    context->config.sc.initial_time = it;

    /* Send dump file if dumping from the beginning */
    if (context->config.sc.av_dumping) {
        sendMessage(MSGN_DUMP_FILE);
        sendString(context->config.dumpfile);
        sendString(context->config.ffmpegoptions);
    }

    /* Build and send the base savestate path/index */
    if (context->config.sc.incremental_savestates) {
        sendMessage(MSGN_BASE_SAVESTATE_INDEX);
        int index = 0;
        sendData(&index, sizeof(int));

        if (!context->config.sc.savestates_in_ram) {
            std::string basesavestatepath = context->config.savestatedir + '/';
            basesavestatepath += context->gamename;
            basesavestatepath += ".state0";
            sendMessage(MSGN_BASE_SAVESTATE_PATH);
            sendString(basesavestatepath);
        }
    }

    /* Build and send the backtrack savestate path/index */
    if (context->config.sc.backtrack_savestate) {
        sendMessage(MSGN_BACKTRACK_SAVESTATE_INDEX);
        int index = 10;
        sendData(&index, sizeof(int));

        if (!context->config.sc.savestates_in_ram) {
            std::string basesavestatepath = context->config.savestatedir + '/';
            basesavestatepath += context->gamename;
            basesavestatepath += ".state10";
            sendMessage(MSGN_BACKTRACK_SAVESTATE_PATH);
            sendString(basesavestatepath);
        }
    }

    sendMessage(MSGN_ENCODING_SEGMENT);
    sendData(&context->encoding_segment, sizeof(int));

    /* End message */
    sendMessage(MSGN_END_INIT);
}

bool GameLoop::startFrameMessages()
{
    /* Wait for frame boundary */
    int message = receiveMessage();

    while (message != MSGB_START_FRAMEBOUNDARY) {
        float fps, lfps;
        switch (message) {
        case MSGB_WINDOW_ID:
            receiveData(&context->game_window, sizeof(Window));
            if (context->game_window != 0)
            {
                const static uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_EXPOSURE };
                xcb_void_cookie_t cwa_cookie = xcb_change_window_attributes (context->conn, context->game_window, XCB_CW_EVENT_MASK, values);
                xcb_generic_error_t *error = xcb_request_check(context->conn, cwa_cookie);
                if (error) {
                    std::cerr << "error in xcb_change_window_attributes: " << error->error_code << std::endl;
                }
            }
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
        case MSGB_ENCODING_SEGMENT:
            receiveData(&context->encoding_segment, sizeof(int));
            break;
        case MSGB_QUIT:
            if (context->config.dumping) {
                /* Finished running a dump from the command line */
                exit(0);
            }
            return true;
        default:
            std::cerr << "Got unknown message!!!" << std::endl;
            return true;
        }
        message = receiveMessage();
    }

    /* Send ram watches */
    if (context->config.sc.osd & SharedConfig::OSD_RAMWATCHES) {
        std::string ramwatch;
        emit getRamWatch(ramwatch);
        while(!ramwatch.empty()) {
            sendMessage(MSGN_RAMWATCH);
            sendString(ramwatch);
            emit getRamWatch(ramwatch);
        }
    }

    sendMessage(MSGN_START_FRAMEBOUNDARY);

    return false;
}

uint8_t GameLoop::nextEvent(struct HotKey &hk)
{
    static xcb_keycode_t last_pressed_key = 0;
    static xcb_generic_event_t *next_event = nullptr;

    while (true) {
        xcb_generic_event_t *event;

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

                /* Detecting auto-repeat, either if detectable auto-repeat is
                 * supported or not by the X server.
                 * If detectable auto-repeat is supported, we detect if pressed
                 * event is from the same key as the last pressed key, without
                 * a key release.
                 * If detectable auto-repeat is not supported, we detect if a
                 * released event is followed by a pressed event with the same
                 * sequence number. If not, we must keep the later event for
                 * the next call (xcb does not support peeking events).
                 */
                if (kc == last_pressed_key) {
                    if (response_type == XCB_KEY_RELEASE) {
                        /* Check the next event. We wait a bit for the next
                         * event to be generated if auto-repeat. There are still
                         * are few cases where we would have to wait a significantly
                         * longer time.
                         */
                        usleep(500);
                        next_event = xcb_poll_for_event(context->conn);
                        xcb_key_press_event_t* next_key_event = reinterpret_cast<xcb_key_press_event_t*>(next_event);

                        if (next_event &&
                           ((next_event->response_type & ~0x80) == XCB_KEY_PRESS) &&
                           (next_key_event->detail = kc) &&
                           ((next_key_event->time - key_event->time) < 5)) {

                            /* Found an auto-repeat sequence, discard both events */
                            free(event);
                            free(next_event);
                            next_event = nullptr;
                            continue;
                        }

                        /* Normal key release */
                        last_pressed_key = 0;
                    }
                    if (response_type == XCB_KEY_PRESS) {
                        /* Auto-repeat, must skip */
                        free(event);
                        continue;
                    }
                }
                if (response_type == XCB_KEY_PRESS) {
                    last_pressed_key = kc;
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
        if (si.inputTypeIsController())
            emit controllerButtonToggled(si.inputTypeToControllerNumber(), si.inputTypeToInputNumber(), pressed);
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

            /* Send the savestate index */
            sendMessage(MSGN_SAVESTATE_INDEX);
            sendData(&statei, sizeof(int));

            /* Send the savestate path */
            std::string savestatepath = context->config.savestatedir + '/';
            savestatepath += context->gamename;
            savestatepath += ".state" + std::to_string(statei);
            if (! context->config.sc.savestates_in_ram) {
                sendMessage(MSGN_SAVESTATE_PATH);
                sendString(savestatepath);
            }
            else {
                /* Create empty savestate files if stored in RAM */
                std::string pagemappath = savestatepath + ".pm";
                std::string pagespath = savestatepath + ".p";
                std::ofstream opm(pagemappath);
                opm.close();
                std::ofstream op(pagespath);
                op.close();
            }

            if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                std::string msg = "Saving state ";
                msg += std::to_string(statei);
                sendMessage(MSGN_OSD_MSG);
                sendString(msg);
            }

            sendMessage(MSGN_SAVESTATE);

            if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                std::string message = "State ";
                message += std::to_string(statei);
                message += " saved";
                sendMessage(MSGN_OSD_MSG);
                sendString(message);
            }

            emit savestatePerformed(statei, context->framecount);

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
        case HOTKEY_LOADSTATE_BACKTRACK:

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

            /* Send the savestate index */
            sendMessage(MSGN_SAVESTATE_INDEX);
            sendData(&statei, sizeof(int));

            /* Building the movie path */
            std::string moviepath = context->config.savestatedir + '/';
            moviepath += context->gamename;
            moviepath += ".movie" + std::to_string(statei) + ".ltm";

            /* Building the savestate path */
            std::string savestatepath = context->config.savestatedir + '/';
            savestatepath += context->gamename;
            savestatepath += ".state" + std::to_string(statei);

            /* Check that the savestate exists */
            std::string pagemappath = savestatepath + ".pm";
            std::string pagespath = savestatepath + ".p";
            if ((access(pagemappath.c_str(), F_OK) != 0) || (access(pagespath.c_str(), F_OK) != 0)) {
                /* If there is no savestate but a movie file, offer to load
                 * the movie and fast-forward to the savestate movie frame.
                 */
                if ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
                    (access(moviepath.c_str(), F_OK) == 0)) {

                    /* Ask the user if they want to load the movie, and get the answer.
                     * Prompting a alert window must be done by the UI thread, so we are
                     * using std::future/std::promise mechanism.
                     */
                    std::promise<bool> answer;
                    std::future<bool> future = answer.get_future();
                    emit askToShow(QString("There is a savestate in that slot from a previous game iteration. Do you want to load the associated movie?"), &answer);

                    if (! future.get()) {
                        /* User answered no */
                        return false;
                    }

                    /* Load the savestate movie */
                    MovieFile savedmovie(context);
                    int ret = savedmovie.loadInputs(moviepath);
                    if (ret < 0) {
                        emit alertToShow(QString("Could not load the moviefile associated with the savestate"));
                        return false;
                    }

                    /* Checking if our movie is a prefix of the savestate movie */
                    if (!savedmovie.isPrefix(movie, context->framecount)) {
                        /* Not a prefix, we don't allow loading */
                        emit alertToShow(QString("We already diverged from the savestate movie"));
                        return false;
                    }

                    /* Loading the movie */
                    emit inputsToBeChanged();
                    movie.loadInputs(moviepath);
                    emit inputsChanged();

                    /* Return if we already are on the correct frame */
                    if (context->framecount == movie.savestateFramecount())
                        return false;

                    /* Fast-forward to savestate frame */
                    context->config.sc.recording = SharedConfig::RECORDING_READ;
                    context->config.sc.movie_framecount = movie.nbFrames();
                    context->pause_frame = movie.savestateFramecount();
                    context->config.sc.running = true;
                    context->config.sc.fastforward = true;
                    context->config.sc_modified = true;

                    emit sharedConfigChanged();

                    return false;
                }

                else {
                    if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                        std::string message = "No savestate in slot ";
                        message += std::to_string(statei);
                        sendMessage(MSGN_OSD_MSG);
                        sendString(message);
                    }
                    else {
                        emit alertToShow(QString("There is no savestate to load in this slot"));
                    }
                    return false;
                }
            }

            /* Send savestate path */
            if (! context->config.sc.savestates_in_ram) {
                sendMessage(MSGN_SAVESTATE_PATH);
                sendString(savestatepath);
            }


            /* When loading in read mode, we don't allow loading a non-prefix movie */
            if (context->config.sc.recording == SharedConfig::RECORDING_READ) {

                /* Checking if the savestate movie is a prefix of our movie */
                MovieFile savedmovie(context);
                int ret = savedmovie.loadInputs(moviepath);
                if (ret < 0) {
                    emit alertToShow(QString("Could not load the moviefile associated with the savestate"));
                    return false;
                }

                if (!movie.isPrefix(savedmovie)) {
                    /* Not a prefix, we don't allow loading */
                    if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                        sendMessage(MSGN_OSD_MSG);
                        sendString(std::string("Savestate inputs mismatch"));
                    }
                    else {
                        emit alertToShow(QString("Trying to load a state in read-only but the inputs mismatch"));
                    }
                    return false;
                }
            }

            if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                std::string msg;
                if (hk.type == HOTKEY_LOADSTATE_BACKTRACK) {
                    msg = "Loading backtrack state ";
                }
                else {
                    msg = "Loading state ";
                    msg += std::to_string(statei);
                }
                sendMessage(MSGN_OSD_MSG);
                sendString(msg);
            }

            sendMessage(MSGN_LOADSTATE);

            emit inputsToBeChanged();

            int message = receiveMessage();
            /* Loading is not assured to succeed, the following must
             * only be done if it's the case.
             */

            bool didLoad = message == MSGB_LOADING_SUCCEEDED;
            if (didLoad) {
                /* The copy of SharedConfig that the game stores may not
                 * be the same as this one due to memory loading, so we
                 * send it.
                 */
                sendMessage(MSGN_CONFIG);
                sendData(&context->config.sc, sizeof(SharedConfig));

                if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                    /* When in writing move, we load the movie associated
                     * with the savestate.
                     */
                    movie.loadInputs(moviepath);
                }

                /* If the movie was modified since last state load, increment
                 * the rerecord count. */
                if (movie.modifiedSinceLastStateLoad) {
                    context->rerecord_count++;
                    emit rerecordChanged();
                    movie.modifiedSinceLastStateLoad = false;
                }

                message = receiveMessage();

                emit savestatePerformed(statei, 0);
            }

            /* The frame count has changed, we must get the new one */
            if (message != MSGB_FRAMECOUNT_TIME) {
                std::cerr << "Got wrong message after state loading" << std::endl;

                if (!context->config.sc.opengl_soft) {
                    emit alertToShow(QString("Crash after loading the savestate. Savestates are unstable unless you check Video>Force software rendering"));
                }

                return false;
            }
            receiveData(&context->framecount, sizeof(unsigned long));
            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                context->config.sc.movie_framecount = context->framecount;
            }
            receiveData(&context->current_time, sizeof(struct timespec));

            emit inputsChanged();
            emit frameCountChanged();

            if (didLoad && (context->config.sc.osd & SharedConfig::OSD_MESSAGES)) {
                std::string msg;
                if (hk.type == HOTKEY_LOADSTATE_BACKTRACK) {
                    msg = "Backtrack state loaded";
                }
                else {
                    msg = "State ";
                    msg += std::to_string(statei);
                    msg += " loaded";
                }
                sendMessage(MSGN_OSD_MSG);
                sendString(msg);
            }

            sendMessage(MSGN_EXPOSE);

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
            return false;

        /* Recalibrate the mouse cursor position */
        case HOTKEY_CALIBRATE_MOUSE:
            if (context->game_window == 0) {
                break;
            }

            if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
                break;
            }

            {
                /* Change the cursor shape */
                uint32_t value_list = context->crosshair_cursor;
                xcb_void_cookie_t cwa_cookie = xcb_change_window_attributes (context->conn, context->game_window, XCB_CW_CURSOR, &value_list);
                xcb_generic_error_t *error = xcb_request_check(context->conn, cwa_cookie);
                if (error) {
                    std::cerr << "error in xcb_change_window_attributes: " << error->error_code << std::endl;
                }

                /* Wait for a mouse press. We cannot use mouse press events
                 * because only one window can select mouse press events.
                 * So we are using the mouse query function.
                 */
                usleep(500*1000);

                xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(context->conn, context->game_window);
                xcb_query_pointer_reply_t* pointer_reply = xcb_query_pointer_reply(context->conn, pointer_cookie, nullptr);

                while (!(pointer_reply->mask & XCB_BUTTON_MASK_1)) {
                    free(pointer_reply);
                    usleep(10*1000);
                    pointer_cookie = xcb_query_pointer(context->conn, context->game_window);
                    pointer_reply = xcb_query_pointer_reply(context->conn, pointer_cookie, nullptr);
                }

                /* Wait for mouse release */
                while (pointer_reply->mask & XCB_BUTTON_MASK_1) {
                    free(pointer_reply);
                    usleep(10*1000);
                    pointer_cookie = xcb_query_pointer(context->conn, context->game_window);
                    pointer_reply = xcb_query_pointer_reply(context->conn, pointer_cookie, nullptr);
                }

                int pointer_x = pointer_reply->win_x;
                int pointer_y = pointer_reply->win_y;

                free(pointer_reply);

                /* Set our calibration offsets */
                pointer_offset_x = prev_ai.pointer_x - pointer_x;
                pointer_offset_y = prev_ai.pointer_y - pointer_y;

                /* Switch back to default cursor */
                value_list = 0;
                cwa_cookie = xcb_change_window_attributes (context->conn, context->game_window, XCB_CW_CURSOR, &value_list);
                error = xcb_request_check(context->conn, cwa_cookie);
                if (error) {
                    std::cerr << "error in xcb_change_window_attributes: " << error->error_code << std::endl;
                }
            }
            return false;

        } /* switch(hk.type) */
        break;

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
    return false;
}


void GameLoop::sleepSendPreview()
{
    /* Sleep a bit to not surcharge the processor */
    struct timespec tim = {0, 17L*1000L*1000L};
    nanosleep(&tim, NULL);

    /* Send a preview of inputs so that the game can display them
     * on the HUD */
#ifdef LIBTAS_ENABLE_HUD

    /* Don't preview when reading inputs */
    if (context->config.sc.recording == SharedConfig::RECORDING_READ)
        return;

    /* Only preview if input focus */
    if (!haveFocus())
        return;

    /* Only preview if we actually print inputs */
    if (!(context->config.sc.osd & SharedConfig::OSD_INPUTS))
        return;

    /* Format the keyboard and mouse state and save it in the AllInputs struct */
    static AllInputs preview_ai, last_preview_ai;
    context->config.km.buildAllInputs(preview_ai, context->conn, context->game_window, keysyms.get(), context->config.sc);
    preview_ai.pointer_x += pointer_offset_x;
    preview_ai.pointer_y += pointer_offset_y;
    emit inputsToBeSent(preview_ai);

    /* Send inputs if changed */
    if (!(preview_ai == last_preview_ai)) {
        sendMessage(MSGN_PREVIEW_INPUTS);
        sendData(&preview_ai, sizeof(AllInputs));
        last_preview_ai = preview_ai;
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
                ai.pointer_x += pointer_offset_x;
                ai.pointer_y += pointer_offset_y;
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

                /* If some inputs are locked, copy the inputs from the movie.
                 * Only do this if the input editor is opened, and if there are
                 * inputs to copy. */
                if (keep_inputs && past_inputs)
                    movie.setLockedInputs(ai);

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

            AutoSave::update(context, movie);
            break;
    }
}

void GameLoop::endFrameMessages(AllInputs &ai)
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
        sendString(context->config.ffmpegoptions);
        context->config.dumpfile_modified = false;
    }

    /* Send inputs and end of frame */
    sendMessage(MSGN_ALL_INPUTS);
    sendData(&ai, sizeof(AllInputs));

    if ((context->status == Context::QUITTING) || (context->status == Context::RESTARTING)) {
        sendMessage(MSGN_USERQUIT);
    }

    sendMessage(MSGN_END_FRAMEBOUNDARY);
}


bool GameLoop::haveFocus()
{
    xcb_window_t window;

    xcb_generic_error_t* error;
    xcb_get_input_focus_cookie_t focus_cookie = xcb_get_input_focus(context->conn);
    xcb_get_input_focus_reply_t* focus_reply = xcb_get_input_focus_reply(context->conn, focus_cookie, &error);
    if (error) {
        std::cerr << "Could not get focussed window, X error" << error->error_code << std::endl;
    }

    window = focus_reply->focus;
    free(focus_reply);

    return (window == context->game_window);
}

void GameLoop::loopExit()
{
    /* We need to restart the game if we got a restart input, or if:
     * - auto-restart is set
     * - we are playing or recording a movie
     * - the user didn't use the Stop button to stop the game
     */

    if ((context->status == Context::RESTARTING) ||
        (context->config.auto_restart &&
        (context->config.sc.recording != SharedConfig::NO_RECORDING) &&
        (context->status != Context::QUITTING))) {

        /* We keep the movie opened and indicate the main thread to restart the game */

        closeSocket();

        /* Remove savestates because they are invalid on future instances of the game */
        remove_savestates(context);

        context->status = Context::RESTARTING;
        emit statusChanged();

        return;
    }

    if (movie.modifiedSinceLastSave) {

        /* Ask the user if he wants to save the movie, and get the answer.
         * Prompting a alert window must be done by the UI thread, so we are
         * using std::future/std::promise mechanism.
         */
        std::promise<bool> saveAnswer;
        std::future<bool> futureSave = saveAnswer.get_future();
        emit askToShow(QString("Do you want to save the movie file?"), &saveAnswer);

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
}

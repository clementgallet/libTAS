/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef __unix__
#include "config.h"
#endif
#include "GameLoop.h"
#include "GameThread.h"
#include "GameEvents.h"
#include "AutoDetect.h"

#ifdef __unix__
#include "GameEventsXcb.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "GameEventsQuartz.h"
#endif

#include "Context.h"
#include "utils.h"
#include "AutoSave.h"
#include "SaveStateList.h"
#include "lua/Input.h"
#include "lua/Callbacks.h"
#include "lua/NamedLuaFunction.h"
#include "ramsearch/MemAccess.h"
#include "ramsearch/BaseAddresses.h"
#include "ui/InputEditorView.h"

#include "../shared/sockethelpers.h"
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"
#include "../shared/inputs/AllInputs.h"
#include "../shared/inputs/ControllerInputs.h"

#include <string>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <unistd.h> // fork()
#include <future>
#include <csignal> // kill
#include <sys/stat.h> // stat
#include <sys/wait.h> // waitpid
#include <stdint.h>
#include <cstdlib>

GameLoop::GameLoop(Context* c) : movie(MovieFile(c)), context(c)
{
#ifdef __unix__
    gameEvents = new GameEventsXcb(c, &movie);
#elif defined(__APPLE__) && defined(__MACH__)
    gameEvents = new GameEventsQuartz(c, &movie);
#endif
}

void GameLoop::start()
{
    init();
    initProcessMessages();

    Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackStartup);

    while (1)
    {
        bool exitMsg = startFrameMessages();
        if (exitMsg) {
            loopExit();
            return;
        }

        emit uiChanged();
        emit newFrame();
        
        /* We are at a frame boundary */
        /* If we did not yet receive the game window id, just make the game running */
        bool endInnerLoop = false;
        if (context->game_window ) do {

            /* Check if game is still running */
            int ret = waitpid(fork_pid, nullptr, WNOHANG);
            if (ret == fork_pid) {
                emit alertToShow(QString("Game was closed"));
                loopExit();
                return;
            }

            int eventFlag = gameEvents->handleEvent();

            if (eventFlag & GameEvents::RETURN_FLAG_UPDATE)
                emit uiChanged();

            endInnerLoop = context->config.sc.running ||
                (eventFlag & GameEvents::RETURN_FLAG_ADVANCE) ||
                (context->status == Context::QUITTING);

            if (!endInnerLoop) {
                sleepSendPreview();
            }
        } while (!endInnerLoop);

        AllInputs ai;
        processInputs(ai);

        /* Set the status to restart */
        if (ai.misc && (ai.misc->flags & (1 << SingleInput::FLAG_RESTART))) {
            context->status = Context::RESTARTING;
        }

        bool shouldQuit = false;

        /* Toggle pause when reaching a marker */
        if (context->config.editor_marker_pause) {
            if (movie.editor->markers.count(context->framecount+1)) {
                context->config.sc.running = false;
                context->config.sc.fastforward = false;
                context->config.sc_modified = true;
                emit sharedConfigChanged();
            }
        }

        /* Pause if needed */
        if ((context->pause_frame == (context->framecount + 1)) ||
            ((context->config.sc.recording == SharedConfig::RECORDING_READ) &&
            ((context->config.sc.movie_framecount + context->pause_frame) == (context->framecount + 1)))) {

            if (!context->interactive) {
                /* Quit at the end of the movie if non-interactive */
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

        if (context->seek_frame == (context->framecount + 1)) {
            /* Disable seek frame */
            context->seek_frame = 0;

            /* Pause and disable fast-forward */
            context->config.sc.running = false;
            context->config.sc.fastforward = false;
            context->config.sc_modified = true;
            emit sharedConfigChanged();
        }

        endFrameMessages(ai);

        if (shouldQuit) {
            context->status = Context::QUITTING;
            emit statusChanged(Context::QUITTING);
        }
    }
}

void GameLoop::init()
{
    /* Unvalidate the game pid */
    context->game_pid = 0;
    MemAccess::fini();
    
    /* Unvalidate the game window id */
    context->game_window = 0;

    /* Reset the frame count if not restarting */
    if (context->status != Context::RESTARTING)
        context->framecount = 0;

    /* Reset the rerecord count if not restarting */
    if (context->status != Context::RESTARTING)
        context->rerecord_count = 0;

    /* Reset the encoding segment if not restarting */
    if (context->status != Context::RESTARTING)
        encoding_segment = 0;

    /* Extract the game executable name from the game executable path */
    context->gamename = fileFromPath(context->gamepath);

    /* Clear the event queue and parameters */
    gameEvents->init();

    /* Remove savestates again in case we did not exist cleanly the previous time */
    remove_savestates(context);

    /* Remove the file socket */
    int err = removeSocket();
    if (err != 0)
        emit alertToShow(QString("Could not remove socket file /tmp/libTAS.socket: %2").arg(strerror(err)));

    /* Clear addresses of loaded files */
    BaseAddresses::clear();

    /* Init savestate list */
    SaveStateList::init(context);

    /* Compute the MD5 hash of the game binary */
    context->md5_game.clear();
    std::ostringstream cmd;
    cmd << "md5sum -b \"" << context->gamepath << "\" 2> /dev/null";

    context->md5_game = queryCmd(cmd.str());
    if (context->md5_game.size() > 32)
        context->md5_game.resize(32);

    /* Only open the movie if we did not restart */
    if (context->status != Context::RESTARTING) {
        /* Opening a movie, which imports the inputs and parameters if in read mode,
         * or prepare a movie if in write mode.
         */
        if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
            int ret = movie.loadMovie();
            if (ret < 0) {
                emit alertToShow(MovieFile::errorString(ret));
                context->config.sc.recording = SharedConfig::NO_RECORDING;
            }
            else {
                /* Update the UI accordingly */
                emit configChanged();
            }

            /* Check md5 match */
            if ((!movie.header->md5_movie.empty()) && (context->md5_game.compare(movie.header->md5_movie) != 0))
                emit alertToShow(QString("Game executable hash does not match with the hash stored in the movie!"));

        }
        else {
            movie.clear();
        }
    }

    /* Detect common game engines and load some known settings. This is done
     * before forking, because it can modify settings used by both the game
     * process (env variables, commandline-options) and the libtas program
     * (shared config sent to the game on startup) */
    AutoDetect::game_engine(context);

    /* We fork here so that the child process calls the game. This is done after,
     * loading the movie so that it benefits from the movie settings. */
    fork_pid = fork();
    if (fork_pid == 0) {
        GameThread::launch(context);
    }

    /* We must add a blank frame in most cases */
    if (context->config.sc.recording != SharedConfig::RECORDING_READ) {
        /* Add one blank frame in every movie corresponding to the input
         * between the game startup and the first screen draw, which is for now
         * impossible to set. Exception is if we restarted.
         */
        if (context->framecount == movie.inputs->nbFrames()) {
            AllInputs ai;
            ai.clear();
            movie.inputs->setInputs(ai, false);
            /* Process the input change immediatly, we don't want to wait for
             * the next frame because the framecount will be 1 */
            movie.inputs->processPendingActions();
        }
    }

    /* Set the current realtime to the initial time, except when restarting */
    if (context->status != Context::RESTARTING) {
        context->current_time_sec = context->config.sc.initial_monotonic_time_sec;
        context->current_time_nsec = context->config.sc.initial_monotonic_time_nsec;
        context->current_realtime_sec = context->config.sc.initial_time_sec;
        context->current_realtime_nsec = context->config.sc.initial_time_nsec;
        context->new_realtime_sec = context->current_realtime_sec;
        context->new_realtime_nsec = context->current_realtime_nsec;
    }

    /* If auto-restart is set, write back savefiles on game exit */
    context->config.sc.write_savefiles_on_exit =
        (context->config.sc.recording != SharedConfig::NO_RECORDING) &&
        (context->config.auto_restart);
}

void GameLoop::initProcessMessages()
{
    /* Connect to the socket between the program and the game */
    bool inited = initSocketProgram(fork_pid);
    if (!inited) {
        loopExit();
        return;
    }

    /* Receive informations from the game */
    int message = receiveMessage();
    while (message != MSGB_END_INIT) {

        switch (message) {
            /* Get the game process pid */
            case MSGB_PID_ARCH: {
                int addr_size;
                receiveData(&context->game_pid, sizeof(pid_t));
                receiveData(&addr_size, sizeof(int));
                MemAccess::init(context->game_pid, addr_size);
                break;
            }

            case MSGB_GIT_COMMIT:
                {
                    std::string lib_commit = receiveString();
#ifdef LIBTAS_INTERIM_COMMIT
                    std::string gui_commit = LIBTAS_INTERIM_COMMIT;
                    if (lib_commit.compare(gui_commit) != 0) {
                        std::cerr << "Interim commit of GUI (" << gui_commit << ") does not match commit of library (" << lib_commit << ")!" << std::endl;
                    }
#else
                    std::cerr << "Library has interim commit (" << lib_commit << ") but not GUI!" << std::endl;
#endif
                }
                break;

            default:
                // ui_print("Message init: unknown message\n");
                loopExit();
                return;
        }
        message = receiveMessage();
    }

    /* Send informations to the game */

    /* Send shared config size */
    sendMessage(MSGN_CONFIG_SIZE);
    int config_size = sizeof(SharedConfig);
    sendData(&config_size, sizeof(int));

    /* Send shared config */

    /* This is a bit hackish, change the initial time to the current realtime before
     * sending so that the game gets the correct time after restarting. */
    struct timespec it = {context->config.sc.initial_time_sec, context->config.sc.initial_time_nsec};
    context->config.sc.initial_time_sec = context->current_realtime_sec;
    context->config.sc.initial_time_nsec = context->current_realtime_nsec;
    sendMessage(MSGN_CONFIG);
    sendData(&context->config.sc, sizeof(SharedConfig));
    context->config.sc.initial_time_sec = it.tv_sec;
    context->config.sc.initial_time_nsec = it.tv_nsec;

    /* Send initial framecount and elapsed time */
    sendMessage(MSGN_INITIAL_FRAMECOUNT_TIME);
    sendData(&context->framecount, sizeof(uint64_t));
    sendData(&context->current_time_sec, sizeof(int64_t));
    sendData(&context->current_time_nsec, sizeof(int64_t));

    /* Send dump file if dumping from the beginning */
    if (context->config.sc.av_dumping) {
        sendMessage(MSGN_DUMP_FILE);
        sendString(context->config.dumpfile);
        sendString(context->config.ffmpegoptions);
    }

    /* Build and send the base savestate path/index */
    if (context->config.sc.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        sendMessage(MSGN_BASE_SAVESTATE_INDEX);
        int index = 0;
        sendData(&index, sizeof(int));

        if (!(context->config.sc.savestate_settings & SharedConfig::SS_RAM)) {
            std::string basesavestatepath = context->config.savestatedir + '/';
            basesavestatepath += context->gamename;
            basesavestatepath += ".state0";
            sendMessage(MSGN_BASE_SAVESTATE_PATH);
            sendString(basesavestatepath);
        }
    }

    /* Send the Steam user data path and remote storage */
    if (context->config.sc.virtual_steam) {
        sendMessage(MSGN_STEAM_USER_DATA_PATH);
        sendString(context->config.steamuserdir);
        std::string remotestorage = context->config.steamuserdir + "/";
        remotestorage += context->gamename;
        if (create_dir(remotestorage) < 0) {
            std::cerr << "Cannot create dir " << remotestorage << std::endl;
        }
        sendMessage(MSGN_STEAM_REMOTE_STORAGE);
        sendString(remotestorage);
    }

    sendMessage(MSGN_ENCODING_SEGMENT);
    sendData(&encoding_segment, sizeof(int));

    /* Sometime games have trouble finding the address of the orginal function
     * `SDL_DYNAPI_entry()` that we hook, so we send right away the symbol
     * address if there is one */
    uint64_t sdl_addr = getSymbolAddress("SDL_DYNAPI_entry", context->gameexecutable.c_str());
    if (sdl_addr != 0) {
        /* If the executable is position-independent (pie), it will be mapped
         * somewhere, and the symbol is only an offset, so we need the base 
         * address of the executable and adds to it. We use `file` to determine
         * if the executable is pie */
        int gameArch = extractBinaryType(context->gameexecutable);
        if (gameArch & BT_PIEAPP) {
            uintptr_t base_executable = BaseAddresses::getBaseAddress(context->gamename.c_str());
            if (base_executable == 0) {
                std::cerr << "Could not find base address of " <<  context->gamename << std::endl;
            }
            sdl_addr += base_executable;
        }

        sendMessage(MSGN_SDL_DYNAPI_ADDR);
        sendData(&sdl_addr, sizeof(uint64_t));
    }

    /* Check for `UnityPlayer_s.debug` presence and send symbol addresses */
    std::string debugfile = dirFromPath(context->gameexecutable) + "/UnityPlayer_s.debug";
    if (access(debugfile.c_str(), F_OK) == 0) {
        
        uintptr_t base_unity = BaseAddresses::getBaseAddress("UnityPlayer.so");
        uint64_t futexwait_addr = getSymbolAddress("_ZN12UnityClassic24Baselib_SystemFutex_WaitEPiij", debugfile.c_str());
        if (futexwait_addr != 0) {
            futexwait_addr += base_unity;
            sendMessage(MSGN_UNITY_WAIT_ADDR);
            sendData(&futexwait_addr, sizeof(uint64_t));
        }
        
        if (sdl_addr == 0) {
            /* Look again inside UnityPlayer.so */
            uint64_t sdl_addr = getSymbolAddress("SDL_DYNAPI_entry", debugfile.c_str());
            if (sdl_addr != 0) {
                sdl_addr += base_unity;
                sendMessage(MSGN_SDL_DYNAPI_ADDR);
                sendData(&sdl_addr, sizeof(uint64_t));
            }
        }
    }

    /* End message */
    sendMessage(MSGN_END_INIT);
    
    context->status = Context::ACTIVE;
    emit statusChanged(context->status);
}

uint64_t GameLoop::getSymbolAddress(const char* symbol, const char* file)
{
    std::ostringstream cmd;
    cmd << "readelf -Ws \"" << file << "\" | grep " << symbol << " | awk '{print $2}'";

    uint64_t addr = std::strtoull(queryCmd(cmd.str()).c_str(), nullptr, 16);    
    return addr;
}

bool GameLoop::startFrameMessages()
{
    context->draw_frame = true;
    
    /* Indicate if the current frame rendering will be skipped due to fast-forward */
    bool skip_draw_frame = false;
    
    /* Wait for frame boundary */
    int message = receiveMessage();

    while (message != MSGB_START_FRAMEBOUNDARY) {
        GameInfo game_info;

        switch (message) {
        case MSGB_WINDOW_ID:
        {
            uint32_t int_window;
            receiveData(&int_window, sizeof(uint32_t));
            gameEvents->registerGameWindow(int_window);
            break;
        }

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
            receiveData(&context->framecount, sizeof(uint64_t));
            receiveData(&context->current_time_sec, sizeof(uint64_t));
            receiveData(&context->current_time_nsec, sizeof(uint64_t));
            receiveData(&context->current_realtime_sec, sizeof(uint64_t));
            receiveData(&context->current_realtime_nsec, sizeof(uint64_t));
            context->new_realtime_sec = context->current_realtime_sec;
            context->new_realtime_nsec = context->current_realtime_nsec;    

            // if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
            //     /* If the input editor is opened, recording does not truncate inputs */
            //     bool notTruncInputs = false;
            //     emit isInputEditorVisible(notTruncInputs);
            // 
            //     if (!notTruncInputs || (context->framecount > context->config.sc.movie_framecount)) {
            //         context->config.sc.movie_framecount = context->framecount;
            //         movie.header->length_sec = context->current_time_sec - context->config.sc.initial_monotonic_time_sec;
            //         movie.header->length_nsec = context->current_time_nsec - context->config.sc.initial_monotonic_time_nsec;
            //         if (movie.header->length_nsec < 0) {
            //             movie.header->length_nsec += 1000000000;
            //             movie.header->length_sec--;
            //         }
            //     }
            // }
            
            /* Check and update the moviefile length when reaching the end of
             * the movie, useful when variable framerate is being used */
            // else if (context->config.sc.recording == SharedConfig::RECORDING_READ && 
            //     context->framecount == context->config.sc.movie_framecount) {
            // 
            //     uint64_t cur_sec = context->current_time_sec - context->config.sc.initial_monotonic_time_sec;
            //     uint64_t cur_nsec = context->current_time_nsec - context->config.sc.initial_monotonic_time_nsec;
            // 
            //     if (movie.header->length_sec != cur_sec ||
            //         movie.header->length_nsec != cur_nsec) {
            // 
            //         if (movie.header->length_sec != -1)
            //             emit alertToShow(QString("Movie length mismatch. Metadata stores %1.%2 seconds but end time is %3.%4 seconds.").arg(movie.header->length_sec).arg(movie.header->length_nsec, 9, 10, QChar('0')).arg(cur_sec).arg(cur_nsec, 9, 10, QChar('0')));
            // 
            //         movie.header->length_sec = cur_sec;
            //         movie.header->length_nsec = cur_nsec;
            //         movie.inputs->wasModified();
            //     }
            // }
            
            /* When not recording, always truncate the movie */
            // if (context->config.sc.recording == SharedConfig::NO_RECORDING) {
            //     context->config.sc.movie_framecount = context->framecount;
            //     movie.header->length_sec = context->current_time_sec - context->config.sc.initial_monotonic_time_sec;
            //     movie.header->length_nsec = context->current_time_nsec - context->config.sc.initial_monotonic_time_nsec;
            //     if (movie.header->length_nsec < 0) {
            //         movie.header->length_nsec += 1000000000;
            //         movie.header->length_sec--;
            //     }
            // }

            break;
        case MSGB_GAMEINFO:
            receiveData(&game_info, sizeof(game_info));
            emit gameInfoChanged(game_info);
            break;
        case MSGB_FPS:
            receiveData(&context->fps, sizeof(float));
            receiveData(&context->lfps, sizeof(float));
            break;
        case MSGB_ENCODING_SEGMENT:
            receiveData(&encoding_segment, sizeof(int));
            break;
        case MSGB_GETTIME_BACKTRACE:
        {
            int type;
            receiveData(&type, sizeof(int));
            uint64_t hash;
            receiveData(&hash, sizeof(uint64_t));
            std::string trace = receiveString();
            emit getTimeTrace(type, static_cast<unsigned long long>(hash), trace);
        }
        break;
        case MSGB_NONDRAW_FRAME:
            context->draw_frame = false;
            break;

        case MSGB_SKIPDRAW_FRAME:
            skip_draw_frame = true;
            break;

        case MSGB_SYMBOL_ADDRESS: {
            std::string sym = receiveString();
            uint64_t addr = getSymbolAddress(sym.c_str(), context->gameexecutable.c_str());
            sendData(&addr, sizeof(uint64_t));
            break;
        }
        case MSGB_QUIT:
            if (!context->interactive) {
                /* Exit the program when game has exit */
                exit(0);
            }
            return true;
        case -1:
            std::cerr << "The connection to the game was lost. Exiting" << std::endl;
            return true;
        case -2:
            std::cerr << "The connection to the game was closed. Exiting" << std::endl;
            return true;
        default:
            std::cerr << "Got unknown message!!!" << std::endl;
            return true;
        }
        message = receiveMessage();
    }

    Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackFrame);

    /* Store in movie and indicate the input editor if the current frame
     * is a draw frame or not */
    movie.editor->setDraw(context->draw_frame);

    /* Send ram watches */
    if (context->draw_frame && !skip_draw_frame) {
        std::string ramwatch;
        emit getRamWatch(ramwatch);
        while(!ramwatch.empty()) {
            sendMessage(MSGN_RAMWATCH);
            sendString(ramwatch);
            emit getRamWatch(ramwatch);
        }
    }

    /* Execute the lua callback onPaint here */
    if (context->draw_frame && !skip_draw_frame)
        Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackPaint);

    sendMessage(MSGN_START_FRAMEBOUNDARY);

    return false;
}

void GameLoop::sleepSendPreview()
{
    /* Sleep a bit to not surcharge the processor */
    struct timespec tim = {0, 33L*1000L*1000L};
    nanosleep(&tim, NULL);

    /* Send marker text if it has changed */
    static std::string old_marker_text;
    std::string text;
    emit getMarkerText(text);
    if (old_marker_text != text) {
        old_marker_text = text;
        sendMessage(MSGN_MARKER);
        sendString(text);
    }

    /* Send a preview of inputs so that the game can display them
     * on the HUD */

    /* Don't preview when reading inputs */
    if (context->config.sc.recording == SharedConfig::RECORDING_READ) {
        sendMessage(MSGN_EXPOSE);
        return;
    }

    static AllInputs preview_ai, last_preview_ai;
    if (gameEvents->haveFocus()) {
        /* Format the keyboard and mouse state and save it in the AllInputs struct */
        context->config.km->buildAllInputs(preview_ai, context->game_window, context->config.sc, false, context->mouse_wheel);
    }

    /* Fill controller inputs from the controller input window. */
    for (int j = 0; j < AllInputs::MAXJOYS; j++) {
        if (preview_ai.controllers[j]) {
            emit fillControllerInputs(*preview_ai.controllers[j], j);            
        }
    }

    /* Send inputs if changed */
    if (!(preview_ai == last_preview_ai)) {
        preview_ai.send(true);
        last_preview_ai = preview_ai;
    }

    sendMessage(MSGN_EXPOSE);
}


void GameLoop::processInputs(AllInputs &ai)
{
    ai.clear();

    /* Don't record inputs if we are quitting */
    if (context->status == Context::QUITTING)
        return;

    /* Record inputs or get inputs from movie file */
    switch (context->config.sc.recording) {
        case SharedConfig::NO_RECORDING:
        case SharedConfig::RECORDING_WRITE:

            /* Get inputs if we have input focus */
            if (gameEvents->haveFocus()) {
                /* Format the keyboard and mouse state and save it in the AllInputs struct */
                context->config.km->buildAllInputs(ai, context->game_window, context->config.sc, context->config.mouse_warp, context->mouse_wheel);

                /* Reset mouse wheel */
                context->mouse_wheel = 0;
            }
            
            /* Scale mouse inputs in case the game window is detached */
            if (context->config.sc.mouse_support && ai.pointer) {
                sendMessage(MSGN_SCALE_POINTER_INPUTS);
                sendData(ai.pointer.get(), sizeof(MouseInputs));
                receiveData(ai.pointer.get(), sizeof(MouseInputs));
            }
            
            /* Fill controller inputs from the controller input window. */
            for (int j = 0; j < AllInputs::MAXJOYS; j++) {
                if (ai.controllers[j]) {
                    emit fillControllerInputs(*ai.controllers[j], j);            
                }
                else if (j < context->config.sc.nb_controllers) {
                    /* If we didn't created an object yet, but if the user
                     * did set something on the controller panel, then create
                     * the object. */
                    ControllerInputs ci;
                    /* Clear the controller inputs incase the joystick input editor
                     * is not open to prevent it from reading garbade data
                     * and adding the joystick axes to the input editor. */
                    ci.clear();
                    emit fillControllerInputs(ci, j);
                    if (!ci.isDefaultController())
                        ai.controllers[j].reset(new ControllerInputs(ci));
                }
            }

            /* Add framerate if necessary */
            if ((context->current_framerate_num != context->config.sc.initial_framerate_num) || 
            (context->current_framerate_num != context->config.sc.initial_framerate_num)) {
                if (!ai.misc)
                    ai.misc.reset(new MiscInputs{});
                ai.misc->framerate_num = context->current_framerate_num;
                ai.misc->framerate_den = context->current_framerate_den;
            }

            /* Add realtime if necessary */
            if ((context->new_realtime_sec != context->current_realtime_sec) || 
                (context->new_realtime_nsec != context->current_realtime_nsec)) {
                if (!ai.misc)
                    ai.misc.reset(new MiscInputs{});
                ai.misc->realtime_sec = context->new_realtime_sec;
                ai.misc->realtime_nsec = context->new_realtime_nsec;
            }

            /* Call lua onInput() here so that a script can modify inputs */
            Lua::Input::registerInputs(&ai);
            Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackInput);

            if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                /* If the input editor is visible, we should keep future inputs.
                 * If not, we truncate inputs if necessary.
                 */
                bool keep_inputs = false;
                bool past_inputs = context->framecount < movie.inputs->nbFrames();
                emit isInputEditorVisible(keep_inputs);

                /* If some inputs are locked, copy the inputs from the movie.
                 * Only do this if the input editor is opened, and if there are
                 * inputs to copy. */
                if (keep_inputs && past_inputs)
                    movie.setLockedInputs(ai);

                /* Save inputs to moviefile */
                movie.inputs->setInputs(ai, keep_inputs);

                AutoSave::update(context, movie);
            }
            else {
                /* Just save the inputs into the backup movie */
                movie.inputs->setInputs(ai, false);
            }
            break;

        case SharedConfig::RECORDING_READ:
            /* Read inputs from file */
            uint64_t moviecount = movie.inputs->size();
            
            if ((context->framecount + 1) == moviecount) {
                /* We are reading the last frame of the movie */
                switch(context->config.on_movie_end) {
                    case Config::MOVIEEND_READ:
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

            if (context->framecount < moviecount) { // movie has inputs for current frame
                ai = movie.inputs->getInputs();

                /* Allow lua to modify movie inputs even in playback mode */
                Lua::Input::registerInputs(&ai);
                bool movieMayBeModifed = Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackInput);
                if (movieMayBeModifed)
                    movie.inputs->setInputs(ai, true);

                /* Update framerate */
                uint32_t new_framerate_num = 0;
                uint32_t new_framerate_den = 0;
                
                if (ai.misc) {
                    new_framerate_num = ai.misc->framerate_num;
                    new_framerate_den = ai.misc->framerate_den;
                }
                
                if (!new_framerate_num)
                    new_framerate_num = context->config.sc.initial_framerate_num;

                if (!new_framerate_den)
                    new_framerate_den = context->config.sc.initial_framerate_den;

                if ((context->current_framerate_num != new_framerate_num) ||
                    (context->current_framerate_den != new_framerate_den)) {
                    
                    context->current_framerate_num = new_framerate_num;
                    context->current_framerate_den = new_framerate_den;
                    emit updateFramerate();
                }

                /* Update realtime */
                if (ai.misc && ai.misc->realtime_sec) {
                    context->current_realtime_sec = ai.misc->realtime_sec;
                    context->current_realtime_nsec = ai.misc->realtime_nsec;
                    context->new_realtime_sec = ai.misc->realtime_sec;
                    context->new_realtime_nsec = ai.misc->realtime_nsec;
                }
            }
            else {
                ai.clear();

                /* Allow lua to modify inputs past the end of the movie */
                Lua::Input::registerInputs(&ai);
                Lua::Callbacks::call(Lua::NamedLuaFunction::CallbackInput);
            }

            /* Update controller inputs if controller window is shown */
            for (int j = 0; j < AllInputs::MAXJOYS; j++) {
                if (ai.controllers[j]) {
                    emit showControllerInputs(*ai.controllers[j], j);
                }
            }

            AutoSave::update(context, movie);
            break;
    }
    
    movie.inputs->processPendingActions();
}

void GameLoop::endFrameMessages(AllInputs &ai)
{
    /* If the user stopped the game with the Stop button, don't write back
     * savefiles.*/
    if (context->status == Context::QUITTING) {
        context->config.sc.write_savefiles_on_exit = false;
        context->config.sc_modified = true;
    }

    /* If the game was restarted, write back savefiles.*/
    if (context->status == Context::RESTARTING) {
        context->config.sc.write_savefiles_on_exit = true;
        context->config.sc_modified = true;
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
        sendString(context->config.ffmpegoptions);
        context->config.dumpfile_modified = false;
    }

    /* Send inputs and end of frame */
    ai.send(false);

    if ((context->status == Context::QUITTING) || (context->status == Context::RESTARTING)) {
        sendMessage(MSGN_USERQUIT);
    }

    sendMessage(MSGN_END_FRAMEBOUNDARY);
}

void GameLoop::loopExit()
{
    /* Unvalidate the game pid */
    context->game_pid = 0;

    /* Stop the appimage process */
    if (context->appimage_pid) {
        kill(context->appimage_pid, SIGINT);
        context->appimage_pid = 0;
    }

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

        /* Backup savestate movies on disk */
        SaveStateList::backupMovies();

        /* wait on the game process to terminate */
        wait(nullptr);

        context->status = Context::RESTARTING;
        emit statusChanged(context->status);

        return;
    }

    /* When not recording, silently save a backup movie and inform the user */
    if (context->config.sc.recording == SharedConfig::NO_RECORDING) {
        movie.saveBackupMovie();
    }
    else if (movie.inputs->modifiedSinceLastSave) {

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

    /* Backup savestate movies on disk */
    SaveStateList::backupMovies();

    /* Unvalidate game pid */
    MemAccess::init(0, 0);

    /* Reset the frame count */
    context->framecount = 0;

    /* Remove fast-forward */
    context->config.sc.fastforward = false;
    emit sharedConfigChanged();

    /* wait on the game process to terminate */
    wait(nullptr);

    context->status = Context::INACTIVE;
    emit statusChanged(Context::INACTIVE);
}

void GameLoop::killForkProcess()
{
    kill(fork_pid, SIGKILL);
}

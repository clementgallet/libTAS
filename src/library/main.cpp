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

#include "main.h"
#include <vector>
#include <string>
#include "dlhook.h"
#include "../shared/sockethelpers.h"
#include "logging.h"
#include "NonDeterministicTimer.h"
#include "DeterministicTimer.h"
#include "../shared/messages.h"
#include "../shared/SharedConfig.h"
#include "../shared/AllInputs.h"
#include "inputs/inputs.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/SaveStateManager.h"
#include "checkpoint/Checkpoint.h"
#include "audio/AudioContext.h"
#include "encoding/AVEncoder.h"
#include <unistd.h> // getpid()
#include "frame.h" // framecount
#include "steam/isteamuser.h" // SteamSetUserDataFolder
#include "steam/isteamremotestorage/isteamremotestorage.h" // SteamSetRemoteStorageFolder
#include "Stack.h"


extern char**environ;

namespace libtas {

void __attribute__((constructor)) init(void)
{
    /* If LIBTAS_DELAY_INIT env variable is > 0, skip initialization and
     * decrease env variable value */
    char* delay_str;
    NATIVECALL(delay_str = getenv("LIBTAS_DELAY_INIT"));
    if (delay_str && (delay_str[0] > '0')) {
        delay_str[0] -= 1;
        setenv("LIBTAS_DELAY_INIT", delay_str, 1);
        debuglogstdio(LCF_INFO, "Skipping libtas init");

        /* Setting native state so that we interact as little as possible
         * with the process */
        GlobalState::setNative(true);

        return;
    }

    /* Hacking `environ` to disable LD_PRELOAD for future processes */
    /* Taken from <https://stackoverflow.com/a/3275799> */
    for (int i=0; environ[i]; i++) {
        if ( strstr(environ[i], "LD_PRELOAD=") ) {
             environ[i][0] = 'D';
        }
        if ( strstr(environ[i], "LD_LIBRARY_PATH=") ) {
             environ[i][0] = 'D';
        }
        if ( strstr(environ[i], "DYLD_INSERT_LIBRARIES=") ) {
            environ[i][0] = 'Y';
        }
        if ( strstr(environ[i], "DYLD_FORCE_FLAT_NAMESPACE=") ) {
            environ[i][0] = 'Y';
        }
    }

    ThreadManager::init();
    SaveStateManager::init();
    Stack::grow();

    initSocketGame();

    /* Send information to the program */

    /* Send game process pid */
    sendMessage(MSGB_PID);
    pid_t mypid;
    NATIVECALL(mypid = getpid());
    debuglogstdio(LCF_SOCKET, "Send pid to program: %d", mypid);
    /* If I replace with the line below, then wine+SuperMeatBoy crashes on
     * on startup... */
    // debuglogstdio(LCF_SOCKET, "Send pid to program: ", mypid);
    sendData(&mypid, sizeof(pid_t));

    /* Send interim commit hash if one */
#ifdef LIBTAS_INTERIM_COMMIT
    std::string commit_hash = LIBTAS_INTERIM_COMMIT;
    sendMessage(MSGB_GIT_COMMIT);
    sendString(commit_hash);
#endif

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Receive information from the program */
    int message = receiveMessage();
    while (message != MSGN_END_INIT) {
        std::string basesavestatepath;
        std::string steamuserdatapath;
        std::string steamremotestorage;
        int index;
        int config_size;
        switch (message) {
            case MSGN_CONFIG_SIZE:
                debuglogstdio(LCF_SOCKET, "Receiving config size");
                receiveData(&config_size, sizeof(int));
                if (config_size != sizeof(SharedConfig)) {
                    debuglogstdio(LCF_SOCKET | LCF_ERROR, "Shared config size mismatch between program and library!");                    
                }
                break;
            case MSGN_CONFIG:
                debuglogstdio(LCF_SOCKET, "Receiving config");
                receiveData(&shared_config, sizeof(SharedConfig));
                break;
            case MSGN_DUMP_FILE:
                debuglogstdio(LCF_SOCKET, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglogstdio(LCF_SOCKET, "File %s", AVEncoder::dumpfile);
                receiveCString(AVEncoder::ffmpeg_options);
                break;
            case MSGN_BASE_SAVESTATE_PATH:
                basesavestatepath = receiveString();
                Checkpoint::setBaseSavestatePath(basesavestatepath);
                break;
            case MSGN_BASE_SAVESTATE_INDEX:
                receiveData(&index, sizeof(int));
                Checkpoint::setBaseSavestateIndex(index);
                break;
            case MSGN_ENCODING_SEGMENT:
                receiveData(&AVEncoder::segment_number, sizeof(int));
                break;
            case MSGN_STEAM_USER_DATA_PATH:
                steamuserdatapath = receiveString();
                SteamSetUserDataFolder(steamuserdatapath);
                break;
            case MSGN_STEAM_REMOTE_STORAGE:
                steamremotestorage = receiveString();
                SteamSetRemoteStorageFolder(steamremotestorage);
                break;
            default:
                debuglogstdio(LCF_ERROR | LCF_SOCKET, "Unknown socket message %d", message);
                exit(1);
        }
        message = receiveMessage();
    }

    if (shared_config.sigint_upon_launch) {
        raise(SIGINT);
    }

    /* Set the frame count to the initial frame count */
    framecount = shared_config.initial_framecount;

    ai.emptyInputs();
    old_ai.emptyInputs();
    game_ai.emptyInputs();
    old_game_ai.emptyInputs();
    game_unclipped_ai.emptyInputs();
    old_game_unclipped_ai.emptyInputs();

    /* Initialize timers. It uses the initial time set in the config object,
     * so they must be initialized after receiving it.
     */
    nonDetTimer.initialize();
    detTimer.initialize();

    /* Initialize sound parameters */
    audiocontext.init();

    is_inited = true;
}

void __attribute__((destructor)) term(void)
{
    if (is_inited) {
        if (!is_fork) {
            sendMessage(MSGB_QUIT);
            closeSocket();
        }
        debuglogstdio(LCF_SOCKET, "Exiting.");
        ThreadManager::deallocateThreads();
    }
}

}

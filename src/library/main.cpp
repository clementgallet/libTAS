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

#include "main.h"
#include "logging.h"
#include "global.h"
#include "NonDeterministicTimer.h"
#include "DeterministicTimer.h"
#include "frame.h" // framecount
#include "Stack.h"
#include "GlobalState.h"
#include "UnityHacks.h"
#include "audio/AudioContext.h"
#include "encoding/AVEncoder.h"
#include "steam/isteamuser/isteamuser.h" // SteamSetUserDataFolder
#include "general/dlhook.h"
#include "general/monowrappers.h"
#include "steam/isteamremotestorage/isteamremotestorage.h" // SteamSetRemoteStorageFolder
#include "inputs/inputs.h"
#include "checkpoint/ThreadManager.h"
#include "checkpoint/SaveStateManager.h"
#include "checkpoint/Checkpoint.h"
#include "sdl/sdldynapi.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "../shared/SharedConfig.h"

#include <unistd.h> // getpid()
#include <vector>
#include <string>
#include <sys/prctl.h>

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
        LOG(LL_INFO, LCF_NONE, "Skipping libtas init");

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

    /* Allow future gdb instances to debug this process */
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

    ThreadManager::init();
    SaveStateManager::init();
    Stack::grow();

    initSocketGame();

    /* Send information to the program */

    /* Send game process pid */
    sendMessage(MSGB_PID_ARCH);
    pid_t mypid;
    NATIVECALL(mypid = getpid());
    LOG(LL_DEBUG, LCF_SOCKET, "Send pid to program: %d", mypid);
    sendData(&mypid, sizeof(pid_t));
    int addr_size = sizeof(void*);
    sendData(&addr_size, sizeof(int));

    /* Send interim commit hash if one */
#ifdef LIBTAS_INTERIM_COMMIT
    std::string commit_hash = LIBTAS_INTERIM_COMMIT;
    sendMessage(MSGB_GIT_COMMIT);
    sendString(commit_hash);
#endif

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Initial framecount and elapsed time */
    int64_t initial_sec = 0, initial_nsec = 0;
    framecount = 0;

    /* Receive information from the program */
    int message = receiveMessage();
    while (message != MSGN_END_INIT) {
        switch (message) {
            case MSGN_CONFIG_SIZE: {
                LOG(LL_DEBUG, LCF_SOCKET, "Receiving config size");
                int config_size;
                receiveData(&config_size, sizeof(int));
                if (config_size != sizeof(SharedConfig)) {
                    LOG(LL_ERROR, LCF_SOCKET, "Shared config size mismatch between program and library!");                    
                }
                break;
            }
            case MSGN_CONFIG:
                LOG(LL_DEBUG, LCF_SOCKET, "Receiving config");
                receiveData(&Global::shared_config, sizeof(SharedConfig));
                break;
            case MSGN_DUMP_FILE:
                LOG(LL_DEBUG, LCF_SOCKET, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                LOG(LL_DEBUG, LCF_SOCKET, "File %s", AVEncoder::dumpfile);
                receiveCString(AVEncoder::ffmpeg_options);
                break;
            case MSGN_BASE_SAVESTATE_PATH: {
                std::string basesavestatepath = receiveString();
                Checkpoint::setBaseSavestatePath(basesavestatepath);
                break;                
            }
            case MSGN_BASE_SAVESTATE_INDEX: {
                int index;
                receiveData(&index, sizeof(int));
                Checkpoint::setBaseSavestateIndex(index);
                break;
            }
            case MSGN_ENCODING_SEGMENT:
                receiveData(&AVEncoder::segment_number, sizeof(int));
                break;
            case MSGN_STEAM_USER_DATA_PATH: {
                std::string steamuserdatapath = receiveString();
                SteamSetUserDataFolder(steamuserdatapath);
                break;
            }
            case MSGN_STEAM_REMOTE_STORAGE: {
                std::string steamremotestorage = receiveString();
                SteamSetRemoteStorageFolder(steamremotestorage);
                break;
            }
            case MSGN_INITIAL_FRAMECOUNT_TIME:
                /* Set the framecount and time to their initial values */
                receiveData(&framecount, sizeof(uint64_t));
                receiveData(&initial_sec, sizeof(int64_t));
                receiveData(&initial_nsec, sizeof(int64_t));
                break;
            case MSGN_SDL_DYNAPI_ADDR: {
                uint64_t addr;
                receiveData(&addr, sizeof(uint64_t));
                setDynapiAddr(addr);
                break;
            }
            case MSGN_UNITY_WAIT_ADDR: {
                uint64_t addr;
                receiveData(&addr, sizeof(uint64_t));
                UnityHacks::patch(addr);
                break;
            }
            default:
                LOG(LL_ERROR, LCF_SOCKET, "Unknown socket message %d", message);
                exit(1);
        }
        message = receiveMessage();
    }

    if (Global::shared_config.sigint_upon_launch) {
        raise(SIGINT);
    }


    /* Initialize timers. It uses the initial time set in the config object,
     * so they must be initialized after receiving it.
     */
    NonDeterministicTimer::get().initialize(initial_sec, initial_nsec);
    DeterministicTimer::get().initialize(initial_sec, initial_nsec);

    /* Initialize sound parameters */
    AudioContext::get().init();

    hook_mono();

    Global::is_inited = true;
}

void __attribute__((destructor)) term(void)
{
    if (Global::is_inited) {
        if (!Global::is_fork) {
            sendMessage(MSGB_QUIT);
            closeSocket();
        }
        LOG(LL_DEBUG, LCF_SOCKET, "Exiting.");
        ThreadManager::deallocateThreads();
    }
}

}

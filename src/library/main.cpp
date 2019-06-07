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
#include "checkpoint/Checkpoint.h"
#include "audio/AudioContext.h"
#include "encoding/AVEncoder.h"
#include "renderhud/RenderHUD.h"
#include <unistd.h> // getpid()
#include "frame.h" // framecount

extern char**environ;

namespace libtas {

// FIXME: should be declared in a header somewhere
// (implementation at library/steam/isteamuser.cpp)
void SteamUser_SetUserDataFolder(std::string path);

static bool is_inited = false;

void __attribute__((constructor)) init(void)
{
    /* Hacking `environ` to disable LD_PRELOAD for future processes */
    /* Taken from <https://stackoverflow.com/a/3275799> */
    
    char* delay_str;
    NATIVECALL(delay_str = getenv("LIBTAS_DELAY_INIT"));
    if (delay_str && (delay_str[0] > '0')) {
        delay_str[0] -= 1;
        setenv("LIBTAS_DELAY_INIT", delay_str, 1);
        debuglog(LCF_INFO, "Skipping libtas init");
        return;
    }
    
    for (int i=0; environ[i]; i++) {
        if ( strstr(environ[i], "LD_PRELOAD=") ) {
             environ[i][0] = 'D';
        }
        if ( strstr(environ[i], "LD_LIBRARY_PATH=") ) {
             environ[i][0] = 'D';
        }
    }

    ThreadManager::init();

    initSocketGame();

    /* Send information to the program */

    /* Send game process pid */
    sendMessage(MSGB_PID);
    pid_t mypid = getpid();
    debuglogstdio(LCF_SOCKET, "Send pid to program: %d", mypid);
    /* If I replace with the line below, then wine+SuperMeatBoy crashes on
     * on startup... */
    // debuglog(LCF_SOCKET, "Send pid to program: ", mypid);
    sendData(&mypid, sizeof(pid_t));

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Receive information from the program */
    int message;
    receiveData(&message, sizeof(int));
    while (message != MSGN_END_INIT) {
        std::string basesavestatepath;
        std::string steamuserdatapath;
        int index;
        switch (message) {
            case MSGN_CONFIG:
                debuglog(LCF_SOCKET, "Receiving config");
                receiveData(&shared_config, sizeof(SharedConfig));
                break;
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglog(LCF_SOCKET, "File ", AVEncoder::dumpfile);
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
                SteamUser_SetUserDataFolder(steamuserdatapath);
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message ", message);
                exit(1);
        }
        receiveData(&message, sizeof(int));
    }

    /* Set the frame count to the initial frame count */
    framecount = shared_config.initial_framecount;

    ai.emptyInputs();
    old_ai.emptyInputs();
    game_ai.emptyInputs();

    /* Initialize timers. It uses the initial time set in the config object,
     * so they must be initialized after receiving it.
     */
    nonDetTimer.initialize();
    detTimer.initialize();

    /* Initialize sound parameters */
    audiocontext.init();
    
#ifdef LIBTAS_ENABLE_HUD
    /* Load HUD fonts */
    RenderHUD::initFonts();
#endif

    is_inited = true;
}

void __attribute__((destructor)) term(void)
{
    if (is_inited) {
        ThreadManager::deallocateThreads();
        sendMessage(MSGB_QUIT);
        closeSocket();
        debuglog(LCF_SOCKET, "Exiting.");
    }
}

}

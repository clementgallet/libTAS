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

#include "libTAS.h"
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
#include "audio/AudioContext.h"
#include "AVEncoder.h"
#include <unistd.h> // getpid()

namespace libtas {

void __attribute__((constructor)) init(void)
{
    bool didConnect = initSocketGame();
    /* Sometimes the game starts a process that is not a thread, so that this constructor is called again
     * In this case, we must detect it and do not run this again
     */
    if (! didConnect)
        return;

    /* We are the first thread */
    setFirstThread();

    /* Send information to the program */

    /* Send game process pid */
    debuglog(LCF_SOCKET, "Send pid to program");
    sendMessage(MSGB_PID);
    pid_t mypid = getpid();
    sendData(&mypid, sizeof(pid_t));

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Receive information from the program */
    int message;
    receiveData(&message, sizeof(int));
    while (message != MSGN_END_INIT) {
        std::string libstring;
        switch (message) {
            case MSGN_CONFIG:
                debuglog(LCF_SOCKET, "Receiving config");
                receiveData(&shared_config, sizeof(SharedConfig));
                break;
#ifdef LIBTAS_ENABLE_AVDUMPING
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglog(LCF_SOCKET, "File ", AVEncoder::dumpfile);
                break;
#endif
            case MSGN_LIB_FILE:
                debuglog(LCF_SOCKET, "Receiving lib filename");
                libstring = receiveString();
                add_lib(libstring);
                debuglog(LCF_SOCKET, "Lib ", libstring.c_str());
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message ", message);
                exit(1);
        }
        receiveData(&message, sizeof(int));
    }

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

    ThreadManager::init();
}

void __attribute__((destructor)) term(void)
{
    ThreadManager::deallocateThreads();
    dlhook_end();

    sendMessage(MSGB_QUIT);

    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}

}

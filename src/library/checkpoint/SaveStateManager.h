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

#ifndef LIBTAS_SAVESTATE_MANAGER_H
#define LIBTAS_SAVESTATE_MANAGER_H

#include <set>
#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <cstddef>
#include <pthread.h>
#include <semaphore.h>

namespace libtas {

struct ThreadInfo;

namespace SaveStateManager {

/* List of error codes */
enum Error {
    ESTATE_OK = 0,
    ESTATE_UNKNOWN = -1, // Unknown error
    ESTATE_NOMEM = -2, // Not enough memory to perform savestate
    ESTATE_NOSTATE = -3, // No state in slot
    ESTATE_NOTSAMETHREADS = -4, // Thread list has changed
    ESTATE_NOTCOMPLETE = -5, // State still being saved
};


void init();

/* Initialize the signal handler for the checkpoint thread */
void initCheckpointThread();

void initThreadFromChild(ThreadInfo* thread);

/* Wait for a child to terminate and register the savestate slot */
int waitChild();

/* Returns if a state is completed (useful for fork savestates) */
bool stateReady(int slot);

/* Change the dirty state of savestate when doing forked savestate */
void stateStatus(int slot, bool dirty);

/* Save a savestate and returns if succeeded */
int checkpoint(int slot);

/* Restore a savestate */
int restore(int slot);

void terminateThreads();

/* Send a signal to suspend all threads before checkpointing */
void suspendThreads();

/* Resume all threads */
void resumeThreads();

/* Function executed by all secondary threads using signal SIGUSR1 */
void stopThisThread(int signum);

/* Save the list of all threads inside reserved memory */
void saveThreadList();

/* Create new threads that were destroyed since the savestate */
void createNewThreads();

/* Function passed to clone() for new threads */
int startNewThread(void *arg);

void waitForAllRestored(ThreadInfo *thread);

/* Is currently loading a savestate? */
bool isLoading();

/* Restore the state of loading a savestate, after memory has been rewritten */
void setLoading();

/* Print savestate error and display it on HUD */
void printError(int err);

/* Returns the signal number of checkpoint and thread suspend */
int sigCheckpoint();
int sigSuspend();

}
}

#endif

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

    Author : Philippe Virouleau <philippe.virouleau@imag.fr>
 */

#ifndef LIBTAS_THREAD_MANAGER_H
#define LIBTAS_THREAD_MANAGER_H
#include "logging.h"
#include "TimeHolder.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <cstddef>
#include <pthread.h>


class ThreadManager {
    //Private constructor
    ThreadManager();
    std::map<pthread_t, std::vector<TimeHolder>> startTime_;
    std::map<pthread_t, std::vector<TimeHolder>> endTime_;
    //This will register every pthread created for each start_routine
    std::map<std::ptrdiff_t, std::set<pthread_t>> threadMap_;
    std::map<pthread_t, std::ptrdiff_t> currentAssociation_;

    std::set<std::ptrdiff_t> refTable_;
    std::set<void *> beforeSDL_;
    bool init_ = false;
    pthread_t main_ = 0;

    static ThreadManager instance_;
public:
    static ThreadManager &get() { return instance_; };

    // Called from SDL_init, assumed to be main thread
    void init(pthread_t tid);
    // Register the start of a new thread
    // It uses the ptrdiff between the start_routine and the calling function
    // ('from'), which is assumed to be a possible invariant
    void start(pthread_t tid, void *from, void *start_routine);
    // Register the end of a thread
    void end(pthread_t tid);
    // Should we wait for this entry point ?
    bool waitFor(pthread_t tid);

    // Attempt to suspend main thread
    void suspend(pthread_t from_tid);
    // Resumes main thread
    void resume();

    pthread_t main() { return main_; };
    // Display a summary of the current execution (which pthread_t for which entry point)
    std::string summary();

    // Sig handler for suspending/resuming thread
    static void sigspin(int sig);
    static std::atomic<int> spin;

};

#endif

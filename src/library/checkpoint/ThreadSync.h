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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
 */

#ifndef LIBTAS_THREAD_SYNC_H
#define LIBTAS_THREAD_SYNC_H

namespace libtas {
namespace ThreadSync {
    void acquireLocks();
    void releaseLocks();
    void waitForThreadsToFinishInitialization();
    void incrementUninitializedThreadCount();
    void decrementUninitializedThreadCount();
    void wrapperExecutionLockLock();
    void wrapperExecutionLockUnlock();
    void detInit();
    void detWait();
    void detSignal(bool stop);

}
}

#endif

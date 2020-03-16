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

#include "BusyLoopDetection.h"
// #include <sstream>
#include "logging.h"
#include "frame.h"
#include "backtrace.h"
#include "DeterministicTimer.h"
//#include "../shared/SharedConfig.h"
// #include "global.h" // game_info
// #include <math.h>
#include <execinfo.h>
#include <map>


namespace libtas {

static std::map<uintptr_t, int>& getTimecallCount() {
    static std::map<uintptr_t, int> timecall_count;
    return timecall_count;
}

void BusyLoopDetection::reset()
{
    if (!shared_config.busyloop_detection)
        return;

    /* Remove any fake ticks cause by the busy loop detector */
    detTimer.fakeAdvanceTimer({0, 0});

    getTimecallCount().clear();
}

void BusyLoopDetection::increment(void* ret_address)
{
    if (!shared_config.busyloop_detection)
        return;
    if (!ThreadManager::isMainThread())
        return;
    if (GlobalState::isNative())
        return;

//    printBacktrace();

    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "Time function called from %p", ret_address);

    uintptr_t intret = reinterpret_cast<uintptr_t>(ret_address);

    auto& timecall_count = getTimecallCount();
    auto it = timecall_count.find(intret);
    if (it != timecall_count.end()) {
        it->second = it->second + 1;
        if (it->second == 1000) {
            debuglogstdio(LCF_TIMESET, "Busy loop detected, fake advance ticks to next frame");
            detTimer.fakeAdvanceTimerFrame();
        }
        if (it->second > 1100) {
            if (!detTimer.insideFrameBoundary) {
                debuglogstdio(LCF_TIMESET, "Still softlocking, advance one frame");
#ifdef LIBTAS_ENABLE_HUD
                static RenderHUD dummy;
                frameBoundary(false, [] () {}, dummy);
#else
                frameBoundary(false, [] () {});
#endif
            }
        }
    }
    else {
        timecall_count[intret] = 1;
    }
}

}

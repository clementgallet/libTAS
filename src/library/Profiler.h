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

#ifndef LIBTAS_PROFILER_H_INCL
#define LIBTAS_PROFILER_H_INCL

#include "TimeHolder.h"
// #include "../shared/lcf.h"

#include <string>
#include <vector>

namespace libtas {

namespace Profiler {

enum {
    PROFILER_INFO_RENDERING,
    PROFILER_INFO_FRAME,
    PROFILER_INFO_UNITY,
};

struct ScopeInfo
{
    std::string label;
    std::string description; // to show in the tooltip
    int type; // for display

    TimeHolder startTime;
    TimeHolder lengthTime;

    int nodeId;
    int parentNodeId;
    unsigned int depth;
};

struct Database
{
    static const int maxNodes = 10000;

    ScopeInfo nodes[maxNodes] {};
    std::vector<std::vector<int>> nodesByDepth;
    TimeHolder minTime;

    int nextNodeId    = 0;
    int maxNodeId     = 0;
    int currentNodeId = -1;
    unsigned int currentDepth  = 0;
    bool dirty        = true;

    static Database& get();
    
    ScopeInfo& initNode(const char* label, int type, const char* desc = nullptr);
    
    std::vector<std::vector<int>>& populateNodes(TimeHolder& minTime);
};

#define PROFILE_SCOPE(label, type) \
    auto& scopeInfo = Profiler::Database::get().initNode((label), (Profiler::type)); \
    const Profiler::ScopeGuard scopeGuard(scopeInfo)

#define PROFILE_SCOPE_DESC(label, type, desc) \
    auto& scopeInfo = Profiler::Database::get().initNode((label), (Profiler::type), (desc)); \
    const Profiler::ScopeGuard scopeGuard(scopeInfo)

struct ScopeGuard
{
    ScopeInfo& scopeInfo;
    int previousNodeId;

    explicit ScopeGuard(ScopeInfo& info);
    ~ScopeGuard();
};

enum Pause {
    PAUSE_ON_IDLE = 0x1, // skip from profiler timing when we idle for user interaction
    PAUSE_ON_SLEEP = 0x2, // skip from profiler timing when we sleep to match the game fps
};

void setPauseFlags(int flags);
int getPauseFlags();

#define PROFILE_PAUSE(flag) \
Profiler::PauseGuard pg(Profiler::flag)

TimeHolder currentTimeWithoutPause();

struct PauseGuard
{
    explicit PauseGuard(int flags);
    ~PauseGuard();
};

void newFrame();
const std::vector<TimeHolder>& getFrameTimings();

}

}

#endif

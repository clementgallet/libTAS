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

struct ScopeInfo
{
    std::string label;

    TimeHolder startTime;
    TimeHolder lengthTime;

    int nodeId;
    int parentNodeId;
    int depth;
};

struct Database
{
    static const int maxNodes = 10000;

    ScopeInfo nodes[maxNodes] {};

    int nextNodeId    = 0;
    int maxNodeId     = 0;
    int currentNodeId = -1;
    int currentDepth  = 0;

    static Database& get();
    
    ScopeInfo& initNode(const char* label);
    
    bool populateNodes(std::vector<std::vector<int>>& nodesByDepth);
};

#define PROFILE_SCOPE(label) \
    auto& scopeInfo = Profiler::Database::get().initNode((label)); \
    const Profiler::ScopeGuard scopeGuard(scopeInfo)

struct ScopeGuard
{
    ScopeInfo& scopeInfo;
    int previousNodeId;

    explicit ScopeGuard(ScopeInfo& info);
    ~ScopeGuard();
};

void startPause();
void stopPause();

TimeHolder currentTimeWithoutPause();

struct PauseGuard
{
    explicit PauseGuard();
    ~PauseGuard();
};

}

}

#endif

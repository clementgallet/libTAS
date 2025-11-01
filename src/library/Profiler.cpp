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

#include "Profiler.h"
#include "logging.h"
#include "GlobalState.h"
#include "TimeHolder.h"

// #include "checkpoint/ThreadManager.h"
// 
// #include <time.h>

namespace libtas {

Profiler::Database& Profiler::Database::get()
{
    static thread_local Profiler::Database tlDatabase;
    return tlDatabase;
}

Profiler::ScopeInfo& Profiler::Database::initNode(const char* label)
{
    int id = nextNodeId;

    nodes[id] = ScopeInfo{
        .label        = label,
        .nodeId       = id,
        .parentNodeId = -1,
        .depth        = currentDepth,
    };

    nextNodeId = (nextNodeId+1) % maxNodes;
    if (nextNodeId > maxNodeId)
        maxNodeId = nextNodeId;

    return nodes[id];
}

Profiler::ScopeGuard::ScopeGuard(ScopeInfo& info) :
    scopeInfo{info},
    previousNodeId{Profiler::Database::get().currentNodeId}
{
    scopeInfo.startTime = Profiler::currentTimeWithoutPause();
    scopeInfo.lengthTime = nullTime;

    auto& db = Profiler::Database::get();

    scopeInfo.parentNodeId = db.currentNodeId;
    db.currentNodeId       = scopeInfo.nodeId;
    db.currentDepth        = scopeInfo.depth + 1;
}

Profiler::ScopeGuard::~ScopeGuard()
{
    auto& db = Profiler::Database::get();

    scopeInfo.lengthTime = Profiler::currentTimeWithoutPause() - scopeInfo.startTime;

    db.currentNodeId = previousNodeId;
    db.currentDepth  = scopeInfo.depth;
}

bool Profiler::Database::populateNodes(std::vector<std::vector<int>>& nodesByDepth)
{
    bool hasChildren = false;
    
    for (auto& vec : nodesByDepth)
        vec.clear();

    for (int n = 0; n < maxNodeId; n++) {
        const ScopeInfo& info = nodes[n];
        
        // if (info.startTime == nullTime)
        //     continue;

        if (info.depth >= nodesByDepth.size())
            nodesByDepth.resize(info.depth+1);

        if (info.depth > 0)
            hasChildren = true;

        // LOG(LL_DEBUG, LCF_WINDOW, "Push Profiler node %d", info.nodeId);

        nodesByDepth[info.depth].push_back(info.nodeId);
    }
    return hasChildren;
}

static TimeHolder pauseStartTime = nullTime;
static TimeHolder pauseElapsedTime = {0, 0};

void Profiler::startPause()
{
    pauseStartTime = TimeHolder::now();
}

void Profiler::stopPause()
{
    pauseElapsedTime += TimeHolder::now() - pauseStartTime;
    pauseStartTime = nullTime;
}

TimeHolder Profiler::currentTimeWithoutPause()
{
    if (pauseStartTime != nullTime)
        return pauseStartTime - pauseElapsedTime;

    return TimeHolder::now() - pauseElapsedTime;
}

Profiler::PauseGuard::PauseGuard()
{
    Profiler::startPause();
}

Profiler::PauseGuard::~PauseGuard()
{
    Profiler::stopPause();
}

}

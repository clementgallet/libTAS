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

#include <vector>
#include <limits>

namespace libtas {

Profiler::Database& Profiler::Database::get()
{
    static thread_local Profiler::Database* tlDatabase = new Profiler::Database;
    return *tlDatabase;
}

Profiler::ScopeInfo& Profiler::Database::initNode(const char* label, int type, const char* desc)
{
    int id = nextNodeId;

    nodes[id] = ScopeInfo{
        .label        = label,
        .description  = desc?desc:"",
        .type         = type,
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
    db.dirty = true;
}

Profiler::ScopeGuard::~ScopeGuard()
{
    auto& db = Profiler::Database::get();

    scopeInfo.lengthTime = Profiler::currentTimeWithoutPause() - scopeInfo.startTime;

    db.currentNodeId = previousNodeId;
    db.currentDepth  = scopeInfo.depth;
}

std::vector<std::vector<int>>& Profiler::Database::populateNodes(TimeHolder& combinedMinTime)
{
    if (dirty) {
        minTime = {std::numeric_limits<time_t>::max(),  std::numeric_limits<long>::max()};
        
        for (auto& vec : nodesByDepth)
        vec.clear();
        
        for (int n = 0; n < maxNodeId; n++) {
            const ScopeInfo& info = nodes[n];
            
            if (info.depth >= nodesByDepth.size())
                nodesByDepth.resize(info.depth+1);
            
            if (info.startTime < minTime)
                minTime = info.startTime;
            nodesByDepth[info.depth].push_back(info.nodeId);
        }
    }
    
    if (minTime < combinedMinTime)
        combinedMinTime = minTime;

    dirty = false;

    return nodesByDepth;
}

static TimeHolder pauseStartTime = nullTime;
static TimeHolder pauseElapsedTime = {0, 0};
static int pauseFlags = Profiler::PAUSE_ON_IDLE | Profiler::PAUSE_ON_SLEEP;

void Profiler::setPauseFlags(int flags)
{
    pauseFlags = flags;
}

int Profiler::getPauseFlags()
{
    return pauseFlags;
}

TimeHolder Profiler::currentTimeWithoutPause()
{
    if (pauseStartTime != nullTime)
        return pauseStartTime - pauseElapsedTime;

    return TimeHolder::now() - pauseElapsedTime;
}

Profiler::PauseGuard::PauseGuard(int flag)
{
    if (getPauseFlags() & flag)
        pauseStartTime = TimeHolder::now();
}

Profiler::PauseGuard::~PauseGuard()
{
    if (pauseStartTime != nullTime)
        pauseElapsedTime += TimeHolder::now() - pauseStartTime;
    pauseStartTime = nullTime;
}

static std::vector<TimeHolder> frameTimings;

void Profiler::newFrame()
{
    TimeHolder newTime = currentTimeWithoutPause();
    frameTimings.push_back(newTime);
}

const std::vector<TimeHolder>& Profiler::getFrameTimings()
{
    return frameTimings;
}

}

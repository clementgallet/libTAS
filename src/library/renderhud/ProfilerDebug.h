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

#ifndef LIBTAS_IMGUI_PROFILERDEBUG_H_INCL
#define LIBTAS_IMGUI_PROFILERDEBUG_H_INCL

#include "TimeHolder.h"
#include "Profiler.h"

#include <cstdint>

namespace libtas {

namespace ProfilerDebug
{
    void nodeToPos(const TimeHolder& startTime, const TimeHolder& lengthTime, float available_size, float& lengthTimeMs, float& leftPos, float& lengthPos);
    
    void renderFrame(int f, const TimeHolder& frame_start, const TimeHolder& frame_end, float available_start, float available_size);
    void renderNode(int nodeId, const Profiler::Database* database, float available_start, float available_size);

    void draw(uint64_t framecount, bool* p_open);
}

}

#endif

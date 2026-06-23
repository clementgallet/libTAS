/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

/**
 * @namespace ProfilerDebug
 * @brief Helper namespace for rendering profiling data in the HUD.
 *
 * Provides functions for converting profiler timestamps into screen
 * positions and drawing frame and node timing overlays.
 */
namespace ProfilerDebug
{
    /**
     * @brief Converts profiler timing values into on-screen positions.
     *
     * @param[in] startTime Start time of the profiler node
     * @param[in] lengthTime Duration of the profiler node
     * @param[in] available_size Available width in the HUD graph
     * @param[out] lengthTimeMs Duration in milliseconds
     * @param[out] leftPos Left position in the HUD graph
     * @param[out] lengthPos Width position in the HUD graph
     */
    void nodeToPos(const TimeHolder& startTime, const TimeHolder& lengthTime, float available_size, float& lengthTimeMs, float& leftPos, float& lengthPos);
    
    /**
     * @brief Renders a frame timing bar in the profiler overlay.
     *
     * @param[in] f Frame index
     * @param[in] frame_start Start time of the frame
     * @param[in] frame_end End time of the frame
     * @param[in] available_start Start position for rendering
     * @param[in] available_size Available width for rendering
     */
    void renderFrame(int f, const TimeHolder& frame_start, const TimeHolder& frame_end, float available_start, float available_size);

    /**
     * @brief Renders a profiler node entry in the HUD profiler view.
     *
     * @param[in] nodeId Node identifier
     * @param[in] database Profiler database containing node timing data
     * @param[in] available_start Start position for rendering
     * @param[in] available_size Available width for rendering
     * @param[in] threadId Thread identifier
     */
    void renderNode(int nodeId, const Profiler::Database* database, float available_start, float available_size, pid_t threadId);

    /**
     * @brief Draws the profiler debug window.
     *
     * @param[in] framecount Current frame index
     * @param[in,out] p_open Controls whether the profiler window is visible
     */
    void draw(uint64_t framecount, bool* p_open);
}

}

#endif

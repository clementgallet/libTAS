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

#ifndef LIBTAS_IMGUI_UNITYDEBUG_H_INCL
#define LIBTAS_IMGUI_UNITYDEBUG_H_INCL

#include <sys/types.h>
#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace libtas {

/**
 * @namespace UnityDebug
 * @brief Helper namespace for Unity-specific HUD debugging visualizations.
 */
namespace UnityDebug
{
    // From ImPlot
    struct ScrollingBuffer {
        size_t MaxSize;
        int Offset;
        std::vector<float> DataX;
        std::vector<float> DataY;
        std::string name;

        ScrollingBuffer(int max_size);
        void AddPoint(int x, int y);
        void Erase();
    };

    struct ScrollingBuffers {
        std::map<int, ScrollingBuffer> Buffers;
        
        ScrollingBuffers();
        void AddPoint(float x, float y, int tid);
    };

    /**
     * @brief Updates Unity debug state for the current frame.
     *
     * @param[in] framecount Current frame index
     */
    void update(uint64_t framecount);

    /**
     * @brief Updates preload progress for Unity debug display.
     *
     * @param[in] added_count Number of items added to preload
     * @param[in] processed_count Number of items processed so far
     */
    void update_preload(int added_count, int processed_count);

    /**
     * @brief Draws the Unity debug window.
     *
     * @param[in] framecount Current frame index
     * @param[in,out] p_open Controls whether the Unity debug window is visible
     */
    void draw(uint64_t framecount, bool* p_open);
}

}

#endif

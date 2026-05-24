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

#ifndef LIBTAS_IMGUI_FRAMEWINDOW_H_INCL
#define LIBTAS_IMGUI_FRAMEWINDOW_H_INCL

#include <string>
#include <cstdint>

namespace libtas {

/**
 * @namespace FrameWindow
 * @brief Helper namespace for displaying frame counts.
 */
namespace FrameWindow
{
    /**
     * @brief Sets an optional marker text shown in the frame window.
     *
     * @param[in] text Marker text to display
     */
    void setMarkerText(std::string text);

    /**
     * @brief Draws the framecount and non-draw count window.
     *
     * @param[in] framecount Rendered frame index
     * @param[in] nondraw_framecount Number of frames without drawing activity
     * @param[in,out] p_open Controls whether the frame window is visible
     */
    void draw(uint64_t framecount, uint64_t nondraw_framecount, bool* p_open);
}

}

#endif

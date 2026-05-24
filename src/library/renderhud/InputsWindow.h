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

#ifndef LIBTAS_IMGUI_INPUTSWINDOW_H_INCL
#define LIBTAS_IMGUI_INPUTSWINDOW_H_INCL

#include "../shared/inputs/AllInputsFlat.h"
#include <string>

namespace libtas {

/**
 * @namespace InputsWindow
 * @brief Helper namespace for rendering current and preview input state.
 */
namespace InputsWindow
{
    /**
     * @brief Draws the inputs debug window.
     *
     * @param[in] ai Current input state
     * @param[in] preview_ai Preview input state for HUD display
     * @param[in,out] p_open Controls whether the inputs window is visible
     */
    void draw(const AllInputsFlat& ai, const AllInputsFlat& preview_ai, bool* p_open);
    
    /**
     * @brief Formats input state for textual display.
     *
     * @param[in] ai Current input state
     * @return String representation of the inputs
     */
    std::string formatInputs(const AllInputsFlat& ai);

}

}

#endif

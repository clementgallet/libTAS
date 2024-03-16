/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef __unix__
#include "config.h"
#endif

#ifndef LIBTAS_RENDERHUD_H_INCL
#define LIBTAS_RENDERHUD_H_INCL

#include "../shared/inputs/AllInputsFlat.h"

#include <memory>
#include <list>
#include <utility>
#include <string>
#include <stdint.h>

namespace libtas {
/* This class handles the display of some text and shapes over the game screen (HUD).
 *
 * Because games have different methods of rendering, this class
 * should be derived for each rendering method. The subclass must
 * define the renderSurface() function
 *
 * Also, OS have different ways of creating a text surface from a string,
 * so this class must be derived for each OS to define the renderText() method.
 *
 * This class is also responsible on formatting and positioning the
 * different elements of the HUD.
 */
class RenderHUD
{
    public:
        /* Called at the beginning of a new frame */
        virtual void newFrame();

        /* Add all hud elements for rendering */
        void drawAll(uint64_t framecount, uint64_t nondraw_framecount, const AllInputsFlat& ai, const AllInputsFlat& preview_ai);
        
        /* Called at the end of a frame to render the hud */
        virtual void render() {}

        /* Called at the end of a frame if we decide to not render the hud */
        void endFrame();

        /* Called to notify that the current frame had user interaction, and
         * we must not idle */
        static void userInputs();

        /* Indicate if we need to render or idle */
        bool doRender();
        
        /* Does the backend supports rendering the game inside an ImGui window? */
        virtual bool supportsGameWindow() {return false;}

        /* Returns if the game is rendered inside an ImGui window */
        bool renderGameWindow();
        
        /* Where is the origin point? */
        virtual bool invertedOrigin() {return false;}
        

    protected:
        bool init();
        
    private:
        /* How many future draws we need before start idling */
        static int framesBeforeIdle;
        
        /* True if we render the game inside an ImGui window */
        bool show_game_window = false;
};
}

#endif

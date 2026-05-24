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

#ifdef __unix__
#include "config.h"
#endif

#ifndef LIBTAS_RENDERHUD_H_INCL
#define LIBTAS_RENDERHUD_H_INCL

#include "../shared/inputs/AllInputsFlat.h"
#include "../shared/inputs/MouseInputs.h"

#include <memory>
#include <list>
#include <utility>
#include <string>
#include <stdint.h>

namespace libtas {
/**
 * @class RenderHUD
 * @brief Core HUD subsystem interface for overlay rendering.
 *
 * RenderHUD manages the composition and display of overlay elements such as
 * text, shapes, and pointer scaling. The class is designed to support multiple
 * rendering backends through subclassing, allowing each rendering method to
 * implement its own presentation and text rendering mechanism.
 *
 * The subsystem is responsible for:
 * - managing HUD layout and update logic
 * - deciding when to render or idle
 * - supporting detached game windows and input scaling
 *
 * Concrete implementations provide backend-specific rendering behavior by
 * overriding virtual methods such as newFrame(), render(), and optional
 * viewport support queries.
 */
class RenderHUD
{
    public:
        /**
         * @brief Prepares the HUD state for a new frame.
         *
         * Called once at the beginning of each frame boundary before HUD elements
         * are drawn. Subclasses may override to reset per-frame state.
         */
        virtual void newFrame();

        /**
         * @brief Collects HUD elements for the current frame.
         *
         * Builds the list of HUD content to render from the current input state
         * and frame counters.
         *
         * @param[in] framecount Number of frames since startup
         * @param[in] nondraw_framecount Number of non-draw frames
         * @param[in] ai Current input state
         * @param[in] preview_ai Preview input state used for HUD display
         */
        void drawAll(uint64_t framecount, uint64_t nondraw_framecount, const AllInputsFlat& ai, const AllInputsFlat& preview_ai);
        
        /**
         * @brief Renders the HUD overlay.
         *
         * Called at the end of the frame after HUD elements have been collected.
         * Subclasses implement the actual rendering commands for their backend.
         */
        virtual void render() {}

        /**
         * @brief Finalizes HUD rendering for the current frame without actually drawing
         *
         * Called at the end of the frame to skip drawing the HUD. This is usually
         * done for non-draw frames, or when we want the HUD elements to guess their
         * size. Either this of render() must be called for each newFrame() call.
         * Resets internal flags and prepares for the next frame.
         */
        void endFrame();

        /**
         * @brief Notifies the HUD that user input occurred.
         *
         * This prevents the subsystem from entering idle mode immediately after
         * user interaction.
         */
        static void userInputs();

        /**
         * @brief Returns whether HUD content should be rendered this frame.
         *
         * The result depends on whether the system is idle or active.
         *
         * @return true if HUD rendering is required, false if idle mode is active
         */
        bool doRender();
        
        /**
         * @brief Indicates whether the backend supports an ImGui game window.
         *
         * Backends that can render the game inside an ImGui panel should return
         * true. Default implementation returns false.
         *
         * @return true if game window rendering is supported
         */
        virtual bool supportsGameWindow() {return false;}

        /**
         * @brief Indicates whether the backend supports a larger detached viewport.
         *
         * Some backends can use extra window space when the game view is detached.
         * Default implementation returns false.
         *
         * @return true if larger viewport support is available
         */
        virtual bool supportsLargerViewport() {return false;}

        /**
         * @brief Sets whether the surrounding window is resizable.
         *
         * @param[in] resizable true if the window should be resizable
         */
        void setWindowResizable(bool resizable);

        /**
         * @brief Returns whether the game is currently rendered inside an ImGui window.
         *
         * @return true when the game is embedded in an ImGui window
         */
        bool renderGameWindow();
        
        /**
         * @brief Indicates whether the HUD origin is inverted.
         *
         * Some rendering backends use a flipped coordinate origin. Default
         * implementation returns false.
         *
         * @return true if the origin is inverted vertically
         */
        virtual bool invertedOrigin() {return false;}
        
        /**
         * @brief Adjusts mouse input coordinates for detached window scaling.
         *
         * Applies the current detached game window offset and scale to pointer
         * coordinates so input remains consistent when the game window is moved
         * or resized.
         *
         * @param[in,out] mi Mouse input structure to scale
         */
        void scaleMouseInputs(MouseInputs* mi);

    protected:
        bool init();
        
    private:
        /* How many future draws we need before start idling */
        static int framesBeforeIdle;
        
        /* True if we render the game inside an ImGui window */
        static bool show_game_window;
        
        /* Coordinates of the game window when detached, to be able to scale
         * the pointer coordinates accordingly */
        float game_window_x;
        float game_window_y;
        float game_window_scale;
};
}

#endif

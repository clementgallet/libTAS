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

#ifndef LIBTAS_RENDERHUD_SDL2_RENDERER_H_INCL
#define LIBTAS_RENDERHUD_SDL2_RENDERER_H_INCL

#ifdef __unix__
#include "RenderHUD_Base_Linux.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "RenderHUD_Base_MacOS.h"
#endif

struct SDL_Renderer;

namespace libtas {
#ifdef __unix__
class RenderHUD_SDL2_renderer : public RenderHUD_Base_Linux
#elif defined(__APPLE__) && defined(__MACH__)
class RenderHUD_SDL2_renderer : public RenderHUD_Base_MacOS
#endif
{
    public:
        /**
         * @class RenderHUD_SDL2_renderer
         * @brief SDL2 renderer-based HUD backend.
         *
         * Implements HUD rendering using the SDL2 rendering API.
         */
        ~RenderHUD_SDL2_renderer();

        /**
         * @brief Sets the SDL2 renderer used by the HUD backend.
         *
         * @param[in] r SDL_Renderer pointer to use for rendering
         */
        void setRenderer(SDL_Renderer* r);

        void newFrame();

        void render();

        bool supportsGameWindow() {return true;}

        bool supportsLargerViewport() {return false;}

    private:
        SDL_Renderer* renderer = nullptr;

};
}

#endif

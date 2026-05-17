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

#ifndef LIBTAS_SDLRENDER_H_INCL
#define LIBTAS_SDLRENDER_H_INCL

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

namespace libtas {

/**
 *  \brief Create a 2D rendering context for a window.
 *
 *  \param window The window where rendering is displayed.
 *  \param index    The index of the rendering driver to initialize, or -1 to
 *                  initialize the first one supporting the requested flags.
 *  \param flags    ::SDL_RendererFlags.
 *
 *  \return A valid rendering context or NULL if there was an error.
 *
 *  \sa SDL_CreateSoftwareRenderer()
 *  \sa SDL_GetRendererInfo()
 *  \sa SDL_DestroyRenderer()
 */

namespace sdl2 {
    SDL_Renderer *SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags);
};

namespace sdl3 {
    SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, const char *name);
};

/**
 *  \brief Destroy the rendering context for a window and free associated
 *         textures.
 *
 *  \sa SDL_CreateRenderer()
 */
OVERRIDE void SDL_DestroyRenderer(SDL_Renderer * renderer);


/**
 *  \brief Update the screen with rendering performed.
 */
OVERRIDE void SDL_RenderPresent(SDL_Renderer * renderer);

/**
 *  \brief Set device independent resolution for rendering
 *
 *  \param renderer The renderer for which resolution should be set.
 *  \param w      The width of the logical resolution
 *  \param h      The height of the logical resolution
 *
 *  This function uses the viewport and scaling functionality to allow a fixed logical
 *  resolution for rendering, regardless of the actual output resolution.  If the actual
 *  output resolution doesn't have the same aspect ratio the output rendering will be
 *  centered within the output display.
 *
 *  If the output display is a window, mouse events in the window will be filtered
 *  and scaled so they seem to arrive within the logical resolution.
 *
 *  \note If this function results in scaling or subpixel drawing by the
 *        rendering backend, it will be handled using the appropriate
 *        quality hints.
 *
 *  \sa SDL_RenderGetLogicalSize()
 *  \sa SDL_RenderSetScale()
 *  \sa SDL_RenderSetViewport()
 */
OVERRIDE int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h);

/**
 * Set a device-independent resolution and presentation mode for rendering.
 *
 * This function sets the width and height of the logical rendering output.
 * The renderer will act as if the current render target is always the
 * requested dimensions, scaling to the actual resolution as necessary.
 *
 * This can be useful for games that expect a fixed size, but would like to
 * scale the output to whatever is available, regardless of how a user resizes
 * a window, or if the display is high DPI.
 *
 * Logical presentation can be used with both render target textures and the
 * renderer's window; the state is unique to each render target, and this
 * function sets the state for the current render target. It might be useful
 * to draw to a texture that matches the window dimensions with logical
 * presentation enabled, and then draw that texture across the entire window
 * with logical presentation disabled. Be careful not to render both with
 * logical presentation enabled, however, as this could produce
 * double-letterboxing, etc.
 *
 * You can disable logical coordinates by setting the mode to
 * SDL_LOGICAL_PRESENTATION_DISABLED, and in that case you get the full pixel
 * resolution of the render target; it is safe to toggle logical presentation
 * during the rendering of a frame: perhaps most of the rendering is done to
 * specific dimensions but to make fonts look sharp, the app turns off logical
 * presentation while drawing text, for example.
 *
 * You can convert coordinates in an event into rendering coordinates using
 * SDL_ConvertEventToRenderCoordinates().
 *
 * \param renderer the rendering context.
 * \param w the width of the logical resolution.
 * \param h the height of the logical resolution.
 * \param mode the presentation mode used.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_ConvertEventToRenderCoordinates
 * \sa SDL_GetRenderLogicalPresentation
 * \sa SDL_GetRenderLogicalPresentationRect
 */
OVERRIDE bool SDL_SetRenderLogicalPresentation(SDL_Renderer *renderer, int w, int h, sdl3::SDL_RendererLogicalPresentation mode);

/**
 *  \brief Get device independent resolution for rendering
 *
 *  \param renderer The renderer from which resolution should be queried.
 *  \param w      A pointer filled with the width of the logical resolution
 *  \param h      A pointer filled with the height of the logical resolution
 *
 *  \sa SDL_RenderSetLogicalSize()
 */
OVERRIDE void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h);

/**
 * Get device independent resolution and presentation mode for rendering.
 *
 * This function gets the width and height of the logical rendering output, or
 * 0 if a logical resolution is not enabled.
 *
 * Each render target has its own logical presentation state. This function
 * gets the state for the current render target.
 *
 * \param renderer the rendering context.
 * \param w an int filled with the logical presentation width.
 * \param h an int filled with the logical presentation height.
 * \param mode a variable filled with the logical presentation mode being
 *             used.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetRenderLogicalPresentation
 */
OVERRIDE bool SDL_GetRenderLogicalPresentation(SDL_Renderer *renderer, int *w, int *h, sdl3::SDL_RendererLogicalPresentation *mode);

/**
 *  \brief Set the drawing area for rendering on the current target.
 *
 *  \param renderer The renderer for which the drawing area should be set.
 *  \param rect The rectangle representing the drawing area, or NULL to set the viewport to the entire target.
 *
 *  The x,y of the viewport rect represents the origin for rendering.
 *
 *  \return 0 on success, or -1 on error
 *
 *  \note If the window associated with the renderer is resized, the viewport is automatically reset.
 *
 *  \sa SDL_RenderGetViewport()
 *  \sa SDL_RenderSetLogicalSize()
 */
OVERRIDE int SDL_RenderSetViewport(SDL_Renderer * renderer,
                                                  const sdl2::SDL_Rect * rect);

/**
 *  \brief Get the drawing area for the current target.
 *
 *  \sa SDL_RenderSetViewport()
 */
OVERRIDE void SDL_RenderGetViewport(SDL_Renderer * renderer,
                                                   sdl2::SDL_Rect * rect);

/**
 *  \brief Set the drawing scale for rendering on the current target.
 *
 *  \param renderer The renderer for which the drawing scale should be set.
 *  \param scaleX The horizontal scaling factor
 *  \param scaleY The vertical scaling factor
 *
 *  The drawing coordinates are scaled by the x/y scaling factors
 *  before they are used by the renderer.  This allows resolution
 *  independent drawing with a single coordinate system.
 *
 *  \note If this results in scaling or subpixel drawing by the
 *        rendering backend, it will be handled using the appropriate
 *        quality hints.  For best results use integer scaling factors.
 *
 *  \sa SDL_RenderGetScale()
 *  \sa SDL_RenderSetLogicalSize()
 */
OVERRIDE int SDL_RenderSetScale(SDL_Renderer * renderer,
                                               float scaleX, float scaleY);

/**
 *  \brief Get the drawing scale for the current target.
 *
 *  \param renderer The renderer from which drawing scale should be queried.
 *  \param scaleX A pointer filled in with the horizontal scaling factor
 *  \param scaleY A pointer filled in with the vertical scaling factor
 *
 *  \sa SDL_RenderSetScale()
 */
OVERRIDE void SDL_RenderGetScale(SDL_Renderer * renderer,
                                               float *scaleX, float *scaleY);

}

#endif

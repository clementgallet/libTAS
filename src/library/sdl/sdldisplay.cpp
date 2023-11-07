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

#include "sdldisplay.h"

#include "hook.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GetNumVideoDisplays)
DECLARE_ORIG_POINTER(SDL_GetDisplayName)
DECLARE_ORIG_POINTER(SDL_GetDisplayBounds)
DECLARE_ORIG_POINTER(SDL_GetDisplayDPI)
DECLARE_ORIG_POINTER(SDL_GetDisplayUsableBounds)
DECLARE_ORIG_POINTER(SDL_GetNumDisplayModes)
DECLARE_ORIG_POINTER(SDL_GetDisplayMode)
DECLARE_ORIG_POINTER(SDL_GetDesktopDisplayMode)
DECLARE_ORIG_POINTER(SDL_GetCurrentDisplayMode)
DECLARE_ORIG_POINTER(SDL_GetClosestDisplayMode)
DECLARE_ORIG_POINTER(SDL_GetWindowDisplayIndex)
DECLARE_ORIG_POINTER(SDL_SetWindowDisplayMode)
DECLARE_ORIG_POINTER(SDL_GetWindowDisplayMode)

/* Override */ int SDL_GetNumVideoDisplays(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GetNumVideoDisplays);

    int ret = orig::SDL_GetNumVideoDisplays();
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ const char *SDL_GetDisplayName(int displayIndex)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);
    LINK_NAMESPACE_SDL2(SDL_GetDisplayName);

    const char* ret = orig::SDL_GetDisplayName(displayIndex);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ int SDL_GetDisplayBounds(int displayIndex, SDL_Rect * rect)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetDisplayBounds);
        ret = orig::SDL_GetDisplayBounds(displayIndex, rect);

    }
    else {
        rect->x = displayIndex*Global::shared_config.screen_width;
        rect->y = 0;
        rect->w = Global::shared_config.screen_width;
        rect->h = Global::shared_config.screen_height;
    }

    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns rect (%d,%d,%d,%d)", rect->x, rect->y, rect->w, rect->h);

    return ret;
}

/* Override */ int SDL_GetDisplayDPI(int displayIndex, float * ddpi, float * hdpi, float * vdpi)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);
    LINK_NAMESPACE_SDL2(SDL_GetDisplayDPI);

    int ret = orig::SDL_GetDisplayDPI(displayIndex, ddpi, hdpi, vdpi);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns ddpi=%f, hdpi=%f, vdpi=", ddpi?*ddpi:0.0f, hdpi?*hdpi:0.0f, vdpi?*vdpi:0.0f);

    return ret;
}

/* Override */ int SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect * rect)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);
    LINK_NAMESPACE_SDL2(SDL_GetDisplayUsableBounds);

    int ret = orig::SDL_GetDisplayUsableBounds(displayIndex, rect);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns rect (%d,%d,%d,%d)", rect->x, rect->y, rect->w, rect->h);

    return ret;
}

/* Override */ int SDL_GetNumDisplayModes(int displayIndex)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 1;

    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetNumDisplayModes);
        ret = orig::SDL_GetNumDisplayModes(displayIndex);
    }
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ int SDL_GetDisplayMode(int displayIndex, int modeIndex, SDL_DisplayMode * mode)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d and mode %d", __func__, displayIndex, modeIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetDisplayMode);
        ret = orig::SDL_GetDisplayMode(displayIndex, modeIndex, mode);
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        LINK_NAMESPACE_SDL2(SDL_GetDesktopDisplayMode);
        ret = orig::SDL_GetDesktopDisplayMode(displayIndex, mode);

        mode->format = SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }
    mode->refresh_rate = Global::shared_config.framerate_num / Global::shared_config.framerate_den;

    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);
    return ret;
}

/* Override */ int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);
    LINK_NAMESPACE_SDL2(SDL_GetDesktopDisplayMode);

    int ret = orig::SDL_GetDesktopDisplayMode(displayIndex, mode);

    if (!GlobalState::isNative() && Global::shared_config.screen_width) {
        mode->format = SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }
    mode->refresh_rate = Global::shared_config.framerate_num / Global::shared_config.framerate_den;

    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);
    return ret;
}

/* Override */ int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetCurrentDisplayMode);
        ret = orig::SDL_GetCurrentDisplayMode(displayIndex, mode);
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        LINK_NAMESPACE_SDL2(SDL_GetDesktopDisplayMode);
        ret = orig::SDL_GetDesktopDisplayMode(displayIndex, mode);

        mode->format = SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }
    mode->refresh_rate = Global::shared_config.framerate_num / Global::shared_config.framerate_den;

    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);
    return ret;
}

/* Override */ SDL_DisplayMode *SDL_GetClosestDisplayMode(int displayIndex, const SDL_DisplayMode * mode, SDL_DisplayMode * closest)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   and mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    SDL_DisplayMode *dm = nullptr;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetClosestDisplayMode);
        dm = orig::SDL_GetClosestDisplayMode(displayIndex, mode, closest);
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        LINK_NAMESPACE_SDL2(SDL_GetDesktopDisplayMode);
        int ret = orig::SDL_GetDesktopDisplayMode(displayIndex, closest);

        if (ret == 0) {
            closest->format = SDL_PIXELFORMAT_RGB888;
            closest->w = Global::shared_config.screen_width;
            closest->h = Global::shared_config.screen_height;

            dm = closest;
        }
    }
    dm->refresh_rate = Global::shared_config.framerate_num / Global::shared_config.framerate_den;

    return dm;
}

/* Override */ int SDL_GetWindowDisplayIndex(SDL_Window * window)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with window %d", __func__, (void*)window);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        LINK_NAMESPACE_SDL2(SDL_GetWindowDisplayIndex);
        ret = orig::SDL_GetWindowDisplayIndex(window);
    }
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns index %d", ret);

    return ret;
}

/* Override */ int SDL_SetWindowDisplayMode(SDL_Window * window, const SDL_DisplayMode* mode)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with window %d", __func__, (void*)window);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   and mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);
    LINK_NAMESPACE_SDL2(SDL_SetWindowDisplayMode);

    int ret = orig::SDL_SetWindowDisplayMode(window, mode);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns ret %d", ret);

    return ret;
}

/* Override */ int SDL_GetWindowDisplayMode(SDL_Window * window, SDL_DisplayMode * mode)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with window %d", __func__, (void*)window);
    LINK_NAMESPACE_SDL2(SDL_GetWindowDisplayMode);

    int ret = orig::SDL_GetWindowDisplayMode(window, mode);

    debuglogstdio(LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    return ret;
}

}

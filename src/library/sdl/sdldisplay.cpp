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

#include "sdldisplay.h"
#include "sdlversion.h"

#include "hook.h"
#include "sdldynapi.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

/* Override */ int SDL_GetNumVideoDisplays(void)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW);

    int ret = ORIG_SDL2_CALL(SDL_GetNumVideoDisplays, ());
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ const char *SDL_GetDisplayName(int displayIndex)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    const char* ret = ORIG_SDL23_CALL(SDL_GetDisplayName, (displayIndex));
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ int SDL_GetDisplayBounds(int displayIndex, sdl2::SDL_Rect * rect)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        ret = ORIG_SDL23_CALL(SDL_GetDisplayBounds, (displayIndex, rect));
    }
    else {
        if (!rect) {
            return -1;
        }
        rect->x = displayIndex*Global::shared_config.screen_width;
        rect->y = 0;
        rect->w = Global::shared_config.screen_width;
        rect->h = Global::shared_config.screen_height;
    }

    if (rect) {
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns rect (%d,%d,%d,%d)", rect->x, rect->y, rect->w, rect->h);
    }
    else {
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns null rect with status %d", ret);
    }

    return ret;
}

/* Override */ int SDL_GetDisplayDPI(int displayIndex, float * ddpi, float * hdpi, float * vdpi)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = ORIG_SDL2_CALL(SDL_GetDisplayDPI, (displayIndex, ddpi, hdpi, vdpi));
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns ddpi=%f, hdpi=%f, vdpi=%f", ddpi?*ddpi:0.0f, hdpi?*hdpi:0.0f, vdpi?*vdpi:0.0f);

    return ret;
}

/* Override */ int SDL_GetDisplayUsableBounds(int displayIndex, sdl2::SDL_Rect * rect)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = ORIG_SDL23_CALL(SDL_GetDisplayUsableBounds, (displayIndex, rect));
    if (rect) {
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns rect (%d,%d,%d,%d)", rect->x, rect->y, rect->w, rect->h);
    }
    else {
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns null rect with status %d", ret);
    }

    return ret;
}

/* Override */ int SDL_GetNumDisplayModes(int displayIndex)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 1;

    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        ret = ORIG_SDL2_CALL(SDL_GetNumDisplayModes, (displayIndex));
    }
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns %d", ret);

    return ret;
}

/* Override */ int SDL_GetDisplayMode(int displayIndex, int modeIndex, sdl2::SDL_DisplayMode * mode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d and mode %d", __func__, displayIndex, modeIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        ret = ORIG_SDL2_CALL(SDL_GetDisplayMode, (displayIndex, modeIndex, mode));
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        ret = ORIG_SDL2_CALL(SDL_GetDesktopDisplayMode, (displayIndex, mode));

        mode->format = sdl2::SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }
    mode->refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;

    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);
    return ret;
}

/* Override */ int sdl2::SDL_GetDesktopDisplayMode(int displayIndex, sdl2::SDL_DisplayMode * mode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = ORIG_SDL2_CALL(SDL_GetDesktopDisplayMode, (displayIndex, mode));

    if (!GlobalState::isNative() && Global::shared_config.screen_width) {
        mode->format = sdl2::SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }
    mode->refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;

    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %p", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    return ret;
}

/* Override */ const sdl3::SDL_DisplayMode * sdl3::SDL_GetDesktopDisplayMode(SDL_DisplayID displayID)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayID);

    const sdl3::SDL_DisplayMode* mode = ORIG_SDL3_CALL(SDL_GetDesktopDisplayMode, (displayID));

    if (!mode)
        return mode;

    static sdl3::SDL_DisplayMode new_mode = *mode;

    if (!GlobalState::isNative() && Global::shared_config.screen_width) {
        new_mode.format = sdl3::SDL_PIXELFORMAT_RGB24;
        new_mode.w = Global::shared_config.screen_width;
        new_mode.h = Global::shared_config.screen_height;
    }
    new_mode.refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;
    new_mode.refresh_rate_numerator = Global::shared_config.initial_framerate_num;
    new_mode.refresh_rate_denominator = Global::shared_config.initial_framerate_den;

    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %p", new_mode.format, new_mode.w, new_mode.h, new_mode.refresh_rate, new_mode.internal);

    return &new_mode;
}

/* Override */ int sdl2::SDL_GetCurrentDisplayMode(int displayIndex, sdl2::SDL_DisplayMode * mode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayIndex);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        ret = ORIG_SDL2_CALL(SDL_GetCurrentDisplayMode, (displayIndex, mode));
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        ret = ORIG_SDL2_CALL(SDL_GetDesktopDisplayMode, (displayIndex, mode));

        mode->format = sdl2::SDL_PIXELFORMAT_RGB888;
        mode->w = Global::shared_config.screen_width;
        mode->h = Global::shared_config.screen_height;
    }

    mode->refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %p", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    return ret;
}

const sdl3::SDL_DisplayMode * sdl3::SDL_GetCurrentDisplayMode(SDL_DisplayID displayID)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d", __func__, displayID);

    const sdl3::SDL_DisplayMode* mode;
    static sdl3::SDL_DisplayMode new_mode;

    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        mode = ORIG_SDL3_CALL(SDL_GetCurrentDisplayMode, (displayID));
        new_mode = *mode;
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        mode = ORIG_SDL3_CALL(SDL_GetDesktopDisplayMode, (displayID));

        new_mode = *mode;
        new_mode.format = sdl3::SDL_PIXELFORMAT_RGB24;
        new_mode.w = Global::shared_config.screen_width;
        new_mode.h = Global::shared_config.screen_height;
    }

    new_mode.refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;
    new_mode.refresh_rate_numerator = Global::shared_config.initial_framerate_num;
    new_mode.refresh_rate_denominator = Global::shared_config.initial_framerate_den;

    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %p", new_mode.format, new_mode.w, new_mode.h, new_mode.refresh_rate, new_mode.internal);

    return &new_mode;
}

/* Override */ sdl2::SDL_DisplayMode *SDL_GetClosestDisplayMode(int displayIndex, const sdl2::SDL_DisplayMode * mode, sdl2::SDL_DisplayMode * closest)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with index %d and mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", __func__, displayIndex, mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    sdl2::SDL_DisplayMode *dm = nullptr;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        dm = ORIG_SDL2_CALL(SDL_GetClosestDisplayMode, (displayIndex, mode, closest));
    }
    else {
        /* We must get one real display mode to have a correct data parameter */
        int ret = ORIG_SDL2_CALL(SDL_GetDesktopDisplayMode, (displayIndex, closest));

        if (ret == 0) {
            closest->format = sdl2::SDL_PIXELFORMAT_RGB888;
            closest->w = Global::shared_config.screen_width;
            closest->h = Global::shared_config.screen_height;

            dm = closest;
        }
    }
    if (dm) {
        dm->refresh_rate = Global::shared_config.initial_framerate_num / Global::shared_config.initial_framerate_den;
    }

    return dm;
}

/* Override */ int SDL_GetWindowDisplayIndex(SDL_Window * window)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with window %d", __func__, (void*)window);

    int ret = 0;
    if (GlobalState::isNative() || !Global::shared_config.screen_width) {
        ret = ORIG_SDL2_CALL(SDL_GetWindowDisplayIndex, (window));
    }
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns index %d", ret);

    return ret;
}

/* Override */ int SDL_SetWindowDisplayMode(SDL_Window * window, const sdl2::SDL_DisplayMode* mode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with window %d and mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", __func__, (void*)window, mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    int ret = ORIG_SDL2_CALL(SDL_SetWindowDisplayMode, (window, mode));
    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns ret %d", ret);

    return ret;
}

/* Override */ int SDL_GetWindowDisplayMode(SDL_Window * window, sdl2::SDL_DisplayMode * mode)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s call with window %d", __func__, (void*)window);

    int ret = ORIG_SDL2_CALL(SDL_GetWindowDisplayMode, (window, mode));

    LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   returns mode format: %d, w: %d, h: %d, refresh rate: %d, data: %d", mode->format, mode->w, mode->h, mode->refresh_rate, mode->driverdata);

    return ret;
}

}

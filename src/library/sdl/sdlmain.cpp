/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdlmain.h"
#include "sdlversion.h"

#include "logging.h"
#include "hook.h"
#include "global.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_Init)
DECLARE_ORIG_POINTER(SDL_InitSubSystem)
DECLARE_ORIG_POINTER(SDL_WasInit)
DECLARE_ORIG_POINTER(SDL_Quit)

/* Override */ int SDL_Init(Uint32 flags){
    LOGTRACE(LCF_SDL);

    LINK_NAMESPACE_SDLX(SDL_Init);

    /* In both SDL1 and SDL2, SDL_Init() calls SDL_InitSubSystem(),
     * but in SDL2, SDL_Init() can actually never be called by the game,
     * so we put the rest of relevant code in the SubSystem function.
     *
     * ...well, this is in theory. If on SDL2 we call SDL_Init(), it
     * does not call our SDL_InitSubSystem() function. Maybe it has to do with
     * some compiler optimization, because the real SDL_Init() function looks
     * like this:
     *      int SDL_Init(Uint32 flags) {
     *          return SDL_InitSubSystem(flags);
     *      }
     * So maybe the compiler is inlining stuff. To fix this, we call
     * ourselves our own SDL_InitSubSystem() function.
     *
     * It seems to be the case with SDL1 for some compiled libraries,
     * so we also call our SDL_InitSubSystem function. This skips some code from
     * SDL_Init but nothing relevant.
     */
    return SDL_InitSubSystem(flags);
}

static Uint32 init_flags = 0;

/* Override */ int SDL_InitSubSystem(Uint32 flags){
    LOGTRACE(LCF_SDL);

    /* Get which sdl version we are using. */
    int SDLver = get_sdlversion();
    GameInfo::Flag sdl_flag = (SDLver==2)?GameInfo::SDL2:((SDLver==1)?GameInfo::SDL1:GameInfo::UNKNOWN);

    /* Link function pointers to SDL functions */
    LINK_NAMESPACE_SDLX(SDL_InitSubSystem);

    if (flags & SDL_INIT_TIMER)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_TIMER enabled.");

    if (flags & SDL_INIT_AUDIO) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_AUDIO fake enabled.");
        SDL_AudioInit(nullptr);
        Global::game_info.audio = sdl_flag;
    }

    if (flags & SDL_INIT_VIDEO) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_VIDEO enabled.");
        Global::game_info.video |= sdl_flag;
        Global::game_info.keyboard = sdl_flag;
        Global::game_info.mouse = sdl_flag;
    }

    if (flags & SDL_INIT_JOYSTICK) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_JOYSTICK fake enabled.");
        Global::game_info.joystick = sdl_flag;
    }

    if (flags & SDL_INIT_HAPTIC)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_HAPTIC fake enabled.");

    if (flags & SDL_INIT_GAMECONTROLLER) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_GAMECONTROLLER fake enabled.");
        Global::game_info.joystick = sdl_flag;
    }

    if (flags & SDL_INIT_EVENTS)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_EVENTS enabled.");

    Global::game_info.tosend = true;

    /* Save the inited flags before modifying them */
    /* TODO: inited subsystem are refcounted! */
    init_flags |= flags;

    /* Disabling Joystick subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ SDL_INIT_JOYSTICK;

    /* Disabling Haptic subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ SDL_INIT_HAPTIC;

    /* Disabling GameController subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ SDL_INIT_GAMECONTROLLER;

    /* Disabling Audio subsystem so that it does not create an extra thread */
    flags &= 0xFFFFFFFF ^ SDL_INIT_AUDIO;

    return orig::SDL_InitSubSystem(flags);
}

Uint32 SDL_WasInit(Uint32 flags)
{
    LOG(LL_TRACE, LCF_SDL, "%s with flags %d", __func__, flags);

    if (flags == 0)
        flags = SDL_INIT_EVERYTHING;

    return flags & init_flags;
}

/* Override */ void SDL_Quit()
{
    LOGTRACE(LCF_SDL);
    LINK_NAMESPACE_SDLX(SDL_Quit);
    orig::SDL_Quit();
}

}

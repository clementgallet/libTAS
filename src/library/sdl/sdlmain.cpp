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

#include "sdlmain.h"
#include "sdlversion.h"
#include "sdldynapi.h"

#include "logging.h"
#include "hook.h"
#include "global.h"

namespace libtas {

/* Override */ int SDL_Init(Uint32 flags){
    LOGTRACE(LCF_SDL);

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
    GameInfo::Flag sdl_flag;
    
    switch (SDLver) {
        case 1:
            sdl_flag = GameInfo::SDL1;
            break;
        case 2:
            sdl_flag = GameInfo::SDL2;
            break;
        case 3:
            sdl_flag = GameInfo::SDL3;
            break;
        default:
            sdl_flag = GameInfo::UNKNOWN;
            LOG(LL_ERROR, LCF_SDL, "Unknown SDL version detected!");
    }

    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_AUDIO) == static_cast<Uint32>(sdl3::SDL_INIT_AUDIO), "SDL2 and SDL3 audio init flag values must be the same");
    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_VIDEO) == static_cast<Uint32>(sdl3::SDL_INIT_VIDEO), "SDL2 and SDL3 video init flag values must be the same");
    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_JOYSTICK) == static_cast<Uint32>(sdl3::SDL_INIT_JOYSTICK), "SDL2 and SDL3 joystick init flag values must be the same");
    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_HAPTIC) == static_cast<Uint32>(sdl3::SDL_INIT_HAPTIC), "SDL2 and SDL3 haptic init flag values must be the same");
    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_GAMECONTROLLER) == static_cast<Uint32>(sdl3::SDL_INIT_GAMEPAD), "SDL2 and SDL3 game controller init flag values must be the same");
    static_assert(static_cast<Uint32>(sdl2::SDL_INIT_EVENTS) == static_cast<Uint32>(sdl3::SDL_INIT_EVENTS), "SDL2 and SDL3 events init flag values must be the same");

    if (flags & sdl2::SDL_INIT_TIMER)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_TIMER enabled.");

    if (flags & sdl2::SDL_INIT_AUDIO) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_AUDIO fake enabled.");
        // sdl2::SDL_AudioInit(nullptr);
        Global::game_info.audio = sdl_flag;
    }

    if (flags & sdl2::SDL_INIT_VIDEO) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_VIDEO enabled.");
        Global::game_info.video |= sdl_flag;
        Global::game_info.keyboard = sdl_flag;
        Global::game_info.mouse = sdl_flag;
    }

    if (flags & sdl2::SDL_INIT_JOYSTICK) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_JOYSTICK fake enabled.");
        Global::game_info.joystick = sdl_flag;
    }

    if (flags & sdl2::SDL_INIT_HAPTIC)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_HAPTIC fake enabled.");

    if (flags & sdl2::SDL_INIT_GAMECONTROLLER) {
        LOG(LL_DEBUG, LCF_SDL, "    SDL_GAMECONTROLLER fake enabled.");
        Global::game_info.joystick = sdl_flag;
    }

    if (flags & sdl2::SDL_INIT_EVENTS)
        LOG(LL_DEBUG, LCF_SDL, "    SDL_EVENTS enabled.");

    Global::game_info.tosend = true;

    /* Save the inited flags before modifying them */
    /* TODO: inited subsystem are refcounted! */
    init_flags |= flags;

    /* Disabling Joystick subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ sdl2::SDL_INIT_JOYSTICK;

    /* Disabling Haptic subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ sdl2::SDL_INIT_HAPTIC;

    /* Disabling GameController subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ sdl2::SDL_INIT_GAMECONTROLLER;

    /* Disabling Audio subsystem so that it does not create an extra thread */
    flags &= 0xFFFFFFFF ^ sdl2::SDL_INIT_AUDIO;

    return ORIG_SDL23_CALL(SDL_InitSubSystem, (flags));
}

Uint32 SDL_WasInit(Uint32 flags)
{
    LOG(LL_TRACE, LCF_SDL, "%s with flags %d", __func__, flags);

    if (flags == 0)
        return init_flags;

    return flags & init_flags;
}

/* Override */ void SDL_Quit()
{
    LOGTRACE(LCF_SDL);
    return ORIG_SDL23_CALL(SDL_Quit, ());
}

}

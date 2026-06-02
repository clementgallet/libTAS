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

#include "sdldynapi.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"
#include "rendering/vulkanwrappers.h"

namespace libtas {

/* Override */ sdl3::SDL_FunctionPointer SDL_Vulkan_GetVkGetInstanceProcAddr(void)
{
    if (GlobalState::isNative())
        return ORIG_SDL3_CALL(SDL_Vulkan_GetVkGetInstanceProcAddr, ());

    LOGTRACE(LCF_SDL | LCF_VULKAN);

    return reinterpret_cast<sdl3::SDL_FunctionPointer>(&vkGetInstanceProcAddr);
}

}

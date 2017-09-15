/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "pulseaudio_channelmap.h"
#include "../../logging.h"
#include "../../GlobalState.h"
#include "../../hook.h"

namespace libtas {

namespace orig {
    static pa_channel_map* (*pa_channel_map_init_auto)(pa_channel_map *m, unsigned channels, pa_channel_map_def_t def);
}

pa_channel_map* pa_channel_map_init_auto(pa_channel_map *m, unsigned channels, pa_channel_map_def_t def)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_channel_map_init_auto, "libpulse");
        return orig::pa_channel_map_init_auto(m, channels, def);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return m;
}

}

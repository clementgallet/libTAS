/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "pulseaudio_introspect.h"
#include "../../logging.h"

namespace libtas {

pa_operation* pa_context_get_sink_info_list(pa_context *c, pa_sink_info_cb_t cb, void *userdata)
{
    DEBUGLOGCALL(LCF_SOUND);
    static pa_sink_info sink {
        .name = "Dummy sink",
        .index = 1,
        .description = "Dummy sink description",
        .sample_spec = {PA_SAMPLE_S16LE, 48000, 2},
        .channel_map = {2, {PA_CHANNEL_POSITION_LEFT, PA_CHANNEL_POSITION_RIGHT}},
        .owner_module = 0,
        .volume = {2, {0x10000U, 0x10000U}},
        .mute = 0,
        .monitor_source = 0,
        .monitor_source_name = "Dummy source",
        .latency = 0,
        .driver = "Dummy driver"
    };

    cb(c, &sink, 0, userdata);

    cb(c, nullptr, 1, userdata);

    return reinterpret_cast<pa_operation *>(1);
}

pa_operation* pa_context_get_source_info_list(pa_context *c, pa_source_info_cb_t cb, void *userdata)
{
    DEBUGLOGCALL(LCF_SOUND);

    cb(c, nullptr, 1, userdata);
    return reinterpret_cast<pa_operation *>(1);
}

}
